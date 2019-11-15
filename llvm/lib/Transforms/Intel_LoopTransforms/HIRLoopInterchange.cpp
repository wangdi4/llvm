//===----- HIRLoopInterchange.cpp - Permutations of HIR loops -------------===//
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
// Performs loop interchange to achieve best legal permutations
// so the number of cache lines accessed is smallest.
// Input:  Temporal & spatial locality reuse per loop nest, DDG
//
// Steps:
// 1) Walk all loops, look for outer loops that is perfectly nested
// 2) Retrieve loop cost computed from Locality Analysis pass
// 3) If already in decreasing order (from outer to inner),  all done
// 4) Exclude loops that has pragma  for unroll or unroll & jam
//    Exclude triangular loop until later
// 5) Gather DV from DDG
//    instead of calling Demand driven, filter out DV that implies INDEP
//    e.g. dv (> = *)  and our loop interchange candidate is from level 2 to 3,
//    then this edge can be ignored.
//    drop other edges that will not prevent loop interchange: e.g.
//    safe reduction, loop indepedent dep (t1=;  = t1)
//    anti dep for temps (< *)
// 6) Sort loop nests based on cost and get permutation P1.
//    If it's all legal to interchange, proceed to Gencode
// 7) Construct next permutation P2 that's legal
//    based on the permutation P1, from outermost to innermost,
//    choose loop L into P2 if legal. Discard L from P1 and repeat to add
//    more on P2
// 8) Extract pre-hdr & post-exit of outermost loop
// 9) Gencode:  based on P1/P2,  update loop bounds/loop body
// 10) clear safe-reduction flag for cases like:
//       do i; do j; s = s + a(j) ->  do j; do i; s = s + a(j)
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopInterchange.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#include <sstream>

#define OPT_SWITCH "hir-loop-interchange"
#define OPT_DESC "HIR Loop Interchange"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

STATISTIC(LoopsInterchanged, "Number of HIR loops interchanged");

static cl::opt<bool> DisableHIRLoopInterchange("disable-hir-loop-interchange",
                                               cl::init(false), cl::Hidden,
                                               cl::desc("Disable " OPT_DESC));

static cl::opt<unsigned> NearPerfectLoopProfitablityTCThreshold(
    OPT_SWITCH "-near-perfect-profitability-tc-threshold", cl::init(16),
    cl::Hidden,
    cl::desc("TripCount threshold to enable " OPT_DESC
             " for near-perfect loopnests"));
namespace {
typedef std::pair<HLLoop *, HLLoop *> CandidateLoopPair;
typedef DDRefGatherer<RegDDRef, MemRefs> MemRefGatherer;

class HIRLoopInterchange {
public:
  HIRLoopInterchange(HIRFramework &HIRF, HIRDDAnalysis &DDA,
                     HIRLoopLocality &LA, HIRSafeReductionAnalysis &SRA,
                     HIRLoopStatistics &HLS)
      : HIRF(HIRF), DDA(DDA), LA(LA), SRA(SRA), HLS(HLS) {}

  bool run();

private:
  HIRFramework &HIRF;
  HIRDDAnalysis &DDA;
  HIRLoopLocality &LA;
  HIRSafeReductionAnalysis &SRA;
  HIRLoopStatistics &HLS;

  bool AnyLoopInterchanged;
  unsigned OutmostNestingLevel;
  unsigned InnermostNestingLevel;
  HLLoop *InnermostLoop;
  struct CollectCandidateLoops;

  SmallVector<CandidateLoopPair, 12> CandidateLoops;
  SmallVector<const HLLoop *, MaxLoopNestLevel> SortedLoops;
  SmallVector<const HLLoop *, MaxLoopNestLevel> LoopPermutation;
  SmallVector<const HLLoop *, MaxLoopNestLevel> NearByPerm;
  SmallVector<const HLLoop *, 5> PerfectLoopsEnabled;
  SmallVector<DirectionVector, 16> DVs;
  std::map<const HLLoop *, InterchangeIgnorableSymbasesTy>
      CandLoopToIgnorableSymBases;

  bool shouldInterchange(const HLLoop *);
  bool getPermutation(const HLLoop *);
  // returns true means legal for any permutation
  bool isLegalForAnyPermutation(const HLLoop *Loop);
  //  SrcLevel and DstLevel start from 1

