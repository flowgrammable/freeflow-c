# Copyright (c) 2014-2015 Flowgrammable.org
# All rights reserved


# Compile format.cc locally
set(contrib-src ${FREEFLOW_CONTRIB_SOURCE_DIR}/cppformat/format.cc)


if(FREEFLOW_USE_PCAP)
  set(pcap-src capture.cpp)
endif()


# The main freeflow library.
add_library(freeflow STATIC
  ${contrib-src}
  format.cpp
  system.cpp
  memory.cpp
  async.cpp
  poll.cpp
  socket.cpp
  ip.cpp
  unix.cpp
  json.cpp
  ${pcap-src})


if(FREEFLOW_USE_PCAP)
  target_link_libraries(freeflow ${PCAP_LIBRARIES})
endif()

add_subdirectory(test)
add_subdirectory(examples)
