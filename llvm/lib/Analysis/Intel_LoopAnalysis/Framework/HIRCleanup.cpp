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

#include "HIRCleanup.h"

#include "llvm/Analysis/LoopInfo.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

#include "HIRCreation.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-cleanup"

HLNode *HIRCleanup::findHIRHook(const BasicBlock *BB) const {
  auto It = LoopLatchHooks.find(BB);

  if (It != LoopLatchHooks.end()) {
    return It->second;
  }
  auto Iter = HIRC.Labels.find(BB);

  if (Iter != HIRC.Labels.end()) {
    return Iter->second;
  }

  llvm_unreachable("Could not find basic block's label!");
  return nullptr;
}

void HIRCleanup::eliminateRedundantGotos() {

  for (auto I = HIRC.Gotos.begin(), E = HIRC.Gotos.end(); I != E; ++I) {
    auto Goto = *I;

    HLLabel *LabelSuccessor =
        dyn_cast_or_null<HLLabel>(HNU.getLexicalControlFlowSuccessor(Goto));

    auto TargetBB = Goto->getTargetBBlock();

    // Goto is redundant, if
    // 1) Its lexical successor is the same as its target.
    // Or
    // 2) It has no lexical successor and jumps to region exit.
    if ((LabelSuccessor && (TargetBB == LabelSuccessor->getSrcBBlock())) ||
        (!LabelSuccessor &&
         (TargetBB == Goto->getParentRegion()->getSuccBBlock()))) {
      HLNodeUtils::erase(Goto);
    } else {
      // Link Goto to its HLLabel target, if available.
      auto It = HIRC.Labels.find(TargetBB);

      if (It != HIRC.Labels.end()) {
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
} // namespace

void HIRCleanup::eliminateRedundantLabels() {
  Loop *Lp = nullptr;

  for (auto I = HIRC.Labels.begin(), E = HIRC.Labels.end(); I != E; ++I) {
    auto LabelBB = I->first;
    auto Label = I->second;

    auto It = std::lower_bound(RequiredLabels.begin(), RequiredLabels.end(),
                               Label, LabelNumberCompareLess());

    // This HLLabel is redundant as no HLGoto is pointing to it.
    if ((It == RequiredLabels.end()) ||
        !LabelNumberCompareEqual()(*It, Label)) {

      // This label represents loop latch bblock. We need to store the successor
      // as it is used by LoopFomation pass to find loop's bottom test.
      if ((Lp = LI.getLoopFor(LabelBB)) && (Lp->getLoopLatch() == LabelBB)) {
        auto LexSuccessor = HNU.getLexicalControlFlowSuccessor(Label);

        HLContainerTy::iterator It(Label);
        assert(LexSuccessor && (&*std::next(It) == LexSuccessor) &&
               "Unexpected loop latch label successor!");

        LoopLatchHooks[LabelBB] = LexSuccessor;
      }

      HLNodeUtils::erase(Label);
    }
  }
}

void HIRCleanup::run() {
  eliminateRedundantGotos();

  // Sort RequiredLabels vector before the query phase.
  std::sort(RequiredLabels.begin(), RequiredLabels.end(),
            LabelNumberCompareLess());
  RequiredLabels.erase(std::unique(RequiredLabels.begin(), RequiredLabels.end(),
                                   LabelNumberCompareEqual()),
                       RequiredLabels.end());

  eliminateRedundantLabels();
}
