add_llvm_library(LLVMVectorize
  BBVectorize.cpp
  LoadStoreVectorizer.cpp
  LoopVectorize.cpp
  SLPVectorizer.cpp
  Vectorize.cpp
  VPlan.cpp
  VPlan/IntelVPlan.cpp
  VPlan/VPLoopInfo.cpp
  VPlan/LoopVectorizationCodeGen.cpp
  VPlan/LoopVectorizationPlanner.cpp
  VPlan/VPlanDriver.cpp
  VPlan/Intel_LoopCFU.cpp
  VPlan/VPlanPredicator.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms

  DEPENDS
  intrinsics_gen
  )
