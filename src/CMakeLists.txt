project(src)

add_subdirectory(config_reader)

add_executable(test_login test_login.cpp )
add_dependencies(test_login jsonpp11)
add_dependencies(test_login bandit)
add_dependencies(test_login RESTClient2)
target_link_libraries(test_login ${RESTClient} ${Boost_SYSTEM_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LOG_LIBRARY})
set_property(
  TARGET test_login
  APPEND
  PROPERTY INCLUDE_DIRECTORIES ${BANDIT_INCLUDE_DIR})
add_test(test_login test_login)

add_executable(cdnalizerd main.cpp utils.cpp AccountCache.cpp Job.cpp Worker.cpp processes/mainProcess.cpp processes/uploader.cpp processes/login.cpp processes/list.cpp)
add_dependencies(cdnalizerd RESTClient2)
target_link_libraries(cdnalizerd config_reader ${CMAKE_THREAD_LIBS_INIT} ${RESTClient} ${Boost_COROUTINE_LIBRARY} ${Boost_LOG_LIBRARY} ${Boost_LOG_SETUP_LIBRARY} ${Boost_IOSTREAMS_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY})
