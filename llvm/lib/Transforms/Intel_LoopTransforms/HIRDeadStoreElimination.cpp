//===- HIRDeadStoreElimination.cpp - Implements DeadStoreElimination class ===//
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
// This file implements HIRDeadStoreElimination class.
// It can eliminate 2 types of dead stores.
//
// Example0: a dead store being dominated by a live store.
// [BEFORE]
// DO i1
//   A[i1] = 0 ; // regular DEAD store
//   T = 0
//
//   DO i2 = 0, 5
//     T = T + i2
//   END DO
//
//   A[i1] = T ; // LIVE store
// END DO
//  ----------------------------------------
// [AFTER]
// DO i1
//   // A[i1] = 0 is a regular DEAD store and is thus removed
//   T = 0
//
//   DO i2 = 0, 5
//     T = T + i2
//   END DO
//
//   A[i1] = T ; // LIVE store
// END DO
//
// ==========================================================================
// Example1: local dead store
//
// [BEFORE]
// foo(){
//   A[N], B[N], C[N];
// ...
//   DO i1
//     C[i1] = A[i1] + B[i1];
//   END DO
// ...
//   Print A;         // only array A is used after the loop,
//                    // thus store into C[i1] is dead.
// }
//
//  ----------------------------------------
// [AFTER]
// foo(){
//   A[N], B[N], C[N];
// ...
//   DO i1
//     t0 = A[i1] + B[i1]; // store into C[i1] is dead
//   END DO
// ...
//   Print A;
// }
//
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/Intel_LoopTransforms/HIRDeadStoreEliminationPass.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGrouping.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"

#include "HIRDeadStoreElimination.h"

#define OPT_SWITCH "hir-dead-store-elimination"
#define OPT_DESC "HIR Dead Store Elimination"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::dse;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

// Count for regular dead store:
STATISTIC(NumHIRDeadRegularStoreEliminated,
          "Number of Regular Dead Stores Eliminated");

// Count for local dead store:
STATISTIC(NumHIRDeadLocalStoreEliminated,
          "Number of Local Dead Stores Eliminated");

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
static void printRefGroupTy(RefGroupTy &Group, std::string Msg = "",
                            bool PrintNewLine = true) {
  formatted_raw_ostream FOS(dbgs());

  if (PrintNewLine) {
    FOS << "\n";
  }

  if (Msg.size()) {
    FOS << Msg << " : " << Group.size() << "\n";
  }

  for (auto &Ref : Group) {
    Ref->dump();
    FOS << ", ";
  }

  if (PrintNewLine) {
    FOS << "\n";
  }
}

static void printRefGroupVecTy(RefGroupVecTy &GroupVec, std::string Msg = "",
                               bool PrintPointer = false,
                               bool PrintNewLine = true) {
  formatted_raw_ostream FOS(dbgs());

  if (PrintNewLine) {
    FOS << "\n";
  }
  if (Msg.size()) {
    FOS << Msg << " : " << GroupVec.size() << "\n";
  }

  for (auto &VecItem : GroupVec) {
    FOS << VecItem.size() << " : ";
    for (auto &Ref : VecItem) {
      if (PrintPointer) {
        FOS << Ref << "\t";
      }
      Ref->dump();
      FOS << ", ";
    }
    FOS << "\n";
  }

  if (PrintNewLine) {
    FOS << "\n";
  }
}

#if 0
static void printLoopVector(SmallVectorImpl<HLLoop *> &LoopVec,
                            std::string Msg = "", bool PrintNewLine = true) {
  formatted_raw_ostream FOS(dbgs());

  if (PrintNewLine)
    FOS << "\n";
  if (Msg.size())
    FOS << Msg << ": " << LoopVec.size() << "\n";

  unsigned Count = 0;
  for (HLLoop *Lp : LoopVec) {
    FOS << Count++ << "\n";
    Lp->print(FOS, 2);
    FOS << "\n";
  }

  if (PrintNewLine)
    FOS << "\n";
}
#endif

static void printRefVector(SmallVectorImpl<const RegDDRef *> &RefVec,
                           std::string Msg = "", bool PrintPointer = false) {
  formatted_raw_ostream FOS(dbgs());
  if (Msg.size())
    FOS << Msg << ": " << RefVec.size() << "\n";
  unsigned Count = 0;
  for (const RegDDRef *Ref : RefVec) {
    FOS << Count++ << "\t";
    if (PrintPointer) {
      FOS << Ref << "\t";
    }
    Ref->print(FOS, true);
    FOS << "\n";
  }
}

