//===-- HIRGeneralUnroll.cpp - Implements GeneralUnroll class -------------===//
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
// This file implements HIRGeneralUnroll class which unrolls a HIR loop
// with significantly larger trip count.
//
// For example:
//
// Original Loop                     Transformed ( UnrollFactor=8)
// for(i=0; i<N; i++)                t = (int)(N/8);
//    A[i] = B[i];                   for(iu=0; iu<=(t-1) ; iu++) {
//                                     A[iu*8] = B[iu*8];
//                                     ...
//                                     A[iu*8+7] = B[iu*8+7];
//
//                                   }
//                                   for(i=8*t; i<N; i++)
//                                     A[i] = B[i];
//
//                                    Note: 't' is avoided if N is constant
//
// The general algorithm is as follows:
//  1. Visit the Region
//  2. Extract the innermost loops
//  3. For each innermost loop
//    3.1 Get Trip Count and perform cost analysis. Ignore loops where not
//          profitable.
//    3.2 If Trip Count < Threshold, ignore this loop
//    3.3 Create a new Unrolled Loop
//    3.4 For UnrollCnt from [0 to UnrollFactor)
//          3.4.1 Append Cloned Original Loop Children into UnrolledLoop
//          3.4.2 Update Canon Exprs (IV*UnrollFactor + Coeff*UnrollCnt)
//                of UnrolledLoop Children.
//    3.5 Modify Original Loop to Remainder Loop with updated LowerBound
//        3.5.1 If Original Loop is Constant and TripCount%UnrollFactor = 0
//              Delete Original Loop as Remainder Loop is not needed.
//
// General Unrolling would increase the register pressure based on the unroll
// factor. Current heuristic just uses trip count to determine if loop needs
// to be unrolled.
//
//===----------------------------------------------------------------------===//

// TODO:
// 1) Optimize the remainder loop to produce switch statements. Think about
//    removing remainder loop if it is 1-trip for constant trip count loops.
// 2) Add a better heuristics for unrolling when platform characteristics are
//    supported.
// 3) Mark loops as modified for DD, which were transformed.
// 4) Update the reduction chain.
// 5) Add guard conditions for Preheader and Postexit. Refer older code.
//    e.g. if(t>0) then enter the unrolled loop.
// 6) Extend General Unrolling for cases where loop is not normalized.
// 7) Ztt support is added in unrolling. Add a working test case when utility
//    is added.
// 8) The Ztt of remainder loop can be avoided if we set t=(N-1)/8. Currently,
//    adding primary unrolled loop as focus. In this case, the remainder loop
//    is always executed. Investigate whether this version is better in
//    performance as compared to the existing one.

#include "llvm/Pass.h"

#include "llvm/ADT/Statistic.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

#define DEBUG_TYPE "hir-general-unroll"

using namespace llvm;
using namespace llvm::loopopt;

STATISTIC(LoopsGenUnrolled, "Number of HIR loops general unrolled");
STATISTIC(LoopsGenAnalyzed,
          "Number of HIR loops analyzed for general unrolling");

// TODO: This should be modified to a better heuristic.
static cl::opt<unsigned> GeneralUnrollTripThreshold(
    "genunroll-trip-threshold", cl::init(100), cl::Hidden,
    cl::desc("Don't unroll if innermost trip count is lesser than this,"
             "threshold."));

static cl::opt<unsigned>
    GeneralUnrollFactor("genunroll-factor", cl::init(8), cl::Hidden,
                        cl::desc("General Unrolling Factor for HIR Loop's."));

static cl::opt<bool>
    DisableHIRGeneralUnroll("disable-hir-general-unroll", cl::init(false),
                            cl::Hidden,
                            cl::desc("Disable HIR Loop General Unrolling"));

/// \brief Visitor to update the CanonExpr.
namespace {
class CanonExprVisitor final : public HLNodeVisitorBase {
private:
  unsigned Level;
  unsigned UnrollFactor;
  int64_t UnrollCnt;

  void processRegDDRef(RegDDRef *RegDD);
  void processCanonExpr(CanonExpr *CExpr);

public:
  CanonExprVisitor(unsigned L, unsigned UFactor, int64_t UCnt)
      : Level(L), UnrollFactor(UFactor), UnrollCnt(UCnt) {}

  /// \brief No processing needed for Goto
  void visit(HLGoto *Goto){};
  /// \brief No processing needed for Label
  void visit(HLLabel *Label){};
  void visit(HLDDNode *Node);
  void visit(HLNode *Node) {
    llvm_unreachable(" Node not supported for unrolling.");
  };
  void postVisit(HLNode *Node) {}
};

} // namespace

