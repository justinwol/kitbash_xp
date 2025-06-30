# KITBASH

A professional C++ toolkit for merging X-Plane OBJ8 files, providing both standalone CLI tools and SDK for developers.

## Overview

KITBASH merges X-Plane OBJ8 files by combining vertices, indices, and animations from an addition file into a base file. It automatically adjusts vertex and triangle indices to maintain proper references, making it perfect for kitbashing workflows in X-Plane aircraft and scenery development.

## Project Structure

This repository contains multiple distribution formats:

- **[`kitbash_cli/`](kitbash_cli/)** - Standalone Windows executable (97 KB)
- **[`kitbash_sdk/`](kitbash_sdk/)** - C++ library for developers
- **[`src/`](src/)** - Source code for building from scratch
- **[`test_objects/`](test_objects/)** - Example OBJ8 files for testing

## Quick Start

### For End Users (CLI Tool)

1. Download the [`kitbash_cli`](kitbash_cli/) directory
2. Use `kitbash.exe` directly:

```bash
# Basic merge (creates backup of base file)
kitbash.exe base.obj addition.obj

# Show detailed statistics
kitbash.exe -s base.obj addition.obj

# Output to new file (preserves originals)
kitbash.exe -o merged.obj base.obj addition.obj

# Help and version
kitbash.exe --help
kitbash.exe --version
```

### For Developers (SDK)

1. Download the [`kitbash_sdk`](kitbash_sdk/) directory
2. Include in your C++ project:

```cpp
#include "kitbash.h"

// Simple merge
if (!kitbash::merge("base.obj", "addition.obj")) {
    // Handle error
}

// Advanced merge with statistics
MergeStats stats;
if (kitbash::merge_with_stats("base.obj", "addition.obj", &stats)) {
    std::cout << "Added " << stats.added_vt_count << " vertices\n";
    std::cout << "Processing time: " << stats.processing_time << "s\n";
}
```

## Features

### Core Functionality
- **OBJ8 File Merging**: Combines multiple X-Plane OBJ8 files into one
- **Automatic Index Adjustment**: Maintains proper vertex and triangle references
- **Statistics Tracking**: Detailed merge information and performance metrics
- **Multiple Output Options**: In-place modification or separate output file

### Safety & Reliability
- **Automatic Backups**: Creates `.bak` files when overwriting originals
- **User Confirmation**: Prompts before modifying files
- **File Validation**: Checks for valid X-Plane OBJ8 format
- **Error Handling**: Comprehensive error reporting with helpful suggestions
- **Clean Output**: No kitbash traces in merged files

### Developer Features
- **Dual API**: Both C-style and C++ wrapper functions
- **Library Integration**: Static library for embedding in applications
- **Detailed Statistics**: Processing time, vertex counts, percentage increases
- **Cross-Platform**: Windows builds included, source available for other platforms

## Building from Source

### Requirements
- **Platform**: Windows (primary), Linux/macOS (via source)
- **Compiler**: Visual Studio 2019+ or GCC with C++17 support
- **Build System**: CMake 3.10+

### Windows Build
```bash
# Using the included build script
build.bat

# Or manually with CMake
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```

### Cross-Platform Build
```bash
mkdir build
cd build
cmake ..
make
```

## Usage Examples

### Command Line Interface

```bash
# Merge landing gear into aircraft model
kitbash.exe aircraft.obj landing_gear.obj

# Merge with statistics and custom output
kitbash.exe -s -o final_model.obj base_aircraft.obj cockpit_details.obj

# Get detailed help
kitbash.exe --help
```

**Example Output with Statistics:**
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

### Library Integration

#### C++ API
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
    
    // Get file statistics
    auto stats = kitbash::get_stats("model.obj");
    std::cout << "Vertices: " << stats.vt_count << std::endl;
    std::cout << "Triangles: " << stats.tris_count << std::endl;
    
    return 0;
}
```

#### C API
```c
#include "kitbash.h"
#include <stdio.h>

