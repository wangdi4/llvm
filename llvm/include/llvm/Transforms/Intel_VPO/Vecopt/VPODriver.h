//===-- VPODriver.h ---------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This header file declares the base class for VPO Vectorizer Driver.
///
/// High-level flow of the vectorizer driver:
///
/// + VPODriver: per function.
//  |   ForEachRegion:
/// |   + VPOScenariosEngine: per SESE region (Wrn);
/// |   |   A region may contain a hierarchy of loops.
/// |   |   ScenariosEngine considers all the vectorization candidates in the region.
/// |   |   +  ForEachScenario (Scenario is a loop/combination-of-loops)
//  |   |   |  +  Do some processing
//  |   |   |  |  ...
//  |   |   |  |  Foreach VectorizationFactor 
//  |   |   |  |  +   VPOVecContext VC <-- (Aloop,VF)
//  |   |   |  |  |   getCost(VC); // evaluate specific candidate.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_VPODRIVER_H
#define LLVM_TRANSFORMS_VPO_VPODRIVER_H

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrGenerate.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOScenarioEvaluation.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VPOSIMDLaneEvolution.h"
#include "llvm/Analysis/TargetTransformInfo.h"

namespace llvm {

class Function;
class LoopInfo;

namespace vpo {

//
// VPODriverBase class can be moved inside anonymous namespace
// inside VPODriver.cpp, as long as the derived classes remain
// in that single file. For now, keep this in the header file.
//
class VPODriverBase : public FunctionPass {

  LoopInfo *LI;
  ScalarEvolution *SC;
  WRegionInfo *WR;

protected:
  /// Handle to Target Information 
  const TargetTransformInfo *TTI;

  /// Handle to AVR Generate Pass
  AVRGenerateBase *AV;

public:
  VPODriverBase(char &ID) : FunctionPass(ID){};
  bool runOnFunction(Function &F) override;

  /// Get a handle to the engine that explores and evaluates the 
  /// vectorization opportunities in a Region.
  virtual VPOScenarioEvaluationBase &getScenariosEngine(AVRWrn *AWrn) = 0;

  /// Call the destcructor of the ScenariosEngine for this region. 
  virtual void resetScenariosEngineForRegion() = 0;
};

} // End namespace vpo

} // End namespace llvm

#endif // LLVM_TRANSFORMS_VPO_VPODRIVER_H
