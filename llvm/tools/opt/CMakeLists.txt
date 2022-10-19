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
  AllTargetsCodeGens
  AllTargetsDescs
  AllTargetsInfos
  AggressiveInstCombine
  Analysis
  AsmParser
  BitWriter
  CFGuard
  CodeGen
  Core
  Coroutines
  Extensions
  IPO
  IRReader
  InstCombine
  Instrumentation
  MC
  ObjCARCOpts
  Remarks
  ScalarOpts
  Support
  SYCLLowerIR
  Target
  TransformUtils
  Vectorize
  Passes
# INTEL_CUSTOMIZATION
  Intel_DPCPPKernelTransforms
  Intel_LoopAnalysis
  Intel_LoopTransforms
  Intel_MapIntrinToIml
  Intel_OpenCLTransforms
# end INTEL_CUSTOMIZATION
# INTEL_COLLAB
  VPOAnalysis
  VPOTransforms
# end INTEL_COLLAB
  )

add_llvm_tool(opt
  AnalysisWrappers.cpp
  BreakpointPrinter.cpp
  NewPMDriver.cpp
<<<<<<< HEAD
# INTEL_CUSTOMIZATION
  Intel_PassPrinters.cpp
# end INTEL_CUSTOMIZATION
  PrintSCC.cpp
=======
>>>>>>> 743087fb6306dd03b1aaf2bdb9c877390649bc77
  opt.cpp

  DEPENDS
  intrinsics_gen
  SUPPORT_PLUGINS
  )
export_executable_symbols_for_plugins(opt)

if(LLVM_BUILD_EXAMPLES)
    target_link_libraries(opt PRIVATE ExampleIRTransforms)
endif(LLVM_BUILD_EXAMPLES)

# INTEL_CUSTOMIZATION
# INTEL_FEATURE_SW_DTRANS
is_intel_feature_enabled(p ${LLVM_INTELFEATURE_PREFIX}_SW_DTRANS)
if(p)
  target_link_libraries(opt PRIVATE LLVMIntel_DTrans)
endif()
# end INTEL_FEATURE_SW_DTRANS
# INTEL_FEATURE_CSA
is_intel_feature_enabled(p ${LLVM_INTELFEATURE_PREFIX}_CSA)
if (p)
# Explicit dependency for LLVMIntel_CSAScalarOpt to satisfy
# ForcePassLinking dependencies on CSA passes.
  target_link_libraries(opt PRIVATE LLVMIntel_CSAScalarOpt)
endif()
# end INTEL_FEATURE_CSA
# end INTEL_CUSTOMIZATION
