//===- HIRCompleteUnroll.cpp - Implements CompleteUnroll class ------------===//
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
// This file implements HIRCompleteUnroll class which unrolls a HIR loop
// with small trip count.
//
// For example:
//
// Original Loop                     Transformed
// for(i=0; i<5; i++)                A[0] = B[0];
//    A[i] = B[i];                   A[1] = B[1];
//                                   ...
//                                   A[4] = B[4];
//
// The general algorithm is as follows:
//  1. Visit the Region
//  2. Recurse from outer to inner loops (Gathering phase for Transformation)
//       Perform cost analysis on the loop ( such as trip count of inner loops )
//       If all criteria meet, add loop to transformation list and return true,
//       Else return false indicating the parent loops should not be unrolled.
//  4. For each loop (inner to outer) of Transformed Loops
//       4.1 Clone LoopChild and insert it before the loop.
//       4.2 Update CanonExprs of LoopChild.
//       4.3 Delete Loop
//
// Unrolling would increase the register pressure based on the unroll factor.
// Current heuristic just uses trip count to determine if loop needs to be
// unrolled.
//
// Works by unrolling transformation from innermost to outermost.
// It avoids outer loops if any of the inner loops are not completely unrolled.
// No candidate loops should have a switch or call statement.
//
//===----------------------------------------------------------------------===//

// TODO: Extensions to be added later.
//  (1) We want Unroll and rebuild DDG to happen once in complete unrolling
//      multi-level cases  for the sake of compile time.
//  (2) Some  rebuildDDG  util need to be invoked before and after unroll for
//      the sake of incremental rebuild
//  (3) Safe reductions chains need to be updated or removed
//  (4) Using a simple heuristic (TripCount) for this implementation. We need to
//     extend it later to incorporate register pressure. Also, for multi-level
//     loops, we are currently summing the trip counts for the loop nest.
//  (5) Handle preheader and postexit of loops during transformation.
//  (6) Conduct some experiments to determine if going from inner to outer saves
//     compile time. Experiment if unrolling HLIf's increases/decreases
//     performance.

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRInvalidationUtils.h"

#define DEBUG_TYPE "hir-complete-unroll"

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<unsigned> CompleteUnrollTripThreshold(
    "completeunroll-trip-threshold", cl::init(20), cl::Hidden,
    cl::desc("Don't unroll if total trip count is bigger than this,"
             "threshold."));

// Option to turn off the HIR Complete Unroll Pass
static cl::opt<bool>
    DisableHIRCompleteUnroll("disable-hir-complete-unroll", cl::init(false),
                             cl::Hidden,
                             cl::desc("Disable HIR Loop Complete Unrolling"));

namespace {

/// \brief Visitor to update the CanonExpr.
class CanonExprVisitor final : public HLNodeVisitorBase {
private:
  unsigned Level;
  int64_t TripVal;

  void processRegDDRef(RegDDRef *RegDD);
  void processCanonExpr(CanonExpr *CExpr);

public:
  CanonExprVisitor(unsigned L, int64_t TripV) : Level(L), TripVal(TripV) {}

  void visit(HLDDNode *Node);
  // No processing needed for Goto and Label's
  void visit(HLGoto *Goto){};
  void visit(HLLabel *Label){};
  void visit(HLNode *Node) {
    llvm_unreachable(" Node not supported for Complete Unrolling.");
  }
  void postVisit(HLNode *Node) {}
};

} // namespace

void CanonExprVisitor::visit(HLDDNode *Node) {

  // Only expecting if and inst inside the loops.
  // Primarily to catch errors of other types.
  assert((isa<HLIf>(Node) || isa<HLInst>(Node)) && " Node not supported for "
                                                   "complete unrolling.");

  for (auto Iter = Node->ddref_begin(), End = Node->ddref_end(); Iter != End;
       ++Iter) {
    processRegDDRef(*Iter);
  }
}

/// processRegDDRef - Processes RegDDRef to call the Canon Exprs
/// present inside it. This is an internal helper function.
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

  RegDD->makeConsistent();

  // Example of an alternative way of updating DDRef which is useful when some
  // manual work is also involved-
  //
  // RegDD->updateBlobDDRefs(BlobDDRefs);
  // assert(BlobDDRefs.empty() && "New blobs found in DDRef after processing!");
  // RegDD->updateCELevel();
}

/// Processes CanonExpr to replace IV by TripVal.
/// This is an internal helper function.
void CanonExprVisitor::processCanonExpr(CanonExpr *CExpr) {
  DEBUG(dbgs() << "Replacing CanonExpr IV by tripval :" << TripVal << " \n");
  CExpr->replaceIVByConstant(Level, TripVal);
}

