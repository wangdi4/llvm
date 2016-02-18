//===------- LoopFormation.cpp - Creates HIR Loops ------------------------===//
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
// This file implements the LoopFormation pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Instructions.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Analysis/Intel_LoopAnalysis/LoopFormation.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRCreation.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRCleanup.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-loop-formation"

INITIALIZE_PASS_BEGIN(LoopFormation, "hir-loop-formation", "HIR Loop Formation",
                      false, true)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass);
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRCreation)
INITIALIZE_PASS_DEPENDENCY(HIRCleanup)
INITIALIZE_PASS_END(LoopFormation, "hir-loop-formation", "HIR Loop Formation",
                    false, true)

char LoopFormation::ID = 0;

FunctionPass *llvm::createLoopFormationPass() { return new LoopFormation(); }

LoopFormation::LoopFormation() : FunctionPass(ID) {
  initializeLoopFormationPass(*PassRegistry::getPassRegistry());
}

void LoopFormation::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<LoopInfoWrapperPass>();
  AU.addRequiredTransitive<ScalarEvolutionWrapperPass>();
  AU.addRequiredTransitive<HIRCreation>();
  AU.addRequired<HIRCleanup>();
}

namespace {
struct LoopCompareLess {
  bool operator()(const LoopFormation::LoopPairTy &LP1,
                  const LoopFormation::LoopPairTy &LP2) {
    return std::less<const Loop *>()(LP1.first, LP2.first);
  }
};

struct LoopCompareEqual {
  bool operator()(const LoopFormation::LoopPairTy &LP1,
                  const LoopFormation::LoopPairTy &LP2) {
    return LP1.first == LP2.first;
  }
};
}

HLLoop *LoopFormation::findOrInsertHLLoopImpl(const Loop *Lp, HLLoop *HLoop,
                                              bool Insert) {
  LoopPairTy LoopPair(Lp, HLoop);

  if (Loops.empty()) {
    if (Insert) {
      Loops.push_back(LoopPair);
    }
    return nullptr;
  }

  auto I =
      std::lower_bound(Loops.begin(), Loops.end(), LoopPair, LoopCompareLess());

  if (I != Loops.end() && LoopCompareEqual()(*I, LoopPair)) {
    if (Insert) {
      assert(false && "Multiple insertions not expected!");
    }
    return I->second;
  }

  if (Insert) {
    Loops.insert(I, LoopPair);
  }

  return nullptr;
}

void LoopFormation::insertHLLoop(const Loop *Lp, HLLoop *HLoop) {
  findOrInsertHLLoopImpl(Lp, HLoop, true);
}

HLLoop *LoopFormation::findHLLoop(const Loop *Lp) {
  return findOrInsertHLLoopImpl(Lp, nullptr, false);
}

const PHINode *LoopFormation::findIVDefInHeader(const Loop *Lp,
                                                const Instruction *Inst) const {

  // Is this a phi node in the loop header?
  if (Inst->getParent() == Lp->getHeader()) {
    if (auto Phi = dyn_cast<PHINode>(Inst)) {
      return Phi;
    }
  }

  for (auto I = Inst->op_begin(), E = Inst->op_end(); I != E; ++I) {
    // Not looking at scev of the IV since in some cases it is unknown even for
    // do loops.
    //
    // Example-
    //
    // for (i = 101; i > 1; i = i/2) {
    // ...
    // }
    if (auto OPInst = dyn_cast<Instruction>(I)) {
      // Instruction lies outside the loop.
      if (!Lp->contains(LI->getLoopFor(OPInst->getParent()))) {
        continue;
      }

      auto IVNode = findIVDefInHeader(Lp, OPInst);

      if (IVNode) {
        return IVNode;
      }
    }
  }

  return nullptr;
}

void LoopFormation::setIVType(HLLoop *HLoop) const {
  Value *Cond;
  auto Lp = HLoop->getLLVMLoop();
  auto Latch = Lp->getLoopLatch();

  assert(Latch && "Loop doesn't have a latch!");

  auto Term = Latch->getTerminator();

  if (auto BottomTest = dyn_cast<BranchInst>(Term)) {
    assert(BottomTest->isConditional() &&
           "Loop bottom test is not a conditional branch!");
    Cond = BottomTest->getCondition();
  } else if (auto BottomTest = dyn_cast<SwitchInst>(Term)) {
    Cond = BottomTest->getCondition();
  } else {
    assert(false && "Cannot handle loop bottom test!");
  }

  assert(isa<Instruction>(Cond) &&
         "Loop exit condition is not an instruction!");

  auto IVNode = findIVDefInHeader(Lp, cast<Instruction>(Cond));
  assert(IVNode && "Could not find loop IV!");

  HLoop->setIVType(IVNode->getType());
}

void LoopFormation::formLoops() {

  // Traverse RequiredLabels set computed by HIRCleanup phase to form loops.
  for (auto I = HIRC->getRequiredLabels().begin(),
            E = HIRC->getRequiredLabels().end();
       I != E; ++I) {
    BasicBlock *HeaderBB = (*I)->getSrcBBlock();

    if (!LI->isLoopHeader(HeaderBB)) {
      continue;
    }

    // Found a loop
    Loop *Lp = LI->getLoopFor(HeaderBB);

    // Find HIR hook for the loop latch.
    auto LatchHook = HIRC->findHIRHook(Lp->getLoopLatch());

    assert(((*I)->getParent() == LatchHook->getParent()) &&
           "Wrong lexical links built!");

    HLContainerTy::iterator LabelIter(*I);
    HLContainerTy::iterator BottomTestIter(LatchHook);

    // Look for the bottom test.
    while (!isa<HLIf>(BottomTestIter)) {
      BottomTestIter = std::next(BottomTestIter);
    }

    // Create a new loop and move its children inside.
    // TODO: Add code to identify ztt, set IsDoWhile flag and populate
    // preheader/postexit.
    // Notes:
    // - Do while loops have SCEV trip count of the form (-C + (C umax/smax n))
    // which can be used to identify them.
    // - Ztt predicate inversion is required if the loop is in else case.
    // - It is possible that some inner loop's ztt has been hoisted outside the
    // loopnest.
    // - It is possible that ztt contains other nodes.
    HLLoop *HLoop = HLNodeUtils::createHLLoop(Lp);
    setIVType(HLoop);
    HLNodeUtils::moveAsFirstChildren(HLoop, std::next(LabelIter),
                                     BottomTestIter);

    // Hook loop into HIR.
    HLNodeUtils::insertBefore(&*LabelIter, HLoop);

    // Remove label and bottom test.
    // Can bottom test contain anything else??? Should probably assert on it.
    HLNodeUtils::erase(&*LabelIter);
    HLNodeUtils::erase(&*BottomTestIter);

    insertHLLoop(Lp, HLoop);
  }
}

bool LoopFormation::runOnFunction(Function &F) {
  this->Func = &F;

  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  HIR = &getAnalysis<HIRCreation>();
  HIRC = &getAnalysis<HIRCleanup>();

  formLoops();

  return false;
}

void LoopFormation::releaseMemory() { Loops.clear(); }

void LoopFormation::print(raw_ostream &OS, const Module *M) const {
  HIR->print(OS, M);
}

void LoopFormation::verifyAnalysis() const {
  /// TODO: implement later
}
