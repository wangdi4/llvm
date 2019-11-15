//===- HIRDeadStoreElimination.cpp - Implements DeadStoreElimination class ===//
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
#include "llvm/InitializePasses.h"
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

class HIRDeadStoreEliminationLegacyPass : public HIRTransformPass {
public:
  static char ID;
  HIRDeadStoreEliminationLegacyPass() : HIRTransformPass(ID) {
    initializeHIRDeadStoreEliminationLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
    AU.setPreservesAll();
  }
};

class HIRDeadStoreElimination {
  HIRLoopStatistics &HLS;

  bool isValidParentChain(const HLNode *PostDominatingNode,
                          const HLNode *PrevNode,
                          const RegDDRef *PostDominatingRef);

public:
  HIRDeadStoreElimination(HIRLoopStatistics &HLS) : HLS(HLS) {}
  bool doTransform(HLRegion &Reg);
};

class UnsafeCallVisitor final : public HLNodeVisitorBase {
  HIRLoopStatistics &HLS;
  const HLNode *StartNode;
  const HLNode *EndNode;
  bool FoundStartNode;
  bool FoundEndNode;
  bool FoundUnsafeCall;

public:
  UnsafeCallVisitor(HIRLoopStatistics &HLS, const HLNode *StartNode,
                    const HLNode *EndNode)
      : HLS(HLS), StartNode(StartNode), EndNode(EndNode), FoundStartNode(false),
        FoundEndNode(false), FoundUnsafeCall(false) {
    assert((isa<HLLoop>(StartNode) || isa<HLInst>(StartNode)) &&
           "Invalid start node!");
    assert((isa<HLLoop>(EndNode) || isa<HLInst>(EndNode)) &&
           "Invalid end node!");
  }

  bool isNodeRelevant(const HLNode *Node) {
    if (Node == StartNode) {
      FoundStartNode = true;
    } else if (Node == EndNode) {
      FoundEndNode = true;
    }

    return FoundStartNode;
  }

  void visit(const HLInst *Inst) {
    if (!isNodeRelevant(Inst)) {
      return;
    }

    // TODO: check mayThrow() as well
    FoundUnsafeCall = Inst->isUnknownAliasingCallInst();
  }

  void visit(const HLLoop *Loop) {
    if (!isNodeRelevant(Loop)) {
      return;
    }

    FoundUnsafeCall =
        HLS.getTotalLoopStatistics(Loop).hasCallsWithUnknownAliasing();
  }

  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}

  bool foundUnsafeCall() const { return FoundUnsafeCall; }
  bool isDone() const { return FoundEndNode || FoundUnsafeCall; }
};

} // namespace

char HIRDeadStoreEliminationLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRDeadStoreEliminationLegacyPass, OPT_SWITCH, OPT_DESC,
                      false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_END(HIRDeadStoreEliminationLegacyPass, OPT_SWITCH, OPT_DESC,
                    false, false)