  bool isBestLocalityInInnermost(const HLLoop *Loop,
                                 const HLLoop *BestLocalityLoop);
  void getNearbyPermutation(const HLLoop *Loop);
  // SrcLevel and DstLevel start from 1
  void permuteNearBy(unsigned DstLevel, unsigned SrcLevel);
  void transformLoop(HLLoop *Loop);
  void updateLoopBody(HLLoop *Loop);
  void reportTransformation(LoopOptReportBuilder &LORBuilder);
  bool isInPresentOrder(SmallVectorImpl<const HLLoop *> &LoopNests) const;
};

// Returns true if a ref in a shape of
//
// DO i1
//    DO i2
//         A[i2][i1]  // 1st dimension = outer dimension
//
// The loop may benefit from interchanging i1 and i2 loops.
// If this function returns true, we may try interchange.
//
// E.g.:
//  A[i2][i1]           returns true  // IVs increases at the outer dimension.
//  A[i3][i2][i1]       returns true
//  A[i2][i1][i3]       returns true
//  A[i3][i1][i2]       returns true
//  A[i1][i2][i3]       returns false
//  A[i2][i1 + 7]       returns true  // no special handling for const
//  A[5*i2][3*i1]       returns true  // no special handling for const coef
//  A[5*i2 + 2][3*i1]   returns true
//  A[i1][i1]           returns false // not increasing at outer dimensions
//  A[i1][i2]           returns false // not increasing at outer dimensions
//  A[b2*i2][b1*i1]     returns false // bails out with blob coeffs
//  A[i1 + i3][i2]      returns false // bails out with more than one IVs
//                                    // in one dimension
//  A[b*i2][i3][i1]     returns true  // due to "[i3][i1]"
//  A[i3][b*i1][i2]     returns true  // due to "[i3]..[i2]"
//
// TODO: Handling of more than one IVs in one dimension.
// E.g.
//   DO i1
//    DO i2
//       A[i1 + 2*i2]
//    END DO
//   END DO
// Interchanging i1 and i2 is useful.
bool areIVsIncreasingWithOuterDimensions(RegDDRef &Ref) {
  unsigned NodeLevel = Ref.getNodeLevel();
  unsigned MinLevelSoFar = MaxLoopNestLevel + 1;

  // Scan from the innermost dimension
  for (int I = 1, E = Ref.getNumDimensions(); I <= E; I++) {
    const CanonExpr *CE = Ref.getDimensionIndex(I);
    // Inspecting IVs in one dimension.

    // Upto one IVs with a constant coef.
    if (!CE->isLinearAtLevel(NodeLevel)) {
      continue;
    }
    // See if zero or 1(constant coeff only) IV's contained.
    // If only one IV with constant coeff exists, make sure
    // the level is larger than that of IV of previous dimension.
    // Currently, non-1 constant coeff of IV is not specially treated.
    unsigned LevelFound = 0;
    for (unsigned Level = 1; Level <= NodeLevel; Level++) {
      unsigned Index = 0;
      int64_t Coeff = 0;
      CE->getIVCoeff(Level, &Index, &Coeff);

      if (!Coeff) {
        continue;
      }

      if (Index != InvalidBlobIndex || LevelFound != 0) {
        LevelFound = 0;
        break;
      }

      LevelFound = Level;
    }

    // LevelFound = 0 if any of the following conditions is true.
    // 1. no IV is found in this dim.
    // 2. more than one IV is found.
    // 3. At least one IV has blob coeff.
    // In those cases, we never return true
    // at this moment but defer decision to next dimension.
    if (LevelFound > MinLevelSoFar) {
      // One IV with a const coef found
      return true; // may interchange
    } else if (LevelFound > 0) {
      // In case LevelFound == 0, we keep previously set MinLevelSoFar.
      MinLevelSoFar = LevelFound;
    }
  }

  return false; // do not try interchange
}

bool isInterchangingNearPerfectProfitable(const HLLoop *OutermostLoop,
                                          const HLLoop *InnermostLoop) {
  // Same as the existing logic.
  // TODO: Consider replacing the following logic with
  //       areIVsIncreasingWithOuterDimensions(RegDDRef &Ref)
  //       This replacement should be done with the study of the
  //       logic's interaction with locality util and/or its usage within
  //       HIRLoopInterchange. Also, consider working with fixing matmul6.ll.
  // E.G.
  //   DO i1
  //     DO i2
  //       DO i3
  //          a[i2][i1][i3]
  //  Intention is to interchange ( 1 2 3 ) --> ( 2 1 3 ).
  if (HLNodeUtils::hasNonUnitStrideRefs(InnermostLoop)) {
    return true;
  }

  // If a constant TC is too small,
  // avoid aggressive interchange.
  // Scan TC from innermost's parent loop to outermost loop.
  const HLLoop *ParentLp = InnermostLoop->getParentLoop();
  const HLLoop *OutParentLp = OutermostLoop->getParentLoop();
  for (const HLLoop *Lp = ParentLp; Lp != OutParentLp;
       Lp = Lp->getParentLoop()) {
    uint64_t TripCount = -1;
    if (Lp->isConstTripLoop(&TripCount) &&
        TripCount < NearPerfectLoopProfitablityTCThreshold) {
      return false;
    }
  }

  // Examine MemRefs in Pre/postloop or prehead/postexit of the innermost.
  // Recursive = true (pre/postexit), RecursiveInsidedLoop = false (no child)
  MemRefGatherer::VectorTy Refs;
  MemRefGatherer::gatherRange<true, false>(ParentLp->child_begin(),
                                           ParentLp->child_end(), Refs);
  bool MayInterchange = false;
  for (RegDDRef *Ref : Refs) {
    LLVM_DEBUG(Ref->dump());
    MayInterchange = areIVsIncreasingWithOuterDimensions(*Ref);
    if (MayInterchange) {
      break;
    }
  }

  LLVM_DEBUG(dbgs() << "MayInterchange: " << MayInterchange << "\n";);

  return MayInterchange;
}

} // namespace

