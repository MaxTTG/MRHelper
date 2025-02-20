cmake_minimum_required(VERSION 3.10)
project(MRHelperExamples)

add_executable(MRHelperExamples
    example.cpp
)

target_link_libraries(MRHelperExamples MRHelper)

target_include_directories(MRHelperExamples PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_include_directories(MRHelperTests PUBLIC
    ${CMAKE_BINARY_DIR}/../src
)
