set(LLVM_LINK_COMPONENTS
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
# INTEL_CUSTOMIZATION
  Intel_DPCPPKernelTransforms
  Intel_LoopAnalysis
  Intel_LoopTransforms
  Intel_MapIntrinToIml
  Intel_OpenCLTransforms
  Intel_OptReport
# end INTEL_CUSTOMIZATION
# INTEL_COLLAB
  VPOAnalysis
  VPOTransforms
# end INTEL_COLLAB
  )

add_llvm_tool(opt
  AnalysisWrappers.cpp
  BreakpointPrinter.cpp
  GraphPrinters.cpp
  NewPMDriver.cpp
  PassPrinters.cpp
  PrintSCC.cpp
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
if(INTEL_INCLUDE_DTRANS)
  target_link_libraries(opt PRIVATE LLVMIntel_DTrans)
endif(INTEL_INCLUDE_DTRANS)
# INTEL_FEATURE_CSA
is_intel_feature_enabled(p ${LLVM_INTELFEATURE_PREFIX}_CSA)
if (p)
# Explicit dependency for LLVMIntel_CSAScalarOpt to satisfy
# ForcePassLinking dependencies on CSA passes.
  target_link_libraries(opt PRIVATE LLVMIntel_CSAScalarOpt)
endif()
# end INTEL_FEATURE_CSA
# end INTEL_CUSTOMIZATION