/// Gather all perfect Loop Nest and enable near perfect one if needed
struct HIRLoopInterchange::CollectCandidateLoops final
    : public HLNodeVisitorBase {

  HIRLoopInterchange &LIP;
  SmallVectorImpl<CandidateLoopPair> &CandidateLoops;
  HIRDDAnalysis &DDA;
  HLNode *SkipNode;

  CollectCandidateLoops(HIRLoopInterchange &LoopIP,
                        SmallVectorImpl<CandidateLoopPair> &CandidateLoops,
                        HIRDDAnalysis &DDA)
      : LIP(LoopIP), CandidateLoops(CandidateLoops), DDA(DDA),
        SkipNode(nullptr) {}

  void visit(HLLoop *Loop) {
    // Gather perfect loop nests
    // TODO: Skip Loop with hasUserCall when flag is sets
    const HLLoop *InnermostLoop = nullptr;

    if (Loop->isInnermost()) {
      SkipNode = Loop;
      return;
    }
    LLVM_DEBUG(dbgs() << "In collect Perfect loopnest\n");
    // Allow Triangular loop, allow Near Perfect loop (and return the result).
    bool IsNearPerfectLoop = false;
    bool IsPerfectNest = HLNodeUtils::isPerfectLoopNest(
        Loop, &InnermostLoop, false, &IsNearPerfectLoop);
    assert((!IsPerfectNest || !IsNearPerfectLoop) &&
           "isPerfectLoopNest is malfunctioning");

    if (!IsPerfectNest && !IsNearPerfectLoop) {
      // Do not skip recursion.
      // We might find a perfect loop nest starting from an inner loop.
      return;
    }

    if (LIP.HLS.getSelfLoopStatistics(InnermostLoop)
            .hasCallsWithUnsafeSideEffects()) {
      LLVM_DEBUG(
          dbgs() << "\nSkipping loop with calls that have side effects\n");
      SkipNode = Loop;
      return;
    }

    for (const HLLoop *TmpLoop = InnermostLoop,
                      *EndLoop = Loop->getParentLoop();
         TmpLoop != EndLoop; TmpLoop = TmpLoop->getParentLoop()) {
      if (TmpLoop->hasUnrollEnablingPragma() ||
          TmpLoop->hasUnrollAndJamEnablingPragma() ||
          TmpLoop->hasVectorizeEnablingPragma()) {
        LLVM_DEBUG(dbgs() << "\nSkipping loop with unroll/vector pragma\n");
        SkipNode = Loop;
        return;
      }
    }

    if (IsPerfectNest) {

      LLVM_DEBUG(dbgs() << "\nIs Perfect Nest\n");

      if (!HLNodeUtils::hasNonUnitStrideRefs(InnermostLoop)) {
        LLVM_DEBUG(
            dbgs() << "\nMemRefs are in unit stride or non-linear Defs\n");
      } else {
        LLVM_DEBUG(dbgs() << "\nHas non unit stride\n");
        CandidateLoops.push_back(
            std::make_pair(Loop, const_cast<HLLoop *>(InnermostLoop)));
      }

      SkipNode = Loop;
      return;
    }

    if (isInterchangingNearPerfectProfitable(Loop, InnermostLoop)) {

      LLVM_DEBUG(dbgs() << "\n Is NearPerfect Loop:\n");
      LLVM_DEBUG(dbgs(); Loop->dump());

      DDGraph DDG = DDA.getGraph(Loop);
      LLVM_DEBUG(dbgs() << "DDG's==\n");
      LLVM_DEBUG(DDG.dump());

      if (DDUtils::enablePerfectLoopNest(
              const_cast<HLLoop *>(InnermostLoop), DDG,
              LIP.CandLoopToIgnorableSymBases[Loop])) {
        CandidateLoops.push_back(
            std::make_pair(Loop, const_cast<HLLoop *>(InnermostLoop)));
        LLVM_DEBUG(dbgs() << "Perfect Loopnest enabled\n");
        LLVM_DEBUG(dbgs(); Loop->dump());
        // Save & invalidate later to avoid DDRebuild and safe reduction map
        // released
        LIP.PerfectLoopsEnabled.push_back(InnermostLoop);
      }
      // Nearperfect loops: skip recursion into the nest regardless of
      // being enabled as perfect loop or not.
      // Either way, loop interchange is not possible due to unconforming
      // innermost loop.
      SkipNode = Loop;
      return;
    }

    // NearPerfect Loop, but concluded not to enable a perfect loop nest.
    // The conclusion could be different for a near perfect loop nest
    // starting from an inner loop of this nest.
    // It depends on "isInterchangingNearPerfectProfitable".
    // Not skipping recursion.
  }

  void visit(HLNode *Node) {}
  void postVisit(HLNode *Node) {}
  bool skipRecursion(const HLNode *Node) const { return Node == SkipNode; }
};

