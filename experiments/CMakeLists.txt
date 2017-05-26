project(experiments)

# Commented out becaues inotify-cxx.h that it imports generates compiler warnings
#add_executable(inotify inotify.cpp ${INOTIFY_SOURCE_FILE})
#set_target_properties(inotify PROPERTIES INCLUDE_DIRECTORIES ${INOTIFY_SOURCE_DIR})

add_executable(locale locale.cpp)

add_executable(utf8 utf8.cpp)

add_executable(inotify_own_wrapper inotify_own_wrapper.cpp)

add_executable(rax-login rax-login.cpp ${CURLPP11_FILE})

add_dependencies(rax-login curlpp11)
set_property(
  TARGET rax-login
  APPEND
  PROPERTY INCLUDE_DIRECTORIES ${JSON_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} ${CURL_INCLUDE_DIR} ${CURLPP11_SOURCE_DIR} ${JSONPP11_SOURCE_DIR})
target_link_libraries(rax-login ${OPENSSL_LIBRARIES} ${CURL_LIBRARIES} ${curlpp11_lib})
