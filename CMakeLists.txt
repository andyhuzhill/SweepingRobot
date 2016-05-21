project(robot)
cmake_minimum_required(VERSION 2.8)

aux_source_directory(. SRC_LIST)
add_definitions(-std=c++11 -Wall)

add_executable(${PROJECT_NAME} ${SRC_LIST})

find_package(CURL REQUIRED)
if(CURL_FOUND)
    target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARY})
else(CURL_FOUND)
    message("Can't found libcurl!")
endif(CURL_FOUND)

find_package(Threads REQUIRED)
if(THREADS_FOUND)
    target_link_libraries(${PROJECT_NAME} pthread)
else(THREADS_FOUND)
    message("Pthread Not Found!")
endif(THREADS_FOUND)
