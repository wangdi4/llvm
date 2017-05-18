//===----- HIRLoopInterchange.cpp - Permutations of HIR loops -------------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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

#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRSafeReductionAnalysis.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#define DEBUG_TYPE "hir-loopinterchange"

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
    AU.addRequiredTransitive<HIRFramework>();
    AU.addRequiredTransitive<HIRDDAnalysis>();
    AU.addRequiredTransitive<HIRLocalityAnalysis>();
    AU.addRequiredTransitive<HIRSafeReductionAnalysis>();
    AU.addRequiredTransitive<HIRLoopStatistics>();
  }

private:
  Function *F;
  HIRDDAnalysis *DDA;
  HIRLocalityAnalysis *LA;
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
  void printOptReport(HLLoop *Loop);
  bool isInPresentOrder(SmallVectorImpl<const HLLoop *> &LoopNests) const;
};
}

char HIRLoopInterchange::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLoopInterchange, "hir-loop-interchange",
                      "HIR Loop Interchange", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysis)
INITIALIZE_PASS_DEPENDENCY(HIRLocalityAnalysis)
INITIALIZE_PASS_DEPENDENCY(HIRSafeReductionAnalysis)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatistics)
INITIALIZE_PASS_END(HIRLoopInterchange, "hir-loop-interchange",
                    "HIR Loop Interchange", false, false)

static bool isBlockingCandidate(const HLLoop *Loop) {
  // Will be in blocking code when it is ready
  // To be deleted
  return false;
}

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
    DEBUG(dbgs() << "In collect Perfect loopnest\n");
    // Last 3 arguments of next call:
    // Allow PrePost Hdr, allow Triangular loop, allow Near Perfect loop
    bool IsNearPerfectLoop = false;
    if (HLNodeUtils::isPerfectLoopNest(Loop, &InnermostLoop, false, false, true,
                                       &IsNearPerfectLoop)) {

      if (LIP->HLS->getSelfLoopStatistics(InnermostLoop)
              .hasCallsWithUnsafeSideEffects()) {
        DEBUG(dbgs() << "Skipping loop with calls that have side effects\n");
        SkipNode = Loop;
        return;
      }

      DEBUG(dbgs() << "Is  Perfect loopnest\n");

      if (!IsNearPerfectLoop) {
        DEBUG(dbgs() << "Is Perfect");
        if (HLNodeUtils::hasNonUnitStrideRefs(InnermostLoop)) {
          DEBUG(dbgs() << "\nHas non unit stride");
          CandidateLoops.push_back(
              std::make_pair(Loop, const_cast<HLLoop *>(InnermostLoop)));
        } else {
          DEBUG(dbgs() << "MemRefs are in unit stride or non-linear Defs");
        }

        SkipNode = Loop;
        return;

      } else if (HLNodeUtils::hasNonUnitStrideRefs(InnermostLoop) ||
                 isBlockingCandidate(Loop)) {
        // Near perfect loops found
        DEBUG(dbgs() << "is NearPerfect Loop:\n");
        DEBUG(dbgs(); Loop->dump());
        DDGraph DDG = DDA->getGraph(Loop);

        if (DDUtils::enablePerfectLoopNest(const_cast<HLLoop *>(InnermostLoop),
                                           DDG)) {
          CandidateLoops.push_back(
              std::make_pair(Loop, const_cast<HLLoop *>(InnermostLoop)));
          SkipNode = Loop;
          DEBUG(dbgs() << "Perfect Loopnest enabled\n");
          DEBUG(dbgs(); Loop->dump());
          // Save & invalidate later to avoid DDRebuild and safe reduction map
          // released
          LIP->PerfectLoopsEnabled.push_back(InnermostLoop);
          return;
        }
      }
    }
  }
  void visit(HLNode *Node) {}
  void postVisit(HLNode *Node) {}
  bool skipRecursion(const HLNode *Node) const override {
    return Node == SkipNode;
  }
};

FunctionPass *llvm::createHIRLoopInterchangePass() {
  return new HIRLoopInterchange();
}

