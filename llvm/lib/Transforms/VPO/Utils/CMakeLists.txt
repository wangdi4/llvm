add_llvm_library(LLVMTransformVPOUtils
  CFGRestructuring.cpp
  IntrinsicUtils.cpp
  GeneralUtils.cpp
)

add_dependencies(LLVMTransformVPOUtils intrinsics_gen)
