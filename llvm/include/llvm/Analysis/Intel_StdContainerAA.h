//===- StdContainerAA.h - Std Container Alias Analysis -------*- C++ -*-===//
// Copyright (C) 2015-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
/// \file
/// This is the interface for a metadata-based alias analysis for STL
//  templates.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_STDCONTAINERAA_H
#define LLVM_ANALYSIS_STDCONTAINERAA_H

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

namespace llvm {

class StdContainerAAResult : public AAResultBase {
public:
  explicit StdContainerAAResult() : AAResultBase() {}
  StdContainerAAResult(StdContainerAAResult &&Arg)
      : AAResultBase(std::move(Arg)) {}

  /// Handle invalidation events from the new pass manager.
  ///
  /// By definition, this result is stateless and so remains valid.
  bool invalidate(Function &, const PreservedAnalyses &,
                  FunctionAnalysisManager::Invalidator &) {
    return false;
  }

  AliasResult alias(const MemoryLocation &LocA, const MemoryLocation &LocB,
                    AAQueryInfo &AAQI);
  ModRefInfo getModRefInfo(const CallBase *Call, const MemoryLocation &Loc,
                           AAQueryInfo &AAQI);
  ModRefInfo getModRefInfo(const CallBase *Call1, const CallBase *Call2,
                           AAQueryInfo &AAQI);

private:
  bool mayAliasInStdContainer(MDNode *M1, MDNode *M2);
};

class StdContainerAA : public AnalysisInfoMixin<StdContainerAA> {
  static AnalysisKey Key; 
  friend AnalysisInfoMixin<StdContainerAA>;
  static char PassID;

public:
  typedef StdContainerAAResult Result;

  StdContainerAAResult run(Function &F, AnalysisManager<Function> &AM);
};

class StdContainerAAWrapperPass : public ImmutablePass {
  std::unique_ptr<StdContainerAAResult> Result;

public:
  static char ID;

  StdContainerAAWrapperPass();

  StdContainerAAResult &getResult() { return *Result; }
  const StdContainerAAResult &getResult() const { return *Result; }

  bool doInitialization(Module &M) override;
  bool doFinalization(Module &M) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

ImmutablePass *createStdContainerAAWrapperPass();
}

#endif
