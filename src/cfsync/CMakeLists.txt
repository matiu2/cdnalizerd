project(cfsync)

add_executable(cfsync main.cpp ../AccountCache.cpp commandLineOptions.cpp cfsync.cpp ../config/config.cpp)
target_link_libraries(cfsync 
    ${C}
    ${Boost_PROGRAM_OPTIONS_LIBRARY} 
    ${Boost_COROUTINE_LIBRARY}
    rackspace
)
