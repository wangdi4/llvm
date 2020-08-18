add_llvm_component_library(LLVMIntel_DPCPPKernelTransforms
  DPCPPKernelTransforms.cpp
  DPCPPKernelVecClone.cpp
  DPCPPPrepareKernelForVecClone.cpp
  DPCPPKernelPostVec.cpp
  DPCPPKernelCompilationUtils.cpp
  DPCPPKernelLoopUtils.cpp
  DPCPPKernelWGLoopCreator.cpp
  DPCPPKernelAnalysis.cpp
  PhiCanonicalization.cpp
  RedundantPhiNodePass.cpp
  SplitBBonBarrierPass.cpp
  DPCPPKernelBarrierUtils.cpp
  WIRelatedValuePass.cpp
  DataPerBarrierPass.cpp
  DataPerValuePass.cpp
  BarrierPass.cpp
  BarrierInFunctionPass.cpp
  PostBarrierPass.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/Intel_DPCPPKernelTransforms

  DEPENDS
  intrinsics_gen
  directives_gen
)
