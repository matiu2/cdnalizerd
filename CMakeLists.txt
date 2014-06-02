cmake_minimum_required(VERSION 2.8)

project (cdnalizerd)

# Dependencies

## libcurlpp-dev
ADD_SUBDIRECTORY(3rd_party/curlcpp)
INCLUDE_DIRECTORIES(${CURLCPP_SOURCE_DIR}/include)

# Source

add_subdirectory(src)


