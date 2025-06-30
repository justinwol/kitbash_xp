# KITBASH CLI Tool

A command-line tool for merging X-Plane OBJ8 files.

## Overview

KITBASH CLI is a standalone executable that merges two X-Plane OBJ8 files by combining vertices, indices, and animations from an addition file into a base file. It automatically adjusts vertex and triangle indices to maintain proper references.

## Contents

- **`kitbash.exe`** - Windows executable (97 KB)
- **`LICENSE`** - GPL v3 license
- **`README.md`** - This documentation

## Usage

### Basic Commands

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

### Command Line Options

- **`-s`** - Show detailed merge statistics
- **`-o FILE`** - Output to specified file (preserves original base file)
- **`-h, --help`** - Show help message
- **`-v, --version`** - Show version information

### Examples

```bash
# Merge landing gear into aircraft model
kitbash.exe aircraft.obj landing_gear.obj

# Merge with statistics and custom output
kitbash.exe -s -o final_model.obj base_aircraft.obj cockpit_details.obj

# Get help
kitbash.exe --help
```

## Safety Features

- **Automatic Backup**: Creates `.bak` files when overwriting originals
- **User Confirmation**: Prompts before modifying files
- **File Validation**: Checks for valid OBJ8 format
- **Error Handling**: Clear error messages and suggestions

## Requirements

- Windows operating system
- Valid X-Plane OBJ8 files as input

## Installation

1. Download the `kitbash_cli` directory
2. Place `kitbash.exe` in your desired location
3. Optionally add to your system PATH for global access

## Error Handling

The tool provides detailed error messages for common issues:

- **File not found**: Check file paths and permissions
- **Invalid format**: Ensure files are valid X-Plane OBJ8 format
- **Permission errors**: Check write permissions for output directory
- **Backup failures**: Ensure sufficient disk space

## Output

When successful, the tool will:
- Create a backup of the original file (if overwriting)
- Merge the files with proper index adjustments
- Display completion message
- Show detailed statistics (if `-s` flag used)

## License

GPL v3 - See LICENSE file for details.

## Related

For C++ library integration, see the separate KITBASH SDK package.