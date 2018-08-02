//===--- HIRLastValueComputationImpl.h
//-----------------------------------------*- C++ -*---===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRLASTVALUECOMPUTATIONIMPL_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRLASTVALUECOMPUTATIONIMPL_H

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"

#include "llvm/Pass.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {

namespace loopopt {

class DDGraph;
class HIRDDAnalysis;
class HIRLoopStatistics;

namespace lastvaluecomputation {

class HIRLastValueComputation {
  HIRFramework &HIRF;
  HIRDDAnalysis &HDDA;

public:
  HIRLastValueComputation(HIRFramework &HIRF, HIRDDAnalysis &HDDA)
      : HIRF(HIRF), HDDA(HDDA) {}

  bool run();

private:
  bool doLastValueComputation(HLLoop *Lp);

  bool isLegalAndProfitable(HLLoop *Lp, HLInst *HInst, unsigned LoopLevel,
                            CanonExpr *UBCE, bool IsUpperBoundComplicated,
                            bool IsNSW);
};

//
} // namespace lastvaluecomputation
} // namespace loopopt
} // namespace llvm

#endif
