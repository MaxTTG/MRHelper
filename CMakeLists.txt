cmake_minimum_required(VERSION 3.10)
project(
    MRHelper
    LANGUAGES CXX
    VERSION 1.0.0
)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(MRHelper STATIC
    src/Scheduler.cpp
    src/SimpleTask.cpp
    src/Task.cpp
    src/ThreadPool.cpp
)

target_include_directories(MRHelper PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

include(GNUInstallDirs)
install(
    TARGETS MRHelper
    EXPORT MRHelperTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp"
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/MRHelperConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MRHelper
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/MRHelperConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/MRHelperConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/MRHelperConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MRHelper
)

install(
    EXPORT MRHelperTargets
    FILE MRHelperTargets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MRHelper
)

option(BUILD_TESTING "Build tests" ON)
if(${BUILD_TESTING})
    add_subdirectory(tests)
endif()

option(BUILD_EXAMPLE "Build example" ON)
if(${BUILD_EXAMPLE})
    add_subdirectory(examples)
endif()
