//===- HIRCompleteUnroll.cpp - Implements CompleteUnroll class -*- C++ -*-===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===---------------------------------------------------------------------===//
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
//  Visit the Region
//  Extract the innermost loops
//  For each innermost loop
//    Get Trip Count and perform cost analysis
//    If Trip Count > Threshold, ignore this loop
//    For each Loop child
//      If Child is HLInst
//        If ConstDDRef or BlobDDRef, clone and add before loop
//        If RegDDRef, clone, replace IV by trip val and add before loop
//    Delete Loop
//
// Unrolling would increase the register pressure based on the unroll factor.
// Current heuristic just uses trip count to determine if loop needs to be
// unrolled.
//
//===---------------------------------------------------------------------===//

// TODO: Extensions to be added later for general unrolling.
//  (1) Traversal is top down, but needs not to  store just  innermost loops
//  alone but  for  but candidates for complete unroll for outerloop as  in  do
//  i=1,2; do j=1,3, do k=1,2  We want Unroll and rebuild DDG to happen once in
//  these cases  for the sake of compile time
//  (2) Some  rebuildDDG  util need to be invoked before and after unroll for
//  the sake of incremental rebuild
//  (3) Safe reductions chains need to be updated or removed
//  (4) Linear-at-level need to be adjusted
//  (5) Linear-in-innermost  may turn   into non-linear  and was-linear as in
//  this case
//
//  Do  I
//     M =   a(i)
//       Do j=1,2
//          B(j) = M + j    // ( M + j) is linear in innermost   canon = i2 + M
//          (or linear at level eventually)
//       Enddo
//  Enddo
//
//  After complete unroll
//   Do  I
//      M =   a(i)
//      B(1) = M + 1    //  (M + j)    becomes non-linear (was linear)  canon =
//      M + 1
//      B(2) = M + 2    //  (M + j)    becomes non-linear (was linear)  canon =
//      M + 2
//   Enddo

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "llvm/IR/Intel_LoopIR/HLSwitch.h"
#include "llvm/IR/Intel_LoopIR/HLLoop.h"
#include "llvm/IR/Intel_LoopIR/HLRegion.h"
#include "llvm/IR/Intel_LoopIR/HLIf.h"
#include "llvm/IR/Intel_LoopIR/HLInst.h"
#include "llvm/IR/Intel_LoopIR/RegDDRef.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRParser.h"

#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"

#define DEBUG_TYPE "hircompleteunroll"

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<unsigned> CompleteUnrollTripThreshold(
    "completeunroll-trip-threshold", cl::init(8), cl::Hidden,
    cl::desc("Don't unroll if innermost trip count is bigger than this,"
             "threshold."));

namespace {

class HIRCompleteUnroll : public HIRTransformPass {
public:
  static char ID;

  HIRCompleteUnroll(int T = -1) : HIRTransformPass(ID) {
    initializeHIRCompleteUnrollPass(*PassRegistry::getPassRegistry());

    CurrentTripThreshold =
        (T == -1) ? CompleteUnrollTripThreshold : unsigned(T);
  }

  bool runOnFunction(Function &F) override;

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRParser>();
  }

private:
  Function *F;
  unsigned CurrentTripThreshold;
  SmallVector<HLLoop *, 16> InnermostLoops;

  void processCompleteUnroll();
  bool isProfitable(HLLoop *Loop, int64_t *TripCount);
  void transformLoop(HLLoop *Loop, int64_t TripCount);
  void processLoopChild(HLNode *ChildNode, int64_t TripVal, HLLoop *Loop);
  void processRegDDRef(RegDDRef *RegDD, int64_t TripVal, unsigned Level);
  void processCanonExpr(CanonExpr *CExpr, int64_t TripVal, unsigned Level);

  // Visitor methods for gathering innermost loops
  void visit(HLNode *Node);
  void visit(HLRegion *Node);
  void visit(HLLoop *Node);
  void visit(HLIf *Node);
  void visit(HLSwitch *Node);
};
}

char HIRCompleteUnroll::ID = 0;
INITIALIZE_PASS_BEGIN(HIRCompleteUnroll, "HIRCompleteUnroll",
                      "HIR Complete Unroll", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRParser)
INITIALIZE_PASS_END(HIRCompleteUnroll, "HIRCompleteUnroll",
                    "HIR Complete Unroll", false, false)

FunctionPass *llvm::createHIRCompleteUnrollPass(int Threshold) {
  return new HIRCompleteUnroll(Threshold);
}

bool HIRCompleteUnroll::runOnFunction(Function &F) {
  DEBUG(dbgs() << "Complete unrolling for Function : " << F.getName() << "\n");
  DEBUG(dbgs() << "Trip Count Threshold : " << CurrentTripThreshold << "\n");

  // Do an early exit if Trip Threshold is less than 1
  // TODO: Check if we want give some feedback to user
  if (CurrentTripThreshold == 0)
    return false;

  this->F = &F;

  auto HIRP = &getAnalysis<HIRParser>();

  // Gather the innermost loops
  for (auto I = HIRP->hir_begin(), E = HIRP->hir_end(); I != E; I++) {
    visit(I);
  }
  processCompleteUnroll();

  return false;
}

