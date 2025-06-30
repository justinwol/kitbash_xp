#include "kitbash.h"
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <chrono>

// CLI function declarations
void print_usage();
void print_help();
void print_version();
void print_error(const std::string& error_type, const std::string& message, const std::string& suggestion = "");
bool confirm_overwrite(const std::string& filename);
void print_detailed_summary(const MergeStats& stats);
std::string format_number(int number);  // Add commas for readability (e.g., "1,245")
bool validate_arguments(int argc, char* argv[]);

// Helper functions
bool is_obj_extension(const std::string& filename);
std::string to_lower(const std::string& str);

// Helper function implementations
bool is_obj_extension(const std::string& filename) {
    if (filename.length() < 4) return false;
    std::string ext = to_lower(filename.substr(filename.length() - 4));
    return ext == ".obj";
}

std::string to_lower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

// CLI implementation functions
void print_usage() {
    std::cout << "Usage: kitbash base.obj addition.obj [OPTIONS]\n";
    std::cout << "Use --help for detailed information.\n";
}

void print_help() {
    std::cout << "KITBASH - X-Plane OBJ8 File Merger\n";
    std::cout << "==================================\n\n";
    std::cout << "USAGE:\n";
    std::cout << "  kitbash base.obj addition.obj [OPTIONS]\n\n";
    std::cout << "DESCRIPTION:\n";
    std::cout << "  Merges X-Plane OBJ8 files by combining vertices, indices, and animations\n";
    std::cout << "  from the addition file into the base file. Automatically adjusts vertex\n";
    std::cout << "  and triangle indices to maintain proper references.\n\n";
    std::cout << "ARGUMENTS:\n";
    std::cout << "  base.obj      Base OBJ8 file (will be modified unless -o is used)\n";
    std::cout << "  addition.obj  Addition OBJ8 file to merge into base\n\n";
    std::cout << "OPTIONS:\n";
    std::cout << "  -s            Show detailed summary statistics\n";
    std::cout << "  -o FILE       Output to specified file (preserves original base file)\n";
    std::cout << "  -h, --help    Show this help message\n";
    std::cout << "  -v, --version Show version information\n\n";
    std::cout << "EXAMPLES:\n";
    std::cout << "  kitbash aircraft.obj landing_gear.obj\n";
    std::cout << "  kitbash -s -o merged.obj base.obj addon.obj\n\n";
    std::cout << "SAFETY:\n";
    std::cout << "  - Automatic backup created when overwriting original files\n";
    std::cout << "  - User confirmation required before any modifications\n";
    std::cout << "  - Clean output files with no kitbash traces\n\n";
    std::cout << "For more information, visit: https://github.com/user/kitbash\n";
}

void print_version() {
    std::cout << "kitbash 1.0.0\n";
    std::cout << "X-Plane OBJ8 File Merger\n";
    std::cout << "Copyright (c) 2025\n";
}

void print_error(const std::string& error_type, const std::string& message, const std::string& suggestion) {
    std::cout << "KITBASH ERROR\n";
    std::cout << "=============\n\n";
    
    if (error_type == "invalid_args") {
        std::cout << "Usage:\n";
        std::cout << "  Invalid number of arguments provided\n";
        std::cout << "    Expected: kitbash base.obj addition.obj [OPTIONS]\n\n";
        std::cout << "Usage:\n";
        std::cout << "  kitbash base.obj addition.obj [-s] [-o output.obj]\n";
    } else if (error_type == "invalid_switch") {
        std::cout << "Command Line:\n";
        std::cout << "  Invalid switch: '" << message << "'\n";
        std::cout << "    Expected: -s, -o, -h, --help, -v, --version\n\n";
        std::cout << "Usage:\n";
        std::cout << "  kitbash base.obj addition.obj [-s] [-o output.obj]\n";
    } else if (error_type == "invalid_obj") {
        std::cout << "File Validation:\n";
        std::cout << "  Invalid file extension: '" << message << "'\n";
        std::cout << "    Expected: .obj file (e.g., model.obj)\n\n";
        std::cout << "Usage:\n";
        std::cout << "  kitbash base.obj addition.obj [-s] [-o output.obj]\n";
    } else if (error_type == "file_not_found") {
        std::cout << "File Access:\n";
        std::cout << "  " << message << "\n";
        std::cout << "    Check: File exists and path is correct\n";
    } else if (error_type == "backup_failed") {
        std::cout << "Backup Operations:\n";
        std::cout << "  " << message << "\n";
        std::cout << "    Check: Write permissions and disk space\n";
    } else if (error_type == "merge_failed") {
        std::cout << "Processing:\n";
        std::cout << "  " << message << "\n";
        std::cout << "    Check: Both files are valid OBJ8 format\n";
    } else {
        std::cout << "System:\n";
        std::cout << "  " << error_type << ": " << message << "\n";
        if (!suggestion.empty()) {
            std::cout << "    Check: " << suggestion << "\n";
        }
    }
}

