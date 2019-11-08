add_llvm_component_library(LLVMVectorize
  LoadStoreVectorizer.cpp
  LoopVectorizationLegality.cpp
  LoopVectorize.cpp
  SLPVectorizer.cpp
  Vectorize.cpp
  VectorCombine.cpp
  VPlan.cpp
  VPlanHCFGBuilder.cpp
  VPlanPredicator.cpp
  VPlanSLP.cpp
  VPlanTransforms.cpp
  VPlanVerifier.cpp
# INTEL_CUSTOMIZATION
  Intel_LoadCoalescing.cpp
  IntelMFReplacement.cpp
  Intel_VPlan/IntelLoopVectorizationLegality.cpp
  Intel_VPlan/IntelLoopVectorizationPlanner.cpp
  Intel_VPlan/IntelVPBasicBlock.cpp
  Intel_VPlan/IntelVPLoopAnalysis.cpp
  Intel_VPlan/IntelVPOCodeGen.cpp
  Intel_VPlan/IntelVPlan.cpp
  Intel_VPlan/IntelVPlanAlignmentAnalysis.cpp
  Intel_VPlan/IntelVPlanAllZeroBypass.cpp
  Intel_VPlan/IntelVPlanCFGBuilder.cpp
  Intel_VPlan/IntelVPlanCallVecDecisions.cpp
  Intel_VPlan/IntelVPlanClone.cpp
  Intel_VPlan/IntelVPlanCostModel.cpp
  Intel_VPlan/IntelVPlanCostModelProprietary.cpp
  Intel_VPlan/IntelVPlanDivergenceAnalysis.cpp
  Intel_VPlan/IntelVPlanDriver.cpp
  Intel_VPlan/IntelVPlanFunctionVectorizer.cpp
  Intel_VPlan/IntelVPlanHCFGBuilder.cpp
  Intel_VPlan/IntelVPlanIdioms.cpp
  Intel_VPlan/IntelVPlanLoopCFU.cpp
  Intel_VPlan/IntelVPlanLoopExitCanonicalization.cpp
  Intel_VPlan/IntelVPlanLoopInfo.cpp
  Intel_VPlan/IntelVPlanLoopUnroller.cpp
  Intel_VPlan/IntelVPlanPragmaOmpOrderedSimdExtract.cpp
  Intel_VPlan/IntelVPlanPredicator.cpp
  Intel_VPlan/IntelVPlanScalarEvolution.cpp
  Intel_VPlan/IntelVPlanScalVecAnalysis.cpp
  Intel_VPlan/IntelVPlanSSADeconstruction.cpp
  Intel_VPlan/IntelVPlanSyncDependenceAnalysis.cpp
  Intel_VPlan/IntelVPlanValueTracking.cpp
  Intel_VPlan/IntelVPlanVLSAnalysis.cpp
  Intel_VPlan/IntelVPlanVLSClient.cpp
  Intel_VPlan/IntelVPlanVectorShape.cpp
  Intel_VPlan/IntelVPlanVerifier.cpp
  Intel_VPlan/IntelVPSOAAnalysis.cpp
  Intel_VPlan/VPlanHIR/IntelLoopVectorizationPlannerHIR.cpp
  Intel_VPlan/VPlanHIR/IntelVPOCodeGenHIR.cpp
  Intel_VPlan/VPlanHIR/IntelVPlanDecomposerHIR.cpp
  Intel_VPlan/VPlanHIR/IntelVPlanHCFGBuilderHIR.cpp
  Intel_VPlan/VPlanHIR/IntelVPlanVLSAnalysisHIR.cpp
  Intel_VPlan/VPlanHIR/IntelVPlanVerifierHIR.cpp
# end INTEL_CUSTOMIZATION

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/Vectorize

  DEPENDS
  intrinsics_gen
# INTEL_CUSTOMIZATION
  directives_gen
# end INTEL_CUSTOMIZATION
  )

# INTEL_CUSTOMIZATION
if(INTEL_INCLUDE_DTRANS)
  target_link_libraries(LLVMVectorize PRIVATE LLVMIntel_DTransOpt)
endif(INTEL_INCLUDE_DTRANS)
# end INTEL_CUSTOMIZATION
