//=--------------------------- SGBuiltin.h -*- C++ -*------------------------=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef BACKEND_SUBGROUP_EMULATION_BUILTIN_H
#define BACKEND_SUBGROUP_EMULATION_BUILTIN_H

#include "SGHelper.h"
#include "SGSizeAnalysis.h"

#include "llvm/Pass.h"

using namespace llvm;

namespace intel {

class SGBuiltin : public ModulePass {

public:
  static char ID;

  SGBuiltin() : ModulePass(ID), SizeAnalysis(nullptr) {}

  bool runOnModule(Module &M) override;

  StringRef getPassName() const override { return "SGBuiltin Pass"; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<SGSizeAnalysis>();
    AU.addPreserved<SGSizeAnalysis>();
  }

private:
  SGHelper Helper;

  SGSizeAnalysis *SizeAnalysis;

  /// Add vector-variants attribute for sub-group functions.
  /// Insert sg_barrier/dummy_sg_barrier before/after the call.
  bool insertSGBarrierForSGCalls(Module &M);

  /// Insert sg_barrier/dummy_sg_barrier for WG Barriers
  bool insertSGBarrierForWGBarriers(Module &M);
};
} // namespace intel
#endif // BACKEND_SUBGROUP_EMULATION_GROUP_BUILTIN_H
