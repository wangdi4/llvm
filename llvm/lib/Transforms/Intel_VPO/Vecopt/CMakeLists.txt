# -------------------------- VPO VECTORIZER ---------------------------

add_llvm_library(LLVMVPOVecopt_Transform
   VPOVecoptTransform.cpp
)

add_dependencies(LLVMVPOVecopt_Transform intrinsics_gen)
