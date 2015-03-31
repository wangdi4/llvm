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

HLIf::HLIf(const HLIf &HLIfObj)
    : HLDDNode(HLIfObj), Predicates(HLIfObj.Predicates),
      Conjunctions(HLIfObj.Conjunctions) {
  const DDRef* Ref;
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

void HLIf::print(formatted_raw_ostream &OS, unsigned Depth) const {
  const DDRef *Ref;

  indent(OS, Depth);

  OS << "if (";

  /// Print predicates
  for (auto I = pred_begin(), E = pred_end(); I != E; I++) {
    Ref = getPredicateOperandDDRef(I, true);
    Ref ? Ref->print(OS) : (void)(OS << Ref);
    OS << " ";

    printPredicate(OS, *I);

    OS << " ";
    Ref = getPredicateOperandDDRef(I, false);
    Ref ? Ref->print(OS) : (void)(OS << Ref);

    auto ConjI = getSucceedingConjunction(I);

    if (ConjI != conj_end()) {
      if (*ConjI == Instruction::And) {
        OS << " && ";
      } else if (*ConjI == Instruction::Or) {
        OS << " || ";
      } else {
        llvm_unreachable("Unexpected conjunction!");
      }
    }
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
  OS << "else";
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

void HLIf::addConjunction(unsigned Conj, CmpInst::Predicate Pred, DDRef *Ref1,
                          DDRef *Ref2) {
  assert((Conj == Instruction::And || Conj == Instruction::Or) &&
         "Unsupported conjunction!");
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

  Conjunctions.push_back(Conj);
  Predicates.push_back(Pred);

  NumOp = getNumOperandsInternal();
  DDRefs.resize(NumOp, nullptr);

  setOperandDDRefImpl(Ref1, NumOp - 2);
  setOperandDDRefImpl(Ref2, NumOp - 1);
}

void HLIf::removeConjunction(conj_iterator ConjI) {
  assert(!Conjunctions.empty() && "No conjuntions present!");

  auto PredI = getSucceedingPredicate(ConjI);

  /// Remove DDRefs associated with the succeeding predicate.
  removePredicateOperandDDRef(PredI, true);
  removePredicateOperandDDRef(PredI, false);

  /// Erase the iterators themselves.
  DDRefs.erase(DDRefs.begin() + getPredicateOperandDDRefOffset(PredI, true));
  DDRefs.erase(DDRefs.begin() + getPredicateOperandDDRefOffset(PredI, true));

  /// Erase the conjunction and predicate.
  Conjunctions.erase(ConjI);
  Predicates.erase(PredI);
}

DDRef *HLIf::getPredicateOperandDDRef(pred_iterator PredI, bool IsLHS) {
  const_pred_iterator CPredI(PredI);
  return const_cast<DDRef *>(getPredicateOperandDDRef(CPredI, IsLHS));
}

const DDRef *HLIf::getPredicateOperandDDRef(const_pred_iterator PredI,
                                            bool IsLHS) const {
  return getOperandDDRefImpl(getPredicateOperandDDRefOffset(PredI, IsLHS));
}

void HLIf::setPredicateOperandDDRef(DDRef *Ref, pred_iterator PredI,
                                    bool IsLHS) {
  setOperandDDRefImpl(Ref, getPredicateOperandDDRefOffset(PredI, IsLHS));
}

DDRef *HLIf::removePredicateOperandDDRef(pred_iterator PredI, bool IsLHS) {
  auto TRef = getPredicateOperandDDRef(PredI, IsLHS);

  if (TRef) {
    setPredicateOperandDDRef(nullptr, PredI, IsLHS);
  }

  return TRef;
}

HLIf::conj_iterator HLIf::getPrecedingConjunction(pred_iterator PredI) {
  if (PredI == pred_begin()) {
    return conj_end();
  }

  return (conj_begin() + (PredI - pred_begin()) - 1);
}

HLIf::const_conj_iterator
HLIf::getPrecedingConjunction(const_pred_iterator PredI) const {
  if (PredI == pred_begin()) {
    return conj_end();
  }

  return (conj_begin() + (PredI - pred_begin()) - 1);
}

HLIf::conj_iterator HLIf::getSucceedingConjunction(pred_iterator PredI) {
  if (PredI == (pred_end() - 1)) {
    return conj_end();
  }

  return (conj_begin() + (PredI - pred_begin()));
}

HLIf::const_conj_iterator
HLIf::getSucceedingConjunction(const_pred_iterator PredI) const {
  if (PredI == (pred_end() - 1)) {
    return conj_end();
  }

  return (conj_begin() + (PredI - pred_begin()));
}

HLIf::pred_iterator HLIf::getPrecedingPredicate(conj_iterator ConjI) {
  return (pred_begin() + (ConjI - conj_begin()));
}

HLIf::const_pred_iterator
HLIf::getPrecedingPredicate(const_conj_iterator ConjI) const {
  return (pred_begin() + (ConjI - conj_begin()));
}

HLIf::pred_iterator HLIf::getSucceedingPredicate(conj_iterator ConjI) {
  return (pred_begin() + (ConjI - conj_begin()) + 1);
}

HLIf::const_pred_iterator
HLIf::getSucceedingPredicate(const_conj_iterator ConjI) const {
  return (pred_begin() + (ConjI - conj_begin()) + 1);
}
