# Copyright (c) 2014-2015 Flowgrammable.org
# All rights reserved


project(flowctl CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(flowctl 
  main.cpp 
  util.cpp
  dataplane.cpp
  port.cpp)

target_link_libraries(flowctl freeflow)
