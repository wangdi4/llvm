//===----- WRegionNodeUtils.cpp - W-Region Node Utils class -----*- C++ -*-===//
//
//   Copyright (C) 2015 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
//   This file implements the the WRegionNode Utils class.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Analysis/Intel_VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionCollection.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionUtils.h"

#define DEBUG_TYPE "WRegionUtils"

using namespace llvm;
using namespace loopopt;
using namespace vpo;

/// \brief Create a specialized WRN based on the DirString.
/// If the string corrensponds to a BEGIN directive, then create
/// a WRN node of WRegionNodeKind corresponding to the directive,
/// and return a pointer to it. Otherwise; return nullptr.
///
/// When dealing with the llvm.directive.region.entry representation
/// (IsRegionIntrinsic==true) we call W->handleOperandBundles() to extract
/// the clause info from the OperandBundles and update WRN accordingly.
WRegionNode *WRegionUtils::createWRegion(int DirID, BasicBlock *EntryBB,
                                         LoopInfo *LI, unsigned NestingLevel,
                                         bool IsRegionIntrinsic) {
  WRegionNode *W = nullptr;

  switch(DirID) {
    case DIR_OMP_PARALLEL:
      W = new WRNParallelNode(EntryBB);
      break;
    case DIR_OMP_PARALLEL_LOOP:
      W = new WRNParallelLoopNode(EntryBB, LI);
      break;
    case DIR_OMP_PARALLEL_SECTIONS:
      W = new WRNParallelSectionsNode(EntryBB, LI);
      break;
    case DIR_OMP_PARALLEL_WORKSHARE:   // Fortran only
      W = new WRNParallelWorkshareNode(EntryBB, LI);
      break;
    case DIR_OMP_TEAMS:
      W = new WRNTeamsNode(EntryBB);
      break;
    case DIR_OMP_DISTRIBUTE_PARLOOP:
      W = new WRNDistributeParLoopNode(EntryBB, LI);
      break;
    case DIR_OMP_TARGET:
      W = new WRNTargetNode(EntryBB);
      break;
    case DIR_OMP_TARGET_DATA:
    case DIR_OMP_TARGET_ENTER_DATA:
    case DIR_OMP_TARGET_EXIT_DATA:
    case DIR_OMP_TARGET_UPDATE:
      W = new WRNTargetDataNode(EntryBB);
      break;
    case DIR_OMP_TASK:
      W = new WRNTaskNode(EntryBB);
      break;
    case DIR_OMP_TASKLOOP:
      W = new WRNTaskloopNode(EntryBB, LI);
      break;
    case DIR_OMP_SIMD:
      W = new WRNVecLoopNode(EntryBB, LI);
      break;
    case DIR_OMP_LOOP:
      W = new WRNWksLoopNode(EntryBB, LI);
      break;
    case DIR_OMP_SECTIONS:
      W = new WRNSectionsNode(EntryBB, LI);
      break;
    case DIR_OMP_WORKSHARE:   // Fortran only
      W = new WRNWorkshareNode(EntryBB, LI);
      break;
    case DIR_OMP_DISTRIBUTE:
      W = new WRNDistributeNode(EntryBB, LI);
      break;
    case DIR_OMP_ATOMIC:
      W = new WRNAtomicNode(EntryBB);
      break;
    case DIR_OMP_BARRIER:
      W = new WRNBarrierNode(EntryBB);
      break;
    case DIR_OMP_CANCEL:
      W = new WRNCancelNode(EntryBB, false);
      break;
    case DIR_OMP_CANCELLATION_POINT:
      W = new WRNCancelNode(EntryBB, true);
      break;
    case DIR_OMP_MASTER:
      W = new WRNMasterNode(EntryBB);
      break;
    case DIR_OMP_ORDERED:
      W = new WRNOrderedNode(EntryBB);
      break;
    case DIR_OMP_SINGLE:
      W = new WRNSingleNode(EntryBB);
      break;
    case DIR_OMP_CRITICAL:
      W = new WRNCriticalNode(EntryBB);
      break;
    case DIR_OMP_FLUSH:
      W = new WRNFlushNode(EntryBB);
      break;
    case DIR_OMP_TASKGROUP:
      W = new WRNTaskgroupNode(EntryBB);
      break;
    case DIR_OMP_TASKWAIT:
      W = new WRNTaskwaitNode(EntryBB);
      break;
    case DIR_OMP_TASKYIELD:
      W = new WRNTaskyieldNode(EntryBB);
      break;
    case DIR_OMP_THREADPRIVATE:
      // #pragma omp threadprivate can be a module-level directive so we
      // handle it outside of the WRN framework 
      break;
  }
  if (W) {
    W->setLevel(NestingLevel);
    W->setDirID(DirID);
    if (IsRegionIntrinsic) {
      W->getClausesFromOperandBundles();
    }
  }
  return W;
}

