cmake_minimum_required(VERSION 2.8)

project (cdnalizerd)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

enable_testing()

# Dependencies

FIND_PACKAGE(CURL REQUIRED)
FIND_PACKAGE(OpenSSL REQUIRED)

include(ExternalProject)

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
add_custom_command(
    OUTPUT ${INOTIFY_SOURCE_FILE}
    DEPENDS inotify_cxx
)
add_library(inotify_lib STATIC ${INOTIFY_SOURCE_FILE})

## json++ - JSON library
ExternalProject_Add(json11
    PREFIX 3rd_party
    GIT_REPOSITORY https://github.com/matiu2/json--11.git
    TLS_VERIFY true
    TLS_CAINFO certs/DigiCertHighAssuranceEVRootCA.crt
    TEST_BEFORE_INSTALL 1
    TEST_COMMAND ctest
    UPDATE_COMMAND "" # Skip annoying updates for every build
    INSTALL_COMMAND ""
)
SET(JSON_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/3rd_party/src/json11/src)

# Sub diretories
add_subdirectory(src)
add_subdirectory(experiments)
