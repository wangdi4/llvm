//===- Intel_TransformSinAndCosCalls.h - Transform sin and cos calls -===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------===//
//
// This pass transforms calls to sin and cos to calls to sinpi, cospi, or
// sincospi.
//
//===----------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SCALAR_INTEL_TRANSFORM_SIN_AND_COS_CALLS_H
#define LLVM_TRANSFORMS_SCALAR_INTEL_TRANSFORM_SIN_AND_COS_CALLS_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include <atomic>
#include <memory>
#include <unordered_set>

namespace llvm {

namespace intel_transform_sin_cos_calls {

class TransformSinAndCosCalls {
  Function &F;
  DominatorTree &DT;
  LoopInfo &LI;
  TargetLibraryInfo &TLI;

public:
  TransformSinAndCosCalls(Function &F, DominatorTree &DT, LoopInfo &LI,
      TargetLibraryInfo &TLI)
    : F(F), DT(DT), LI(LI), TLI(TLI) {};

  /// Main entry point to the optimization.
  bool run();

private:
};

} // end namespace intel_transform_sin_cos_calls

/*
 * New Pass Manager
 */

class TransformSinAndCosCallsPass :
  public PassInfoMixin<TransformSinAndCosCallsPass> {
public:
  bool runImpl(Function &F,
               DominatorTree &DT,
               LoopInfo &LI,
               TargetLibraryInfo &TLI);
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

/*
 * Legacy (Old) Pass Manager
 */

class TransformSinAndCosCallsLegacyPass : public FunctionPass {
  TransformSinAndCosCallsPass Impl;

public:
  static char ID; // Pass identification, replacement for typeid

  TransformSinAndCosCallsLegacyPass();

  bool runOnFunction(Function &F) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.setPreservesAll();
  }
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_INTEL_TRANSFORM_SIN_AND_COS_CALLS_H
