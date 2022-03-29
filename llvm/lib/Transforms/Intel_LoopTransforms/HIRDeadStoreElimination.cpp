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
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

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

// Count for loads eliminated using forward substitution:
STATISTIC(NumHIRForwardSubstitutedLoads,
          "Number of loads eliminated using forward substitution");

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

// Calculate the lexical range for intervening store in case of different
// parent loops.
// Ex:
//
// DO i1
//   DO i2          >>>>>> Range starts here
//     ...
//     DO i3
//       DO i4
//         A[i4] = ...      StoreRef
//       ENDDO i4
//     ENDDO i3
//   ENDDO i2
//
//   DO i2
//     DO i3
//       DO i4
//         A[i4] = ...      PostDomStoreRef
//         ... = A[i4 + 1]  LoadRef
//       ENDDO i4
//     ENDDO i3
//   ENDDO i2        <<<<<< Range ends here
//
// ENDDO i1
void calculateLexicalRange(unsigned &MinTopSortNumber,
                           unsigned &MaxTopSortNumber, const HLLoop *StoreLoop,
                           const HLLoop *PostDomStoreLoop) {
  // By default the outermost parent of the Store and PostDomStore loopnests is
  // a region. If so take the outermost loop of each loopnest.
  unsigned Level = 1;

  // Otherwise calculate lowest common ancestor loop of the parent loops of
  // StoreRef and PostDomStoreRef (i1 loop).
  const HLLoop *LCALoop =
      HLNodeUtils::getLowestCommonAncestorLoop(StoreLoop, PostDomStoreLoop);
  if (LCALoop)
    Level = LCALoop->getNestingLevel() + 1;

  // Calculate StoreRef parent loop ancestor (i2 loop) if any.
  if (StoreLoop) {
    auto *OutermostStoreParent = StoreLoop->getParentLoopAtLevel(Level);
    if (OutermostStoreParent)
      MinTopSortNumber = OutermostStoreParent->getTopSortNum();
  }

  // Calculate PostDomStoreRef parent loop ancestor (second i2 loop) if any.
  if (PostDomStoreLoop) {
    auto *OutermostPostDomStoreParent =
        PostDomStoreLoop->getParentLoopAtLevel(Level);
    if (OutermostPostDomStoreParent)
      MaxTopSortNumber =
          OutermostPostDomStoreParent->getLastChild()->getMaxTopSortNum();
  }
}

