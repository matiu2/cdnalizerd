project(src)

add_subdirectory(config_reader)

add_executable(test_login test_login.cpp )
add_dependencies(test_login jsonpp11)
add_dependencies(test_login bandit)
add_dependencies(test_login RESTClient)
target_link_libraries(test_login ${RESTClient} ${Boost_SYSTEM_LIBRARY})
set_property(
  TARGET test_login
  APPEND
  PROPERTY INCLUDE_DIRECTORIES ${BANDIT_INCLUDE_DIR})
add_test(test_login test_login)

add_executable(cdnalizerd main.cpp utils.cpp AccountCache.cpp globals.cpp Worker.cpp processes/mainProcess.cpp processes/uploader.cpp)
add_dependencies(cdnalizerd RESTClient2)
target_link_libraries(cdnalizerd config_reader ${RESTClient} ${Boost_SYSTEM_LIBRARY} ${Boost_COROUTINE_LIBRARY})
