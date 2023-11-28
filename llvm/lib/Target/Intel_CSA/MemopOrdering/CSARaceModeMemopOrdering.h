//===-- CSARaceModeMemopOrdering.h - Race mode memop ordering ---*- C++ -*-===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
///===---------------------------------------------------------------------===//
/// \file
///
/// This file contains the interface for the race mode memop ordering pass.
///
///===---------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_MEMOPORDERING_RACEMODEMEMOPORDERING_H
#define LLVM_LIB_TARGET_CSA_MEMOPORDERING_RACEMODEMEMOPORDERING_H

#include "CSAMemopOrderingBase.h"
#include "llvm/IR/Dominators.h"

namespace llvm {

/// A race mode memop ordering pass for CSA.
///
/// This pass only orders memory operations with the beginning and end of the
/// function but not with anything else, so expect data races. This will not
/// produce correct code in general, but it can be useful for testing codegen
/// and for doing certain kinds of experiments. It is effectively the opposite
/// of linear memop ordering, which enforces too much ordering to achieve
/// correctness at the cost of performance.
class CSARaceModeMemopOrdering : public CSAMemopOrderingBase {
public:
  static char ID;
  CSARaceModeMemopOrdering() : CSAMemopOrderingBase{ID} {}
  StringRef getPassName() const override;
  void getAnalysisUsage(AnalysisUsage &) const override;

protected:
  void order(Function &) override;

private:
  /// A dominator tree for the current function being ordered.
  DominatorTree *DT;

  /// A collection of important information that needs to be tracked for each
  /// basic block.
  struct BBRecord {

    /// An indicator for the recursion level at which this basic block dominates
    /// the basic block under consideration. Zero indicates that the basic block
    /// does not dominate any basic blocks currently under consideration.
    unsigned DomLevel = 0;

    /// A token value representing the completion of all of the ops in this
    /// basic block.
    Value *MergedOps;
  };

  /// A map to keep track of the BBRecord for each basic block in the current
  /// function.
  DenseMap<BasicBlock *, BBRecord> BBRecs;

  /// Merges all of the non-return ops in \p BB to create a new BBRecord and
  /// adds any return instructions encountered to \p Returns.
  BBRecord mergeBBOps(BasicBlock &BB, SmallVectorImpl<ReturnInst *> &Returns);

  /// Recursively collects ordering edges from ops that are not already
  /// accounted for in the current traversal.
  ///
  /// \param Start The basic block to start at when collecting op edges. The
  /// traversal will cover the dominators of this block and any predecessors
  /// that aren't already marked elsewhere in the call stack.
  /// \param DomLevel The recursion level to use when marking dominators.
  /// \result The merged value from this portion of the traversal.
  Value *collectMergedOps(BasicBlock *Start, unsigned DomLevel = 1);
};

void initializeCSARaceModeMemopOrderingPass(PassRegistry &);

} // namespace llvm

#endif
