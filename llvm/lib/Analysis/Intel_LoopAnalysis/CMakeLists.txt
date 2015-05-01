add_llvm_library(LLVMIntel_LoopAnalysis
  Intel_LoopAnalysis.cpp
  RegionIdentification.cpp
  DDAnalysis.cpp
  SymbaseAssignment.cpp
  HIRCreation.cpp
  HIRCleanup.cpp
  LoopFormation.cpp
  HIRParser.cpp
  DDTests.cpp
  )
add_dependencies(LLVMIntel_LoopAnalysis intrinsics_gen)
