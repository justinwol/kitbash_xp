#ifndef KITBASH_H
#define KITBASH_H

#include <string>
#include <vector>

// Forward declarations
struct MergeStats;

// C-style API for library usage
extern "C" {
    // Main kitbash function - merges addition file into base file
    int kitbash_merge(const char* base_file, const char* addition_file);
    
    // Advanced function with custom output file
    int kitbash_merge_to_file(const char* base_file, const char* addition_file, const char* output_file);
    
    // Get statistics about an OBJ file
    int kitbash_get_stats(const char* obj_file, int* vt_count, int* tris_count);
    
    // Error handling
    const char* kitbash_get_last_error();
}

// Core data structures
struct ObjLine {
    std::string type;           // "VT", "IDX", "TRIS", etc.
    std::string content;        // Full line content
    std::vector<std::string> tokens; // Parsed tokens
};

struct ObjInfo {
    int vt_count = 0;
    int tris_count = 0;
    int line_count = 0;         // Total lines in file
    std::vector<ObjLine> lines;
};

struct MergeStats {
    int original_vt_count = 0;
    int original_tris_count = 0;
    int original_line_count = 0;
    int added_vt_count = 0;
    int added_tris_count = 0;
    int added_line_count = 0;
    int final_vt_count = 0;
    int final_tris_count = 0;
    int final_line_count = 0;
    double processing_time = 0.0;
    std::string base_filename;
    std::string addition_filename;
    std::string output_filename;
    std::string backup_filename;
    
    // Calculated percentages for display
    double vt_increase_percent() const {
        return original_vt_count > 0 ? (double)added_vt_count / original_vt_count * 100.0 : 0.0;
    }
    double tris_increase_percent() const {
        return original_tris_count > 0 ? (double)added_tris_count / original_tris_count * 100.0 : 0.0;
    }
    double line_increase_percent() const {
        return original_line_count > 0 ? (double)added_line_count / original_line_count * 100.0 : 0.0;
    }
};

// C++ convenience wrapper namespace
namespace kitbash {
    // Simple merge functions
    bool merge(const std::string& base, const std::string& addition);
    bool merge_to_file(const std::string& base, const std::string& addition, const std::string& output);
    
    // Statistics structure for C++ API
    struct Stats {
        int vt_count = 0;
        int tris_count = 0;
        int line_count = 0;
    };
    Stats get_stats(const std::string& obj_file);
    
    // Advanced merge with detailed statistics
    bool merge_with_stats(const std::string& base, const std::string& addition, MergeStats* stats = nullptr);
    bool merge_to_file_with_stats(const std::string& base, const std::string& addition,
                                  const std::string& output, MergeStats* stats = nullptr);
    
    // Core file operations
    std::vector<std::string> read_file(const std::string& filename);
    void write_file(const std::string& filename, const std::vector<std::string>& lines);
    bool create_backup(const std::string& filename);
    std::string generate_backup_filename(const std::string& filename);
    bool is_obj_file(const std::string& filename);
    bool validate_obj_format(const std::vector<std::string>& lines);
}

#endif // KITBASH_H