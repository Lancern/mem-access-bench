cmake_minimum_required(VERSION 3.12)
project(mem-access-bench)

set(CMAKE_CXX_STANDARD 17)

set(MAB_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
include_directories("${MAB_INCLUDE_DIR}")

option(MAB_ENABLE_TESTS "Whether to build test artifacts of mem-access-bench" ON)
if (MAB_ENABLE_TESTS)
    enable_testing()
endif ()

add_subdirectory(src)

if (MAB_ENABLE_TESTS)
    add_subdirectory(tests)
endif ()
