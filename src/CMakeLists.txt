project(src)

add_subdirectory(config_reader)

add_executable(test_login test_login.cpp)
add_dependencies(test_login curlpp11)
add_dependencies(test_login jsonpp11)
add_dependencies(test_login bandit)
set_property(
  TARGET test_login
  APPEND
  PROPERTY INCLUDE_DIRECTORIES ${JSON_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} ${CURL_INCLUDE_DIR} ${CURLPP11_SOURCE_DIR} ${BANDIT_INCLUDE_DIR})
target_link_libraries(test_login ${OPENSSL_LIBRARIES} ${CURL_LIBRARIES} ${curlpp11_lib})
add_test(test_login test_login)
