
# The build places binary in ${CMAKE_CURRENT_BINARY_DIR}/flowpath/pipelines
# rather than their subdirectories. This makes them easier to find
# during testing and easier to install.


# A helper macro for adding pipeline targets.
macro(add_pipeline target)
  add_library(${target} SHARED ${ARGN})
  target_link_libraries(${target} flowpath-rt)
endmacro()


# Pipelines
add_pipeline(wire wire/wire.c)

