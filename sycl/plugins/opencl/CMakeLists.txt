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
    # Some code is shared with the UR adapter
    "../unified_runtime/pi2ur.hpp"
    "../unified_runtime/pi2ur.cpp"
    "../unified_runtime/ur/ur.hpp"
    "../unified_runtime/ur/ur.cpp"
    "../unified_runtime/ur/adapters/opencl/adapter.cpp"
    "../unified_runtime/ur/adapters/opencl/adapter.hpp"
    "../unified_runtime/ur/adapters/opencl/common.cpp"
    "../unified_runtime/ur/adapters/opencl/common.hpp"
    "../unified_runtime/ur/adapters/opencl/context.cpp"
    "../unified_runtime/ur/adapters/opencl/context.hpp"
    "../unified_runtime/ur/adapters/opencl/device.cpp"
    "../unified_runtime/ur/adapters/opencl/device.hpp"
    "../unified_runtime/ur/adapters/opencl/enqueue.cpp"
    "../unified_runtime/ur/adapters/opencl/image.cpp"
    "../unified_runtime/ur/adapters/opencl/kernel.cpp"
    "../unified_runtime/ur/adapters/opencl/platform.cpp"
    "../unified_runtime/ur/adapters/opencl/platform.hpp"
    "../unified_runtime/ur/adapters/opencl/sampler.cpp"
    "../unified_runtime/ur/adapters/opencl/memory.cpp"
    "../unified_runtime/ur/adapters/opencl/usm.cpp"
    "../unified_runtime/ur/adapters/opencl/program.cpp"
    "../unified_runtime/ur/adapters/opencl/event.cpp"
    "../unified_runtime/ur/adapters/opencl/queue.cpp"
    "../unified_runtime/ur/adapters/opencl/command_buffer.hpp"
    "../unified_runtime/ur/adapters/opencl/command_buffer.cpp"
    "../unified_runtime/ur/adapters/opencl/usm_p2p.cpp"
    # ---
    "${sycl_inc_dir}/sycl/detail/pi.h"
    "pi_opencl.cpp"
  INCLUDE_DIRS
    ${sycl_inc_dir}
    ${CMAKE_CURRENT_SOURCE_DIR}/../unified_runtime
  LIBRARIES
    UnifiedRuntime-Headers
    OpenCL-ICD
)

# INTEL_CUSTOMIZATION
dpcpptarget_add_resource_file(pi_opencl
                              "pi_opencl.dll"
                              "Intel(R) oneAPI DPC++ Library"
                              "Intel(R) oneAPI DPC++ Library")
#end INTEL_CUSTOMIZATION

set_target_properties(pi_opencl PROPERTIES LINKER_LANGUAGE CXX)
