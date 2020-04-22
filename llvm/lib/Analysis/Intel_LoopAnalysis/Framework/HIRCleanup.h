//===------ HIRCleanup.h - Cleans up redundant HIR nodes ------*-- C++ --*-===//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This analysis is used to clean up redundant nodes created by HIRCreation
// pass.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_LOOPANALYSIS_HIRCLEANUP_H
#define LLVM_ANALYSIS_INTEL_LOOPANALYSIS_HIRCLEANUP_H

#include "llvm/ADT/DenseMap.h"

namespace llvm {

class Function;
class LoopInfo;
class BasicBlock;

namespace loopopt {

class HIRCreation;
class HLNodeUtils;
class HLNode;
class HLLabel;

class HIRCleanup {
public:
  typedef SmallVector<HLLabel *, 64> RequiredLabelsTy;

private:
  /// Analysis results
  LoopInfo &LI;
  HIRCreation &HIRC;
  HLNodeUtils &HNU;

  /// LoopLatchHooks - Stores HLNodes representing start of a loop latch block.
  SmallDenseMap<const BasicBlock *, HLNode *, 32> LoopLatchHooks;

  /// RequiredLabels - HLLabels which aren't redundant.
  RequiredLabelsTy RequiredLabels;

  /// \brief Eliminates redundant HLGotos created by HIRCreation pass.
  void eliminateRedundantGotos();

  /// \brief Eliminates redundant HLLabels created by HIRCreation pass.
  void eliminateRedundantLabels();

public:
  HIRCleanup(LoopInfo &LI, HIRCreation &HIRC, HLNodeUtils &HNU)
      : LI(LI), HIRC(HIRC), HNU(HNU) {}

  void run();

  /// \brief Returns the set of required labels.
  const RequiredLabelsTy &getRequiredLabels() const { return RequiredLabels; }

  /// \brief Finds a hook in the HIR which corresponds to this basic block.
  HLNode *findHIRHook(const BasicBlock *BB) const;
};

} // End namespace loopopt

} // End namespace llvm

#endif
