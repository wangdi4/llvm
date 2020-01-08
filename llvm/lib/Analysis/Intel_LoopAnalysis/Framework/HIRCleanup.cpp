//===---- HIRCleanup.cpp - Clean up redundant HIR Nodes -------------------===//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
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
    auto TargetBB = Goto->getTargetBBlock();

    HLNode *CurNode = Goto;

    // We either remove Goto as redundant by looking at its control flow
    // successors or link it to its target HLLabel.
    while (1) {
      auto *Successor = HNU.getLexicalControlFlowSuccessor(CurNode);

      bool Erase = false, CheckNext = false;

      if (!Successor) {
        if (TargetBB == Goto->getParentRegion()->getSuccBBlock()) {
          // Goto is redundant if it has no lexical successor and jumps to
          // region exit.
          Erase = true;
        }
      } else if (auto LabelSuccessor = dyn_cast<HLLabel>(Successor)) {

        if (TargetBB == LabelSuccessor->getSrcBBlock()) {
          // Goto is redundant if its lexical successor is the same as its
          // target.
          Erase = true;
        } else {
          // If successor is a label, goto can still be redundant based on
          // label's successor.
          // Example-
          // goto L1; << This goto is redundant.
          // L2:
          // L1:
          CurNode = Successor;
          CheckNext = true;
        }
      } else if (auto GotoSuccessor = dyn_cast<HLGoto>(Successor)) {
        // If the successor is a goto which also jumps to the same bblock,
        // this goto is redundant.
        // Example-
        // goto L1; << This goto is redundant.
        // L2:
        // goto L1;
        auto SuccTargetBB = GotoSuccessor->getTargetBBlock();

        if (!SuccTargetBB) {
          SuccTargetBB = GotoSuccessor->getTargetLabel()->getSrcBBlock();
        }

        if (SuccTargetBB == TargetBB) {
          Erase = true;
        }
      }

      if (Erase) {
        HLNodeUtils::erase(Goto);
        break;

      } else if (!CheckNext) {
        // Link Goto to its HLLabel target, if available.
        auto It = HIRC.Labels.find(TargetBB);

        if (It != HIRC.Labels.end()) {
          Goto->setTargetLabel(It->second);
          RequiredLabels.push_back(It->second);
        }

        break;
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
