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
  AggressiveInstCombine
  Analysis
  BitReader
  BitWriter
  CodeGenTypes
  Core
  Coroutines
  Coverage
  Demangle
  Extensions
  FrontendHLSL
  FrontendOpenMP
  IPO
  IRPrinter
  IRReader
  InstCombine
  Instrumentation
  LTO
  Linker
  MC
  ObjCARCOpts
  Object
  Passes
  ProfileData
  ScalarOpts
  Support
  SYCLLowerIR
  Target
  TargetParser
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
  ABIInfo.cpp
  ABIInfoImpl.cpp
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
  Targets/AArch64.cpp
  Targets/AMDGPU.cpp
  Targets/ARC.cpp
  Targets/ARM.cpp
  Targets/AVR.cpp
  Targets/BPF.cpp
  Targets/CSKY.cpp
  Targets/Hexagon.cpp
  Targets/Lanai.cpp
  Targets/LoongArch.cpp
  Targets/M68k.cpp
  Targets/MSP430.cpp
  Targets/Mips.cpp
  Targets/NVPTX.cpp
  Targets/PNaCl.cpp
  Targets/PPC.cpp
  Targets/RISCV.cpp
  Targets/SPIR.cpp
  Targets/Sparc.cpp
  Targets/SystemZ.cpp
  Targets/TCE.cpp
  Targets/VE.cpp
  Targets/WebAssembly.cpp
  Targets/X86.cpp
  Targets/XCore.cpp
  VarBypassDetector.cpp
# INTEL_CUSTOMIZATION
  ${INTEL_SOURCE_FILES_TO_BUILD}
# end INTEL_CUSTOMIZATION

  DEPENDS
  ${codegen_deps}
  intrinsics_gen
  ClangDriverOptions

  LINK_LIBS
  clangAST
  clangAnalysis
  clangBasic
  clangFrontend
  clangLex
  clangSema
  clangSerialization
  )
