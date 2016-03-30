#
#    Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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

add_llvm_library(LLVMIntel_VPOAnalysis
  VPOAnalysis.cpp

  WRegionInfo/WRegionNode.cpp
  WRegionInfo/WRegion.cpp
  WRegionInfo/WRegionCollection.cpp
  WRegionInfo/WRegionInfo.cpp
  WRegionInfo/WRegionUtils.cpp
  WRegionInfo/WRegionInfoAnalysis.cpp

  Vecopt/VPOAvrGenerate.cpp
  Vecopt/VPOAvr.cpp
  Vecopt/VPOAvrFunction.cpp
  Vecopt/VPOAvrLoop.cpp
  Vecopt/VPOAvrLoopIR.cpp
  Vecopt/VPOAvrLoopHIR.cpp
  Vecopt/VPOAvrStmt.cpp
  Vecopt/VPOAvrStmtIR.cpp
  Vecopt/VPOAvrStmtHIR.cpp
  Vecopt/VPOAvrSwitch.cpp
  Vecopt/VPOAvrSwitchIR.cpp
  Vecopt/VPOAvrSwitchHIR.cpp
  Vecopt/VPOAvrIf.cpp
  Vecopt/VPOAvrIfIR.cpp
  Vecopt/VPOAvrIfHIR.cpp
  Vecopt/VPOAvrUtils.cpp
  Vecopt/VPOAvrUtilsIR.cpp
  Vecopt/VPOAvrUtilsHIR.cpp

  # These two files are to be deleted.
  Vecopt/VPOVecCandIdentify.cpp
  Vecopt/VPOVectorCandidate.cpp
)

add_dependencies(LLVMIntel_VPOAnalysis intrinsics_gen)


