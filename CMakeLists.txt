cmake_minimum_required(VERSION 2.8)

project (cdnalizerd)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

enable_testing()

# Dependencies

## libcurlpp-dev

include(ExternalProject)
ExternalProject_Add(curlcpp
    PREFIX 3rd_party/curlcpp
    GIT_REPOSITORY https://github.com/JosephP91/curlcpp.git
    TLS_VERIFY true
    TLS_CAINFO certs/DigiCertHighAssuranceEVRootCA.crt
    TEST_BEFORE_INSTALL 1
    TEST_COMMAND ctest
    UPDATE_COMMAND "" # Skip annoying updates for every build
    INSTALL_COMMAND ""
)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/3rd_party/curlcpp/src/include)

## Bandit (for tests)
ExternalProject_Add(bandit
    PREFIX 3rd_party/bandit
    GIT_REPOSITORY https://github.com/joakimkarlsson/bandit.git
    TLS_VERIFY true
    TLS_CAINFO certs/DigiCertHighAssuranceEVRootCA.crt
    UPDATE_COMMAND "" # Skip annoying updates for every build
    INSTALL_COMMAND ""
)
SET(BANDIT_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/3rd_party/bandit/src/bandit)

ExternalProject_Add(inotify_cxx
    PREFIX 3rd_party/inotify_cxx
    #--Download step--------------
    URL http://inotify.aiken.cz/download/inotify-cxx/inotify-cxx-0.7.4.tar.gz
    URL_HASH SHA1=1f009dc92c29f1b12e14212dddf8d4696c63051a
    #--Configure step-------------
    CONFIGURE_COMMAND ""
    #--Build step-----------------
    BUILD_COMMAND ""
    #--Install step---------------
    UPDATE_COMMAND "" # Skip annoying updates for every build
    INSTALL_COMMAND ""
)
SET(INOTIFY_SRC ${CMAKE_CURRENT_BINARY_DIR}/3rd_party/inotify_cxx/src/inotify-cxx.cpp)

## yajp - Yet another JSON Parser
ExternalProject_Add(yajp
    PREFIX 3rd_party/yajp
    GIT_REPOSITORY https://github.com/matiu2/yajp.git
    TLS_VERIFY true
    TLS_CAINFO certs/DigiCertHighAssuranceEVRootCA.crt
    TEST_BEFORE_INSTALL 1
    TEST_COMMAND ctest
    UPDATE_COMMAND "" # Skip annoying updates for every build
    INSTALL_COMMAND ""
)
SET(YAJP_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/3rd_party/yajp/src)

# Source
add_subdirectory(src)