/// \brief Similar to createWRegion, but for HIR vectorizer support
WRegionNode *WRegionUtils::createWRegionHIR(
  int              DirID,
  loopopt::HLNode *EntryHLNode,
  unsigned         NestingLevel
)
{
  WRegionNode *W = nullptr;

  switch(DirID) {
    // TODO: complete the list for all WRegionNodeKinds needed
    //       to support vectorizer
    case DIR_OMP_SIMD:
      W = new WRNVecLoopNode(EntryHLNode);
      break;
  }
  if (W) {
    W->setLevel(NestingLevel);
    W->setDirID(DirID);
  }
  return W;
}

/// \brief Update WRGraph from processing HIR representation
void WRegionUtils::updateWRGraphFromHIR (
  IntrinsicInst   *Call,
  Intrinsic::ID   IntrinId,
  WRContainerImpl *WRGraph,
  WRStack<WRegionNode*> &S, 
  loopopt::HLNode *H
)
{
  WRegionNode *W = nullptr;
  StringRef DirOrClauseStr = VPOAnalysisUtils::getDirectiveMetadataString(Call);
  if (IntrinId == Intrinsic::intel_directive) {
    int DirID = VPOAnalysisUtils::getDirectiveID(DirOrClauseStr);
    // If the intrinsic represents a BEGIN directive for a construct 
    // needed by the vectorizer (eg: DIR.OMP.SIMD), then
    // createWRegionHIR creates a WRN for it and returns its pointer.
    // Otherwise, the W returned is a nullptr.
    W = WRegionUtils::createWRegionHIR(DirID, H, S.size());
    if (W) {
      // DEBUG(dbgs() << "\n Starting New WRegion from HIR{\n");
      if (S.empty()) {
        WRGraph->push_back(W);
      }
      else {
        WRegionNode *Parent = S.top();
        Parent->getChildren().push_back(W);
        W->setParent(Parent);
      }
      S.push(W);
    }
    else if (VPOAnalysisUtils::isEndDirective(DirID)) {
      // DEBUG(dbgs() << "\n} Ending WRegion.\n");
      if (!S.empty()) {
        W = S.top();
        if(WRNVecLoopNode *V = dyn_cast<WRNVecLoopNode>(W)) {
          V->setExitHLNode(H);
          S.pop();
        }
        else {
          llvm_unreachable("VPO: Expected a WRNVecLoopNode");
        }
      }
    }
  } else if (VPOAnalysisUtils::isIntelClause(IntrinId)) {
    //process clauses
    W = S.top();

    // Extract clause properties
    ClauseSpecifier ClauseInfo(DirOrClauseStr);

    // Parse the clause and update W
    W->parseClause(ClauseInfo, Call);
  }
}


/// \brief Visitor class to walk the HIR and build WRNs
/// based on HIR. Main logic is in the visit() member function
/// This visitor class is intended to be instantiated and used
/// only by WRegionUtils::buildWRGraphFromHIR(). 
struct HIRVisitor final : public HLNodeVisitorBase {
  WRContainerImpl *WRGraph;
  WRStack<WRegionNode*> S; 
  HIRVisitor() : WRGraph(new WRContainerTy){}

  WRContainerImpl *getWRGraph() const { return WRGraph; }
  void visit(loopopt::HLNode *Node);
  void postVisit(loopopt::HLNode *) {}
};

void HIRVisitor::visit(loopopt::HLNode *Node) {
  // Node->dump();
  if (HLInst *HI = dyn_cast<HLInst>(Node)) {
    const Instruction *I = HI->getLLVMInstruction();
    Instruction *II = const_cast<Instruction *> (I);
    IntrinsicInst* Call = dyn_cast<IntrinsicInst>(II);
    if (Call) {
      Intrinsic::ID IntrinId = Call->getIntrinsicID();
      if (VPOAnalysisUtils::isIntelDirectiveOrClause(IntrinId)) {
        // The intrinsic is one of these: intel_directive,
        //                                intel_directive_qual, 
        //                                intel_directive_qual_opnd, 
        //                                intel_directive_qual_opndlist 
        // Process them and create or update WRN accordingly
        WRegionUtils::updateWRGraphFromHIR(
                              Call, IntrinId, WRGraph, S, Node);
      }
    }
  }
  else if (HLLoop *L = dyn_cast<HLLoop>(Node)) {
    // Found a loop L; check if there's a pending WRNVecLoopNode
    // that still has empty getHLLoop() and needs updating
    if (!S.empty()) {
      WRegionNode *W = S.top();
      WRNVecLoopNode *VLN = dyn_cast<WRNVecLoopNode>(W);
      if (VLN && !(VLN->getHLLoop())) {
        VLN->setHLLoop(L);
      }
    }
  } 
}

