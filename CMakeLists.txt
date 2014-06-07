cmake_minimum_required(VERSION 2.8)

project (cdnalizerd)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

enable_testing()

# Dependencies

## libcurlpp-dev
ADD_SUBDIRECTORY(3rd_party/curlcpp)
INCLUDE_DIRECTORIES(${CURLCPP_SOURCE_DIR}/include)

## Bandit (for tests)
ADD_SUBDIRECTORY(3rd_party/bandit)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/bandit)

# Source
add_subdirectory(src)
