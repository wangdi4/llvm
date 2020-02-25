//==-- HIRLoopCollapse.h - HIR Loop Collapse Pass ---------- --*- C++ -*---===//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_LOOPCOLLAPSEIMPL_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_LOOPCOLLAPSEIMPL_H

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"

#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {
class Function;

namespace loopopt {

namespace collapse {

typedef std::pair<HLLoop *, HLLoop *> InnerOuterLoopPairTy;

// This class contains a UBCE if a loop's TripCount is a single Blob, or an
// unsigned if the loop's TripCount is an integer constant.
class UBTCTuple {
  CanonExpr *UBCE; // can be either an UBCE or a TripCount, not both
  unsigned TripCount;
  bool IsConst;

public:
  UBTCTuple(void) : UBCE(nullptr), TripCount(0), IsConst(true) {}

  // getters + setters:
  bool isConstant(void) const { return IsConst; }
  CanonExpr *getUBCE(void) const { return UBCE; }
  unsigned getTripCount(void) const { return TripCount; }

  void set(unsigned TC) {
    TripCount = TC;
    UBCE = nullptr;
    IsConst = true;
  }

  void set(CanonExpr *UB) {
    UBCE = UB;
    TripCount = 0;
    IsConst = false;
  }

#ifndef NDEBUG
  // Print the UBCEConstTuple
  LLVM_DUMP_METHOD void print(bool PrintHeader = false,
                              bool PrintNewLine = true) const {
#ifndef NDEBUG
    formatted_raw_ostream FOS(dbgs());
#endif

    if (PrintHeader) {
      FOS << "UBTCTuple: ";
    }

    if (IsConst) {
      FOS << "TripCount: " << TripCount << " ";
    } else {
      FOS << "UBCE: ";
      UBCE->dump();
      FOS << " ";
    }

    if (PrintNewLine) {
      FOS << "\n";
    }
  }
#endif
};

class HIRLoopCollapse {
  HIRFramework &HIRF;

  SmallVector<RegDDRef *, 32> RefVec;    // non-MemRef Refs
  SmallVector<RegDDRef *, 32> GEPRefVec; // GEPRef Refs, include AddressOf ones

  class CollectRefs;
  class CollectCandidateLoops;
  unsigned InnermostLevel;
  HLLoop *InnermostLp = nullptr;

  HLNodeUtils *HNU = nullptr;
  BlobUtils *BU = nullptr;

  // Among all GEPRefs's max collapse levels:
  //
  // E.g.
  // i1
  // | i2
  // | | i3
  // | | | A[i1][i2][i3] = .. B[2][i2][i3] ...
  // | |
  // |
  //
  // CollapseLevel is 3 for A[i1][i2][i3], and 2 for B[2][i2][i3].
  // Thus, the NumCollapsableLoops (for this loop body) is min(3, 2) = 2.
  unsigned NumCollapsableLoops;

  // an array storing relevant constant TripCounts or UpperBound CanonExpr*
  // for each relevant loop in the loop nest.
  std::array<UBTCTuple, MaxLoopNestLevel> UBTCArry;

  // an array storing the entire loop nest: [InnermostLp .. OutermostLp].
  // This helps mapping a particular level to its matching loop.
  std::array<HLLoop *, MaxLoopNestLevel> LoopNest;

  // Save IV type for the collapse-able loop nest:
  Type *IVType = nullptr;

public:
  HIRLoopCollapse(HIRFramework &HIRF) : HIRF(HIRF) {}
  bool run();

