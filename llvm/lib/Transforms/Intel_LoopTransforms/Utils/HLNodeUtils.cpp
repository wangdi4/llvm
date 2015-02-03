//===------- HLNodeUtils.cpp - Implements HLNodeUtils class ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements HLNodeUtils class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
using namespace llvm;
using namespace loopopt;

HLRegion* HLNodeUtils::createHLRegion(std::set< BasicBlock* >& OrigBBs,
  BasicBlock* PredBB, BasicBlock* SuccBB) {

  return new HLRegion(OrigBBs, PredBB, SuccBB);
}

HLSwitch* HLNodeUtils::createHLSwitch(HLNode* Par) {
  return new HLSwitch(Par);
}

HLLabel* HLNodeUtils::createHLLabel(BasicBlock* SrcBB, HLNode* Par) {
  return new HLLabel(Par, SrcBB);
}

HLGoto* HLNodeUtils::createHLGoto(BasicBlock* TargetBB, HLLabel* TargetL, 
  HLNode* Par) {
  
  return new HLGoto(Par, TargetBB, TargetL);
}

HLInst* HLNodeUtils::createHLInst(Instruction* In, HLNode* Par) {
  return new HLInst(Par, In);
}

HLIf* HLNodeUtils::createHLIf(HLNode* Par) {
  return new HLIf(Par);
}

HLLoop* HLNodeUtils::createHLLoop(HLNode* Par, HLIf* ZttIf, bool isDoWh, 
  unsigned NumEx) {

  return new HLLoop(Par, ZttIf, isDoWh, NumEx);
}

void HLNodeUtils::destroy(HLNode* Node) {
  Node->destroy();
}

void HLNodeUtils::destroyAll() {
  HLNode::destroyAll();
}

template<typename HV>
void HLNodeUtils::forwardVisit(HV* Visitor, HLContainerTy::iterator Begin, 
  HLContainerTy::iterator End, bool Recursive) {
  HLNodeVisitor<HV> V(Visitor);
  V.forwardVisit(Begin, End, Recursive);  
}

template<typename HV>
void HLNodeUtils::backwardVisit(HV* Visitor, HLContainerTy::iterator Begin,
    HLContainerTy::iterator End, bool Recursive) {
  HLNodeVisitor<HV> V(Visitor);
  V.backwardVisit(Begin, End, Recursive);
}

template<typename HV>
void HLNodeUtils::forwardVisitAll(HV* Visitor) {
  HLNodeVisitor<HV> V(Visitor);
  V.forwardVisitAll();
}

template<typename HV>
void HLNodeUtils::backwardVisitAll(HV* Visitor) {
  HLNodeVisitor<HV> V(Visitor);
  V.backwardVisitAll();
}

void HLNodeUtils::setSimpleLoopZtt(HLIf *Ztt, BasicBlock *BBlock) {

}

void HLNodeUtils::dbgPushBackChild(HLNode *Parent, HLNode* Child) {
    if(HLRegion *R = dyn_cast<HLRegion>(Parent) ) {
        R->Children.push_back(Child); 
    } else if(HLLoop *L = dyn_cast<HLLoop>(Parent) ) {
        L->Children.push_back(Child); 
    }
   

}
void HLNodeUtils::dbgPushDDRef(HLNode *Node, DDRef *Ref) {
    if(HLLoop *L = dyn_cast<HLLoop>(Node) ) {
        L->DDRefs.push_back(Ref);
    } else if(HLInst *I = dyn_cast<HLInst>(Node) ) {
        I->DDRefs.push_back(Ref);
    }
}