// Returns true if there is an intervening load between \p StoreRef and \p
// PostDomStoreRef which aliases with \p StoreRef. For example-
//
// A[i] =
//      = B[i]
// A[i] =
static bool
foundInterveningLoad(HIRDDAnalysis &HDDA, const RegDDRef *StoreRef,
                     const RegDDRef *PostDomStoreRef,
                     SmallVectorImpl<const RegDDRef *> &SubstitutibleLoads,
                     HIRLoopLocality::RefGroupVecTy &EqualityGroups) {

  assert((PostDomStoreRef->getDestTypeSizeInBytes() >=
          StoreRef->getDestTypeSizeInBytes()) &&
         "Post-dominating ref's size cannot be smaller than the other ref!");
  const HLDDNode *StoreNode = StoreRef->getHLDDNode();
  const HLDDNode *PostDomStoreNode = PostDomStoreRef->getHLDDNode();
  unsigned StoreSymbase = StoreRef->getSymbase();
  unsigned MinTopSortNum = StoreNode->getTopSortNum();
  unsigned MaxTopSortNum = PostDomStoreNode->getTopSortNum();
  unsigned MaxSubstitutibleLoadTopSortNum =
      !SubstitutibleLoads.empty()
          ? SubstitutibleLoads[0]->getHLDDNode()->getTopSortNum()
          : 0;

  const HLLoop *StoreLoop = StoreNode->getLexicalParentLoop();
  const HLLoop *PostDomStoreLoop = PostDomStoreNode->getLexicalParentLoop();
  bool InDifferentLoops = (StoreLoop != PostDomStoreLoop);

  // If StoreRef and PostDomStoreRef are in the different loops, we need to
  // calculate wider lexical range where the load could be intervening.
  if (InDifferentLoops)
    calculateLexicalRange(MinTopSortNum, MaxTopSortNum, StoreLoop,
                          PostDomStoreLoop);

  for (auto &AliasingMemRefGroup : EqualityGroups) {

    if (AliasingMemRefGroup.front()->getSymbase() != StoreSymbase) {
      continue;
    }

    // Refs are ordered in reverse lexical order.
    for (auto *AliasingMemRef : AliasingMemRefGroup) {

      if (AliasingMemRef == PostDomStoreRef) {
        // In case of same parent loop, don't need to analyze same group as this
        // is done in the caller.
        if (!InDifferentLoops) {
          break;
        } else {
          continue;
        }
      }

      unsigned AliasingMemRefTopSortNum =
          AliasingMemRef->getHLDDNode()->getTopSortNum();

      if (AliasingMemRefTopSortNum <= MinTopSortNum) {
        break;
      }

      // We can ignore refs which are lexically after PostDomStoreRef.
      if (AliasingMemRefTopSortNum > MaxTopSortNum) {
        continue;
      }

      int64_t Distance;

      bool IsFake = AliasingMemRef->isFake();

      // In the absence of substitutible loads, only intervening loads are a
      // problem and stores can be ignored. In the presence of substitutible
      // loads, aliasing stores between StoreRef and loads are also a problem.
      // Fake stores cannot be ignored as they can be either reads or writes in
      // the callee.
      if (!IsFake && AliasingMemRef->isLval() &&
          (AliasingMemRefTopSortNum >= MaxSubstitutibleLoadTopSortNum)) {
        continue;
      }

      // In case of different parent loops, we consider any load which fails
      // into calculated lexical range as intervening.
      if (InDifferentLoops) {
        return true;
      }

      if (!DDRefUtils::getConstByteDistance(StoreRef, AliasingMemRef,
                                            &Distance)) {
        if (!HDDA.doRefsAlias(StoreRef, AliasingMemRef)) {
          continue;
        }
        return true;
      }

      // Access pattern of fake refs is not known so distance cannot be used.
      if (IsFake) {
        return true;
      }

      if (Distance <= 0) {
        // Handles this case-
        //
        // %A is i8* type
        // StoreRef - (i16*)(%A)[0]
        // AliasingMemRef - (%A)[1]
        //
        if ((uint64_t)(-Distance) < StoreRef->getDestTypeSizeInBytes()) {
          return true;
        }

      } else if ((uint64_t)Distance <
                 AliasingMemRef->getDestTypeSizeInBytes()) {
        // Handles this case-
        //
        // %A is i8* type
        // StoreRef - (%A)[1]
        // AliasingMemRef - (i16*)(%A)[0]
        //
        return true;
      }
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

  if (Ref->isRval() || !Ref->accessesAlloca() || Ref->isFake() ||
      !UniqueGroupSymbases.count(Ref->getSymbase())) {
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

// Returns true if \p Ref is structually invalidated by a temp redefinition
// before we hit \p EndNode. Both of them lie inside \p ParentLp.
//
// For example, store's rval ref gets invalidated in the example below which
// prevents its forwarding into load.
//
// A[0] = %t1
// %t1 =
//     = A[0]
static bool isRefInvalidatedBeforeNode(const RegDDRef *Ref,
                                       const HLNode *EndNode,
                                       const HLLoop *ParentLp) {
  assert(!Ref->isMemRef() && "Memref not expected!");
  assert(EndNode && "EndNode is null!");

  unsigned Symbase = Ref->getSymbase();

  bool IsRegionInvariant = ((Symbase == ConstantSymbase) ||
                            EndNode->getParentRegion()->isInvariant(Symbase));

  if (IsRegionInvariant) {
    return false;
  }

  if (ParentLp && Ref->isLinearAtLevel(ParentLp->getNestingLevel())) {
    return false;
  }

  auto &BU = Ref->getBlobUtils();

  // Check nodes in range (RefNode, EndNode) for redefinitions of any temp
  // blobs used in Ref.
  for (auto *Node = Ref->getHLDDNode()->getNextNode(); Node != EndNode;
       Node = Node->getNextNode()) {
    auto *Inst = dyn_cast_or_null<HLInst>(Node);

    // Only handle straight line code when substituting non-linear Ref.
    if (!Inst) {
      return true;
    }

    auto *LvalRef = Inst->getLvalDDRef();

    if (!LvalRef || !LvalRef->isTerminalRef()) {
      continue;
    }

    unsigned TempIndex = BU.findTempBlobIndex(LvalRef->getSymbase());

    if ((TempIndex != InvalidBlobIndex) && Ref->usesTempBlob(TempIndex)) {
      return true;
    }
  }

  return false;
}

// Somtimes we can eliminate the store by forward substituting its RHS into
// loads. For example-
//
// A[0] = 0;
// %t = A[0];
// A[0] = 5;
//
// Can be optimized to-
//
// %t = 0;
// A[0] = 5;
//
// This function returns true if the collected loads can be substituted.
// Loads are collected in reverse lexical order.
static bool
canSubstituteLoads(const RegDDRef *StoreRef,
                   SmallVectorImpl<const RegDDRef *> &SubstitutibleLoads) {
  if (SubstitutibleLoads.empty()) {
    return true;
  }

  // Give up if load and store are not identical which can happen due to
  // bitcasts.
  if (!DDRefUtils::areEqual(StoreRef, SubstitutibleLoads[0])) {
    return false;
  }

  const HLInst *SInst = cast<HLInst>(StoreRef->getHLDDNode());

  // Only handle store instructions which is the common case, for simplicity.
  if (!isa<StoreInst>(SInst->getLLVMInstruction())) {
    return false;
  }

  auto *StoreRHS = SInst->getRvalDDRef();

  // Forward substituting a load requires additional aliasing checks so give up
  // for now.
  if (StoreRHS->isMemRef()) {
    return false;
  }

  // Check if store can be legally substituted.
  const HLLoop *ParLoop = SInst->getLexicalParentLoop();

  for (auto *LoadRef : SubstitutibleLoads) {
    auto *LoadNode = LoadRef->getHLDDNode();

    if ((ParLoop != LoadNode->getLexicalParentLoop()) ||
        !HLNodeUtils::dominates(SInst, LoadNode)) {
      return false;
    }
  }

  // Check if it is structurally legal to substitute StoreRHS into loads.
  // Only the lexically last load needs to be passed in.
  if (isRefInvalidatedBeforeNode(StoreRHS, SubstitutibleLoads[0]->getHLDDNode(),
                                 ParLoop)) {
    return false;
  }

  return true;
}

static void
removeDeadStore(const HLDDNode *StoreNode,
                SmallVectorImpl<const RegDDRef *> &SubstitutibleLoads) {
  auto *StoreRHS = StoreNode->getRvalDDRef();

  for (auto *LoadRef : SubstitutibleLoads) {
    HIRTransformUtils::replaceOperand(const_cast<RegDDRef *>(LoadRef),
                                      StoreRHS->clone());
  }

  auto *StoreParent = StoreNode->getParent();
  HLNodeUtils::remove(const_cast<HLDDNode *>(StoreNode));
  HLNodeUtils::removeRedundantNodes(StoreParent, true);

  ++NumHIRDeadRegularStoreEliminated;
  NumHIRForwardSubstitutedLoads += SubstitutibleLoads.size();
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
  // order. This needs to be done before calling foundInterveningLoad()
  // below
  // TODO: Is it worth changing the setup so reversal is not required?
  for (auto &RefGroup : EqualityGroups) {
    std::reverse(RefGroup.begin(), RefGroup.end());
  }

  bool Result = false;
  SmallPtrSet<HLLoop *, 8> OptimizedLoops;

  for (auto &RefGroup : EqualityGroups) {
    auto *Ref = RefGroup.front();

    if (Ref->isNonLinear()) {
      continue;
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

    bool IsUniqueSymbase = UniqueGroupSymbases.count(Ref->getSymbase());

    // For each store, check whether it post dominates another store and
    // eliminate the dominated store if-
    //
    // 1) There are no aliasing loads in between these two stores.
    //
    // Example with i8* type %A-
    //       %A[0] =
    // (i16*)%A[0] =
    //
    // Or,
    //
    // 2) All the intermediate loads are identical to the lexically first store
    // and can be forward substituted by its RHS.
    //
    // Example-
    // %A[0] = 0;
    //       = %A[0]
    //       = %A[0]
    // %A[0] =
    //
    // Forward substitution is performed in this case.
    //
    for (unsigned Index = 0; Index != RefGroup.size(); ++Index) {
      auto *PostDomRef = RefGroup[Index];
      // Skip any load/fake/masked ref.
      if (!PostDomRef->isLval() || PostDomRef->isFake() ||
          PostDomRef->isMasked()) {
        continue;
      }

      SmallVector<const RegDDRef *, 4> SubstitutibleLoads;
      const HLDDNode *PostDomDDNode = PostDomRef->getHLDDNode();

      for (unsigned I = Index + 1; I != RefGroup.size();) {
        auto *PrevRef = RefGroup[I];
        const HLDDNode *PrevDDNode = PrevRef->getHLDDNode();

        // Detached nodes can be encountered because removeRedundantNodes() is
        // run between the processing of ref groups
        if (!PrevDDNode->isAttached()) {
          RefGroup.erase(RefGroup.begin() + I);
          continue;
        }

        // Skip if we encounter a fake ref in between two stores.
        // Also skip if the PostDomRef's size is smaller than PrevRef as it does
        // not make PrevRef completely dead.
        if (PrevRef->isFake() || PrevRef->getDestTypeSizeInBytes() >
                                     PostDomRef->getDestTypeSizeInBytes()) {
          break;
        }

        // Refer to comments on canSubstituteLoads() for explanation.
        if (PrevRef->isRval()) {
          if (SubstitutibleLoads.empty() ||
              // Give up if the loads are not identical which can happen
              // due to bitcasts.
              DDRefUtils::areEqual(PrevRef, SubstitutibleLoads[0])) {
            SubstitutibleLoads.push_back(PrevRef);
            ++I;
            continue;
          }
          break;
        } else if (!canSubstituteLoads(PrevRef, SubstitutibleLoads)) {
          break;
        }

        // Check whether the DDRef with a high top sort number (PostDomRef) post
        // dominates the DDRef with a lower top sort number (PrevRef).
        // If Yes, remove the store instruction on PrevRef.
        if (!HLNodeUtils::postDominates(PostDomDDNode, PrevDDNode)) {
          ++I;
          continue;
        }

        if (!isValidParentChain(PostDomDDNode, PrevDDNode, PostDomRef)) {
          ++I;
          continue;
        }

        const HLLoop *StoreLoop = PrevDDNode->getLexicalParentLoop();
        const HLLoop *PostDomStoreLoop = PostDomDDNode->getLexicalParentLoop();
        bool InDifferentLoops = (StoreLoop != PostDomStoreLoop);

        // If there is aliasing load between the two stores, give up on current
        // PostDomRef.
        if ((!IsUniqueSymbase || InDifferentLoops) &&
            foundInterveningLoad(HDDA, PrevRef, PostDomRef, SubstitutibleLoads,
                                 EqualityGroups)) {
          break;
        }

        // After complete unroll, we may optimize multiple stores from the same
        // loop so postpone the invalidation.
        if (auto *ParLoop = PrevDDNode->getLexicalParentLoop()) {
          OptimizedLoops.insert(ParLoop);
        }

        // Delete the StoreInst on PrevRef, possibly even the entire loop.
        removeDeadStore(PrevDDNode, SubstitutibleLoads);

        // Remove Store and SubstitutibleLoads from collection and continue to
        // iterate.
        unsigned NumLoads = SubstitutibleLoads.size();
        auto BegIt = RefGroup.begin() + I - NumLoads;
        auto EndIt = RefGroup.begin() + I + 1;
        RefGroup.erase(BegIt, EndIt);
        SubstitutibleLoads.clear();

        I -= NumLoads;
        Result = true;
      }
    }
  }

  releaseMemory();

  if (!Result) {
    return false;
  }

  OptReportBuilder &ORBuilder = HNU.getHIRFramework().getORBuilder();

  for (auto *Lp : OptimizedLoops) {
    // remark: Dead stores eliminated in loop
    ORBuilder(*Lp).addRemark(OptReportVerbosity::Low, 25529u);

    if (Lp->isAttached()) {
      HIRInvalidationUtils::invalidateBody<HIRLoopStatistics>(Lp);
    }
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

PreservedAnalyses HIRDeadStoreEliminationPass::runImpl(
    llvm::Function &F, llvm::FunctionAnalysisManager &AM, HIRFramework &HIRF) {
  runDeadStoreElimination(HIRF, AM.getResult<HIRDDAnalysisPass>(F),
                          AM.getResult<HIRLoopStatisticsAnalysis>(F));
  return PreservedAnalyses::all();
}
