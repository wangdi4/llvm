//===----- HIRLoopInterchange.cpp - Permutations of HIR loops -------------===//
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

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
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

#define DEBUG_TYPE "hir-loop-interchange"

using namespace llvm;
using namespace llvm::loopopt;

STATISTIC(LoopsInterchanged, "Number of HIR loops interchanged");

static cl::opt<bool>
    DisableHIRLoopInterchange("disable-hir-loop-interchange", cl::init(false),
                              cl::Hidden,
                              cl::desc("Disable HIR Loop Interchange"));

namespace {
typedef std::pair<HLLoop *, HLLoop *> CandidateLoopPair;

class HIRLoopInterchange : public HIRTransformPass {

public:
  static char ID;

  HIRLoopInterchange() : HIRTransformPass(ID) {
    initializeHIRLoopInterchangePass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    AU.addRequiredTransitive<HIRLoopLocalityWrapperPass>();
    AU.addRequiredTransitive<HIRSafeReductionAnalysisWrapperPass>();
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
  }

private:
  Function *F;
  HIRDDAnalysis *DDA;
  HIRLoopLocality *LA;
  HIRSafeReductionAnalysis *SRA;
  HIRLoopStatistics *HLS;
  bool AnyLoopInterchanged;
  unsigned OutmostNestingLevel;
  unsigned InnermostNestingLevel;
  HLLoop *InnermostLoop;
  struct CollectDDInfo;
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
  bool isLegalToShiftLoop(unsigned DstLevel, unsigned SrcLevel) const;

  bool isBestLocalityInInnermost(const HLLoop *Loop,
                                 const HLLoop *BestLocalityLoop);
  void getNearbyPermutation(const HLLoop *Loop);
  // SrcLevel and DstLevel start from 1
  bool isLegalForPermutation(unsigned DstLevel, unsigned SrcLevel) const;
  // SrcLevel and DstLevel start from 1
  void permuteNearBy(unsigned DstLevel, unsigned SrcLevel);
  void transformLoop(HLLoop *Loop);
  void updateLoopBody(HLLoop *Loop);
  void reportTransformation(LoopOptReportBuilder &LORBuilder);
  bool isInPresentOrder(SmallVectorImpl<const HLLoop *> &LoopNests) const;
};
} // namespace

char HIRLoopInterchange::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLoopInterchange, "hir-loop-interchange",
                      "HIR Loop Interchange", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopLocalityWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRSafeReductionAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_END(HIRLoopInterchange, "hir-loop-interchange",
                    "HIR Loop Interchange", false, false)

namespace {
bool isBlockingCandidate(const HLLoop *Loop) {
  // Will be in blocking code when it is ready
  // To be deleted
  return false;
}
} // namespace

