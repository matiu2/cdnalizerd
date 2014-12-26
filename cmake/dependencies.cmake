
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

## jsonpp11 - JSON wrapper

ExternalProject_Add(jsonpp11
    PREFIX 3rd_party
    GIT_REPOSITORY https://github.com/matiu2/jsonpp11.git
    TLS_VERIFY true
    TLS_CAINFO certs/DigiCertHighAssuranceEVRootCA.crt
    TEST_BEFORE_INSTALL 0
    TEST_COMMAND ctest
    UPDATE_COMMAND "" # Skip annoying updates for every build
    INSTALL_COMMAND ""
)


## curlpp11 - CURL wrapper

FIND_PACKAGE(CURL REQUIRED)
FIND_PACKAGE(OpenSSL REQUIRED)

ExternalProject_Add(curlpp11
    PREFIX 3rd_party
    GIT_REPOSITORY https://github.com/matiu2/curlpp11.git
    TLS_VERIFY true
    TLS_CAINFO certs/DigiCertHighAssuranceEVRootCA.crt
    TEST_BEFORE_INSTALL 0
    TEST_COMMAND ctest
    UPDATE_COMMAND "" # Skip annoying updates for every build
    INSTALL_COMMAND ""
)
SET(CURLPP11_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/3rd_party/src/curlpp11/src)
set(CURLPP11_FILE ${CURLPP11_SOURCE_DIR}/curlpp11.cpp)
