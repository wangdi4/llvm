//===- HIRDeadStoreElimination.cpp - Implements DeadStoreElimination class
//------------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"

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

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
    AU.setPreservesAll();
  }
};
} // namespace

char HIRDeadStoreElimination::ID = 0;
INITIALIZE_PASS_BEGIN(HIRDeadStoreElimination, OPT_SWITCH, OPT_DESC, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_END(HIRDeadStoreElimination, OPT_SWITCH, OPT_DESC, false, false)

FunctionPass *llvm::createHIRDeadStoreEliminationPass() {
  return new HIRDeadStoreElimination();
}

// Check whether the DDRefs in other groups have the same symbase as the
// current DDRef group and then check the distance between each other. If
// the distance is less than the size of current DDRef, return true and
// skip this case. If false, then send this DDRef group to process in dead
// store elimination. The following is an example
// A[i] =
// A[i+1] =
// A[i] =
static bool
overlapsWithAnotherGroup(HIRLoopLocality::RefGroupTy &RefGroup,
                         HIRLoopLocality::RefGroupVecTy &TemporalGroups,
                         const RegDDRef *FirstRef) {
  uint64_t SizeofRef =
      FirstRef->getCanonExprUtils().getTypeSizeInBytes(FirstRef->getDestType());

  for (auto &TmpRefGroup : TemporalGroups) {
    auto *CurRef = TmpRefGroup.front();

    if (FirstRef == CurRef) {
      continue;
    }

    if (CurRef->getSymbase() != FirstRef->getSymbase()) {
      continue;
    }

    int64_t Distance;

    if (!DDRefUtils::getConstByteDistance(FirstRef, CurRef, &Distance)) {
      return true;
    }

    uint64_t Dist = std::abs(Distance);

    if (Dist < SizeofRef) {
      return true;
    }
  }
  return false;
}

static bool checkParentLoopBounds(HLLoop *PostDominatingLoop, HLLoop *PrevLoop,
                                  const RegDDRef *PostDominatingRef) {
  unsigned LoopLevel = PostDominatingLoop->getNestingLevel();
  unsigned PrevLoopLevel = PrevLoop->getNestingLevel();

  if (LoopLevel != PrevLoopLevel) {
    return false;
  }

  for (; PrevLoop != PostDominatingLoop;
       LoopLevel--, PostDominatingLoop = PostDominatingLoop->getParentLoop(),
                    PrevLoop = PrevLoop->getParentLoop()) {

    if (!PrevLoop->isDo() || !PostDominatingLoop->isDo()) {
      return false;
    }

    if (!PostDominatingRef->hasIV(LoopLevel)) {
      continue;
    }

    RegDDRef *PDLoopUpperRef = PostDominatingLoop->getUpperDDRef();
    RegDDRef *PDLoopLowerRef = PostDominatingLoop->getLowerDDRef();
    RegDDRef *PDLoopStrideRef = PostDominatingLoop->getStrideDDRef();

    RegDDRef *PrevLoopUpperRef = PrevLoop->getUpperDDRef();
    RegDDRef *PrevLoopLowerRef = PrevLoop->getLowerDDRef();
    RegDDRef *PrevLoopStrideRef = PrevLoop->getStrideDDRef();

    if (!DDRefUtils::areEqual(PDLoopUpperRef, PrevLoopUpperRef) ||
        !DDRefUtils::areEqual(PDLoopLowerRef, PrevLoopLowerRef) ||
        !DDRefUtils::areEqual(PDLoopStrideRef, PrevLoopStrideRef)) {
      return false;
    }
  }

  return true;
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

  for (auto &RefGroup : TemporalGroups) {
    auto *Ref = RefGroup.front();

    if (Ref->isNonLinear()) {
      continue;
    }

    if (!UniqueGroupSymbases.count(Ref->getSymbase())) {
      if (overlapsWithAnotherGroup(RefGroup, TemporalGroups, Ref)) {
        continue;
      }
    }

    std::sort(RefGroup.begin(), RefGroup.end(), HigherTopSortNum);

    // For each store, check whether it post-dominates other stores and there is
    // no load in between two stores.
    for (unsigned Index = 0; Index != RefGroup.size(); ++Index) {

      auto *PostDominatingRef = RefGroup[Index];

      if (!PostDominatingRef->isLval() || PostDominatingRef->isFake()) {
        continue;
      }

      const HLDDNode *DDNode = PostDominatingRef->getHLDDNode();

      for (unsigned I = Index + 1; I != RefGroup.size();) {

        auto *PrevRef = RefGroup[I];

        // Skip if we encounter a load or fake ref in between two stores.
        if (PrevRef->isRval() || PrevRef->isFake()) {
          break;
        }

        // Check whether the DDRef with high top sort number post-dominates the
        // DDRef with lower top sort number. If Yes, remove the instruction with
        // lower top sort number.
        const HLDDNode *PrevDDNode = PrevRef->getHLDDNode();
        if (!HLNodeUtils::postDominates(DDNode, PrevDDNode)) {
          I++;
          continue;
        }

        // Check whether PostDominatingRef and PrevRef's parent loops have the
        // same upperbound, lowerbound and stride.
        HLLoop *PostDominatingLoop = DDNode->getLexicalParentLoop();
        HLLoop *PrevLoop = PrevDDNode->getLexicalParentLoop();

        if (!checkParentLoopBounds(PostDominatingLoop, PrevLoop,
                                   PostDominatingRef)) {
          I++;
          continue;
        }

        auto ParentNode = PrevDDNode->getParent();
        HLNodeUtils::remove(const_cast<HLDDNode *>(PrevDDNode));
        HLNodeUtils::removeEmptyNodes(ParentNode, true);

        Result = true;
        RefGroup.erase(RefGroup.begin() + I);
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

static bool runDeadStoreElimination(HIRFramework &HIRF,
                                    HIRLoopStatistics &HLS) {
  if (DisablePass) {
    LLVM_DEBUG(dbgs() << "HIR Dead Store Elimination Disabled \n");
    return false;
  }

  SmallVector<HLLoop *, 64> CandidateLoops;

  HIRF.getHLNodeUtils().gatherOutermostLoops(CandidateLoops);

  if (CandidateLoops.empty()) {
    LLVM_DEBUG(dbgs() << HIRF.getFunction().getName()
                      << "() has no outer-most loop\n ");
    return false;
  }

  bool Result = false;

  for (auto &Lp : CandidateLoops) {
    if (HLS.getTotalLoopStatistics(Lp).hasCallsWithUnknownMemoryAccess()) {
      continue;
    }
    Result = doTransform(Lp) || Result;
  }

  return Result;
}

bool HIRDeadStoreElimination::runOnFunction(Function &F) {
  if (skipFunction(F)) {
    LLVM_DEBUG(dbgs() << "HIR Dead Store Elimination Disabled \n");
    return false;
  }

  bool Result = runDeadStoreElimination(
      getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
      getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS());
  return Result;
}

PreservedAnalyses
HIRDeadStoreEliminationPass::run(llvm::Function &F,
                                 llvm::FunctionAnalysisManager &AM) {
  runDeadStoreElimination(AM.getResult<HIRFrameworkAnalysis>(F),
                          AM.getResult<HIRLoopStatisticsAnalysis>(F));
  return PreservedAnalyses::all();
}

void HIRDeadStoreElimination::releaseMemory() {}
