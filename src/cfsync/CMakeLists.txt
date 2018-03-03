project(cfsync)

add_executable(cfsync main.cpp commandLineOptions.cpp)
target_link_libraries(cfsync 
    ${Boost_PROGRAM_OPTIONS_LIBRARY} 
    rackspace
)
