add_llvm_library(LLVMSIMDFunctionCloning
  ../Services/utils/VectorizerUtils.cpp
  ../Utils/VectorVariant.cpp
  SIMDFunctionCloning.cpp
)

add_dependencies(LLVMSIMDFunctionCloning intrinsics_gen)
