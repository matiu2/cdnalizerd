cmake_minimum_required(VERSION 2.8)

project (cdnalizerd)

SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

# Dependencies

## libcurlpp-dev
ADD_SUBDIRECTORY(3rd_party/curlcpp)
INCLUDE_DIRECTORIES(${CURLCPP_SOURCE_DIR}/include)

## Bandit (for tests)
ADD_SUBDIRECTORY(3rd_party/bandit)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/bandit/include)

# Source
add_subdirectory(src)