WRContainerImpl *WRegionUtils::buildWRGraphFromHIR(HIRFramework &HIRF)
{
  HIRVisitor Visitor;

  HIRF.getHLNodeUtils().visitAll(Visitor);
  return Visitor.getWRGraph();
}

// Clause Utilities
int WRegionUtils::getClauseIdFromAtomicKind(WRNAtomicKind Kind) {
  switch (Kind) {
  case WRNAtomicUpdate:
    return QUAL_OMP_UPDATE;
  case WRNAtomicRead:
    return QUAL_OMP_READ;
  case WRNAtomicWrite:
    return QUAL_OMP_WRITE;
  case WRNAtomicCapture:
    return QUAL_OMP_CAPTURE;
  default:
    llvm_unreachable("Unsupported Atomic Kind");
  }
}

// gets the induction variable of the OMP loop.
PHINode *WRegionUtils::getOmpCanonicalInductionVariable(Loop* L) {
  assert(L && "getOmpCanonicalInductionVariable: null loop");
  BasicBlock *H = L->getHeader();
  assert(L && "getOmpCanonicalInductionVariable: null loop header");

  BasicBlock *Incoming = nullptr, *Backedge = nullptr;
  pred_iterator PI = pred_begin(H);
  // TBD: messages will become warnings or opt report messages.
  assert(PI != pred_end(H) &&
         "Omp loop must have at least one backedge!");
  Backedge = *PI++;
  if (PI == pred_end(H))
    llvm_unreachable("Omp loop is dead loop");
  Incoming = *PI++;
  if (PI != pred_end(H)) 
    llvm_unreachable("Omp loop has multiple backedges");

  if (L->contains(Incoming)) {
    if (L->contains(Backedge))
      llvm_unreachable("Omp loop cannot have both incoming and backedge BB!");
    std::swap(Incoming, Backedge);
  } else if (!L->contains(Backedge))
    llvm_unreachable("Omp loop cannot have neither incoming nor backedge BB");

  for (BasicBlock::iterator I = H->begin(); isa<PHINode>(I); ++I) {
    PHINode *PN = cast<PHINode>(I);
    if (Instruction *Inc =
        dyn_cast<Instruction>(PN->getIncomingValueForBlock(Backedge)))
      if ((Inc->getOpcode() == Instruction::Add ||
           Inc->getOpcode() == Instruction::Sub) &&
          (Inc->getOperand(0) == PN || Inc->getOperand(1) == PN))
        return PN;
  }
  llvm_unreachable("Omp loop must have induction variable!");

}

// gets the loop lower bound of the OMP loop.
Value *WRegionUtils::getOmpLoopLowerBound(Loop *L) {
  PHINode *PN = getOmpCanonicalInductionVariable(L);
  assert(PN != nullptr && "Omp loop must have induction variable!");
  assert(L->getLoopPreheader() && "Omp loop must have preheader!");

  return PN->getIncomingValueForBlock(L->getLoopPreheader());
}

// gets the loop stride of the OMP loop.
Value *WRegionUtils::getOmpLoopStride(Loop *L, bool &IsNeg) {
  PHINode *PN = getOmpCanonicalInductionVariable(L);
  assert(PN != nullptr && "Omp loop must have induction variable!");

  if (Instruction *Inc = 
      dyn_cast<Instruction>(PN->getIncomingValueForBlock(L->getLoopLatch())))
    if ((Inc->getOpcode() == Instruction::Add ||
         Inc->getOpcode() == Instruction::Sub) &&
        (Inc->getOperand(0) == PN || Inc->getOperand(1) == PN)) {
      if (Inc->getOpcode() == Instruction::Sub)
        IsNeg = true;
      else
        IsNeg = false;
      if (Inc->getOperand(0) == PN)
        return Inc->getOperand(1);
      else
        return Inc->getOperand(0);
    }
  llvm_unreachable("Omp loop must have stride!");
}

