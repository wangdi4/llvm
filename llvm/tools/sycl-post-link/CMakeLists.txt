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
if(INTEL_CUSTOMIZATION)
get_property(LLVMGenXIntrinsics_SOURCE_DIR GLOBAL PROPERTY LLVMGenXIntrinsics_SOURCE_PROP)
get_property(LLVMGenXIntrinsics_BINARY_DIR GLOBAL PROPERTY LLVMGenXIntrinsics_BINARY_PROP)

include_directories(
  ${LLVMGenXIntrinsics_SOURCE_DIR}/GenXIntrinsics/include
  ${LLVMGenXIntrinsics_BINARY_DIR}/GenXIntrinsics/include)
endif()

set(LLVM_LINK_COMPONENTS
  BitWriter
  Core
  IPO
  IRReader
  Support
# INTEL_CUSTOMIZATION
  Linker
  GenXIntrinsics
  VPOTransforms
# end INTEL_CUSTOMIZATION
  TransformUtils
  SYCLLowerIR
  InstCombine
  ScalarOpts
  Linker
  Passes
  Analysis
  )

get_property(LLVMGenXIntrinsics_SOURCE_DIR GLOBAL PROPERTY LLVMGenXIntrinsics_SOURCE_PROP)
get_property(LLVMGenXIntrinsics_BINARY_DIR GLOBAL PROPERTY LLVMGenXIntrinsics_BINARY_PROP)

include_directories(
  ${LLVMGenXIntrinsics_SOURCE_DIR}/GenXIntrinsics/include
  ${LLVMGenXIntrinsics_BINARY_DIR}/GenXIntrinsics/include)

# INTEL_CUSTOMIZATION
add_llvm_tool_in_bin(sycl-post-link
# end INTEL_CUSTOMIZATION
  sycl-post-link.cpp
  CompileTimePropertiesPass.cpp
  DeviceGlobals.cpp
  ModuleSplitter.cpp
  SpecConstants.cpp
  SYCLDeviceLibReqMask.cpp
  SYCLKernelParamOptInfo.cpp
  SYCLDeviceRequirements.cpp
  ADDITIONAL_HEADER_DIRS
  ${LLVMGenXIntrinsics_SOURCE_DIR}/GenXIntrinsics/include
  ${LLVMGenXIntrinsics_BINARY_DIR}/GenXIntrinsics/include

# INTEL_CUSTOMIZATION
  LINK_LIBS
  LLVMVPOTransforms
  LLVMGenXIntrinsics
  LLVMVPOTransforms
# end INTEL_CUSTOMIZATION

  DEPENDS
  intrinsics_gen
  LLVMGenXIntrinsics
# INTEL_CUSTOMIZATION
  LLVMVPOTransforms
# end INTEL_CUSTOMIZATION
  CUSTOM_WIN_VER
  )
if(INTEL_CUSTOMIZATION)
  dpcpptarget_add_resource_file(sycl-post-link
    "sycl-post-link.exe"
    "Intel(R) oneAPI DPC++ Compiler"
    "SYCL post-link device code processing tool")
endif()

target_link_libraries(sycl-post-link PRIVATE LLVMGenXIntrinsics)
