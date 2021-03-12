cmake_minimum_required(VERSION 3.12)
project(mem-access-bench)

set(CMAKE_CXX_STANDARD 17)

set(MAB_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
include_directories("${MAB_INCLUDE_DIR}")

add_subdirectory(src)