  // The only entry for all caller(s) to do HIR Loop Collapse.
  //
  // Collapse the relevant part of loop nest.
  //
  //
  // E.g.
  // [from]
  // i1
  // | i2
  // | | i3
  // | | | BODY
  // | |
  // |
  //
  // [to]
  // i1
  // | BODY_Transformed
  //
  // provided each IV-relevant GEPRef/Non-MemRef in the loop's body can be
  // successfully collapsed w.r.t the loop nest.
  //
  bool doLoopCollapse(HLLoop *OutermostLp, HLLoop *InnermostLp);

private:
  // Do preliminary tests on the loop nest
  //
  // Check: each loop in the loop nest
  //
  // - is a DO loop;
  // - is normalized;
  // - has same IV type;
  // - is either a loop with Constant-Only TripCount
  //   or
  //   is a loop with Blob-Only Trip Count.
  //   (Bail out on any failure)
  //
  //   and Populate UBTCArry for each loop in the loop nest;
  //
  bool doPreliminaryChecks(void);

  // Setup for the perfect loop nest:
  // - initialize: OutermostLevel/Lp, InnermostLevel/Lp, NumCollapsableLoops,
  // and LoopNest;
  // - call ClearWorkingSetMemory() to clean the board;
  void setupEnvLoopNest(HLLoop *OutermostLp, HLLoop *InnermostLp);

  // Return the current valid outermost loop level
  // during analysis.
  // \p NumCollapsableLoops may get refined (i.e. decreased) during analysis.
  unsigned getOutermostLevel() const {
    return InnermostLevel - NumCollapsableLoops + 1;
  }

  // Analyze the Loop by doing preliminary checks, collection, profit analysis,
  // and legal analysis, etc.
  //
  // Return true indicates that the all Ref(s) collected from the loop body are
  // suitable (legal+profitable) for collapse.
  //
  bool doAnalysis(void);

  // Collect each ref in the loop's body that has at least 1 iv for any valid
  // nesting level within [OuterLevel, InnerLevel]
  // E.g.
  // A[0][i], A[j][1], A[i][j], A[i+j][j-1], i +1, 2*j, etc.
  //
  // Note:
  // Not to collect any loop invariant memref.
  // E.g. A[1], B[2][t]
  //           (t is defined out of loop and is not modified inside loop)
  //
  bool doCollection(void);

  // **HIR Loop Collapse's legal model**
  //
  // A LoopNest is legal for collapse IF&F it is legal for each collected
  // GEPRef.
  //
  // A GEPRef is legal for loop collapse IF&F:
  // Each Dimension is a StandAlone-IV matching its corresponding loop level;
  // E.g.
  // - Dimension1 is a StandAlone IV (e.g. 1*i3) on Innermost-Level;
  // - Dimension2 is a StandAlone IV (e.g. 1*i2) on Innermost-1 Level;
  // - etc.
  //
  // E.g.
  // int A[10][10];
  //
  // i: 0, 4, 1
  //   j: 0, 9, 1
  // | | .  = A[i][j]; // OK
  //
  // ------------------------------------
  // E.g.
  // int A[10][19];
  //
  // i: 0, 4, 1
  //   j: 0, 9, 1
  // | | .  = A[i][2j]; // NOT OK, 2*j is not in SingleStandAlone form
  //
  // ------------------------------------
  // E.g.
  // int A[10][19];
  //
  // i: 0, 4, 1
  //   j: 0, 9, 1
  // | | .  = A[j][j]; // NOT OK, j can't appear in Dimension2
  //
  // *GEPRef check*
  // IF: any collected GEPRef has struct access(es) that is not on dimension1,
  // THEN: the LoopNest is not suitable.
  //
  // E.g.
  //   A[i].0[j][k]: not good;
  //   A[i][j][k].0: good;
  //
  bool areGEPRefsLegal(void);

  // Obtain the max level that a GEPRef can collapse
  // E.g.
  // i1
  // | i2
  // | | i3
  // | | | A[i1][i2][i3] = .. B[2][i2][i3] ...
  // | |
  // |
  //
  // CollapseLevel is 3 for A[i1][i2][i3], and 2 for B[2][i2][i3].
  // (thus, the NumCollapsableLoops for both is 2.)
  //
  // Note:
  // IF: there is any access to struct NOT on dimension1,
  // THEN: return 0; (NO collapse-able level);
  //
  // If this function return 0 or 1, there is NO collapse-able level in the
  // given GEPRef.
  unsigned getNumCollapsableLevels(RegDDRef *GEPRef);

