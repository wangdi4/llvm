//===- HIRDeadStoreElimination.cpp - Implements DeadStoreElimination class
//------------===//
//
// Copyright (C) 2015-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements HIRDeadStoreElimination class which eliminate the dead
// store instruction.
//
// For example:
//
// DO i1
// A[i1] = 0 << dead store
// T = 0
//
// DO i2 = 0, 5
// T = T + i2
// END DO
//
// A[i1] = T
// END DO
//
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/Intel_LoopTransforms/HIRDeadStoreElimination.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"

#include "llvm/ADT/SmallSet.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#define OPT_SWITCH "hir-dead-store-elimination"
#define OPT_DESC "HIR Dead Store Elimination"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

namespace {

class HIRDeadStoreElimination : public HIRTransformPass {

public:
  static char ID;

  HIRDeadStoreElimination() : HIRTransformPass(ID) {
    initializeHIRDeadStoreEliminationPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.setPreservesAll();
  }
};
} // namespace

char HIRDeadStoreElimination::ID = 0;
INITIALIZE_PASS_BEGIN(HIRDeadStoreElimination, OPT_SWITCH, OPT_DESC, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_END(HIRDeadStoreElimination, OPT_SWITCH, OPT_DESC, false, false)

FunctionPass *llvm::createHIRDeadStoreEliminationPass() {
  return new HIRDeadStoreElimination();
}

static bool doTransform(HLLoop *OutermostLp) {
  bool Result = false;

  HIRLoopLocality::RefGroupVecTy TemporalGroups;
  SmallSet<unsigned, 8> UniqueGroupSymbases;

  // Populates TemporalGroups by populating it with memref groups which have
  // unique temporal locality.
  HIRLoopLocality::populateTemporalLocalityGroups(
      OutermostLp, 0, TemporalGroups, &UniqueGroupSymbases);

  auto HigherTopSortNum = [](const RegDDRef *Ref1, const RegDDRef *Ref2) {
    return Ref1->getHLDDNode()->getTopSortNum() >
           Ref2->getHLDDNode()->getTopSortNum();
  };

  for (auto &RefVec : TemporalGroups) {
    auto *Ref = RefVec.front();

    if (!UniqueGroupSymbases.count(Ref->getSymbase())) {
      // TODO: improve logic when we have references like-
      //
      // A[i] =
      // A[i+1] =
      // A[i] =
      continue;
    }

    std::sort(RefVec.begin(), RefVec.end(), HigherTopSortNum);

    // For each store, check whether it post-dominates other stores and there is
    // no load in between two stores.
    for (unsigned Index = 0; Index != RefVec.size(); ++Index) {

      if (!RefVec[Index]->isLval()) {
        continue;
      }

      const HLDDNode *DDNode = RefVec[Index]->getHLDDNode();

      for (unsigned I = Index + 1; I != RefVec.size();) {

        // Skip if we encounter a load in between two stores.
        if (RefVec[I]->isRval()) {
          break;
        }

        // Check whether the DDRef with high top sort number post-dominates the
        // DDRef with lower top sort number. If Yes, remove the instruction with
        // lower top sort number.
        const HLDDNode *PrevDDNode = RefVec[I]->getHLDDNode();
        if (!HLNodeUtils::postDominates(DDNode, PrevDDNode)) {
          I++;
          continue;
        }

        auto ParentNode = PrevDDNode->getParent();
        HLNodeUtils::remove(const_cast<HLDDNode *>(PrevDDNode));
        HLNodeUtils::removeEmptyNodes(ParentNode, true);

        Result = true;
        RefVec.erase(RefVec.begin() + I);
      }
    }
  }

  // Mark the loop and its parent loop/region have been changed
  if (Result) {
    OutermostLp->getParentRegion()->setGenCode();
    HIRInvalidationUtils::invalidateBody(OutermostLp);
  }

  return Result;
}

static bool runDeadStoreElimination(HIRFramework &HIRF) {
  if (DisablePass) {
    DEBUG(dbgs() << "HIR Dead Store Elimination Disabled \n");
    return false;
  }

  SmallVector<HLLoop *, 64> CandidateLoops;

  HIRF.getHLNodeUtils().gatherOutermostLoops(CandidateLoops);

  if (CandidateLoops.empty()) {
    DEBUG(dbgs() << HIRF.getFunction().getName()
                 << "() has no outer-most loop\n ");
    return false;
  }

  bool Result = false;

  for (auto &Lp : CandidateLoops) {
    Result = doTransform(Lp) || Result;
  }

  return Result;
}

bool HIRDeadStoreElimination::runOnFunction(Function &F) {
  if (skipFunction(F)) {
    DEBUG(dbgs() << "HIR Dead Store Elimination Disabled \n");
    return false;
  }

  bool Result =
      runDeadStoreElimination(getAnalysis<HIRFrameworkWrapperPass>().getHIR());
  return Result;
}

PreservedAnalyses
HIRDeadStoreEliminationPass::run(llvm::Function &F,
                                 llvm::FunctionAnalysisManager &AM) {
  runDeadStoreElimination(AM.getResult<HIRFrameworkAnalysis>(F));
  return PreservedAnalyses::all();
}

void HIRDeadStoreElimination::releaseMemory() {}