bool HIRLoopInterchange::run() {
  if (DisableHIRLoopInterchange)
    return false;

  LLVM_DEBUG(dbgs() << "Loop Interchange for Function : "
                    << HIRF.getFunction().getName() << "\n");

  AnyLoopInterchanged = false;

  // 1) Walk all loops, look for outer loops that are perfectly nested

  CollectCandidateLoops CCL(*this, CandidateLoops, DDA);
  HIRF.getHLNodeUtils().visitAll(CCL);

  for (auto &Iter : CandidateLoops) {
    HLLoop *Loop = Iter.first;
    InnermostLoop = Iter.second;
    InnermostNestingLevel = InnermostLoop->getNestingLevel();

    LLVM_DEBUG(dbgs() << "\nIn CandiateLoop\n"; Loop->dump());

    if (shouldInterchange(Loop) && getPermutation(Loop)) {
      transformLoop(Loop);
    } else {
      if (std::find(PerfectLoopsEnabled.begin(), PerfectLoopsEnabled.end(),
                    Loop) != PerfectLoopsEnabled.end()) {

        HIRInvalidationUtils::invalidateBody(Loop);
      }
    }
  }

  CandidateLoops.clear();
  PerfectLoopsEnabled.clear();
  CandLoopToIgnorableSymBases.clear();

  return AnyLoopInterchanged;
}

bool HIRLoopInterchange::shouldInterchange(const HLLoop *Loop) {

  SortedLoops.clear();

  bool InterchangeNeeded = true;

  // Call Util in Locality Analysis to get Best Permutation
  LA.sortedLocalityLoops(Loop, SortedLoops);

  if (isInPresentOrder(SortedLoops)) {
    InterchangeNeeded = false;
  }

  LLVM_DEBUG(dbgs() << "\n\tBased on Locality Analysis:");
  LLVM_DEBUG(dbgs() << "\n\tInterchange Needed=" << InterchangeNeeded << "\n");
  return InterchangeNeeded;
}

