add_llvm_library(LLVMVectorize
  LoadStoreVectorizer.cpp
  LoopVectorize.cpp
  SLPVectorizer.cpp
  Vectorize.cpp
  VPlan.cpp
# INTEL_CUSTOMIZATION
  Intel_VPlan.cpp
  Intel_VPlan/LoopCFU.cpp
  Intel_VPlan/VPlan.cpp
  Intel_VPlan/LoopVectorizationCodeGen.cpp
  Intel_VPlan/LoopVectorizationPlanner.cpp
  Intel_VPlan/LoopVectorizationPlannerHIR.cpp
  Intel_VPlan/VPlanDriver.cpp
  Intel_VPlan/VPlanHCFGBuilder.cpp
  Intel_VPlan/VPlanHCFGBuilderHIR.cpp
  Intel_VPlan/VPlanPredicator.cpp
  Intel_VPlan/VPlanVerifier.cpp
  Intel_VPlan/VPlanVerifierHIR.cpp
  Intel_VPlan/VPOCodeGenHIR.cpp
# end INTEL_CUSTOMIZATION

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms

  DEPENDS
  intrinsics_gen
  )
