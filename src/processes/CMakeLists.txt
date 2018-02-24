project(processes)

add_library(processes STATIC
  mainProcess.cpp 
  login.cpp 
  list.cpp 
  syncAllDirectories.cpp
)
target_link_libraries(processes 
  jobs 
  ${Boost_DATE_TIME_LIBRARY}
)
