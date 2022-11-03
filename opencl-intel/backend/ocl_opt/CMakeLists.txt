#
# INTEL CONFIDENTIAL
#
# Copyright (C) 2021 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may
# not use, modify, copy, publish, distribute, disclose or transmit this software
# or the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express or
# implied warranties, other than those that are expressly stated in the License.
#
include_directories(
  ${LLVM_INCLUDE_DIRS} ${BACKEND_ROOT_DIR}/passes ${BACKEND_ROOT_DIR}/optimizer
  ${BACKEND_ROOT_DIR}/arch_headers)

# Backend passes
set(OCL_BE_PASSES BuiltinLibInfo DebugInfoPass LLVMBarrier LLVMVectorizer
                  SmartGVN StripIntelIP)

# Dependencies of the passes
set(OCL_BE_PASSES_DEPS LoopUtils OclBackendUtils reflection_module)

if(OPENCL_INTREE_BUILD)
  llvm_map_components_to_libnames(
    LLVM_MODULE_LIBS
    AllTargetsAsmParsers
    AllTargetsCodeGens
    AllTargetsDescs
    AllTargetsInfos
    AggressiveInstCombine
    Analysis
    AsmParser
    BitWriter
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
    Intel_DPCPPKernelTransforms
    Intel_LoopAnalysis
    Intel_LoopTransforms
    Intel_MapIntrinToIml
    Intel_OpenCLTransforms
    VPOAnalysis
    VPOTransforms)
else()
  # Remove unused LLVM libs for oclopt to avoid duplicate command line option
  # registration issue in shared link.
  set(UNUSED_LLVM_LIBS
      LLVMCoverage
      LLVMDebugInfoGSYM
      LLVMDebugInfoPDB
      LLVMDlltoolDriver
      LLVMDWARFLinker
      LLVMExecutionEngine
      LLVMFrontendOpenACC
      LLVMFuzzMutate
      LLVMIntelJITEvents
      LLVMInterpreter
      LLVMJITLink
      LLVMLibDriver
      LLVMLineEditor
      LLVMLTO
      LLVMMCA
      LLVMMCJIT
      LLVMMIRParser
      LLVMObjectYAML
      LLVMOption
      LLVMOrcError
      LLVMOrcJIT
      LLVMRuntimeDyld
      LLVMSymbolize
      LLVMTableGen
      LLVMWindowsManifest
      LLVMX86Disassembler
      LLVMXRay)
  list(REMOVE_ITEM LLVM_MODULE_LIBS ${UNUSED_LLVM_LIBS})
endif()

# INTEL_FEATURE_SW_DTRANS Don't know why the following 3 components are not in
# LLVM_MODULE_LIBS. Add them here as a workaround to resolve link issue.
is_intel_feature_enabled(p ${LLVM_INTELFEATURE_PREFIX}_SW_DTRANS)
if(p)
  set(INTEL_DTRANS_LIBS LLVMIntel_DTrans LLVMIntel_DTransOpt
                        LLVMIntel_DTransAnalysis)
else()
  set(INTEL_DTRANS_LIBS)
endif()
# end INTEL_FEATURE_SW_DTRANS

add_opencl_executable(
  oclopt
  AnalysisWrappers.cpp
  BreakpointPrinter.cpp
  GraphPrinters.cpp
  NewPMDriver.cpp
  PassPrinters.cpp
  PrintSCC.cpp
  ocl_opt.cpp
  LINK_LIBS
  ${LLVM_MODULE_LIBS}
  ${OCL_BE_PASSES}
  ${OCL_BE_PASSES_DEPS}
  cl_sys_utils
  # INTEL_FEATURE_SW_DTRANS
  ${INTEL_DTRANS_LIBS}
  # end INTEL_FEATURE_SW_DTRANS
)

set_target_properties(oclopt PROPERTIES FOLDER ${BACKEND_FOLDER_NAME})
