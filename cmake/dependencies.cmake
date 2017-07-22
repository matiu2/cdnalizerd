
include(ExternalProject)

## Threads
find_package(Threads)

## Boost
## We need to compile our own boost, because we need clang's libc++, which is
## not compatible with the gcc standard library. If we link them together we
## get segfaults and other horrors

FIND_PACKAGE(Boost 1.58 REQUIRED system log log_setup coroutine filesystem)
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

## Bandit (for tests)
ExternalProject_Add(bandit
    PREFIX 3rd_party
    GIT_REPOSITORY https://github.com/joakimkarlsson/bandit.git
    TLS_VERIFY true
    TLS_CAINFO certs/DigiCertHighAssuranceEVRootCA.crt
    BUILD_COMMAND ""
    UPDATE_COMMAND "" # Skip annoying updates for every build
    INSTALL_COMMAND ""
)
SET(BANDIT_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/3rd_party/src/bandit)

## RESTClient2 - Allows us to talk to Rackspace cloud files et al.

ExternalProject_Add(RESTClient2
    PREFIX 3rd_party
    GIT_REPOSITORY git@github.com:matiu2/RESTClient2.git
    TLS_VERIFY true
    TLS_CAINFO certs/DigiCertHighAssuranceEVRootCA.crt
    TEST_BEFORE_INSTALL 0
    TEST_COMMAND ""   # Requires generation of rackspace credentials
    UPDATE_COMMAND "" # Skip annoying updates for every build
    INSTALL_COMMAND ""
)
SET(RESTClient2_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/3rd_party/src/RESTClient2/src)
find_library(RESTClient RESTClient HINTS ${CMAKE_CURRENT_BINARY_DIR}/3rd_party/src/RESTClient2-build/src)

ExternalProject_Add(jsonpp11
     PREFIX 3rd_party
     GIT_REPOSITORY https://github.com/matiu2/jsonpp11.git
     TLS_VERIFY true
     TLS_CAINFO certs/DigiCertHighAssuranceEVRootCA.crt
     TEST_BEFORE_INSTALL 0
     TEST_COMMAND ""   # Test command requires rackspace cloud authentication to work
     UPDATE_COMMAND "" # Skip annoying updates for every build
     INSTALL_COMMAND ""
 )
SET(JSONPP11_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/3rd_party/src/jsonpp11/src)
