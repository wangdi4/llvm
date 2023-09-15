#if INTEL_FEATURE_SHARED_SW_ADVANCED
//===---- HIRCleanup.cpp - Clean up redundant HIR Nodes -------------------===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
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
#include "llvm/Analysis/ValueTracking.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRRegionIdentification.h"
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

void HIRCleanup::eliminateRedundantLabels() {
  Loop *Lp = nullptr;

  for (auto I = HIRC.Labels.begin(), E = HIRC.Labels.end(); I != E; ++I) {
    auto LabelBB = I->first;
    auto Label = I->second;

    // This HLLabel is redundant as no HLGoto is pointing to it.
    if (!RequiredLabels.count(Label)) {

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

std::optional<bool> HIRCleanup::isImpliedUsingSCEVAnalysis(Value *Cond) {

  auto *ICmp = dyn_cast<ICmpInst>(Cond);

  // Restrict to EQ predicates for compile time savings.
  if (!ICmp || (ICmp->getPredicate() != CmpInst::ICMP_EQ)) {
    return std::nullopt;
  }

  auto &SE = HIRC.RI.getScopedSE();

  auto *LHS = SE.getSCEV(ICmp->getOperand(0));
  auto *RHS = SE.getSCEV(ICmp->getOperand(1));

  return SE.evaluatePredicateAt(CmpInst::ICMP_EQ, LHS, RHS, ICmp);
}

// Replaces HLIfs which can be proven to be true/false using ValueTracking's
// isImpliedByDomCondition() functionality, by their then/else bodies.
// TODO: Move this logic to SimplifyCFG pass when it is capable of using and
// maintaining DominatorTree.
void HIRCleanup::eliminateRedundantIfs() {

  auto &DL = HNU.getDataLayout();
  auto *DT = &HIRC.DT;

  bool IsFunctionLevelRegion =
      !HIRC.Ifs.empty() &&
      HIRC.Ifs.begin()->first->getParentRegion()->isFunctionLevel();

  for (auto &IfBlockPair : HIRC.Ifs) {
    auto *SrcBB = IfBlockPair.second;
    auto *BI = cast<BranchInst>(SrcBB->getTerminator());

    auto *Cond = BI->getCondition();

    // Do not optimize undef conditions using dominating undefs as it
    // doesn't make much sense and breaks existing lit tests.
    if (isa<UndefValue>(Cond)) {
      continue;
    }

    auto *Lp = LI.getLoopFor(SrcBB);
    // Do not optimize away loop latches.
    if (Lp && (Lp->getLoopLatch() == SrcBB)) {
      continue;
    }

    auto *If = IfBlockPair.first;
    std::optional<bool> Res = isImpliedByDomCondition(Cond, BI, DL, DT);
    bool ImpliedCond = false;

    if (!Res) {

      // Only enable ScalarEvolution's implied logic for function level regions
      // to save compile time as SCEV analysis is expensive.
      if (!IsFunctionLevelRegion) {
        continue;
      }

      if (std::optional<bool> SubRes = isImpliedUsingSCEVAnalysis(Cond)) {
        ImpliedCond = *SubRes;
      } else {
        continue;
      }
    } else {
      ImpliedCond = *Res;
    }

    bool ReplaceWithThenCase = ImpliedCond;

    const HLNode *LastChild =
        ReplaceWithThenCase ? If->getLastThenChild() : If->getLastElseChild();

    auto *LastGoto = dyn_cast_or_null<HLGoto>(LastChild);
    // If optimizing HLIf produces unconditional goto, verifier may complain
    // about dead nodes after goto. It is non-trivial to clean up the nodes at
    // this stage as we haven't formed loops out of loop header labels. It is
    // better to simply give up.
    if (LastGoto && !HLNodeUtils::isLexicalLastChildOfParent(If)) {
      // If the next node is target of goto, eliminateRedundantGotos() will
      // handle it.
      auto *LabelSuccessor = dyn_cast<HLLabel>(&*std::next(If->getIterator()));

      if (!LabelSuccessor || (LastGoto->getTargetLabel() != LabelSuccessor)) {
        continue;
      }
    }

    OptimizedRegions.insert(If->getParentRegion());

    HLNodeUtils::replaceNodeWithBody(If, ReplaceWithThenCase);
  }
}

void HIRCleanup::run() {

  // Setup goto target labels so that we can eliminate any redundant gotos.
  // The call to eliminateRedundantGotos uses the target labels.
  for (auto *Goto : HIRC.Gotos) {
    auto TargetBB = Goto->getTargetBBlock();
    auto It = HIRC.Labels.find(TargetBB);
    if (It != HIRC.Labels.end())
      Goto->setTargetLabel(It->second);
  }

  // Setting target labels for gotos helps eliminate more ifs.
  eliminateRedundantIfs();

  HLNodeUtils::eliminateRedundantGotos(HIRC.Gotos, RequiredLabels);
  eliminateRedundantLabels();
}
#endif // INTEL_FEATURE_SHARED_SW_ADVANCED
