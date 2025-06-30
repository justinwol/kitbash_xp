Kitbash C++ Implementation Plan

## Overview

This plan outlines a minimal, robust C++ implementation of the kitbash.lua functionality for merging X-Plane OBJ8 files. The implementation will provide both a standalone CLI tool and a library interface for integration into C++ projects. The focus is on simplicity, optimization, minimal code, and YAGNI principles.

## Core Functionality

The tool merges two OBJ8 files by:
1. Reading the base object file and the addition object file
2. Combining vertices (VT), indices (IDX/IDX10), and animations (ANIM/TRIS)
3. Adjusting vertex and triangle indices for the addition object data
4. Adding required X-Plane attributes for proper rendering
5. Writing the merged result back to the base object file (with user confirmation)

## Architecture

### Dual-Purpose Design
- **kitbash.h** - Header with library interface (~50 lines)
- **kitbash.cpp** - Core implementation (~300-400 lines)
- **main.cpp** - CLI wrapper (~100 lines)
- Simple C-style API for library usage
- Standalone executable for command-line usage

### Core Data Structures

```cpp
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
```

### Library Interface (kitbash.h)

```cpp
// C-style API for library usage
extern "C" {
    // Main kitbash function
    int kitbash_merge(const char* base_file, const char* addition_file);
    
    // Advanced function with output file
    int kitbash_merge_to_file(const char* base_file, const char* addition_file, const char* output_file);
    
    // Get statistics about an OBJ file
    int kitbash_get_stats(const char* obj_file, int* vt_count, int* tris_count);
    
    // Error handling
    const char* kitbash_get_last_error();
}

// C++ convenience wrapper
namespace kitbash {
    bool merge(const std::string& base, const std::string& addition);
    bool merge_to_file(const std::string& base, const std::string& addition, const std::string& output);
    
    struct Stats { int vt_count, tris_count, line_count; };
    Stats get_stats(const std::string& obj_file);
    
    // Advanced merge with detailed statistics
    bool merge_with_stats(const std::string& base, const std::string& addition, MergeStats* stats = nullptr);
    bool merge_to_file_with_stats(const std::string& base, const std::string& addition,
                                  const std::string& output, MergeStats* stats = nullptr);
}
```

### Internal Functions (kitbash.cpp)

```cpp
// File I/O
std::vector<std::string> read_file(const std::string& filename);
void write_file(const std::string& filename, const std::vector<std::string>& lines);

// Parsing
ObjInfo parse_obj(const std::vector<std::string>& lines);
std::vector<std::string> tokenize(const std::string& line);

// Core merging logic
std::vector<std::string> merge_objects(const ObjInfo& base, const ObjInfo& addition, MergeStats* stats = nullptr);
std::vector<std::string> adjust_indices(const std::vector<std::string>& lines, int vt_offset, int tris_offset);

// Section processing
std::vector<std::string> add_gizmo_attributes(const std::vector<std::string>& lines);

// TRIS line processing (preserves indentation)
std::string adjust_tris_line(const std::string& line, int tris_offset);

// Backup and utilities
bool create_backup(const std::string& filename);
std::string generate_backup_filename(const std::string& filename);
bool is_obj_file(const std::string& filename);
bool validate_obj_format(const std::vector<std::string>& lines);
```

### CLI Functions (main.cpp)

```cpp
void print_usage();
void print_help();
void print_version();
void print_error(const std::string& error_type, const std::string& message, const std::string& suggestion = "");
bool confirm_overwrite(const std::string& filename);
void print_detailed_summary(const MergeStats& stats);
std::string format_number(int number);  // Add commas for readability (e.g., "1,245")
bool validate_arguments(int argc, char* argv[]);
int main(int argc, char* argv[]);
```

## Implementation Details

### 1. File Structure
```
kitbash/
├── kitbash.h          # Library header interface
├── kitbash.cpp        # Core implementation
├── main.cpp           # CLI wrapper
├── CMakeLists.txt     # Build configuration
└── README.md          # Usage instructions
```

