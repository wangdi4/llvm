//===------------ HLIf.cpp - Implements the HLIf class --------------------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

void HLIf::initialize() {
  unsigned NumOp;

  /// This call is to get around calling virtual functions in the constructor.
  NumOp = getNumOperandsInternal();

  RegDDRefs.resize(NumOp, nullptr);
}

HLIf::HLIf(HLNodeUtils &HNU, const HLPredicate &FirstPred, RegDDRef *Ref1,
           RegDDRef *Ref2)
    : HLDDNode(HNU, HLNode::HLIfVal) {
  assert(((FirstPred == PredicateTy::FCMP_FALSE) ||
          (FirstPred == PredicateTy::FCMP_TRUE) || (Ref1 && Ref2)) &&
         "DDRefs cannot be null!");
  assert((!Ref1 || (Ref1->getDestType() == Ref2->getDestType())) &&
         "Ref1/Ref2 type mismatch!");
  assert((!Ref1 || ((CmpInst::isIntPredicate(FirstPred) &&
                     (Ref1->getDestType()->isIntegerTy() ||
                      Ref1->getDestType()->isPointerTy())) ||
                    (CmpInst::isFPPredicate(FirstPred) &&
                     Ref1->getDestType()->isFloatingPointTy()))) &&
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
      BranchDbgLoc(HLIfObj.BranchDbgLoc) {
  const RegDDRef *Ref;
  ElseBegin = Children.end();
  initialize();

  /// Clone DDRefs
  auto II = HLIfObj.pred_begin();
  for (auto I = pred_begin(), E = pred_end(); I != E; I++, II++) {
    Ref = HLIfObj.getPredicateOperandDDRef(II, true);
    setPredicateOperandDDRef(Ref ? Ref->clone() : nullptr, I, true);
    Ref = HLIfObj.getPredicateOperandDDRef(II, false);
    setPredicateOperandDDRef(Ref ? Ref->clone() : nullptr, I, false);
  }
}

HLIf *HLIf::cloneImpl(GotoContainerTy *GotoList, LabelMapTy *LabelMap,
                      HLNodeMapper *NodeMapper) const {
  // Call the Copy Constructor
  HLIf *NewHLIf = new HLIf(*this);

  /// Loop over Then children and Else children
  for (auto ThenIter = this->then_begin(), ThenIterEnd = this->then_end();
       ThenIter != ThenIterEnd; ++ThenIter) {
    HLNode *NewHLNode =
        cloneBaseImpl(&*ThenIter, GotoList, LabelMap, NodeMapper);
    getHLNodeUtils().insertAsLastChild(NewHLIf, NewHLNode, true);
  }

  for (auto ElseIter = this->else_begin(), ElseIterEnd = this->else_end();
       ElseIter != ElseIterEnd; ++ElseIter) {
    HLNode *NewHLNode =
        cloneBaseImpl(&*ElseIter, GotoList, LabelMap, NodeMapper);
    getHLNodeUtils().insertAsLastChild(NewHLIf, NewHLNode, false);
  }

  return NewHLIf;
}

HLIf *HLIf::clone(HLNodeMapper *NodeMapper) const {
  return cast<HLIf>(HLNode::clone(NodeMapper));
}

void HLIf::printHeaderImpl(formatted_raw_ostream &OS, unsigned Depth,
                           const HLLoop *Loop, bool Detailed) const {

  bool FirstPred = true;
  OS << "if (";

  bool AnyFMF = false;

  /// Print predicates
  for (auto I = pred_begin(), E = pred_end(); I != E; ++I) {
    const RegDDRef *Ref;
    if (!FirstPred) {
      OS << " && ";
    }

    Ref = Loop ? Loop->getZttPredicateOperandDDRef(I, true)
               : getPredicateOperandDDRef(I, true);
    Ref ? Ref->print(OS, false) : (void)(OS << Ref);

    printPredicate(OS, *I);

    Ref = Loop ? Loop->getZttPredicateOperandDDRef(I, false)
               : getPredicateOperandDDRef(I, false);
    Ref ? Ref->print(OS, false) : (void)(OS << Ref);

    FirstPred = false;
    AnyFMF = AnyFMF || I->FMF.any();
  }

  OS << ")";

  if (Detailed && AnyFMF) {
    for (auto I = pred_begin(), E = pred_end(); I != E; ++I) {
      OS << " ";
      printFMF(OS, I->FMF);
    }
  }
}

void HLIf::printHeader(formatted_raw_ostream &OS, unsigned Depth,
                       bool Detailed) const {
  printHeaderImpl(OS, Depth, nullptr, Detailed);
}

void HLIf::printZttHeader(formatted_raw_ostream &OS, const HLLoop *Loop) const {
  printHeaderImpl(OS, 0, Loop, true);
}

void HLIf::print(formatted_raw_ostream &OS, unsigned Depth,
                 bool Detailed) const {

  indent(OS, Depth);
  printHeader(OS, Depth, Detailed);
  OS << "\n";

  HLDDNode::print(OS, Depth, Detailed);

  indent(OS, Depth);
  OS << "{\n";

  /// Print then children
  for (auto I = then_begin(), E = then_end(); I != E; ++I) {
    I->print(OS, Depth + 1, Detailed);
  }

  indent(OS, Depth);
  OS << "}\n";

  if (hasElseChildren()) {
    indent(OS, Depth);
    OS << "else\n";
    indent(OS, Depth);
    OS << "{\n";

    /// Print else children
    for (auto I = else_begin(), E = else_end(); I != E; ++I) {
      I->print(OS, Depth + 1, Detailed);
    }

    indent(OS, Depth);
    OS << "}\n";
  }
}

unsigned HLIf::getNumOperandsInternal() const {
  return (2 * Predicates.size());
}

unsigned HLIf::getNumOperands() const { return getNumOperandsInternal(); }

HLNode *HLIf::getFirstThenChild() {
  if (hasThenChildren()) {
    return &*then_begin();
  }

  return nullptr;
}

HLNode *HLIf::getLastThenChild() {
  if (hasThenChildren()) {
    return &*(std::prev(then_end()));
  }

  return nullptr;
}

HLNode *HLIf::getFirstElseChild() {
  if (hasElseChildren()) {
    return &*else_begin();
  }

  return nullptr;
}

HLNode *HLIf::getLastElseChild() {
  if (hasElseChildren()) {
    return &*(std::prev(else_end()));
  }

  return nullptr;
}

unsigned HLIf::getPredicateOperandDDRefOffset(const_pred_iterator CPredI,
                                              bool IsLHS) const {
  assert((CPredI != pred_end()) && "End iterator is not a valid input!");
  return ((2 * (CPredI - pred_begin())) + (IsLHS ? 0 : 1));
}

void HLIf::addPredicate(const HLPredicate &Pred, RegDDRef *Ref1,
                        RegDDRef *Ref2) {
  assert(Ref1 && Ref2 && "DDRef is null!");
  assert((Pred != PredicateTy::FCMP_FALSE) &&
         (Pred != PredicateTy::FCMP_TRUE) && "Invalid predicate!");
  assert((Ref1->getDestType() == Ref2->getDestType()) &&
         "Ref1/Ref2 type mismatch!");
  assert(((CmpInst::isIntPredicate(Pred) &&
           (Ref1->getDestType()->isIntegerTy() ||
            Ref1->getDestType()->isPointerTy())) ||
          (CmpInst::isFPPredicate(Pred) &&
           Ref1->getDestType()->isFloatingPointTy())) &&
         "Predicate/DDRef type mismatch!");
  unsigned NumOp;

  Predicates.push_back(Pred);

  NumOp = getNumOperandsInternal();
  RegDDRefs.resize(NumOp, nullptr);

  setOperandDDRefImpl(Ref1, NumOp - 2);
  setOperandDDRefImpl(Ref2, NumOp - 1);
}

HLIf::pred_iterator HLIf::getNonConstPredIterator(const_pred_iterator CPredI) {
  pred_iterator PredI(Predicates.begin());
  std::advance(PredI, std::distance<const_pred_iterator>(PredI, CPredI));
  return PredI;
}

void HLIf::removePredicate(const_pred_iterator CPredI) {
  assert(!Predicates.empty() && "No conjuntions present!");
  assert((CPredI != pred_end()) && "End iterator is not a valid input!");

  auto PredI = getNonConstPredIterator(CPredI);

  /// Remove DDRefs associated with the predicate.
  removePredicateOperandDDRef(CPredI, true);
  removePredicateOperandDDRef(CPredI, false);

  /// Erase the DDRef slots.
  RegDDRefs.erase(RegDDRefs.begin() +
                  getPredicateOperandDDRefOffset(CPredI, true));
  RegDDRefs.erase(RegDDRefs.begin() +
                  getPredicateOperandDDRefOffset(CPredI, true));

  /// Erase the predicate.
  Predicates.erase(PredI);
}

void HLIf::replacePredicate(const_pred_iterator CPredI,
                            const HLPredicate &NewPred) {
  assert((CPredI != pred_end()) && "End iterator is not a valid input!");
  auto PredI = getNonConstPredIterator(CPredI);
  *PredI = NewPred;
}

void HLIf::replacePredicate(const_pred_iterator CPredI, PredicateTy NewPred) {
  assert((CPredI != pred_end()) && "End iterator is not a valid input!");
  auto PredI = getNonConstPredIterator(CPredI);
  PredI->Kind = NewPred;
}

void HLIf::invertPredicate(const_pred_iterator CPredI) {
  assert((CPredI != pred_end()) && "End iterator is not a valid input!");
  auto PredI = getNonConstPredIterator(CPredI);
  auto PredKind = PredI->Kind;
  
  // Inversion is a no-op for undef predicate.
  if (PredKind != UNDEFINED_PREDICATE) {
    PredI->Kind = CmpInst::getInversePredicate(PredKind);
  }
}

RegDDRef *HLIf::getPredicateOperandDDRef(const_pred_iterator CPredI,
                                         bool IsLHS) const {
  return getOperandDDRefImpl(getPredicateOperandDDRefOffset(CPredI, IsLHS));
}

void HLIf::setPredicateOperandDDRef(RegDDRef *Ref, const_pred_iterator CPredI,
                                    bool IsLHS) {
  setOperandDDRefImpl(Ref, getPredicateOperandDDRefOffset(CPredI, IsLHS));
}

RegDDRef *HLIf::removePredicateOperandDDRef(const_pred_iterator CPredI,
                                            bool IsLHS) {
  auto TRef = getPredicateOperandDDRef(CPredI, IsLHS);

  if (TRef) {
    setPredicateOperandDDRef(nullptr, CPredI, IsLHS);
  }

  return TRef;
}

bool HLIf::isThenChild(const HLNode *Node) const {
  return getHLNodeUtils().isInTopSortNumMaxRange(Node, getFirstThenChild(),
                                                 getLastThenChild());
}

bool HLIf::isElseChild(const HLNode *Node) const {
  return getHLNodeUtils().isInTopSortNumMaxRange(Node, getFirstElseChild(),
                                                 getLastElseChild());
}

void HLIf::verify() const {
  bool ContainsTrueFalsePred = false;

  assert(getNumPredicates() > 0 &&
         "HLIf should contain at least one predicate");

  for (auto I = pred_begin(), E = pred_end(); I != E; ++I) {
    assert((CmpInst::isFPPredicate(*I) || CmpInst::isIntPredicate(*I) ||
            *I == UNDEFINED_PREDICATE) &&
           "Invalid predicate value, should be one of PredicateTy");

    bool IsBooleanPred = isPredicateTrueOrFalse(*I);
    ContainsTrueFalsePred = ContainsTrueFalsePred || IsBooleanPred;

    if (IsBooleanPred) {
      auto *DDRefLhs = getPredicateOperandDDRef(I, true);
      auto *DDRefRhs = getPredicateOperandDDRef(I, false);

      (void)DDRefLhs;
      (void)DDRefRhs;
      assert(DDRefLhs->isStandAloneUndefBlob() &&
             DDRefRhs->isStandAloneUndefBlob() &&
             "DDRefs should be undefined for FCMP_TRUE/FCMP_FALSE predicate");
    }
  }

  assert((!ContainsTrueFalsePred || getNumPredicates() == 1) &&
         "FCMP_TRUE/FCMP_FALSE cannot be combined with any other predicates");

  assert((hasThenChildren() || hasElseChildren()) &&
         "Found an empty *IF* construction, assumption that there should be no "
         "empty HLIfs");

  if (isUnknownLoopBottomTest()) {
    if (auto Child = getFirstThenChild()) {
      assert(isa<HLGoto>(Child) && "Unexpected bottom test structure!");
      assert((Child == getLastThenChild()) &&
             "Unexpected bottom test structure!");
      assert(!hasElseChildren() && "Unexpected bottom test structure!");
    } else {
      Child = getFirstElseChild();
      assert((Child && isa<HLGoto>(Child)) &&
             "Unexpected bottom test structure!");
      assert((Child == getLastElseChild()) &&
             "Unexpected bottom test structure!");
    }
  }

  HLDDNode::verify();
}

#ifndef NDEBUG
LLVM_DUMP_METHOD
void HLIf::dumpHeader() const {
  formatted_raw_ostream OS(dbgs());
  OS << "<" << getNumber() << "> ";
  printHeader(OS, 0);
}
#endif

bool HLIf::isKnownPredicate(bool *IsTrue) const {
  bool FinalResult = true;

  // Check every predicate if its value is known. Evaluate the result of the
  // whole HLIf statement.
  for (auto I = pred_begin(), E = pred_end(); I != E; ++I) {
    auto *DDRefLhs = getPredicateOperandDDRef(I, true);
    auto *DDRefRhs = getPredicateOperandDDRef(I, false);

    if (!DDRefLhs->isTerminalRef() || !DDRefRhs->isTerminalRef()) {
      return false;
    }

    bool Result;
    if (!HLNodeUtils::isKnownPredicate(DDRefLhs->getSingleCanonExpr(), *I,
                                       DDRefRhs->getSingleCanonExpr(),
                                       &Result)) {
      return false;
    }

    FinalResult = FinalResult && Result;
  }

  if (IsTrue) {
    *IsTrue = FinalResult;
  }

  return true;
}

bool HLIf::isUnknownLoopBottomTest() const {
  auto ParentLoop = dyn_cast<HLLoop>(getParent());
  return (ParentLoop && (ParentLoop->getBottomTest() == this));
}
