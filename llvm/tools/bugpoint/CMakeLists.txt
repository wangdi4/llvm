set(LLVM_LINK_COMPONENTS
  AllTargetsAsmParsers
  AllTargetsCodeGens
  AllTargetsDescs
  AllTargetsInfos
  Analysis
  BitWriter
  CodeGen
  Core
  IPO
  IRReader
  AggressiveInstCombine
  InstCombine
  Instrumentation
  Linker
  ObjCARCOpts
  ScalarOpts
  Support
  Target
  TransformUtils
  Vectorize
# INTEL_CUSTOMIZATION
  Intel_LoopAnalysis
  Intel_LoopTransforms
  Intel_MapIntrinToIml
  Intel_OptReport
# end INTEL_CUSTOMIZATION
# INTEL_COLLAB
  VPOAnalysis
  VPOTransforms
# end INTEL_COLLAB
  )

# Support plugins.
set(LLVM_NO_DEAD_STRIP 1)

add_llvm_tool(bugpoint
  BugDriver.cpp
  CrashDebugger.cpp
  ExecutionDriver.cpp
  ExtractFunction.cpp
  FindBugs.cpp
  Miscompilation.cpp
  OptimizerDriver.cpp
  ToolRunner.cpp
  bugpoint.cpp

  DEPENDS
  intrinsics_gen
  )
export_executable_symbols(bugpoint)

# INTEL_CUSTOMIZATION
if(INTEL_INCLUDE_DTRANS)
  target_link_libraries(bugpoint PRIVATE LLVMIntel_DTrans)
endif(INTEL_INCLUDE_DTRANS)
# INTEL_FEATURE_CSA
is_intel_feature_enabled(p ${LLVM_INTELFEATURE_PREFIX}_CSA)
if (p)
# Explicit dependency for LLVMIntel_CSAScalarOpt to satisfy
# ForcePassLinking dependencies on CSA passes.
  target_link_libraries(bugpoint PRIVATE LLVMIntel_CSAScalarOpt)
endif()
# end INTEL_FEATURE_CSA
# end INTEL_CUSTOMIZATION

if(WITH_POLLY AND LINK_POLLY_INTO_TOOLS)
  target_link_libraries(bugpoint PRIVATE Polly)
  # Ensure LLVMTarget can resolve dependences in Polly.
  target_link_libraries(bugpoint PRIVATE LLVMTarget)
endif(WITH_POLLY AND LINK_POLLY_INTO_TOOLS)
