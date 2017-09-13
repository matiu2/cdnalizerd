project(src)

add_executable(test_login test_login.cpp https.cpp)
target_link_libraries(test_login ${ALL_LIBS})
add_test(test_login test_login)

add_executable(cdnalizerd 
  main.cpp https.cpp utils.cpp inotify.cpp https.cpp AccountCache.cpp Job.cpp Worker.cpp logging.cpp 
  ${URL_INCLUDE_DIR}/LUrlParser.cpp
  jobs/upload.cpp 
  processes/mainProcess.cpp processes/login.cpp processes/list.cpp processes/syncAllDirectories.cpp
  config_reader/config_reader.cpp config_reader/config.cpp config_reader/config_writer.cpp)
target_link_libraries(cdnalizerd ${ALL_LIBS})
