cmake_minimum_required(VERSION 2.8)

project (cdnalizerd)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

enable_testing()

# Dependencies

set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
FIND_PACKAGE(Boost REQUIRED COMPONENTS system regex filesystem thread)
FIND_PACKAGE(OpenSSL REQUIRED)
FIND_PACKAGE(Threads REQUIRED)

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

## cppnet-lib for https
ExternalProject_Add(cpp-netlib
    PREFIX 3rd_party
    GIT_REPOSITORY https://github.com/cpp-netlib/cpp-netlib.git
    GIT_TAG cpp-netlib-0.11.0-final
    TLS_VERIFY true
    TLS_CAINFO certs/DigiCertHighAssuranceEVRootCA.crt
    PATCH_COMMAND git submodule init && git submodule update
    CMAKE_ARGS -DBUILD_SHARED_LIBS=OFF                                                                                                                                          
               -DCMAKE_BUILD_TYPE=Release                                                                                                                                      
               -DCMAKE_INSTALL_PREFIX=/usr/local                                                                                                                                   
               -DCPP-NETLIB_BUILD_SHARED_LIBS=OFF                                                                                                                                          
               -DCPP-NETLIB_DISABLE_FEATURE_TEST=ON   
    TEST_BEFORE_INSTALL 1
    #TEST_COMMAND ctest
    TEST_COMMAND ""
    UPDATE_COMMAND "" # Skip annoying updates for every build
    INSTALL_COMMAND ""
)
SET(CPP_NETLIB_INCLUDE_DIRS
      ${CMAKE_CURRENT_BINARY_DIR}/3rd_party/src/cpp-netlib
)
SET(CPP_NETLIB_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/3rd_party/src/cpp-netlib-build)

find_library(CPP_NETLIB_URI cppnetlib-uri ${CPP_NETLIB_LIB_DIR}/libs/network/src)
find_library(CPP_NETLIB_CONNECTIONS cppnetlib-client-connections ${CPP_NETLIB_LIB_DIR}/libs/network/src)

set_property(SOURCE ${CPP_NETLIB_NETWORK_URI} PROPERTY OBJECT_DEPENDS ${Boost_SYSTEM_LIBRARY_DEBUG})

SET(CPP_NETLIB_LIBS 
    ${CPP_NETLIB_URI}
    ${CPP_NETLIB_CONNECTIONS}
    ${Boost_FILESYSTEM_LIBRARY_DEBUG}
    ${Boost_ASIO_LIBRARY_DEBUG}
    ${Boost_THREAD_LIBRARY_DEBUG}
    ${Boost_SYSTEM_LIBRARY_DEBUG}
    ${CMAKE_THREAD_LIBS_INIT}
    ${OPENSSL_CRYPTO_LIBRARY}
    ${OPENSSL_SSL_LIBRARY}
)

# Sub diretories
add_subdirectory(src)
add_subdirectory(experiments)
