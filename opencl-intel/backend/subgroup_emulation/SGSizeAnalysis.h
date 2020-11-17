//=------------------------- SGSizeAnalysis.h -*- C++ -*---------------------=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef BACKEND_SUBGROUP_EMULATION_SIZE_ANALYSIS_H
#define BACKEND_SUBGROUP_EMULATION_SIZE_ANALYSIS_H

#include "llvm/Pass.h"

#include <map>
#include <set>

using namespace llvm;

namespace intel {

class SGSizeAnalysis : public ModulePass {

public:
  static char ID;

  SGSizeAnalysis() : ModulePass(ID) {}

  bool runOnModule(Module &M) override;

  StringRef getPassName() const override { return "SGSizeAnalysis Pass"; }

  virtual void print(raw_ostream &OS, const Module *M) const override;

  virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  bool hasEmuSize(Function *F) { return FuncToEmuSizes.count(F); }

  const std::set<unsigned> &getEmuSizes(Function *F) {
    assert(hasEmuSize(F) && "Function doesn't have emulation size");
    return FuncToEmuSizes[F];
  }

private:
  std::map<Function *, std::set<unsigned>> FuncToEmuSizes;
};
} // namespace intel
#endif // BACKEND_SUBGROUP_EMULATION_SIZE_ANALYSIS_H
