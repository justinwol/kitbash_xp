# Library target
add_library(kitbash_core STATIC
    kitbash.cpp
    kitbash.h
)

# Set include directories for the library
target_include_directories(kitbash_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# CLI executable
add_executable(kitbash main.cpp)
target_link_libraries(kitbash kitbash_core)

# Test executables (commented out until test files are created)
# add_executable(test_parsing test_parsing.cpp)
# target_link_libraries(test_parsing kitbash_core)

# add_executable(test_merge test_merge.cpp)
# target_link_libraries(test_merge kitbash_core)

# Compiler options
if(MSVC)
    target_compile_options(kitbash_core PRIVATE /W4)
    target_compile_options(kitbash PRIVATE /W4)
else()
    target_compile_options(kitbash_core PRIVATE -Wall -Wextra -O3)
    target_compile_options(kitbash PRIVATE -Wall -Wextra -O3)
endif()

# Installation
install(TARGETS kitbash_core kitbash
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)
install(FILES kitbash.h DESTINATION include)