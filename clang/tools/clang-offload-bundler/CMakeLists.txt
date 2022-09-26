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
  AllTargetsAsmParsers
  AllTargetsDescs
  AllTargetsInfos
  BitWriter
  Core
  IRReader
  Object
  Support
  )

add_clang_tool(clang-offload-bundler
  ClangOffloadBundler.cpp
  CUSTOM_WIN_VER

  DEPENDS
  intrinsics_gen
  )

set(CLANG_OFFLOAD_BUNDLER_LIB_DEPS
  clangBasic
  clangDriver
  )

add_dependencies(clang clang-offload-bundler)

# A standalone clang build uses a pre-installed llvm-objcopy
if (NOT CLANG_BUILT_STANDALONE)
  add_dependencies(clang-offload-bundler llvm-objcopy)
endif()

clang_target_link_libraries(clang-offload-bundler
  PRIVATE
  ${CLANG_OFFLOAD_BUNDLER_LIB_DEPS}
  )

if(INTEL_CUSTOMIZATION)
  dpcpptarget_add_resource_file(clang-offload-bundler
    "clang-offload-bundler.exe"
    "Intel(R) oneAPI DPC++ Compiler"
    "Clang offload bundler.")
endif()
