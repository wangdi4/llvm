# PI Level0 plugin library

set(L0 "${CMAKE_SYCL_USE_LEVEL0}")
include_directories("${L0}/usr/include")
link_libraries("${L0}/usr/lib/liblevel_zero.so")

include_directories("${sycl_inc_dir}")
include_directories(${OPENCL_INCLUDE})

add_library(pi_level0 SHARED
  "${sycl_inc_dir}/CL/sycl/detail/pi.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/pi_level0.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/pi_level0.hpp"
)

install(TARGETS pi_level0
    LIBRARY DESTINATION "lib" COMPONENT pi_level0)
