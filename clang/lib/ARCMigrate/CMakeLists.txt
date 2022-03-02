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
  Support
  )

# INTEL_CUSTOMIZATION
if (MSVC)
  set_source_files_properties(TransAutoreleasePool.cpp PROPERTIES COMPILE_FLAGS /bigobj)
  set_source_files_properties(Transforms.cpp PROPERTIES COMPILE_FLAGS /bigobj)
  set_source_files_properties(ObjCMT.cpp PROPERTIES COMPILE_FLAGS /bigobj)
endif()
# end INTEL_CUSTOMIZATION
# By default MSVC has a 2^16 limit on the number of sections in an object
# file, and Transforms.cpp needs more than that.
if (MSVC)
  set_source_files_properties(Transforms.cpp PROPERTIES COMPILE_FLAGS /bigobj)
endif()

add_clang_library(clangARCMigrate
  ARCMT.cpp
  ARCMTActions.cpp
  FileRemapper.cpp
  ObjCMT.cpp
  PlistReporter.cpp
  TransAPIUses.cpp
  TransARCAssign.cpp
  TransAutoreleasePool.cpp
  TransBlockObjCVariable.cpp
  TransEmptyStatementsAndDealloc.cpp
  TransGCAttrs.cpp
  TransGCCalls.cpp
  TransProperties.cpp
  TransProtectedScope.cpp
  TransRetainReleaseDealloc.cpp
  TransUnbridgedCasts.cpp
  TransUnusedInitDelegate.cpp
  TransZeroOutPropsInDealloc.cpp
  TransformActions.cpp
  Transforms.cpp

  LINK_LIBS
  clangAST
  clangAnalysis
  clangBasic
  clangEdit
  clangFrontend
  clangLex
  clangRewrite
  clangSema
  clangSerialization

  DEPENDS
  omp_gen
  )
