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
add_llvm_component_library(LLVMLTO
  LTO.cpp
  LTOBackend.cpp
  LTOModule.cpp
  LTOCodeGenerator.cpp
  SummaryBasedOptimizations.cpp
  UpdateCompilerUsed.cpp
  ThinLTOCodeGenerator.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/LTO
  DEPENDS
  intrinsics_gen
  llvm_vcsrevision_h

  LINK_COMPONENTS
  AggressiveInstCombine
  Analysis
  BinaryFormat
  BitReader
  BitWriter
  CodeGen
  Core
  Extensions
  IPO
  InstCombine
  Instrumentation
  Linker
  MC
  ObjCARC
  Object
  Passes
  Remarks
  Scalar
  Support
  SYCLLowerIR
  Target
  TransformUtils
)

# INTEL_CUSTOMIZATION
# INTEL_FEATURE_SW_DTRANS
is_intel_feature_enabled(p ${LLVM_INTELFEATURE_PREFIX}_SW_DTRANS)
if (p)
  target_link_libraries(LLVMLTO PRIVATE LLVMIntel_DTrans
                                        LLVMIntel_DTransAnalysis)
endif()
# end INTEL_FEATURE_SW_DTRANS
# end INTEL_CUSTOMIZATION
