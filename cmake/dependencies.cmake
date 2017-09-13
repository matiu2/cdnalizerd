
include(ExternalProject)

# C++
find_library(CPP c++)

# Lib sup c++
find_library(SUP_CPP supc++
             HINTS /usr/lib/gcc/x86_64-linux-gnu/5)

# dl
find_library(DL dl)

# z
find_library(Z z)

# c
find_library(C c)

## Threads
find_package(Threads REQUIRED)

## OpenSSL
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIRS})
link_directories(${OPENSSL_LIBRARIES})
message(STATUS "Using OpenSSL ${OPENSSL_VERSION}")

## Boost
## We need to compile our own boost, because we need clang's libc++, which is
## not compatible with the gcc standard library. If we link them together we
## get segfaults and other horrors

FIND_PACKAGE(Boost 1.64 REQUIRED system log log_setup coroutine context filesystem program_options date_time iostreams thread)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

## Boost hana (not yet available in 1.58) (header only library)
ExternalProject_Add(hana
    PREFIX 3rd_party
    GIT_REPOSITORY https://github.com/boostorg/hana.git
    GIT_TAG v1.2.0
    GIT_SHALLOW 1
    TLS_VERIFY true
    TLS_CAINFO certs/DigiCertHighAssuranceEVRootCA.crt
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    UPDATE_COMMAND "" # Skip annoying updates for every build
    INSTALL_COMMAND ""
)
SET(HANA_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/3rd_party/src/hana/include)
INCLUDE_DIRECTORIES(${HANA_INCLUDE_DIR})

## JSON - Nice json parser

ExternalProject_Add(json
    PREFIX 3rd_party
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_SHALLOW 1
    TLS_VERIFY true
    TLS_CAINFO certs/DigiCertHighAssuranceEVRootCA.crt
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    UPDATE_COMMAND "" # Skip annoying updates for every build
    INSTALL_COMMAND ""
)
SET(EXTRA_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/3rd_party/src/json/src)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/3rd_party/src/json/src)

## Beast - Allows us to talk to Rackspace cloud files et al.

ExternalProject_Add(Beast
    PREFIX 3rd_party
    GIT_REPOSITORY https://github.com/boostorg/beast.git
    TLS_VERIFY true
    GIT_SHALLOW 1
    TLS_CAINFO certs/DigiCertHighAssuranceEVRootCA.crt
    TEST_BEFORE_INSTALL 0
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    TEST_COMMAND ""   # Requires generation of rackspace credentials
    UPDATE_COMMAND "" # Skip annoying updates for every build
    INSTALL_COMMAND ""
)
SET(BEAST_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/3rd_party/src/Beast/include")
INCLUDE_DIRECTORIES(${BEAST_INCLUDE_DIR})

## Loguru - logging library

ExternalProject_Add(Loguru
    PREFIX 3rd_party
    GIT_REPOSITORY https://github.com/emilk/loguru.git
    TLS_VERIFY true
    GIT_SHALLOW 1
    TLS_CAINFO certs/DigiCertHighAssuranceEVRootCA.crt
    TEST_BEFORE_INSTALL 0
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    TEST_COMMAND ""   # Requires generation of rackspace credentials
    UPDATE_COMMAND "" # Skip annoying updates for every build
    INSTALL_COMMAND ""
)
SET(LOGURU_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/3rd_party/src/Loguru")
INCLUDE_DIRECTORIES(${LOGURU_INCLUDE_DIR})

## URL parser

ExternalProject_Add(URL
    PREFIX 3rd_party
    GIT_REPOSITORY https://github.com/corporateshark/LUrlParser.git
    TLS_VERIFY true
    GIT_SHALLOW 1
    TLS_CAINFO certs/DigiCertHighAssuranceEVRootCA.crt
    TEST_BEFORE_INSTALL 0
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    TEST_COMMAND ""   # Requires generation of rackspace credentials
    UPDATE_COMMAND "" # Skip annoying updates for every build
    INSTALL_COMMAND ""
)
SET(URL_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/3rd_party/src/URL")
INCLUDE_DIRECTORIES(${URL_INCLUDE_DIR})
