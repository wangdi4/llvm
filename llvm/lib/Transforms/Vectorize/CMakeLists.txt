add_llvm_library(LLVMVectorize
  LoadStoreVectorizer.cpp
  LoopVectorize.cpp
  SLPVectorizer.cpp
  Vectorize.cpp
  VPlan.cpp
  Intel_VPlan.cpp
# INTEL_CUSTOMIZATION
  VPlan/Intel_LoopCFU.cpp
  VPlan/IntelVPlan.cpp
  VPlan/LoopVectorizationCodeGen.cpp
  VPlan/LoopVectorizationPlanner.cpp
  VPlan/VPlanDriver.cpp
  VPlan/VPlanHCFGBuilder.cpp
  VPlan/VPlanPredicator.cpp
  VPlan/VPlanVerifier.cpp
# end INTEL_CUSTOMIZATION

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms

  DEPENDS
  intrinsics_gen
  )
