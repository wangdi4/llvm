add_llvm_library(LLVMIntel_OpenCLTransforms
  FMASplitter.cpp
  OpenCLTransforms.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/Intel_OpenCLTransforms

  DEPENDS
  intrinsics_gen
)