bool HIRLoopInterchange::runOnFunction(Function &F) {
  if (skipFunction(F))
    return false;

  DEBUG(dbgs() << "Loop Interchange for Function : " << F.getName() << "\n");

  this->F = &F;
  auto HIRF = &getAnalysis<HIRFramework>();
  DDA = &getAnalysis<HIRDDAnalysis>();
  LA = &getAnalysis<HIRLocalityAnalysis>();
  SRA = &getAnalysis<HIRSafeReductionAnalysis>();
  HLS = &getAnalysis<HIRLoopStatistics>();

  AnyLoopInterchanged = false;

  // 1) Walk all loops, look for outer loops that are perfectly nested

  CollectCandidateLoops CCL(this, CandidateLoops, DDA);
  HIRF->getHLNodeUtils().visitAll(CCL);

  for (auto &Iter : CandidateLoops) {
    HLLoop *Loop = Iter.first;
    InnermostLoop = Iter.second;
    InnermostNestingLevel = InnermostLoop->getNestingLevel();

    DEBUG(dbgs() << "\nIn CandiateLoop\n"; Loop->dump());

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

  DEBUG(dbgs() << "\n\tBased on Locality Analysis:");
  DEBUG(dbgs() << "\n\tInterchange Needed=" << InterchangeNeeded << "\n");
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
    DEBUG(dbgs() << "\n\tBest permutation available\n");
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
      DEBUG(dbgs() << "\nNearby permutation obtained\n");
      for (auto &I : LoopPermutation) {
        DEBUG(dbgs(); I->dump());
      }
#endif
      CanInterchange = true;
    }
  }

  if (!CanInterchange) {
    DEBUG(dbgs() << "\nNo legal permutation available\n");
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
  DEBUG(dbgs() << "\nCannot move best locality loop as innermost\n");
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

///  1. Ignore all  (= = ..)
///  2. for temps, ignore  anti (< ..)
///     If there is a loop carried flow for scalars, the DV will not
///     be all =
///     (check no longer needed because of change in DD for compile time
///     saving)
///  3. Safe reduction (already excluded out in collectDDInfo)
static bool ignoreEdge(const DDEdge *Edge, const HLLoop *CandidateLoop,
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
static bool ignoreDVWithNoLTGT(const DirectionVector &DV,
                               unsigned OutmostNestingLevel,
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
      DEBUG(dbgs() << "\n\tIs Safe Red");
      return;
    }
    for (auto I = DDNode->ddref_begin(), E = DDNode->ddref_end(); I != E; ++I) {

      for (auto II = DDG.outgoing_edges_begin(*I),
                EE = DDG.outgoing_edges_end(*I);
           II != EE; ++II) {
        // Examining outoging edges is sufficent
        const DDEdge *Edge = *II;
        DDRef *DDref = Edge->getSink();
        if (ignoreEdge(Edge, CandidateLoop)) {

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
          DEBUG(dbgs() << "\n\t<Edge dropped>");
          DEBUG(dbgs() << "\t"; Edge->print(dbgs()));
#endif
          continue;
        }
        const DirectionVector *TempDV = &Edge->getDV();

        // Calling Demand Driven DD to refine DV
        DirectionVector RefinedDV;
        if (RefineDV) {
          DDRef *SrcDDRef = Edge->getSrc();
          DDRef *DstDDRef = DDref;
          DistanceVector RefinedDistV;
          bool IsIndep;
          bool IsDVRefined = LIP.DDA->refineDV(
              SrcDDRef, DstDDRef, LIP.InnermostNestingLevel,
              LIP.OutmostNestingLevel, RefinedDV, RefinedDistV, &IsIndep);
          if (IsIndep) {
            continue;
          }
          if (IsDVRefined) {
            if (ignoreEdge(Edge, CandidateLoop, &RefinedDV)) {
              continue;
            }
            TempDV = &RefinedDV;
          }
        }
        if (ignoreDVWithNoLTGT(*TempDV, LIP.OutmostNestingLevel,
                               LIP.InnermostNestingLevel)) {
          continue;
        }

        //  Save the DV in an array which will be used later
        LIP.DVs.push_back(*TempDV);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
        DEBUG(dbgs() << "\n\t<Edge selected>");
        DEBUG(dbgs() << "\t"; Edge->print(dbgs()));
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
  DEBUG(dbgs() << "\n\tStart, End level\n"
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
  DEBUG(dbgs() << "\n\tDV array is empty\n");
  return true;
}

// Update DD REF
struct UpdateDDRef final : public HLNodeVisitorBase {

  // Smallest value of OutmostNestingLevel & InnermostNestingLevel is 1.
  unsigned OutmostNestingLevel;
  unsigned InnermostNestingLevel;
  unsigned *NewLoopLevels;
  void updateDDRef(HLDDNode *Node, unsigned InnermostNestingLevel,
                   unsigned OutmostNestingLevel, unsigned *NewLoopLevels);
  void updateCE(CanonExpr *CE, unsigned InnermostNestingLevel,
                unsigned OutmostNestingLevel, unsigned *NewLoopLevels);

  UpdateDDRef(unsigned OutmostNestingLevel, unsigned InnermostNestingLevel,
              unsigned *NewLoopLevels)
      : OutmostNestingLevel(OutmostNestingLevel),
        InnermostNestingLevel(InnermostNestingLevel),
        NewLoopLevels(NewLoopLevels) {}

  void visit(const HLNode *Node) {}
  void visit(HLDDNode *Node) {
    updateDDRef(Node, InnermostNestingLevel, OutmostNestingLevel,
                NewLoopLevels);
  }
  void postVisit(HLNode *) {}
};

void UpdateDDRef::updateDDRef(HLDDNode *Node, unsigned InnermostNestingLevel,
                              unsigned OutmostNestingLevel,
                              unsigned *NewLoopLevels) {

  for (auto I = Node->ddref_begin(), E = Node->ddref_end(); I != E; ++I) {
    RegDDRef *RegRef = *I;

    for (auto Iter = RegRef->canon_begin(), E2 = RegRef->canon_end();
         Iter != E2; ++Iter) {
      CanonExpr *CE = *Iter;
      updateCE(CE, InnermostNestingLevel, OutmostNestingLevel, NewLoopLevels);
    }
    if (RegRef->hasGEPInfo()) {
      updateCE(RegRef->getBaseCE(), InnermostNestingLevel, OutmostNestingLevel,
               NewLoopLevels);
    }
  }
}

void UpdateDDRef::updateCE(CanonExpr *CE, unsigned InnermostNestingLevel,
                           unsigned OutmostNestingLevel,
                           unsigned *NewLoopLevels) {

  if (!(CE->hasIV())) {
    return;
  }

  // Save Coffs
  int64_t ConstCoeff[MaxLoopNestLevel];
  unsigned BlobCoeff[MaxLoopNestLevel];

  unsigned II = 0;
  for (II = OutmostNestingLevel; II <= InnermostNestingLevel; ++II) {
    ConstCoeff[II - 1] = 0;
    BlobCoeff[II - 1] = 0;
  }
  for (auto CurIVPair = CE->iv_begin(), E2 = CE->iv_end(); CurIVPair != E2;
       ++CurIVPair) {
    II = CE->getLevel(CurIVPair);
    ConstCoeff[II - 1] = CE->getIVConstCoeff(CurIVPair);
    BlobCoeff[II - 1] = CE->getIVBlobCoeff(CurIVPair);
  }

  // For each level, replace coeffs with the new one
  // Indexes to local arrays here start with 0
  // Levels used start with at least 1
  for (unsigned OL = OutmostNestingLevel; OL <= InnermostNestingLevel; ++OL) {
    unsigned NL = NewLoopLevels[OL - OutmostNestingLevel];
    if (OL == NL || (ConstCoeff[OL - 1] == 0 && ConstCoeff[NL - 1] == 0)) {
      continue;
    }
    CE->removeIV(OL);
    CE->addIV(OL, BlobCoeff[NL - 1], ConstCoeff[NL - 1]);
  }
}

void HIRLoopInterchange::printOptReport(HLLoop *Loop) {

  //  This will turn into Opt Report later
  //  e.g. Loopnest Interchanged: ( 2 3 4 ) --> ( 4 3 2 )
  //  Print out now for LIT
  //  Input *Loop will be used later when source info is ready

  raw_ostream &OS = dbgs();
  OS << "\n\tLoopnest Interchanged: ( ";
  for (unsigned I = OutmostNestingLevel; I <= InnermostNestingLevel; ++I) {
    OS << I << " ";
  }
  OS << ") --> ( ";
  for (auto &I : LoopPermutation) {
    OS << I->getNestingLevel() << " ";
  }
  OS << ")\n";
}

/// Update Loop Body
void HIRLoopInterchange::updateLoopBody(HLLoop *Loop) {

  unsigned NewLoopLevels[MaxLoopNestLevel];
  unsigned Idx = 0;

  for (auto &I : LoopPermutation) {
    NewLoopLevels[Idx++] = I->getNestingLevel();
  }

  UpdateDDRef UpdateDDRef(OutmostNestingLevel, InnermostNestingLevel,
                          &NewLoopLevels[0]);

  Loop->getHLNodeUtils().visit(UpdateDDRef, Loop);
}

void HIRLoopInterchange::transformLoop(HLLoop *Loop) {

  // Invalidate all analysis for InnermostLoop.
  HIRInvalidationUtils::invalidateBounds(InnermostLoop);
  HIRInvalidationUtils::invalidateBody(InnermostLoop);
  DEBUG(dbgs() << "\tBefore permuteloopNests:"; Loop->dump());
  HIRTransformUtils::permuteLoopNests(Loop, LoopPermutation);

  updateLoopBody(Loop);
  DEBUG(dbgs(); printOptReport(Loop));

  Loop->getParentRegion()->setGenCode();
  LoopsInterchanged++;
  AnyLoopInterchanged = true;
}
