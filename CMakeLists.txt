cmake_minimum_required(VERSION 2.8)

project (cdnalizerd)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

enable_testing()

# Dependencies

## libcurlpp-dev

include(ExternalProject)
ExternalProject_Add(curlcpp
    PREFIX 3rd_party
    GIT_REPOSITORY https://github.com/JosephP91/curlcpp.git
    TLS_VERIFY true
    TLS_CAINFO certs/DigiCertHighAssuranceEVRootCA.crt
    TEST_BEFORE_INSTALL 1
    TEST_COMMAND ""
    UPDATE_COMMAND "" # Skip annoying updates for every build
    INSTALL_COMMAND ""
)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/3rd_party/src/curlcpp/include)

## Bandit (for tests)
ExternalProject_Add(bandit
    PREFIX 3rd_party
    GIT_REPOSITORY https://github.com/joakimkarlsson/bandit.git
    TLS_VERIFY true
    TLS_CAINFO certs/DigiCertHighAssuranceEVRootCA.crt
    UPDATE_COMMAND "" # Skip annoying updates for every build
    INSTALL_COMMAND ""
)
SET(BANDIT_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/3rd_party/src/bandit)

ExternalProject_Add(inotify_cxx
    PREFIX 3rd_party
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
set(INOTIFY_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/3rd_party/src/inotify_cxx)
set(INOTIFY_SOURCE_FILE ${INOTIFY_SOURCE_DIR}/inotify-cxx.cpp)
add_library(inotify_lib STATIC ${INOTIFY_SOURCE_FILE})
add_dependencies(inotify_lib inotify_cxx)

## yajp - Yet another JSON Parser
ExternalProject_Add(yajp
    PREFIX 3rd_party
    GIT_REPOSITORY https://github.com/matiu2/yajp.git
    TLS_VERIFY true
    TLS_CAINFO certs/DigiCertHighAssuranceEVRootCA.crt
    TEST_BEFORE_INSTALL 1
    TEST_COMMAND ctest
    UPDATE_COMMAND "" # Skip annoying updates for every build
    INSTALL_COMMAND ""
)
SET(YAJP_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/3rd_party/yajp/src)

# Sub diretories
add_subdirectory(src)
add_subdirectory(experiments)