### 2. Command Line Interface
```bash
kitbash base.obj addition.obj [-s] [-o output.obj] [--help] [--version]
```
- Required: base.obj, addition.obj
- Optional: -s for summary statistics
- Optional: -o output.obj for custom output file
- Optional: --help, -h for usage information
- Optional: --version, -v for version information
- No complex argument parsing - simple string comparisons

#### CLI Workflow
1. **Argument Validation**: Check file extensions (.obj required)
2. **File Existence**: Verify both input files exist
3. **Backup Creation**: If overwriting original file (no -o flag), create automatic backup
   ```
   Creating backup: base.obj.bak
   ```
4. **User Confirmation**: Prompt before overwriting target file
   ```
   This will overwrite base.obj. A backup has been created as base.obj.bak. Do you want to continue? (Type Y to continue, anything else will cancel.):
   ```
5. **Processing**: Perform merge with optional timing
6. **Summary**: Display detailed statistics if -s flag used

### 3. Library Usage
```cpp
#include "kitbash.h"

// C API
int result = kitbash_merge("aircraft.obj", "landing_gear.obj");
if (result != 0) {
    printf("Error: %s\n", kitbash_get_last_error());
}

// C++ API
if (!kitbash::merge("base.obj", "addon.obj")) {
    // Handle error
}

// Get statistics
auto stats = kitbash::get_stats("model.obj");
printf("VT: %d, TRIS: %d\n", stats.vt_count, stats.tris_count);
```

### 4. Core Algorithm

#### Step 1: Read and Parse Files
```cpp
auto base_lines = read_file(base_file);
auto addition_lines = read_file(addition_file);
auto base_info = parse_obj(base_lines);
auto addition_info = parse_obj(addition_lines);
```

#### Step 2: Extract Counts
- Parse POINT_COUNTS line to get VT and TRIS counts
- Store for index adjustment calculations

#### Step 3: Merge Components
```cpp
std::vector<std::string> result;

// 1. Copy header up to POINT_COUNTS
// 2. Write updated POINT_COUNTS with combined totals
// 3. Copy all base object VT lines
// 4. Copy all addition object VT lines
// 5. Copy all base object IDX/IDX10 lines
// 6. Copy addition object IDX/IDX10 lines with adjusted vertex indices
// 7. Copy base object footer (ANIM sections)
// 8. Add ATTR attributes and copy addition object footer with adjusted TRIS indices
```

#### Step 4: Index Adjustment
- **Vertex indices**: Add base object VT count to all addition object vertex references
- **Triangle indices**: Add base object TRIS count to all addition object TRIS references

### 5. Essential Error Handling
```cpp
// File existence and format validation:
if (!file_exists(base_file)) {
    std::cerr << "Error: Base file not found\n";
    return 1;
}

if (!file_exists(addition_file)) {
    std::cerr << "Error: Addition file not found\n";
    return 1;
}

if (!is_obj_file(base_file)) {
    std::cerr << "Error: '" << base_file << "' is not a valid .obj filename\n";
    return 1;
}

if (!is_obj_file(addition_file)) {
    std::cerr << "Error: '" << addition_file << "' is not a valid .obj filename\n";
    return 1;
}

// Basic OBJ8 format validation
auto base_lines = read_file(base_file);
if (!validate_obj_format(base_lines)) {
    std::cerr << "Error: Invalid OBJ8 format in base file\n";
    return 1;
}
```

### 6. String Processing
- Use `std::istringstream` for tokenizing lines
- Simple string operations for line parsing
- No regex - just basic string manipulation
- Preserve indentation for TRIS lines using `string.find()` and `string.substr()`

### 7. Backup and X-Plane Attributes
```cpp
// Create backup of original file before overwriting
bool create_backup(const std::string& filename) {
    auto backup_name = generate_backup_filename(filename);
    auto lines = read_file(filename);
    write_file(backup_name, lines);
    return true;
}

// Generate backup filename (e.g., "model.obj" -> "model.obj.bak")
std::string generate_backup_filename(const std::string& filename) {
    return filename + ".bak";
}

// Add required X-Plane attributes for gizmo ANIM section
std::vector<std::string> add_gizmo_attributes(const std::vector<std::string>& lines) {
    auto result = lines;
    result.push_back("\tATTR_draw_enable");
    result.push_back("\tATTR_cockpit");
    return result;
}
```

