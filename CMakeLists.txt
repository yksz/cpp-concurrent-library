cmake_minimum_required(VERSION 2.8.7)
project(ccl CXX)

set(CMAKE_CXX_FLAGS "-Wall -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

option(build_tests "Build all of own tests." OFF)
option(build_examples "Build example programs." OFF)

### Library
include_directories(
    ${PROJECT_SOURCE_DIR}/src
)

### Install
file(GLOB header_files src/ccl/*.h)
install(DIRECTORY DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ccl)
install(FILES ${header_files} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ccl)

### Test
if(build_tests)
    enable_testing()
    add_subdirectory(vendor/googletest)
    add_subdirectory(test)
endif()

### Example
if(build_examples)
    add_subdirectory(example)
endif()