  // Check: Ref's SrcType and DestType on any dimension with LoopNest IV.
  // Expect: both SrcType and DstType being same as the IVType.
  // Return how many dimensions of the ref has the same src and dest type.
  unsigned getNumDimensionsOfMatchedSrcAndDestType(RegDDRef *GEPRef);

  // ** HIR Loop Collapse's profit model **
  //
  // A HIR LoopNest is profitable for Loop Collapsing if EVERY collected
  // non-GEPRef Ref is in SingleCanonExpr form of N * OuterLpIV + InnerLpIV
  // form, where: N is the InnerLpTripCount (constant or blob)
  //
  // E.g.
  // int A[10][10][10];
  //
  // i1: 0,9,1
  // | i2: 0,9,1
  // | | A[i1][i2][i3] = 100 *i1 + 10* i2 + i3; //more complex case
  // | | A[ 0][i2][i3] =           10* i2 + i3; //simple case
  // |
  //                     ^test for profit model
  bool areNonGEPRefsProfitable(void);

  // Check if there are continuous sub ranges of a given non-MemRef Ref, w.r.t.
  // all levels of the loop nest, starting from the InnermostLp level.
  // E.g.
  // |||   A[i][j][k] =  i;
  //                     ^ check this!
  //
  // After check continuous range, the bool array has:
  // ----------------------------
  // |i1 level|i2 level|i3 level|
  // ----------------------------
  // |  0     |    1   |    1   |
  // ----------------------------
  //
  // - i2-i3 are continuous and the length is 2;
  // - the function will return 2;
  //
  // E.g.
  // |||   A[i][j][k] =  k;
  //
  // After check continuous range, the bool array has:
  // ----------------------------
  // |i1 level|i2 level|i3 level|
  // ----------------------------
  // |  1     |    1   |    0   |
  // ----------------------------
  //
  // Because the innermost Level is not suitable, the function will return 0.
  // (May consider to support it in future, if we find suitable test cases.)
  //
  unsigned getContinuousSubRanges(const CanonExpr *CE) const;

  // Do HIR Loop Collapse Transformation:
  //
  // Note:
  // The loop nest is suitable for collapse up to NumCollapsableLoops level.
  //
  // Overall Transformation:
  // 1. Identify the proper loop pair (within the loop nest) for collapse;
  // 2. do collapsing;
  //
  // E.g. Collapse a 2-level LoopNest into a single-level loop
  //   E.g.
  //   [FROM]
  //   | i1: 0, M-1, 1
  //   | |  i2: 0, N-1, 1
  //   | |    BODY
  //   |
  //
  //   [TO]
  //   | i3: 0, M*N-1, 1
  //   | ...
  //   |
  //
  // - Simplify any NotLinearized GEPRef:
  //   FROM: A[0][i1][i2]
  //   TO  : A[0][0 ][i3]
  //
  // - Simplify any GEPRef that have only 1 dimension:
  //   FROM: A[N*i1 + i2], where N is innermost Lp's trip count (a blob)
  //   TO  : A[i3]
  //
  // ** Steps **
  // [Prepare]
  //   Obtain the relevant portion of the loop nest that will be collapsed.
  //
  // Starting point:
  //   [FROM]
  //   | i1: 0, M-1, 1
  //   | |  i2: 0, N-1, 1
  //   | |    A[0][i1][i2] = 1;
  //   |
  //
  // [Move loop]
  // move i2 loop ahead of i1 loop, so the new LoopNest will look like:
  //
  //   | i2: 0, N-1, 1
  //   |   A[0][i1][i2] = 1;
  //
  //   | i1: 0, M-1, 1
  //   |   empty body
  //
  // Note: the original i2 loop becomes the collapsed loop.
  //
  // [Fix the new trip count for the collapsed loop]
  //
  //   | i2: 0, N*M-1, 1
  //   |   A[0][i1][i2] = 1;
  //
  //   | i1: 0, M-1, 1
  //   |   empty body
  //
  // [Do replacement inside i2 loop (the collapsed loop)]
  // - Collapse each GEPRef (from collection)
  // - Collapse each Non-MemRef (from collection)
  //
  //   | i2: 0, N*M-1, 1
  //   |   A[0][0][i2] = 1;
  //
  //   | i1: 0, M-1, 1
  //   |   empty body
  //
  // [Remove the original i1 loop, thus obtain the result after collapsing]
  //
  //   | i2: 0, N*M-1, 1
  //   |   A[0][0][i2] = 1;
  //
  bool doTransform(HLLoop *const ToCollapseLp,
                   const unsigned OrigInnermostLevel,
                   const unsigned OrigOutermostLevel);

