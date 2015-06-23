//===------------ HLIf.cpp - Implements the HLIf class --------------------===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
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

  RegDDRefs.resize(NumOp, nullptr);
}

HLIf::HLIf(CmpInst::Predicate FirstPred, RegDDRef *Ref1, RegDDRef *Ref2)
    : HLDDNode(HLNode::HLIfVal) {
  assert(((FirstPred == CmpInst::Predicate::FCMP_FALSE) ||
          (FirstPred == CmpInst::Predicate::FCMP_TRUE) || (Ref1 && Ref2)) &&
         "DDRefs cannot be null!");
  assert((!Ref1 || (Ref1->getLLVMType() == Ref2->getLLVMType())) &&
         "Ref1/Ref2 type mismatch!");
  assert((!Ref1 || ((CmpInst::isIntPredicate(FirstPred) &&
                     Ref1->getLLVMType()->isIntegerTy()) ||
                    (CmpInst::isFPPredicate(FirstPred) &&
                     Ref1->getLLVMType()->isFloatingPointTy()))) &&
         "Predicate/DDRef type mismatch!");

  /// TODO: add check for type consistency (integer/float)

  ElseBegin = Children.end();
  Predicates.push_back(FirstPred);

  initialize();

  setPredicateOperandDDRef(Ref1, pred_begin(), true);
  setPredicateOperandDDRef(Ref2, pred_begin(), false);
}

HLIf::HLIf(const HLIf &HLIfObj, GotoContainerTy *GotoList, LabelMapTy *LabelMap)
    : HLDDNode(HLIfObj), Predicates(HLIfObj.Predicates) {
  const RegDDRef *Ref;
  ElseBegin = Children.end();
  initialize();

  /// Clone DDRefs
  auto II = HLIfObj.pred_begin();
  for (auto I = pred_begin(), E = pred_end(); I != E; I++, II++) {
    Ref = getPredicateOperandDDRef(II, true);
    setPredicateOperandDDRef(Ref ? Ref->clone() : nullptr, I, true);
    Ref = getPredicateOperandDDRef(II, false);
    setPredicateOperandDDRef(Ref ? Ref->clone() : nullptr, I, false);
  }

  /// Loop over Then children and Else children
  for (auto ThenIter = HLIfObj.then_begin(), ThenIterEnd = HLIfObj.then_end();
       ThenIter != ThenIterEnd; ++ThenIter) {
    HLNode *NewHLNode = cloneBaseImpl(ThenIter, GotoList, LabelMap);
    HLNodeUtils::insertAsLastChild(this, NewHLNode, true);
  }

  for (auto ElseIter = HLIfObj.else_begin(), ElseIterEnd = HLIfObj.else_end();
       ElseIter != ElseIterEnd; ++ElseIter) {
    HLNode *NewHLNode = cloneBaseImpl(ElseIter, GotoList, LabelMap);
    HLNodeUtils::insertAsLastChild(this, NewHLNode, false);
  }
}

HLIf *HLIf::cloneImpl(GotoContainerTy *GotoList, LabelMapTy *LabelMap) const {
  // Call the Copy Constructor
  HLIf *NewHLIf = new HLIf(*this, GotoList, LabelMap);

  return NewHLIf;
}

HLIf *HLIf::clone() const {

  HLContainerTy NContainer;
  HLNodeUtils::cloneSequence(&NContainer, this);
  HLIf *NewIf = cast<HLIf>(NContainer.remove(NContainer.begin()));
  return NewIf;
}

void HLIf::print(formatted_raw_ostream &OS, unsigned Depth) const {
  const RegDDRef *Ref;
  bool FirstPred = true;

  indent(OS, Depth);

  OS << "if (";

  /// Print predicates
  for (auto I = pred_begin(), E = pred_end(); I != E; I++) {
    if (!FirstPred) {
      OS << " && ";
    }

    Ref = getPredicateOperandDDRef(I, true);
    Ref ? Ref->print(OS) : (void)(OS << Ref);
    OS << " ";

    printPredicate(OS, *I);

    OS << " ";
    Ref = getPredicateOperandDDRef(I, false);
    Ref ? Ref->print(OS) : (void)(OS << Ref);

    FirstPred = false;
  }

  OS << ")\n";

  indent(OS, Depth);
  OS << "{\n";

  /// Print then children
  for (auto I = then_begin(), E = then_end(); I != E; I++) {
    I->print(OS, Depth + 1);
  }

  indent(OS, Depth);
  OS << "}\n";
  indent(OS, Depth);
  OS << "else\n";
  indent(OS, Depth);
  OS << "{\n";

  /// Print else children
  for (auto I = else_begin(), E = else_end(); I != E; I++) {
    I->print(OS, Depth + 1);
  }

  indent(OS, Depth);
  OS << "}\n";
}

