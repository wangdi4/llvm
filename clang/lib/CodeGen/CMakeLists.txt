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
  FrontendOffloading
  HIPStdPar
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

# Workaround for MSVC ARM64 performance regression:
# https://developercommunity.visualstudio.com/t/Compiling-a-specific-code-for-ARM64-with/10444970
# Since /O1 and /O2 represent a set of optimizations,
# our goal is to disable the /Og flag while retaining the other optimizations from the /O1|/O2 set
if(MSVC AND NOT CMAKE_CXX_COMPILER_ID MATCHES Clang
    AND MSVC_VERSION VERSION_GREATER_EQUAL 1932
    AND CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64")

  string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)
  string(REGEX MATCHALL "/[Oo][12]" opt_flags "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${uppercase_CMAKE_BUILD_TYPE}}")
  if (opt_flags)
    if(opt_flags MATCHES "1$")
      set(opt_flags "/Od;/Os;/Oy;/Ob2;/GF;/Gy")
    elseif (opt_flags MATCHES "2$")
      set(opt_flags "/Od;/Oi;/Ot;/Oy;/Ob2;/GF;/Gy")
    endif()
    set_source_files_properties(CGBuiltin.cpp PROPERTIES COMPILE_OPTIONS "${opt_flags}")
  endif()
endif()

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
