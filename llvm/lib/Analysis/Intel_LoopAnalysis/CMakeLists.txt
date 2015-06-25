add_llvm_library(LLVMIntel_LoopAnalysis
  Intel_LoopAnalysis.cpp
  RegionIdentification.cpp
  SCCFormation.cpp
  ScalarSymbaseAssignment.cpp
  HIRCreation.cpp
  HIRCleanup.cpp
  LoopFormation.cpp
  HIRParser.cpp
  DDAnalysis.cpp
  SymbaseAssignment.cpp
  DDTests.cpp
  )
add_dependencies(LLVMIntel_LoopAnalysis intrinsics_gen)
