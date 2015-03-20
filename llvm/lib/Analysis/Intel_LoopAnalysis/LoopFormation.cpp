//===------- LoopFormation.cpp - Creates HIR Loops ------*- C++ -*---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the LoopFormation pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_LoopAnalysis/LoopFormation.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRCreation.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-loops"

static RegisterPass<LoopFormation> X("hir-loops", "HIR Loop Formation", false,
                                     true);

char LoopFormation::ID = 0;

FunctionPass *llvm::createLoopFormationPass() { return new LoopFormation(); }

LoopFormation::LoopFormation() : FunctionPass(ID) {}

void LoopFormation::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<LoopInfo>();
  AU.addRequiredTransitive<HIRCreation>();
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

void LoopFormation::formLoops(HLRegion *Reg) {

  BasicBlock *HeaderBB;
  Loop *Lp;
  assert(Reg->hasChildren() && "Empty region encountered!");
  assert(isa<HLLabel>(Reg->getFirstChild()) && "First child is not a label!");

  HeaderBB = cast<HLLabel>(Reg->getFirstChild())->getSrcBBlock();

  assert(LI->isLoopHeader(HeaderBB) && "First child is not a loop header!");
  assert(isa<HLIf>(Reg->getLastChild()) && "Last child is not an if!");

  /// Remove label and bottom test
  HLNodeUtils::erase(Reg->getFirstChild());
  HLNodeUtils::erase(Reg->getLastChild());

  Lp = LI->getLoopFor(HeaderBB);
  /// Create a new loop and move region children into loop children.
  /// TODO: Add code to identify ztt and set IsDoWhile flag accordingly.
  HLLoop *HLoop = HLNodeUtils::createHLLoop(Lp);
  HLNodeUtils::moveAsFirstChildren(HLoop, Reg->child_begin(), Reg->child_end());

  /// Insert loop in region.
  HLNodeUtils::insertAsFirstChild(Reg, HLoop);

  insertHLLoop(Lp, HLoop);
}

bool LoopFormation::runOnFunction(Function &F) {
  this->Func = &F;

  LI = &getAnalysis<LoopInfo>();
  auto HIR = &getAnalysis<HIRCreation>();

  for (auto I = HIR->begin(), E = HIR->end(); I != E; I++) {
    formLoops(cast<HLRegion>(I));
  }

  return false;
}

void LoopFormation::releaseMemory() { Loops.clear(); }

void LoopFormation::print(raw_ostream &OS, const Module *M) const {
  /// TODO: implement later
}

void LoopFormation::verifyAnalysis() const {
  /// TODO: implement later
}
