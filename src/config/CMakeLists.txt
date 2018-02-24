project(config_reader)

add_library(config STATIC
  config_reader.cpp config.cpp config_writer.cpp
)
target_link_libraries(config
  ${Boost_SYSTEM_LIBRARY}
  ${CMAKE_THREAD_LIBS_INIT}
  ${DL}
)
