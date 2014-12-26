project(config_reader)

include_directories(${BANDIT_INCLUDE_DIR})

add_executable(test_string_functions test_string_functions.cpp string_functions.cpp)
add_dependencies(test_string_functions bandit)
add_test(test_all_string_functions test_string_functions)

add_test(test_trim_already test_string_functions --only=1.1)
add_test(test_trim_space_and_tab test_string_functions --only=1.2)
add_test(test_trim_left test_string_functions --only=1.3)
add_test(test_trim_right test_string_functions --only=1.4)

add_test(test_dequote_empty test_string_functions --only=2.1)
add_test(test_dequote_plain test_string_functions --only=2.2)
add_test(test_dequote_double_escape test_string_functions --only=2.3)
add_test(test_dequote_de-escapes_x test_string_functions --only=2.4)
add_test(test_dequote_escapes_quote test_string_functions --only=2.5)

add_library(config_reader config_reader.cpp string_functions.cpp)

ADD_DEFINITIONS( -fno-access-control ) # Helps us access private methods in functions
add_executable(test_config_reader test_config_reader.cpp)
target_link_libraries(test_config_reader config_reader)
add_dependencies(test_config_reader bandit)
add_test(test_config_reader_all test_config_reader)
