//==--- HIRLoopCollapse.cpp -Implements Loop Collapse Pass -*- C++ -*---===//
//
// Copyright (C) 2015-2021 Intel Corporation. All rights reserved.
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
#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopCollapsePass.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/DDGraph.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#include "HIRLoopCollapse.h"

#define DEBUG_TYPE "hir-loop-collapse"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::collapse;

// Disable the HIR Loop Collapse Optimization (default is false:enabled)
static cl::opt<bool>
    DisableHIRLoopCollapse("disable-hir-loop-collapse", cl::init(false),
                           cl::Hidden,
                           cl::desc("Disable HIR Loop Collapse (HLC)"));

// Disable dynamic-shape array support.
// -default is false: allow dynamic-shape array matching.
static cl::opt<bool>
    DisableDynShapeArray("disable-dynshape-array", cl::init(false), cl::Hidden,
                         cl::desc("Disable Dynamic Shape Array Support"));

STATISTIC(HIRLoopNestsCollapsed, "Number of HIR LoopNest(s) Collapsed");

// ** Ref Collector **
// - collect all LoopNest-IV relevant GEPRef into GEPRefVec
// - collect all LoopNest-IV relevant non-GEPRef Ref into RefVec
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
    for (auto I = Node->ddref_begin(), E = Node->ddref_end(); I != E; ++I) {
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

  void collectRef(RegDDRef *Ref);

  void print(raw_ostream &OS, bool PrintDetail) const {
    // Non-GEP RefVec:
    unsigned const RefVecSize = RefVec.size();
    OS << "Non-GEP RefVec: " << RefVecSize << "\n";
    if (RefVecSize) {
      unsigned Count = 0;
      for (auto &Ref : RefVec) {
        OS << Count++ << " : ";
        StringRef RW = (Ref->isLval()) ? " W " : " R ";
        OS << RW;
        Ref->dump(PrintDetail);
        OS << "\n";
      }
    }

    // GEPRef Vec:
    unsigned const GEPRefVecSize = GEPRefVec.size();
    OS << "GEP RefVec: " << GEPRefVecSize << "\n";
    if (GEPRefVecSize) {
      unsigned Count = 0;
      for (auto &Ref : GEPRefVec) {
        OS << Count++ << " : ";
        StringRef RW = (Ref->isLval()) ? " W " : " R ";
        OS << RW;
        Ref->dump(PrintDetail);
        OS << "\n";
      }
    }
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(bool PrintDetail = false) const {
    formatted_raw_ostream FOS(dbgs());
    print(FOS, PrintDetail);
  }
#endif
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
  // forming a perfect (sub) LoopNest.
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
  // InnerOuterLoopPair candidate.
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
  if (!doAnalysis(InnermostLp)) {
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

bool HIRLoopCollapse::doAnalysis(HLLoop *InnermostLp) {
  if (!doPreliminaryChecks()) {
    LLVM_DEBUG(dbgs() << "HIRLoopCollapse::doPreliminaryChecks() failed\n");
    return false;
  }

  if (!doCollection()) {
    LLVM_DEBUG(dbgs() << "HIRLoopCollapse::doCollection() failed\n");
    return false;
  }

  if (!areGEPRefsLegal(InnermostLp)) {
    LLVM_DEBUG(dbgs() << "HIRLoopCollapse::areGEPRefsLegal(.) failed\n");
    return false;
  }

  if (!areNonGEPRefsProfitable()) {
    LLVM_DEBUG(dbgs() << "HIRLoopCollapse::areNonGEPRefsProfitable() failed\n");
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
        CurLp->hasVectorizeEnablingPragma() || CurLp->isMVFallBack()) {
      break;
    }

    unsigned LoopLevel = CurLp->getNestingLevel();

    // Check: does CurLp have a constant TripCount?
    // If yes: save its trip count into TCArry;
    if (CurLp->isConstTripLoop(&TripCount)) {
      TCArry[LoopLevel].set(TripCount);
      continue;
    }

    CanonExpr *UB = CurLp->getUpperCanonExpr();
    if (UB->canConvertToStandAloneBlobOrConstant()) {
      CanonExpr *TC = CurLp->getTripCountCanonExpr();
      const bool CanConvert = TC->convertToStandAloneBlobOrConstant();
      assert(CanConvert && "Expect a good conversion");
      (void)CanConvert;
      TCArry[LoopLevel].set(TC);
      continue;
    }

    // OTHER CASES ARE NOT HANDLED
    break;
  }

  // Check: not allow preheader or postexit for the outer-most loop
  unsigned OuterLevel = getOutermostLevel();
  HLLoop *OuterLp = InnermostLp->getParentLoopAtLevel(OuterLevel);
  if (OuterLp->hasPreheader() || OuterLp->hasPostexit())
    return false;

  // Check: NumcollapsableLoops to be 2+
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
  LLVM_DEBUG(Collector.dump(););

  // Check if there is at least 1 GEPRef available after collection.
  //
  // [Note]
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
bool HIRLoopCollapse::areGEPRefsLegal(HLLoop *InnerLp) {
  const unsigned InnerLpLevel = InnerLp->getNestingLevel();

  for (RegDDRef *GEPRef : GEPRefVec) {
    if (GEPRef->isFake())
      continue;

    const unsigned NumDims = GEPRef->getNumDimensions();
    unsigned CollapseLevel = (NumDims == 1)
                                 ? matchSingleDimDynShapeArray(GEPRef)
                                 : getNumMatchedDimensions(GEPRef);

    NumCollapsableLoops = std::min(NumCollapsableLoops, CollapseLevel);

    // Multi-dim array:
    if (NumDims > 1) {
      // Try: match GEPRef with dynamic-shape pattern
      int Level = matchMultiDimDynShapeArray(GEPRef, InnerLpLevel);

      if (Level == -1) {
        return false;
      }
      CollapseLevel = Level;

      // If matchMultiDimDynShapeArray() returns 0 or 1, the dyn match failed.
      // Need to count collapsable levels on Ref as a normal ref.
      // E.g.
      // -----------------------------
      // GEPRef        |CollapseLevel |
      // -----------------------------
      // A[ 0][ 0][i1] |  1           |
      // A[ 0][i1][i2] |  2           |
      // A[i1][i2][i3] |  3           |
      // -----------------------------
      if (CollapseLevel < 2) {
        CollapseLevel = getNumCollapsableLevels(GEPRef);
      }

      NumCollapsableLoops = std::min(NumCollapsableLoops, CollapseLevel);
    }
  }

  return NumCollapsableLoops > 1;
}

// For any partial match: check for anomaly in any unmatched dimension(s).
//
// E.g.
// |i1:
// ||i2:
// ||    . = A[i1][i2][i1][i2];
// ||          ^   ^
// ||         (1)  (2)
// ||    ...
//
// This function should return false.
//
// Return: bool
// - true: if there is NO anomaly in any un-match dimension
//         - the unmatched dimensions are all good.
// - false: otherwise.
//
static bool hasValidUnmatchedDims(RegDDRef *Ref, unsigned MatchedDims,
                                  const unsigned OutermostLoopLevel) {
  const unsigned DimMax = Ref->getNumDimensions();
  assert((MatchedDims >= 2) && (MatchedDims <= DimMax));

  // Check: in any non-matched dimension index -- CE, expect any call to
  // CE->isInvariantAtLevel(OutermostLoopLevel) is true.
  //
  // E.g.
  // | i1
  // || i2
  // ||    . = A[i1][i2][i1][i2];
  //             ^   ^
  //            (1) (2)
  // The partial match recognized 2 dimensions (dim1-dim2). Consider the
  // high-dimension CEs in both (1) and (2), if any CE->isInvariantAtLevel(1)
  // is not true, bail out and return 0 as the final result.
  //
  // [Note]
  // - 1 is the outermost loop level (i1).
  //
  for (unsigned I = MatchedDims + 1; I <= DimMax; ++I) {
    if (Ref->getDimensionIndex(I)->isInvariantAtLevel(OutermostLoopLevel) ==
        false) {
      return false;
    }
  }

  return true;
}

// ------------------------------
// GEPRef        |CollapseLevel |
// ------------------------------
// A[ 0][ 0][i1] |  1           |
// A[ 0][i1][i2] |  2           |
// A[i1][i2][i3] |  3           |
// ------------------------------
unsigned HIRLoopCollapse::getNumCollapsableLevels(RegDDRef *GEPRef) {
  unsigned NewNumCollapsableLevels =
      std::min(GEPRef->getNumDimensions(), NumCollapsableLoops);

  // Examine dimensions pairwise and find MAX collapse-able level.
  unsigned Idx = 2;

  for (unsigned InnerLoopLevel = InnermostLevel; Idx <= NewNumCollapsableLevels;
       ++Idx, --InnerLoopLevel) {

    // This dimension is collapse-able with the lower dimension if-
    // 1) It is contiguous with the lower dimension, and
    // 2) The index of inner dimension is a standalone IV, and
    // 3) The index of outer dimension has outer standalone IV, and
    // 4) The trip count of inner loop matches the number of elements in the
    //    lower dimension.

    unsigned IVBlob = 0;
    int64_t IVCoeff = 0;

    auto *OuterIdxCE = GEPRef->getDimensionIndex(Idx);
    OuterIdxCE->getIVCoeff(InnerLoopLevel - 1, &IVBlob, &IVCoeff);

    bool OuterDimCollapsible = !GEPRef->hasTrailingStructOffsets(Idx) &&
                               IVBlob == InvalidBlobIndex && IVCoeff == 1 &&
                               OuterIdxCE->getDenominator() == 1 &&
                               OuterIdxCE->isInvariantAtLevel(InnerLoopLevel);

    if (!OuterDimCollapsible) {
      LLVM_DEBUG(dbgs() << "Dimension number " << Idx
                        << " is illegal to collapse\n";);
    }

    unsigned InnerDimIVLevel = UINT_MAX;
    bool InnerDimCollapsible =
        GEPRef->getDimensionIndex(Idx - 1)->isStandAloneIV(true,
                                                           &InnerDimIVLevel) &&
        (InnerDimIVLevel == InnerLoopLevel);

    bool Collapsable = OuterDimCollapsible && InnerDimCollapsible &&
                       TCArry[InnerLoopLevel].isConstant() &&
                       (TCArry[InnerLoopLevel].getConstTripCount() ==
                        GEPRef->getNumDimensionElements(Idx - 1));

    if (!Collapsable) {
      break;
    }
  }

  NewNumCollapsableLevels = Idx - 1; // last collapsable level.

  if (NewNumCollapsableLevels < 2) {
    return 0;
  }

  // Check the dimensions outside [1, Idx], i.e. [idx+1, getNumDimensions()].
  // These dimensions should not have IV in the range of collapsed loop levels.
  // I.e. [OutermostCollapsableLevel, InnermostLevel].
  // E.g. [i2][i2][i3] is not valid when two innermost loops are collapsed.
  // Note that in a ref [3rd dim][2nd dim][1st dim], but in loop-level
  // [i1][i2][i3].
  const unsigned OutermostCollapsableLevel =
      InnermostLevel - NewNumCollapsableLevels + 1;

  return hasValidUnmatchedDims(GEPRef, NewNumCollapsableLevels,
                               OutermostCollapsableLevel)
             ? NewNumCollapsableLevels
             : 0;
}

unsigned HIRLoopCollapse::getNumMatchedDimensions(RegDDRef *GEPRef) {
  assert(IVType && "IVType must have been set by now");
  assert(GEPRef->getNumDimensions() > 1 && "Expect a multi-dimension Ref");

  unsigned Idx = 1;
  for (unsigned End = std::min(GEPRef->getNumDimensions(), NumCollapsableLoops);
       Idx <= End; ++Idx) {
    CanonExpr *CE = GEPRef->getDimensionIndex(Idx);
    if ((CE->getSrcType() != IVType) || (CE->getDestType() != IVType)) {
      break;
    }
  }

  return Idx - 1;
}

bool HIRLoopCollapse::areNonGEPRefsProfitable(void) {

  for (auto Ref : RefVec) {
    if (Ref->isFake())
      continue;

    for (auto I = Ref->canon_begin(), E = Ref->canon_end(); I != E; ++I) {
      CanonExpr *CE = (*I);

      // Check: match IVPattern?
      unsigned NumLevels = matchCEOnIVLevels(CE);

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

// Clear loop-relevant IV(s) in the given CE,
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

// Replace each relevant IV inside GEPRefVec and NonGEPRef with the newIV from
// NewLp.
//
// - Simplify any GEPRef with multiple dimensions:
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

static void updateProfDataforCollapsedLoop(HLLoop *OutermostLp,
                                           HLLoop *CollapsedLp) {
  // No Profile Data available
  if (!OutermostLp->getProfileData() || !CollapsedLp->getProfileData())
    return;

  uint64_t OuterTrueWeight, OuterFalseWeight, InnerTrueWeight, InnerFalseWeight;
  OutermostLp->extractProfileData(OuterTrueWeight, OuterFalseWeight);
  CollapsedLp->extractProfileData(InnerTrueWeight, InnerFalseWeight);

  CollapsedLp->setProfileData(InnerTrueWeight, OuterFalseWeight);
}

// Collect all symbase(s) from a given Ref, and put the results into ZTTLivIns.
static void collectSymbase(RegDDRef *Ref, SmallSet<unsigned, 8> &ZTTLiveIns) {
  if (Ref->isConstant())
    return;

  if (Ref->isSelfBlob()) {
    ZTTLiveIns.insert(Ref->getSymbase());
  } else {
    for (const BlobDDRef *BRef :
         make_range(Ref->blob_begin(), Ref->blob_end())) {
      ZTTLiveIns.insert(BRef->getSymbase());
    }
  }
}

// Extract ZTT on each relevant loop level, and also save ZTTLiveIns.
static void moveZttLiveIn(HLLoop *InnermostLp, unsigned OrigInnermostLevel,
                          unsigned OrigOutermostLevel,
                          SmallVectorImpl<PredicateTuple> &ZTTs,
                          SmallSet<unsigned, 8> &ZTTLiveIns) {

  assert(InnermostLp->getNestingLevel() == OrigInnermostLevel);

  for (unsigned Level = OrigInnermostLevel; Level >= OrigOutermostLevel;
       --Level) {
    HLLoop *Lp = InnermostLp->getParentLoopAtLevel(Level);
    HIRTransformUtils::cloneOrRemoveZttPredicates(Lp, ZTTs, false /* remove */);
    Lp->removeZtt();
  }

  // Collect symbase(s) from each available ZTT:
  for (auto &ZTT : ZTTs) {
    collectSymbase(ZTT.Op1, ZTTLiveIns);
    collectSymbase(ZTT.Op2, ZTTLiveIns);
  }
}

static void mergeZttLiveIn(HLLoop *ToCollapseLp,
                           SmallVectorImpl<PredicateTuple> &ZTTs,
                           SmallSet<unsigned, 8> &ZTTLiveIns) {
  HIRTransformUtils::mergeZtt(ToCollapseLp, ZTTs);

  for (unsigned LiveIn : ZTTLiveIns) {
    ToCollapseLp->addLiveInTemp(LiveIn);
  }
}

// For lexically backward dependence in i1 (<), both flow and output dep
// as in this example:
//
// DO  i1=1,N
//  DO  i2=1,4
//        A[i1+1][i2] = A[i1][i2]                           (< =) (1 0)
//  After collapsing, we have  A[2][i1] = A[1][i1]
//  Assuming INDEP is incorrect.  DD is expected to create  (<)   (4)
//  Need to save the trip count in DDREF.
//  The loop can be vectorized with vectcr Length <= 4.
//  If the UB of i2 is non-constant, use 2 as it is unlikely that user wrote
//  code, which uses a dimension size/extent of 1 using a variable.
//
//  No issue with forward flow/output dep in i1.
void HIRLoopCollapse::setMaxVecLenAllowed(HLLoop *const OrigOutermostLp,
                                          const unsigned OrigInnermostLevel,
                                          const unsigned OrigOutermostLevel) {
  const DDGraph &DDG = DDA.getGraph(OrigOutermostLp);

  for (RegDDRef *Ref : GEPRefVec) {
    if (!Ref->isLval())
      continue;

    for (auto &Edge : DDG.outgoing(Ref)) {
      auto *SinkRef = cast<RegDDRef>(Edge->getSink());
      if (HLNodeUtils::dominates(SinkRef->getHLDDNode(), Ref->getHLDDNode())) {
        if (Edge->getDVAtLevel(OrigOutermostLevel) == DVKind::LT) {
          unsigned TC = 2;
          if (TCArry[OrigInnermostLevel].isConstant()) {
            TC = TCArry[OrigInnermostLevel].getConstTripCount();
          }
          for (unsigned I = OrigInnermostLevel - 1; I > OrigOutermostLevel;
               I--) {
            if (TCArry[I].isConstant()) {
              TC *= TCArry[I].getConstTripCount();
            }
          }
          Ref->setMaxVecLenAllowed(TC);
          SinkRef->setMaxVecLenAllowed(TC);
        }
      }
    }
  }
}

bool HIRLoopCollapse::doTransform(HLLoop *const ToCollapseLp,
                                  const unsigned OrigInnermostLevel,
                                  const unsigned OrigOutermostLevel) {

  HLLoop *OrigOutermostLp =
      ToCollapseLp->getParentLoopAtLevel(OrigOutermostLevel);
  LLVM_DEBUG({
    dbgs() << "Before HIR Loop Collapse:\n";
    OrigOutermostLp->dump();
    dbgs() << "\n";
  });

  setMaxVecLenAllowed(OrigOutermostLp, OrigInnermostLevel, OrigOutermostLevel);

  //  Save UBs to be passed to makeConsistent
  //  Make a copy to avoid overlay

  SmallVector<const RegDDRef *, MaxLoopNestLevel> UpperBoundRefs;
  for (unsigned Level = OrigInnermostLevel, EndLevel = OrigOutermostLevel;
       Level >= EndLevel; --Level) {
    UpperBoundRefs.push_back((LoopNest[Level]->getUpperDDRef())->clone());
  }

  OrigOutermostLp->extractPreheaderAndPostexit();
  SmallVector<PredicateTuple, 8> ZTTs;
  SmallSet<unsigned, 8> ZTTLiveInSBs;
  moveZttLiveIn(ToCollapseLp, OrigInnermostLevel, OrigOutermostLevel, ZTTs,
                ZTTLiveInSBs);

  // *** Accumulate and update TripCount ***

  // Base case: from the innermost loop

  CanonExpr *AccumulatedTripCountCE = ToCollapseLp->getUpperCanonExpr();

  AccumulatedTripCountCE->addConstant(1, false);

  // Accumulate TripCount over each loop in the collapse-able loop nest
  // (exclude the Innermost loop)
  for (unsigned Level = OrigInnermostLevel - 1, E = OrigOutermostLevel;
       Level >= E; --Level) {
    if (TCArry[Level].isConstant()) {
      AccumulatedTripCountCE->multiplyByConstant(
          TCArry[Level].getConstTripCount());
    } else {
      CanonExpr *TCCE = TCArry[Level].getTripCount();
      AccumulatedTripCountCE->multiplyByBlob(TCCE->getSingleBlobIndex());
    }
  }

  AccumulatedTripCountCE->addConstant(-1, false);

  auto *UBRef = ToCollapseLp->getUpperDDRef();
  UBRef->makeConsistent(UpperBoundRefs, OrigOutermostLevel);

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

  // Upper bound may have new temp blobs. Add them as livein to loop.
  for (auto BRefIt = UBRef->blob_begin(), E = UBRef->blob_end(); BRefIt != E;
       ++BRefIt) {
    ToCollapseLp->addLiveInTemp((*BRefIt)->getSymbase());
  }

  // Collapse each relevant Ref from collection:
  // E.g.
  // [FROM]  A[0][i1][i2] = 10*i1+i2;
  // [TO]    A[0][0][i3]  = i3;

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

  // Transform each non-GEP Ref in RefVec:
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

  mergeZttLiveIn(ToCollapseLp, ZTTs, ZTTLiveInSBs);

  // Remove the original OrigOutermostLp loop(s):
  HLNodeUtils::remove(OrigOutermostLp);

  // Mark the loop and its parent loop/region have been changed
  // Note: HIRSafeReductionAnalysis is preserved
  ToCollapseLp->getParentRegion()->setGenCode();
  HIRInvalidationUtils::invalidateBody<HIRSafeReductionAnalysis>(ToCollapseLp);
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion<
      HIRSafeReductionAnalysis>(ToCollapseLp);

  // Update Profile Data here.
  // The new collapsed loop uses the loopexit count of the outer loop
  // while keeping the backedge count of the inner loop. In the example
  // collapsing i2-i3, would modify the i2 backedge count to 1000, while
  // keeping the i2 loopexit count of 10.
  // E.g.:  DO i1 = 0, 9, 1     !{!"branch_weights", 10, 1}
  //          DO i2 = 0, 9, 1     !{!"branch_weights", 100, 10}
  //            DO i3 = 0, 9, 1     !{!"branch_weights", 1000, 100}
  //              ...
  //            END LOOP
  //          END LOOP
  //        END LOOP
  //
  updateProfDataforCollapsedLoop(OrigOutermostLp, ToCollapseLp);
  ++HIRLoopNestsCollapsed;

  OptReportBuilder &ORBuilder =
      ToCollapseLp->getHLNodeUtils().getHIRFramework().getORBuilder();

  // ID: 25567u, remark string: "%d loops have been collapsed"
  ORBuilder(*ToCollapseLp)
      .addRemark(OptReportVerbosity::Low, 25567u, NumCollapsableLoops);

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
  initializeTCArry();
  IVType = nullptr;
}

unsigned HIRLoopCollapse::matchSingleDimDynShapeArray(RegDDRef *Ref) {
  if (DisableDynShapeArray) {
    LLVM_DEBUG(dbgs() << "Dynamic-shape array not supported\n";);
    return 0;
  }

  if ((uint64_t)Ref->getDimensionConstStride(1) !=
      Ref->getSrcTypeSizeInBytes()) {
    LLVM_DEBUG(
        dbgs() << "Expect Ref's Stride be a positive integer constant in "
                  "Dimension 1\n");
    return false;
  }

  // Match Ref's dimension-1 index:
  return matchCEOnIVLevels(Ref->getDimensionIndex(1));
}

// Match: a multi-dimension dynamic-shape RegDDRef similar to
// (%A)[0:i1:8*(%N2*%N1)(double*:0)][0:i2:8*%N1(double*:0)][0:i3:8(double*:0)].
//
// [Rules]
// On any dimension of the Ref:
// - LB: may not necessarily be 0
// - index: stand-alone IV
// - stride:
//   . dim1: a positive integer constant, value: size of (src element type)
//   . any other dim: accumulated, stride_i = stride_(i+1) * TripCount(i+1)
//
// Return:
// - a positive integer: the number of matched dimensions
// - 0: match failure, nothing matched
// - -1: strides not multiple of previous one, need to bail out from collapsing
//

int HIRLoopCollapse::matchMultiDimDynShapeArray(RegDDRef *Ref, unsigned Level) {
  if (DisableDynShapeArray) {
    LLVM_DEBUG(dbgs() << "Dynamic-shape array not supported\n";);
    return 0;
  }

  LLVM_DEBUG(dbgs() << "Ref: "; Ref->dump(); dbgs() << "\n";);

  // Special case for dimension1:
  unsigned Dim = 1;

  // Index: expect a standalone IV, with IV Level matching dimension
  // E.g.
  // | i1:
  // | | i2
  // | | | i3:
  // | | | ...
  // | | |  . = A[i1][i2][i3] .
  // | | | ...
  //
  // The following cases are invalid:
  // A[i1][i1][i1], A[i3][i1][i2], A[i1][i2][i3+i1], etc.
  //
  // [Note]
  // -stride: expect an integer constant with value equal to sizeof(src type)
  //
  //  Normally strides are multiples of one another, except F90 pointers
  //  e.g.
  //  real*8, target   ::  A(5,5)
  //  real*8, pointer  ::  P(:,:)
  //  P  => A(1:5:2, 1:4)
  //  P  = 1.0
  //  For the pointer reference here, after constant prop
  //  P becomes A(1:5:2, 1:4) with strides  16, 40
  //  40 is not a multiple of 16. No collapsing can be done.

  const int64_t SrcTypeSize = Ref->getSrcTypeSizeInBytes();
  if (Ref->getDimensionConstStride(Dim) != SrcTypeSize) {
    LLVM_DEBUG(dbgs() << "Expect Dim1 stride be an integer constant with value "
                         "equal to src type size\n");
    return -1;
  }

  // Expect: Index is a standlone IV on Level:
  CanonExpr *IndexCE = Ref->getDimensionIndex(Dim);
  LLVM_DEBUG(dbgs() << "IndexCE: "; IndexCE->dump(); dbgs() << "\n";);
  unsigned CurLevel = 0;
  if (!IndexCE->isStandAloneIV(false, &CurLevel) || (Level != CurLevel)) {
    return 0;
  }

  // Initialize the CarryBlob:
  unsigned CarryBlob = -1;
  BU->createBlob(SrcTypeSize, Ref->getDimensionStride(Dim)->getDestType(),
                 true /* insert */, &CarryBlob);
  //[Note]
  // - the pattern is accumulating multiplications in blob form.
  LLVM_DEBUG({
    dbgs() << "CarryBlob - index: " << CarryBlob << ",\tBlob Print: ";
    BU->printBlob(dbgs(), BU->getBlob(CarryBlob));
    dbgs() << "\n";
  });

  // On any non-dim1 dimension:
  // - Stride: stride_i = stride_(i+1) * TripCount(i+1)
  //

  const unsigned DimMax = Ref->getNumDimensions();

  int64_t PrevStride = 0;

  CanonExpr *Stride = Ref->getDimensionStride(1);
  if (Stride->isConstant()) {
    PrevStride = Stride->getConstant();
  }

  // Use a new loop to scan for constant strides to see if one is
  // a multiple of the previous because the large loop below exits
  // after hitting a constant stride.
  // For symbolic strides, current logic collapses the innermost 2 levels in
  // this example, which is correct and optimal
  // real*8, target   ::  A(5,5,5,5)
  // real*8, pointer  ::  P(:,:,:,:)
  // P => A(:,:,1:5:N,:)
  // P = 1
  //

  CurLevel = Level;
  for (Dim = 2; Dim <= DimMax; ++Dim) {
    --CurLevel;
    IndexCE = Ref->getDimensionIndex(Dim);
    // Use same logic as the loop below, when hitting a structure reference that
    // has no IV skip checking. The Strides will no longer be a multiple
    //
    // type S1
    //  real*8 A(10,20,30)
    //  real*4 B
    // end type S1
    // type (S1)  Structure
    // Structure%A = 1.0
    // (%"sub_$STRUCTURE")[0:0:48008(%"SUB$.btS1"*:0)].0
    //                    [0:i1:1600([30 x [20 x [10 x double]]]:30)]
    //                    [0:i2:80([20 x [10 x double]]:20)]
    //                    [0:i3:8([10 x double]:10)]
    unsigned TheLevel = 0;
    if (!IndexCE->isStandAloneIV(false, &TheLevel) || (TheLevel != CurLevel)) {
      break;
    }

    CanonExpr *Stride = Ref->getDimensionStride(Dim);
    if (Stride->isConstant()) {
      int64_t StrideVal = Stride->getConstant();
      if (PrevStride && (StrideVal % PrevStride) != 0) {
        return -1;
      }
      PrevStride = StrideVal;
    } else {
      break;
    }
  }

  CurLevel = Level;
  for (Dim = 2; Dim <= DimMax; ++Dim) {
    --CurLevel;

    // Expect: Index is a standlone IV on CurLevel
    IndexCE = Ref->getDimensionIndex(Dim);
    LLVM_DEBUG(dbgs() << "IndexCE: "; IndexCE->dump(); dbgs() << "\n";);
    unsigned TheLevel = 0;
    if (!IndexCE->isStandAloneIV(false, &TheLevel) || (TheLevel != CurLevel)) {
      break;
    }

    // Expect: Stride not be a constant
    CanonExpr *Stride = Ref->getDimensionStride(Dim);
    LLVM_DEBUG(dbgs() << "StrideCE: "; Stride->dump(); dbgs() << "\n";);
    if (Stride->isConstant()) {
      LLVM_DEBUG(dbgs() << "Not expect Stride be a constant for dimension "
                        << Dim << "\n";);
      break;
    }

    // Expect: Stride can convert into a single stand-alone blob
    std::unique_ptr<CanonExpr> StrideCE(Stride->clone());
    const bool CanConvert = StrideCE->convertToStandAloneBlobOrConstant();
    assert(CanConvert && "Expect a good conversion");
    (void)CanConvert;

    unsigned StrideBlobIndex = StrideCE->getSingleBlobIndex();
    LLVM_DEBUG({
      dbgs() << "StrideBlobIndex - index: " << StrideBlobIndex
             << ",\tBlob Print: ";
      BU->printBlob(dbgs(), BU->getBlob(StrideBlobIndex));
      dbgs() << "\n";
    });

    // Handle TC as either a constant integer or a blob:
    unsigned TCBlobIndex = -1;
    if (TCArry[CurLevel + 1].isConstant()) {
      BU->createBlob(TCArry[CurLevel + 1].getConstTripCount(),
                     Ref->getDimensionStride(Dim)->getDestType(),
                     true /* insert */, &TCBlobIndex);
    } else {
      TCBlobIndex = TCArry[CurLevel + 1].getTripCount()->getSingleBlobIndex();
    }

    LLVM_DEBUG({
      dbgs() << "\nTCBlobIndex - index: " << TCBlobIndex << ",\tBlob Print: ";
      BU->printBlob(dbgs(), BU->getBlob(TCBlobIndex));
      dbgs() << "\n";
    });

    // Compute the accumulated stride: stride_i = stride_(i+1) * TC(i+1)
    unsigned NewBlobIndex = 0;
    BU->createMulBlob(BU->getBlob(CarryBlob), BU->getBlob(TCBlobIndex), true,
                      &NewBlobIndex);
    LLVM_DEBUG({
      dbgs() << "\nNewBlobIndex: ";
      BU->printBlob(dbgs(), BU->getBlob(NewBlobIndex));
      dbgs() << "\n";
    });

    // Verify: the computed stride equals to the stride fetched from Dim
    if (NewBlobIndex != StrideBlobIndex) {
      LLVM_DEBUG(dbgs() << "Stride Mismatch in dimension " << Dim << "\n";);
      break;
    }

    // Match in Dim, save the Blob Index into CarryBlob, and continue
    CarryBlob = NewBlobIndex;
  }

  // For a potential partial match, check any un-matched dimension(s):
  const unsigned MatchedDimNum = Dim - 1;

  if (MatchedDimNum < 2) {
    return 0;
  }

  const unsigned OutermostLoopLevel = Level - Dim + 2;
  return hasValidUnmatchedDims(Ref, MatchedDimNum, OutermostLoopLevel)
             ? MatchedDimNum
             : 0;
}

unsigned HIRLoopCollapse::matchCEOnIVLevels(CanonExpr *CE) const {
  int64_t IVConstCoeff = 0;
  unsigned IVIndex = 0, LevelsMatched = 0;

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

    if (TCArry[PrevLevel].isConstant()) {
      AccumuConst *= TCArry[PrevLevel].getConstTripCount();
    } else {
      CanonExpr *TCCE = TCArry[PrevLevel].getTripCount();
      unsigned BlobIndex = TCCE->getSingleBlobIndex();

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
    (void)InnermostLp;

    dbgs() << "OutermostLp: \n";
    OutermostLp->dump();
    dbgs() << "\n";
  }
}

LLVM_DUMP_METHOD void HIRLoopCollapse::printTCArry(void) const {
  for (auto &Item : TCArry) {
    Item.print(true, true);
  }
}

#endif

PreservedAnalyses HIRLoopCollapsePass::runImpl(
    llvm::Function &F, llvm::FunctionAnalysisManager &AM, HIRFramework &HIRF) {

  HIRLoopCollapse(HIRF, AM.getResult<HIRDDAnalysisPass>(F)).run();
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

    return HIRLoopCollapse(getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
                           getAnalysis<HIRDDAnalysisWrapperPass>().getDDA())
        .run();
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
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
