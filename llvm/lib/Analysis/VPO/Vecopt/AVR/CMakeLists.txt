add_llvm_library(LLVMVecopt_AvrAnalysis
  VPOAvrGenerate.cpp
  VPOAvr.cpp
  VPOAvrFunction.cpp
  VPOAvrStmt.cpp
  VPOAvrStmtIR.cpp
  VPOAvrStmtHIR.cpp
  VPOAvrIf.cpp
  VPOAvrIfIR.cpp
  VPOAvrIfHIR.cpp
  VPOAvrUtils.cpp
  VPOAvrUtilsIR.cpp
  VPOAvrUtilsHIR.cpp
)

add_dependencies(LLVMVecopt_AvrAnalysis intrinsics_gen)
