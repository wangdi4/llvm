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
void HLNodeVisitor<HV>::forwardVisit(HLContainerTy::iterator Begin, 
  HLContainerTy::iterator End, bool Recursive) {

  for (auto I = Begin, Next = I, E = End; I != E; I = Next) {

    ///Visitor indicated that the traversal is done
    if (Visitor->isDone()) {
      return;
    }

    Next++;
    
    if (isa<HLRegion>(I)) {
      HLRegion* Reg = cast<HLRegion>(I);

      Visitor->visitRegion(Reg);

      if (Recursive) {
        forwardVisit(Reg->child_begin(), Reg->child_end());
      }
    }
    else if (isa<HLSwitch>(I)) {
      Visitor->visitSwitch(cast<HLSwitch>(I));
    }
    else if (isa<HLLabel>(I)) {
      Visitor->visitLabel(cast<HLLabel>(I));
    }
    else if (isa<HLGoto>(I)) {
      Visitor->visitGoto(cast<HLGoto>(I));
    }
    else if (isa<HLInst>(I)) {
      Visitor->visitInstruction(cast<HLInst>(I));
    }
    else if (isa<HLIf>(I)) {
      HLIf* If = cast<HLIf>(I);

      Visitor->visitIf(cast<HLIf>(I));

      if (Recursive) {
        forwardVisit(If->then_begin(), If->then_end());
        forwardVisit(If->else_begin(), If->else_end());
      }
    }
    else if (isa<HLLoop>(I)) {
      HLLoop* Loop = cast<HLLoop>(I);
 
      Visitor->visitLoop(cast<HLLoop>(I));

      if (Recursive) {
        forwardVisit(Loop->pre_begin(), Loop->pre_end());
        forwardVisit(Loop->child_begin(), Loop->child_end());
        forwardVisit(Loop->post_begin(), Loop->post_end());
      }
    }
    else {
      llvm_unreachable("Unknown HLNode type!");
    }
  }
}

template<typename HV>
void HLNodeVisitor<HV>::backwardVisit(HLContainerTy::iterator Begin,
  HLContainerTy::iterator End, bool Recursive) {

  HLContainerTy::reverse_iterator RI(End), RE(Begin);

  for (auto RNext = RI; RI != RE; RI = RNext) {

    if (Visitor->isDone()) {
      return;
    }

    RNext++;

    if (isa<HLRegion>(RI)) {
      HLRegion* Reg = cast<HLRegion>(RI);

      Visitor->visitRegion(Reg);

      if (Recursive) {
        backwardVisit(Reg->child_begin(), Reg->child_end());
      }
    }
    else if (isa<HLSwitch>(RI)) {
      Visitor->visitSwitch(cast<HLSwitch>(RI));
    }
    else if (isa<HLLabel>(RI)) {
      Visitor->visitLabel(cast<HLLabel>(RI));
    }
    else if (isa<HLGoto>(RI)) {
      Visitor->visitGoto(cast<HLGoto>(RI));
    }
    else if (isa<HLInst>(RI)) {
      Visitor->visitInstruction(cast<HLInst>(RI));
    }
    else if (isa<HLIf>(RI)) {
      HLIf* If = cast<HLIf>(RI);

      Visitor->visitIf(cast<HLIf>(RI));

      if (Recursive) {
        backwardVisit(If->else_begin(), If->else_end());
        backwardVisit(If->then_begin(), If->then_end());
      }
    }
    else if (isa<HLLoop>(RI)) {
      HLLoop* Loop = cast<HLLoop>(RI);

      Visitor->visitLoop(cast<HLLoop>(RI));

      if (Recursive) {
        backwardVisit(Loop->post_begin(), Loop->post_end());
        backwardVisit(Loop->child_begin(), Loop->child_end());
        backwardVisit(Loop->pre_begin(), Loop->pre_end());
      }
    }
    else {
      llvm_unreachable("Unknown HLNode type!");
    }
  }
}

template<typename HV>
void HLNodeVisitor<HV>::forwardVisitAll() {
  forwardVisit(HLRegions.begin(), HLRegions.end());
}

template<typename HV>
void HLNodeVisitor<HV>::backwardVisitAll() {
  backwardVisit(HLRegions.begin(), HLRegions.end());
}
