#
#    Copyright (C) 2015 Intel Corporation. All rights reserved.
#
#    The information and source code contained herein is the exclusive
#    property of Intel Corporation. and may not be disclosed, examined
#    or reproduced in whole or in part without explicit written authorization
#    from the company.
#
#    Source file: CMakeLists.txt
#    ------------
#    Configures build for project.
#

add_llvm_library(LLVMVecopt_Analysis
  VPOVecoptAnalysis.cpp
  VPOAvrGenerate.cpp
  VPOAvr.cpp
  VPOAvrFunction.cpp
  VPOAvrLoop.cpp
  VPOAvrStmt.cpp
  VPOAvrStmtIR.cpp
  VPOAvrStmtHIR.cpp
  VPOAvrIf.cpp
  VPOAvrIfIR.cpp
  VPOAvrIfHIR.cpp
  VPOAvrUtils.cpp
  VPOAvrUtilsIR.cpp
  VPOAvrUtilsHIR.cpp

  # These two files are to be deleted.
  VPOVecCandIdentify.cpp
  VPOVectorCandidate.cpp
)

add_dependencies(LLVMVecopt_Analysis intrinsics_gen)


