add_llvm_component_library(LLVMIntel_DPCPPKernelTransforms
  DPCPPKernelTransforms.cpp
  DPCPPKernelVecClone.cpp
  DPCPPPrepareKernelForVecClone.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/Intel_DPCPPKernelTransforms

  DEPENDS
  intrinsics_gen
  directives_gen
)
