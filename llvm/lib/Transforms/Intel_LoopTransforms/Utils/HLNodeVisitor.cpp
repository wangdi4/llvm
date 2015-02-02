//===------ HLNodeVisitor.cpp - Implements HLNodevisitor class --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements HLNodeVisitor class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeVisitor.h"
#include "llvm/IR/Intel_LoopIR/HLNode.h"
#include "llvm/IR/Intel_LoopIR/HLRegion.h"
#include "llvm/IR/Intel_LoopIR/HLSwitch.h"
#include "llvm/IR/Intel_LoopIR/HLLabel.h"
#include "llvm/IR/Intel_LoopIR/HLGoto.h"
#include "llvm/IR/Intel_LoopIR/HLInst.h"
#include "llvm/IR/Intel_LoopIR/HLIf.h"
#include "llvm/IR/Intel_LoopIR/HLLoop.h"

using namespace llvm;
using namespace loopopt;

template<typename HV>
bool HLNodeVisitor<HV>::visit(HLContainerTy::iterator It, bool Recursive, 
  bool Forward) {

  bool Ret;
  
  if (isa<HLRegion>(It)) {
    HLRegion* Reg = cast<HLRegion>(It);

    Visitor->visitRegion(Reg);

    if (Recursive) {
      Ret = Forward ? forwardVisit(Reg->child_begin(), Reg->child_end(), true) :
            backwardVisit(Reg->child_begin(), Reg->child_end(), true);

      if (Ret) {
        return true;
      }

      Visitor->postVisitRegion(Reg);
    }
  }
  else if (isa<HLSwitch>(It)) {
    Visitor->visitSwitch(cast<HLSwitch>(It));
  }
  else if (isa<HLLabel>(It)) {
    Visitor->visitLabel(cast<HLLabel>(It));
  }
  else if (isa<HLGoto>(It)) {
    Visitor->visitGoto(cast<HLGoto>(It));
  }
  else if (isa<HLInst>(It)) {
    Visitor->visitInstruction(cast<HLInst>(It));
  }
  else if (isa<HLIf>(It)) {
    HLIf* If = cast<HLIf>(It);

    Visitor->visitIf(If);

    if (Recursive) {
      Ret = Forward ? forwardVisit(If->then_begin(), If->then_end(), true) :
            backwardVisit(If->else_begin(), If->else_end(), true);

      if (Ret) {
        return true;
      }

      Ret = Forward ? forwardVisit(If->else_begin(), If->else_end(), true) :
            backwardVisit(If->then_begin(), If->then_end(), true);

      if (Ret) {
        return true;
      }

      Visitor->postVisitIf(If);
    }
  }
  else if (isa<HLLoop>(It)) {
    HLLoop* Loop = cast<HLLoop>(It);

    Visitor->visitLoop(Loop);

    if (Recursive) {
      Ret = Forward ? forwardVisit(Loop->pre_begin(), Loop->pre_end(), true) :
            backwardVisit(Loop->post_begin(), Loop->post_end());

      if (Ret) {
        return true;
      }
      
      Ret = Forward ? forwardVisit(Loop->child_begin(), Loop->child_end(), true)
            : backwardVisit(Loop->child_begin(), Loop->child_end(), true);

      if (Ret) {
        return true;
      }

      Ret = Forward ? forwardVisit(Loop->post_begin(), Loop->post_end(), true) :
            backwardVisit(Loop->pre_begin(), Loop->pre_end(), true);
      
      if (Ret) {
        return true;
      }

      Visitor->postVisitLoop(Loop);
    }
  }
  else {
    llvm_unreachable("Unknown HLNode type!");
  }

  /// Visitor indicated that the traversal is done
  if (Visitor->isDone()) {
    return true;
  }

  return false;
}

template<typename HV>
bool HLNodeVisitor<HV>::forwardVisit(HLContainerTy::iterator Begin, 
  HLContainerTy::iterator End, bool Recursive) {

  for (auto I = Begin, Next = I, E = End; I != E; I = Next) {

    Next++;
    
    if (visit(I, Recursive, true)) {
      return true;
    }
  }

  return false;
}

template<typename HV>
bool HLNodeVisitor<HV>::backwardVisit(HLContainerTy::iterator Begin,
  HLContainerTy::iterator End, bool Recursive) {

  /// Change direction and Iterate backwards
  for (auto RI = End, RNext = RI, RE = Begin; RI != RE; RI = RNext) {

    RNext--;

    if (visit((RI - 1), Recursive, false)) {
      return true;
    }
  }

  return false;

}

template<typename HV>
void HLNodeVisitor<HV>::forwardVisitAll() {
  forwardVisit(HLRegions.begin(), HLRegions.end());
}

template<typename HV>
void HLNodeVisitor<HV>::backwardVisitAll() {
  backwardVisit(HLRegions.begin(), HLRegions.end());
}

