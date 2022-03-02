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
set(LLVM_LINK_COMPONENTS BitWriter Core Object Support TransformUtils)

if(INTEL_CUSTOMIZATION)
list(APPEND LLVM_LINK_COMPONENTS ObjectYAML)
endif(INTEL_CUSTOMIZATION)
add_clang_tool(clang-offload-wrapper
  ClangOffloadWrapper.cpp

  DEPENDS
  intrinsics_gen
  CUSTOM_WIN_VER
  )

set(CLANG_OFFLOAD_WRAPPER_LIB_DEPS
  clangBasic
  )

add_dependencies(clang clang-offload-wrapper)

clang_target_link_libraries(clang-offload-wrapper
  PRIVATE
  ${CLANG_OFFLOAD_WRAPPER_LIB_DEPS}
  )

if(INTEL_CUSTOMIZATION)
  dpcpptarget_add_resource_file(clang-offload-wrapper
    "clang-offload-wrapper.exe"
    "Intel(R) oneAPI DPC++ Compiler"
    "Clang offload wrapper.")
endif()
