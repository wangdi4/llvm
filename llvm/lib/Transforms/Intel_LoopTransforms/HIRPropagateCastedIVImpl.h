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

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRPROPAGATECASTEDIVIMPL_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRPROPAGATECASTEDIVIMPL_H

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"

#include "llvm/Pass.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {

namespace loopopt {

class DDGraph;
class HIRDDAnalysis;
class HIRLoopStatistics;

class HIRPropagateCastedIV {
  HIRFramework &HIRF;

public:
  HIRPropagateCastedIV(HIRFramework &HIRF) : HIRF(HIRF) {}

  bool run();

private:
  bool doCollection(HLLoop *Lp, SmallVectorImpl<RegDDRef *> &MemRefs,
                    unsigned &CandidateBlobIndex, RegDDRef *&CandidateRef,
                    bool &CanDeleteCandidateInst);

  bool propagateCastedIV(HLLoop *Lp);
};

//
} // namespace loopopt
} // namespace llvm

#endif
