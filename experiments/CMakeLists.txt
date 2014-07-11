project(experiments)

add_executable(inotify inotify.cpp)
target_link_libraries(inotify inotify_lib)
set_target_properties(inotify PROPERTIES INCLUDE_DIRECTORIES ${INOTIFY_SOURCE_DIR})

add_executable(locale locale.cpp)

add_executable(utf8 utf8.cpp)

add_executable(rax-login rax-login.cpp)
set_property(
  TARGET rax-login
  APPEND
  PROPERTY INCLUDE_DIRECTORIES ${JSON_INCLUDE_DIR} ${CURLCPP_INCLUDE_DIR})
target_link_libraries(rax-login ${CURL_CPP} ${CURL_LIBRARY})
