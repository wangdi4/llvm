add_llvm_library(LLVMIntel_LoopAnalysis
  Intel_LoopAnalysis.cpp
  HIRRegionIdentification.cpp
  HIRSCCFormation.cpp
  HIRScalarSymbaseAssignment.cpp
  HIRCreation.cpp
  HIRCleanup.cpp
  HIRLoopFormation.cpp
  HIRParser.cpp
  HIRSymbaseAssignment.cpp
  HIRFramework.cpp
  HIRDDAnalysis.cpp
  DDTests.cpp
  HIRLocalityAnalysis.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Analysis
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Analysis/Intel_LoopAnalysis
  )

add_dependencies(LLVMIntel_LoopAnalysis intrinsics_gen)
