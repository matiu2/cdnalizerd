project(src)

add_subdirectory(config_reader)

add_executable(test_login test_login.cpp)
add_dependencies(test_login jsonpp11)
add_dependencies(test_login bandit)
add_dependencies(test_login RESTClient2)
target_link_libraries(test_login ${ALL_LIBS})
set_property(
  TARGET test_login
  APPEND
  PROPERTY INCLUDE_DIRECTORIES ${BANDIT_INCLUDE_DIR})
add_test(test_login test_login)

add_executable(cdnalizerd 
  main.cpp utils.cpp inotify.cpp AccountCache.cpp Job.cpp Worker.cpp
  jobs/upload.cpp
  processes/mainProcess.cpp processes/login.cpp processes/list.cpp processes/syncAllDirectories.cpp
  config_reader/config_reader.cpp config_reader/config.cpp config_reader/config_writer.cpp)
add_dependencies(cdnalizerd RESTClient2)
target_link_libraries(cdnalizerd ${ALL_LIBS})