/// visit - general visitor method for HLDDNode
/// to gather the innermost loops.
/// TODO: Add better heuristic to find right candidates
/// for unrolling. For example: Do not unroll if the innermost
/// loop has more than 3 loop levels or 2 nested switch.
/// This may not be possible with general HLNodeUtils.
/// If not, use HLNodeVisitor class
void HIRCompleteUnroll::visit(HLNode *Node) {

  if (isa<HLRegion>(Node)) {
    HLRegion *Region = cast<HLRegion>(Node);
    visit(Region);
  } else if (isa<HLLoop>(Node)) {
    HLLoop *Loop = cast<HLLoop>(Node);
    visit(Loop);
  } else if (isa<HLIf>(Node)) {
    HLIf *IfNode = cast<HLIf>(Node);
    visit(IfNode);
  } else if (isa<HLSwitch>(Node)) {
    HLSwitch *SwitchNode = cast<HLSwitch>(Node);
    visit(SwitchNode);
  } else if (isa<HLInst>(Node) || isa<HLGoto>(Node) || isa<HLLabel>(Node)) {
    return;
  } else {
    llvm_unreachable("Unknown HLNode type!");
  }
}

/// visit - Visitor implementation of HLRegion
/// to collect all the innermost loops.
void HIRCompleteUnroll::visit(HLRegion *Region) {

  assert(Region && " Complete unroll used on null pointer HIRRegion");
  DEBUG(dbgs() << " Num of Children : " << Region->getNumChildren() << "\n");

  assert((Region->getNumChildren() > 0) &&
         " HIR Region should have atleast 1 child ");

  // Visit Region's Children to gather innermost loops
  for (auto Iter = Region->child_begin(), End = Region->child_end();
       Iter != End; Iter++) {
    HLNode *Node = cast<HLNode>(Iter);
    visit(Node);
  }
}

/// visit - visitor implementation for loops to gather
/// the innermost loops
void HIRCompleteUnroll::visit(HLLoop *Loop) {

  // Preheader and Post exit is not hanlded currently
  // TODO: Properly handle these conditions later
  assert(!(Loop->hasPreheader() || Loop->hasPostexit()) &&
         " Loop Preheader and Postexit not handled currently");

  // Update loop vector and return if innermost found
  if (Loop->isInnermost()) {
    InnermostLoops.push_back(Loop);
    return;
  }

  // Iterate children to collect innermost loops from
  // nested HL Objects.
  for (auto ChildIter = Loop->child_begin(), ChildIterEnd = Loop->child_end();
       ChildIter != ChildIterEnd; ++ChildIter) {
    HLNode *Node = cast<HLNode>(ChildIter);
    visit(Node);
  }
}

/// visit - visitor implementation for HLIf to gather
/// the innermost loops
void HIRCompleteUnroll::visit(HLIf *IfNode) {

  /// Loop over Then children and Else children
  for (auto ThenIter = IfNode->then_begin(), ThenIterEnd = IfNode->then_end();
       ThenIter != ThenIterEnd; ++ThenIter) {
    HLNode *Node = cast<HLNode>(ThenIter);
    visit(Node);
  }

  for (auto ElseIter = IfNode->else_begin(), ElseIterEnd = IfNode->else_end();
       ElseIter != ElseIterEnd; ++ElseIter) {
    HLNode *Node = cast<HLNode>(ElseIter);
    visit(Node);
  }
}

/// visit - visitor implementation for switch statement
/// TODO: Complete implementation when HLSwitch is implemented.
void HIRCompleteUnroll::visit(HLSwitch *Switch) {
  llvm_unreachable("HLSwitch not implemented currently.");
}

/// processCompleteUnroll - Main routine to perform unrolling.
/// First, performs cost analysis and then does the transformation.
void HIRCompleteUnroll::processCompleteUnroll() {

  int64_t TripCount = 0;
  // Visit each innermost loop to run cost analysis
  for (auto Iter = InnermostLoops.begin(), End = InnermostLoops.end();
       Iter != End; ++Iter, TripCount = 0) {
    HLLoop *Loop = *Iter;

    // Perform a cost/profitability analysis on the loop
    // If all conditions are met, unroll it
    if (isProfitable(Loop, &TripCount))
      transformLoop(Loop, TripCount);
  }
}

