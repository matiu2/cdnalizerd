project(experiments)

include_directories(${INOTIFY_SOURCE_DIR})
add_executable(inotify inotify.cpp)
target_link_libraries(inotify inotify_lib)

add_executable(locale locale.cpp)

add_executable(utf8 utf8.cpp)
