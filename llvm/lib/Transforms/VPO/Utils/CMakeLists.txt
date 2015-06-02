add_llvm_library(LLVMTransformVPOUtils
  CFGRestructuring.cpp
  IntrinsicUtils.cpp
)

add_dependencies(LLVMTransformVPOUtils intrinsics_gen)
