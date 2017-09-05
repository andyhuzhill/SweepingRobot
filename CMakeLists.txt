cmake_minimum_required(VERSION 2.8)

set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)


# Add application code
add_subdirectory(src)

# Add Test
add_subdirectory(test)


