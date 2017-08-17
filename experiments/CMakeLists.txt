project(experiments)

# Commented out becaues inotify-cxx.h that it imports generates compiler warnings
#add_executable(inotify inotify.cpp ${INOTIFY_SOURCE_FILE})
#set_target_properties(inotify PROPERTIES INCLUDE_DIRECTORIES ${INOTIFY_SOURCE_DIR})

add_executable(locale locale.cpp)

add_executable(utf8 utf8.cpp)

add_executable(inotify_own_wrapper inotify_own_wrapper.cpp)
target_link_libraries(inotify_own_wrapper ${ALL_LIBS})


#add_executable(rax-login rax-login.cpp)
