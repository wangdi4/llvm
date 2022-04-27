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
set(LLVM_LINK_COMPONENTS
  Core
  Demangle
  IRReader
  Support
  )

add_llvm_tool(llvm-no-spir-kernel
  llvm-no-spir-kernel.cpp

  CUSTOM_WIN_VER
  DEPENDS
  intrinsics_gen
  )

if(INTEL_CUSTOMIZATION)
  dpcpptarget_add_resource_file(llvm-no-spir-kernel
    "llvm-no-spir-kernel.exe"
    "Intel(R) oneAPI DPC++ Compiler"
    "LLVM SPIR kernel checking tool.")
endif()