void CanonExprVisitor::visit(HLDDNode *Node) {

  // Only expecting if and inst inside the innermost loops.
  // Primarily to catch errors of other types.
  assert((isa<HLIf>(Node) || isa<HLInst>(Node)) && " Node not supported for "
                                                   "unrolling.");

  for (auto Iter = Node->ddref_begin(), End = Node->ddref_end(); Iter != End;
       ++Iter) {
    processRegDDRef(*Iter);
  }
}

/// processRegDDRef - Processes RegDDRef to call the Canon Exprs
/// present inside it.
/// This is an internal helper function.
void CanonExprVisitor::processRegDDRef(RegDDRef *RegDD) {

  // Process CanonExprs inside the RegDDRefs
  for (auto Iter = RegDD->canon_begin(), End = RegDD->canon_end(); Iter != End;
       ++Iter) {
    processCanonExpr(*Iter);
  }

  // Process GEP Base
  if (RegDD->hasGEPInfo()) {
    processCanonExpr(RegDD->getBaseCE());
  }
}

/// processCanonExpr - Processes CanonExpr to modify IV to
/// IV*UF + (Original IVCoeff)*UnrollCnt.
/// This is an internal helper function.
void CanonExprVisitor::processCanonExpr(CanonExpr *CExpr) {

  // Shift the canon expr to create the offset.
  if (UnrollCnt)
    CExpr->shift(Level, UnrollCnt);

  // IV*UF .
  CExpr->multiplyIVByConstant(Level, UnrollFactor);
}

namespace {

class HIRGeneralUnroll : public HIRTransformPass {
public:
  static char ID;

  HIRGeneralUnroll(int T = -1, int UFactor = -1) : HIRTransformPass(ID) {
    initializeHIRGeneralUnrollPass(*PassRegistry::getPassRegistry());

    // TODO: Decide on whether we need to expose parameters to users.
    // Also, bounds for these parameters.
    CurrentTripThreshold = (T == -1) ? GeneralUnrollTripThreshold : unsigned(T);
    UnrollFactor = (UFactor == -1) ? GeneralUnrollFactor : unsigned(UFactor);
  }

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRFramework>();
  }

private:
  unsigned CurrentTripThreshold;
  unsigned UnrollFactor;
  // TODO: Remove this when loop resource is added.
  const unsigned InstCountThreshold = 10;
  bool IsUnrollTriggered;

  /// \brief Main method to be invoked after all the innermost loops
  /// are gathered.
  void processGeneralUnroll(SmallVectorImpl<HLLoop *> &CandidateLoops);

  /// \brief Determines if Unrolling is profitable for the given Loop.
  bool isProfitable(const HLLoop *Loop, bool *IsConstLoop, int64_t *TripCount);

  /// \brief High level method which gives call to other sub-methods.
  void transformLoop(HLLoop *OrigLoop, bool IsConstLoop, int64_t TripCount);

  /// \brief Performs the actual unrolling.
  void processUnrollLoop(HLLoop *OrigLoop, HLLoop *UnrollLoop);

  /// \brief Processes the remainder loop and determines if it necessary.
  void processRemainderLoop(HLLoop *&OrigLoop, bool IsConstLoop,
                            int64_t TripCount, int64_t NewBound,
                            const RegDDRef *NewRef);

  /// \brief Updates bound DDRef by setting the correct defined at level and
  /// adding a blob DDref for the newly created temp.
  void updateBoundDDRef(RegDDRef *BoundRef, unsigned BlobIndex,
                        unsigned DefLevel);

  /// \brief Creates the unrolled loop.
  HLLoop *createUnrollLoop(HLLoop *OrigLoop, bool IsConstLoop,
                           int64_t TripCount, int64_t NewBound,
                           const RegDDRef *NewRef);

  /// \brief Return true if the loop has more instructions than threshold.
  bool exceedInstThreshold(const HLLoop *Loop);

  /// \brief Creates new bounds for unrolled loops.
  // NewUBInt is used for constant trip loops and NewUBRef is used for
  // non-constant trip loops.
  void createNewBound(HLLoop *OrigLoop, bool IsConstLoop, int64_t TripCount,
                      int64_t *NewConstBound, RegDDRef **NewRef);
};
}