/// Return true if loop can be interchanged with best permutation or
/// Nearby permutation
bool HIRLoopInterchange::getPermutation(const HLLoop *Loop) {

  // 3) If already in decreasing order (from outer to inner) of loop cost,
  //   nothing needs to be done, Otherwise. Try to find a permutation that's
  //   legal

  bool CanInterchange = false;
  LoopPermutation.clear();
  NearByPerm.clear();
  OutmostNestingLevel = Loop->getNestingLevel();

  // Save it in local vector because it may change later
  for (auto &I : SortedLoops) {
    LoopPermutation.push_back(I);
  }

  // When returning legal == true, we can just interchange w/o
  // examining DV.
  if (isLegalForAnyPermutation(Loop)) {
    LLVM_DEBUG(dbgs() << "\n\tBest permutation available\n");
    CanInterchange = true;
  } else {
    // Check if largest locality can be moved as innermost loop
    // If no, Stop. Otherwise while loop in next function will loop forever

    const HLLoop *BestLocalityLoop = LoopPermutation.back();

    if (!isBestLocalityInInnermost(Loop, BestLocalityLoop)) {
      CanInterchange = false;
    } else {
      // Find Nearby permutation
      getNearbyPermutation(Loop);

      if (isInPresentOrder(NearByPerm)) {
        return false;
      }

      LoopPermutation = NearByPerm;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      LLVM_DEBUG(dbgs() << "\nNearby permutation obtained\n");
      for (auto &I : LoopPermutation) {
        (void)I; // Variable is used in DEBUG only.
        LLVM_DEBUG(dbgs(); I->dump());
      }
#endif
      CanInterchange = true;
    }
  }

  if (!CanInterchange) {
    LLVM_DEBUG(dbgs() << "\nNo legal permutation available\n");
  }

  return CanInterchange;
}

///  Check if the best locality loop can stay or move as innermost
bool HIRLoopInterchange::isBestLocalityInInnermost(
    const HLLoop *Loop, const HLLoop *BestLocalityLoop) {
  unsigned SrcLevel =
      BestLocalityLoop->getNestingLevel() - OutmostNestingLevel + 1;
  if (InnermostNestingLevel == BestLocalityLoop->getNestingLevel() ||
      DDUtils::isLegalForPermutation(InnermostNestingLevel, SrcLevel,
                                     OutmostNestingLevel, DVs)) {
    return true;
  }
  LLVM_DEBUG(dbgs() << "\nCannot move best locality loop as innermost\n");
  return false;
}

///  Nearby Permutation:
///  "Optimizing for Parallelism and Data Locality, Kennedy & McKinley"
/// Input:
///  O = the orginal loop ordering, from 1 to n
///  DV =  set of dv for Loop
///  L = Permutation of O
/// Output:  P,   Nearby permutation
/// Steps:
///  P = null; k = 0; m = n
///  while (L != null)
///     for j=1,m
///       l =  lj in L
///       if all dv for {p1,...,pk,l} are legal {
///          P = {p1, .... pk, l}
///          L = L - {l};  k++;  m--;
///          break for
///       endif
///     endfor
///  endwhile
///
/// There is some issue in the paper.
/// The DVs in (p1,...pk,l) alone cannot determine the legality
/// The original DVs need to be permuted when L is changed and that should
/// used for legality check. In the example below, the first * can be moved
//  to the left but the second * cannot be moved before the first *.
/// Also, if l is already in the k-th position, select it w/o checking
/// legality because it is not changing in position.

/// Modified Algorithm:
///  P = {1 2 ... n}; k = 1; m = n
///  while (L != null)
///     for j=1,m
///       l =  lj in L
///       if ((l is already at k-th loop level  ||
///           (all Permuted dv are legal for l to be shifted as k-th loop)
///          P is updated  by putting l as the k-th loop
///          L = L - {l};  k++;  m--;
///          Permute dv accordingly if needed
///          break for
///       endif
///     endfor
///  endwhile
///
///  An example: assuming we have just 1 dv (* * = =)
///  Best permutation is L = (4 2 1 3)
///  P = (1 2 3 4)
///  (l3 is the one with best locality, should be in the innermost
//   loop when while loop terminates)
///
///  j=1,4 in  L
///    Is l4 at level 1 already? No
///    Is l4 legal as 1st level?  Yes
///    P = (4 1 2 3)  L = (2 1 3)
//     dv becomes (= * * =), corresponding  to (4 1 2 3)
///  j=1,3 in  L
///    Is l2 at level 2 already? No.
///    Is l2 legal as 2nd level? No. (* cannot shift pass *)
///    Is l1 at level 2 already? Yes.
//     No change in P or dv
///    P = (4 1 2 3) L =(2 3)
///  j=1,2
///    Is l2 at 3rd level? Yes.  No change in P or sv
///    P = (4 1 2 3) L =(3)
///  j=1,1
///    Is l3 at 4th level? Yes
///    P = (4 1 2 3) L =()  no change in dv

