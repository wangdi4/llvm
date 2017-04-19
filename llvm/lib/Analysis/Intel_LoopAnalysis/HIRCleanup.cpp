//===---- HIRCleanup.cpp - Clean up redundant HIR Nodes -------------------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HIR cleanup pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/LoopInfo.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRCleanup.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRCreation.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-cleanup"

INITIALIZE_PASS_BEGIN(HIRCleanup, "hir-cleanup", "HIR Cleanup", false, true)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRCreation)
INITIALIZE_PASS_END(HIRCleanup, "hir-cleanup", "HIR Cleanup", false, true)

char HIRCleanup::ID = 0;

FunctionPass *llvm::createHIRCleanupPass() { return new HIRCleanup(); }

HIRCleanup::HIRCleanup() : FunctionPass(ID) {
  initializeHIRCleanupPass(*PassRegistry::getPassRegistry());
}

void HIRCleanup::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<LoopInfoWrapperPass>();
  AU.addRequiredTransitive<HIRCreation>();
}

HLNode *HIRCleanup::findHIRHook(const BasicBlock *BB) const {
  auto It = LoopLatchHooks.find(BB);

  if (It != LoopLatchHooks.end()) {
    return It->second;
  }
  auto Iter = HIR->Labels.find(BB);

  if (Iter != HIR->Labels.end()) {
    return Iter->second;
  }

  llvm_unreachable("Could not find basic block's label!");
  return nullptr;
}

void HIRCleanup::eliminateRedundantGotos() {

  for (auto I = HIR->Gotos.begin(), E = HIR->Gotos.end(); I != E; ++I) {
    auto Goto = *I;

    HLLabel *LabelSuccessor = dyn_cast_or_null<HLLabel>(
        HIR->getHLNodeUtils().getLexicalControlFlowSuccessor(Goto));

    auto TargetBB = Goto->getTargetBBlock();

    // Goto is redundant, if
    // 1) Its lexical successor is the same as its target.
    // Or
    // 2) It has no lexical successor and jumps to region exit.
    if ((LabelSuccessor && (TargetBB == LabelSuccessor->getSrcBBlock())) ||
        (!LabelSuccessor &&
         (TargetBB == Goto->getParentRegion()->getSuccBBlock()))) {
      HIR->getHLNodeUtils().erase(Goto);
    } else {
      // Link Goto to its HLLabel target, if available.
      auto It = HIR->Labels.find(TargetBB);

      if (It != HIR->Labels.end()) {
        Goto->setTargetLabel(It->second);
        RequiredLabels.push_back(It->second);
      }
    }
  }
}

namespace {
// Used to keep RequiredLabels sorted by HLLabel number.
struct LabelNumberCompareLess {
  bool operator()(const HLLabel *L1, const HLLabel *L2) {
    return L1->getNumber() < L2->getNumber();
  }
};

// Used to keep RequiredLabels unique by HLLabel number.
struct LabelNumberCompareEqual {
  bool operator()(const HLLabel *L1, const HLLabel *L2) {
    return L1->getNumber() == L2->getNumber();
  }
};
}

void HIRCleanup::eliminateRedundantLabels() {
  Loop *Lp = nullptr;

  for (auto I = HIR->Labels.begin(), E = HIR->Labels.end(); I != E; ++I) {
    auto LabelBB = I->first;
    auto Label = I->second;

    auto It = std::lower_bound(RequiredLabels.begin(), RequiredLabels.end(),
                               Label, LabelNumberCompareLess());

    // This HLLabel is redundant as no HLGoto is pointing to it.
    if ((It == RequiredLabels.end()) ||
        !LabelNumberCompareEqual()(*It, Label)) {

      // This label represents loop latch bblock. We need to store the successor
      // as it is used by LoopFomation pass to find loop's bottom test.
      if ((Lp = LI->getLoopFor(LabelBB)) && (Lp->getLoopLatch() == LabelBB)) {
        auto LexSuccessor =
            HIR->getHLNodeUtils().getLexicalControlFlowSuccessor(Label);

        HLContainerTy::iterator It(Label);
        assert(LexSuccessor && (&*std::next(It) == LexSuccessor) &&
               "Unexpected loop latch label successor!");

        LoopLatchHooks[LabelBB] = LexSuccessor;
      }

      HIR->getHLNodeUtils().erase(Label);
    }
  }
}

bool HIRCleanup::runOnFunction(Function &F) {
  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  HIR = &getAnalysis<HIRCreation>();

  eliminateRedundantGotos();

  // Sort RequiredLabels vector before the query phase.
  std::sort(RequiredLabels.begin(), RequiredLabels.end(),
            LabelNumberCompareLess());
  RequiredLabels.erase(std::unique(RequiredLabels.begin(), RequiredLabels.end(),
                                   LabelNumberCompareEqual()),
                       RequiredLabels.end());

  eliminateRedundantLabels();

  return false;
}

void HIRCleanup::releaseMemory() {
  LoopLatchHooks.clear();
  RequiredLabels.clear();
}

void HIRCleanup::print(raw_ostream &OS, const Module *M) const {
  HIR->print(OS, M);
}

void HIRCleanup::verifyAnalysis() const {
  // TODO: Implement later
}
