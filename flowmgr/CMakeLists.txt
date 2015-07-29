# Copyright (c) 2014-2015 Flowgrammable.org
# All rights reserved


project(flowmgr CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# The flowmgr server
add_executable(flowmgr 
  main.cpp
  control.cpp
  switch.cpp
  utility.cpp
  request.cpp
  dataplane.cpp
  port.cpp
  device.cpp)
target_link_libraries(flowmgr flowpath-common freeflow)

# Testing
add_subdirectory(tests)
