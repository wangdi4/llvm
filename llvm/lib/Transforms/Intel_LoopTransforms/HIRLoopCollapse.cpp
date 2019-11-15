//==--- HIRLoopCollpase.cpp -Implements Loop Collapse Pass -*- C++ -*---===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===-----------------------------------------------------------------===//
// HIR Loop Collapse Case1: 2D array collapse
//
// [ORIGINAL]                       [AFTER HIR LoopCollapse]
// int A[N][N];                     int A[N][N];
//
// for(int i=0;i<=N-1;++i){
//   for(int j=0;j<=N-1;++j){       for(k=0;k<=N*N-1;++k){
//     A[i][j] = 0;                   A[0][k] = 0;
//   }                              }
// }
//
// *** ----------------             --------------------***
// HIR Loop Collapse Case2: 3D array collapse
//
// [ORIGINAL]                       [AFTER HIR LoopCollapse]
// int A[N][N][N];                  int A[N][N][N];
//
// for(int i=0;i<=N-1;++i){
//   for(int j=0;j<=N-1;++j){
//     for(int k=0;k<=N-1;++k){     for(r=0;r<=N*N*N-1;++r){
//       A[i][j][k] = 1;              A[0][0][r] = 1;
//     }                            }
//   }
// }
//
// Note:
// - There is no limitation on the levels of loop nest and dimensions of array
//   that can be collapsed.
//   As long as the conditions fit, the loop-collapsing optimization can
//   collapse as many dimensions/levels of nesting as possible.
//
//
//
//===----------------------------------------------------------------------===//
//
// This file implements HIR Loop Collapse Transformation (HLC) Pass.
//
// Available options:
// -hir-loop-collapse:          Perform HIR Loop Collapse
// -disable-hir-loop-collapse:  Disable/Bypass HIR Loop Collapse
//
// TODO:
// Revise transformation to allow collapse from non-innermost level.
// E.g.
//
//  for (i = 0; i <= 9; ++i) {
//    for (j = 0; j <= 9; ++j) {
//      for (k = 0; k <= 9; ++k) {
//        A[i][j][0] = 1; //can collapse on i-j level, not i-j-k
//      }
//    }
//  }
//
#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopCollapse.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"

#include "HIRLoopCollapseImpl.h"

#define DEBUG_TYPE "hir-loop-collapse"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::collapse;

// Disable the HIR Loop Collapse Optimization (default is false:enabled)
static cl::opt<bool>
    DisableHIRLoopCollapse("disable-hir-loop-collapse", cl::init(false),
                           cl::Hidden,
                           cl::desc("Disable HIR Loop Collapse (HLC)"));

STATISTIC(HIRLoopNestsCollapsed, "Number of HIR LoopNest(s) Collapsed");

// ** Ref Collector **
// - Collect all LoopNest-IV relevant GEPRef (GEP and MemRef) into GEPRefVec;
// - Collect all LoopNest-IV relevant non-GEPRef Ref into RefVec;
class HIRLoopCollapse::CollectRefs final : public HLNodeVisitorBase {
private:
  HIRLoopCollapse *HLC = nullptr;
  SmallVectorImpl<RegDDRef *> &RefVec;
  SmallVectorImpl<RegDDRef *> &GEPRefVec;

public:
  CollectRefs(HIRLoopCollapse *HLC, SmallVectorImpl<RegDDRef *> &RefVec,
              SmallVectorImpl<RegDDRef *> &GEPRefVec)
      : HLC(HLC), RefVec(RefVec), GEPRefVec(GEPRefVec) {}

  void visit(HLDDNode *Node) {
    for (auto I = Node->op_ddref_begin(), E = Node->op_ddref_end(); I != E;
         ++I) {
      collectRef(*I);
    }
  }

  // No processing needed for Goto, Label and HLNode types
  void visit(HLGoto *Goto){};
  void visit(HLLabel *Label){};
  void visit(HLNode *Node) {
    llvm_unreachable(" visit(HLNode *) - Node not supported\n");
  }
  void postVisit(const HLNode *Node) {}

  unsigned getNumGEPRef(void) const { return GEPRefVec.size(); }
  unsigned getNumNonGEPRef(void) const { return RefVec.size(); }

  void collectRef(RegDDRef *Ref);
};

