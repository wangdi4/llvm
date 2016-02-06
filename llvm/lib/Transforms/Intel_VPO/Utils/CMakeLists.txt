add_llvm_library(LLVMTransformVPOUtils
  CFGRestructuring.cpp
  IntrinsicUtils.cpp
  OpenMPDirectivesAndClauses.cpp
  GeneralUtils.cpp
  MultiVersioning.cpp
)

add_dependencies(LLVMTransformVPOUtils intrinsics_gen)