/// Gather all perfect Loop Nest and enable near perfect one if needed
struct HIRLoopInterchange::CollectCandidateLoops final
    : public HLNodeVisitorBase {

  HIRLoopInterchange *LIP;
  SmallVectorImpl<CandidateLoopPair> &CandidateLoops;
  HIRDDAnalysis *DDA;
  HLNode *SkipNode;

  CollectCandidateLoops(HIRLoopInterchange *LoopIP,
                        SmallVectorImpl<CandidateLoopPair> &CandidateLoops,
                        HIRDDAnalysis *DDA)
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

    if (LIP->HLS->getSelfLoopStatistics(InnermostLoop)
            .hasCallsWithUnsafeSideEffects()) {
      LLVM_DEBUG(
          dbgs() << "\nSkipping loop with calls that have side effects\n");
      SkipNode = Loop;
      return;
    }

    for (const HLLoop *TmpLoop = InnermostLoop,
                      *EndLoop = Loop->getParentLoop();
         TmpLoop != EndLoop; TmpLoop = TmpLoop->getParentLoop()) {
      if (TmpLoop->hasUnrollEnablingPragma()) {
        LLVM_DEBUG(dbgs() << "\nSkipping loop with unroll pragma\n");
        SkipNode = Loop;
        return;
      }
    }

    bool HasNonUnitStrideRefs =
        HLNodeUtils::hasNonUnitStrideRefs(InnermostLoop);

    if (IsPerfectNest) {

      LLVM_DEBUG(dbgs() << "\nIs Perfect Nest\n");

      if (!HasNonUnitStrideRefs) {
        LLVM_DEBUG(
            dbgs() << "\nMemRefs are in unit stride or non-linear Defs\n");
      } else {
        LLVM_DEBUG(dbgs() << "\nHas non unit stride\n");
        CandidateLoops.push_back(
            std::make_pair(Loop, const_cast<HLLoop *>(InnermostLoop)));
      }

      SkipNode = Loop;
      return;

    } else if (HasNonUnitStrideRefs || isBlockingCandidate(Loop)) {

      LLVM_DEBUG(dbgs() << "\n Is NearPerfect Loop:\n");
      LLVM_DEBUG(dbgs(); Loop->dump());

      DDGraph DDG = DDA->getGraph(Loop);
      LLVM_DEBUG(dbgs() << "DDG's==\n");
      LLVM_DEBUG(DDG.dump());

      if (DDUtils::enablePerfectLoopNest(
              const_cast<HLLoop *>(InnermostLoop), DDG,
              LIP->CandLoopToIgnorableSymBases[Loop])) {
        CandidateLoops.push_back(
            std::make_pair(Loop, const_cast<HLLoop *>(InnermostLoop)));
        LLVM_DEBUG(dbgs() << "Perfect Loopnest enabled\n");
        LLVM_DEBUG(dbgs(); Loop->dump());
        // Save & invalidate later to avoid DDRebuild and safe reduction map
        // released
        LIP->PerfectLoopsEnabled.push_back(InnermostLoop);
      }
      // Nearperfect loops: skip recursion into the nest regardless of
      // being enabled as perfect loop or not.
      // Either way, loop interchange is not possible due to unconforming
      // innermost loop.
      SkipNode = Loop;
      return;
    }
  }

  void visit(HLNode *Node) {}
  void postVisit(HLNode *Node) {}
  bool skipRecursion(const HLNode *Node) const { return Node == SkipNode; }
};

FunctionPass *llvm::createHIRLoopInterchangePass() {
  return new HIRLoopInterchange();
}

