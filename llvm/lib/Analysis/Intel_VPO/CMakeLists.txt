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

  Utils/IntrinsicUtils.cpp
  Utils/GeneralUtils.cpp
  Utils/OpenMPDirectivesAndClauses.cpp

  WRegionInfo/WRegionNode.cpp
  WRegionInfo/WRegion.cpp
  WRegionInfo/WRegionClause.cpp
  WRegionInfo/WRegionCollection.cpp
  WRegionInfo/WRegionInfo.cpp
  WRegionInfo/WRegionUtils.cpp
  WRegionInfo/WRegionInfoAnalysis.cpp

  Vecopt/VPOAvrGenerate.cpp
  Vecopt/VPOAvrDecomposeHIR.cpp
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
  Vecopt/VPODefUse.cpp
  Vecopt/VPOCFG.cpp
  Vecopt/VPOSIMDLaneEvolution.cpp
  Vecopt/VPOPredicator.cpp
  Vecopt/VPOScenarioEvaluation.cpp

  Vecopt/VectorGraph.cpp
  Vecopt/VectorGraphUtils.cpp
  Vecopt/VectorGraphInfo.cpp
  Vecopt/VectorGraphPredicator.cpp
)

add_dependencies(LLVMIntel_VPOAnalysis intrinsics_gen)


