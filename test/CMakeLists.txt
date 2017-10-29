aux_source_directory(. TST_LIST)
add_definitions(-std=c++11)

find_package(GTest REQUIRED)
find_package(Threads)
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(../src/)

add_executable(solution_test ${TST_LIST})
target_link_libraries(solution_test ${GTEST_BOTH_LIBRARIES})
target_link_libraries(solution_test ${CMAKE_THREAD_LIBS_INIT})

enable_testing()
add_test(NAME TestSolution COMMAND solution_test)