add_llvm_library(LLVMSIMDFunctionCloning
  SIMDFunctionCloning.cpp
)

add_dependencies(LLVMSIMDFunctionCloning intrinsics_gen)
