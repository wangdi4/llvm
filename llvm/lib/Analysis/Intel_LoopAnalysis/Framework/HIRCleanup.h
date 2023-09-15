//===------ HIRCleanup.h - Cleans up redundant HIR nodes ------*-- C++ --*-===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
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
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include <optional>

namespace llvm {

class Function;
class LoopInfo;
class BasicBlock;

namespace loopopt {

class HIRCreation;
class HLGoto;
class HLNode;
class HLLabel;

class HIRCleanup {
public:
  typedef HLNodeUtils::RequiredLabelsTy RequiredLabelsTy;

private:
  /// Analysis results
  LoopInfo &LI;
  HIRCreation &HIRC;
  HLNodeUtils &HNU;

  /// LoopLatchHooks - Stores HLNodes representing start of a loop latch block.
  SmallDenseMap<const BasicBlock *, HLNode *, 32> LoopLatchHooks;

  /// RequiredLabels - HLLabels which aren't redundant.
  RequiredLabelsTy RequiredLabels;

  /// Regions where Ifs were optimized out.
  SmallPtrSet<const HLRegion *, 8> OptimizedRegions;

  /// Eliminates redundant HLLabels created by HIRCreation pass.
  void eliminateRedundantLabels();

  /// Checks if \p Cond can be implied true or false using SCEV analysis.
  std::optional<bool> isImpliedUsingSCEVAnalysis(Value *Cond);

  /// Eliminates redundant HLIfs in the incoming IR which can be proven to be
  /// true or false.
  void eliminateRedundantIfs();

public:
  HIRCleanup(LoopInfo &LI, HIRCreation &HIRC, HLNodeUtils &HNU)
      : LI(LI), HIRC(HIRC), HNU(HNU) {}

  void run();

  /// Returns the set of required labels.
  const RequiredLabelsTy &getRequiredLabels() const { return RequiredLabels; }

  /// Finds a hook in the HIR which corresponds to this basic block.
  HLNode *findHIRHook(const BasicBlock *BB) const;

  /// Returns true if (non-label/goto) nodes were eliminated from the region.
  bool isOptimizedRegion(const HLRegion *Reg) const {
    return OptimizedRegions.count(Reg);
  }
};

} // End namespace loopopt

} // End namespace llvm

#endif
