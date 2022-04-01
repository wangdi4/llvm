# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Modifications, Copyright (C) 2021 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may not
# use, modify, copy, publish, distribute, disclose or transmit this software or
# the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the
# License.
#
# end INTEL_CUSTOMIZATION
#TODO:
#1. Figure out why CMP0057 has to be set. Should have been taken care of earlier in the build
#2. Use AddLLVM to modify the build and access config options
#cmake_policy(SET CMP0057 NEW)
#include(AddLLVM)

# Plugin for OpenCL
# Create Shared library for libpi_opencl.so.
#TODO: remove dependency on pi.h in sycl project.
#TODO: Currently, the pi.h header is common between sycl and plugin library sources.
#This can be changed by copying the pi.h file in the plugins project.

add_sycl_plugin(opencl
  SOURCES
    "${sycl_inc_dir}/CL/sycl/detail/pi.h"
    "pi_opencl.cpp"
  LIBRARIES
    OpenCL-ICD
)

<<<<<<< HEAD
  # This script file is used to allow exporting pi* symbols only.
  # All other symbols are regarded as local (hidden)
  set(linker_script "${CMAKE_CURRENT_SOURCE_DIR}/../ld-version-script.txt")

  # Filter symbols based on the scope defined in the script file,
  # and export pi* function symbols in the library.
  target_link_libraries( pi_opencl
    PRIVATE "-Wl,--version-script=${linker_script}"
  )
endif()

add_common_options(pi_opencl)

# INTEL_CUSTOMIZATION
dpcpptarget_add_resource_file(pi_opencl
                              "pi_opencl.dll"
                              "Intel(R) oneAPI DPC++ Library"
                              "Intel(R) oneAPI DPC++ Library")
#end INTEL_CUSTOMIZATION

install(TARGETS pi_opencl
  LIBRARY DESTINATION "lib${LLVM_LIBDIR_SUFFIX}" COMPONENT pi_opencl
  RUNTIME DESTINATION "bin" COMPONENT pi_opencl)
=======
set_target_properties(pi_opencl PROPERTIES LINKER_LANGUAGE CXX)
>>>>>>> c16412b5fb32814461b82b5c615f771f97e9d26d