### 8. Memory Efficiency
- Process files line by line where possible
- Use `std::vector<std::string>` for line storage
- Minimal data copying

## Build System

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.10)
project(kitbash VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

# Library target
add_library(kitbash_lib STATIC kitbash.cpp)
target_include_directories(kitbash_lib PUBLIC .)

# CLI executable
add_executable(kitbash main.cpp)
target_link_libraries(kitbash kitbash_lib)

# Compiler options
if(MSVC)
    target_compile_options(kitbash_lib PRIVATE /W4)
    target_compile_options(kitbash PRIVATE /W4)
else()
    target_compile_options(kitbash_lib PRIVATE -Wall -Wextra -O3)
    target_compile_options(kitbash PRIVATE -Wall -Wextra -O3)
endif()

# Installation
install(TARGETS kitbash_lib kitbash
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)
install(FILES kitbash.h DESTINATION include)
```

## Design Principles

### YAGNI Implementation
- ❌ Complex class hierarchies
- ❌ Extensive validation beyond basic file checks
- ❌ Advanced error recovery mechanisms
- ❌ Configuration files or settings
- ❌ Logging frameworks
- ❌ Threading or async operations
- ❌ Plugin or extension systems

### Core Features
- ✅ Essential OBJ8 parsing for kitbash operations
- ✅ Accurate vertex/triangle index adjustment
- ✅ Reliable file I/O operations
- ✅ Professional error handling with helpful suggestions
- ✅ Automatic backup creation before overwriting
- ✅ User confirmation for file overwrites
- ✅ Enhanced summary statistics with percentages and formatting
- ✅ X-Plane ATTR attributes (draw_enable, cockpit)
- ✅ TRIS line indentation preservation
- ✅ Clean output files (no kitbash traces)
- ✅ Both library and CLI interfaces

## Performance Optimizations

### 1. Minimal Parsing
- Only parse lines that need modification (IDX, TRIS)
- Pass through other lines unchanged
- No full object model construction

### 2. In-Place Processing
- Modify lines during iteration
- Avoid multiple passes through data
- Stream processing where possible

### 3. Efficient String Operations
- Reserve vector capacity based on input size
- Use string_view for read-only operations
- Minimize string copying

## Implementation Estimate

### Lines of Code
- **kitbash.h**: ~80 lines (expanded MergeStats with percentage calculations)
- **kitbash.cpp**: ~420 lines (added backup, attributes, enhanced stats)
- **main.cpp**: ~180 lines (help/version, confirmation dialog, improved formatting)
- **CMakeLists.txt**: ~25 lines
- **Total**: ~705 lines

## Usage Examples

### CLI Usage
```bash
# Help and version information
./kitbash --help
./kitbash -h
./kitbash --version
./kitbash -v

# Basic usage (overwrites base file, creates backup)
./kitbash aircraft.obj landing_gear.obj

# With detailed summary statistics (creates backup)
./kitbash -s base_model.obj addon_parts.obj

# Output to different file (no backup needed)
./kitbash -o merged.obj base_model.obj addon_parts.obj

# Combined options (no backup needed)
./kitbash -s -o result.obj base_model.obj addon_parts.obj
```

#### Example Help Output (--help)
```
KITBASH - X-Plane OBJ8 File Merger
==================================

USAGE:
  kitbash base.obj addition.obj [OPTIONS]

DESCRIPTION:
  Merges X-Plane OBJ8 files by combining vertices, indices, and animations
  from the addition file into the base file. Automatically adjusts vertex
  and triangle indices to maintain proper references.

ARGUMENTS:
  base.obj      Base OBJ8 file (will be modified unless -o is used)
  addition.obj  Addition OBJ8 file to merge into base

OPTIONS:
  -s            Show detailed summary statistics
  -o FILE       Output to specified file (preserves original base file)
  -h, --help    Show this help message
  -v, --version Show version information

EXAMPLES:
  kitbash aircraft.obj landing_gear.obj
  kitbash -s -o merged.obj base.obj addon.obj

SAFETY:
  - Automatic backup created when overwriting original files
  - User confirmation required before any modifications
  - Clean output files with no kitbash traces

For more information, visit: https://github.com/user/kitbash
```

#### Example Version Output (--version)
```
kitbash 1.0.0
X-Plane OBJ8 File Merger
Copyright (c) 2025
```

#### Example Summary Output (with -s flag)
```
Creating backup: base_model.obj.bak

KITBASH MERGE SUMMARY
=====================

Input Files:
  Base:     base_model.obj (2,847 lines, 1,200 vertices, 800 triangles)
  Addition: addon_parts.obj (156 lines, 45 vertices, 12 triangles)

Changes:
  + Added 156 lines (+5.5%)
  + Added 45 vertices (+3.8%)
  + Added 12 triangles (+1.5%)

Result:
  Output:   base_model.obj (3,015 lines, 1,245 vertices, 812 triangles)
  Backup:   base_model.obj.bak

Completed successfully in 0.023 seconds.
```

### Library Usage
```cpp
// Simple merge (overwrites base file)
if (!kitbash::merge("base_model.obj", "addon_parts.obj")) {
    // Handle error
}

// Merge to new file
if (!kitbash::merge_to_file("base_model.obj", "addon_parts.obj", "output.obj")) {
    // Handle error
}

// C API usage
int result = kitbash_merge_to_file("base_model.obj", "addon_parts.obj", "output.obj");
if (result != 0) {
    printf("Error: %s\n", kitbash_get_last_error());
}
```

## Error Scenarios Handled

1. **Invalid number of arguments**: Display usage syntax and exit
2. **Invalid file extension**: Check for .obj extension, error if missing
3. **File not found**: Clear error message for missing base or addition files
4. **Invalid OBJ8 format**: Basic validation of file header and structure
5. **Backup creation failure**: Error if backup cannot be created before overwrite
6. **Write permission**: Check file permissions before attempting write
7. **User cancellation**: Respect user choice to abort overwrite operation
8. **Invalid switch**: Error message for unrecognized command line options
9. **Help/version requests**: Display appropriate information and exit cleanly

### Enhanced Error Messages
```
KITBASH ERROR
=============

File Validation:
  ✗ Invalid file extension: 'filename.txt'
    Expected: .obj file (e.g., model.obj)

File Access:
  ✗ Base file not found: 'base_model.obj'
    Check: File exists and path is correct

  ✗ Addition file not found: 'addon_parts.obj'
    Check: File exists and path is correct

Backup Operations:
  ✗ Failed to create backup: 'base_model.obj.bak'
    Check: Write permissions and disk space

User Actions:
  ⚠ Operation cancelled by user
    Note: No files were modified (probably very wise!)

Usage:
  kitbash base.obj addition.obj [-s] [-o output.obj]
```

## What This Plan Achieves

### Simplicity
- Single file implementation
- Minimal dependencies (standard library only)
- Straightforward build process
- Easy to understand and maintain

### Robustness
- Handles all OBJ8 features used by kitbash.lua
- Proper index adjustment for vertices and triangles
- File safety with user confirmation
- Basic error handling for common issues

### Performance
- Minimal memory allocation
- Efficient string processing
- No unnecessary object construction
- Fast compilation and execution

### YAGNI Compliance
- No features beyond core kitbashing
- No extensibility hooks
- No configuration complexity
- No over-abstraction

This plan delivers a production-ready kitbash tool that **fully replicates** the functionality of the original Lua script while adding a library interface for C++ integration. The implementation includes all original features plus safety enhancements:

- **Complete CLI compatibility**: User confirmation, detailed summary statistics, error handling
- **Enhanced safety**: Automatic backup creation before overwriting original files
- **X-Plane compatibility**: ATTR_draw_enable and ATTR_cockpit attributes for proper rendering
- **Clean output files**: No kitbash traces or comments in the merged OBJ file
- **Timing and statistics**: Processing time reporting in CLI summary output
- **Library interface**: Additional C++ API for programmatic usage

The implementation prioritizes simplicity, performance, and ease of integration while maintaining **100% functional parity** with the original kitbash.lua behavior and full compatibility with the OBJ8 specification. The output files are clean and indistinguishable from manually merged OBJ files, while automatic backups provide additional safety beyond the original Lua implementation.