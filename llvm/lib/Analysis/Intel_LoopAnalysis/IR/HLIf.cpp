//===------------ HLIf.cpp - Implements the HLIf class --------------------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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

#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLIf.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Support/Debug.h"

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
    : HLDDNode(HNU, HLNode::HLIfVal), UnswitchDisabled(false) {
  assert((isPredicateTrueOrFalse(FirstPred) || (Ref1 && Ref2)) &&
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
      UnswitchDisabled(HLIfObj.UnswitchDisabled),
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

HLIf *HLIf::cloneEmpty() const { return new HLIf(*this); }

HLIf *HLIf::cloneImpl(GotoContainerTy *GotoList, LabelMapTy *LabelMap,
                      HLNodeMapper *NodeMapper) const {

  HLIf *NewHLIf = cloneEmpty();

  /// Loop over Then children and Else children
  for (auto ThenIter = this->then_begin(), ThenIterEnd = this->then_end();
       ThenIter != ThenIterEnd; ++ThenIter) {
    HLNode *NewHLNode =
        cloneBaseImpl(&*ThenIter, GotoList, LabelMap, NodeMapper);
    HLNodeUtils::insertAsLastThenChild(NewHLIf, NewHLNode);
  }

  for (auto ElseIter = this->else_begin(), ElseIterEnd = this->else_end();
       ElseIter != ElseIterEnd; ++ElseIter) {
    HLNode *NewHLNode =
        cloneBaseImpl(&*ElseIter, GotoList, LabelMap, NodeMapper);
    HLNodeUtils::insertAsLastElseChild(NewHLIf, NewHLNode);
  }

  return NewHLIf;
}

HLIf *HLIf::clone(HLNodeMapper *NodeMapper) const {
  return cast<HLIf>(HLNode::clone(NodeMapper));
}

void HLIf::printHeaderImpl(formatted_raw_ostream &OS, unsigned Depth,
                           const HLLoop *Loop, bool Detailed) const {
#if !INTEL_PRODUCT_RELEASE

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

  printDistributePoint(OS);

  if (isUnswitchDisabled()) {
    OS << " <no_unswitch>";
  }
#endif // !INTEL_PRODUCT_RELEASE
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
#if !INTEL_PRODUCT_RELEASE

  indent(OS, Depth);
  printHeader(OS, Depth, Detailed);
  OS << "\n";

  HLDDNode::print(OS, Depth, Detailed);

  indent(OS, Depth);
  OS << "{\n";

  // Print IV update for the unknown loop which will be generated during code
  // gen.
  // ddref_begin() check is a workaround to skip the backedge check until we
  // form ddrefs in the framework otherwise we encounter an assert for null
  // stride ref.
  if (*ddref_begin() && isUnknownLoopBottomTest()) {
    auto Level = getParentLoop()->getNestingLevel();
    indent(OS, Depth);
    // Extra indentation needed to print iv update child.
    OS.indent(IndentWidth);
    OS << "<i" << Level << " = "
       << "i" << Level << " + 1>\n";
  }

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
#endif // !INTEL_PRODUCT_RELEASE
}

unsigned HLIf::getNumOperandsInternal() const {
  return (2 * Predicates.size());
}

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
  assert((Ref1->getDestType() == Ref2->getDestType()) &&
         "Ref1/Ref2 type mismatch!");
  assert(((CmpInst::isIntPredicate(Pred) &&
           (Ref1->getDestType()->isIntegerTy() ||
            Ref1->getDestType()->isPointerTy())) ||
          (CmpInst::isFPPredicate(Pred) &&
           (isPredicateTrueOrFalse(Pred) ||
            Ref1->getDestType()->isFloatingPointTy()))) &&
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

    if (getProfileData()) {
      swapProfileData();
    }
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
  assert(getNumPredicates() > 0 &&
         "HLIf should contain at least one predicate");

  for (auto I = pred_begin(), E = pred_end(); I != E; ++I) {
    assert((CmpInst::isFPPredicate(*I) || CmpInst::isIntPredicate(*I) ||
            *I == UNDEFINED_PREDICATE) &&
           "Invalid predicate value, should be one of PredicateTy");

    if (isPredicateTrueOrFalse(*I)) {
      auto *DDRefLhs = getPredicateOperandDDRef(I, true);
      auto *DDRefRhs = getPredicateOperandDDRef(I, false);

      (void)DDRefLhs;
      (void)DDRefRhs;
      assert(DDRefLhs->isStandAloneUndefBlob() &&
             DDRefRhs->isStandAloneUndefBlob() &&
             "DDRefs should be undefined for FCMP_TRUE/FCMP_FALSE predicate");
    }
  }

  assert((hasThenChildren() || hasElseChildren()) &&
         "Found an empty *IF* construction, assumption that there should be no "
         "empty HLIfs");

  if (isUnknownLoopBottomTest()) {
    auto *Backedge = getLastThenChild();
    (void)Backedge;
    assert(isa<HLGoto>(Backedge) && "Could not find unknown loop's backedge!");
    assert(cast<HLGoto>(Backedge)->getTargetLabel() ==
               getParentLoop()->getFirstChild() &&
           "Could not find unknown loop's backedge!");
    assert(!hasElseChildren() && "Unexpected bottom test structure!");
  }

  HLDDNode::verify();
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD
void HLIf::dumpHeader() const {
  formatted_raw_ostream OS(dbgs());
  indent(OS, 0);
  printHeader(OS, 0);
}
#endif

bool HLIf::isKnownPredicate(bool *IsTrue) const {
  return HLNodeUtils::isKnownPredicateRange(
      pred_begin(), pred_end(),
      std::bind(&HLIf::getPredicateOperandDDRef, this, std::placeholders::_1,
                std::placeholders::_2),
      IsTrue);
}

bool HLIf::isUnknownLoopBottomTest() const {
  auto ParentLoop = dyn_cast_or_null<HLLoop>(getParent());
  return (ParentLoop && (ParentLoop->getBottomTest() == this));
}
