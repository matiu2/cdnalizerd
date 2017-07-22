project(config_reader)

include_directories(${BANDIT_INCLUDE_DIR})

add_library(config_reader config_reader.cpp)
target_link_libraries(config_reader ${CPP} ${Boost_SYSTEM_LIBRARY} ${Boost_FILESYSTEM_LIBRARY})

add_executable(test_config_reader test_config_reader.cpp)
target_link_libraries(test_config_reader config_reader)
add_dependencies(test_config_reader bandit)
add_test(test_config_reader_all test_config_reader)
