//===-------------------- Intel_XmainOptLevelPass.h -----------------------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/PassAnalysisSupport.h"

namespace llvm {

class XmainOptLevel {
  unsigned OptLevel;

public:
  XmainOptLevel(unsigned OptLevel);

  unsigned getOptLevel() const { return OptLevel; }
  void setOptLevel(unsigned Level) { OptLevel = Level; }

  // Handle invalidation explicitly
  bool invalidate(Function &, const PreservedAnalyses &,
                  FunctionAnalysisManager::Invalidator &) {
    // Never invalidate analysis.
    return false;
  }

  bool invalidate(Module &M, const PreservedAnalyses &,
                  ModuleAnalysisManager::Invalidator &) {
    // Never invalidate analysis.
    return false;
  }
};

class XmainOptLevelAnalysis : public AnalysisInfoMixin<XmainOptLevelAnalysis> {
  friend AnalysisInfoMixin<XmainOptLevelAnalysis>;
  static AnalysisKey Key;

public:
  typedef XmainOptLevel Result;
  Result run(Function &F, FunctionAnalysisManager &AM);
  Result run(Module &M, ModuleAnalysisManager &AM);
};

// Helper class for initializing XmainOptLevelAnalysis in the new pass manager
class XmainOptLevelAnalysisInit :
      public PassInfoMixin<XmainOptLevelAnalysisInit> {
private:
  unsigned OptLevel;

public:
  XmainOptLevelAnalysisInit(unsigned OptLevel = 2) : OptLevel(OptLevel) {}
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

class XmainOptLevelWrapperPass : public ImmutablePass {
  XmainOptLevel Impl;

public:
  static char ID;

  XmainOptLevelWrapperPass(unsigned OptLevel = 2);

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  unsigned getOptLevel() const { return Impl.getOptLevel(); }
};

ImmutablePass *createXmainOptLevelWrapperPass(unsigned OptLevel = 2);

} // namespace llvm

#endif
