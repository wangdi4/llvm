add_llvm_library(LLVMVPOParopt
  VPOParopt.cpp
  VPOParoptTransform.cpp
  VPOParoptUtils.cpp
)

add_dependencies(LLVMVPOParopt intrinsics_gen)
