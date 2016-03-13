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
// Input:  Temporal & spatial locality reuse per loop nest
//         DDG
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

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRLocalityAnalysis.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRInvalidationUtils.h"

#define DEBUG_TYPE "hir-loopinterchange"

using namespace llvm;
using namespace llvm::loopopt;
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
    AU.addRequiredTransitive<DDAnalysis>();
    AU.addRequiredTransitive<HIRLocalityAnalysis>();
  }

private:
  Function *F;
  DDAnalysis *DDA;
  HIRLocalityAnalysis *LA;
  unsigned OutmostNestingLevel;
  unsigned InnermostNestingLevel;
  HLLoop *InnermostLoop;
  struct CollectDDInfo;

  SmallVector<CandidateLoopPair, 12> CandidateLoops;
  SmallVector<LoopLocalityPair, MaxLoopNestLevel> LoopLocality;
  SmallVector<HLLoop *, MaxLoopNestLevel> LoopPermutation;
  SmallVector<HLLoop *, MaxLoopNestLevel> NearByPerm;
  SmallVector<DVType *, 16> DVs;

  bool shouldInterchange(const HLLoop *);
  bool getPermutation(const HLLoop *);
  // returns true means legal for any permutation
  bool legalForAnyPermutation(const HLLoop *Loop);
  bool legalToShiftLoop(unsigned StartLevel, unsigned EndLevel, bool InToOut);

  bool bestLocalityInInnermost(const HLLoop *Loop,
                               const HLLoop *BestLocalityLoop);
  void getNearbyPermutation(const HLLoop *Loop);
  bool legalForPermutation(const HLLoop *StartLoop, const HLLoop *SrcLoop,
                           unsigned DstLevel);
  void transformLoop(HLLoop *Loop);
  void updateLoopBody(HLLoop *Loop);
  void printOptReport(HLLoop *Loop);
};
}

char HIRLoopInterchange::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLoopInterchange, "HIRLoopInterchange",
                      "HIR Loop Interchange", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_DEPENDENCY(DDAnalysis)
INITIALIZE_PASS_DEPENDENCY(HIRLocalityAnalysis)
INITIALIZE_PASS_END(HIRLoopInterchange, "HIRLoopInterchange",
                    "HIR Loop Interchange", false, false)

/// Gather all perfect Loop Nest (will improve later)
struct CollectCandidateLoops final : public HLNodeVisitorBase {

  SmallVector<CandidateLoopPair, 12> *CandidateLoops;
  HLNode *SkipNode;
  CollectCandidateLoops(SmallVector<CandidateLoopPair, 12> *CandidateLoops)
      : CandidateLoops(CandidateLoops), SkipNode(nullptr) {}

