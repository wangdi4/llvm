add_llvm_library(LLVMVectorize
  LoadStoreVectorizer.cpp
  LoopVectorizationLegality.cpp
  LoopVectorize.cpp
  SLPVectorizer.cpp
  Vectorize.cpp
  VPlan.cpp
  VPlanHCFGBuilder.cpp
  VPlanHCFGTransforms.cpp
  VPlanVerifier.cpp
# INTEL_CUSTOMIZATION
  Intel_VPlan/IntelLoopCFU.cpp
  Intel_VPlan/IntelLoopVectorizationCodeGen.cpp
  Intel_VPlan/IntelLoopVectorizationPlanner.cpp
  Intel_VPlan/IntelVPlan.cpp
  Intel_VPlan/IntelVPlanBranchDependenceAnalysis.cpp
  Intel_VPlan/IntelVPlanCostModel.cpp
  Intel_VPlan/IntelVPlanCostModelProprietary.cpp
  Intel_VPlan/IntelVPlanDriver.cpp
  Intel_VPlan/IntelVPlanHCFGBuilder.cpp
  Intel_VPlan/IntelVPlanPredicator.cpp
  Intel_VPlan/IntelVPlanVerifier.cpp
  Intel_VPlan/VPlanHIR/IntelLoopVectorizationPlannerHIR.cpp
  Intel_VPlan/VPlanHIR/IntelVPOCodeGenHIR.cpp
  Intel_VPlan/VPlanHIR/IntelVPlanDecomposerHIR.cpp
  Intel_VPlan/VPlanHIR/IntelVPlanHCFGBuilderHIR.cpp
  Intel_VPlan/VPlanHIR/IntelVPlanVerifierHIR.cpp
  Intel_VPlan/VPlanHIR/IntelVPLoopAnalysisHIR.cpp
# end INTEL_CUSTOMIZATION

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms

  DEPENDS
  intrinsics_gen
# INTEL_CUSTOMIZATION
  intel_directives_gen
# end INTEL_CUSTOMIZATION
  )
