project(config_reader)

add_executable(test_string_functions test_string_functions.cpp string_functions.cpp)
add_test(test_all_string_functions test_string_functions)

add_test(test_trim_already test_string_functions --only=1.1)
add_test(test_trim_space_and_tab test_string_functions --only=1.2)
add_test(test_trim_left test_string_functions --only=1.3)
add_test(test_trim_right test_string_functions --only=1.4)

add_test(test_dequote_empty test_string_functions --only=2.1)
add_test(test_dequote_plain test_string_functions --only=2.2)
add_test(test_dequote_double_quote test_string_functions --only=2.3)
