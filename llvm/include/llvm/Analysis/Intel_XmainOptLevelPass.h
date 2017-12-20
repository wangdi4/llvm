//===-------------------- Intel_XmainOptLevelPass.h -----------------------===//
//
// Copyright (C) 2015-2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This is an immutable pass which stores the opt level.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_XMAINOPTLEVEL_H
#define LLVM_ANALYSIS_XMAINOPTLEVEL_H

#include "llvm/Pass.h"
#include "llvm/PassAnalysisSupport.h"

namespace llvm {

class XmainOptLevelPass : public ImmutablePass {
public:
  static char ID;
  unsigned OptLevel;

  XmainOptLevelPass(unsigned OptLevel = 2)
      : ImmutablePass(ID), OptLevel(OptLevel) {
    initializeXmainOptLevelPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  bool doInitialization(Module &M) override;

  unsigned getOptLevel() const { return OptLevel; }
};

ImmutablePass *createXmainOptLevelPass(unsigned OptLevel = 2);

} // namespace llvm

#endif
