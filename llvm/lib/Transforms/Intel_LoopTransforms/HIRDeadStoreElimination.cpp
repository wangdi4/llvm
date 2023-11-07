//===- HIRDeadStoreElimination.cpp - Implements DeadStoreElimination class ===//
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

static cl::opt<bool>
    DeduceRegionLocalAlloca(OPT_SWITCH "-deduce-region-local-alloca",
                            cl::init(true), cl::Hidden,
                            cl::desc("Disable " OPT_DESC " pass"));

// Count for dead stores.
STATISTIC(NumHIRDeadStoreEliminated, "Number of Dead Stores Eliminated");

// Count for loads eliminated using forward substitution.
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
    if (Ref) {
      Ref->dump();
    } else {
      dbgs() << "nullptr\n";
    }
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

namespace {
/// Collects all AddressOf refs and fake refs attached to lifetime end
/// intrinsics.
class AddressOfAndFakeLifetimeRefCollector final : public HLNodeVisitorBase {
private:
  SmallVectorImpl<RegDDRef *> &AddressOfRefs;
  BasePtrToLifetimeEndInfoMapTy &FakeLifetimeEndRefs;

public:
  AddressOfAndFakeLifetimeRefCollector(
      SmallVectorImpl<RegDDRef *> &AddressOfRefs,
      BasePtrToLifetimeEndInfoMapTy &FakeLifetimeEndRefs)
      : AddressOfRefs(AddressOfRefs), FakeLifetimeEndRefs(FakeLifetimeEndRefs) {
  }

  void visit(HLDDNode *Node) {
    for (auto *Ref : make_range(Node->op_ddref_begin(), Node->op_ddref_end())) {
      if (Ref->isAddressOf()) {
        AddressOfRefs.push_back(Ref);
      }
    }

    auto *Inst = dyn_cast<HLInst>(Node);
    if (Inst && Inst->isLifetimeEndIntrinsic()) {
      assert((std::distance(Inst->fake_ddref_begin(), Inst->fake_ddref_end()) ==
              1) &&
             "Single fake ref expected!");

      auto *Ref = *Inst->fake_ddref_begin();
      auto *SizeRef = Inst->getOperandDDRef(0);

      int64_t Size = 0;
      bool Ret = SizeRef->isIntConstant(&Size);
      (void)Ret;
      assert(Ret && "Constant size expected in lifetime end intrinsic!");

      FakeLifetimeEndRefs[Ref->getBasePtrBlobIndex()].emplace_back(Ref, Size);
    }
  }

