set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
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
  Intel_VPOAnalysis
  Intel_VPOTransforms
  Intel_OptReport
# end INTEL_CUSTOMIZATION
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
# end INTEL_CUSTOMIZATION

if(WITH_POLLY AND LINK_POLLY_INTO_TOOLS)
  target_link_libraries(bugpoint PRIVATE Polly)
  # Ensure LLVMTarget can resolve dependences in Polly.
  target_link_libraries(bugpoint PRIVATE LLVMTarget)
endif(WITH_POLLY AND LINK_POLLY_INTO_TOOLS)
