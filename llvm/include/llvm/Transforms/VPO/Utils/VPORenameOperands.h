#if INTEL_COLLAB // -*- C++ -*-
//===----- VPRenameOperands.cpp - Rename Operands Pass for OpenMP ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file declares the VPO Rename Operands pass.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_VPORENAMEOPERANDS_H
#define LLVM_TRANSFORMS_VPO_VPORENAMEOPERANDS_H

#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class DominatorTree;
class LoopInfo;

/// Renames clause operands on memory-motion-guard directives using a
/// store-then-load to prevent motion of constant-expressions using clause
/// operands as base, across the region directives. \see
/// VPOUtils::renameOperandsUsingStoreThenLoad() for details on how the renaming
/// is done. \see VPOUtils::restoreOperands() for how the renaming is undone.
class VPORenameOperandsPass : public PassInfoMixin<VPORenameOperandsPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

  bool runImpl(Function &F, vpo::WRegionInfo &WI, DominatorTree *DT,
               LoopInfo *LI);
  static bool isRequired() { return true; }
};

namespace vpo {

class VPORenameOperands : public FunctionPass {
public:
  static char ID; // Pass identification

  VPORenameOperands();

  StringRef getPassName() const override { return "VPO Rename Operands"; }
  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;

private:
  VPORenameOperandsPass Impl;
};

} // end namespace vpo
} // end namespace llvm

#endif // LLVM_TRANSFORMS_VPO_VPORESTOREOPERANDS_H
#endif // INTEL_COLLAB