int main() {
    // Using C-style functions
    int result = kitbash_merge("base_model.obj", "addition_model.obj");
    
    if (result != 0) {
        const char* error = kitbash_get_last_error();
        printf("Error: %s\n", error);
    }
    
    return result;
}
```

## File Format Support

- **Input**: X-Plane OBJ8 files (.obj)
- **Output**: X-Plane OBJ8 files (.obj)
- **Validation**: Automatic format checking
- **Compatibility**: X-Plane 11 and X-Plane 12

## Technical Details

### Algorithm
1. Parse both input files to extract vertex and triangle counts
2. Combine headers and update POINT_COUNTS with new totals
3. Copy all base object vertices (VT lines)
4. Copy all addition object vertices
5. Copy base object indices (IDX/IDX10) unchanged
6. Copy addition object indices with adjusted vertex references
7. Merge animation sections (ANIM/TRIS) with proper index adjustments
8. Add required X-Plane attributes for proper rendering

### Performance
- **Memory Efficient**: Processes files line-by-line
- **Fast Processing**: Optimized string operations
- **Minimal Dependencies**: Standard C++ library only
- **Small Footprint**: CLI executable is only 97 KB

## Distribution Packages

### CLI Tool (`kitbash_cli/`)
- **Target**: End users who need a simple merge tool
- **Contents**: Windows executable, documentation, license
- **Size**: 97 KB executable
- **Usage**: Drag-and-drop or command line

### SDK (`kitbash_sdk/`)
- **Target**: C++ developers integrating merge functionality
- **Contents**: Header file, static library, examples, documentation
- **Platform**: Windows (x64)
- **Integration**: Visual Studio projects, CMake builds

### Source Code (`src/`)
- **Target**: Developers building from source or porting to other platforms
- **Contents**: Complete C++ source, CMake configuration
- **Platforms**: Cross-platform (Windows, Linux, macOS)
- **Customization**: Full source access for modifications

## Error Handling

The toolkit provides comprehensive error handling for common scenarios:

- **File not found**: Clear error messages with path verification
- **Invalid format**: OBJ8 format validation with helpful suggestions
- **Permission errors**: Write permission checks with troubleshooting tips
- **Backup failures**: Disk space and permission validation
- **User cancellation**: Graceful handling of user abort operations

## Development Workflow

### Building and Packaging
```bash
# Build the project
build.bat

# Package for release (creates ZIP files in dist/)
package.bat 0.1.0
```

### Release Process
1. **Build**: Run `build.bat` to compile the project
2. **Package**: Run `package.bat [version]` to create distribution packages
3. **Test**: Extract and test the packages locally
4. **Tag**: Create git tag: `git tag -a v0.1.0 -m "Release version 0.1.0"`
5. **Push**: Push tag: `git push origin v0.1.0`
6. **Release**: Create GitHub release and upload ZIP files from `dist/` directory

### Project Structure for Development
```
kitbash_xp/
├── src/                    # Source code
├── kitbash_cli/           # CLI distribution template (tracked)
├── kitbash_sdk/           # SDK distribution template (tracked)
├── build/                 # Build outputs (ignored)
├── dist/                  # Release packages (ignored)
├── build.bat              # Build script
└── package.bat            # Packaging script
```

**Note**: The `kitbash_cli/` and `kitbash_sdk/` directories serve as distribution templates. The actual binaries (`.exe`, `.lib`) are ignored by git and copied from the build directory during packaging.

## Version History

- **v0.1.0** (In Development): Initial working release
  - Complete OBJ8 merge functionality
  - Automatic backup creation
  - Detailed statistics reporting
  - CLI tool and SDK distributions
  - Professional error handling

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

When using this software in your projects, you must comply with GPL v3 requirements, including:
- Making your source code available if you distribute your application
- Including the GPL license notice
- Indicating any changes made to the library

## Contributing

This project follows YAGNI (You Aren't Gonna Need It) principles, focusing on core functionality without over-engineering. Contributions should maintain this philosophy while improving reliability and performance.

## Support

For issues, questions, or feature requests, please refer to the project repository or contact the maintainers.