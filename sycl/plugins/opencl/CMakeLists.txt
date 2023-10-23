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

# Get the OpenCL adapter sources so they can be shared with the OpenCL PI plugin
get_target_property(UR_OPENCL_ADAPTER_SOURCES ur_adapter_opencl SOURCES)

add_sycl_plugin(opencl
  SOURCES
    ${UR_OPENCL_ADAPTER_SOURCES}
    # Some code is shared with the UR adapter
    "../unified_runtime/pi2ur.hpp"
    "../unified_runtime/pi2ur.cpp"
    "${sycl_inc_dir}/sycl/detail/pi.h"
    "${sycl_inc_dir}/sycl/detail/pi.hpp"
    "pi_opencl.cpp"
    "pi_opencl.hpp"
  INCLUDE_DIRS
    ${sycl_inc_dir}
    ${CMAKE_CURRENT_SOURCE_DIR}/../unified_runtime # for Unified Runtime
    ${UNIFIED_RUNTIME_SOURCE_DIR}/source/ # for adapters/opencl
  LIBRARIES
    UnifiedRuntime-Headers
    UnifiedRuntimeCommon
    OpenCL-ICD
)

# INTEL_CUSTOMIZATION
dpcpptarget_add_resource_file(pi_opencl
                              "pi_opencl.dll"
                              "Intel(R) oneAPI DPC++ Library"
                              "Intel(R) oneAPI DPC++ Library")
#end INTEL_CUSTOMIZATION

set_target_properties(pi_opencl PROPERTIES LINKER_LANGUAGE CXX)