namespace {
/// \brief Data structure to store loop information.
/// Extend later to store triangular loop information.
struct LoopData {
  // Loop Lower Bound.
  int64_t LB;
  // Loop Upper Bound.
  int64_t UB;
  // Loop Step Value.
  int64_t Step;

  LoopData(int64_t LowerB, int64_t UpperB, int64_t Stride)
      : LB(LowerB), UB(UpperB), Step(Stride) {}
};

class HIRCompleteUnroll : public HIRTransformPass {
public:
  static char ID;

  HIRCompleteUnroll(int T = -1) : HIRTransformPass(ID) {
    initializeHIRCompleteUnrollPass(*PassRegistry::getPassRegistry());

    CurrentTripThreshold =
        (T == -1) ? CompleteUnrollTripThreshold : unsigned(T);
  }

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRFramework>();
  }

private:
  unsigned CurrentTripThreshold;

  // Storage for Outermost Loops
  SmallVector<HLLoop *, 64> OuterLoops;

  /// Storage for loops which will be transformed.
  /// The ordering inside the container is from inner to outer.
  SmallVector<std::pair<HLLoop *, LoopData *>, 32> TransformLoops;

  /// \brief Performs cost analysis to determine if a loop
  /// is eligible for complete unrolling. If loop meets all the criteria,
  /// it return true, else false. This routine updates the child trip count
  /// for use by parent loop.
  bool isProfitable(const HLLoop *Loop, LoopData **LD, int64_t *ChildTripCnt);

  /// \brief Performs the complete unrolling transformation.
  void transformLoop(HLLoop *&Loop, LoopData *LD);

  /// \brief Main routine to drive the complete unrolling transformation.
  void processCompleteUnroll();

  /// \brief Processes a HLLoop to check if it candidate for transformation.
  /// ChildTripCnt denotes the trip count of the children.
  bool processLoop(HLLoop *Loop, int64_t *ChildTripCnt);

  /// \brief Routine to drive the transformation of candidate loops.
  void transformLoops();
};
}

