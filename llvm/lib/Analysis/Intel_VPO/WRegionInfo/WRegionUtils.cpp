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
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionCollection.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionUtils.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"

#define DEBUG_TYPE "WRegionUtils"

using namespace llvm;
using namespace loopopt;
using namespace vpo;

/// \brief Create a specialized WRN based on the DirString.
/// If the string corrensponds to a BEGIN directive, then create
/// a WRN node of WRegionNodeKind corresponding to the directive,
/// and return a pointer to it. Otherwise; return nullptr.
WRegionNode *WRegionUtils::createWRegion(
  StringRef  DirString,
  BasicBlock *EntryBB,
  LoopInfo   *LI,
  unsigned   NestingLevel
)
{
  WRegionNode *W = nullptr;
  int DirID = VPOUtils::getDirectiveID(DirString);

  switch(DirID) {
    // TODO: complete the list for all WRegionNodeKinds
    case DIR_OMP_PARALLEL:
      W = new WRNParallelNode(EntryBB);
      break;
    case DIR_OMP_PARALLEL_LOOP:
      W = new WRNParallelLoopNode(EntryBB, LI);
      break;
    case DIR_OMP_SIMD:
      W = new WRNVecLoopNode(EntryBB, LI);
      break;
    case DIR_OMP_ATOMIC:
      W = new WRNAtomicNode(EntryBB);
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
  }
  if (W)
    W->setLevel(NestingLevel);
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
  if (W)
    W->setLevel(NestingLevel);
  return W;
}

/// \brief Update WRGraph from processing HIR representation
void WRegionUtils::updateWRGraphFromHIR (
  IntrinsicInst   *Call,
  Intrinsic::ID   IntrinId,
  WRContainerTy   *WRGraph,
  WRStack<WRegionNode*> &S, 
  loopopt::HLNode *H
)
{
  WRegionNode *W = nullptr;
  StringRef DirOrClauseStr = VPOUtils::getDirectiveMetadataString(Call);

  if (IntrinId == Intrinsic::intel_directive) {
    int DirID = VPOUtils::getDirectiveID(DirOrClauseStr);
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
        if (!Parent->hasChildren()) {
          WRContainerTy::iterator WI(W);
          WRegionUtils::insertFirstChild(Parent, WI);
        } else {
          WRegionNode *C = Parent->getLastChild();
          WRContainerTy::iterator WI(C);
          WRegionUtils::insertAfter(WI, W);
        }
      }
      S.push(W);
    }
    else if (VPOUtils::isEndDirective(DirID)) {
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
  } else { //process clauses
    W = S.top();
    int ClauseID = VPOUtils::getClauseID(DirOrClauseStr);
    if (IntrinId == Intrinsic::intel_directive_qual) {
      // Handle clause with no arguments
      assert (Call->getNumArgOperands()==1 && 
              "Bad number of opnds for intel_directive_qual");
      W->handleQual(ClauseID);
    }
    else if (IntrinId == Intrinsic::intel_directive_qual_opnd) {
      // Handle clause with one argument
      assert (Call->getNumArgOperands()==2 && 
              "Bad number of opnds for intel_directive_qual_opnd");
      Value *V = Call->getArgOperand(1);
      W->handleQualOpnd(ClauseID, V);
    }
    else if (IntrinId == Intrinsic::intel_directive_qual_opndlist) {
      // Handle clause with argument list
      assert (Call->getNumArgOperands()>=2 && 
              "Bad number of opnds for intel_directive_qual_opndlist");
      W->handleQualOpndList(ClauseID, Call);
    }
  }
}


/// \brief Visitor class to walk the HIR and build WRNs
/// based on HIR. Main logic is in the visit() member function
/// This visitor class is intended to be instantiated and used
/// only by WRegionUtils::buildWRGraphFromHIR(). 
struct HIRVisitor final : public HLNodeVisitorBase {
  WRContainerTy *WRGraph;
  WRStack<WRegionNode*> S; 
  HIRVisitor() : WRGraph(new WRContainerTy){}

  WRContainerTy *getWRGraph() const { return WRGraph; }
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
      if (VPOUtils::isIntelDirectiveOrClause(IntrinId)) {
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

WRContainerTy *WRegionUtils::buildWRGraphFromHIR()
{
  HIRVisitor Visitor;

  HLNodeUtils::visitAll(Visitor);
  return Visitor.getWRGraph();
}


// Insertion Utilities
void WRegionUtils::insertFirstChild(
  WRegionNode *Parent, 
  WrnIter wrn
) 
{
  insertWRegionNode(Parent, WrnIter(nullptr), wrn, WRegionUtils::FirstChild);
  return;
}

void WRegionUtils::insertLastChild(
  WRegionNode *Parent, 
  WrnIter wrn
) 
{
  insertWRegionNode(Parent, WrnIter(nullptr), wrn, WRegionUtils::LastChild);
  return;
}

void WRegionUtils::insertAfter(
  WrnIter  pos, 
  WRegionNode *wrn
) 
{
  assert(&*pos && "Insert Position is Null");
  insertWRegionNode(pos->getParent(), pos, WrnIter(wrn), WRegionUtils::Append);
}

void WRegionUtils::insertBefore(
  WrnIter  pos, 
  WRegionNode *wrn
) 
{
  assert(&*pos && "Insert Position is Null");
  insertWRegionNode(pos->getParent(), pos, WrnIter(wrn), WRegionUtils::Prepend);
}

void WRegionUtils::insertWRegionNode(
  WRegionNode  *Parent, 
  WrnIter  Pos, 
  WrnIter  W, 
  OpType   Op
) 
{
  assert(Parent && "Parent is Null");

  WRContainerTy &WRContainer = Parent->getChildren();

  WrnIter InsertionPoint;

  switch (Op) {
      case WRegionUtils::FirstChild:
        WRContainer.insertAfter(WrnIter(nullptr), &*W);
        break;
      case WRegionUtils::LastChild:
        WRContainer.insertAfter(WrnIter(Parent->getLastChild()), &*W);
        break;
      case WRegionUtils::Append:
        WRContainer.insertAfter(Pos, &*W);
        break;
      case WRegionUtils::Prepend:
        WRContainer.insert(Pos, &*W);
        break;
      default:
        llvm_unreachable("VPO: Unknown WRegionNode Insertion Operation Type");
  }

  W->setParent(Parent);

  return;
}

//Removal Utilities
bool WRegionUtils::stripDirectives(WRegionNode *WRN) {
  bool success = true;
  BasicBlock *EntryBB = WRN->getEntryBBlock();
  BasicBlock *ExitBB = WRN->getExitBBlock();

  success = success && VPOUtils::stripDirectives(*EntryBB);
  success = success && VPOUtils::stripDirectives(*ExitBB);

  return success;
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
