//==--- PhiCanonicalization.h - Canonicalization of Phi nodes ---- C++ -*---==//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_PHI_CANONICALIZATION_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_PHI_CANONICALIZATION_H

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"

namespace llvm {
class DominatorTree;
class PostDominatorTree;
/// Phi canonicalizatiton. This pass converts each PHINode with three
/// or more entries into a two-based PHINode. It does so by
/// splitting two of the edges and creating an additional basic block.
/// This trashes the CFG. However, future passes can easily go over the
/// CFG and clean it.
class PhiCanonicalization : public PassInfoMixin<PhiCanonicalization> {
  /// Perform the modifications to the BasicBlock
  /// Create a new BasicBlock with incoming edges
  /// ToFix BasicBlock to Fix
  void fixBlock(BasicBlock *ToFix, DominatorTree *DT, PostDominatorTree *PDT);

  /// Make new intermediate Basic Block so that it will become PHI block
  /// for 'prev0' and 'prev1' instead of old one
  /// ToFix old block to be replaced as PHI block for 'prev0' and 'prev1'
  /// Prev0 first block to be retargeted to the intermediate block
  /// Prev1 second block to be retargeted to the intermediate block
  /// Return pointer to the created Basic Block
  BasicBlock *makeNewPhiBB(BasicBlock *ToFix, BasicBlock *Prev0,
                           BasicBlock *Prev1);

  /// After creating a new intermediate BasicBlock,
  /// predecessors must jump to the new BB and not to the old one.
  /// To_fix The Pointer
  /// Old_target The pointee
  /// New_target New target to point
  void fixBasicBlockSucessor(BasicBlock *To_fix, BasicBlock *Old_target,
                             BasicBlock *New_target);

public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);

  bool runImpl(Function &F, DominatorTree *DT, PostDominatorTree *PDT);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_PHI_CANONICALIZATION_H
