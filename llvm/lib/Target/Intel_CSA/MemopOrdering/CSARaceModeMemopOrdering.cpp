//===-- CSARaceModeMemopOrdering.cpp - Race mode memop ordering -*- C++ -*-===//
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
/// This file implements the race mode memop ordering pass.
///
///===---------------------------------------------------------------------===//

#include "CSARaceModeMemopOrdering.h"

#include "llvm/IR/CFG.h"
#include "llvm/InitializePasses.h"

using namespace llvm;

#define DEBUG_TYPE "csa-race-mode-memop-ordering"

char CSARaceModeMemopOrdering::ID = 0;
constexpr auto PASS_DESC          = "CSA: Race mode memory operation ordering";

INITIALIZE_PASS_BEGIN(CSARaceModeMemopOrdering, DEBUG_TYPE, PASS_DESC, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)
INITIALIZE_PASS_END(CSARaceModeMemopOrdering, DEBUG_TYPE, PASS_DESC, false,
                    false)

StringRef CSARaceModeMemopOrdering::getPassName() const { return PASS_DESC; }

void CSARaceModeMemopOrdering::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.setPreservesCFG();
  return CSAMemopOrderingBase::getAnalysisUsage(AU);
}

void CSARaceModeMemopOrdering::order(Function &F) {
  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();

  SmallVector<ReturnInst *, 1> Returns;
  for (BasicBlock &BB : F) {
    BBRecs.insert({&BB, mergeBBOps(BB, Returns)});
  }

  for (ReturnInst *const RI : Returns) {
    createOrderingEdges(RI, collectMergedOps(RI->getParent()));
  }

  BBRecs.clear();
}

auto CSARaceModeMemopOrdering::mergeBBOps(
  BasicBlock &BB, SmallVectorImpl<ReturnInst *> &Returns) -> BBRecord {
  SmallVector<Value *, 4> Outords;
  if (&BB == &BB.getParent()->getEntryBlock())
    Outords.push_back(MemEntry);
  for (Instruction &I : BB) {
    if (needsOrderingEdges(I)) {
      if (const auto RI = dyn_cast<ReturnInst>(&I))
        Returns.push_back(RI);
      else
        Outords.push_back(createOrderingEdges(&I, MemEntry));
    }
  }
  BBRecord BBR;
  BBR.MergedOps = createAll0(Outords, BB.getTerminator());
  return BBR;
}

Value *CSARaceModeMemopOrdering::collectMergedOps(BasicBlock *Start,
                                                  unsigned DomLevel) {
  using std::pair;

  // Mark all dominators; they will be accounted for in the main traversal, so
  // they don't need to be handled in the predecessor traversals.
  for (auto DTNode = DT->getNode(Start); DTNode != nullptr;
       DTNode      = DTNode->getIDom()) {
    BBRecord &BBR = BBRecs[DTNode->getBlock()];
    if (not BBR.DomLevel)
      BBR.DomLevel = DomLevel;
  }

  // Walk up the dominator tree and collect edges from those blocks and any
  // non-dominating predecessors.
  SmallVector<Value *, 4> Collected;
  for (auto DTNode = DT->getNode(Start); DTNode != nullptr;
       DTNode      = DTNode->getIDom()) {
    BasicBlock *const BB = DTNode->getBlock();
    BBRecord &BBR        = BBRecs[BB];

    // If the traversal reaches a block at a different DomLevel, it will be
    // handled up the call stack and doesn't need to be handled here. This is
    // true for blocks that dominate that one as well.
    if (BBR.DomLevel != DomLevel)
      break;

    // Otherwise, this block _does_ need its edges collected.
    Collected.push_back(BBR.MergedOps);

    // Its predecessors might also need handling; go through them and check.
    bool NeedsPHI = false;
    SmallVector<pair<BasicBlock *, Value *>, 2> PredValues;
    for (BasicBlock *const PBB : predecessors(BB)) {
      BBRecord &PBBR = BBRecs[PBB];

      // Any predecessors of at DomLevel 0 are not accounted for yet along this
      // path, so recurse into them and collect their edges. Otherwise, just add
      // a <none>.
      if (not PBBR.DomLevel) {
        Value *const PredOps = collectMergedOps(PBB, DomLevel + 1);
        PredValues.push_back({PBB, PredOps});
        if (PredOps != NoneVal)
          NeedsPHI = true;
      } else {
        PredValues.push_back({PBB, NoneVal});
      }
    }

    // If any of the predecessors ended up producing a non-<none> value, add in
    // a phi to collect those.
    if (NeedsPHI) {
      PHINode *const PHI = createPHI(BB);
      for (const auto &PV : PredValues)
        PHI->addIncoming(PV.second, PV.first);
      Collected.push_back(PHI);
    }
  }

  // Clear all of the marked dominators.
  for (auto DTNode = DT->getNode(Start); DTNode != nullptr;
       DTNode      = DTNode->getIDom()) {
    BBRecord &BBR = BBRecs[DTNode->getBlock()];
    if (BBR.DomLevel == DomLevel)
      BBR.DomLevel = 0;
  }

  // Return an all0 with the collected values.
  return createAll0(Collected, Start->getTerminator());
}
