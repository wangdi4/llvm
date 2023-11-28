//===- Intel_ForcedCMOVGeneration.cpp - CMOV generation for special cases -===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SCALAR_FORCEDCMOVGENERATION_H
#define LLVM_TRANSFORMS_SCALAR_FORCEDCMOVGENERATION_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include <atomic>
#include <memory>
#include <unordered_set>

namespace llvm {

namespace intel_forcedcmovgen {

class ForcedCMOVGeneration {
  struct IfThenConstruct {
    BasicBlock *ThenBlock;
    BasicBlock *EndBlock;
    // ThenIsTrue=true means 'true' cond in IfBlock leads to conditional block.
    // (i.e) br i1 %cmp14, label %if.then, label %if.end
    // ThenIsTrue=false means 'false' cond in IfBlock leads to conditional block.
    // (i.e) br i1 %cmp14, label %if.end, label %if.else
    bool ThenIsTrue;
  };
  Function &F;
public:
  ForcedCMOVGeneration(Function &F)
      : F(F) {};

  /// Main entry point to the optimization.
  bool run();

private:
  bool IsACandidateBasicBlock(BasicBlock &B, IfThenConstruct &Construct);
  bool HasSingleStore(BasicBlock *B);
  bool IsProfitableForCMOV(BasicBlock *Entry);
  void AddSelectInst(BasicBlock &B, IfThenConstruct &Construct);
};
} // end namespace intel_forcedcmovgen

class ForcedCMOVGenerationPass : public PassInfoMixin<ForcedCMOVGenerationPass> {
public:
  // Entry point for forced CMOV generation.
  bool runImpl(Function &F);
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

class ForcedCMOVGenerationLegacyPass : public FunctionPass {
  ForcedCMOVGenerationPass Impl;

public:
  static char ID; // Pass identification, replacement for typeid

  ForcedCMOVGenerationLegacyPass();

  bool runOnFunction(Function &F) override;
  
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addPreserved<ScalarEvolutionWrapperPass>();
    AU.addPreserved<GlobalsAAWrapperPass>();
  }
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_FORCEDCMOVGENERATION_H
