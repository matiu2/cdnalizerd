project(src)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/url_parser.hpp
    COMMAND ragel -C url_parser.hpp.rl -o url_parser.hpp
    MAIN_DEPENDENCY url_parser.hpp.rl
    DEPENDS url_actions.rl
    DEPENDS url.rl
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
add_custom_target(
    url_parser.hpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/url_parser.hpp
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/url.svg
    COMMAND ragel -Vp ${CMAKE_CURRENT_SOURCE_DIR}/url_parser.hpp.rl -o url_parser.dot
    COMMAND dot -Tsvg url_parser.dot -o ${CMAKE_CURRENT_BINARY_DIR}/url.svg
    MAIN_DEPENDENCY url_parser.hpp.rl
    DEPENDS url_actions.rl
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(
    url.svg
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/url.svg
)

add_executable(test_login test_login.cpp https.cpp)
target_link_libraries(test_login ${ALL_LIBS})
add_test(test_login test_login)

add_executable(cdnalizerd 
  main.cpp https.cpp utils.cpp inotify.cpp https.cpp AccountCache.cpp Job.cpp Worker.cpp logging.cpp url.cpp
  jobs/upload.cpp 
  jobs/delete.cpp 
  processes/mainProcess.cpp processes/login.cpp processes/list.cpp processes/syncAllDirectories.cpp
  config_reader/config_reader.cpp config_reader/config.cpp config_reader/config_writer.cpp)
add_dependencies(cdnalizerd url_parser.hpp)
target_link_libraries(cdnalizerd ${ALL_LIBS})

add_executable(cfsync cfsync.cpp) 
target_link_libraries(cfsync ${ALL_LIBS})

add_executable(testExcludeFilter config_reader/config.cpp testExcludeFilter.cpp logging.cpp)
target_link_libraries(testExcludeFilter ${ALL_LIBS})

add_test(NAME testExclude1 COMMAND testExcludeFilter "^f.n")
add_test(NAME testExclude2 COMMAND testExcludeFilter "^fun")
add_test(NAME testExclude3 COMMAND testExcludeFilter "a.*b.*c")
add_test(NAME testExclude4 COMMAND testExcludeFilter "abc")
add_test(NAME testExclude5 COMMAND testExcludeFilter "abc$")
add_test(NAME testExclude6 COMMAND testExcludeFilter "fun")
