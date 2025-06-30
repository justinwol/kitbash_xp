cmake_minimum_required(VERSION 3.10)

# KITBASH SDK - Easy integration for C++ projects
# This CMakeLists.txt allows easy integration of the KITBASH library

# Create imported library target for the pre-compiled static library
add_library(kitbash_core STATIC IMPORTED)

# Set the location of the pre-compiled library
set_target_properties(kitbash_core PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/kitbash_core.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}"
)

# Set C++ standard requirement
target_compile_features(kitbash_core INTERFACE cxx_std_17)

# Optional: Add the CLI executable as a custom target (for reference)
# The kitbash.exe is included but doesn't need to be built since it's pre-compiled

# Usage in parent CMakeLists.txt:
# add_subdirectory(kitbash_sdk)
# target_link_libraries(your_target kitbash_core)