cmake_minimum_required(VERSION 2.8)

project (cdnalizerd)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -stdlib=libc++")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -fstandalone-debug -fwritable-strings -fno-builtin")

include_directories("/usr/include/libcxxabi")

include(cmake/dependencies.cmake)

message("Threading Library: ${CMAKE_THREAD_LIBS_INIT}")

add_definitions(-DBOOST_COROUTINE_NO_DEPRECATION_WARNING)
add_definitions(-DBOOST_COROUTINES_NO_DEPRECATION_WARNING)
#ADD_DEFINITIONS(-DBOOST_LOG_NO_THREADS)
## This is needed if linking statically
#ADD_DEFINITIONS(-DBOOST_ALL_DYN_LINK)

# This is just for youcompleteme vim plugin
set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")

enable_testing()


# Sub diretories
add_subdirectory(src)
add_subdirectory(experiments)
