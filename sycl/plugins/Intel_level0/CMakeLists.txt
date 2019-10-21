# PI Level0 plugin library

set(L0 "${CMAKE_SYCL_USE_LEVEL0}")
if (NOT DEFINED L0_INCLUDE_DIR)
  set (L0_INCLUDE_DIR "${L0}/usr/include")
endif()
if (NOT DEFINED L0_LIBRARY)
  set (L0_LIBRARY "${L0}/usr/lib/liblevel_zero.so")
endif()

include_directories("${L0_INCLUDE_DIR}")
link_libraries("${L0_LIBRARY}")

include_directories("${sycl_inc_dir}")
include_directories(${OPENCL_INCLUDE})

add_library(pi_level0 SHARED
  "${sycl_inc_dir}/CL/sycl/detail/pi.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/pi_level0.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/pi_level0.hpp"
)

# Built in the path to L0 RT
set_target_properties(pi_level0 PROPERTIES
  INSTALL_RPATH_USE_LINK_PATH TRUE)

install(TARGETS pi_level0
  LIBRARY DESTINATION "lib" COMPONENT pi_level0)
