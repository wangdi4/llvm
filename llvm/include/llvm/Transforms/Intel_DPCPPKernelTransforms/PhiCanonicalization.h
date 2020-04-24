//==--- PhiCanonicalization.h - Canonicalization of Phi nodes ---- C++ -*---==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef __INTEL_DPCPP_PHI_CANONICALIZATION_H__
#define __INTEL_DPCPP_PHI_CANONICALIZATION_H__

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"

namespace llvm {

/// Phi canonicalizatiton. This pass converts each PHINode with three
/// or more entries into a two-based PHINode. It does so by
/// splitting two of the edges and creating an additional basic block.
/// This trashes the CFG. However, future passes can easily go over the
/// CFG and clean it.
class PhiCanonicalization : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid

  PhiCanonicalization();

  /// Provides name of pass
  StringRef getPassName() const override { return "PhiCanonicalization"; }

  /// LLVM Function pass entry
  /// F Function to transform
  /// Return 'true' if changed
  bool runOnFunction(Function &F) override;

  /// Perform the modifications to the BasicBlock
  /// Create a new BasicBlock with incoming edges
  /// ToFix BasicBlock to Fix
  void fixBlock(BasicBlock *ToFix);

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

  // Need Dominator Tree and PostDominator tree prior to Phi Canonization
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

} // namespace llvm

#endif // define __INTEL_DPCPP_PHI_CANONICALIZATION_H__
