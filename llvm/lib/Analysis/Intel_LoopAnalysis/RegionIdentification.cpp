//===- RegionIdentification.cpp - Identifies HIR Regions *- C++ -*---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HIR Region Identification pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"

#include "llvm/IR/Dominators.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/IR/Intel_LoopIR/CanonExpr.h"

#include "llvm/Analysis/Intel_LoopAnalysis/RegionIdentification.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-region"

INITIALIZE_PASS_BEGIN(RegionIdentification, "hir-region",
                      "HIR Region Identification", false, true)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfo)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolution)
INITIALIZE_PASS_END(RegionIdentification, "hir-region",
                    "HIR Region Identification", false, true)

char RegionIdentification::ID = 0;

FunctionPass *llvm::createRegionIdentificationPass() {
  return new RegionIdentification();
}

RegionIdentification::IRRegion::IRRegion(
    BasicBlock *Entry, RegionIdentification::RegionBBlocksTy BBlocks)
    : EntryBB(Entry), BasicBlocks(BBlocks) {}

RegionIdentification::IRRegion::IRRegion(const IRRegion &Reg)
    : EntryBB(Reg.EntryBB), BasicBlocks(Reg.BasicBlocks) {}

RegionIdentification::IRRegion::~IRRegion() {}

RegionIdentification::RegionIdentification() : FunctionPass(ID) {
  initializeRegionIdentificationPass(*PassRegistry::getPassRegistry());
}

void RegionIdentification::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<DominatorTreeWrapperPass>();
  AU.addRequiredTransitive<LoopInfo>();
  AU.addRequiredTransitive<ScalarEvolution>();
}

bool RegionIdentification::isSelfGenerable(const Loop &Lp,
                                           unsigned LoopnestDepth) const {

  // Loop is not in a handleable form.
  if (!Lp.isLoopSimplifyForm()) {
    return false;
  }

  // At least one of this loop's subloops reach MaxLoopNestLevel so we cannot
  // generate this loop.
  if (LoopnestDepth > MaxLoopNestLevel) {
    return false;
  }

  const SCEV *BETC = SE->getBackedgeTakenCount(&Lp);

  // Don't handle unknown loops for now.
  if (isa<SCEVCouldNotCompute>(BETC)) {
    return false;
  }

  for (auto I = Lp.block_begin(), E = Lp.block_end(); I != E; ++I) {

    // Skip this bblock as it has been checked by an inner loop.
    if (!Lp.empty() && LI->getLoopFor(*I) != (&Lp)) {
      continue;
    }

    if ((*I)->isLandingPad()) {
      return false;
    }

    auto Term = (*I)->getTerminator();

    if (isa<IndirectBrInst>(Term) || isa<InvokeInst>(Term) ||
        isa<ResumeInst>(Term)) {
      return false;
    }
  }

  return true;
}

void RegionIdentification::createRegion(const Loop &Lp) {
  IRRegion *Reg =
      new IRRegion(Lp.getHeader(), RegionBBlocksTy(Lp.getBlocks().begin(),
                                                   Lp.getBlocks().end()));
  IRRegions.push_back(Reg);
}

bool RegionIdentification::formRegionForLoop(const Loop &Lp,
                                             unsigned *LoopnestDepth) {
  SmallVector<Loop *, 8> GenerableLoops;
  bool Generable = true;

  *LoopnestDepth = 0;

  // Check which sub loops are generable.
  for (auto I = Lp.begin(), E = Lp.end(); I != E; ++I) {
    unsigned SubLoopnestDepth;

    if (formRegionForLoop(**I, &SubLoopnestDepth)) {
      GenerableLoops.push_back(*I);

      // Set maximum sub-loopnest depth
      *LoopnestDepth = std::max(*LoopnestDepth, SubLoopnestDepth);
    } else {
      Generable = false;
    }
  }

  // Check whether Lp is generable.
  if (Generable && !isSelfGenerable(Lp, ++(*LoopnestDepth))) {
    Generable = false;
  }

  // Lp itself is not generable so create regions for generable sub loops.
  if (!Generable) {
    // TODO: add logic to merge fuseable loops. This might also require
    // recognition of ztt and splitting basic blocks which needs to be done
    // in a transformation pass.
    for (auto I = GenerableLoops.begin(), E = GenerableLoops.end(); I != E;
         ++I) {
      createRegion(**I);
    }
  }

  return Generable;
}

void RegionIdentification::formRegions() {
  for (LoopInfo::iterator I = LI->begin(), E = LI->end(); I != E; ++I) {
    unsigned Depth;
    if (formRegionForLoop(**I, &Depth)) {
      createRegion(**I);
    }
  }
}

bool RegionIdentification::runOnFunction(Function &F) {
  this->Func = &F;

  LI = &getAnalysis<LoopInfo>();
  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  SE = &getAnalysis<ScalarEvolution>();

  formRegions();

  return false;
}

void RegionIdentification::releaseMemory() {

  for (auto &I : IRRegions) {
    delete I;
  }

  IRRegions.clear();
}

void RegionIdentification::print(raw_ostream &OS, const Module *M) const {
  for (auto I = IRRegions.begin(), E = IRRegions.end(); I != E; ++I) {
    OS << "\nRegion " << I - IRRegions.begin() + 1;
    OS << "\n  Entry BBlock: " << (*I)->EntryBB->getName();
    OS << "\n  Member BBlocks: ";

    for (auto II = (*I)->BasicBlocks.begin(), EE = (*I)->BasicBlocks.end();
         II != EE; ++II) {
      if (II != (*I)->BasicBlocks.begin()) {
        OS << ", ";
      }
      OS << (*II)->getName();
    }

    OS << "\n";
  }
}

void RegionIdentification::verifyAnalysis() const {
  /// TODO: implement later
}