unsigned HLIf::getNumOperandsInternal() const {
  return (2 * Predicates.size());
}

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

unsigned HLIf::getPredicateOperandDDRefOffset(pred_iterator PredI,
                                              bool IsLHS) const {
  const_pred_iterator CPredI(PredI);
  return getPredicateOperandDDRefOffset(CPredI, IsLHS);
}

unsigned HLIf::getPredicateOperandDDRefOffset(const_pred_iterator PredI,
                                              bool IsLHS) const {
  return ((2 * (PredI - Predicates.begin())) + (IsLHS ? 0 : 1));
}

void HLIf::addPredicate(CmpInst::Predicate Pred, RegDDRef *Ref1,
                        RegDDRef *Ref2) {
  assert(Ref1 && Ref2 && "DDRef is null!");
  assert((Pred != CmpInst::Predicate::FCMP_FALSE) &&
         (Pred != CmpInst::Predicate::FCMP_TRUE) && "Invalid predicate!");
  assert((Ref1->getLLVMType() == Ref2->getLLVMType()) &&
         "Ref1/Ref2 type mismatch!");
  assert(((CmpInst::isIntPredicate(Pred) &&
           CmpInst::isIntPredicate(Predicates[0])) ||
          (CmpInst::isFPPredicate(Pred) &&
           CmpInst::isFPPredicate(Predicates[0]))) &&
         "Predicate type mismatch!");
  assert(
      ((CmpInst::isIntPredicate(Pred) && Ref1->getLLVMType()->isIntegerTy()) ||
       (CmpInst::isFPPredicate(Pred) &&
        Ref1->getLLVMType()->isFloatingPointTy())) &&
      "Predicate/DDRef type mismatch!");
  unsigned NumOp;

  Predicates.push_back(Pred);

  NumOp = getNumOperandsInternal();
  RegDDRefs.resize(NumOp, nullptr);

  setOperandDDRefImpl(Ref1, NumOp - 2);
  setOperandDDRefImpl(Ref2, NumOp - 1);
}

void HLIf::removePredicate(pred_iterator PredI) {
  assert(!Predicates.empty() && "No conjuntions present!");

  /// Remove DDRefs associated with the predicate.
  removePredicateOperandDDRef(PredI, true);
  removePredicateOperandDDRef(PredI, false);

  /// Erase the DDRef slots.
  RegDDRefs.erase(RegDDRefs.begin() +
                  getPredicateOperandDDRefOffset(PredI, true));
  RegDDRefs.erase(RegDDRefs.begin() +
                  getPredicateOperandDDRefOffset(PredI, true));

  /// Erase the predicate.
  Predicates.erase(PredI);
}

RegDDRef *HLIf::getPredicateOperandDDRef(pred_iterator PredI, bool IsLHS) {
  const_pred_iterator CPredI(PredI);
  return const_cast<RegDDRef *>(getPredicateOperandDDRef(CPredI, IsLHS));
}

const RegDDRef *HLIf::getPredicateOperandDDRef(const_pred_iterator PredI,
                                               bool IsLHS) const {
  return getOperandDDRefImpl(getPredicateOperandDDRefOffset(PredI, IsLHS));
}

void HLIf::setPredicateOperandDDRef(RegDDRef *Ref, pred_iterator PredI,
                                    bool IsLHS) {
  setOperandDDRefImpl(Ref, getPredicateOperandDDRefOffset(PredI, IsLHS));
}

RegDDRef *HLIf::removePredicateOperandDDRef(pred_iterator PredI, bool IsLHS) {
  auto TRef = getPredicateOperandDDRef(PredI, IsLHS);

  if (TRef) {
    setPredicateOperandDDRef(nullptr, PredI, IsLHS);
  }

  return TRef;
}
