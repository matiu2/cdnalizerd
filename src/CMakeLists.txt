project(src)

add_subdirectory(config_reader)

include_directories(${CURL_INCLUDE_DIRS} ${CURLPP11_SOURCE_DIR} ${JSON_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR}  ${INOTIFY_SOURCE_DIR})

add_executable(test_login test_login.cpp ${CURLPP11_FILE})
add_dependencies(test_login jsonpp11)
add_dependencies(test_login bandit)
target_link_libraries(test_login ${OPENSSL_LIBRARIES} ${CURL_LIBRARIES})
set_property(
  TARGET test_login
  APPEND
  PROPERTY INCLUDE_DIRECTORIES ${BANDIT_INCLUDE_DIR})
add_test(test_login test_login)

add_executable(cdnalizerd main.cpp ${INOTIFY_SOURCE_FILE} ${CURLPP11_FILE})
target_link_libraries(cdnalizerd config_reader ${OPENSSL_LIBRARIES} ${CURL_LIBRARIES})