char HIRGeneralUnroll::ID = 0;
INITIALIZE_PASS_BEGIN(HIRGeneralUnroll, "hir-general-unroll",
                      "HIR General Unroll", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_END(HIRGeneralUnroll, "hir-general-unroll",
                    "HIR General Unroll", false, false)

FunctionPass *llvm::createHIRGeneralUnrollPass(int Threshold, int UFactor) {
  return new HIRGeneralUnroll(Threshold, UFactor);
}

bool HIRGeneralUnroll::runOnFunction(Function &F) {
  // Skip if DisableHIRGeneralUnroll is enabled
  if (DisableHIRGeneralUnroll) {
    DEBUG(dbgs() << "HIR LOOP General Unroll Transformation Disabled \n");
    return false;
  }

  DEBUG(dbgs() << "General unrolling for Function : " << F.getName() << "\n");
  DEBUG(dbgs() << "Trip Count Threshold : " << CurrentTripThreshold << "\n");
  DEBUG(dbgs() << "GeneralUnrollFactor : " << UnrollFactor << "\n");

  IsUnrollTriggered = false;

  // Do an early exit if Trip Threshold is less than 1
  // TODO: Check if we want give some feedback to user
  if (CurrentTripThreshold == 0)
    return false;

  // Gather the innermost loops as candidates.
  SmallVector<HLLoop *, 64> CandidateLoops;
  HLNodeUtils::gatherInnermostLoops(CandidateLoops);

  // Process General Unrolling
  processGeneralUnroll(CandidateLoops);

  return IsUnrollTriggered;
}

// Nothing to release?
void HIRGeneralUnroll::releaseMemory() {}

/// processGeneralUnroll - Main routine to perform unrolling.
/// First, performs cost analysis and then do the transformation.
void HIRGeneralUnroll::processGeneralUnroll(
    SmallVectorImpl<HLLoop *> &CandidateLoops) {

  int64_t TripCount = 0;
  bool isConstantLoop = false;

  // Visit each candidate loop to run cost analysis.
  for (auto Iter = CandidateLoops.begin(), End = CandidateLoops.end();
       Iter != End; ++Iter, TripCount = 0, isConstantLoop = false) {

    HLLoop *Loop = (*Iter);

    // Perform a cost/profitability analysis on the loop
    // If all conditions are met, unroll it.
    if (isProfitable(Loop, &isConstantLoop, &TripCount)) {
      transformLoop(Loop, isConstantLoop, TripCount);
      IsUnrollTriggered = true;
      LoopsGenUnrolled++;
    }
    LoopsGenAnalyzed++;
  }
}

// Temporarily added as a basic cost model.
// TODO: Remove this when more heuristics are added in the cost model.
struct InstVisitor final : public HLNodeVisitorBase {

  unsigned Threshold;
  unsigned InstCount;

  InstVisitor(unsigned Thres) : Threshold(Thres), InstCount(0) {}

  void visit(const HLInst *I) { InstCount += 1; }
  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}
  bool isDone() const override { return exceedInstThreshold(); }
  bool exceedInstThreshold() const { return (InstCount > Threshold); }
};

bool HIRGeneralUnroll::exceedInstThreshold(const HLLoop *Loop) {

  InstVisitor IVisit(InstCountThreshold);
  HLNodeUtils::visit(IVisit, Loop);
  return IVisit.exceedInstThreshold();
}

/// isProfitable - Check if the loop trip count is less
/// than the trip count threshold. Return true, if this loop
/// is a candidate for general unrolling.
bool HIRGeneralUnroll::isProfitable(const HLLoop *Loop, bool *IsConstLoop,
                                    int64_t *TripCount) {

  // TODO: Preheader and PostExit not handled currently.
  if (Loop->hasPreheader() || Loop->hasPostexit()) {
    return false;
  }

  if (!Loop->hasChildren()) {
    return false;
  }

  // Ignore loops with SIMD directive.
  if (Loop->isSIMD()) {
    return false;
  }

  // Loop should be normalized before this pass
  // TODO: Decide whether we can remove this, just to save compile time.
  if (!Loop->isNormalized() || Loop->isUnknown()) {
    return false;
  }

  // TODO: Add loop resource analysis.
  // E.g. if insts reach above threshold, don't unroll.
  // Currently adding some simple instruction count.
  if (exceedInstThreshold(Loop)) {
    return false;
  }

  if (Loop->isConstTripLoop(TripCount)) {
    if ((*TripCount) < CurrentTripThreshold)
      return false;
    *IsConstLoop = true;
  } else {
    *IsConstLoop = false;
  }

  // Ignore loops which have switch or function calls for unrolling.
  if (HLNodeUtils::hasSwitchOrCall(Loop->getFirstChild(),
                                   Loop->getLastChild())) {
    return false;
  }

  return true;
}

