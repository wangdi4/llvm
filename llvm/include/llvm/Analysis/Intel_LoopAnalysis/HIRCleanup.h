//===------ HIRCleanup.h - Cleans up redundant HIR nodes ------*-- C++ --*-===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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

#include "llvm/Pass.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallPtrSet.h"

namespace llvm {

class Function;
class LoopInfo;

namespace loopopt {

class HIRCreation;
class HLNode;
class HLLabel;

class HIRCleanup : public FunctionPass {
public:
  typedef SmallVector<HLLabel *, 64> RequiredLabelsTy;

private:
  /// HIR - pointer to HIRCreation pass.
  HIRCreation *HIR;

  /// LI - The loop information for the function we are currently analyzing.
  LoopInfo *LI;

  /// LoopLatchHooks - Stores HLNodes representing start of a loop latch block.
  SmallDenseMap<const BasicBlock *, HLNode *, 32> LoopLatchHooks;

  /// RequiredLabels - HLLabels which aren't redundant.
  RequiredLabelsTy RequiredLabels;

  /// \brief Eliminates redundant HLGotos created by HIRCreation pass.
  void eliminateRedundantGotos();

  /// \brief Eliminates redundant HLLabels created by HIRCreation pass.
  void eliminateRedundantLabels();

public:
  static char ID; // Pass identification
  HIRCleanup();

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void print(raw_ostream &OS, const Module * = nullptr) const override;
  void verifyAnalysis() const override;

  /// \brief Returns the set of required labels.
  const RequiredLabelsTy &getRequiredLabels() const { return RequiredLabels; }

  /// \brief Finds a hook in the HIR which corresponds to this basic block.
  HLNode *findHIRHook(const BasicBlock *BB) const;
};

} // End namespace loopopt

} // End namespace llvm

#endif
