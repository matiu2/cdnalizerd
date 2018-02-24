project(jobs)

add_library(jobs STATIC
  upload.cpp 
  delete.cpp 
)
target_link_libraries(jobs
    ${Boost_COROUTINE_LIBRARY} 
    ${Boost_CONTEXT_LIBRARY} 
    ${Boost_FILESYSTEM_LIBRARY} 
    ${CMAKE_THREAD_LIBS_INIT} 
    ${Boost_THREAD_LIBRARY} 
    ${Boost_SYSTEM_LIBRARY} 
    ${OPENSSL_LIBRARIES}
    ${Boost_IOSTREAMS_LIBRARY} 
    ${DL}
    ${CPP}
    ${Z}
    ${C}
)