// transformLoop - Perform the unrolling transformation for
// the given loop.
void HIRGeneralUnroll::transformLoop(HLLoop *OrigLoop, bool IsConstLoop,
                                     int64_t TripCount) {

  DEBUG(dbgs() << "\t GeneralUnroll Loop: ");
  DEBUG(OrigLoop->dump());

  // Extract Ztt and add it outside the loop.
  OrigLoop->extractZtt();

  // Create UB instruction before the loop 't = (Orig UB)/(UnrollFactor)' for
  // non-constant trip loops. For const trip loops calculate the bound.
  RegDDRef *NewRef = nullptr;
  int64_t NewConstBound = 0;
  createNewBound(OrigLoop, IsConstLoop, TripCount, &NewConstBound, &NewRef);

  // Create the unrolled main loop.
  HLLoop *UnrollLoop =
      createUnrollLoop(OrigLoop, IsConstLoop, TripCount, NewConstBound, NewRef);

  processUnrollLoop(OrigLoop, UnrollLoop);

  // Update the OrigLoop to remainder loop.
  processRemainderLoop(OrigLoop, IsConstLoop, TripCount, NewConstBound, NewRef);

  // Mark parent loop as modified, if it exists.
  if (auto ParentLoop = UnrollLoop->getParentLoop()) {
    HIRInvalidationUtils::invalidateBody(ParentLoop);
  } else if (!IsConstLoop) {
    // Mark region as modified as we have inserted a new instruction.
    HIRInvalidationUtils::invalidateNonLoopRegion(
        UnrollLoop->getParentRegion());
  }

  DEBUG(dbgs() << "\n\t Transformed GeneralUnroll Loops No:"
               << LoopsGenUnrolled);
  DEBUG(UnrollLoop->dump());
}

void HIRGeneralUnroll::createNewBound(HLLoop *OrigLoop, bool IsConstLoop,
                                      int64_t TripCount, int64_t *NewConstBound,
                                      RegDDRef **NewRef) {

  if (IsConstLoop) {
    *NewConstBound = (int64_t)(TripCount / UnrollFactor) - 1;
    return;
  }

  // Process for non-const trip loop.
  RegDDRef *Ref = OrigLoop->getTripCountDDRef();
  // New instruction should only be created for non-constant trip loops.
  assert(!Ref->isIntConstant() && " Creating a new instruction for constant"
                                  "trip loops should not occur.");

  // This will create a new instruction for calculating ub of unrolled loop
  // and lb of remainder loop. The new instruction is t = (N/8) where 'N' is
  // the trip count of the original loop.
  HLInst *TempInst = nullptr;
  CanonExpr *TripCE = Ref->getSingleCanonExpr();
  if (TripCE->isSignedDiv() && (TripCE->getDenominator() != 1)) {
    // Create DDRef for Unroll Factor.
    RegDDRef *UFDD =
        DDRefUtils::createConstDDRef(Ref->getDestType(), UnrollFactor);
    TempInst = HLNodeUtils::createUDiv(Ref, UFDD, "tgu");
  } else {
    SmallVector<const RegDDRef *, 3> AuxRefs = {OrigLoop->getStrideDDRef(),
                                                OrigLoop->getLowerDDRef(),
                                                OrigLoop->getUpperDDRef()};

    // Use the same canon expr to generate the division.
    TripCE->divide(UnrollFactor, true);

    Ref->setSymbase(DDRefUtils::getNewSymbase());

    Ref->makeConsistent(&AuxRefs, OrigLoop->getNestingLevel() - 1);

    TempInst = HLNodeUtils::createCopyInst(Ref, "tgu");
  }
  HLNodeUtils::insertBefore(const_cast<HLLoop *>(OrigLoop), TempInst);
  *NewRef = TempInst->getLvalDDRef();
}

void HIRGeneralUnroll::updateBoundDDRef(RegDDRef *BoundRef, unsigned BlobIndex,
                                        unsigned DefLevel) {
  // Overwrite symbase to a newly created one to avoid unnecessary DD edges.
  BoundRef->setSymbase(DDRefUtils::getNewSymbase());

  // Add blob DDRef for the temp in UB.
  BoundRef->addBlobDDRef(BlobIndex, DefLevel);
  BoundRef->updateDefLevel();
}

