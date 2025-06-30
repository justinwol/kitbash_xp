# KITBASH SDK

A C++ library and command-line tool for merging X-Plane OBJ8 files.

## Overview

KITBASH merges two X-Plane OBJ8 files by combining vertices, indices, and animations from an addition file into a base file. It automatically adjusts vertex and triangle indices to maintain proper references.

## Features

- Merges X-Plane OBJ8 files safely with automatic backups
- Adjusts vertex and triangle indices automatically
- Provides detailed merge statistics
- Both C and C++ library interfaces plus command-line tool
- User confirmation before overwriting files

## SDK Contents

This SDK contains everything you need to integrate KITBASH into your C++ project:

- **`kitbash.h`** - Header file with API declarations
- **`kitbash_core.lib`** - Pre-compiled static library (Windows)
- **`CMakeLists.txt`** - CMake integration file
- **`LICENSE`** - GPL v3 license
- **`README.md`** - This documentation

## Quick Start

### 1. Copy SDK to Your Project

Copy the entire `kitbash_sdk` directory into your C++ project.

### 2. Add to CMakeLists.txt

```cmake
# Add kitbash subdirectory
add_subdirectory(kitbash_sdk)

# Link to your target
target_link_libraries(your_target kitbash_core)
```

### 3. Include Header

```cpp
#include "kitbash_sdk/kitbash.h"
```

### 4. Build and Run

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Integration Methods

### Method 1: CMake Integration (Recommended)

Add to your `CMakeLists.txt`:
```cmake
add_subdirectory(kitbash_sdk)
target_link_libraries(your_app kitbash_core)
```

### Method 2: Manual Linking

```bash
# Compile your application
g++ -std=c++17 -I./kitbash_sdk your_app.cpp ./kitbash_sdk/kitbash_core.lib -o your_app
```

### Method 3: Visual Studio

1. Add `kitbash_sdk` folder to your project
2. In Project Properties:
   - Add `kitbash_sdk` to Include Directories
   - Add `kitbash_sdk/kitbash_core.lib` to Additional Dependencies
3. Set C++ Standard to C++17 or later

## Usage

### C++ API

```cpp
#include "kitbash.h"

// Simple merge
if (!kitbash::merge("base.obj", "addition.obj")) {
    // Handle error
}

// Merge to new file
kitbash::merge_to_file("base.obj", "addition.obj", "output.obj");

// Get file statistics
auto stats = kitbash::get_stats("model.obj");
std::cout << "Vertices: " << stats.vt_count << std::endl;

// Advanced merge with statistics
MergeStats merge_stats;
if (kitbash::merge_with_stats("base.obj", "addition.obj", &merge_stats)) {
    std::cout << "Added " << merge_stats.added_vt_count << " vertices" << std::endl;
}
```

### C API

```c
#include "kitbash.h"

// Simple merge
int result = kitbash_merge("base.obj", "addition.obj");
if (result != 0) {
    printf("Error: %s\n", kitbash_get_last_error());
}

// Merge to new file
kitbash_merge_to_file("base.obj", "addition.obj", "output.obj");

// Get statistics
int vt_count, tris_count;
if (kitbash_get_stats("model.obj", &vt_count, &tris_count) == 0) {
    printf("Vertices: %d, Triangles: %d\n", vt_count, tris_count);
}
```

## API Reference

### C++ Namespace: `kitbash`

#### Functions

- `bool merge(const std::string& base, const std::string& addition)`
  - Merges addition file into base file (modifies base file)
  - Returns true on success

- `bool merge_to_file(const std::string& base, const std::string& addition, const std::string& output)`
  - Merges files and saves to output file (preserves originals)
  - Returns true on success

- `Stats get_stats(const std::string& obj_file)`
  - Returns file statistics (vertex count, triangle count, line count)

- `bool merge_with_stats(const std::string& base, const std::string& addition, MergeStats* stats = nullptr)`
  - Advanced merge with detailed statistics
  - Returns true on success

#### Data Structures

```cpp
struct Stats {
    int vt_count = 0;      // Vertex count
    int tris_count = 0;    // Triangle count
    int line_count = 0;    // Total lines
};

struct MergeStats {
    int original_vt_count = 0;
    int original_tris_count = 0;
    int added_vt_count = 0;
    int added_tris_count = 0;
    int final_vt_count = 0;
    int final_tris_count = 0;
    double processing_time = 0.0;
    std::string base_filename;
    std::string addition_filename;
    std::string output_filename;
    std::string backup_filename;
    
    // Helper methods
    double vt_increase_percent() const;
    double tris_increase_percent() const;
    double line_increase_percent() const;
};
```

### C API Functions

- `int kitbash_merge(const char* base_file, const char* addition_file)`
  - Returns 0 on success, -1 on error

- `int kitbash_merge_to_file(const char* base_file, const char* addition_file, const char* output_file)`
  - Returns 0 on success, -1 on error

- `int kitbash_get_stats(const char* obj_file, int* vt_count, int* tris_count)`
  - Returns 0 on success, -1 on error

- `const char* kitbash_get_last_error()`
  - Returns last error message

## Requirements

- C++17 or later
- CMake 3.10+ (for CMake builds)
- Standard C++ library with filesystem support

## Safety Features

- Automatic backup creation when overwriting files
- User confirmation required before modifications
- File validation and error checking
- Clean output with no kitbash traces

## License

GPL v3 - See LICENSE file for details.

## Integration Examples

### With Existing CMake Project

```cmake
# In your CMakeLists.txt
add_subdirectory(kitbash_sdk)
target_link_libraries(your_app kitbash_core)
```

### With Visual Studio

1. Add all `.cpp` and `.h` files to your project
2. Set C++ standard to C++17 or later
3. Include `kitbash.h` in your source files

### With Code::Blocks or Dev-C++

1. Add `kitbash.cpp` and `kitbash.h` to your project
2. Set compiler to C++17 mode
3. Include `kitbash.h` in your source files

## Troubleshooting

### Common Issues

1. **Compilation errors**: Ensure C++17 support is enabled
2. **Linking errors**: Make sure to link the kitbash library
3. **Runtime errors**: Check file paths and permissions
4. **Invalid OBJ format**: Ensure files are valid X-Plane OBJ8 format

### Error Handling

Always check return values and use error functions:

```cpp
// C++ API
if (!kitbash::merge("base.obj", "addition.obj")) {
    std::cerr << "Merge failed" << std::endl;
}

// C API
if (kitbash_merge("base.obj", "addition.obj") != 0) {
    printf("Error: %s\n", kitbash_get_last_error());
}