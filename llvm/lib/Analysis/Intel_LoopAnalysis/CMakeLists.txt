add_llvm_library(LLVMIntel_LoopAnalysis
  Intel_LoopAnalysis.cpp
  IR/BlobDDRef.cpp
  IR/CanonExpr.cpp
  IR/DDRef.cpp
  IR/HIRVerifier.cpp
  IR/HLDDNode.cpp
  IR/HLGoto.cpp
  IR/HLIf.cpp
  IR/HLInst.cpp
  IR/HLLabel.cpp
  IR/HLLoop.cpp
  IR/HLNode.cpp
  IR/HLRegion.cpp
  IR/HLSwitch.cpp
  IR/IRRegion.cpp
  IR/RegDDRef.cpp
  IR/Diag.cpp
  Framework/HIRRegionIdentification.cpp
  Framework/HIRSCCFormation.cpp
  Framework/HIRCreation.cpp
  Framework/HIRCleanup.cpp
  Framework/HIRLoopFormation.cpp
  Framework/HIRScalarSymbaseAssignment.cpp
  Framework/HIRParser.cpp
  Framework/HIRSymbaseAssignment.cpp
  Framework/HIRFramework.cpp
  Analysis/HIRAnalysisPass.cpp
  Analysis/HIRDDAnalysis.cpp
  Analysis/DDTests.cpp
  Analysis/DDGraph.cpp
  Analysis/HIRLoopResource.cpp
  Analysis/HIRLoopStatistics.cpp
  Analysis/HIRSafeReductionAnalysis.cpp
  Analysis/HIRSparseArrayReductionAnalysis.cpp
  Analysis/HIRLocalityAnalysis.cpp
  Analysis/HIRParVecAnalysis.cpp
  Utils/BlobUtils.cpp
  Utils/DDUtils.cpp
  Utils/HLNodeUtils.cpp
  Utils/CanonExprUtils.cpp
  Utils/DDRefUtils.cpp
  
  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Analysis
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Analysis/Intel_LoopAnalysis
  )

add_dependencies(LLVMIntel_LoopAnalysis intrinsics_gen directives_gen)
add_subdirectory(OptReport)

if(INTEL_INCLUDE_DTRANS)
  target_link_libraries(LLVMIntel_LoopAnalysis PRIVATE LLVMIntel_DTransOpt)
endif(INTEL_INCLUDE_DTRANS)
