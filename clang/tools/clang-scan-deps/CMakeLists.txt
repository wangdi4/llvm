# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Modifications, Copyright (C) 2023 Intel Corporation
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
  Support
# INTEL_CUSTOMIZATION
  TargetParser
  Option
# end INTEL_CUSTOMIZATION
  )

add_clang_tool(clang-scan-deps
  ClangScanDeps.cpp
  )

set(CLANG_SCAN_DEPS_LIB_DEPS
  clangAST
  clangBasic
  clangCodeGen
  clangDriver
  clangFrontend
  clangFrontendTool
  clangLex
  clangParse
  clangSema
  clangSerialization
  clangTooling
  clangDependencyScanning
  )

clang_target_link_libraries(clang-scan-deps
  PRIVATE
  ${CLANG_SCAN_DEPS_LIB_DEPS}
  )

