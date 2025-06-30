# Kitbash SDK

A C++ library for merging X-Plane 12 OBJ8 3D model files. This SDK allows developers to programmatically combine multiple OBJ8 files into a single merged file, perfect for kitbashing workflows in X-Plane aircraft and scenery development.

## Features

- **OBJ8 File Merging**: Combine multiple X-Plane OBJ8 files into one
- **Statistics Tracking**: Get detailed information about vertex counts, triangle counts, and processing metrics
- **Dual API**: Both C-style and C++ wrapper functions available
- **Error Handling**: Comprehensive error reporting
- **Backup Creation**: Automatic backup of original files before modification

## Contents

```
kitbash_sdk/
├── kitbash.h           # Header file with complete API
├── kitbash_core.lib    # Windows static library
├── LICENSE             # GPL v3 license
└── README.md           # This file
```

## Requirements

- **Platform**: Windows (x64)
- **Compiler**: Visual Studio 2019+ or compatible C++ compiler
- **Standard**: C++11 or later

## Integration

### 1. Copy SDK to Your Project

Copy the entire `kitbash_sdk` directory into your project folder.

### 2. Configure Include Path

Add the SDK directory to your compiler's include path:

**Visual Studio:**
- Right-click project → Properties
- C/C++ → General → Additional Include Directories
- Add: `$(ProjectDir)kitbash_sdk`

**Command Line:**
```bash
g++ -I./kitbash_sdk your_source.cpp
```

### 3. Link the Library

**Visual Studio:**
- Right-click project → Properties
- Linker → General → Additional Library Directories
- Add: `$(ProjectDir)kitbash_sdk`
- Linker → Input → Additional Dependencies
- Add: `kitbash_core.lib`

**Command Line:**
```bash
g++ -L./kitbash_sdk -lkitbash_core your_source.cpp
```

### 4. Include the Header

```cpp
#include "kitbash.h"
```

## Usage Examples

### Basic C++ API

```cpp
#include "kitbash.h"
#include <iostream>

int main() {
    // Simple merge operation
    bool success = kitbash::merge("base_model.obj", "addition_model.obj");
    
    if (success) {
        std::cout << "Merge completed successfully!" << std::endl;
    } else {
        std::cout << "Merge failed" << std::endl;
    }
    
    return 0;
}
```

### Advanced Usage with Statistics

```cpp
#include "kitbash.h"
#include <iostream>

int main() {
    MergeStats stats;
    
    // Merge with detailed statistics
    bool success = kitbash::merge_with_stats(
        "base_model.obj", 
        "addition_model.obj", 
        &stats
    );
    
    if (success) {
        std::cout << "Original vertices: " << stats.original_vt_count << std::endl;
        std::cout << "Added vertices: " << stats.added_vt_count << std::endl;
        std::cout << "Final vertices: " << stats.final_vt_count << std::endl;
        std::cout << "Vertex increase: " << stats.vt_increase_percent() << "%" << std::endl;
        std::cout << "Processing time: " << stats.processing_time << "s" << std::endl;
    }
    
    return 0;
}
```

### Custom Output File

```cpp
#include "kitbash.h"

int main() {
    // Merge to a specific output file
    bool success = kitbash::merge_to_file(
        "base_model.obj",
        "addition_model.obj", 
        "merged_output.obj"
    );
    
    return success ? 0 : 1;
}
```

### C-Style API

```cpp
#include "kitbash.h"
#include <iostream>

int main() {
    // Using C-style functions
    int result = kitbash_merge("base_model.obj", "addition_model.obj");
    
    if (result != 0) {
        const char* error = kitbash_get_last_error();
        std::cout << "Error: " << error << std::endl;
    }
    
    return result;
}
```

### Getting File Statistics

```cpp
#include "kitbash.h"
#include <iostream>

int main() {
    // Get statistics about an OBJ8 file
    auto stats = kitbash::get_stats("model.obj");
    
    std::cout << "Vertices: " << stats.vt_count << std::endl;
    std::cout << "Triangles: " << stats.tris_count << std::endl;
    std::cout << "Lines: " << stats.line_count << std::endl;
    
    return 0;
}
```

## API Reference

### C++ Namespace Functions

#### Core Merge Functions
- `bool kitbash::merge(const std::string& base, const std::string& addition)`
- `bool kitbash::merge_to_file(const std::string& base, const std::string& addition, const std::string& output)`

#### Advanced Merge with Statistics
- `bool kitbash::merge_with_stats(const std::string& base, const std::string& addition, MergeStats* stats = nullptr)`
- `bool kitbash::merge_to_file_with_stats(const std::string& base, const std::string& addition, const std::string& output, MergeStats* stats = nullptr)`

#### File Operations
- `kitbash::Stats kitbash::get_stats(const std::string& obj_file)`
- `std::vector<std::string> kitbash::read_file(const std::string& filename)`
- `void kitbash::write_file(const std::string& filename, const std::vector<std::string>& lines)`
- `bool kitbash::create_backup(const std::string& filename)`

#### Utility Functions
- `bool kitbash::is_obj_file(const std::string& filename)`
- `bool kitbash::validate_obj_format(const std::vector<std::string>& lines)`

### C-Style Functions

- `int kitbash_merge(const char* base_file, const char* addition_file)`
- `int kitbash_merge_to_file(const char* base_file, const char* addition_file, const char* output_file)`
- `int kitbash_get_stats(const char* obj_file, int* vt_count, int* tris_count)`
- `const char* kitbash_get_last_error()`

### Data Structures

#### MergeStats
Contains detailed information about merge operations:
- Vertex counts (original, added, final)
- Triangle counts (original, added, final)
- Line counts and processing time
- File names and calculated percentages

#### kitbash::Stats
Simple statistics structure for C++ API:
- `int vt_count` - Vertex count
- `int tris_count` - Triangle count  
- `int line_count` - Total line count

## Error Handling

The library provides comprehensive error handling:

**C++ API**: Returns `bool` values indicating success/failure
**C API**: Returns integer error codes (0 = success, non-zero = error)

Use `kitbash_get_last_error()` to get detailed error messages.

## File Format Support

- **Input**: X-Plane 12 OBJ8 files (.obj)
- **Output**: X-Plane 12 OBJ8 files (.obj)
- **Automatic backup**: Creates `.bak` files before modifying originals

## License

This SDK is licensed under the GNU General Public License v3.0. See the `LICENSE` file for complete terms.

When using this SDK in your projects, you must comply with GPL v3 requirements, including:
- Making your source code available if you distribute your application
- Including the GPL license notice
- Indicating any changes made to the library

## Support

For issues, questions, or contributions, please refer to the main project repository.

## Version

SDK Version: 1.0.0
Compatible with: Kitbash Core Library