bool confirm_overwrite(const std::string& filename) {
    std::cout << "Warning: This operation will overwrite " << filename << "\n";
    std::cout << "Please confirm you have a backup before proceeding.\n";
    std::cout << "Continue? (Y/N): ";
    
    std::string response;
    std::getline(std::cin, response);
    
    // Match Lua behavior: only "Y" (uppercase) continues
    return !response.empty() && std::toupper(response[0]) == 'Y';
}

void print_detailed_summary(const MergeStats& stats) {
    // Enhanced summary format as specified in the plan
    std::cout << "\nKITBASH MERGE SUMMARY\n";
    std::cout << "=====================\n\n";
    
    std::cout << "Input Files:\n";
    std::cout << "  Base:     " << stats.base_filename
              << " (" << format_number(stats.original_line_count) << " lines, "
              << format_number(stats.original_vt_count) << " vertices, "
              << format_number(stats.original_tris_count) << " triangles)\n";
    std::cout << "  Addition: " << stats.addition_filename
              << " (" << format_number(stats.added_line_count) << " lines, "
              << format_number(stats.added_vt_count) << " vertices, "
              << format_number(stats.added_tris_count) << " triangles)\n\n";
    
    std::cout << "Changes:\n";
    std::cout << "  + Added " << format_number(stats.added_line_count)
              << " lines (+" << std::fixed << std::setprecision(1)
              << stats.line_increase_percent() << "%)\n";
    std::cout << "  + Added " << format_number(stats.added_vt_count)
              << " vertices (+" << std::fixed << std::setprecision(1)
              << stats.vt_increase_percent() << "%)\n";
    std::cout << "  + Added " << format_number(stats.added_tris_count)
              << " triangles (+" << std::fixed << std::setprecision(1)
              << stats.tris_increase_percent() << "%)\n\n";
    
    std::cout << "Result:\n";
    std::cout << "  Output:   " << stats.output_filename
              << " (" << format_number(stats.final_line_count) << " lines, "
              << format_number(stats.final_vt_count) << " vertices, "
              << format_number(stats.final_tris_count) << " triangles)\n";
    if (!stats.backup_filename.empty()) {
        std::cout << "  Backup:   " << stats.backup_filename << "\n";
    }
    std::cout << "\nCompleted successfully in " << std::fixed << std::setprecision(3)
              << stats.processing_time << " seconds.\n";
}

std::string format_number(int number) {
    // Add commas for readability (e.g., "1,245")
    std::string num_str = std::to_string(number);
    std::string result;
    
    int count = 0;
    for (int i = static_cast<int>(num_str.length()) - 1; i >= 0; --i) {
        if (count > 0 && count % 3 == 0) {
            result = ',' + result;
        }
        result = num_str[i] + result;
        count++;
    }
    
    return result;
}

bool validate_arguments(int argc, char* argv[]) {
    // This function is now simplified since main() handles complex parsing
    // Minimum: program name + 2 files = 3 args
    // Maximum: program name + flags + output + 2 files = reasonable limit
    (void)argv; // Suppress unused parameter warning
    
    if (argc < 3) {
        return false;
    }
    
    // Allow reasonable maximum (program + multiple flags + output + 2 files)
    if (argc > 8) {
        return false;
    }
    
    return true;
}