bool HIRLoopInterchange::runOnFunction(Function &F) {
  if (skipFunction(F) || DisableHIRLoopInterchange)
    return false;

  LLVM_DEBUG(dbgs() << "Loop Interchange for Function : " << F.getName()
                    << "\n");

  this->F = &F;
  auto HIRF = &getAnalysis<HIRFrameworkWrapperPass>().getHIR();
  DDA = &getAnalysis<HIRDDAnalysisWrapperPass>().getDDA();
  LA = &getAnalysis<HIRLoopLocalityWrapperPass>().getHLL();
  SRA = &getAnalysis<HIRSafeReductionAnalysisWrapperPass>().getHSR();
  HLS = &getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS();

  AnyLoopInterchanged = false;

  // 1) Walk all loops, look for outer loops that are perfectly nested

  CollectCandidateLoops CCL(this, CandidateLoops, DDA);
  HIRF->getHLNodeUtils().visitAll(CCL);

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
  LA->sortedLocalityLoops(Loop, SortedLoops);

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
      isLegalForPermutation(InnermostNestingLevel, SrcLevel)) {
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
      if (isLegalForPermutation(DstLevel, SrcLevel)) {
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

namespace {
///  1. Ignore all  (= = ..)
///  2. for temps, ignore  anti (< ..)
///     If there is a loop carried flow for scalars, the DV will not
///     be all =
///     (check no longer needed because of change in DD for compile time
///     saving)
///  3. Safe reduction (already excluded out in collectDDInfo)
bool ignoreEdge(const DDEdge *Edge, const HLLoop *CandidateLoop,
                DirectionVector *RefinedDV = nullptr) {

  const DirectionVector *DV = RefinedDV;
  if (DV == nullptr) {
    DV = &Edge->getDV();
  }
  if (DV->isEQ()) {
    return true;
  }

  const HLLoop *Loop = CandidateLoop;
  if (DV->isIndepFromLevel(Loop->getNestingLevel())) {
    return true;
  }

  // t1 =
  //    = t1
  // Anti dep (<) for LoopIndepDepTemp is no longer generated
  // no need to check

  return false;
}

///  Scan presence of  < ... >
///  If none, return true, which  means DV can be dropped for
///  Interchange legality checking
bool ignoreDVWithNoLTGT(const DirectionVector &DV, unsigned OutmostNestingLevel,
                        unsigned InnermostNestingLevel) {

  bool DVhasLT = false;
  unsigned LTLevel = 0;

  for (unsigned II = OutmostNestingLevel; II <= InnermostNestingLevel; ++II) {
    if (DVhasLT && (DV[II - 1] & DVKind::GT)) {
      if (II != LTLevel) {
        return false;
      }
    }

    if (!DVhasLT && (DV[II - 1] & DVKind::LT)) {
      DVhasLT = true;
      LTLevel = II;
    }
  }
  return true;
}

} // namespace

/// Collect all DV edges in loop nests
/// Call Demand Driven DD as needed (Currently  not calling)
struct HIRLoopInterchange::CollectDDInfo final : public HLNodeVisitorBase {

  HIRLoopInterchange &LIP;
  const HLLoop *CandidateLoop;
  DDGraph DDG;

  CollectDDInfo(HIRLoopInterchange &LIP, const HLLoop *CandidateLoop,
                bool RefineDV)
      : LIP(LIP), CandidateLoop(CandidateLoop),
        DDG(LIP.DDA->getGraph(CandidateLoop)), RefineDV(RefineDV) {
    LIP.DVs.clear();
  }

  // Indicates if we need to call Demand Driven DD to refine DV
  bool RefineDV;
  // start, end level of Candidate Loop nest

  void visit(const HLDDNode *DDNode) {

    const HLInst *Inst = dyn_cast<HLInst>(DDNode);
    if (Inst && LIP.SRA->isSafeReduction(Inst)) {
      LLVM_DEBUG(dbgs() << "\n\tIs Safe Red");
      return;
    }

    const InterchangeIgnorableSymbasesTy &IgnorableSymBases =
        (LIP.CandLoopToIgnorableSymBases)[CandidateLoop];

    for (auto I = DDNode->ddref_begin(), E = DDNode->ddref_end(); I != E; ++I) {

      // Ignorable symbases are symbases of temps originally were
      // in pre(post)loop or preheader/postexit.
      // Those were legally sinked into the innermost loop.
      // The fact allows us to ignore DDs related to those temps.
      if ((*I)->isTerminalRef() &&
          IgnorableSymBases.count((*I)->getSymbase())) {
        continue;
      }

      for (auto II = DDG.outgoing_edges_begin(*I),
                EE = DDG.outgoing_edges_end(*I);
           II != EE; ++II) {
        // Examining outoging edges is sufficent
        const DDEdge *Edge = *II;
        DDRef *DDref = Edge->getSink();

        if (ignoreEdge(Edge, CandidateLoop)) {

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
          LLVM_DEBUG(dbgs() << "\n\t<Edge dropped>");
          LLVM_DEBUG(dbgs() << "\t"; Edge->print(dbgs()));
#endif
          continue;
        }
        const DirectionVector *TempDV = &Edge->getDV();

        // Calling Demand Driven DD to refine DV
        RefinedDependence RefinedDep;

        if (RefineDV) {
          DDRef *SrcDDRef = Edge->getSrc();
          DDRef *DstDDRef = DDref;

          // Refine works only for non-terminal refs
          RefinedDep =
              LIP.DDA->refineDV(SrcDDRef, DstDDRef, LIP.OutmostNestingLevel,
                                LIP.InnermostNestingLevel, false);

          if (RefinedDep.isIndependent()) {
            LLVM_DEBUG(dbgs() << "\n\t<Edge dropped with DDTest Indep>");
            LLVM_DEBUG(dbgs() << "\t"; Edge->print(dbgs()));
            continue;
          }

          if (RefinedDep.isRefined()) {
            LLVM_DEBUG(dbgs() << "\n\t<Edge with refined DV>");
            LLVM_DEBUG(dbgs() << "\t"; Edge->print(dbgs()));
            if (ignoreEdge(Edge, CandidateLoop, &RefinedDep.getDV())) {
              LLVM_DEBUG(dbgs() << "\n\t<Edge dropped with refined DV Ignore>");
              LLVM_DEBUG(dbgs() << "\t"; Edge->print(dbgs()));
              continue;
            }

            TempDV = &RefinedDep.getDV();
          }
        }
        if (ignoreDVWithNoLTGT(*TempDV, LIP.OutmostNestingLevel,
                               LIP.InnermostNestingLevel)) {
          LLVM_DEBUG(dbgs() << "\n\t<Edge dropped with NoLTGT>");
          LLVM_DEBUG(dbgs() << "\t"; Edge->print(dbgs()));
          continue;
        }

        //  Save the DV in an array which will be used later
        LIP.DVs.push_back(*TempDV);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
        LLVM_DEBUG(dbgs() << "\n\t<Edge selected>");
        LLVM_DEBUG(dbgs() << "\t"; Edge->print(dbgs()));
#endif
      }
    }
  }

  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}
  void postVisit(const HLDDNode *Node) {}
};

bool HIRLoopInterchange::isLegalToShiftLoop(unsigned DstLevel,
                                            unsigned SrcLevel) const {

  unsigned SmallerLevel;

  //  Trying to move Loop from Srclevel to DstLevel
  //  The loop can either shift inwards or outwards depending if SrcLevel <
  //  DstLevel
  //  It would become illegal if the leftmost non-equal dv is  ">".
  //  for each DV
  // (1)
  //  First scan if there is any leading dv "<" outside the range of movement
  //  e.g.  ( <  < = >)  if we are moving only last 3 levels then it is always
  //  legal
  //  because it will end up as ( < > = <)
  // (2) Moving < inwards: Once we hit <, then it is legal , if we hit > or *
  //     return illegal
  // (3) Moving > outwards: okay to shift * to left as long as it does
  //     not hit <
  //

  //  Adjust DstLevel based on OutmostNestingLevel
  //  because DV are based on actual loop level, input Dst/Src level
  //  are relative to 1
  DstLevel += OutmostNestingLevel - 1;
  SrcLevel += OutmostNestingLevel - 1;
  SmallerLevel = std::min(SrcLevel, DstLevel);

  for (auto &II : DVs) {
    bool Ok = false;
    const DirectionVector &WorkDV = II;
    // (1)
    for (unsigned KK = OutmostNestingLevel; KK < SmallerLevel; ++KK) {
      if (WorkDV[KK - 1] == DVKind::LT) {
        Ok = true;
        break;
      }
    }
    if (Ok) {
      continue;
    }
    // (2)
    if (DstLevel > SrcLevel) {
      if (WorkDV[SrcLevel - 1] & DVKind::LT) {
        for (unsigned JJ = SrcLevel + 1; JJ <= DstLevel; ++JJ) {
          if (WorkDV[JJ - 1] == DVKind::LT || WorkDV[JJ - 1] == DVKind::LE) {
            break;
          }
          if (WorkDV[JJ - 1] & DVKind::GT) {
            return false;
          }
        }
      }
    } else {
      // (3)
      // (= = *)  Okay to shift * to left as long as it does not hit <
      if (WorkDV[SrcLevel - 1] & DVKind::GT) {
        for (unsigned JJ = SrcLevel - 1; JJ >= DstLevel; --JJ) {
          if (WorkDV[JJ - 1] & DVKind::LT) {
            return false;
          }
        }
      }
    }
  }
  return true;
}

///  Check all DV to see if it's legal to move SrcLoop pass DstLevel
///  e.g. Assuming  dv = (< = >),  ScrLoop is the 3rd level loop
///                 DstLevel  = 1
///       It will return false because > cannot cross <
///  Input Levels are relative to 1 (starting in level 1)
bool HIRLoopInterchange::isLegalForPermutation(unsigned DstLevel,
                                               unsigned SrcLevel) const {
  if (SrcLevel == DstLevel) {
    return true;
  }
  return isLegalToShiftLoop(DstLevel, SrcLevel);
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

  SRA->computeSafeReductionChains(Lp);

  //  Set refineDV as false for now (last argument) until we see kernels
  //  that really need to refine DV.

  // The following visitor will gather DVs from DDG and push them into
  // HIRLoopInterchange::DVs;

  CollectDDInfo CDD(*this, Lp, false);
  Lp->getHLNodeUtils().visit(CDD, Lp);

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

  HIRTransformUtils::permuteLoopNests(Loop, LoopPermutation);

  assert(OutmostNestingLevel == Loop->getNestingLevel());
  assert(InnermostNestingLevel == InnermostLoop->getNestingLevel());
  HIRTransformUtils::updatePermutedLoopBody(Loop, LoopPermutation,
                                            InnermostNestingLevel);

  LoopOptReportBuilder &LORBuilder =
      Loop->getHLNodeUtils().getHIRFramework().getLORBuilder();

  reportTransformation(LORBuilder);

  Loop->getParentRegion()->setGenCode();

  LoopsInterchanged++;
  AnyLoopInterchanged = true;
}
