add_llvm_component_library(LLVMIntel_LoopAnalysis
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
  Framework/HIRRegionIdentification.cpp
  Framework/HIRSCCFormation.cpp
  Framework/HIRCreation.cpp
  # INTEL_FEATURE_SHARED_SW_ADVANCED
  Framework/HIRCleanup.cpp
  #end INTEL_FEATURE_SHARED_SW_ADVANCED
  Framework/HIRLoopFormation.cpp
  Framework/HIRScalarSymbaseAssignment.cpp
  Framework/HIRParser.cpp
  # INTEL_FEATURE_SHARED_SW_ADVANCED
  Framework/HIRSymbaseAssignment.cpp
  #end INTEL_FEATURE_SHARED_SW_ADVANCED
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
  Analysis/HIRArraySectionAnalysis.cpp
  Utils/BlobUtils.cpp
  Utils/DDUtils.cpp
  Utils/HLNodeUtils.cpp
  Utils/CanonExprUtils.cpp
  Utils/DDRefUtils.cpp
  Utils/SIMDIntrinsicChecker.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Analysis
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Analysis/Intel_LoopAnalysis

  DEPENDS
  intrinsics_gen
  directives_gen

  LINK_COMPONENTS
  Analysis
  Core
  Support
  ProfileData
  Object
  TransformUtils
  )

if(INTEL_CUSTOMIZATION)
# INTEL_FEATURE_SW_DTRANS
  is_intel_feature_enabled(p ${LLVM_INTELFEATURE_PREFIX}_SW_DTRANS)
  if (p)
    target_link_libraries(LLVMIntel_LoopAnalysis PRIVATE LLVMIntel_DTransOpt
                                                      LLVMIntel_DTransAnalysis)
  endif()
# end INTEL_FEATURE_SW_DTRANS
endif()