/// isProfitable - Check if the loop trip count is less
/// than the trip count threshold. Return true, if this loop
/// should be completely unrolled.
bool HIRCompleteUnroll::isProfitable(HLLoop *Loop, int64_t *TripCount) {

  // Loop should be normalized before this pass

  // If loop doesn't have children, don't process it
  // TODO: Check if we want to delete such loops, ideally
  // this should be taken care by earlier passes.
  if (Loop->getNumChildren() == 0)
    return false;

  // Check if Loop Trip Count is constant value
  // If not, delete this candidate loop and proceed to next
  // TODO: General unrolling will be extended later
  RegDDRef *UBRef = Loop->getUpperDDRef();
  if (!UBRef)
    return false;

  // Check if UB is Constant or not
  int64_t UBConst;
  if (!UBRef->isIntConstant(&UBConst))
    return false;

  // TripCount is (Upper + 1).
  int64_t ConstTripCount = UBConst + 1;

  DEBUG(dbgs() << " Const Trip Count: " << ConstTripCount << "\n");
  if (ConstTripCount > CurrentTripThreshold)
    return false;

  // Set the final unroll factor for this loop
  *TripCount = ConstTripCount;

  return true;
}

/// transformLoop - Perform the unrolling transformation for
/// the given loop. TripCount is the unrolling factor.
/// For now, we are performing complete unrolling.
void HIRCompleteUnroll::transformLoop(HLLoop *Loop, int64_t TripCount) {

  // Iterate over Loop Child for unrolling with trip value incremented
  // each time. Thus, loop body will be expanded by No. of stmts x TripCount.
  // TODO: Multi-level loop nest to be handled later
  for (int64_t TripVal = 0; TripVal < TripCount; TripVal++) {
    /// Loop through the child
    for (auto Iter = Loop->child_begin(), End = Loop->child_end(); Iter != End;
         Iter++) {
      processLoopChild(Iter, TripVal, Loop);
    }
  }

  // TODO: Handle innermost flag for multi-level loop nest
  DEBUG(dbgs() << " Delete Loop \n");

  Loop->getParentRegion()->setGenCode();

  HLNodeUtils::erase(Loop);

  return;
}

/// processLoopChild - Process Loop Child to replace IV by Trip Value.
void HIRCompleteUnroll::processLoopChild(HLNode *ChildNode, int64_t TripVal,
                                         HLLoop *Loop) {

  // DEBUG(dbgs() << " Processing Loop Child \n");

  // TODO: Currently only handling Inst, extend later for others like HLIf
  HLInst *Inst = dyn_cast<HLInst>(ChildNode);
  assert(Inst && " HLInst for HLLoop is null");

  // Clone the Instruction and work with it
  HLInst *ClonedInst = Inst->clone();

  for (auto Iter = ClonedInst->ddref_begin(), End = ClonedInst->ddref_end();
       Iter != End; Iter++) {

    assert(!isa<BlobDDRef>(*Iter) && "BlobDDRef should not be present here");

    // Process RegDDRef
    RegDDRef *RegDD = dyn_cast<RegDDRef>(*Iter);
    assert(RegDD && " RegDD for ClonedInst is null");
    processRegDDRef(RegDD, TripVal, Loop->getNestingLevel());
  }

  // Insert this instruction before the loop
  HLNodeUtils::insertBefore(Loop, ClonedInst);
}

/// processRegDDRef - Processes RegDDRef to replace IV by TripVal.
/// This is an internal helper function.
void HIRCompleteUnroll::processRegDDRef(RegDDRef *RegDD, int64_t TripVal,
                                        unsigned Level) {

  // DEBUG(dbgs() << " Processing RegDDRef \n");

  /// Process CanonExprs inside the RegDDRefs
  for (auto Iter = RegDD->canon_begin(), End = RegDD->canon_end(); Iter != End;
       Iter++) {
    processCanonExpr(*Iter, TripVal, Level);
  }

  // Process Blob DDRef inside RegDDRef
  // TODO: Remove this later as BlobDDRef should not have IV
  // TODO: Remove BlobDDRef for blobs which get cleared out from CanonExprs.
  for (auto Iter = RegDD->blob_cbegin(), End = RegDD->blob_cend(); Iter != End;
       Iter++) {
    // DEBUG(dbgs() << "   Processing BlobDDRefs inside RegDDRefs \n");
    assert(!(*Iter)->getCanonExpr()->hasIV() && "Blob DDRef contains IV!");
  }

  if (RegDD->hasGEPInfo()) {
    // TODO: Remove this, whenever CanonExpr will contain this
    // information and will not need to handle GEP explicitly
    // Process GEP Base
    processCanonExpr(RegDD->getBaseCE(), TripVal, Level);

    // Process GEP Strides
    for (auto Iter = RegDD->stride_begin(), End = RegDD->stride_end();
         Iter != End; Iter++) {
      // DEBUG(dbgs() << "   Processing GEP Strides inside RegDDRefs \n");
      processCanonExpr((*Iter), TripVal, Level);
    }
  }
}

/// processRegDDRef - Processes CanonExpr to replace IV by TripVal.
/// This is an internal helper function.
void HIRCompleteUnroll::processCanonExpr(CanonExpr *CExpr, int64_t TripVal,
                                         unsigned Level) {
  bool IsBlobCoeff;

  if (CExpr->getIVCoeff(Level, &IsBlobCoeff)) {
    DEBUG(dbgs() << "Replacing CanonExpr IV by tripval :" << TripVal << " \n");
    CExpr->replaceIVByConstant(Level, TripVal);
  }
}
