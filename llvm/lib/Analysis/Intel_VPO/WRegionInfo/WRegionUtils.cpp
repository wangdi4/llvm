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
    case DIR_OMP_MASTER:
      W = new WRNMasterNode(EntryBB);
      break;
  }
  if (W)
    W->setLevel(NestingLevel);
  return W;
}

/// \brief Similar to createWRegion, but for HIR vectorizer support
WRegionNode *WRegionUtils::createWRegionHIR(
  StringRef        DirString,
  loopopt::HLNode *EntryHLNode,
  unsigned         NestingLevel
)
{
  WRegionNode *W = nullptr;
  int DirID = VPOUtils::getDirectiveID(DirString);

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
WRegionNode *WRegionUtils::updateWRGraphFromHIR (
  IntrinsicInst   *Call,
  Intrinsic::ID   IntrinId,
  WRContainerTy   *WRGraph,
  WRegionNode     *CurrentWRN,
  loopopt::HLNode *H
)
{
  WRegionNode *W = nullptr;
  StringRef DirOrClauseStr = VPOUtils::getDirectiveMetadataString(Call);

  // If there isn't a pending WRN currently, then only need to check
  // for directives that begin a construct 
  if (!CurrentWRN) {
    if (IntrinId == Intrinsic::intel_directive) {
      // If the intrinsic represents a BEGIN directive for a construct 
      // needed by the vectorizer (eg: DIR.OMP.LOOP.SIMD), then
      // createWRegionHIR creates a WRN for it and returns its pointer.
      // Otherwise, the W returned is a nullptr.
      // Nesting level is 0 because the WRGraph is flat at this point.
      W = WRegionUtils::createWRegionHIR(DirOrClauseStr, H, 0);
      if (W) {
        // Current HIR doesn't have end directives, so we're not supporting 
        // hierarchically nested WRNs yet. Therefore, there is no need to 
        // use a stack as in WRegionCollection::doPreOrderDomTreeVisit().
        // The resulting WRGraph is just a flat list of WRNs currently.
        WRGraph->push_back(W);
      }
    }
  }
  else {//there's a pending WRN currently; process its clauses, if any
    W = CurrentWRN;
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
  return W;
}


/// \brief Visitor class to walk the HIR and build WRNs
/// based on HIR. Main logic is in the visit() member function
/// This visitor class is intended to be instantiated and used
/// only by WRegionUtils::buildWRGraphFromHIR(). 
struct HIRVisitor final : public HLNodeVisitorBase {
  WRContainerTy *WRGraph;
  WRegionNode   *CurrentWRN;
  HIRVisitor() : WRGraph(new WRContainerTy), CurrentWRN(nullptr){}

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
        CurrentWRN = WRegionUtils::updateWRGraphFromHIR(
                              Call, IntrinId, WRGraph, CurrentWRN, Node);
      }
    }
  }
  else if (CurrentWRN) {
    if (HLLoop *L = dyn_cast<HLLoop>(Node)) {
      WRNVecLoopNode *VLN = dyn_cast<WRNVecLoopNode>(CurrentWRN);
      assert(VLN && "Pending HIR-based WRN must be a WRNVecLoopNode");
      VLN->setExitHLNode(Node); // temporary until HIR shows the end directives
      VLN->setHLLoop(L);
      // The WRN is complete; close it by setting W to null
      CurrentWRN=nullptr; // will be a stack pop when nested WRN 
                          // is supported in HIR
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