int main(int argc, char* argv[]) {
    // Handle no arguments case
    if (argc == 1) {
        print_usage();
        return 0;
    }
    
    // Handle help and version flags first
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            print_help();
            return 0;
        }
        if (arg == "--version" || arg == "-v") {
            print_version();
            return 0;
        }
    }
    
    // Parse command line arguments
    bool wants_summary = false;
    bool has_output_file = false;
    std::string base_file;
    std::string addition_file;
    std::string output_file;
    
    // Parse arguments - handle various combinations
    std::vector<std::string> non_flag_args;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-s") {
            wants_summary = true;
        } else if (arg == "-o") {
            // Next argument should be output file
            if (i + 1 >= argc) {
                print_error("invalid_args", "Missing output filename after -o", "");
                return 1;
            }
            output_file = argv[++i];  // Consume next argument
            has_output_file = true;
        } else if (arg[0] == '-') {
            // Unknown flag
            print_error("invalid_switch", arg, "");
            return 1;
        } else {
            // Non-flag argument (should be filename)
            non_flag_args.push_back(arg);
        }
    }
    
    // Validate we have exactly 2 input files
    if (non_flag_args.size() != 2) {
        print_error("invalid_args", "", "");
        return 1;
    }
    
    base_file = non_flag_args[0];
    addition_file = non_flag_args[1];
    
    // If no output file specified, use base file (in-place modification)
    if (!has_output_file) {
        output_file = base_file;
    }
    
    // Validate file extensions (case-insensitive .obj)
    if (!is_obj_extension(base_file)) {
        print_error("invalid_obj", base_file, "");
        return 1;
    }
    
    if (!is_obj_extension(addition_file)) {
        print_error("invalid_obj", addition_file, "");
        return 1;
    }
    
    if (has_output_file && !is_obj_extension(output_file)) {
        print_error("invalid_obj", output_file, "");
        return 1;
    }
    
    // Check if input files exist
    if (!std::filesystem::exists(base_file)) {
        print_error("file_not_found", "Base file '" + base_file + "' not found", "Check the file path and try again");
        return 1;
    }
    
    if (!std::filesystem::exists(addition_file)) {
        print_error("file_not_found", "Addition file '" + addition_file + "' not found", "Check the file path and try again");
        return 1;
    }
    
    // Create backup if overwriting original file (no -o flag)
    std::string backup_filename;
    if (!has_output_file) {
        // Confirm overwrite with user (matching Lua script behavior exactly)
        if (!confirm_overwrite(base_file)) {
            std::cout << "\nUser Actions:\n";
            std::cout << "  Operation cancelled by user\n";
            std::cout << "    Note: No files were modified\n";
            return 0;
        }
        
        // Create backup before overwriting
        backup_filename = kitbash::generate_backup_filename(base_file);
        if (!kitbash::create_backup(base_file)) {
            print_error("backup_failed", "Failed to create backup of " + base_file, "Check file permissions");
            return 1;
        }
        std::cout << "Creating backup: " << backup_filename << "\n";
    }
    
    // Perform the merge operation
    try {
        MergeStats stats;
        
        // Set filenames for stats
        stats.base_filename = base_file;
        stats.addition_filename = addition_file;
        stats.output_filename = output_file;
        if (!has_output_file) {
            stats.backup_filename = backup_filename;
        } else {
            stats.backup_filename = ""; // No backup for separate output
        }
        
        // Record start time for benchmarking
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Call the kitbash merge function
        bool success = kitbash::merge_to_file_with_stats(base_file, addition_file, output_file, &stats);
        
        // Record end time
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double>(end_time - start_time);
        stats.processing_time = duration.count();
        
        if (!success) {
            print_error("merge_failed", "Failed to merge files", "Check that both files are valid OBJ8 format");
            return 1;
        }
        
        // Print completion message
        std::cout << "Merge completed successfully.\n";
        
        // Print summary if requested
        if (wants_summary) {
            print_detailed_summary(stats);
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        print_error("exception", e.what(), "Check file permissions and disk space");
        return 1;
    }
}