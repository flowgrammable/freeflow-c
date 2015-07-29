# Copyright (c) 2014-2015 Flowgrammable.org
# All rights reserved


# Configure the set of port definitions to include the core
# library. This depends on the configuration of the build.
set(port_src port_udp.c)
if(FREEFLOW_USE_NETMAP)
  list(APPEND port_src port_netmap.c)
  add_definitions(-DNP_USE_NETMAP)
elseif(FREEFLOW_USE_NADK)
  list(APPEND port_src port_nadk.c)
  add_definitions(-DNP_USE_NADK)
elseif(FREEFLOW_USE_PCAP)
  list(APPEND port_src port_pcap.c)
  add_definitions(-DNP_USE_PCAP)
endif()


# Common facilities shared by flowpath with other
# Freeflow libraries and tools.
add_library(flowpath-common STATIC 
  error.c
  proto.c)


# The flowpath core library
add_library(flowpath-rt SHARED 
  types.c
  util.c
  manage.c
  
  # Algorithms
  hash.c
  trie.c

  # Abstractions
  packet.c
  port.c
  ${port_src}
  flow.c

  # Processing
  dataplane.c
  pipeline.c
)

target_link_libraries(flowpath-rt flowpath-common ${CMAKE_DL_LIBS})


# NADK dependendcies
if(FREEFLOW_USE_NADK)
  target_link_libraries(flowpath-rt nadk nadk_kni)
  
  # Fix -- less ugly way of checking for pthread
  CHECK_C_COMPILER_FLAG(-pthread PTHREAD)
  if(PTHREAD)
    target_compile_options(flowpath-rt PUBLIC "-pthread")
    # Should this just be 'set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")' ?
  endif()
  if(CMAKE_THREAD_LIBS_INIT)
    target_link_libraries(flowpath-rt "${CMAKE_THREAD_LIBS_INIT}")
  endif()
endif()


# To fix compilation problem, relocation R_X86_64_32:
#  recompile with -fPIC
# See http://www.cmake.org/pipermail/cmake/2007-May/014350.html
set_target_properties(flowpath-rt PROPERTIES POSITION_INDEPENDENT_CODE TRUE)


# Noproto executables
add_executable(flowpath main.c)
target_link_libraries(flowpath flowpath-rt)


# Hand-coded pipeline modules.
# add_subdirectory(pipelines)


# Testing
# add_subdirectory(tests)