void WRegionUtils::getLoopIndexPosInPredicate(Value *LoopIndex,
                                              Instruction *CondInst,
                                              bool& IsLeft) {
  Value *Operand = CondInst->getOperand(0);
  if (isa<SExtInst>(Operand) || isa<ZExtInst>(Operand)) 
    Operand = cast<Instruction>(Operand)->getOperand(0);

  if (Operand == LoopIndex) {
      IsLeft = true;
      return;
  }

  Operand = CondInst->getOperand(1);
  if (isa<SExtInst>(Operand) || isa<ZExtInst>(Operand)) 
    Operand = cast<Instruction>(Operand)->getOperand(1);

  if (Operand == LoopIndex) {
      IsLeft = false;
      return;
  }
  llvm_unreachable("Omp loop bottom test must have loop index!");
}

// gets the loop upper bound of the OMP loop.
Value *WRegionUtils::getOmpLoopUpperBound(Loop *L) {
  ICmpInst *CondInst;
  Value *Res;

  CondInst = getOmpLoopBottomTest(L);
  PHINode *PN = getOmpCanonicalInductionVariable(L);
  Instruction *Inc = 
    dyn_cast<Instruction>(PN->getIncomingValueForBlock(L->getLoopLatch()));
  bool IsLeft;
  getLoopIndexPosInPredicate(Inc, CondInst, IsLeft);
  if (IsLeft) 
    Res = CondInst->getOperand(1);
  else
    Res = CondInst->getOperand(0);
  
  return Res;
}

// gets the zero trip test of the OMP loop if the zero trip
// test exists.
ICmpInst *WRegionUtils::getOmpLoopZeroTripTest(Loop *L) {

  BasicBlock *PB = L->getLoopPreheader();
  assert(std::distance(pred_begin(PB), pred_end(PB))==1);
  do {
    PB = *(pred_begin(PB));
    if (std::distance(succ_begin(PB), succ_end(PB))==2)
      break;
  }while (PB);
  assert(PB && "Expect to see zero trip test block.");
  for (BasicBlock::reverse_iterator J = PB->rbegin();
       J != PB->rend(); ++J) {
    ICmpInst *CondInst = dyn_cast<ICmpInst>(&*J);
    if (CondInst && ICmpInst::isRelational(CondInst->getPredicate())) {
      bool IsLeft;
      getLoopIndexPosInPredicate(getOmpLoopLowerBound(L), CondInst, IsLeft);
      return CondInst;
    }
  }
  llvm_unreachable("Omp loop with non-const \
    upper bound must have zero trip test!");
  
}

// gets the bottom test of the OMP loop.
ICmpInst *WRegionUtils::getOmpLoopBottomTest(Loop *L) {
  PHINode *PN = getOmpCanonicalInductionVariable(L);
  assert(PN != nullptr && "Omp loop must have induction variable!");
  assert(L->isLoopExiting(L->getLoopLatch()) &&
         "Omp loop must have been rotated!");

  BranchInst *ExitBrInst;
  ExitBrInst = dyn_cast<BranchInst>(&*L->getLoopLatch()->rbegin());
  ICmpInst *CondInst = dyn_cast<ICmpInst>(ExitBrInst->getCondition());
  if (CondInst && ICmpInst::isRelational(CondInst->getPredicate()))
    return CondInst;
  
  llvm_unreachable("Omp loop must have bottom test!");
}

// gets the exit block of the OMP loop. The OMP loop may contain exit
// call. The existing LoopInfo returns two exit blocks. The utility
// is to handle this situation.
BasicBlock *WRegionUtils::getOmpExitBlock(Loop* L) {
  BranchInst *ExitBrInst;

  ExitBrInst = dyn_cast<BranchInst>(&*L->getLoopLatch()->rbegin());
  for (unsigned I = 0; I < ExitBrInst->getNumSuccessors(); I++) {
    if (ExitBrInst->getSuccessor(I) != L->getHeader()) 
      return ExitBrInst->getSuccessor(I);
  }
  llvm_unreachable("Omp loop must have one exit block");
}

// gets the predicate for the bottom test.
CmpInst::Predicate WRegionUtils::getOmpPredicate(Loop* L, bool& IsLeft) {
  BranchInst *ExitBrInst;
  ExitBrInst = dyn_cast<BranchInst>(&*L->getLoopLatch()->rbegin());
  ICmpInst *CondInst = dyn_cast<ICmpInst>(ExitBrInst->getCondition());
  assert(CondInst && "Omp loop must have cmp instruction at the end!");
  PHINode *PN = getOmpCanonicalInductionVariable(L);
  Instruction *Inc = 
    dyn_cast<Instruction>(PN->getIncomingValueForBlock(L->getLoopLatch()));

  getLoopIndexPosInPredicate(Inc, CondInst, IsLeft);

  return CondInst->getPredicate();
}
