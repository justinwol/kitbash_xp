cmake_minimum_required(VERSION 3.10)
project(kitbash VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Library target
add_library(kitbash_lib STATIC kitbash.cpp)
target_include_directories(kitbash_lib PUBLIC .)

# CLI executable
add_executable(kitbash main.cpp)
target_link_libraries(kitbash kitbash_lib)

# Test executables (commented out until test files are created)
# add_executable(test_parsing test_parsing.cpp)
# target_link_libraries(test_parsing kitbash_lib)

# add_executable(test_merge test_merge.cpp)
# target_link_libraries(test_merge kitbash_lib)

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

# Optional: Add debug configuration
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(kitbash_lib PRIVATE DEBUG=1)
    target_compile_definitions(kitbash PRIVATE DEBUG=1)
endif()