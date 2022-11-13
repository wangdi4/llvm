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
  Analysis
  BitReader
  BitWriter
  Core
  Coroutines
  Coverage
  Demangle
  Extensions
  FrontendHLSL
  FrontendOpenMP
  IPO
  IRReader
  AggressiveInstCombine
  InstCombine
  Instrumentation
  LTO
  Linker
  MC
  ObjCARCOpts
  Object
  Passes
  ProfileData
  Remarks
  ScalarOpts
  Support
  SYCLLowerIR
  Target
  TransformUtils
  )

# INTEL_CUSTOMIZATION
set(INTEL_SOURCE_FILES_TO_BUILD)
intel_add_file(INTEL_SOURCE_FILES_TO_BUILD
  COMPLEMENT LLVM_OPTIONAL_SOURCES
# Add custom Intel files below.  Files listed here will be included in
# INTEL_CUSTOMIZATION build.
  intel/CGBuiltin_getBuiltinIntelLibFunction.cpp
  intel/CGSPIRMetadataAdder.cpp
  intel/CGIntelHLS.cpp
  )

# INTEL_FEATURE_SW_DTRANS
intel_add_file(INTEL_SOURCE_FILES_TO_BUILD
  COMPLEMENT LLVM_OPTIONAL_SOURCES
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_SW_DTRANS
  intel/CGDTransInfo.cpp
  )
# end INTEL_FEATURE_SW_DTRANS
# end INTEL_CUSTOMIZATION

add_clang_library(clangCodeGen
  BackendUtil.cpp
  CGAtomic.cpp
  CGBlocks.cpp
  CGBuiltin.cpp
  CGCUDANV.cpp
  CGCUDARuntime.cpp
  CGCXX.cpp
  CGCXXABI.cpp
  CGCall.cpp
  CGClass.cpp
  CGCleanup.cpp
  CGCoroutine.cpp
  CGDebugInfo.cpp
  CGDecl.cpp
  CGDeclCXX.cpp
  CGException.cpp
  CGExpr.cpp
  CGExprAgg.cpp
  CGExprCXX.cpp
  CGExprComplex.cpp
  CGExprConstant.cpp
  CGExprScalar.cpp
  CGGPUBuiltin.cpp
  CGHLSLRuntime.cpp
  CGLoopInfo.cpp
  CGNonTrivialStruct.cpp
  CGObjC.cpp
  CGObjCGNU.cpp
  CGObjCMac.cpp
  CGObjCRuntime.cpp
  CGOpenCLRuntime.cpp
  CGOpenMPRuntime.cpp
  CGOpenMPRuntimeGPU.cpp
  CGRecordLayoutBuilder.cpp
  CGStmt.cpp
  CGStmtOpenMP.cpp
# INTEL_COLLAB
  intel/CGOpenMPLateOutline.cpp
# end INTEL_COLLAB
  CGSYCLRuntime.cpp
  CGVTT.cpp
  CGVTables.cpp
  CodeGenABITypes.cpp
  CodeGenAction.cpp
  CodeGenFunction.cpp
  CodeGenModule.cpp
  CodeGenPGO.cpp
  CodeGenTBAA.cpp
  CodeGenTypes.cpp
  ConstantInitBuilder.cpp
  CoverageMappingGen.cpp
  ItaniumCXXABI.cpp
  MacroPPCallbacks.cpp
  MicrosoftCXXABI.cpp
  ModuleBuilder.cpp
  ObjectFilePCHContainerOperations.cpp
  PatternInit.cpp
  SanitizerMetadata.cpp
  SwiftCallingConv.cpp
  TargetInfo.cpp
  VarBypassDetector.cpp
# INTEL_CUSTOMIZATION
  ${INTEL_SOURCE_FILES_TO_BUILD}
# end INTEL_CUSTOMIZATION

  DEPENDS
  ${codegen_deps}
  intrinsics_gen

  LINK_LIBS
  clangAnalysis
  clangAST
  clangBasic
  clangFrontend
  clangLex
  clangSema
  clangSerialization
  )
