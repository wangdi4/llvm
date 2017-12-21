add_llvm_library(LLVMIntel_DTrans
  DTransOpt.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/Intel_DTrans

  DEPENDS
  intrinsics_gen
  LLVMIntel_DTransAnalysis
  )