void HIRLoopInterchange::getNearbyPermutation(const HLLoop *Loop) {

  unsigned DstLevel = 1;
  unsigned Iter = 0;
  const HLNode *Node = Loop;

  //  Based on the paper above, the while loop will halt when
  //  the last loop in L can be moved legally as the innermost loop,
  //  which is verified before calling this function.
  while (Node) {
    const HLLoop *Lp = dyn_cast<HLLoop>(Node);
    if (!Lp) {
      break;
    }
    NearByPerm.push_back(const_cast<HLLoop *>(Lp));
    Node = Lp->getFirstChild();
  }

  while (!LoopPermutation.empty()) {
    for (auto &I : LoopPermutation) {
      // Get current level for loop
      unsigned SrcLevel = 1;
      for (auto &J : NearByPerm) {

        if (J->getNestingLevel() == I->getNestingLevel()) {
          break;
        }
        SrcLevel++;
      }
      assert(SrcLevel != 0 && "Loop not found");
      if (DDUtils::isLegalForPermutation(DstLevel, SrcLevel,
                                         OutmostNestingLevel, DVs)) {
        permuteNearBy(DstLevel, SrcLevel);
        LoopPermutation.erase(&I);
        DstLevel++;
        break;
      }
      Iter += 1;
      //  Assert to avoid looping
      if (Iter > (MaxLoopNestLevel * MaxLoopNestLevel)) {
        dbgs() << "NearbyPermutation is looping";
        std::abort();
      }
    }
  }
}

///  No need to interchange if suggested Permutation is same as present order
bool HIRLoopInterchange::isInPresentOrder(
    SmallVectorImpl<const HLLoop *> &LoopNests) const {

  unsigned PrevLevel = 1;

  for (auto &Loop : LoopNests) {
    unsigned Level = Loop->getNestingLevel();
    if (PrevLevel > Level) {
      return false;
    }
    PrevLevel = Level;
  }
  return true;
}

///  1. Move Loop at SrcLevel to DstLevel loop
///  2. Update all DV accordingly
///  Levels are relative to 1
void HIRLoopInterchange::permuteNearBy(unsigned DstLevel, unsigned SrcLevel) {

  if (SrcLevel == DstLevel) {
    return;
  }
  assert((SrcLevel > DstLevel) && "Loops are shifting to the left");
  bool Erased = false;
  for (auto &Loop : NearByPerm) {
    if (Loop->getNestingLevel() == SrcLevel + OutmostNestingLevel - 1) {
      const HLLoop *LoopSave = Loop;
      NearByPerm.erase(&Loop);
      NearByPerm.insert(NearByPerm.begin() + DstLevel - 1, LoopSave);
      Erased = true;
      break;
    }
  }

  (void)Erased;
  assert(Erased && "Loop not found");
  // Permute DV accordingly
  DstLevel += OutmostNestingLevel - 1;
  SrcLevel += OutmostNestingLevel - 1;

  for (auto &WorkDV : DVs) {
    DVKind DVSrc = WorkDV[SrcLevel - 1];
    // Shift right by 1 for these [Dst : Src-1]
    for (unsigned JJ = SrcLevel; JJ > DstLevel; --JJ) {
      WorkDV[JJ - 1] = WorkDV[JJ - 2];
    }
    // Fill in Dst with Src
    WorkDV[DstLevel - 1] = DVSrc;
  }
}

/// Return true if legal for any permutations
bool HIRLoopInterchange::isLegalForAnyPermutation(const HLLoop *Loop) {

  //
  // 4) exclude loops that has pragma for unroll or unroll & jam
  //    exclude triangular loop for now (TODO)

  // 5) Gather DV from DDG. Filter out loop indep dep for temps,
  // safe reduction.
  // We plan to avoid demand driven DD refining DV.

  HLLoop *Lp = const_cast<HLLoop *>(Loop);
  LLVM_DEBUG(dbgs() << "\n\tStart, End level\n"
                    << OutmostNestingLevel << " " << InnermostNestingLevel);

  SRA.computeSafeReductionChains(Lp);

  //  Set refineDV as false for now (last argument) until we see kernels
  //  that really need to refine DV.

  // The following visitor will gather DVs from DDG and push them into
  // HIRLoopInterchange::DVs;

  DDUtils::computeDVsForPermuteIgnoringSBs(DVs, Lp, InnermostNestingLevel, DDA,
                                           SRA, false,
                                           &CandLoopToIgnorableSymBases[Lp]);

  // If edges are selected,
  // there are dependencies to check out w.r.t to interchange order

  if (DVs.size() > 0) {
    return false;
  }
  LLVM_DEBUG(dbgs() << "\n\tDV array is empty\n");
  return true;
}