static void printRefVector(SmallVectorImpl<RegDDRef *> &RefVec,
                           std::string Msg = "", bool PrintPointer = false) {
  formatted_raw_ostream FOS(dbgs());
  if (Msg.size())
    FOS << Msg << ": " << RefVec.size() << "\n";
  unsigned Count = 0;
  for (const RegDDRef *Ref : RefVec) {
    FOS << Count++ << "\t";
    if (PrintPointer) {
      FOS << Ref << "\t";
    }
    Ref->print(FOS, true);
    FOS << "\n";
  }
}

#endif //! defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

char HIRDeadStoreEliminationLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRDeadStoreEliminationLegacyPass, OPT_SWITCH, OPT_DESC,
                      false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
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
// A[i]   = .
// A[i+1] = .
// A[i]   = .
static bool
overlapsWithAnotherGroup(HIRDDAnalysis &HDDA,
                         HIRLoopLocality::RefGroupTy &RefGroup,
                         HIRLoopLocality::RefGroupVecTy &EqualityGroups) {
  auto *FirstRef = RefGroup.front();

  // A[0] and (i32*)A[0] are put in the same group. We need to take the max size
  // to check overlap correctly and also return the memref with largest size.
  auto GetMaxRefSize = [](HIRLoopLocality::RefGroupTy &Group) {
    uint64_t Size = 0;
    const RegDDRef *MaxRef = nullptr;

    for (auto *Ref : Group) {
      // Can happen for fake refs with opaque destination types.
      uint64_t RefSize = Ref->getDestType()->isSized()
                             ? Ref->getDestTypeSizeInBytes()
                             : UINT64_MAX;
      // Use >= here to avoid case when RefSize is 0 and MaxRef can be nullptr.
      if (RefSize >= Size) {
        Size = RefSize;
        MaxRef = Ref;
      }
    }

    std::pair<uint64_t, const RegDDRef *> MaxRefSizePair = {Size, MaxRef};
    return MaxRefSizePair;
  };

  uint64_t MaxRefSize;
  const RegDDRef *MaxRef = nullptr;

  std::tie(MaxRefSize, MaxRef) = GetMaxRefSize(RefGroup);

  // Refs are arranged in reverse topological order.
  unsigned MinTopSortNum = RefGroup.back()->getHLDDNode()->getTopSortNum();
  unsigned MaxTopSortNum = RefGroup.front()->getHLDDNode()->getTopSortNum();

  for (auto &TmpRefGroup : EqualityGroups) {
    auto *CurRef = TmpRefGroup.front();

    if (FirstRef == CurRef) {
      continue;
    }

    if (CurRef->getSymbase() != FirstRef->getSymbase()) {
      continue;
    }

    // Refs are arranged in reverse topological order.
    unsigned TmpMinTopSortNum =
        TmpRefGroup.back()->getHLDDNode()->getTopSortNum();
    unsigned TmpMaxTopSortNum =
        TmpRefGroup.front()->getHLDDNode()->getTopSortNum();

    if (MinTopSortNum > TmpMaxTopSortNum || MaxTopSortNum < TmpMinTopSortNum) {
      continue;
    }

    uint64_t CurMaxRefSize;
    const RegDDRef *CurMaxRef = nullptr;
    std::tie(CurMaxRefSize, CurMaxRef) = GetMaxRefSize(TmpRefGroup);
    int64_t Distance;

    if (!DDRefUtils::getConstByteDistance(FirstRef, CurRef, &Distance)) {
      if (!HDDA.doRefsAlias(MaxRef, CurMaxRef)) {
        continue;
      }
      return true;
    }

    if (Distance <= 0) {
      // Handles this case-
      //
      // %A is i8* type
      // FirstRef - (i16*)(%A)[0]
      // CurRef - (%A)[1]
      //
      if ((uint64_t)(-Distance) < MaxRefSize) {
        return true;
      }

    } else if ((uint64_t)Distance < CurMaxRefSize) {
      // Handles this case-
      //
      // %A is i8* type
      // FirstRef - (%A)[1]
      // CurRef - (i16*)(%A)[0]
      //
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

    if (!Ref->hasIV(LoopLevel)) {
      continue;
    }

    if (!PrevLoop->isDo() || !PostDominatingLoop->isDo()) {
      return false;
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

// Collect:
// - MemRef(s) on region or loop level,
// - AddressOf Ref(s) on region level.
//
// Return true if memref collection is not empty.
//
bool HIRDeadStoreElimination::doCollection(HLRegion &Region, HLLoop *Loop,
                                           bool IsRegion) {
  // Collect equal MemRef(s): populates EqualityGroups with memrefs with the
  // same address
  if (IsRegion) {
    HIRLoopLocality::populateEqualityGroups(Region.child_begin(),
                                            Region.child_end(), EqualityGroups,
                                            &UniqueGroupSymbases);
  } else {
    HIRLoopLocality::populateEqualityGroups(Loop->child_begin(),
                                            Loop->child_end(), EqualityGroups,
                                            &UniqueGroupSymbases);
  }
  if (EqualityGroups.empty()) {
    LLVM_DEBUG(dbgs() << "No MemRef is available\n";);
    return false;
  }

  // Collect AddressOf Ref(s) from the region: recurse into loops
  AddressOfRefCollector RC(AddressOfRefVec);
  HNU.visitRange(RC, Region.child_begin(), Region.child_end());

  // Examine the collection:
  LLVM_DEBUG({
    printRefGroupVecTy(EqualityGroups, "EqualityGroups:");
    printRefVector(AddressOfRefVec, "AddressOfRefVec in Region:");
  });

  return true;
}

// Special case when the group only has 1 Ref.
//
// Check the following:
// - region is function level;
// (and)
// - ref is a store to a local array;
// (and)
// - ref is inside a loop that has no unknown alias;
// (and)
// - ref has no edge (neither incoming, nor outgoing);
// (and)
// - ref is not address taken;
//
bool HIRDeadStoreElimination::doSingleItemGroup(
    HLRegion &Region, SmallVectorImpl<const RegDDRef *> &RefGroup) {

  if (!Region.isFunctionLevel()) {
    return false;
  }

  auto *Ref = RefGroup.front();
  LLVM_DEBUG({
    printRefVector(RefGroup, "RefVector: ");
    dbgs() << "Ref: ";
    Ref->dump();
    dbgs() << "\n";
  });

  if (Ref->isRval() || !Ref->accessesAlloca() || Ref->isFake()) {
    return false;
  }

  // - parent loop has unknown alias
  auto *Lp = Ref->getLexicalParentLoop();
  if (!Lp || HLS.getTotalLoopStatistics(Lp).hasCallsWithUnknownAliasing()) {
    return false;
  }

  // Skip the RefGroup if there is any use of the Ref in AddressOf Ref Vector or
  // any symbase matching in the AddressOf Ref Vector.
  if (std::any_of(AddressOfRefVec.begin(), AddressOfRefVec.end(),
                  [&](const RegDDRef *AddressOfRef) {
                    return Ref->getSymbase() == AddressOfRef->getSymbase();
                  })) {
    return false;
  }

  // Skip if there is either incoming or outgoing edges to|from the Ref.
  DDGraph DDG = HDDA.getGraph(&Region);
  if (DDG.getTotalNumIncomingFlowEdges(Ref) || DDG.getNumOutgoingEdges(Ref)) {
    return false;
  }

  // the StoreInst can be deleted safely
  auto *DDNode = const_cast<HLDDNode *>(Ref->getHLDDNode());
  if (!isa<StoreInst>(dyn_cast<HLInst>(DDNode)->getLLVMInstruction())) {
    LLVM_DEBUG(dbgs() << "Expect a StoreInst to delete\n";);
    return false;
  }

  if (auto *ParentLoop = DDNode->getLexicalParentLoop()) {
    HIRInvalidationUtils::invalidateBody<HIRLoopStatistics>(ParentLoop);
  }

  auto *Parent = DDNode->getParent();
  HLNodeUtils::remove(DDNode);
  HLNodeUtils::removeRedundantNodes(Parent, true);
  ++NumHIRDeadLocalStoreEliminated;

  return true;
}

bool HIRDeadStoreElimination::run(HLRegion &Region, HLLoop *Lp, bool IsRegion) {
  // It isn't worth optimizing incoming single bblock regions.
  if (Region.isLoopMaterializationCandidate()) {
    return false;
  }

  if (!doCollection(Region, Lp, IsRegion)) {
    LLVM_DEBUG(dbgs() << "Failed collection\n";);
    return false;
  }

  // Refs returned by populateEqualityGroups() are in lexical order within the
  // group. We neet to reverse them as they are processed in reverse lexical
  // order. This needs to be done before calling overlapsWithAnotherGroup()
  // below
  // TODO: Is it worth changing the setup so reversal is not required?
  for (auto &RefGroup : EqualityGroups) {
    std::reverse(RefGroup.begin(), RefGroup.end());
  }

  bool Result = false;
  for (auto &RefGroup : EqualityGroups) {
    auto *Ref = RefGroup.front();

    if (Ref->isNonLinear()) {
      continue;
    }

    if (!UniqueGroupSymbases.count(Ref->getSymbase())) {
      if (overlapsWithAnotherGroup(HDDA, RefGroup, EqualityGroups)) {
        continue;
      }
    }

    LLVM_DEBUG({
      printRefGroupTy(RefGroup, "RefGroup: ");
      dbgs() << "Ref: ";
      Ref->dump();
      dbgs() << "\n";
    });

    // Special case: if there is just 1 item in the group
    if (RefGroup.size() == 1) {
      Result = doSingleItemGroup(Region, RefGroup) || Result;
      continue;
    }

    // For each store, check whether it post dominates another store and there
    // is no load interleaving between these two stores.
    for (unsigned Index = 0; Index != RefGroup.size(); ++Index) {
      auto *PostDomRef = RefGroup[Index];
      // Skip any load/fake/masked ref.
      if (!PostDomRef->isLval() || PostDomRef->isFake() ||
          PostDomRef->isMasked()) {
        continue;
      }

      const HLDDNode *PostDomDDNode = PostDomRef->getHLDDNode();
      for (unsigned I = Index + 1; I != RefGroup.size();) {
        auto *PrevRef = RefGroup[I];

        // Skip if we encounter a load or fake ref in between two stores.
        // Also skip if the PostDomRef's size is smaller than PrevRef as it does
        // not make PrevRef completely dead.
        if (PrevRef->isRval() || PrevRef->isFake() ||
            PrevRef->getDestTypeSizeInBytes() >
                PostDomRef->getDestTypeSizeInBytes()) {
          break;
        }

        // Check whether the DDRef with a high top sort number (PostDomRef) post
        // dominates the DDRef with a lower top sort number (PrevRef).
        // If Yes, remove the store instruction on PrevRef.
        const HLDDNode *PrevDDNode = PrevRef->getHLDDNode();
        if (!HLNodeUtils::postDominates(PostDomDDNode, PrevDDNode)) {
          ++I;
          continue;
        }

        if (!isValidParentChain(PostDomDDNode, PrevDDNode, PostDomRef)) {
          ++I;
          continue;
        }

        // Delete the StoreInst on PrevRef, possibly even the entire loop.
        if (auto *PrevLoop = PrevDDNode->getLexicalParentLoop()) {
          HIRInvalidationUtils::invalidateBody<HIRLoopStatistics>(PrevLoop);
        }

        auto *PrevParent = PrevDDNode->getParent();
        HLNodeUtils::remove(const_cast<HLDDNode *>(PrevDDNode));
        HLNodeUtils::removeRedundantNodes(PrevParent, true);
        ++NumHIRDeadRegularStoreEliminated;
        Result = true;

        // Remove Index I from collection and continue to iterate.
        RefGroup.erase(RefGroup.begin() + I);
      }
    }
  }

  releaseMemory();

  if (!Result) {
    return false;
  }

  Region.setGenCode();
  return true;
}

static bool runDeadStoreElimination(HIRFramework &HIRF, HIRDDAnalysis &HDDA,
                                    HIRLoopStatistics &HLS) {
  if (DisablePass) {
    LLVM_DEBUG(dbgs() << "HIR Dead Store Elimination Disabled \n");
    return false;
  }

  HIRDeadStoreElimination DSE(HIRF, HDDA, HLS);
  bool Result = false;

  for (auto &Region : make_range(HIRF.hir_begin(), HIRF.hir_end())) {
    Result =
        DSE.run(cast<HLRegion>(Region), nullptr, /* IsRegion */ true) || Result;
  }

  return Result;
}

bool HIRDeadStoreEliminationLegacyPass::runOnFunction(Function &F) {
  if (skipFunction(F)) {
    LLVM_DEBUG(dbgs() << "HIR Dead Store Elimination Disabled \n");
    return false;
  }

  return runDeadStoreElimination(
      getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
      getAnalysis<HIRDDAnalysisWrapperPass>().getDDA(),
      getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS());
}

PreservedAnalyses
HIRDeadStoreEliminationPass::run(llvm::Function &F,
                                 llvm::FunctionAnalysisManager &AM) {
  runDeadStoreElimination(AM.getResult<HIRFrameworkAnalysis>(F),
                          AM.getResult<HIRDDAnalysisPass>(F),
                          AM.getResult<HIRLoopStatisticsAnalysis>(F));
  return PreservedAnalyses::all();
}
