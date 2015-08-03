# Copyright (c) 2014-2015 Flowgrammable.org
# All rights reserved

# A helper macro for adding example programs.
macro(add_example target)
  add_executable(${target} ${ARGN})
  target_link_libraries(${target} freeflow)
endmacro()


add_example(unix-echo-server unix-sockets/echo-server.cpp)

if(FREEFLOW_USE_PCAP)
  add_example(pcap-read capture/reader.cpp)
endif()