void HIRLoopInterchange::reportTransformation(
    LoopOptReportBuilder &LORBuilder) {
  // Do not do any string processing if OptReports are not needed.
  // "&& DebugFlag" should be deleted when lit-tests are rewritten to use opt
  // report info.
  if (!LORBuilder.getVerbosity() && !DebugFlag)
    return;

  HLLoop *OutermostLp = nullptr;

  std::ostringstream OS;
  OS << "Loopnest Interchanged: ( ";
  for (unsigned I = OutmostNestingLevel; I <= InnermostNestingLevel; ++I) {
    OS << I << " ";
  }
  OS << ") --> ( ";
  for (auto &I : LoopPermutation) {
    OS << I->getNestingLevel() << " ";
    if (I->getNestingLevel() == OutmostNestingLevel) {
      assert(!OutermostLp);
      OutermostLp = const_cast<HLLoop *>(I);
    }
  }
  OS << ")";
  LORBuilder(*OutermostLp).addRemark(OptReportVerbosity::Low, OS.str().c_str());

  // This is needed for lit-tests for now.
  LLVM_DEBUG(dbgs() << OS.str() << '\n');
}

void HIRLoopInterchange::transformLoop(HLLoop *Loop) {

  // Invalidate all analysis for InnermostLoop.
  HIRInvalidationUtils::invalidateBounds(InnermostLoop);
  HIRInvalidationUtils::invalidateBody(InnermostLoop);
  LLVM_DEBUG(dbgs() << "\tBefore permuteloopNests:"; Loop->dump());
  assert(OutmostNestingLevel == Loop->getNestingLevel());
  assert(InnermostNestingLevel == InnermostLoop->getNestingLevel());
  InnermostLoop->setIsUndoSinkingCandidate(false);
  HIRTransformUtils::permuteLoopNests(Loop, LoopPermutation,
                                      InnermostNestingLevel);

  LoopOptReportBuilder &LORBuilder =
      Loop->getHLNodeUtils().getHIRFramework().getLORBuilder();

  reportTransformation(LORBuilder);

  Loop->getParentRegion()->setGenCode();

  LoopsInterchanged++;
  AnyLoopInterchanged = true;
}

PreservedAnalyses
HIRLoopInterchangePass::run(llvm::Function &F,
                            llvm::FunctionAnalysisManager &AM) {
  HIRLoopInterchange(AM.getResult<HIRFrameworkAnalysis>(F),
                     AM.getResult<HIRDDAnalysisPass>(F),
                     AM.getResult<HIRLoopLocalityAnalysis>(F),
                     AM.getResult<HIRSafeReductionAnalysisPass>(F),
                     AM.getResult<HIRLoopStatisticsAnalysis>(F))
      .run();

  return PreservedAnalyses::all();
}

class HIRLoopInterchangeLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRLoopInterchangeLegacyPass() : HIRTransformPass(ID) {
    initializeHIRLoopInterchangeLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    AU.addRequiredTransitive<HIRLoopLocalityWrapperPass>();
    AU.addRequiredTransitive<HIRSafeReductionAnalysisWrapperPass>();
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
  }

  bool runOnFunction(Function &F) {
    if (skipFunction(F)) {
      return false;
    }

    return HIRLoopInterchange(
               getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
               getAnalysis<HIRDDAnalysisWrapperPass>().getDDA(),
               getAnalysis<HIRLoopLocalityWrapperPass>().getHLL(),
               getAnalysis<HIRSafeReductionAnalysisWrapperPass>().getHSR(),
               getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS())
        .run();
  }
};

char HIRLoopInterchangeLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLoopInterchangeLegacyPass, "hir-loop-interchange",
                      "HIR Loop Interchange", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopLocalityWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRSafeReductionAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_END(HIRLoopInterchangeLegacyPass, "hir-loop-interchange",
                    "HIR Loop Interchange", false, false)

FunctionPass *llvm::createHIRLoopInterchangePass() {
  return new HIRLoopInterchangeLegacyPass();
}
