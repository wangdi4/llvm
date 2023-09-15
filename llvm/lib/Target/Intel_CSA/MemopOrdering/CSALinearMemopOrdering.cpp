//===-- CSALinearMemopOrdering.cpp - Linear memop ordering pass -*- C++ -*-===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
///===---------------------------------------------------------------------===//
/// \file
///
/// This file implements the linear memop ordering pass.
///
///===---------------------------------------------------------------------===//

#include "CSALinearMemopOrdering.h"

#include "llvm/IR/CFG.h"
#include "llvm/InitializePasses.h"

using namespace llvm;

#define DEBUG_TYPE "csa-linear-memop-ordering"

char CSALinearMemopOrdering::ID = 0;
constexpr auto PASS_DESC        = "CSA: Linear memory operation ordering";

INITIALIZE_PASS_BEGIN(CSALinearMemopOrdering, DEBUG_TYPE, PASS_DESC, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)
INITIALIZE_PASS_END(CSALinearMemopOrdering, DEBUG_TYPE, PASS_DESC, false, false)

StringRef CSALinearMemopOrdering::getPassName() const { return PASS_DESC; }
void CSALinearMemopOrdering::getAnalysisUsage(AnalysisUsage& AU) const {
  AU.setPreservesCFG();
  return CSAMemopOrderingBase::getAnalysisUsage(AU);
}

void CSALinearMemopOrdering::order(Function &F) {
  BBConMap ConMap;
  for (BasicBlock &BB : F)
    ConMap.insert({&BB, orderBB(BB)});
  connectBBs(ConMap);
  removeUselessPHIs(ConMap);
}

auto CSALinearMemopOrdering::orderBB(BasicBlock &BB) -> BBConnections {
  BBConnections BBCon;
  Value *CurOrd;
  if (pred_empty(&BB)) {
    CurOrd = MemEntry;
  } else {
    BBCon.PHI = createPHI(&BB);
    CurOrd    = BBCon.PHI;
  }

  for (Instruction &I : BB) {
    if (needsOrderingEdges(I))
      CurOrd = createOrderingEdges(&I, CurOrd);
  }

  BBCon.EndVal = CurOrd;

  return BBCon;
}

void CSALinearMemopOrdering::connectBBs(BBConMap &ConMap) {
  for (const auto &ConPair : ConMap) {
    for (BasicBlock *const Pred : predecessors(ConPair.first)) {
      ConPair.second.PHI->addIncoming(ConMap[Pred].EndVal, Pred);
    }
  }
}

void CSALinearMemopOrdering::removeUselessPHIs(BBConMap &ConMap) {
  SmallVector<BBConnections *, 4> ConsToCheck;
  for (auto &ConPair : ConMap)
    ConsToCheck.push_back(&ConPair.second);

  while (not ConsToCheck.empty()) {
    BBConnections &CurCon = *ConsToCheck.back();
    ConsToCheck.pop_back();
    if (not CurCon.PHI)
      continue;

    Value *const ComVal = CurCon.PHI->hasConstantValue();
    if (not ComVal)
      continue;

    for (User *const U : CurCon.PHI->users()) {
      PHINode *const UPHI = dyn_cast<PHINode>(U);
      if (not UPHI)
        continue;
      ConsToCheck.push_back(&ConMap[UPHI->getParent()]);
    }

    CurCon.PHI->replaceAllUsesWith(ComVal);
    CurCon.PHI->eraseFromParent();
    CurCon.PHI = nullptr;
  }
}
