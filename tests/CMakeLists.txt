cmake_minimum_required(VERSION 3.10)
project(MRHelperTests)


add_executable(MRHelperTests
    tests.cpp
)

target_link_libraries(MRHelperTests PUBLIC MRHelper)

target_include_directories(MRHelperTests PUBLIC
    ${CMAKE_BINARY_DIR}/../src/MapReduceTask.cpp
    ${CMAKE_BINARY_DIR}/../src/Scheduler.cpp
    ${CMAKE_BINARY_DIR}/../src/SimpleTask.cpp
    ${CMAKE_BINARY_DIR}/../src/ThreadPool.cpp
    ${CMAKE_BINARY_DIR}/../src/Task.cpp
)

add_test(
    NAME MRHelperTests
    COMMAND MRHelperTests
)