char HIRCompleteUnroll::ID = 0;
INITIALIZE_PASS_BEGIN(HIRCompleteUnroll, "hir-complete-unroll",
                      "HIR Complete Unroll", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_END(HIRCompleteUnroll, "hir-complete-unroll",
                    "HIR Complete Unroll", false, false)

FunctionPass *llvm::createHIRCompleteUnrollPass(int Threshold) {
  return new HIRCompleteUnroll(Threshold);
}

bool HIRCompleteUnroll::runOnFunction(Function &F) {
  // Skip if DisableHIRCompleteUnroll is enabled
  if (DisableHIRCompleteUnroll) {
    DEBUG(dbgs() << "HIR LOOP Complete Unroll Transformation Disabled \n");
    return false;
  }

  DEBUG(dbgs() << "Complete unrolling for Function : " << F.getName() << "\n");
  DEBUG(dbgs() << "Trip Count Threshold : " << CurrentTripThreshold << "\n");

  // Do an early exit if Trip Threshold is less than 1
  // TODO: Check if we want give some feedback to user
  if (CurrentTripThreshold == 0) {
    return false;
  }

  // Gather the outermost loops
  HLNodeUtils::gatherOutermostLoops(OuterLoops);

  // Process Loop Complete Unrolling
  processCompleteUnroll();

  return false;
}

/// processCompleteUnroll - Main routine to perform unrolling.
/// First, performs cost analysis and then do the transformation.
void HIRCompleteUnroll::processCompleteUnroll() {

  // Walk over the outermost loops across the regions.
  for (auto Iter = OuterLoops.begin(), E = OuterLoops.end(); Iter != E;
       ++Iter) {
    // Child trip count should be set 1 for innermost loops.
    int64_t TotalTripCnt = 0;
    processLoop(*Iter, &TotalTripCnt);
  }

  transformLoops();
}

bool HIRCompleteUnroll::processLoop(HLLoop *Loop, int64_t *TotalTripCnt) {

  // 1. Gather Loops starting from the outer-most level, put into ChildLoops
  SmallVector<HLLoop *, 8> ChildLoops;
  HLNodeUtils::gatherLoopswithLevel(Loop, ChildLoops,
                                    Loop->getNestingLevel() + 1);

  // 2.Process each Loop for Complete Unrolling
  bool ChildValid = true;
  // Recurse through the children.
  for (auto Iter = ChildLoops.begin(), E = ChildLoops.end(); Iter != E;
       ++Iter) {
    int64_t TripCnt = 0;
    ChildValid &= processLoop(*Iter, &TripCnt);
    (*TotalTripCnt) += TripCnt;
  }

  if (!ChildValid) {
    return false;
  }

  // Add the loop for transformation if profitable.
  LoopData *LD;
  if (isProfitable(Loop, &LD, TotalTripCnt)) {
    TransformLoops.push_back(std::make_pair(Loop, LD));
    return true;
  }

  return false;
}

bool HIRCompleteUnroll::isProfitable(const HLLoop *Loop, LoopData **LData,
                                     int64_t *ChildTripCnt) {

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

  const RegDDRef *UBRef = Loop->getUpperDDRef();
  assert(UBRef && " Loop UpperBound not found.");

  const RegDDRef *LBRef = Loop->getLowerDDRef();
  assert(LBRef && " Loop LowerBound not found.");

  const RegDDRef *StrideRef = Loop->getStrideDDRef();
  assert(StrideRef && " Loop Stride not found.");

  // Check if UB is Constant or not.
  int64_t UBConst;
  if (!UBRef->isIntConstant(&UBConst)) {
    return false;
  }

  // Check if LB is Constant or not.
  int64_t LBConst;
  if (!LBRef->isIntConstant(&LBConst)) {
    return false;
  }

  // Check if StepVal is Constant or not.
  int64_t StepConst;
  if (!StrideRef->isIntConstant(&StepConst)) {
    return false;
  }

  // TripCount is (Upper -Lower)/Stride + 1.
  int64_t ConstTripCount = (int64_t)((UBConst - LBConst) / StepConst) + 1;
  assert(ConstTripCount && " Zero Trip count loop found.");

  int64_t TotalTripCnt =
      Loop->isInnermost() ? ConstTripCount : ConstTripCount * (*ChildTripCnt);

  DEBUG(dbgs() << " Const Trip Count: " << ConstTripCount << "\n");
  if (TotalTripCnt > CurrentTripThreshold) {
    DEBUG(dbgs() << "TotalTripCnt:" << TotalTripCnt << "\n");
    return false;
  }

  // Update the child trip count for outer loops.
  *ChildTripCnt = TotalTripCnt;

  // Ignore loops which have switch or function calls for unrolling.
  if (HLNodeUtils::hasSwitchOrCall(Loop->getFirstChild(), Loop->getLastChild(),
                                   false)) {
    return false;
  }

  // Store loop information for transformation phase.
  *LData = new LoopData(LBConst, UBConst, StepConst);

  return true;
}

// Transform (Complete Unroll) each loop inside the TransformLoops vector
void HIRCompleteUnroll::transformLoops() {

  // Transform the loop nest from innermost to outermost.
  for (auto &I : TransformLoops) {
    transformLoop(I.first, I.second);
  }
}

// Complete Unroll the given Loop, using provided LD as helper data
void HIRCompleteUnroll::transformLoop(HLLoop *&Loop, LoopData *LD) {

  // Guard against the scanning phase setting it appropriately.
  assert(Loop && LD && " Loop info (loop ptr or data) is null.");

  // Container for cloning body.
  HLContainerTy LoopBody;

  // Iterate over Loop Child for unrolling with trip value incremented
  // each time. Thus, loop body will be expanded by no. of stmts x TripCount.
  for (int64_t TripVal = LD->LB; TripVal <= LD->UB; TripVal += LD->Step) {
    // Clone 0th iteration
    HLNodeUtils::cloneSequence(&LoopBody, Loop->getFirstChild(),
                               Loop->getLastChild());

    // Store references as LoopBody will be empty after insertion.
    HLNode *CurFirstChild = &(LoopBody.front());
    HLNode *CurLastChild = &(LoopBody.back());

    HLNodeUtils::insertBefore(Loop, &LoopBody);

    // Update CanonExpr and DD Info
    CanonExprVisitor CEVisit(Loop->getNestingLevel(), TripVal);
    HLNodeUtils::visitRange(CEVisit, CurFirstChild, CurLastChild);
  }

  assert(Loop->getParentRegion() && " Loop does not have a parent region.");
  Loop->getParentRegion()->setGenCode();

  // Invalidate all analysis for the loop.
  HIRInvalidationUtils::invalidateBody(Loop);

  // Delete the original loop.
  HLNodeUtils::erase(Loop);
  Loop = nullptr;
}

void HIRCompleteUnroll::releaseMemory() {

  // Delete all Loopinfo.
  for (auto &I : TransformLoops) {
    delete I.second;
  }
  TransformLoops.clear();
  OuterLoops.clear();
}