HLLoop *HIRGeneralUnroll::createUnrollLoop(HLLoop *OrigLoop, bool IsConstLoop,
                                           int64_t TripCount, int64_t NewBound,
                                           const RegDDRef *NewRef) {

  // TODO: Not sure if we need to add Ztt?
  // Currently the clone utility handles it.
  HLLoop *NewLoop = OrigLoop->cloneEmptyLoop();
  NewLoop->setNumExits((OrigLoop->getNumExits() - 1) * UnrollFactor + 1);

  HLNodeUtils::insertBefore(OrigLoop, NewLoop);

  // Update the loop upper bound.
  if (IsConstLoop) {
    assert((NewBound > 0) && " NewBound cannot be zero or less.");
    NewLoop->getUpperCanonExpr()->setConstant(NewBound);
  } else {

    // Create 't-1' as new UB.
    assert(NewRef && " New Ref is null.");
    RegDDRef *NewUBRef = NewRef->clone();

    // Subtract 1.
    NewUBRef->getSingleCanonExpr()->addConstant(-1);

    NewLoop->setUpperDDRef(NewUBRef);

    // Sets defined at level of bound ref to (nesting level - 1) as the UB temp
    // is defined just before the loop.
    updateBoundDDRef(NewUBRef, NewRef->getSelfBlobIndex(),
                     OrigLoop->getNestingLevel() - 1);

    // Generate the Ztt.
    NewLoop->createZtt(false);
  }

  // Set the code gen for modified region
  assert(NewLoop->getParentRegion() && " Loop does not have a parent region.");
  NewLoop->getParentRegion()->setGenCode();

  return NewLoop;
}

void HIRGeneralUnroll::processUnrollLoop(HLLoop *OrigLoop, HLLoop *UnrollLoop) {

  // Container for cloning body.
  HLContainerTy LoopBody;

  // Loop through the 0th iteration unrolled loop children and create new
  // children
  // with updated References based on unroll factor.
  for (int64_t UnrollCnt = 0; UnrollCnt < UnrollFactor; ++UnrollCnt) {

    // Clone 0th iteration
    HLNodeUtils::cloneSequence(&LoopBody, OrigLoop->getFirstChild(),
                               OrigLoop->getLastChild());

    // Store references as LoopBody will be empty after insertion.
    HLNode *CurFirstChild = &(LoopBody.front());
    HLNode *CurLastChild = &(LoopBody.back());

    HLNodeUtils::insertAsLastChildren(UnrollLoop, &LoopBody);

    CanonExprVisitor CEVisit(UnrollLoop->getNestingLevel(), UnrollFactor,
                             UnrollCnt);
    HLNodeUtils::visitRange(CEVisit, CurFirstChild, CurLastChild);
  }
}

void HIRGeneralUnroll::processRemainderLoop(HLLoop *&OrigLoop, bool IsConstLoop,
                                            int64_t TripCount, int64_t NewBound,
                                            const RegDDRef *NewRef) {
  // Mark Loop bounds as modified.
  HIRInvalidationUtils::invalidateBounds(OrigLoop);

  // Check if the Remainder Loop is necessary.
  // This condition occurs when the original constant Trip Count is divided by
  // UnrollFactor without a remainder.
  if (IsConstLoop && (TripCount % UnrollFactor == 0)) {
    HLNodeUtils::erase(OrigLoop);
    OrigLoop = nullptr;
    return;
  }

  // Modify the LB of original loop.
  if (IsConstLoop) {
    // OrigLoop is a const-trip loop.
    RegDDRef *OrigLBRef = OrigLoop->getLowerDDRef();
    CanonExpr *LBCE = OrigLBRef->getSingleCanonExpr();
    LBCE->setConstant((NewBound + 1) * UnrollFactor);
  } else {

    // Non-constant trip loop, lb = (UnrollFactor)*t.
    RegDDRef *NewLBRef = NewRef->clone();
    NewLBRef->getSingleCanonExpr()->multiplyByConstant(UnrollFactor);

    OrigLoop->setLowerDDRef(NewLBRef);
    // Sets the defined at level of new LB to (nesting level - 1) as the LB temp
    // is defined just before the loop.
    updateBoundDDRef(NewLBRef, NewRef->getSelfBlobIndex(),
                     OrigLoop->getNestingLevel() - 1);

    OrigLoop->createZtt(false);
  }

  DEBUG(dbgs() << "\n Remainder Loop \n");
  DEBUG(OrigLoop->dump());
}
