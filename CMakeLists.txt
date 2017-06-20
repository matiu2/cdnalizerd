cmake_minimum_required(VERSION 2.8)

project (cdnalizerd)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -stdlib=libc++")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -fstandalone-debug -fwritable-strings -fno-builtin")

include_directories("/usr/include/libcxxabi")

# This is just for youcompleteme vim plugin
set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")

enable_testing()

include(cmake/dependencies.cmake)

# Sub diretories
add_subdirectory(src)
add_subdirectory(experiments)
