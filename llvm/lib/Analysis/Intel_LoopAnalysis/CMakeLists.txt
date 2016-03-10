add_llvm_library(LLVMIntel_LoopAnalysis
  Intel_LoopAnalysis.cpp
  RegionIdentification.cpp
  SCCFormation.cpp
  ScalarSymbaseAssignment.cpp
  HIRCreation.cpp
  HIRCleanup.cpp
  LoopFormation.cpp
  HIRParser.cpp
  SymbaseAssignment.cpp
  HIRFramework.cpp
  DDAnalysis.cpp
  DDTests.cpp
  HIRLocalityAnalysis.cpp
  ParVecAnalysis.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Analysis
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Analysis/Intel_LoopAnalysis
  )

add_dependencies(LLVMIntel_LoopAnalysis intrinsics_gen)
