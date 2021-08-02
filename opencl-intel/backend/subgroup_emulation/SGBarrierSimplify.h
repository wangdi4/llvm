//=------------------------ SGBarrierSimplify.h -*- C++ -*-------------------=//
//
// Copyright (C) 2020-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef BACKEND_SUBGROUP_EMULATION_BARRIER_SIMPLIFY_H
#define BACKEND_SUBGROUP_EMULATION_BARRIER_SIMPLIFY_H

#include "SGHelper.h"
#include "SGSizeAnalysis.h"

#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/KernelBarrierUtils.h"

using namespace llvm;

namespace intel {

// The sub-group emulation passes should be decoupled with barrier passes.
class SGBarrierSimplify : public ModulePass {

public:
  static char ID;

  SGBarrierSimplify() : ModulePass(ID), SizeAnalysis(nullptr) {}

  bool runOnModule(Module &M) override;

  StringRef getPassName() const override { return "SGBarrierSimplify Pass"; }
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<SGSizeAnalysis>();
    AU.addPreserved<SGSizeAnalysis>();
  }

private:
  SGHelper Helper;
  BarrierUtils Utils;

  SGSizeAnalysis *SizeAnalysis;

  /// Remove redundant sub_group_barrier calls and dummy_sg_barrier calls.
  bool removeRedundantBarriers(Function *F);

  bool splitBarrierBB(Function *F);

  /// Remove calls in the dummy region.
  bool simplifyDummyRegion(Function *F);

  /// Remove calls in call regions.
  bool simplifyCallRegion(Function *F);
};
} // namespace intel
#endif // BACKEND_SUBGROUP_EMULATION_BARRIER_SIMPLIFY_H