  void clearWorkingSetMemory(void);

  // *** Utility functions ***

  // Check: if a CE has any loopnest-level [OutermostLevel .. InnermostLevel] IV
  bool hasLoopNestIV(const CanonExpr *CE) const;

  // Initialize the UBTC array, since it has UBTCTuple objects
  void initializeUBTCArry(void) {
    for (unsigned I = 0; I < MaxLoopNestLevel; ++I) {
      UBTCArry[I] = UBTCTuple();
    }
  }

  bool isSingleDimensionMemRef(RegDDRef *MemRef) const {
    return (MemRef->getNumDimensions() == 1);
  }

  // *** Pattern Matching Code ***
  //
  // Requirements:
  // - one function, match all possible patterns;
  // - iterative through a loop, no recursion;
  // - support any type loop (BLOB-only TripCount or Constant TripCount);
  //
  // Algorithm scratch:
  // - maintains 2 variables: 1 constant integer (ConstInt: start from 1),
  //  and 1 Blob (BlobVal, start from nullptr);
  //
  // - on 1st level:
  //  . set the ConstInt: if there is any valid IVCoeff;
  //  . set the BlobVal: if there is any valid IVBlobIndex;
  // -
  // - on each valid additional level:
  //  . accumulate (*=) the ConstInt: if there is any valid IVCoeff;
  //  . accumulate (*=) the BlobVal: if there is any valid IVBlobIndex;
  //
  // -terminate: when there is no more loop to match
  //
  // Input:
  // -CanonExpr *: the CE to match
  // - NumCollapsableLoops: the number of collapse-able loops (implicit);
  // - OutermostLp..InnermostLp: the LoopNest being considered;
  //
  // Return: unsigned
  // -the number of levels matched
  //
  // \brief
  // E.g. Try to match the following CanonExpr*:
  // CE: 100 * zext.i32.i64(%Q) * i1 + zext.i32.i64(%Q) * i2 + i3
  //                                                          ^1st
  //					              ^2nd
  //    ^3rd
  //
  // Expected return:
  //- LevelsMatched: 3
  //
  unsigned getLevelsOfIVPattern(CanonExpr *CE) const;

#ifndef NDEBUG
  // Print GEPRefVec and RefVec
  LLVM_DUMP_METHOD void printCollectedRefVec(bool PrintGEPRefVec = true,
                                             bool PrintNonMemRefVec = true,
                                             bool NewLine = true) const;

  // Print CandidateLoops;
  LLVM_DUMP_METHOD void printCandidateLoops(
      SmallVector<InnerOuterLoopPairTy, 12> &CandidateLoops) const;

  // Print UBTCArry;
  LLVM_DUMP_METHOD void printUBTCArry(void) const;
#endif
};

//
} // namespace collapse
} // namespace loopopt
} // namespace llvm

#endif
