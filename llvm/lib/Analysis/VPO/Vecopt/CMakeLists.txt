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
  AVR/VPOAvrGenerate.cpp
  AVR/VPOAvr.cpp
  AVR/VPOAvrFunction.cpp
  AVR/VPOAvrLoop.cpp
  AVR/VPOAvrStmt.cpp
  AVR/VPOAvrIf.cpp
  AVR/VPOAvrUtils.cpp
  CandidateIdent/VPOVecCandIdentify.cpp
  CandidateIdent/VPOVectorCandidate.cpp
)

add_dependencies(LLVMVecopt_Analysis intrinsics_gen)


