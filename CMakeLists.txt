cmake_minimum_required(VERSION 2.8)

project (cdnalizerd)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

enable_testing()

include(cmake/dependencies.cmake)

# Sub diretories
add_subdirectory(src)
add_subdirectory(experiments)