  void visit(HLGoto *Goto) {}
  void visit(HLLabel *Label) {}
  void visit(HLNode *Node) {
    llvm_unreachable(" visit(HLNode *) - Node not supported\n");
  }
  void postVisit(const HLNode *Node) {}
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
        HLS.getTotalStatistics(Loop).hasCallsWithUnknownAliasing();
  }

  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}

  bool foundUnsafeCall() const { return FoundUnsafeCall; }
  bool isDone() const { return FoundEndNode || FoundUnsafeCall; }
};

} // namespace

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
  if (PostDomStoreLoop && (PostDomStoreLoop != LCALoop)) {
    auto *OutermostPostDomStoreParent =
        PostDomStoreLoop->getParentLoopAtLevel(Level);
    if (OutermostPostDomStoreParent)
      MaxTopSortNumber =
          OutermostPostDomStoreParent->getLastChild()->getMaxTopSortNum();
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void dumpNode(const HLDDNode *StoreNode) {
  if (StoreNode) {
    StoreNode->dump();
  } else {
    dbgs() << "nullptr\n";
  }
}

void dumpInterveningRefInfo(const RegDDRef *AliasingMemRef,
                            const HLDDNode *StoreNode,
                            const HLDDNode *PostDomStoreNode) {
  dbgs() << "Found intervening aliasing ref: ";

  AliasingMemRef->getHLDDNode()->dump();

  dbgs() << "for StoreRef: ";
  dumpNode(StoreNode);
  dbgs() << "and PostDomStoreRef: ";

  dumpNode(PostDomStoreNode);
}
#endif //! defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Returns true if \p MemRef is accessing structure fields in an unusual way.
//
// For example, suppose %s is a structure containing 2 i32 fields.
// Then the following store and load have loop-carried dependency as the load is
// accessing structure fields in a combined manner.
//
// (%s)[i1+1].1 =
//            = (%s)[i1]
static bool isUnconventionalStructFieldAccess(const RegDDRef *MemRef,
                                              const DataLayout &DL) {

  auto *StructTy = dyn_cast<StructType>(MemRef->getDimensionElementType(1));

  if (!StructTy) {
    return false;
  }

  auto Offsets = MemRef->getTrailingStructOffsets(1);
  auto *OffsetTy = DDRefUtils::getOffsetType(StructTy, Offsets);

  // For empty offsets, assume zeroth field access of structs.
  if (Offsets.empty()) {
    while (OffsetTy->isStructTy()) {
      OffsetTy = cast<StructType>(OffsetTy)->getTypeAtIndex(0u);
    }
  }

  if (MemRef->getDestTypeSizeInBytes() > DL.getTypeAllocSize(OffsetTy)) {
    return true;
  }

  return false;
}

static bool areDistinctLocations(const RegDDRef *MemRef1,
                                 const RegDDRef *MemRef2) {
  assert(MemRef1->getNumDimensions() == MemRef2->getNumDimensions() &&
         "Number of dimensions don't match!");

  unsigned NumDims = MemRef1->getNumDimensions();
  auto &DL = MemRef1->getCanonExprUtils().getDataLayout();

  for (unsigned I = 1; I <= NumDims; ++I) {
    int64_t Const1 = 0, Const2 = 0;
    if (MemRef1->getDimensionIndex(I)->isIntConstant(&Const1) &&
        MemRef2->getDimensionIndex(I)->isIntConstant(&Const2) &&
        (Const1 != Const2)) {
      return true;
    }

    // Even if the structure of the memrefs is similar, one of the refs may be
    // accessing structure fields in an unconventional way. The following logic
    // tries to identify such accesses and gives up on looking at field
    // accesses.
    if ((I == 1) && (isUnconventionalStructFieldAccess(MemRef1, DL) ||
                     isUnconventionalStructFieldAccess(MemRef2, DL))) {
      continue;
    }

    auto *DimTy1 = MemRef1->getDimensionElementType(I);
    auto *DimTy2 = MemRef2->getDimensionElementType(I);
    auto Offsets1 = MemRef1->getTrailingStructOffsets(I);
    auto Offsets2 = MemRef2->getTrailingStructOffsets(I);

    if (DDRefUtils::getOffsetDistance(DimTy1, DL, Offsets1) !=
        DDRefUtils::getOffsetDistance(DimTy2, DL, Offsets2)) {
      return true;
    }
  }

  return false;
}

// Returns true if there is an intervening load/store between \p StoreRef and \p
// PostDomStoreRef which aliases with \p StoreRef. For example-
//
// A[i] =
//      = B[i]
// A[i] =
//
// Intervening stores matter when we have loads which need to be substituted.
// If StoreRef is null, we are checking for intervening memrefs between region
// entry and PostDomStoreRef. If PostDomStoreRef is null, we are checking for
// intervening memrefs between StoreRef and region exit. At least, one of them
// has to be non-null.
static bool foundInterveningLoadOrStore(
    HIRDDAnalysis &HDDA, const RegDDRef *StoreRef,
    const RegDDRef *PostDomStoreRef,
    SmallVectorImpl<const RegDDRef *> &SubstitutableMemRefs,
    HIRLoopLocality::RefGroupVecTy &EqualityGroups) {

  assert((StoreRef || PostDomStoreRef) &&
         "Atleast one of StoreRef/PostDomStoreRef should be non-null!");

  assert((!PostDomStoreRef || PostDomStoreRef->isFake() || !StoreRef ||
          (PostDomStoreRef->getDestTypeSizeInBytes() >=
           StoreRef->getDestTypeSizeInBytes())) &&
         "Post-dominating ref's size cannot be smaller than the other ref!");

  const HLDDNode *StoreNode = StoreRef ? StoreRef->getHLDDNode() : nullptr;
  const HLDDNode *PostDomStoreNode =
      PostDomStoreRef ? PostDomStoreRef->getHLDDNode() : nullptr;

  unsigned StoreSymbase =
      StoreRef ? StoreRef->getSymbase() : PostDomStoreRef->getSymbase();

  // If StoreNode is null, we want to find intervening memrefs between region
  // entry and PostDomStoreRef.
  unsigned StoreTopSortNum = StoreNode ? StoreNode->getTopSortNum() : 0;
  unsigned MinTopSortNum = StoreTopSortNum;

  // Null PostDomStoreNode means we are trying to remove a store of base ptr
  // which is only used in current region. For this case we check aliasing loads
  // in the rest of the region.
  unsigned MaxTopSortNum =
      PostDomStoreNode ? PostDomStoreNode->getTopSortNum()
                       : StoreNode->getParentRegion()->getMaxTopSortNum();
  unsigned MaxSubstitutableLoadTopSortNum =
      !SubstitutableMemRefs.empty()
          ? SubstitutableMemRefs[0]->getHLDDNode()->getTopSortNum()
          : 0;

  const HLLoop *StoreLoop =
      StoreNode ? StoreNode->getLexicalParentLoop() : nullptr;
  const HLLoop *PostDomStoreLoop =
      PostDomStoreNode ? PostDomStoreNode->getLexicalParentLoop() : nullptr;
  bool InDifferentLoops = (StoreLoop != PostDomStoreLoop);

  // If StoreRef and PostDomStoreRef are in the different loops, we need to
  // calculate wider lexical range where the load could be intervening.
  if (InDifferentLoops)
    calculateLexicalRange(MinTopSortNum, MaxTopSortNum, StoreLoop,
                          PostDomStoreLoop);

  // Overwrite StoreRef to make it non-null as it is used in DD query and
  // distance calculation in the for loop below. This is somewhat hacky.
  if (!StoreRef)
    StoreRef = PostDomStoreRef;

  for (auto &AliasingMemRefGroup : EqualityGroups) {

    auto *LastRef = AliasingMemRefGroup.back();

    // A group can have an only nullptr ref left if all the other refs have been
    // optimized away.
    if (!LastRef || LastRef->getSymbase() != StoreSymbase) {
      continue;
    }

    // Refs are ordered in reverse lexical order.
    for (auto *AliasingMemRef : AliasingMemRefGroup) {

      if (!AliasingMemRef) {
        continue;
      }

      // If PostDomStoreRef is fake, that means it is attached to lifetime.end
      // intrinsic. These refs may be inserted in multiple groups so we cannot
      // identify the current group using them.
      if ((AliasingMemRef == PostDomStoreRef) && !PostDomStoreRef->isFake()) {
        // In case of same parent loop, don't need to analyze same group as this
        // is done in the caller.
        if (!InDifferentLoops) {
          break;
        } else {
          continue;
        }
      }

      auto *AliasingRefNode = AliasingMemRef->getHLDDNode();
      unsigned AliasingMemRefTopSortNum = AliasingRefNode->getTopSortNum();

      if (AliasingMemRefTopSortNum <= MinTopSortNum) {
        break;
      }

      // We can ignore refs which are lexically after PostDomStoreRef.
      if (AliasingMemRefTopSortNum > MaxTopSortNum) {
        continue;
      }

      bool IsFake = AliasingMemRef->isFake();

      // We can ignore fake refs attached to lifetime intrinsics.
      if (IsFake && cast<HLInst>(AliasingRefNode)->isLifetimeIntrinsic()) {
        continue;
      }

      // In the absence of substitutable loads, only intervening loads are a
      // problem and stores can be ignored. In the presence of substitutable
      // loads, aliasing stores between StoreRef and loads are also a problem.
      // Fake stores cannot be ignored as they can be either reads or writes in
      // the callee.
      if (!IsFake && AliasingMemRef->isLval() &&
          (AliasingMemRefTopSortNum >= MaxSubstitutableLoadTopSortNum ||
           AliasingMemRefTopSortNum <= StoreTopSortNum)) {
        continue;
      }

      // We should ignore SubstitutableMemRefs as intervening refs.
      if (std::any_of(
              SubstitutableMemRefs.begin(), SubstitutableMemRefs.end(),
              [=](const RegDDRef *Ref) { return Ref == AliasingMemRef; })) {
        continue;
      }

      int64_t Distance;
      if (!DDRefUtils::getConstByteDistance(StoreRef, AliasingMemRef,
                                            &Distance)) {
        if (!HDDA.doRefsAlias(StoreRef, AliasingMemRef)) {
          continue;
        }

        LLVM_DEBUG(dumpInterveningRefInfo(AliasingMemRef, StoreNode,
                                          PostDomStoreNode));
        return true;
      }

      // Access pattern of fake refs is not known so distance cannot be used.
      if (IsFake) {
        LLVM_DEBUG(dumpInterveningRefInfo(AliasingMemRef, StoreNode,
                                          PostDomStoreNode));
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
          LLVM_DEBUG(dumpInterveningRefInfo(AliasingMemRef, StoreNode,
                                            PostDomStoreNode));
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
        LLVM_DEBUG(dumpInterveningRefInfo(AliasingMemRef, StoreNode,
                                          PostDomStoreNode));
        return true;
      }

      // When StoreRef and PostDomStoreRef are in different parent loops
      // checking constant distance is not enough. Unless we can prove that
      // StoreRef and AliasingMemRef are always distinct locations we have to
      // give up on possible loop-carried dependency. For example-
      //
      // DO i1 = 0, 10
      //     = A[i1-1] // load used store's value
      //   A[i1] =
      // END DO
      //
      // DO i1 = 0, 10
      //   A[i1] =
      // END DO
      //
      // If store is structurally invariant in loopnest, constant distance
      // implies distinct locations.
      if (InDifferentLoops && AliasingMemRef->isRval() &&
          !StoreRef->isStructurallyInvariantAtLevel(1) &&
          !areDistinctLocations(StoreRef, AliasingMemRef)) {

        LLVM_DEBUG(dumpInterveningRefInfo(AliasingMemRef, StoreNode,
                                          PostDomStoreNode));
        return true;
      }
    }
  }
  return false;
}

/// Returns the immediate child loop of \p OuterLoop which is a parent of \p
/// InnerLoop. If \p OuterLoop is null, the outermost loop of InnerLoop is
/// returned.
static const HLLoop *getImmediateChildLoop(const HLLoop *OuterLoop,
                                           const HLLoop *InnerLoop) {
  assert(!OuterLoop ||
         HLNodeUtils::contains(OuterLoop, InnerLoop) && "Invalid input loops!");

  auto *ParLp = InnerLoop;
  const HLLoop *ImmediateChildLoop = nullptr;

  while (ParLp != OuterLoop) {
    ImmediateChildLoop = ParLp;
    ParLp = ParLp->getParentLoop();
  }

  return ImmediateChildLoop;
}

bool HIRDeadStoreElimination::hasValidParentLoopBounds(
    const HLLoop *PostDominatingLoop, const HLLoop *PrevLoop,
    const RegDDRef *Ref, const HLNode *&OutermostPostDominatingNode,
    const HLNode *&OutermostPrevNode) {

  // PostDominatingLoop is the LCA loop if it contains PrevLoop.
  if (HLNodeUtils::contains(PostDominatingLoop, PrevLoop)) {
    // Extend the range to include outermost parent loop of PrevLoop which is
    // under LCA loop.
    //
    // DO i1 =      // LCA loop
    //   DO i2 =    // Extend the range to i2 loop to include unsafe_call()
    //     unsafe_call()
    //     DO i3 =
    //       A[5] = // store ref
    //     END DO
    //   END DO
    //
    //   A[5] =     // post-dominating store ref
    // END DO
    if (PrevLoop != PostDominatingLoop) {
      OutermostPrevNode = getImmediateChildLoop(PostDominatingLoop, PrevLoop);
    }
  } else {

    unsigned LoopLevel = PostDominatingLoop->getNestingLevel();
    unsigned PrevLoopLevel = PrevLoop->getNestingLevel();

    if (LoopLevel != PrevLoopLevel) {
      return false;
    }

    // Check whether PostDominatingLoop and PrevLoop's parent loop chain have
    // the same upperbound, lowerbound and stride.
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
  // There is one exception to this. If we know that the only non-linear blob in
  // the ref has a single definition which dominates all its uses, we can still
  // perform DSE.
  // TODO: refine the check in the visitor.
  if (PostDominatingLoop) {
    // PostDominatingLoop is now the LCA loop.
    unsigned LCALevel = PostDominatingLoop->getNestingLevel();

    if (!Ref->isLinearAtLevel(LCALevel) &&
        !hasSingleDominatingNonLinearTempAtLevel(Ref, LCALevel)) {
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
        !HLS.getTotalStatistics(CommonLoop).hasCallsWithUnknownAliasing()) {
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
bool HIRDeadStoreElimination::doCollection(HLRegion &Region) {
  // Collect equal MemRef(s): populates EqualityGroups with memrefs with the
  // same address
  HIRLoopLocality::populateEqualityGroups(Region.child_begin(),
                                          Region.child_end(), EqualityGroups,
                                          &UniqueGroupSymbases);

  if (EqualityGroups.empty()) {
    LLVM_DEBUG(dbgs() << "No MemRef is available\n";);
    return false;
  }

  // Collect AddressOf Ref(s) from the region: recurse into loops
  AddressOfAndFakeLifetimeRefCollector RC(AddressOfRefs, FakeLifetimeEndRefs);
  HNU.visitRange(RC, Region.child_begin(), Region.child_end());

  // Examine the collection:
  LLVM_DEBUG({
    printRefGroupVecTy(EqualityGroups, "EqualityGroups:");
    printRefVector(AddressOfRefs, "AddressOfRefs in Region:");
  });

  return true;
}

// Somtimes we can eliminate the store by replacing it with temp and all the
// intermediate memrefs in \p SubstitutableMemRefs with the same temp.
// For example-
//
// A[0] = 0;
// if () {
//   A[0] = 2;
// }
// %t = A[0];
// A[0] = 5;
//
// Can be optimized to-
//
// %temp = 0;
// if () {
//   %temp = 2;
// }
// %t = %temp
// A[0] = 5;
//
// This function returns true if the collected intermediate memrefs can be
// substituted.
static bool
canSubstituteMemRefs(const RegDDRef *StoreRef, const HLDDNode *PostDomStoreNode,
                     SmallVectorImpl<const RegDDRef *> &SubstitutableMemRefs) {
  if (SubstitutableMemRefs.empty()) {
    return true;
  }

  // Give up if load and store are not identical which can happen due to
  // bitcasts.
  if (!DDRefUtils::areEqual(StoreRef, SubstitutableMemRefs[0])) {
    return false;
  }

  const HLDDNode *StoreNode = StoreRef->getHLDDNode();
  auto *StoreLoop = StoreNode->getLexicalParentLoop();

  // Needed to pass to hasValidParentLoopBounds() but are unused.
  const HLLoop *PostDomStoreLoop =
      PostDomStoreNode ? PostDomStoreNode->getLexicalParentLoop() : nullptr;

  for (auto *MemRef : SubstitutableMemRefs) {
    auto *MemRefNode = MemRef->getHLDDNode();

    if (!HLNodeUtils::dominates(StoreNode, MemRefNode)) {
      return false;
    }

    auto *MemRefLoop = MemRefNode->getLexicalParentLoop();

    if (MemRefLoop && (MemRefLoop != StoreLoop)) {
      auto *MemRefStoreLCA =
          HLNodeUtils::getLowestCommonAncestorLoop(MemRefLoop, StoreLoop);

      // We can safely substitute memref if it is inside the same or inner loop.
      if (StoreLoop && !HLNodeUtils::contains(StoreLoop, MemRefLoop)) {

        // We can still substitute memref if store is invariant w.r.t the child
        // loop of the LCA loop.
        //
        // For example, this is a valid case-
        //
        // DO i1 = 0, 5   // LCA loop
        //   DO i2 = 0, 5 // Child of LCA loop
        //      A[i1] =   // StoreNode
        //   END DO
        //
        //   DO i2 = 0, 5
        //      = A[i1]   // LoadNode
        //   END DO
        // END DO
        //
        // This is not a valid case-
        //
        // DO i1 = 0, 5   // LCA loop
        //   DO i2 = 0, 5 // Child of LCA loop
        //      A[i2] =   // StoreNode
        //   END DO
        //
        //   DO i2 = 0, 5
        //      = A[i2]  // LoadNode
        //   END DO
        // END DO
        //
        const HLLoop *LCAChildLoop =
            getImmediateChildLoop(MemRefStoreLCA, StoreLoop);

        unsigned InvariantNestingLevel = LCAChildLoop->getNestingLevel();
        if (!StoreRef->isStructurallyInvariantAtLevel(InvariantNestingLevel)) {
          return false;
        }
      }

      // If PostDomStoreNode is 'closer' to MemRefNode than StoreNode, loop
      // nesting wise than it can reach the load and substitution is illegal.
      // For example-
      //
      // DO i1 = 0, 5
      //   A[0] =   // StoreNode
      //
      //   DO i2 = 0, 5
      //      = A[0] // LoadNode
      //     A[0] =    // PostDomStoreNode
      //   END DO
      // END DO
      //
      if (PostDomStoreLoop && MemRef->isStructurallyInvariantAtLevel(
                                  MemRefLoop->getNestingLevel())) {
        auto *MemRefPostDomStoreLCA = HLNodeUtils::getLowestCommonAncestorLoop(
            MemRefLoop, PostDomStoreLoop);

        if (MemRefPostDomStoreLCA) {
          if (!MemRefStoreLCA) {
            return false;
          }

          if ((MemRefStoreLCA != MemRefPostDomStoreLCA) &&
              HLNodeUtils::contains(MemRefStoreLCA, MemRefPostDomStoreLCA)) {
            return false;
          }
        }
      }
    }
  }

  return true;
}

// Marks \p Symbase liveout in \p Loop until we reach \p LCALoop.
static void markLiveout(unsigned Symbase, HLLoop *Loop, const HLLoop *LCALoop) {

  while (Loop != LCALoop) {
    Loop->addLiveOutTemp(Symbase);
    Loop = Loop->getParentLoop();
  }
}

// Marks \p Symbase livein in \p Loop until we reach \p LCALoop.
static void markLivein(unsigned Symbase, HLLoop *Loop, const HLLoop *LCALoop) {

  while (Loop != LCALoop) {
    Loop->addLiveInTemp(Symbase);
    Loop = Loop->getParentLoop();
  }
}

// Eliminates the store and all the intermediate memrefs represented by \p
// SubstitutableMemRefs. if \p SubstitutableMemRefs is empty, the store node is
// removed, otherwise store and all the intermediate memrefs are substituted
// with the same temp. \p SubstitutableMemRefs are in reverse lexical order.
static void
removeDeadStore(HLDDNode *StoreNode,
                SmallVectorImpl<const RegDDRef *> &SubstitutableMemRefs) {

  auto *StoreParent = StoreNode->getParent();

  // If there are substitutable loads, replace the store and the loads with the
  // same temp and rely on constant/copy propagation to clean it up.
  if (!SubstitutableMemRefs.empty()) {
    auto *StoreRef = StoreNode->getLvalDDRef();
    auto *Tmp = StoreNode->getHLNodeUtils().createTemp(StoreRef->getDestType());

    unsigned TmpSymbase = Tmp->getSymbase();

    HIRTransformUtils::replaceOperand(StoreRef, Tmp);

    auto *DeepestTempDef = Tmp;

    // First replace all the stores and figure out the deepest temp definition
    // so we can use it for calling makeConsistent() on temp uses.
    for (unsigned I = 0, E = SubstitutableMemRefs.size(); I < E; ++I) {

      auto *IntermediateStore = SubstitutableMemRefs[I];

      if (!IntermediateStore->isLval()) {
        continue;
      }

      ++NumHIRDeadStoreEliminated;

      auto *IntermediateStoreNode = IntermediateStore->getHLDDNode();
      auto *TmpClone = Tmp->clone();

      if (IntermediateStoreNode->getNodeLevel() >
          DeepestTempDef->getNodeLevel()) {
        DeepestTempDef = TmpClone;
      }

      HIRTransformUtils::replaceOperand(
          const_cast<RegDDRef *>(IntermediateStore), TmpClone);
      // Also substitute in the vector as this will be used to mark loop
      // livein/liveouts.
      SubstitutableMemRefs[I] = TmpClone;
    }

    SmallVector<RegDDRef *, 2> PrevTempDefs;
    PrevTempDefs.push_back(Tmp);

    // Process loads in lexical order so we can collect preceding stores.
    for (auto *LoadRef : make_range(SubstitutableMemRefs.rbegin(),
                                    SubstitutableMemRefs.rend())) {

      if (LoadRef->isLval()) {
        PrevTempDefs.push_back(const_cast<RegDDRef *>(LoadRef));
        continue;
      }

      ++NumHIRForwardSubstitutedLoads;

      auto *NonConstLoadRef = const_cast<RegDDRef *>(LoadRef);

      // Go through all previous defs to set loop liveins/liveouts.
      for (auto *PrevTempDef : PrevTempDefs) {
        HLLoop *LoadLoop = NonConstLoadRef->getLexicalParentLoop();
        HLLoop *PrevDefLoop = PrevTempDef->getLexicalParentLoop();

        auto *LCALoop =
            HLNodeUtils::getLowestCommonAncestorLoop(LoadLoop, PrevDefLoop);

        markLiveout(TmpSymbase, PrevDefLoop, LCALoop);
        markLivein(TmpSymbase, LoadLoop, LCALoop);
      }

      auto *TmpClone = Tmp->clone();
      HIRTransformUtils::replaceOperand(NonConstLoadRef, TmpClone);

      TmpClone->makeConsistent(DeepestTempDef);
    }

  } else {

    HLNodeUtils::remove(StoreNode);
  }

  // Redundant nodes visitor is run on once after optimizing the entire region
  // so we don't need to run it on the region here.
  if (!isa<HLRegion>(StoreParent)) {
    HLNodeUtils::removeRedundantNodes(StoreParent);
  }

  ++NumHIRDeadStoreEliminated;
}

bool HIRDeadStoreElimination::basePtrEscapesAnalysis(
    const RegDDRef *Ref) const {
  unsigned Symbase = Ref->getSymbase();
  unsigned BaseIndex = Ref->getBasePtrBlobIndex();

  for (auto *AddressRef : AddressOfRefs) {
    if (AddressRef->getSymbase() != Symbase) {
      continue;
    }

    auto *Inst = dyn_cast<HLInst>(AddressRef->getHLDDNode());
    // AddressOf refs in other nodes like HLIfs is fine.
    if (!Inst) {
      continue;
    }

    auto *Call = Inst->getCallInst();

    // Give up on non-call insts.
    if (!Call) {
      // We can ignore copy insts which are defining the base ptr of the ref.
      if ((Inst->isCopyInst() ||
           isa<GetElementPtrInst>(Inst->getLLVMInstruction())) &&
          (Inst->getLvalDDRef()->getSelfBlobIndex() == BaseIndex)) {
        continue;
      }

      LLVM_DEBUG(dbgs() << "Base ptr alloca of ref: "; Ref->dump();
                 dbgs() << "escapes in: "; Inst->dump());
      return true;
    }

    // Use in lifetime intrinsics is fine.
    if (Inst->isLifetimeIntrinsic()) {
      continue;
    }

    // If the AddressOf argument is known to not be dereferenced by the call, we
    // will not have a fake ref for it and the use can be ignored.
    for (auto *FakeRef :
         make_range(Inst->fake_ddref_begin(), Inst->fake_ddref_end())) {
      if (FakeRef->getSymbase() == Symbase) {
        LLVM_DEBUG(dbgs() << "Base ptr alloca of ref: "; Ref->dump();
                   dbgs() << "escapes in: "; Inst->dump());
        return true;
      }
    }
  }

  return false;
}

bool HIRDeadStoreElimination::hasAllLoadsWithinRegion(HLRegion &Region,
                                                      const RegDDRef *Ref) {

  if (!DeduceRegionLocalAlloca) {
    return false;
  }

  auto *BaseVal = Ref->getTempBaseValue();

  if (!BaseVal) {
    return false;
  }

  // In fortran, it is possible for base to be a GEP which traces back to an
  // alloca. This can happen if the alloca array was split into multiple
  // dimensions in fortran src code. So we trace through the GEP chain to get to
  // the alloca. Technically, if the base is not region invariant, we should
  // trace back in HIR but I don't think any HIR transformation can invalidate
  // this kind of incoming IR.
  // Example test case in-
  // llvm/test/Transforms/Intel_LoopTransforms/HIRDeadStoreElimination/region-local-alloca-gep-base-ptr.ll
  auto *GEPBase = dyn_cast<GetElementPtrInst>(BaseVal);

  while (GEPBase) {
    BaseVal = GEPBase->getPointerOperand();
    GEPBase = dyn_cast<GetElementPtrInst>(BaseVal);
  }

  auto *Alloca = dyn_cast<AllocaInst>(BaseVal);

  if (!Alloca) {
    return false;
  }

  auto Iter = AllLoadsInSingleRegion.find(Alloca);

  if (Iter != AllLoadsInSingleRegion.end()) {
    return (Iter->second == &Region);
  }

  if (!Region.containsAllDereferences(Alloca, true) ||
      basePtrEscapesAnalysis(Ref)) {
    AllLoadsInSingleRegion[Alloca] = nullptr;

    LLVM_DEBUG(dbgs() << "Base ptr alloca of ref: "; Ref->dump();
               dbgs() << "either escapes or is not local to region.\n");
    return false;
  }

  AllLoadsInSingleRegion[Alloca] = &Region;

  LLVM_DEBUG(dbgs() << "Alloca: "; Alloca->printAsOperand(dbgs());
             dbgs() << " identified as region local alloca in region:";
             formatted_raw_ostream FOS(dbgs());
             Region.printHeader(FOS, 0, false, false));

  return true;
}

// Returns the size of the largest ref in the group.
// Due to BitcastDestTy, the size of the refs in the group may be different.
// Returns 0 if the group only has fake refs.
static uint64_t getMaxRefByteSize(const RefGroupTy &RefGroup) {
  uint64_t MaxSize = 0;

  for (auto *Ref : RefGroup) {
    if (Ref->isFake()) {
      continue;
    }

    MaxSize = std::max(MaxSize, Ref->getDestTypeSizeInBytes());
  }

  return MaxSize;
}

// Returns underlying alloca's size if \p Ref accesses alloca and we know
// alloca's size, else returns zero.
static int64_t getAllocaSizeInBytes(const RegDDRef *Ref) {
  auto *BaseVal = Ref->getTempBaseValue();

  if (!BaseVal) {
    return 0;
  }

  auto *Alloca = dyn_cast<AllocaInst>(BaseVal);

  if (!Alloca) {
    return 0;
  }

  auto Size =
      Alloca->getAllocationSizeInBits(Ref->getCanonExprUtils().getDataLayout());

  if (!Size) {
    return 0;
  }

  return (*Size) / 8;
}

// Inserts fake memrefs attached to lifetime end intrinsics which are usually of
// the form
// &(%A)[0] to this ref group if it is also based on %A as the base ptr.
void HIRDeadStoreElimination::insertFakeLifetimeRefs(RefGroupTy &RefGroup) {

  auto *FirstRef = RefGroup.front();
  unsigned BasePtrIndex = FirstRef->getBasePtrBlobIndex();

  auto FakeRefInfosIt = FakeLifetimeEndRefs.find(BasePtrIndex);

  if (FakeRefInfosIt == FakeLifetimeEndRefs.end()) {
    return;
  }

  auto &FakeRefInfos = FakeRefInfosIt->second;

  uint64_t MaxRefSize = getMaxRefByteSize(RefGroup);

  // Group only has fake refs...
  if (MaxRefSize == 0) {
    return;
  }

  // Insert fake refs in RefGroup while maintaining the lexical order of refs.
  auto RefIt = RefGroup.begin();

  for (auto &CurInfo : FakeRefInfos) {
    auto *FakeRef = CurInfo.FakeRef;
    int64_t LifetimeEndSize = CurInfo.Size;

    // Check if FakeRef is applicable to this group based on size info.
    // If not and fake ref is already inserted, we need to remove it.
    bool RemoveFakeRef = false;
    if ((LifetimeEndSize != -1) &&
        (LifetimeEndSize != getAllocaSizeInBytes(FakeRef))) {

      int64_t Distance;
      if (!DDRefUtils::getConstByteDistance(FirstRef, FakeRef, &Distance,
                                            true)) {
        continue;
      }

      assert(Distance >= 0 && "Non-negative distance expected!");

      // Lifetime end intrinsic does not apply if distance is larger.
      if (Distance + MaxRefSize > (uint64_t)LifetimeEndSize) {

        // Refs may not be equal due to '.0' trailing struct offsets.
        if ((Distance == 0) && DDRefUtils::areEqual(FirstRef, FakeRef, true)) {
          RemoveFakeRef = true;
        } else {
          continue;
        }
      }
    }

    unsigned FakeTopSortNum = FakeRef->getHLDDNode()->getTopSortNum();

    while (RefIt != RefGroup.end() &&
           (*RefIt)->getHLDDNode()->getTopSortNum() < FakeTopSortNum) {
      ++RefIt;
    }

    if (RemoveFakeRef) {
      assert(((*RefIt) == FakeRef) && "Unexpected ref encountered in group!");
      RefIt = RefGroup.erase(RefIt);

    } else {
      RefIt = RefGroup.insert(RefIt, FakeRef);
      ++RefIt;
    }
  }
}

// Finds definitions of temp with \p Symbase. Stops as soon as it finds the
// second definition.
class DefFinder final : public HLNodeVisitorBase {
  unsigned Symbase;
  unsigned NumSymbaseDefs;

public:
  DefFinder(unsigned Symbase) : Symbase(Symbase), NumSymbaseDefs(0) {}

  void visit(const HLNode *) {}
  void postVisit(const HLNode *) {}

  void visit(const HLInst *Inst);

  bool foundMultipleDefs() const { return NumSymbaseDefs > 1; }
  bool isDone() const { return foundMultipleDefs(); }
};

void DefFinder::visit(const HLInst *Inst) {
  auto *LvalRef = Inst->getLvalDDRef();

  if (LvalRef && (LvalRef->getSymbase() == Symbase)) {
    ++NumSymbaseDefs;
  }
}

bool HIRDeadStoreElimination::hasSingleDominatingNonLinearTempAtLevel(
    const RegDDRef *Ref, unsigned Level) {
  if (Level == 0) {
    return false;
  }

  auto *SingleBlob = Ref->getSingleNonLinearBlobRef(Level);

  if (!SingleBlob) {
    return false;
  }

  unsigned Symbase = SingleBlob->getSymbase();

  auto *ParLoop = Ref->getParentLoop()->getParentLoopAtLevel(Level);

  auto It = NonLinearTempInfoMap.find(Symbase);

  if (It != NonLinearTempInfoMap.end()) {
    for (auto &Info : It->second) {
      if (Info.DefLoop == ParLoop) {
        return Info.HasSingleDominatingDef;
      }
    }
  }

  // To prove that the temp dominates all its uses in the loop, we check that-
  // 1) It is not livein to the loop.
  // 2) It has a single definition inside the loop.
  if (ParLoop->isLiveIn(Symbase)) {
    NonLinearTempInfoMap[Symbase].emplace_back(ParLoop, false);
    return false;
  }

  DefFinder DF(Symbase);

  HLNodeUtils::visitRange(DF, ParLoop->child_begin(), ParLoop->child_end());

  // Ideally, we would like to assert that we found at least one definition for
  // the base ptr but some transformations are not able to set precise def level
  // info so we can't do that.
  bool FoundSingleDef = !DF.foundMultipleDefs();

  NonLinearTempInfoMap[Symbase].emplace_back(ParLoop, FoundSingleDef);

  return FoundSingleDef;
}

// Returns the first ref in the group which needs to be analyzed for
// correctness.
static const RegDDRef *getFirstNonIgnorableRef(const RefGroupTy &RefGroup) {

  // We iterate group in reverse which is the lexical order.
  for (auto *Ref : make_range(RefGroup.rbegin(), RefGroup.rend())) {
    if (Ref->isFake()) {
      // Return fake refs not coming from lifetime intrinsics.
      // We can ignore lifetime intrinsic refs.
      if (!cast<HLInst>(Ref->getHLDDNode())->isLifetimeIntrinsic())
        return Ref;

    } else {
      return Ref;
    }
  }

  return nullptr;
}

bool HIRDeadStoreElimination::foundReuseInAliasingLiveinLoad(
    const HLRegion &Region, const RefGroupTy &RefGroup) {
  if (!Region.canBeReentered())
    return false;

  const RegDDRef *FirstStoreRef = getFirstNonIgnorableRef(RefGroup);

  if (!FirstStoreRef || !FirstStoreRef->isLval() || FirstStoreRef->isFake())
    return true;

  SmallVector<const RegDDRef *, 1> SubstitutableMemRefs;
  return foundInterveningLoadOrStore(HDDA, nullptr, FirstStoreRef,
                                     SubstitutableMemRefs, EqualityGroups);
}

bool HIRDeadStoreElimination::run(HLRegion &Region) {
  // It isn't worth optimizing incoming single bblock regions.
  if (Region.isLoopMaterializationCandidate()) {
    return false;
  }

  if (!doCollection(Region)) {
    LLVM_DEBUG(dbgs() << "Failed collection\n";);
    return false;
  }

  // Refs returned by populateEqualityGroups() are in lexical order within the
  // group. We need to reverse them as they are processed in reverse lexical
  // order. This needs to be done before calling foundInterveningLoadOrStore()
  // below
  // TODO: Is it worth changing the setup so reversal is not required?
  for (auto &RefGroup : EqualityGroups) {

    insertFakeLifetimeRefs(RefGroup);

    std::reverse(RefGroup.begin(), RefGroup.end());
  }

  // Add a null ref at the beginning of the group if the base ptr is a region
  // local alloca. It will act as a dummy post-dominating store ref for the
  // entire group. This approach fits nicely with the existing setup below which
  // analyzes the group. We do this after all groups have been reversed because
  // we are using foundInterveningLoadOrStore() for legality checks which relies
  // on reverse lexical order.
  for (auto &RefGroup : EqualityGroups) {
    if (hasAllLoadsWithinRegion(Region, RefGroup.front()) &&
        !foundReuseInAliasingLiveinLoad(Region, RefGroup)) {
      RefGroup.insert(RefGroup.begin(), nullptr);
    }
  }

  bool Result = false;
  bool SubstitutedLoads = false;
  SmallPtrSet<HLLoop *, 8> OptimizedLoops;

  for (auto &RefGroup : EqualityGroups) {
    auto *Ref = RefGroup.back();
    assert(Ref && "Ref is unexpectedly null!");

    if (Ref->isNonLinear()) {
      // We can handle non-linear refs if the only non-linear blob has a single
      // definition in the appropriate scope. The node could be detached if it
      // was optimized away while processing a previous ref group.
      auto *RefNode = Ref->getHLDDNode();
      if (!RefNode->isAttached() || !hasSingleDominatingNonLinearTempAtLevel(
                                        Ref, RefNode->getNodeLevel())) {
        continue;
      }
    }

    LLVM_DEBUG({
      printRefGroupTy(RefGroup, "RefGroup: ");
      dbgs() << "Ref: ";
      Ref->dump();
      dbgs() << "\n";
    });

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
      // Null first ref acts as the post-dominating store for the entire group.
      assert((PostDomRef || Index == 0) && "Unexpected null ref!");

      // Skip any load/fake/masked ref.
      // Fake ref attached to lifetime end intrinsic can act as a valid post
      // dominating ref.
      if (PostDomRef &&
          (!PostDomRef->isLval() ||
           (PostDomRef->isFake() && !cast<HLInst>(PostDomRef->getHLDDNode())
                                         ->isLifetimeEndIntrinsic()) ||
           PostDomRef->isMasked())) {
        continue;
      }

      SmallVector<const RegDDRef *, 4> SubstitutableMemRefs;
      const HLDDNode *PostDomDDNode =
          PostDomRef ? PostDomRef->getHLDDNode() : nullptr;

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
        if (PrevRef->isFake() || (PostDomRef && !PostDomRef->isFake() &&
                                  (PrevRef->getDestTypeSizeInBytes() >
                                   PostDomRef->getDestTypeSizeInBytes()))) {
          break;
        }

        // Refer to comments on canSubstituteMemRefs() for explanation.
        if (PrevRef->isRval()) {
          if (SubstitutableMemRefs.empty() ||
              // Give up if the loads are not identical which can happen
              // due to bitcasts.
              DDRefUtils::areEqual(PrevRef, SubstitutableMemRefs[0])) {
            SubstitutableMemRefs.push_back(PrevRef);
            ++I;
            continue;
          }
          break;
        } else if (!canSubstituteMemRefs(PrevRef, PostDomDDNode,
                                         SubstitutableMemRefs)) {

          LLVM_DEBUG(dbgs() << "Cannot substitute loads in between StoreRef: ";
                     PrevDDNode->dump(); dbgs() << " and PostDomStoreRef: ");
          LLVM_DEBUG(dumpNode(PostDomDDNode));

          // This may be a conditional store which cannot be eliminated due to
          // an intermediate load but it might be possible to eliminate both of
          // them using a previous dominating store so we should keep looking.
          if (DDRefUtils::areEqual(PrevRef, SubstitutableMemRefs[0])) {
            SubstitutableMemRefs.push_back(PrevRef);
            ++I;
            continue;

          } else {
            break;
          }
        }

        if (PostDomDDNode) {
          // Check whether the DDRef with a high top sort number (PostDomRef)
          // post dominates the DDRef with a lower top sort number (PrevRef). If
          // Yes, remove the store instruction on PrevRef.
          if (!HLNodeUtils::postDominates(PostDomDDNode, PrevDDNode)) {
            LLVM_DEBUG(dbgs() << "PostDomStoreRef: ");
            LLVM_DEBUG(dumpNode(PostDomDDNode));
            LLVM_DEBUG(dbgs() << " does not post dominate StoreRef: ";
                       PrevDDNode->dump());

            break;
          }

          if (!isValidParentChain(PostDomDDNode, PrevDDNode, PostDomRef)) {
            LLVM_DEBUG(dbgs() << "Invalid parent chain for StoreRef: ";
                       PrevDDNode->dump(); dbgs() << "and PostDomStoreRef: ");
            LLVM_DEBUG(dumpNode(PostDomDDNode));

            break;
          }
        }

        const HLLoop *StoreLoop = PrevDDNode->getLexicalParentLoop();
        const HLLoop *PostDomStoreLoop =
            PostDomDDNode ? PostDomDDNode->getLexicalParentLoop() : nullptr;
        bool InDifferentLoops = (StoreLoop != PostDomStoreLoop);

        // If there is aliasing memref between the two stores, give up on
        // current PostDomRef. For null PostDomDDNode, we should check the rest
        // of the region.
        if ((!IsUniqueSymbase || InDifferentLoops || !PostDomDDNode) &&
            foundInterveningLoadOrStore(HDDA, PrevRef, PostDomRef,
                                        SubstitutableMemRefs, EqualityGroups)) {
          break;
        }

        // After complete unroll, we may optimize multiple stores from the same
        // loop so postpone the invalidation.
        if (auto *ParLoop = PrevDDNode->getLexicalParentLoop()) {
          OptimizedLoops.insert(ParLoop);
        }

        // Delete the StoreInst on PrevRef, possibly even the entire loop.
        removeDeadStore(const_cast<HLDDNode *>(PrevDDNode),
                        SubstitutableMemRefs);

        // Remove Store and SubstitutableMemRefs from collection and continue to
        // iterate.
        unsigned NumLoads = SubstitutableMemRefs.size();
        auto BegIt = RefGroup.begin() + I - NumLoads;
        auto EndIt = RefGroup.begin() + I + 1;
        RefGroup.erase(BegIt, EndIt);
        SubstitutableMemRefs.clear();

        I -= NumLoads;
        SubstitutedLoads = (SubstitutedLoads || (NumLoads != 0));
        Result = true;
      }
    }
  }

  releaseMemory();

  if (!Result) {
    return false;
  }

  if (SubstitutedLoads)
    HIRTransformUtils::doConstantAndCopyPropagation(&Region);

  OptReportBuilder &ORBuilder = HNU.getHIRFramework().getORBuilder();

  for (auto *Lp : OptimizedLoops) {
    // remark: Dead stores eliminated in loop
    ORBuilder(*Lp).addRemark(OptReportVerbosity::Low,
                             OptRemarkID::DeadStoresEliminated);

    if (Lp->isAttached()) {

      if (!HIRTransformUtils::propagateSingleUseLoads(Lp)) {
        // propagateSingleUseLoads() invalidates loop body if it makes changes
        // so we only need to invalidate if it doesn't kick in.
        HIRInvalidationUtils::invalidateBody<HIRLoopStatistics>(Lp);
      }
    }
  }

  HLNodeUtils::removeRedundantNodes(&Region);

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
    Result = DSE.run(cast<HLRegion>(Region)) || Result;
  }

  return Result;
}

PreservedAnalyses HIRDeadStoreEliminationPass::runImpl(
    llvm::Function &F, llvm::FunctionAnalysisManager &AM, HIRFramework &HIRF) {
  ModifiedHIR =
      runDeadStoreElimination(HIRF, AM.getResult<HIRDDAnalysisPass>(F),
                              AM.getResult<HIRLoopStatisticsAnalysis>(F));
  return PreservedAnalyses::all();
}
