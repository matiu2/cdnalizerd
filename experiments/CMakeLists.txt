project(experiments)

include_directories(${INOTIFY_SOURCE_DIR})
add_executable(inotify inotify.cpp)
target_link_libraries(inotify inotify_lib)
