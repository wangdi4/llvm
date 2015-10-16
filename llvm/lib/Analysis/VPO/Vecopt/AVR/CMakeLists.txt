add_llvm_library(LLVMVecopt_AvrAnalysis
  VPOAvrGenerate.cpp
  VPOAvr.cpp
  VPOAvrFunction.cpp
  VPOAvrStmt.cpp
  VPOAvrStmtIR.cpp
  VPOAvrIf.cpp
  VPOAvrUtils.cpp
  VPOAvrUtilsIR.cpp
)

add_dependencies(LLVMVecopt_AvrAnalysis intrinsics_gen)
