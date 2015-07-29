
include_directories(..)


macro(add_test_driver target)
  add_executable(${target} ${ARGN})
  target_link_libraries(${target} noproto-core dpmp)
endmacro()


# Build a simple UDP test driver.
add_test_driver(test-udp udp.c)


# Build a simple test driver for a wire.
#
# TODO: Make this a general purpose wire-driven
add_test_driver(test-wire test-wire.c)

# Test general Util data structures:
add_test_driver(test-util-ring test-util-ring.c)


# Build a simple NADK test driver.
if (NOPROTO_USE_NADK)
  add_test_driver(test-nadk test-nadk.c)
endif()

