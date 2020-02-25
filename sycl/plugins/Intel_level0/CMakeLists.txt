# PI Level0 plugin library

set(L0 "${CMAKE_SYCL_USE_LEVEL0}")
if (NOT DEFINED L0_INCLUDE_DIR)
  if (UNIX)
    set (L0_INCLUDE_DIR "${L0}/usr/include")
  else()
    set (L0_INCLUDE_DIR "${L0}/include")
  endif()
endif()
if (NOT DEFINED L0_LIBRARY)
  if (UNIX)
    set (L0_LIBRARY "${L0}/usr/lib/liblevel_zero.so")
  else()
    set (L0_LIBRARY "${L0}/level_zero64.lib")
  endif()
endif()

include_directories("${L0_INCLUDE_DIR}")
include_directories("${sycl_inc_dir}")
include_directories(${OPENCL_INCLUDE})

add_library(pi_level0 SHARED
  "${sycl_inc_dir}/CL/sycl/detail/pi.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/pi_level0.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/pi_level0.hpp"
)

add_dependencies(sycl-toolchain pi_level0)

if (UNIX)
  target_link_libraries(pi_level0 PRIVATE "${L0_LIBRARY}")
  target_link_libraries(pi_level0 PRIVATE pthread)
else()
  # We cannot use UNC paths with forward slashes with MS linker,
  # so we have to copy the L0 library to the current binary directory.
  # Only static import library is copied which means that runtime is
  # not affected by this copy.
  # TODO: implement removal of this file to be sure that it doesn't
  # cause any side effects
  get_filename_component(L0_LIB_BASENAME "${L0_LIBRARY}" NAME)
  set(L0_LIB_FILE ${CMAKE_CURRENT_BINARY_DIR}/${L0_LIB_BASENAME})
  add_custom_target(l0-lib-file
    DEPENDS "${L0_LIBRARY}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
	  "${L0_LIBRARY}" ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Copying ${L0_LIBRARY} to ${CMAKE_CURRENT_BINARY_DIR}")

  add_dependencies(pi_level0 l0-lib-file)
  target_link_libraries(pi_level0 PRIVATE ${L0_LIB_FILE})
endif()

add_common_options(pi_level0)

install(TARGETS pi_level0
    LIBRARY DESTINATION "lib" COMPONENT pi_level0
    RUNTIME DESTINATION "bin" COMPONENT pi_level0)
