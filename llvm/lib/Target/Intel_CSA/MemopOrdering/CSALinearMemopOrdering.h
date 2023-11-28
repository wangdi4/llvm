//===-- CSALinearMemopOrdering.h - Linear memop ordering pass ---*- C++ -*-===//
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
/// This file contains the interface for the linear memop ordering pass.
///
///===---------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_MEMOPORDERING_LINEARMEMOPORDERING_H
#define LLVM_LIB_TARGET_CSA_MEMOPORDERING_LINEARMEMOPORDERING_H

#include "CSAMemopOrderingBase.h"

namespace llvm {

/// A linear memop ordering pass for CSA.
///
/// This pass orders memory operations linearly so that no memory operation can
/// be started until the one before it finishes. This gives pretty bad
/// performance but is trivial to implement and is always correct.
class CSALinearMemopOrdering : public CSAMemopOrderingBase {
public:
  static char ID;
  CSALinearMemopOrdering() : CSAMemopOrderingBase{ID} {}
  StringRef getPassName() const override;
  void getAnalysisUsage(AnalysisUsage&) const override;

protected:
  void order(Function &) override;

private:
  /// Connection points at the beginning and end of basic blocks.
  struct BBConnections {

    /// The phi node inserted at the beginning of the basic block.
    PHINode *PHI = nullptr;

    /// The final ordering edge coming out of the basic block.
    Value *EndVal = nullptr;
  };

  /// A convenient alias for a map that keeps track of BBConnections.
  using BBConMap = DenseMap<BasicBlock *, BBConnections>;

  /// Builds a BBConnections by linearly ordering all of the operations within
  /// a basic block.
  BBConnections orderBB(BasicBlock &);

  /// Connects chains produced by orderBB across basic blocks.
  void connectBBs(BBConMap &);

  /// Cleans up phi nodes where all of the inputs are the same.
  void removeUselessPHIs(BBConMap &);
};

void initializeCSALinearMemopOrderingPass(PassRegistry &);

} // namespace llvm

#endif
