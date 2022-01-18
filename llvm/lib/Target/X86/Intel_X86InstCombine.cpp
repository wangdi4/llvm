//===--------- Intel_X86InstCombine.cpp - X86 Instruction Combine ---------===//
//
// Copyright (C) 2019-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass is mainly anti middle end optimization which is not really good
// for X86 target, at the same time, it is not a good choice to disable it
// at middle end. Just put these optimizations here.
//
//===----------------------------------------------------------------------===//

#include "X86.h"
#include "X86TargetMachine.h"
#include "X86Subtarget.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/CodeGen/TargetPassConfig.h"

using namespace llvm;
using namespace llvm::PatternMatch;

#define DEBUG_TYPE "x86-inst-combine"

namespace {

class X86InstCombine : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid

  X86InstCombine() : FunctionPass(ID) {
    initializeX86InstCombinePass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetTransformInfoWrapperPass>();
    AU.setPreservesCFG();
    AU.addRequired<AssumptionCacheTracker>();
    AU.addRequired<DominatorTreeWrapperPass>();
    FunctionPass::getAnalysisUsage(AU);
  }

  bool doInitialization(Module &M) override {
    auto *TPC = getAnalysisIfAvailable<TargetPassConfig>();
    if (!TPC)
      return false;

    TM = &TPC->getTM<X86TargetMachine>();
    return false;
  }


  bool runOnFunction(Function &F) override;

private:
  bool replaceOrToAdd(Instruction &I);

  X86TargetMachine *TM = nullptr;
  const X86Subtarget *ST = nullptr;
  DominatorTree *DT = nullptr;
  TargetTransformInfo *TTI = nullptr;
  AssumptionCache *AC = nullptr;
};

} // end anonymous namespace

char X86InstCombine::ID = 0;
char &llvm::X86InstCombineID = X86InstCombine::ID;

INITIALIZE_PASS_BEGIN(X86InstCombine, DEBUG_TYPE,
                    "Instruction Combine for X86 Target", false, false)
INITIALIZE_PASS_DEPENDENCY(AssumptionCacheTracker)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_END(X86InstCombine, DEBUG_TYPE,
                    "Instruction Combine for X86 Target", false, false)

FunctionPass *llvm::createX86InstCombinePass() {
  return new X86InstCombine();
}

static void replaceValue(Value &Old, Value &New) {
  Old.replaceAllUsesWith(&New);
  New.takeName(&Old);
}

static bool isBitwiseInst(Value *V) {
  if (auto BinOper = dyn_cast<BinaryOperator>(V))
    if (BinOper->getOpcode() == Instruction::Or ||
        BinOper->getOpcode() == Instruction::And ||
        BinOper->getOpcode() == Instruction::Xor)
      return true;

  return false;
}

bool X86InstCombine::replaceOrToAdd(Instruction &I) {
  Value *LHS = I.getOperand(0), *RHS = I.getOperand(1);
  const DataLayout &DL = I.getModule()->getDataLayout();

  // Check if it is profitable to replace:
  // For avx512, 'Or' with other bitwise instructions may can be
  // transformed to VPTERNLOG.
  if ((ST && ST->hasAVX512()) && I.getType()->isVectorTy()) {
    for (auto User : I.users())
      if (isBitwiseInst(User))
        return false;

    for (Value *V : I.operands())
      if (isBitwiseInst(V))
          return false;
  }

  // Check if 'or' can be transformed to 'add'.
  if (!haveNoCommonBitsSet(LHS, RHS, DL, AC, &I, DT))
    return false;

  IRBuilder<> Builder(&I);
  auto Add = Builder.CreateAdd(LHS, RHS);
  replaceValue(I, *Add);

  return true;
}

bool X86InstCombine::runOnFunction(Function &F) {
  if (skipFunction(F))
    return false;

  TTI = &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  if (TM)
    ST = &TM->getSubtarget<X86Subtarget>(F);
  AC = &getAnalysis<AssumptionCacheTracker>().getAssumptionCache(F);

  bool MadeChange = false;
  for (Instruction &I : make_early_inc_range(instructions(F))) {
    switch (I.getOpcode()) {
      case Instruction::Or:
        MadeChange |= replaceOrToAdd(I);
        break;
    }
  }

  // We're done with transforms, so remove dead instructions.
  if (MadeChange)
    for (BasicBlock &BB : F)
      SimplifyInstructionsInBlock(&BB);

  return MadeChange;
}

