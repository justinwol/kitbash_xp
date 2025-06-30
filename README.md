# KITBASH

A C++ command-line tool for merging X-Plane OBJ8 files.

## Overview

KITBASH merges two X-Plane OBJ8 files by combining vertices, indices, and animations from an addition file into a base file. It automatically adjusts vertex and triangle indices to maintain proper references.

## Features

- Merges X-Plane OBJ8 files safely with automatic backups
- Adjusts vertex and triangle indices automatically
- Provides detailed merge statistics
- Both library and command-line interfaces
- User confirmation before overwriting files

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

### Command Line
```bash
# Basic merge (creates backup of base file)
./kitbash base.obj addition.obj

# Show detailed statistics
./kitbash -s base.obj addition.obj

# Output to new file (preserves originals)
./kitbash -o merged.obj base.obj addition.obj

# Help and version
./kitbash --help
./kitbash --version
```

### Library Usage
```cpp
#include "kitbash.h"

// Simple merge
if (!kitbash::merge("base.obj", "addition.obj")) {
    // Handle error
}

// Merge to new file
kitbash::merge_to_file("base.obj", "addition.obj", "output.obj");
```

## Safety Features

- Automatic backup creation when overwriting files
- User confirmation required before modifications
- File validation and error checking
- Clean output with no kitbash traces

## License

GPL v3 - See LICENSE file for details.