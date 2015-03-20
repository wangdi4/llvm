//===------------ HLIf.cpp - Implements the HLIf class --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HLIf class.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Intel_LoopIR/HLIf.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

void HLIf::initialize() {
  unsigned NumOp;

  /// This call is to get around calling virtual functions in the constructor.
  NumOp = getNumOperandsInternal();

  DDRefs.resize(NumOp, nullptr);
}

HLIf::HLIf(CmpInst::Predicate FirstPred, DDRef *Ref1, DDRef *Ref2)
    : HLDDNode(HLNode::HLIfVal) {
  assert(((FirstPred == CmpInst::Predicate::FCMP_FALSE) ||
          (FirstPred == CmpInst::Predicate::FCMP_TRUE) || (Ref1 && Ref2)) &&
         "DDRefs cannot be null!");
  /// TODO: add check for type consistency (integer/float)

  ElseBegin = Children.end();
  Preds.push_back(FirstPred);

  initialize();

  setOperandDDRef(Ref1, 0);
  setOperandDDRef(Ref2, 1);
}

HLIf::HLIf(const HLIf &HLIfObj)
    : HLDDNode(HLIfObj), Preds(HLIfObj.Preds),
      Conjunctions(HLIfObj.Conjunctions) {
  unsigned Count = 0;

  ElseBegin = Children.end();
  initialize();

  /// Clone DDRefs
  for (auto I = HLIfObj.ddref_begin(), E = HLIfObj.ddref_end(); I != E;
       I++, Count++) {
    setOperandDDRef((*I)->clone(), Count);
  }

  /// Loop over Then children and Else children
  for (auto ThenIter = HLIfObj.then_begin(), ThenIterEnd = HLIfObj.then_end();
       ThenIter != ThenIterEnd; ++ThenIter) {
    HLNode *NewHLNode = ThenIter->clone();
    HLNodeUtils::insertAsLastIfChild(this, NewHLNode);
  }

  for (auto ElseIter = HLIfObj.else_begin(), ElseIterEnd = HLIfObj.else_end();
       ElseIter != ElseIterEnd; ++ElseIter) {
    HLNode *NewHLNode = ElseIter->clone();
    HLNodeUtils::insertAsLastIfChild(this, NewHLNode, false);
  }
}

HLIf *HLIf::clone() const {

  /// Call the Copy Constructor
  HLIf *NewHLIf = new HLIf(*this);

  return NewHLIf;
}

unsigned HLIf::getNumOperandsInternal() const { return (2 * Preds.size()); }

unsigned HLIf::getNumOperands() const { return getNumOperandsInternal(); }

HLNode *HLIf::getFirstThenChild() {
  if (hasThenChildren()) {
    return then_begin();
  }

  return nullptr;
}

HLNode *HLIf::getLastThenChild() {
  if (hasThenChildren()) {
    return std::prev(then_end());
  }

  return nullptr;
}

HLNode *HLIf::getFirstElseChild() {
  if (hasElseChildren()) {
    return else_begin();
  }

  return nullptr;
}

HLNode *HLIf::getLastElseChild() {
  if (hasElseChildren()) {
    return std::prev(else_end());
  }

  return nullptr;
}