// Collect any Ref that has at least 1 LoopNest-relevant IV in it, into either
// GEPRefVec (for GEPRef) and RefVec (for non-GEPRef Ref)
void HIRLoopCollapse::CollectRefs::collectRef(RegDDRef *Ref) {

  bool HasLoopNestIV = false;

  for (auto I = Ref->canon_begin(), E = Ref->canon_end(); I != E; ++I) {
    CanonExpr *CE = (*I);

    // Check for any loop-nest IV:
    if (HLC->hasLoopNestIV(CE)) {
      HasLoopNestIV = true;
      break;
    }
  }

  // Early exit: if no loop-level IV on any dimension inside Ref
  if (!HasLoopNestIV) {
    return;
  }

  // Collect:
  Ref->hasGEPInfo() ? GEPRefVec.push_back(Ref) : RefVec.push_back(Ref);
}

// ** LoopNest Collector **
// Gather all perfect LoopNest(s) starting from an given Outermost Loop
class HIRLoopCollapse::CollectCandidateLoops final : public HLNodeVisitorBase {
  SmallVectorImpl<InnerOuterLoopPairTy> &CandidateLoops;
  HLNode *SkipNode;

public:
  CollectCandidateLoops(SmallVectorImpl<InnerOuterLoopPairTy> &CandidateLoops)
      : CandidateLoops(CandidateLoops), SkipNode(nullptr) {}

  void visit(HLNode *Node) {}
  void postVisit(HLNode *Node) {}

  bool skipRecursion(const HLNode *Node) const { return Node == SkipNode; }

  // main collection routine:
  // Find perfect-loop nest, and collect its (OutermostLp, InnermostLp) pair
  void visit(HLLoop *Loop) {

    // Gather all perfect loop nests:
    const HLLoop *InnermostLoop = nullptr;

    if (Loop->isInnermost()) {
      SkipNode = Loop;
    } else if (HLNodeUtils::isPerfectLoopNest(Loop, &InnermostLoop)) {
      CandidateLoops.push_back(
          std::make_pair(Loop, const_cast<HLLoop *>(InnermostLoop)));

      // Once collected, NO need to recurse deeper into the current loop nest
      // starting from Loop.
      SkipNode = Loop;
    }
  }
};

