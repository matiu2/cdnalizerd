project(config_reader)

add_executable(test_string_functions test_string_functions.cpp string_functions.cpp)
add_test(test_trim_already test_string_functions --only=1)
add_test(test_trim_space_and_tab test_string_functions --only=2)
add_test(test_trim_left test_string_functions --only=3)
add_test(test_trim_right test_string_functions --only=4)
