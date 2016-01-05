add_subdirectory(ParOpt)
add_subdirectory(Vecopt)
add_subdirectory(Utils)

add_llvm_library(LLVMVPODriver
  Driver/VPODriver.cpp
  Vecopt/CodeGen/VPOAvrLLVMCodeGen.cpp
  Vecopt/CodeGen/VPOAvrHIRCodeGen.cpp
)

add_dependencies(LLVMVPODriver intrinsics_gen)
