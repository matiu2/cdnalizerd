project(config_reader)

include_directories(${BANDIT_INCLUDE_DIR})

add_executable(test_config_reader test_config_reader.cpp config_reader.cpp config.cpp config_writer.cpp)
target_link_libraries(test_config_reader ${ALL_LIBS})
add_dependencies(test_config_reader bandit)
add_test(test_config_reader_all test_config_reader)
