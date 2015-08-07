
cmake_minimum_required(VERSION 2.8.12)
include(CheckCCompilerFlag)

project(freeflow C CXX)
enable_testing()

# Configuration options
option(FREEFLOW_USE_PCAP "Enable pcap files as virtual ports" FALSE)
option(FREEFLOW_USE_NETMAP "Enable netmap ports" FALSE)
option(FREEFLOW_USE_NADK "Enable NADK ports" FALSE)

# Flags for profiling (no-omit-frame-pointer)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-omit-frame-pointer")


# Configure the compiler.
#
# We effectively require a functioning C++11 implementation.
# For Clang, use the libc++ as the standard library.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.8")
    message(FATAL_ERROR "Requires GCC version 4.8 or greater")
  endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()


# Check if C99 or GNU99 compatible, default to GNU99
CHECK_C_COMPILER_FLAG(-std=c99 C99)
CHECK_C_COMPILER_FLAG(-std=gnu99 GNU99)
if (GNU99)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
elseif (C99)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
else()
  message(FATAL_ERROR "C Compiler must support GNU99 or C99")
endif()


# C/C++ Build flags
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pedantic -Wall")

# Configure contrib source folder.
set(FREEFLOW_CONTRIB_SOURCE_DIR "${CMAKE_SOURCE_DIR}/contrib")


# Configure pcap use.
if(FREEFLOW_USE_PCAP)
  find_library(PCAP_LIBRARIES pcap)
  add_definitions(-D_BSD_SOURCE)
endif()


# Configure netmap.
# By default, we're going to point to the Git submodule
# that is the contrib source directory.
#
set(NETMAP_SOURCE_DIR ${FREEFLOW_CONTRIB_SOURCE_DIR}/netmap_source)
if(FREEFLOW_USE_NETMAP)
  find_path(NETMAP_INCLUDE_DIR "net/netmap.h" PATHS ${NETMAP_SOURCE_DIR}/sys)
  if(NOT NETMAP_INCLUDE_DIR)
    message(FATAL_ERROR "Cannot find netmap headers")
  endif()
  add_definitions(-DNETMAP_WITH_LIBS)
  include_directories(${NETMAP_INCLUDE_DIR})
endif()


# Configure nadk use.
if(FREEFLOW_USE_NADK)
  find_library(LIB_NADK nadk)
  if(NOT LIB_NADK)
    message(FATAL_ERROR "Cannot find lib nadk")
  endif()
  find_library(LIB_NADK_KNI nadk_kni)
  if(NOT LIB_NADK_KNI)
    message(FATAL_ERROR "Cannot find lib nadk_kni")
  endif()
  
  find_package(Threads REQUIRED)

  find_path(NADK_INCLUDE_DIR "nadk/nadk.h")
  if(NOT NADK_INCLUDE_DIR)
    message(FATAL_ERROR "Cannot find NADK headers")
  endif()
  include_directories(${NADK_INCLUDE_DIR}/nadk)
  include_directories(${NADK_INCLUDE_DIR}/nadk/rts)
  include_directories(${NADK_INCLUDE_DIR}/nadk/eth)
  include_directories(${NADK_INCLUDE_DIR}/nadk/ncs)
  include_directories(${NADK_INCLUDE_DIR}/nadk/arch/arm)
  ## all those includes should be more intelligently discovered..?
  ## there is a choice for arch:   ../nadk/arch/arm vs. ../nadk/arch/powerpc
endif()


# Allow includes to find from headers from this dir.
include_directories(.)


# Core libraries.
add_subdirectory(freeflow)


# The freeflow components
add_subdirectory(flowpath)
add_subdirectory(flowmgr)
add_subdirectory(flowctl)

# Traffic generator
add_subdirectory(traffic-gen)