  void visit(HLLoop *Loop) {
    // Gather perfect loop nests (for now, will improve later)
    // TODO: Skip Loop with hasUserCall when flag is set
    const HLLoop *InnermostLoop;
    if (Loop->isInnermost()) {
      SkipNode = Loop;
      return;
    }
    if (HLNodeUtils::isPerfectLoopNest(Loop, &InnermostLoop)) {
      CandidateLoops->push_back(
          std::make_pair(Loop, const_cast<HLLoop *>(InnermostLoop)));
      SkipNode = Loop;
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
  DEBUG(dbgs() << "Loop Interchange for Function : " << F.getName() << "\n");

  this->F = &F;
  DDA = &getAnalysis<DDAnalysis>();
  LA = &getAnalysis<HIRLocalityAnalysis>();

  // 1) Walk all loops, look for outer loops that are perfectly nested

  CollectCandidateLoops CCL(&CandidateLoops);

  HLNodeUtils::visitAll(CCL);

  for (auto Iter = CandidateLoops.begin(), End = CandidateLoops.end();
       Iter != End; ++Iter) {
    HLLoop *Loop = Iter->first;
    InnermostLoop = Iter->second;
    InnermostNestingLevel = InnermostLoop->getNestingLevel();

    if (shouldInterchange(Loop) && getPermutation(Loop)) {
      transformLoop(Loop);
    }
  }

  CandidateLoops.clear();

  return false;
}

bool HIRLoopInterchange::shouldInterchange(const HLLoop *Loop) {

  LoopLocality.clear();
  unsigned PrevLevel = 1;
  bool InterchangeNeeded = false;

  // Call Util in Locality Analysis to get best Permutation
  LA->sortedLocalityLoops(Loop, LoopLocality);

  for (auto &I : LoopLocality) {
    HLLoop *L = const_cast<HLLoop *>(I.first);

    unsigned Level = L->getNestingLevel();
    if (PrevLevel > Level) {
      InterchangeNeeded = true;
      break;
    }
    PrevLevel = Level;
  }

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
  for (auto &I : LoopLocality) {
    HLLoop *L = const_cast<HLLoop *>(I.first);
    LoopPermutation.push_back(L);
  }

  // When returning legal == true, means we can just interchange w/o
  // examining DV.
  if (legalForAnyPermutation(Loop)) {
    DEBUG(dbgs() << "\n\tBest permutation available\n");
    CanInterchange = true;
  } else {
    // Check if largest locality can be moved as innermost loop
    // If no, Stop. Otherwise while loop in next function will loop forever

    const HLLoop *BestLocalityLoop = LoopPermutation.back();
    if (!bestLocalityInInnermost(Loop, BestLocalityLoop)) {
      CanInterchange = false;
    } else {
      // Find Nearby permutation
      getNearbyPermutation(Loop);
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

  for (auto &I : DVs) {
    delete I;
  }

  return CanInterchange;
}

///  Check if the best locality loop can stay or move as innermost
bool HIRLoopInterchange::bestLocalityInInnermost(
    const HLLoop *Loop, const HLLoop *BestLocalityLoop) {

  if (InnermostNestingLevel == BestLocalityLoop->getNestingLevel() ||
      legalForPermutation(Loop, BestLocalityLoop, InnermostNestingLevel)) {
    return true;
  }
  DEBUG(dbgs() << "\nCannot move best LOC loop as innermost\n");
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
///  An example: assuming just 1  dv ( < = >)
///  Best & legal permutation is L = (l3, l2, l1)
///  j=1,3 in best permutation
///  Is l3 legal as outermost?  No
///  Is l2 legal as outermost?  Yes. P = (l2) L = (l3 l1)
///  Is l2 legal as 2nd level?  Yes. P = (l2, l1) L = (l3)
///  When while loop exits,  P = (l2,l1,l3), L = null
void HIRLoopInterchange::getNearbyPermutation(const HLLoop *Loop) {

  unsigned DstLevel = 1;
  unsigned Iter = 0;

  //  Based on the paper above, the while loop will halt when
  //  the last loop in L can be moved legally as the innermost loop,
  //  which is verified before calling this function.

  while (!LoopPermutation.empty()) {
    for (auto I = LoopPermutation.begin(), E = LoopPermutation.end(); I != E;
         ++I) {
      if (legalForPermutation(Loop, *I, DstLevel)) {
        NearByPerm.push_back(*I);
        LoopPermutation.erase(I);
        DstLevel++;
        break;
      }
      Iter += 1;
      //  Assert addedd to avoid looping
      assert((Iter < MaxLoopNestLevel * MaxLoopNestLevel) &&
             "NearbyPermutation is looping");
    }
  }
}

///  1. Ignore all  (= = ..)
///  2. for temps, ignore  anti (< ..)
///     If there is a loop carried flow for scalars, the DV will not
///     be all =
///     (check no longer needed because of change in DD for compile time
///     saving)
///  3. Safe reduction (already done before calling here)
static bool ignoreEdge(const DDEdge *Edge, HLLoop *CandidateLoop,
                       DVType *RefinedDV = nullptr) {

  const DVType *DV = RefinedDV;
  if (DV == nullptr) {
    DV = Edge->getDV();
  }

  if (isDValEQ(DV)) {
    return true;
  }

  HLLoop *Loop = CandidateLoop;

  if (isDVIndepFromLevel(DV, Loop->getNestingLevel())) {
    return true;
  }

  if (Edge->isINPUTdep()) {
    return true;
  }

  return false;
}

///  Scan presence of  <  ... >
///  If none, return true, whiich  means DV can be dropped for
///  Interchange legality checking
static bool ignoreDVWithNoLTGT(const DVType *DV, unsigned OutmostNestingLevel,
                               unsigned InnermostNestingLevel) {

  bool DVhasLT = false;
  bool DVhasGT = false;

  for (unsigned II = OutmostNestingLevel; II <= InnermostNestingLevel; ++II) {
    if (!DVhasGT && (DV[II - 1] & DV::GT)) {
      DVhasGT = true;
    }
    if (!DVhasLT && (DV[II - 1] & DV::LT)) {
      DVhasLT = true;
    }
  }

  // Note: 2 stars will get both boolean flags set on

  if (!DVhasLT || !DVhasGT) {
    return true;
  }
  return false;
}

/// Collect all DV edges in loop nests
/// Call Demand Driven DD as needed (Currently  not calling)
struct HIRLoopInterchange::CollectDDInfo final : public HLNodeVisitorBase {

  HIRLoopInterchange *LIP;
  DDGraph DDG;
  HLLoop *CandidateLoop;

  CollectDDInfo(HIRLoopInterchange *LoopIP, DDGraph DDGraph,
                HLLoop *CandidateLoop, bool RefineDV)
      : LIP(LoopIP), DDG(DDGraph), CandidateLoop(CandidateLoop),
        RefineDV(RefineDV) {
    DVs.clear();
  }

  // Indicates if we need to call Demand Driven DD to refine DV
  bool RefineDV;
  SmallVector<DVType *, 16> DVs;
  // start, end level of Candidate Loop nest

  void visit(HLNode *Node) {

    HLInst *Inst = dyn_cast<HLInst>(Node);
    if (!Inst) {
      return;
    }
    if (Inst->isSafeRedn()) {
      return;
    }
    for (auto I = Inst->ddref_begin(), E = Inst->ddref_end(); I != E; ++I) {
      if ((*I)->isConstant()) {
        continue;
      }
      for (auto II = DDG.outgoing_edges_begin(*I),
                EE = DDG.outgoing_edges_end(*I);
           II != EE; ++II) {
        // Examining outoging edges is sufficent
        DDRef *DDref = II->getSink();
        if (!(HLNodeUtils::contains(CandidateLoop, DDref->getHLDDNode()))) {
          continue;
        }
        const DDEdge *edge = &(*II);
        if (ignoreEdge(edge, CandidateLoop)) {

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
          DEBUG(dbgs() << "\n\t<Edge dropped>");
          DEBUG(dbgs() << "\t"; II->print(dbgs()));
#endif
          continue;
        }
        const DVType *TempDV = II->getDV();

        // Calling Demand Driven DD to refine DV
        DVectorTy RefinedDV;
        if (RefineDV) {
          DDRef *SrcDDRef = II->getSrc();
          DDRef *DstDDRef = DDref;
          bool IsIndep;
          bool IsDVRefined =
              refineDV(SrcDDRef, DstDDRef, LIP->InnermostNestingLevel,
                       LIP->OutmostNestingLevel, RefinedDV, &IsIndep);
          if (IsIndep) {
            continue;
          }
          if (IsDVRefined) {
            if (ignoreEdge(edge, CandidateLoop, &RefinedDV[0])) {
              continue;
            }
            TempDV = &RefinedDV[0];
          }
        }
        if (ignoreDVWithNoLTGT(TempDV, LIP->OutmostNestingLevel,
                               LIP->InnermostNestingLevel)) {
          continue;
        }

        //  Save the DV in an array which will be used lter
        DVType *WorkDV = new DVectorTy;
        memcpy(WorkDV, TempDV, MaxLoopNestLevel);
        DVs.push_back(WorkDV);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
        DEBUG(dbgs() << "\n\t<Edge selected>");
        DEBUG(dbgs() << "\t"; II->print(dbgs()));
#endif
      }
    }
  }
  void postVisit(HLNode *) {}
};

bool HIRLoopInterchange::legalToShiftLoop(unsigned StartLevel,
                                          unsigned EndLevel, bool InToOut) {

  DVType SrcDV;
  DVType DstDV;
  unsigned LevelForSrcDV;

  //  InToOut = true if moving from inner to outer
  //  LevelForSrcDV indicates where to pick up DV
  //  e.g. for InToOut = true
  //               ( < = = > =)
  //  Assuming we try to move Loop from level 4 to level 1.
  //        EndLevel = 1,  StartLevel = 3, LevelForSrcDV = 4

  if (InToOut) {
    SrcDV = DV::GT;
    DstDV = DV::LT;
    LevelForSrcDV = EndLevel + 1;
  } else {
    SrcDV = DV::LT;
    DstDV = DV::GT;
    LevelForSrcDV = StartLevel - 1;
  }
  for (auto &II : DVs) {
    DVType *WorkDV = II;
    if (WorkDV[LevelForSrcDV - 1] & SrcDV) {
      for (unsigned JJ = StartLevel; JJ <= EndLevel; ++JJ) {
        if (WorkDV[JJ - 1] & DstDV) {
          return false;
        }
      }
    }
  }
  return true;
}

///  Check all DV to see if it's legal to move SrcLoop pass DstLevel
///  e.g. Assuming  dv = (< = >),  ScrLoop is the 3rd level loop
///                 DstLevel   = 1
///       It will return false because > cannot cross <
bool HIRLoopInterchange::legalForPermutation(const HLLoop *StartLoop,
                                             const HLLoop *SrcLoop,
                                             unsigned DstLevel) {

  //  Adjust DstLevel based on OutmostNestingLevel
  DstLevel += OutmostNestingLevel - 1;
  unsigned SrcLevel = SrcLoop->getNestingLevel();

  if (SrcLevel == DstLevel) {
    return true;
  }
  if (SrcLevel > DstLevel) {
    return legalToShiftLoop(DstLevel, SrcLevel - 1, true);
  }
  return legalToShiftLoop(SrcLevel + 1, DstLevel, false);
}

/// Return true if legal for any permutations
bool HIRLoopInterchange::legalForAnyPermutation(const HLLoop *Loop) {

  // TODO
  // 4) exclude loops that has pragma for unroll or unroll & jam
  //    exclude triangular loop for now

  // 5) Gather DV from DDG. Filter out loop indep dep for temps,
  // safe reduction
  // We plan to avoid demand driven DD refining DV.
  // Will set last srgument of WalkHIR as false later after testing

  HLLoop *Loop2 = const_cast<HLLoop *>(Loop);

  DEBUG(dbgs() << "\n\tStart, End level\n" << OutmostNestingLevel << " "
               << InnermostNestingLevel);

  DDGraph DDG = DDA->getGraph(Loop2, false);

  //  Set refineDV as false for now (last argument) until we see kernels
  //  that really need to refine DV.

  CollectDDInfo CDD(this, DDG, Loop2, false);
  HLNodeUtils::visit(CDD, Loop2);

  DVs = CDD.DVs;

  // If edges are selected,
  // there are dependencies to check out w.r.t to interchange order

  if (DVs.size() > 0) {
    return false;
  }
  DEBUG(dbgs() << "\n\tDV array is empty\n");
  return true;
}

static void updateCE(CanonExpr *CE, unsigned InnermostNestingLevel,
                     unsigned OutmostNestingLevel, unsigned *NewLoopLevels) {

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

static void updateDDRefCE(HLDDNode *Node, unsigned InnermostNestingLevel,
                          unsigned OutmostNestingLevel,
                          unsigned *NewLoopLevels) {

  for (auto I = Node->ddref_begin(), E = Node->ddref_end(); I != E; ++I) {
    RegDDRef *RegRef = *I;
    if (!RegRef || RegRef->isConstant()) {
      continue;
    }
    for (auto Iter = RegRef->canon_begin(), E = RegRef->canon_end(); Iter != E;
         ++Iter) {
      CanonExpr *CE = *Iter;
      updateCE(CE, InnermostNestingLevel, OutmostNestingLevel, NewLoopLevels);
    }
    if (RegRef->hasGEPInfo()) {
      updateCE(RegRef->getBaseCE(), InnermostNestingLevel, OutmostNestingLevel,
               NewLoopLevels);
    }
  }
}

// Update DD REF
struct UpdateDDRef final : public HLNodeVisitorBase {

  // Smallest value of OutmostNestingLevel & InnermostNestingLevel is 1.
  unsigned OutmostNestingLevel;
  unsigned InnermostNestingLevel;
  unsigned *NewLoopLevels;
  UpdateDDRef(unsigned OutmostNestingLevel, unsigned InnermostNestingLevel,
              unsigned *NewLoopLevels)
      : OutmostNestingLevel(OutmostNestingLevel),
        InnermostNestingLevel(InnermostNestingLevel),
        NewLoopLevels(NewLoopLevels) {}

  void visit(const HLNode *Node) {}
  void visit(HLDDNode *Node) {
    updateDDRefCE(Node, InnermostNestingLevel, OutmostNestingLevel,
                  NewLoopLevels);
  }
  void postVisit(HLNode *) {}
};

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

  UpdateDDRef UpdateDDDRef(OutmostNestingLevel, InnermostNestingLevel,
                           &NewLoopLevels[0]);

  HLNodeUtils::visit(UpdateDDDRef, Loop);
}

void HIRLoopInterchange::transformLoop(HLLoop *Loop) {

  // Invalidate all analysis for InnermostLoop.
  // TODO: we should probably invalidate analysis for all the involved loops.
  HIRInvalidationUtils::invalidateBounds(InnermostLoop);
  HIRInvalidationUtils::invalidateBody(InnermostLoop);

  HLNodeUtils::permuteLoopNests(Loop, LoopPermutation);
  updateLoopBody(Loop);
  printOptReport(Loop);
  Loop->getParentRegion()->setGenCode();
}
