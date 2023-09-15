//=== SGBarrierSimplify.h - Simplify subgroup barrier ------------ C++ -*---==//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_SG_EMULATION_SG_BARRIER_SIMPLIFY_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_SG_EMULATION_SG_BARRIER_SIMPLIFY_H

#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/SYCLTransforms/SubgroupEmulation/SGHelper.h"
#include "llvm/Transforms/SYCLTransforms/Utils/BarrierUtils.h"

namespace llvm {

/// Simplify subgroup barriers:
///   * remove duplicate subgroup barriers.
///   * split barrier BasicBlock.
class SGBarrierSimplifyPass : public PassInfoMixin<SGBarrierSimplifyPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // Glue for old PM.
  bool runImpl(Module &M);

private:
  BarrierUtils Utils;
  SGHelper Helper;

  /// Remove calls in call regions.
  bool simplifyCallRegion(Function *F);

  /// Remove redundant sub_group_barrier calls and dummy_sg_barrier calls.
  bool removeRedundantBarriers(Function *F);

  /// Remove calls in the dummy region.
  bool simplifyDummyRegion(Function *F);

  /// Split basic block to assure barrier instructions appears only at the
  /// begining of basic block and not more than once in each basic block.
  bool splitBarrierBB(Function *F);
};

} // namespace llvm

#endif
