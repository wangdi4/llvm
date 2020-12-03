//=----------------------- SGBarrierPropagate.h -*- C++ -*-------------------=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef BACKEND_SUBGROUP_EMULATION_BARRIER_PROPAGATE_H
#define BACKEND_SUBGROUP_EMULATION_BARRIER_PROPAGATE_H

#include "SGHelper.h"
#include "SGSizeAnalysis.h"

#include "llvm/Pass.h"

using namespace llvm;

namespace intel {

class SGBarrierPropagate : public ModulePass {

public:
  static char ID;

  SGBarrierPropagate() : ModulePass(ID), SizeAnalysis(nullptr) {}

  bool runOnModule(Module &M) override;

  StringRef getPassName() const override { return "SGBarrierPropagate Pass"; }
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<SGSizeAnalysis>();
    AU.addPreserved<SGSizeAnalysis>();
  }

private:
  SGHelper Helper;

  SGSizeAnalysis *SizeAnalysis;

  /// Insert sub_group_barrier at the end of the function
  /// Insert dummy_sg_barrier at the begin of the function
  void addBarrierToFunction(Function &F);
};
} // namespace intel
#endif // BACKEND_SUBGROUP_EMULATION_BARRIER_PROPAGATE_H
