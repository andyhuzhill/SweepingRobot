project(robot)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_COMPILER g++-6)
add_definitions(-std=c++11)
aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})

target_link_libraries(${PROJECT_NAME} PocoNet PocoFoundation PocoUtil)