FunctionPass *llvm::createHIRDeadStoreEliminationPass() {
  return new HIRDeadStoreEliminationLegacyPass();
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
                         HIRLoopLocality::RefGroupVecTy &EqualityGroups,
                         const RegDDRef *FirstRef) {
  auto *DestTy = FirstRef->getDestType();

  // Can happen for fake refs with opqaue types.
  if (!DestTy->isSized()) {
    return true;
  }

  uint64_t SizeofRef = FirstRef->getCanonExprUtils().getTypeSizeInBytes(DestTy);

  for (auto &TmpRefGroup : EqualityGroups) {
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

static bool hasValidParentLoopBounds(const HLLoop *PostDominatingLoop,
                                     const HLLoop *PrevLoop,
                                     const RegDDRef *Ref,
                                     const HLNode *&OutermostPostDominatingNode,
                                     const HLNode *&OutermostPrevNode) {
  unsigned LoopLevel = PostDominatingLoop->getNestingLevel();
  unsigned PrevLoopLevel = PrevLoop->getNestingLevel();

  if (LoopLevel != PrevLoopLevel) {
    return false;
  }

  // Check whether PostDominatingLoop and PrevLoop's parent loop chain have the
  // same upperbound, lowerbound and stride.
  for (; PrevLoop != PostDominatingLoop;
       LoopLevel--, OutermostPostDominatingNode = PostDominatingLoop,
                    OutermostPrevNode = PrevLoop,
                    PostDominatingLoop = PostDominatingLoop->getParentLoop(),
                    PrevLoop = PrevLoop->getParentLoop()) {

    if (!PrevLoop->isDo() || !PostDominatingLoop->isDo()) {
      return false;
    }

    if (!Ref->hasIV(LoopLevel)) {
      continue;
    }

    auto *PDLoopUpperRef = PostDominatingLoop->getUpperDDRef();
    auto *PDLoopLowerRef = PostDominatingLoop->getLowerDDRef();
    auto *PDLoopStrideRef = PostDominatingLoop->getStrideDDRef();

    auto *PrevLoopUpperRef = PrevLoop->getUpperDDRef();
    auto *PrevLoopLowerRef = PrevLoop->getLowerDDRef();
    auto *PrevLoopStrideRef = PrevLoop->getStrideDDRef();

    if (!DDRefUtils::areEqual(PDLoopUpperRef, PrevLoopUpperRef) ||
        !DDRefUtils::areEqual(PDLoopLowerRef, PrevLoopLowerRef) ||
        !DDRefUtils::areEqual(PDLoopStrideRef, PrevLoopStrideRef)) {
      return false;
    }
  }

  // We should conservatively return false if the ref is non-linear at the
  // common loop level. For example, different values of 't' prevent dead store
  // elimination in the following case-
  //
  // DO i1
  //   t =
  //   DO i2 = 0, 10
  //    A[t+i2] =
  //   END DO
  //
  //   t =
  //   DO i2 = 0, 10
  //    A[t+i2] =
  //   END DO
  // END DO
  //
  // TODO: refine the check in the visitor.
  if (PrevLoop) {
    // PrevLoop is now the LCA loop.
    if (!Ref->isLinearAtLevel(PrevLoop->getNestingLevel())) {
      return false;
    }
  } else {
    auto *Reg = OutermostPrevNode->getParentRegion();

    // Check that all the blobs in Ref are live in to the region to avoid
    // incorrectly optimizing cases like this-
    //
    // A[5+t] =
    // t =
    // A[5+t] =
    for (auto *BlobRef : make_range(Ref->blob_begin(), Ref->blob_end())) {
      if (!Reg->isLiveIn(BlobRef->getSymbase())) {
        return false;
      }
    }
  }

  return true;
}

bool HIRDeadStoreElimination::isValidParentChain(
    const HLNode *PostDominatingNode, const HLNode *PrevNode,
    const RegDDRef *PostDominatingRef) {

  auto *PrevLoop = PrevNode->getLexicalParentLoop();
  auto *PostDominatingLoop = PostDominatingNode->getLexicalParentLoop();

  auto *OutermostPrevNode = PrevNode;
  auto *OutermostPostDominatingNode = PostDominatingNode;
  const HLLoop *CommonLoop = nullptr;

  // We only handle the following two cases right now-
  // 1) Both refs have a parent loop.
  // 2) Neither ref has parent loop.
  if (PrevLoop && PostDominatingLoop) {
    if (!hasValidParentLoopBounds(
            PostDominatingLoop, PrevLoop, PostDominatingRef,
            OutermostPostDominatingNode, OutermostPrevNode)) {
      return false;
    }

    CommonLoop =
        HLNodeUtils::getLowestCommonAncestorLoop(PrevLoop, PostDominatingLoop);

    // Shortcut to avoid visitor below.
    if (CommonLoop &&
        !HLS.getTotalLoopStatistics(CommonLoop).hasCallsWithUnknownAliasing()) {
      return true;
    }

  } else if (PrevLoop || PostDominatingLoop) {
    // Give up if only one of the refs has parent loop.
    // This can be improved later.
    return false;
  }

  auto *Reg = OutermostPrevNode->getParentRegion();

  // Check whether an intermediate unsafe call prevents dead store elimination.
  // Outermost nodes determine the range that needs to be checked for safety of
  // the transformation. This will be the outermost parent loop of the refs,
  // leading up to the LCA loop. For example, consider this case-
  //
  // DO i1
  //   DO i2
  //     unsafe_call()
  //     DO i3
  //       A[i2][i3] =
  //     END DO
  //   END DO
  //
  //   DO i3
  //     DO i3
  //       A[i2][i3] =
  //     END DO
  //   END DO
  // END DO
  //
  // Even though unsafe_call() lies lexically before the first ref, it still
  // prevents dead store elimination as array A can be read inside the call.
  // That is why range starts from first i2 loop.
  UnsafeCallVisitor UCV(HLS, OutermostPrevNode, OutermostPostDominatingNode);

  // No need to recurse into loops. Checking the statistics of outermost loop is
  // enough for now.
  HLNodeUtils::visitRange<true, false>(
      UCV, CommonLoop ? CommonLoop->child_begin() : Reg->child_begin(),
      CommonLoop ? CommonLoop->child_end() : Reg->child_end());

  return !UCV.foundUnsafeCall();
}

bool HIRDeadStoreElimination::doTransform(HLRegion &Reg) {
  bool Result = false;

  // It isn't worth optimizing incoming single bblock regions.
  if (Reg.isLoopMaterializationCandidate()) {
    return false;
  }

  HIRLoopLocality::RefGroupVecTy EqualityGroups;
  SmallSet<unsigned, 8> UniqueGroupSymbases;

  // Populates EqualityGroups with memrefs with the same address, like A[i1] and
  // (i32*)A[i1].
  HIRLoopLocality::populateEqualityGroups(Reg.child_begin(), Reg.child_end(),
                                          EqualityGroups, &UniqueGroupSymbases);

  auto HigherTopSortNum = [](const RegDDRef *Ref1, const RegDDRef *Ref2) {
    return Ref1->getHLDDNode()->getTopSortNum() >
           Ref2->getHLDDNode()->getTopSortNum();
  };

  for (auto &RefGroup : EqualityGroups) {
    auto *Ref = RefGroup.front();

    if (Ref->isNonLinear()) {
      continue;
    }

    if (!UniqueGroupSymbases.count(Ref->getSymbase())) {
      if (overlapsWithAnotherGroup(RefGroup, EqualityGroups, Ref)) {
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

        if (!isValidParentChain(DDNode, PrevDDNode, PostDominatingRef)) {
          I++;
          continue;
        }

        if (auto *PrevLoop = PrevDDNode->getLexicalParentLoop()) {
          HIRInvalidationUtils::invalidateBody<HIRLoopStatistics>(PrevLoop);
        }

        auto *PrevParent = PrevDDNode->getParent();
        HLNodeUtils::remove(const_cast<HLDDNode *>(PrevDDNode));
        HLNodeUtils::removeEmptyNodes(PrevParent, true);

        Result = true;
        RefGroup.erase(RefGroup.begin() + I);
      }
    }
  }

  if (Result) {
    Reg.setGenCode();
  }

  return Result;
}

static bool runDeadStoreElimination(HIRFramework &HIRF,
                                    HIRLoopStatistics &HLS) {
  if (DisablePass) {
    LLVM_DEBUG(dbgs() << "HIR Dead Store Elimination Disabled \n");
    return false;
  }

  HIRDeadStoreElimination DSE(HLS);

  bool Result = false;

  for (auto &Reg : make_range(HIRF.hir_begin(), HIRF.hir_end())) {
    Result = DSE.doTransform(cast<HLRegion>(Reg)) || Result;
  }

  return Result;
}

bool HIRDeadStoreEliminationLegacyPass::runOnFunction(Function &F) {
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

void HIRDeadStoreEliminationLegacyPass::releaseMemory() {}