bool HIRLoopCollapse::run() {
  if (DisableHIRLoopCollapse) {
    LLVM_DEBUG(dbgs() << "HIR Loop Collapse Disabled\n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "HIRLoopCollapse on Function : "
                    << HIRF.getFunction().getName() << "()\n");

  HNU = &HIRF.getHLNodeUtils();
  BU = &HIRF.getBlobUtils();

  // Collect all possible perfect-LoopNest candidate InnerOuterLoopPairs into
  // CandidateLoops. Each InnerOuterLoopPair marks (OutermostLp,InnermostLp),
  // forming a
  // perfect (sub) LoopNest.
  //
  // E.g.
  // | i1:
  // | IF(.)|
  // |  | i2:
  // |  |  | i3:
  // |  |  |
  // |  |
  // |
  //
  // There is no perfect loop nest within the original i1-i2-i3 nesting, but
  // (i2-i3) is a perfect sub loop nest, thus is collected as a
  // InnerOuterLoopPair
  // candidate for collapsing.
  //
  SmallVector<InnerOuterLoopPairTy, 12> CandidateLoops;
  CollectCandidateLoops CCL(CandidateLoops);
  HNU->visitAll(CCL);

  if (CandidateLoops.empty()) {
    LLVM_DEBUG(dbgs() << HIRF.getFunction().getName()
                      << "() has no perfect loop nest\n";);
    return false;
  }

  bool Result = false;

  for (auto &LPPair : CandidateLoops) {
    // Do loop collapse over a non-single-level perfect loop nest in
    // [OutermostLp, .., InnermostLp]
    Result = doLoopCollapse(LPPair.first, LPPair.second) || Result;
  }

  return Result;
}

bool HIRLoopCollapse::doLoopCollapse(HLLoop *OutermostLp, HLLoop *InnermostLp) {
  // Setup environment for the current perfect loop nest from
  // (OutermostLp, InnermostLp) pair.
  setupEnvLoopNest(OutermostLp, InnermostLp);

  // Analyze the LoopNest for loop collapse and reject if unsuitable
  if (!doAnalysis()) {
    return false;
  }

  // Do Loop Collapse Transformation on the loop nest
  doTransform(InnermostLp, InnermostLevel, getOutermostLevel());

  return true;
}

void HIRLoopCollapse::setupEnvLoopNest(HLLoop *OutermostLp,
                                       HLLoop *InnermostLp) {
  clearWorkingSetMemory();

  this->InnermostLp = InnermostLp;
  InnermostLevel = InnermostLp->getNestingLevel();
  unsigned OutermostLevel = OutermostLp->getNestingLevel();

  assert((OutermostLevel < InnermostLevel) &&
         "Expect OutermostLevel < InnermostLevel?\n");

  // Initialize NumCollapsableLoops once per potential LoopNest
  // (This value may shrink depending on DDRefs' situations.)
  NumCollapsableLoops = InnermostLevel - OutermostLevel + 1;
  assert((NumCollapsableLoops >= 2) && "Nothing to collapse\n");

  // Setup LoopNest
  LoopNest.fill(nullptr);
  unsigned Level = InnermostLevel;
  for (HLLoop *Lp = InnermostLp, *E = OutermostLp->getParentLoop(); Lp != E;
       Lp = Lp->getParentLoop(), --Level) {
    LoopNest[Level] = Lp;
  }
}

bool HIRLoopCollapse::doAnalysis(void) {

  if (!doPreliminaryChecks()) {
    LLVM_DEBUG(dbgs() << "HIRLoopCollapse: failed PreliminaryChecks\n");
    return false;
  }

  if (!doCollection()) {
    LLVM_DEBUG(dbgs() << "HIRLoopCollapse: failed Collection\n");
    return false;
  }

  if (!areGEPRefsLegal()) {
    LLVM_DEBUG(dbgs() << "HIRLoopCollapse: failed legal test\n");
    return false;
  }

  if (!areNonGEPRefsProfitable()) {
    LLVM_DEBUG(dbgs() << "HIRLoopCollapse: failed profit test\n");
    return false;
  }

  return true;
}

bool HIRLoopCollapse::doPreliminaryChecks(void) {

  IVType = InnermostLp->getIVType();
  uint64_t TripCount = 0;
  HLLoop *CurLp = InnermostLp;
  unsigned Count = 0;
  for (; Count < NumCollapsableLoops; Count++, CurLp = CurLp->getParentLoop()) {

    if (!CurLp->isDo() || !CurLp->isNormalized()) {
      break;
    }

    if (CurLp->getIVType() != IVType) {
      break;
    }

    if (CurLp->hasUnrollEnablingPragma() ||
        CurLp->hasUnrollAndJamEnablingPragma() ||
        CurLp->hasVectorizeEnablingPragma()) {
      break;
    }

    unsigned LoopLevel = CurLp->getNestingLevel();
    CanonExpr *UBCE = CurLp->getUpperCanonExpr();

    // Check: does CurLp have a constant TripCount?
    // If yes: save its trip count into UBTCArry;
    if (CurLp->isConstTripLoop(&TripCount)) {
      UBTCArry[LoopLevel].set(TripCount);
    }
    // Check: is there ONLY 1 blob in the UBCE and UBCE can't have any IV?
    //        Currently only "N-1" form of UB are handled.
    // If yes: save its UBCE into UBTCArry;
    else if (UBCE->numBlobs() == 1 && !UBCE->hasIV() &&
             UBCE->getConstant() == -1) {
      UBTCArry[LoopLevel].set(UBCE);
    } else {
      // Other cases are not supported!
      break;
    }
  }

  NumCollapsableLoops = std::min(Count, NumCollapsableLoops);

  return (NumCollapsableLoops > 1);
}

bool HIRLoopCollapse::doCollection(void) {
  // Collect each Ref that has at least 1 loop-relevant iv in it, including both
  // GEPRef(s) and non GEPRef(s).
  CollectRefs Collector(this, RefVec, GEPRefVec);
  HNU->visitRange(Collector, InnermostLp->getFirstChild(),
                  InnermostLp->getLastChild());

  // Examine all Ref(s) collected:

  // Check if there is at least 1 GEPRef available after collection.
  //
  // Note:
  // After collection, the results are in RefVec for non GEPRef(s) and
  // GEPRefVec for GEPRef(s).
  return (Collector.getNumGEPRef() >= 1);
}

// Get CollapseLevel on each GEPRefs, and Obtain the overall Minimal
// CollapseLevel (NumCollapsableLoops) over all GEPRefs.
//
// Note:
// - NumCollapsableLoops has a default value. The legal test may reduce it.
//
bool HIRLoopCollapse::areGEPRefsLegal(void) {

  for (RegDDRef *GEPRef : GEPRefVec) {

    bool Has1DimensionOnly = (GEPRef->getNumDimensions() == 1);
    unsigned CollapseLevel = getNumDimensionsOfMatchedSrcAndDestType(GEPRef);
    if (Has1DimensionOnly) {
      // This ref has only 1 dimenstion, so return false if the number of
      // collapsable dimesions is zero (i.e. no dimension to work on).
      // Also, do not update global NumCollapsableLoops with
      // getNumDimensionsOfMatchedSrcAndDestType
      if (CollapseLevel == 0) {
        return false;
      }
      // Check: does GEPRef match pattern on Dimension1?
      CollapseLevel = getLevelsOfIVPattern(GEPRef->getDimensionIndex(1));
    } else {
      if (CollapseLevel <= 1) {
        return false;
      }
      NumCollapsableLoops = std::min(NumCollapsableLoops, CollapseLevel);
      // ------------------------------
      // GEPRef        |CollapseLevel |
      // ------------------------------
      // A[ 0][ 0][i1] |  1           |
      // A[ 0][i1][i2] |  2           |
      // A[i1][i2][i3] |  3           |
      // ------------------------------
      CollapseLevel = getNumCollapsableLevels(GEPRef);
    }

    // Check: if CollapseLevel <= 1, legal test fails
    // Note:
    // - for A[i1][i2][0], getNumCollapsableLevels() will return 0, since
    // Dimension1 is not valid.
    //
    // - for A[i1].0[i2], getNumCollapsableLevels() will return 0, since there
    // is at least 1 struct-access not on Dimension1.
    if (CollapseLevel <= 1) {
      return false;
    }

    // Update NumCollapsableLoops
    NumCollapsableLoops = std::min(NumCollapsableLoops, CollapseLevel);
  }

  assert((NumCollapsableLoops > 1) && "Invalid NumCollapsableLoops\n");

  return true;
}

unsigned HIRLoopCollapse::getNumCollapsableLevels(RegDDRef *GEPRef) {

  // Struct's access. In the following example, only the first and
  // up to the second dimension may be collapsed because the 3rd and
  // 2nd dimensions are not consecutive.
  // This is shown by the trailing offset (.1) after 3rd dimension.
  // Pankaj's example: A[i1].1[i2][i3]
  // See in which level the first appearing trailing struct offset
  // first appears.
  unsigned Idx = 2;
  for (unsigned End = std::min(GEPRef->getNumDimensions(), NumCollapsableLoops);
       Idx <= End; ++Idx) {
    if (GEPRef->hasTrailingStructOffsets(Idx)) {
      break;
    }
  }
  unsigned NewNumCollapsableLevels = std::min(Idx - 1, NumCollapsableLoops);

  // Examine each applicable dimension and find MAX collapse-able level.
  Idx = 1;
  unsigned LoopLevel = InnermostLevel;
  for (unsigned End =
           std::min(GEPRef->getNumDimensions(), NewNumCollapsableLevels);
       Idx <= End; ++Idx) {

    // (i) CE on Dimension(I) is a StandAloneIV() on its matching loop level
    // and
    // (ii)
    // If:   Current loop has a constant integer trip count,
    // Then: Check the TripCount == #Elements on the matching dimension
    bool IsConstTripLp = UBTCArry[LoopLevel].isConstant();
    CanonExpr *CE = GEPRef->getDimensionIndex(Idx);
    unsigned Level = UINT_MAX;

    // Blob Upperbound: at this moment we do not know
    // the number of elements in terms of blob.
    // So, we do have no way to ensure trip count of the loop
    // is the same as the number of elements. We just bail out.
    //
    // Note that bailing out here do not hinder
    // collapsing in the existence of C99 variable length arrays(VLA)
    // and its number of elements (naturally blob) as upperbounds.
    // Refrences to VLAs are linearized into one dimension, and handled
    // in getLevelsOfIVPattern before this fuction is called.
    // See areGEPRefsLegal().
    bool Valid = IsConstTripLp && CE->isStandAloneIV(true, &Level) &&
                 Level == LoopLevel &&
                 UBTCArry[LoopLevel].getTripCount() ==
                     GEPRef->getNumDimensionElements(Idx);

    if (!Valid) {
      break;
    }
    --LoopLevel; // move toward OutermostLp, match Dimension Increase on Idx
  }

  Idx = Idx - 1; // last valid Idx

  // Check for the dimensions outside [1, Idx], i.e. [idx + 1,
  // getNumDimensions()], These dimesions should not have IV in the range of
  // collapsed loop levels. I.e. [InnermostLevel - (Idx - 1), InnermostLevel].
  // E.g. [i2][i2][i3] is not valid when two innermost loops are collapsed.
  // Note that in a ref [3rd dim][2nd dim][1st dim], but in loop-level
  // [i1][i2][i3].
  bool SeenInvalidIVs = false;
  for (unsigned I = Idx + 1, E = GEPRef->getNumDimensions(); I <= E; I++) {
    CanonExpr *CE = GEPRef->getDimensionIndex(I);
    unsigned KEnd = InnermostLevel - Idx + 1;
    for (unsigned K = InnermostLevel; K >= KEnd; K--) {
      if (CE->hasIV(K)) {
        SeenInvalidIVs = true;
        break;
      }
    }
  }

  return !SeenInvalidIVs ? Idx : 0;
}

unsigned
HIRLoopCollapse::getNumDimensionsOfMatchedSrcAndDestType(RegDDRef *GEPRef) {
  assert(IVType && "IVType must have been set by now\n");

  unsigned Idx = 1;
  for (unsigned End = std::min(GEPRef->getNumDimensions(), NumCollapsableLoops);
       Idx <= End; ++Idx) {
    CanonExpr *CE = GEPRef->getDimensionIndex(Idx);
    if (hasLoopNestIV(CE)) {
      if ((CE->getSrcType() != IVType) || (CE->getDestType() != IVType)) {
        break;
      }
    }
  }
  return Idx - 1;
}

bool HIRLoopCollapse::areNonGEPRefsProfitable(void) {

  // Test on each Non-GEPRef
  for (auto Ref : RefVec) {

    for (auto I = Ref->canon_begin(), E = Ref->canon_end(); I != E; ++I) {
      CanonExpr *CE = (*I);

      // Check: match IVPattern?
      unsigned NumLevels = getLevelsOfIVPattern(CE);

      if (NumLevels <= 1) {
        // Check: Not a IVPattern, but outstanding IVs within CollapseLevels?
        NumLevels = getContinuousSubRanges(CE);
        if (NumLevels <= 1) {
          return false;
        }
      }
      NumCollapsableLoops = std::min(NumCollapsableLoops, NumLevels);
    }
  }
  return true;
}

// clear loop-relevant IV(s) in the given CE
// the range of loop-relevant IVs are given in [LowLpLevel .. HighLpLevel]
static bool clearRelevantIVs(CanonExpr *CE, unsigned HighLpLevel,
                             unsigned LowLpLevel) {
  assert((HighLpLevel >= LowLpLevel) && "LoopLevel Range error\n");

  bool IsRemoved = false;
  for (unsigned Level = LowLpLevel, EndLevel = HighLpLevel; Level <= EndLevel;
       Level++) {
    if (CE->hasIV(Level)) {
      CE->removeIV(Level);
      IsRemoved = true;
    }
  }
  return IsRemoved;
}

// Replace each relevant IV inside GEPRefVec and NonGEPRef
// with the newIV from NewLp.
//
// - Simplify any GEPRef with only 1 dimension:
//   FROM: A[0][i1][i2]
//   TO  : A[0][0 ][i3]
//
// - Simplify any GEPRef with only 1 dimension:
//   FROM: A[N*i1 + i2], where N is innermost Lp's trip count (a blob)
//   TO  : A[i3]
//
static void adjustIVCoeffs(RegDDRef *Ref, unsigned StartDim, unsigned EndDim,
                           unsigned OrigInnermostLevel,
                           unsigned OrigOutermostLevel, bool SetCollapsed) {
  bool IsChanged = false;
  for (unsigned I = StartDim, E = EndDim; I <= E; ++I) {
    // In each valid dimension in range [1 .. E], clear IVs within
    // [OrigInnerLevel .. OrigOuterLv]
    IsChanged = clearRelevantIVs(Ref->getDimensionIndex(I), OrigInnermostLevel,
                                 OrigOutermostLevel) ||
                IsChanged;
  }

  if (!IsChanged) {
    return;
  }

  // set Dimension1 to stand-alone iv:
  Ref->getDimensionIndex(1)->setIVCoeff(OrigOutermostLevel, InvalidBlobIndex,
                                        1);
  Ref->makeConsistent({}, OrigOutermostLevel);
  if (SetCollapsed) {
    Ref->setCollapsed(true);
  }
}

bool HIRLoopCollapse::doTransform(HLLoop *const ToCollapseLp,
                                  const unsigned OrigInnermostLevel,
                                  const unsigned OrigOutermostLevel) {

  // *** Identify relevant loops for collapsing ***

  HLLoop *OrigOutermostLp =
      ToCollapseLp->getParentLoopAtLevel(OrigOutermostLevel);
  LLVM_DEBUG(dbgs() << "Before LoopCollase:\n"; OrigOutermostLp->dump();
             dbgs() << "\n";);

  OrigOutermostLp->extractPreheaderAndPostexit();

  // *** Accumulate and update TripCount ***

  // Base case: from the innermost loop
  CanonExpr *AccumulatedTripCountCE = ToCollapseLp->getUpperCanonExpr();
  AccumulatedTripCountCE->addConstant(1, false);

  // Accumulate TripCount over each loop in the collapse-able loop nest
  // (exclude the Innermost loop)
  for (unsigned Level = OrigInnermostLevel - 1, E = OrigOutermostLevel;
       Level >= E; --Level) {
    if (UBTCArry[Level].isConstant()) {
      AccumulatedTripCountCE->multiplyByConstant(
          UBTCArry[Level].getTripCount());
    } else {
      AccumulatedTripCountCE->multiplyByBlob(
          UBTCArry[Level].getUBCE()->getSingleBlobIndex());
    }
  }

  AccumulatedTripCountCE->addConstant(-1, false);

  // *** DO Collapsing ***
  // Move loop:
  // move InnerLp ahead of OrigOutermostLp, so the new loop-nesting will look
  // like:
  //
  // | InnerLp: 0, N, 1
  // |  BODY;
  //
  // | OrigOutermostLp: 0, M, 1
  // |  potential further nesting
  // |  .....
  // |  | ..|  empty body
  //
  HNU->moveBefore(OrigOutermostLp, ToCollapseLp);

  // Force Ref to be consistent w.r.t. the LoopNest over all available loops'
  // UpperBounds.
  // Add UBDDRefs from the entire LoopNest into the vector
  SmallVector<const RegDDRef *, MaxLoopNestLevel> UpperBoundRefs;
  for (unsigned Level = OrigInnermostLevel, EndLevel = OrigOutermostLevel;
       Level >= EndLevel; --Level) {
    UpperBoundRefs.push_back(LoopNest[Level]->getUpperDDRef());
  }

  auto *UBRef = ToCollapseLp->getUpperDDRef();

  UBRef->makeConsistent(UpperBoundRefs, ToCollapseLp->getNestingLevel());

  // Upper bound may have new temp blobs. Add them as livein to loop.
  for (auto BRefIt = UBRef->blob_begin(), E = UBRef->blob_end(); BRefIt != E;
       ++BRefIt) {
    ToCollapseLp->addLiveInTemp((*BRefIt)->getSymbase());
  }

  // Collapse each relevant Ref from collection:
  // E.g.
  // [FROM]  A[0][i1][i2] = 10*i1+i2;
  // [TO]    A[0][0][i3] = i3;

  for (RegDDRef *GEPRef : GEPRefVec) {
    // Collapse current GEPRef:
    // E.g. regular case
    // [FROM]: A[i][j]
    // [TO]  : A[0][r], where r is CollapsedLp's IV
    //
    // E.g. 1-dimension only case
    // [FROM]: A[i1 + N*i2+i3]
    // [TO]  : A[i1 + r], where r is CollapsedLp's IV, and N is i3's TripCount
    //
    adjustIVCoeffs(GEPRef, 1,
                   std::min(GEPRef->getNumDimensions(), NumCollapsableLoops),
                   OrigInnermostLevel, OrigOutermostLevel, true);
  }

  // Transform each non-GEPRef Ref in RefVec
  for (RegDDRef *Ref : RefVec) {
    // Collapse current non-GEPRef Ref:
    // E.g.: with-Blob case
    // [FROM]: N*i1+i2
    // [TO]  : i1
    // where i1 is the collapsed loop's nesting level, and N is i2's TripCount
    //
    // E.g.: constant-only case
    // [FROM]: 10*i1+i2
    // [TO]  : i1
    // where i1 is the collapsed loop's nesting level, and 10 is i2's
    // TripCount
    //
    adjustIVCoeffs(Ref, 1, 1, OrigInnermostLevel, OrigOutermostLevel, false);
  }

  // Remove the original OrigOutermostLp loop(s):
  HLNodeUtils::remove(OrigOutermostLp);

  // Mark the loop and its parent loop/region have been changed
  // Note: HIRSafeReductionAnalysis is preserved
  ToCollapseLp->getParentRegion()->setGenCode();
  HIRInvalidationUtils::invalidateBody<HIRSafeReductionAnalysis>(ToCollapseLp);
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion<
      HIRSafeReductionAnalysis>(ToCollapseLp);

  ++HIRLoopNestsCollapsed;

  LoopOptReportBuilder &LORBuilder =
      ToCollapseLp->getHLNodeUtils().getHIRFramework().getLORBuilder();

  LORBuilder(*ToCollapseLp)
      .addRemark(OptReportVerbosity::Low, "%d loops have been collapsed",
                 NumCollapsableLoops);

  LLVM_DEBUG(dbgs() << "After Collapse:\n"; ToCollapseLp->dump();
             dbgs() << "\n";);

  return true;
}

unsigned HIRLoopCollapse::getContinuousSubRanges(const CanonExpr *CE) const {
  int64_t IVConstCoeff;
  unsigned IVIndex, Count = 0;

  for (unsigned Level = InnermostLevel, EndLevel = getOutermostLevel();
       Level >= EndLevel; --Level) {
    CE->getIVCoeff(Level, &IVIndex, &IVConstCoeff);

    // As long as the current Level IV is seen in the CE, bail out right away!
    if (IVConstCoeff) {
      break;
    }
    ++Count;
  }
  return Count;
}

bool HIRLoopCollapse::hasLoopNestIV(const CanonExpr *CE) const {
  for (unsigned Level = getOutermostLevel(); Level <= InnermostLevel; ++Level) {
    if (CE->hasIV(Level)) {
      return true;
    }
  }
  return false;
}

void HIRLoopCollapse::clearWorkingSetMemory(void) {
  RefVec.clear();
  GEPRefVec.clear();
  NumCollapsableLoops = 0;
  initializeUBTCArry();
  IVType = nullptr;
}

unsigned HIRLoopCollapse::getLevelsOfIVPattern(CanonExpr *CE) const {

  int64_t IVConstCoeff;
  unsigned IVIndex;
  unsigned LevelsMatched = 0;

  // Try to match the 1st level (on InnermostLevel):
  CE->getIVCoeff(InnermostLevel, &IVIndex, &IVConstCoeff);
  if ((IVConstCoeff != 1) || (IVIndex != InvalidBlobIndex)) {
    return 0;
  }
  unsigned AccumuConst = IVConstCoeff;
  unsigned AccumuBlobIndex = IVIndex;
  ++LevelsMatched;

  // Try to match each applicable level within [InnermostLevel-1 .. EndLevel]
  for (unsigned Level = InnermostLevel - 1, EndLevel = getOutermostLevel();
       Level >= EndLevel; --Level) {

    // Obtain: data from CE on current Level
    CE->getIVCoeff(Level, &IVIndex, &IVConstCoeff);
    unsigned PrevLevel = Level + 1;

    if (UBTCArry[PrevLevel].isConstant()) {
      AccumuConst *= UBTCArry[PrevLevel].getTripCount();
    } else {
      CanonExpr *CurUBCE = UBTCArry[PrevLevel].getUBCE();
      unsigned BlobIndex = CurUBCE->getSingleBlobIndex();

      // Accumulate Blob into AccumuBlobIndex:
      if (AccumuBlobIndex == InvalidBlobIndex) {
        AccumuBlobIndex = BlobIndex;
      } else {
        unsigned NewBlobIndex = 0;
        BU->createMulBlob(BU->getBlob(AccumuBlobIndex), BU->getBlob(BlobIndex),
                          true, &NewBlobIndex);

        AccumuBlobIndex = NewBlobIndex;
      }
    }

    // Compare: match CE on current level?
    if (IVConstCoeff != AccumuConst || IVIndex != AccumuBlobIndex) {
      break;
    }

    // Increase the levels matched
    ++LevelsMatched;
  }

  return LevelsMatched;
}

#ifndef NDEBUG
LLVM_DUMP_METHOD void HIRLoopCollapse::printCollectedRefVec(
    bool PrintGEPRefVec, bool PrintNonGEPRefVec, bool NewLine) const {
  unsigned NumRVals = 0, NumLVals = 0;
  unsigned GEPRefSize = GEPRefVec.size(), RefSize = RefVec.size();

  // Check: is the collection empty?
  if (GEPRefSize + RefSize == 0) {
    dbgs() << "Empty";
    if (NewLine) {
      dbgs() << "\n";
    }
    return;
  }

  // Print each item in RefVec:
  if (PrintNonGEPRefVec) {
    dbgs() << "Non-GEPRefs: " << RefSize << ": {";
    for (auto &Ref : RefVec) {
      Ref->dump();

      // Print: (L) or (W)
      if (Ref->isLval()) {
        dbgs() << "(LVal)";
        ++NumLVals;
      } else {
        dbgs() << "(RVal)";
        ++NumRVals;
      }
      dbgs() << ", ";
    }
    dbgs() << "}\n";
  }

  // Print each item in GEPRefVec:
  if (PrintGEPRefVec) {
    dbgs() << "GEPRefs: " << GEPRefSize << ": {";
    for (auto &GEPRef : GEPRefVec) {
      GEPRef->dump();

      // Print: (L) or (W)
      if (GEPRef->isLval()) {
        dbgs() << "(LVal)";
        ++NumLVals;
      } else {
        dbgs() << "(RVal)";
        ++NumRVals;
      }
      dbgs() << ", ";
    }
    dbgs() << "} ";
  }

  // Print: NumRVals and NumLVals
  dbgs() << NumRVals << " RVal(s) " << NumLVals << " LVal(s) ";

  if (NewLine) {
    dbgs() << "\n";
  }
}

// Print CandidateLoops;
LLVM_DUMP_METHOD void HIRLoopCollapse::printCandidateLoops(
    SmallVector<InnerOuterLoopPairTy, 12> &CandidateLoops) const {

  unsigned Size = CandidateLoops.size();
  if (!Size) {
    dbgs() << "Empty CandidateLoops\n";
  } else {
    dbgs() << "Collected CandidateLoops: " << Size
           << " InnerOuterLoopPair(s), only print OutermostLp\n";
  }

  for (auto &LPPair : CandidateLoops) {
    HLLoop *OutermostLp = LPPair.first;
    assert(OutermostLp && "OutermostLp can't be null\n");

    HLLoop *InnermostLp = LPPair.second;
    assert(InnermostLp && "InnermostLp can't be null\n");

    dbgs() << "OutermostLp: \n";
    OutermostLp->dump();
    dbgs() << "\n";
  }
}

LLVM_DUMP_METHOD void HIRLoopCollapse::printUBTCArry(void) const {
  for (auto &Item : UBTCArry) {
    Item.print(true, true);
  }
}

#endif

PreservedAnalyses HIRLoopCollapsePass::run(llvm::Function &F,
                                           llvm::FunctionAnalysisManager &AM) {
  HIRLoopCollapse(AM.getResult<HIRFrameworkAnalysis>(F)).run();
  return PreservedAnalyses::all();
}

namespace {

class HIRLoopCollapseLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRLoopCollapseLegacyPass() : HIRTransformPass(ID) {
    initializeHIRLoopCollapseLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F)) {
      LLVM_DEBUG(dbgs() << "HIR Loop Collapse Skipped\n");
      return false;
    }

    return HIRLoopCollapse(getAnalysis<HIRFrameworkWrapperPass>().getHIR())
        .run();
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
  }
};

} // namespace

char HIRLoopCollapseLegacyPass::ID = 0;

INITIALIZE_PASS_BEGIN(HIRLoopCollapseLegacyPass, "hir-loop-collapse",
                      "HIR Loop Collapse", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_END(HIRLoopCollapseLegacyPass, "hir-loop-collapse",
                    "HIR Loop Collapse", false, false)

FunctionPass *llvm::createHIRLoopCollapsePass() {
  return new HIRLoopCollapseLegacyPass();
}
