#include "kitbash.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <algorithm>

// Global error message for C API
static std::string g_last_error;

// Internal helper functions
namespace {
    // Forward declarations
    std::vector<std::string> tokenize(const std::string& line);
    std::pair<int, int> extract_point_counts(const std::string& line);
    std::string adjust_indices_line(const std::string& line, int vt_offset);
    std::string adjust_tris_line(const std::string& line, int tris_offset);
    
    // File I/O functions
    std::vector<std::string> read_file(const std::string& filename) {
        std::vector<std::string> lines;
        std::ifstream file(filename);
        
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filename);
        }
        
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        
        file.close();
        return lines;
    }

    void write_file(const std::string& filename, const std::vector<std::string>& lines) {
        std::ofstream file(filename);
        
        if (!file.is_open()) {
            throw std::runtime_error("Cannot create file: " + filename);
        }
        
        for (const auto& line : lines) {
            file << line << "\n";
        }
        
        file.close();
    }

    // Helper function to extract VT and TRIS counts from POINT_COUNTS line
    std::pair<int, int> extract_point_counts(const std::string& line) {
        // Parse POINT_COUNTS line to get VT and TRIS values
        // Format: "POINT_COUNTS VT_count line_count light_count TRIS_count"
        // VT count is at position 2 (index 1), TRIS count is at position 5 (index 4)
        std::vector<std::string> tokens = tokenize(line);
        
        if (tokens.size() >= 5 && tokens[0] == "POINT_COUNTS") {
            try {
                int vt_count = std::stoi(tokens[1]);    // Position 2 (index 1)
                int tris_count = std::stoi(tokens[4]);  // Position 5 (index 4)
                return std::make_pair(vt_count, tris_count);
            } catch (const std::exception&) {
                // If parsing fails, return zeros
                return std::make_pair(0, 0);
            }
        }
        
        return std::make_pair(0, 0);
    }

    // Parsing functions
    ObjInfo parse_obj(const std::vector<std::string>& lines) {
        // Parse lines and extract VT/TRIS counts and categorize lines into ObjInfo structure
        // Replicates the behavior of Lua's get_objInfo() function
        ObjInfo info;
        info.line_count = static_cast<int>(lines.size());
        
        // Parse through all lines looking for POINT_COUNTS and categorizing lines
        for (size_t i = 0; i < lines.size(); ++i) {
            const std::string& line = lines[i];
            
            // Skip empty lines
            if (line.empty()) {
                continue;
            }
            
            // Check for POINT_COUNTS line to extract VT and TRIS counts
            if (line.find("POINT_COUNTS") != std::string::npos) {
                auto counts = extract_point_counts(line);
                info.vt_count = counts.first;
                info.tris_count = counts.second;
            }
            
            // Create ObjLine for each line and categorize by type
            ObjLine obj_line;
            obj_line.content = line;
            obj_line.tokens = tokenize(line);
            
            // Determine line type from first token
            if (!obj_line.tokens.empty()) {
                obj_line.type = obj_line.tokens[0];
            } else {
                obj_line.type = ""; // Empty line
            }
            
            info.lines.push_back(obj_line);
        }
        
        return info;
    }

    std::vector<std::string> tokenize(const std::string& line) {
        // Split line into tokens using whitespace (like Lua's string.gmatch(line, "%S+"))
        std::vector<std::string> tokens;
        std::istringstream iss(line);
        std::string token;
        
        // Extract each non-whitespace token
        while (iss >> token) {
            tokens.push_back(token);
        }
        
        return tokens;
    }

    // Core merging logic - replicates kitbash.lua behavior exactly
    std::vector<std::string> merge_objects(const ObjInfo& base, const ObjInfo& addition, MergeStats* stats = nullptr) {
        (void)stats; // Suppress unused parameter warning
        std::vector<std::string> result;
        
        // 1. Combine headers and update POINT_COUNTS with new totals
        for (const auto& line : base.lines) {
            if (line.content.find("POINT_COUNTS") != std::string::npos) {
                // Update POINT_COUNTS with combined totals
                auto tokens = tokenize(line.content);
                if (tokens.size() >= 5) {
                    int new_vt_count = base.vt_count + addition.vt_count;
                    int new_tris_count = base.tris_count + addition.tris_count;
                    std::string new_point_counts = "POINT_COUNTS " + std::to_string(new_vt_count) +
                                                 " " + tokens[2] + " " + tokens[3] + " " + std::to_string(new_tris_count);
                    result.push_back(new_point_counts);
                }
                break; // Stop after header
            } else {
                result.push_back(line.content);
            }
        }
        
        // 2. Copy all base VT lines
        for (const auto& line : base.lines) {
            if (line.type == "VT") {
                result.push_back(line.content);
            }
        }
        
        // 3. Copy all addition VT lines
        for (const auto& line : addition.lines) {
            if (line.type == "VT") {
                result.push_back(line.content);
            }
        }
        
        // 4. Copy all base IDX/IDX10 lines
        for (const auto& line : base.lines) {
            if (line.type == "IDX" || line.type == "IDX10") {
                result.push_back(line.content);
            }
        }
        
        // 5. Copy addition IDX/IDX10 lines with adjusted vertex indices
        for (const auto& line : addition.lines) {
            if (line.type == "IDX" || line.type == "IDX10") {
                result.push_back(adjust_indices_line(line.content, base.vt_count));
            }
        }
        
        // 6. Copy base footer (ANIM sections)
        bool past_idx = false;
        for (const auto& line : base.lines) {
            if (line.type == "IDX" || line.type == "IDX10") {
                past_idx = true;
            } else if (past_idx && line.type != "IDX" && line.type != "IDX10") {
                result.push_back(line.content);
            }
        }
        
        // 7. Add attributes and addition footer with adjusted TRIS indices
        result.push_back("\tATTR_draw_enable");
        result.push_back("\tATTR_cockpit");
        
        past_idx = false;
        for (const auto& line : addition.lines) {
            if (line.type == "IDX" || line.type == "IDX10") {
                past_idx = true;
            } else if (past_idx && line.type != "IDX" && line.type != "IDX10") {
                if (line.type == "TRIS") {
                    result.push_back(adjust_tris_line(line.content, base.tris_count));
                } else {
                    result.push_back(line.content);
                }
            }
        }
        
        return result;
    }

    // Adjust vertex indices in IDX/IDX10 lines
    std::string adjust_indices_line(const std::string& line, int vt_offset) {
        auto tokens = tokenize(line);
        if (tokens.empty()) return line;
        
        std::string result = tokens[0]; // IDX or IDX10
        
        // Add vt_offset to each vertex index (starting from token 1)
        for (size_t i = 1; i < tokens.size(); ++i) {
            try {
                int index = std::stoi(tokens[i]);
                result += "\t" + std::to_string(index + vt_offset);
            } catch (const std::exception&) {
                result += "\t" + tokens[i]; // Keep original if not a number
            }
        }
        
        return result;
    }

    // TRIS line processing (preserves indentation exactly like Lua script)
    std::string adjust_tris_line(const std::string& line, int tris_offset) {
        auto tokens = tokenize(line);
        if (tokens.size() < 3 || tokens[0] != "TRIS") {
            return line; // Not a valid TRIS line
        }
        
        // Find the position of "TRIS" in the original line to preserve indentation
        size_t tris_pos = line.find("TRIS");
        if (tris_pos == std::string::npos) {
            return line;
        }
        
        try {
            // Extract and adjust the TRIS index (second token)
            int tris_index = std::stoi(tokens[1]);
            int adjusted_index = tris_index + tris_offset;
            
            // Reconstruct line preserving indentation: [indentation]TRIS[tab][adjusted_index][tab][count]
            std::string indentation = line.substr(0, tris_pos);
            std::string result = indentation + "TRIS\t" + std::to_string(adjusted_index) + "\t" + tokens[2];
            
            return result;
        } catch (const std::exception&) {
            return line; // Return original if parsing fails
        }
    }

    // Helper function for adjusting indices (used by merge_objects)
    std::vector<std::string> adjust_indices(const std::vector<std::string>& lines, int vt_offset, int tris_offset) {
        std::vector<std::string> adjusted;
        
        for (const auto& line : lines) {
            auto tokens = tokenize(line);
            if (tokens.empty()) {
                adjusted.push_back(line);
                continue;
            }
            
            if (tokens[0] == "IDX" || tokens[0] == "IDX10") {
                adjusted.push_back(adjust_indices_line(line, vt_offset));
            } else if (tokens[0] == "TRIS") {
                adjusted.push_back(adjust_tris_line(line, tris_offset));
            } else {
                adjusted.push_back(line);
            }
        }
        
        return adjusted;
    }

    // Add required X-Plane attributes (ATTR_draw_enable, ATTR_cockpit)
    std::vector<std::string> add_gizmo_attributes(const std::vector<std::string>& lines) {
        auto result = lines;
        result.push_back("\tATTR_draw_enable");
        result.push_back("\tATTR_cockpit");
        return result;
    }

    // Backup and utilities
    std::string generate_backup_filename(const std::string& filename) {
        return filename + ".bak";
    }

    bool create_backup(const std::string& filename) {
        try {
            if (!std::filesystem::exists(filename)) {
                return false; // Original file doesn't exist
            }
            
            std::string backup_filename = generate_backup_filename(filename);
            std::filesystem::copy_file(filename, backup_filename,
                                     std::filesystem::copy_options::overwrite_existing);
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }

    bool is_obj_file(const std::string& filename) {
        if (filename.size() < 4) {
            return false;
        }
        
        // Convert last 4 characters to uppercase for case-insensitive comparison
        std::string extension = filename.substr(filename.size() - 4);
        std::transform(extension.begin(), extension.end(), extension.begin(),
                      [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        
        return extension == ".OBJ";
    }

    bool validate_obj_format(const std::vector<std::string>& lines) {
        // Basic OBJ8 format validation - check for "800" version and "OBJ" type
        if (lines.size() < 3) {
            return false; // Need at least 3 lines for basic format
        }
        
        // Line 2 should contain "800" (version)
        if (lines.size() > 1 && lines[1].find("800") == std::string::npos) {
            return false;
        }
        
        // Line 3 should contain "OBJ" (type)
        if (lines.size() > 2 && lines[2].find("OBJ") == std::string::npos) {
            return false;
        }
        
        return true;
    }

    void set_last_error(const std::string& error) {
        g_last_error = error;
    }
}

// C API Implementation
extern "C" {
    int kitbash_merge(const char* base_file, const char* addition_file) {
        try {
            // Load both files
            auto base_lines = read_file(base_file);
            auto addition_lines = read_file(addition_file);
            
            // Validate OBJ format
            if (!validate_obj_format(base_lines) || !validate_obj_format(addition_lines)) {
                set_last_error("Invalid OBJ8 format");
                return -1;
            }
            
            // Parse both files
            ObjInfo base_info = parse_obj(base_lines);
            ObjInfo addition_info = parse_obj(addition_lines);
            
            // Create backup
            if (!create_backup(base_file)) {
                set_last_error("Failed to create backup");
                return -1;
            }
            
            // Merge objects
            auto merged_lines = merge_objects(base_info, addition_info);
            
            // Write result back to base file
            write_file(base_file, merged_lines);
            
            return 0; // Success
        } catch (const std::exception& e) {
            set_last_error(e.what());
            return -1; // Error
        }
    }

    int kitbash_merge_to_file(const char* base_file, const char* addition_file, const char* output_file) {
        try {
            // Load both files
            auto base_lines = read_file(base_file);
            auto addition_lines = read_file(addition_file);
            
            // Validate OBJ format
            if (!validate_obj_format(base_lines) || !validate_obj_format(addition_lines)) {
                set_last_error("Invalid OBJ8 format");
                return -1;
            }
            
            // Parse both files
            ObjInfo base_info = parse_obj(base_lines);
            ObjInfo addition_info = parse_obj(addition_lines);
            
            // Merge objects
            auto merged_lines = merge_objects(base_info, addition_info);
            
            // Write result to output file
            write_file(output_file, merged_lines);
            
            return 0; // Success
        } catch (const std::exception& e) {
            set_last_error(e.what());
            return -1; // Error
        }
    }

    int kitbash_get_stats(const char* obj_file, int* vt_count, int* tris_count) {
        try {
            // Load file
            auto lines = read_file(obj_file);
            
            // Validate OBJ format
            if (!validate_obj_format(lines)) {
                set_last_error("Invalid OBJ8 format");
                return -1;
            }
            
            // Parse file
            ObjInfo info = parse_obj(lines);
            
            // Return counts
            if (vt_count) *vt_count = info.vt_count;
            if (tris_count) *tris_count = info.tris_count;
            
            return 0; // Success
        } catch (const std::exception& e) {
            set_last_error(e.what());
            return -1; // Error
        }
    }

    const char* kitbash_get_last_error() {
        return g_last_error.c_str();
    }
}

// C++ API Implementation
namespace kitbash {
    bool merge(const std::string& base, const std::string& addition) {
        return kitbash_merge(base.c_str(), addition.c_str()) == 0;
    }

    bool merge_to_file(const std::string& base, const std::string& addition, const std::string& output) {
        return kitbash_merge_to_file(base.c_str(), addition.c_str(), output.c_str()) == 0;
    }

    Stats get_stats(const std::string& obj_file) {
        Stats stats;
        int vt_count, tris_count;
        if (kitbash_get_stats(obj_file.c_str(), &vt_count, &tris_count) == 0) {
            stats.vt_count = vt_count;
            stats.tris_count = tris_count;
            
            // Get line count by reading the file
            try {
                auto lines = ::read_file(obj_file);
                stats.line_count = static_cast<int>(lines.size());
            } catch (const std::exception&) {
                stats.line_count = 0;
            }
        }
        return stats;
    }

    bool merge_with_stats(const std::string& base, const std::string& addition, MergeStats* stats) {
        try {
            auto start_time = std::chrono::high_resolution_clock::now();
            
            // Load both files
            auto base_lines = ::read_file(base);
            auto addition_lines = ::read_file(addition);
            
            // Validate OBJ format
            if (!::validate_obj_format(base_lines) || !::validate_obj_format(addition_lines)) {
                return false;
            }
            
            // Parse both files
            ObjInfo base_info = ::parse_obj(base_lines);
            ObjInfo addition_info = ::parse_obj(addition_lines);
            
            // Populate initial stats
            if (stats) {
                stats->base_filename = base;
                stats->addition_filename = addition;
                stats->output_filename = base;
                stats->backup_filename = ::generate_backup_filename(base);
                stats->original_vt_count = base_info.vt_count;
                stats->original_tris_count = base_info.tris_count;
                stats->original_line_count = base_info.line_count;
                stats->added_vt_count = addition_info.vt_count;
                stats->added_tris_count = addition_info.tris_count;
                stats->added_line_count = addition_info.line_count;
            }
            
            // Create backup
            if (!::create_backup(base)) {
                return false;
            }
            
            // Merge objects
            auto merged_lines = ::merge_objects(base_info, addition_info);
            
            // Write result back to base file
            ::write_file(base, merged_lines);
            
            // Complete stats
            if (stats) {
                stats->final_vt_count = stats->original_vt_count + stats->added_vt_count;
                stats->final_tris_count = stats->original_tris_count + stats->added_tris_count;
                stats->final_line_count = static_cast<int>(merged_lines.size());
                
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
                stats->processing_time = duration.count() / 1000000.0; // Convert to seconds
            }
            
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }

    bool merge_to_file_with_stats(const std::string& base, const std::string& addition,
                                  const std::string& output, MergeStats* stats) {
        try {
            auto start_time = std::chrono::high_resolution_clock::now();
            
            // Load both files
            auto base_lines = ::read_file(base);
            auto addition_lines = ::read_file(addition);
            
            // Validate OBJ format
            if (!::validate_obj_format(base_lines) || !::validate_obj_format(addition_lines)) {
                return false;
            }
            
            // Parse both files
            ObjInfo base_info = ::parse_obj(base_lines);
            ObjInfo addition_info = ::parse_obj(addition_lines);
            
            // Populate initial stats
            if (stats) {
                stats->base_filename = base;
                stats->addition_filename = addition;
                stats->output_filename = output;
                // Don't overwrite backup_filename if it was already set
                stats->original_vt_count = base_info.vt_count;
                stats->original_tris_count = base_info.tris_count;
                stats->original_line_count = base_info.line_count;
                stats->added_vt_count = addition_info.vt_count;
                stats->added_tris_count = addition_info.tris_count;
                stats->added_line_count = addition_info.line_count;
            }
            
            // Merge objects
            auto merged_lines = ::merge_objects(base_info, addition_info);
            
            // Write result to output file
            ::write_file(output, merged_lines);
            
            // Complete stats
            if (stats) {
                stats->final_vt_count = stats->original_vt_count + stats->added_vt_count;
                stats->final_tris_count = stats->original_tris_count + stats->added_tris_count;
                stats->final_line_count = static_cast<int>(merged_lines.size());
                
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
                stats->processing_time = duration.count() / 1000000.0; // Convert to seconds
            }
            
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }

    // Core file operations - public wrappers around internal functions
    std::vector<std::string> read_file(const std::string& filename) {
        return ::read_file(filename);
    }

    void write_file(const std::string& filename, const std::vector<std::string>& lines) {
        ::write_file(filename, lines);
    }

    bool create_backup(const std::string& filename) {
        return ::create_backup(filename);
    }

    std::string generate_backup_filename(const std::string& filename) {
        return ::generate_backup_filename(filename);
    }

    bool is_obj_file(const std::string& filename) {
        return ::is_obj_file(filename);
    }

    bool validate_obj_format(const std::vector<std::string>& lines) {
        return ::validate_obj_format(lines);
    }
}