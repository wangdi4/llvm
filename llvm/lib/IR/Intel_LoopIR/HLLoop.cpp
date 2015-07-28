//===-------- HLLoop.cpp - Implements the HLLoop class --------------------===//
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
// This file implements the HLLoop class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Debug.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Intel_LoopIR/HLLoop.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

void HLLoop::initialize() {
  unsigned NumOp;

  ChildBegin = Children.end();
  PostexitBegin = Children.end();

  /// This call is to get around calling virtual functions in the constructor.
  NumOp = getNumOperandsInternal();

  RegDDRefs.resize(NumOp, nullptr);
}

HLLoop::HLLoop(const Loop *LLVMLoop, bool IsDoWh)
    : HLDDNode(HLNode::HLLoopVal), OrigLoop(LLVMLoop), Ztt(nullptr),
      IsDoWhile(IsDoWh), NestingLevel(0), IsInnermost(true) {
  assert(LLVMLoop && "LLVM loop cannot be null!");

  SmallVector<BasicBlock *, 8> Exits;

  initialize();
  OrigLoop->getExitingBlocks(Exits);
  setNumExits(Exits.size());
}

HLLoop::HLLoop(HLIf *ZttIf, RegDDRef *LowerDDRef, RegDDRef *UpperDDRef,
               RegDDRef *StrideDDRef, bool IsDoWh, unsigned NumEx)
    : HLDDNode(HLNode::HLLoopVal), OrigLoop(nullptr), Ztt(nullptr),
      IsDoWhile(IsDoWh), NestingLevel(0), IsInnermost(true) {
  assert((!ZttIf || !IsDoWh) && "Do while loop cannot have ztt!");

  initialize();
  setNumExits(NumEx);

  assert(((LowerDDRef && UpperDDRef && StrideDDRef) ||
          (!LowerDDRef && !UpperDDRef && !StrideDDRef)) &&
         "Inconsistent loop DDRefs!");

  /// Sets ztt properly, with all the ddref setup.
  setZtt(ZttIf);

  setLowerDDRef(LowerDDRef);
  setUpperDDRef(UpperDDRef);
  setStrideDDRef(StrideDDRef);
}

HLLoop::HLLoop(const HLLoop &HLLoopObj, GotoContainerTy *GotoList,
               LabelMapTy *LabelMap)
    : HLDDNode(HLLoopObj), OrigLoop(HLLoopObj.OrigLoop), Ztt(nullptr),
      IsDoWhile(HLLoopObj.IsDoWhile), NumExits(HLLoopObj.NumExits),
      NestingLevel(0), IsInnermost(HLLoopObj.IsInnermost) {

  assert(GotoList && " GotoList is null.");
  assert(LabelMap && " LabelMap is null.");

  const RegDDRef *Ref;

  initialize();

  /// Clone the Ztt
  if (HLLoopObj.hasZtt()) {
    setZtt(HLLoopObj.Ztt->clone());
  }

  /// Clone loop RegDDRefs
  setLowerDDRef((Ref = HLLoopObj.getLowerDDRef()) ? Ref->clone() : nullptr);
  setUpperDDRef((Ref = HLLoopObj.getUpperDDRef()) ? Ref->clone() : nullptr);
  setStrideDDRef((Ref = HLLoopObj.getStrideDDRef()) ? Ref->clone() : nullptr);

  /// Loop over children, preheader and postexit
  for (auto PreIter = HLLoopObj.pre_begin(), PreIterEnd = HLLoopObj.pre_end();
       PreIter != PreIterEnd; ++PreIter) {
    HLNode *NewHLNode = PreIter->clone();
    HLNodeUtils::insertAsLastPreheaderNode(this, NewHLNode);
  }

  // Clone the children.
  // The goto target label's will not be updated and would be done by caller.
  for (auto ChildIter = HLLoopObj.child_begin(),
            ChildIterEnd = HLLoopObj.child_end();
       ChildIter != ChildIterEnd; ++ChildIter) {
    HLNode *NewHLNode = cloneBaseImpl(ChildIter, GotoList, LabelMap);
    HLNodeUtils::insertAsLastChild(this, NewHLNode);
  }

  for (auto PostIter = HLLoopObj.post_begin(),
            PostIterEnd = HLLoopObj.post_end();
       PostIter != PostIterEnd; ++PostIter) {
    HLNode *NewHLNode = PostIter->clone();
    HLNodeUtils::insertAsLastPostexitNode(this, NewHLNode);
  }
}

// HLLoop::HLLoop(const HLLoop &HLLoopObj) :
//    HLLoop(HLLoopObj, nullptr, nullptr) {}

HLLoop *HLLoop::cloneImpl(GotoContainerTy *GotoList,
                          LabelMapTy *LabelMap) const {

  // Call the Copy Constructor
  HLLoop *NewHLLoop = new HLLoop(*this, GotoList, LabelMap);

  return NewHLLoop;
}

HLLoop *HLLoop::clone() const {

  HLContainerTy NContainer;
  HLNodeUtils::cloneSequence(&NContainer, this);
  HLLoop *NewLoop = cast<HLLoop>(NContainer.remove(NContainer.begin()));
  return NewLoop;
}

void HLLoop::print(formatted_raw_ostream &OS, unsigned Depth) const {
  const RegDDRef *Ref;

  indent(OS, Depth);

  /// Print preheader
  for (auto I = pre_begin(), E = pre_end(); I != E; I++) {
    I->print(OS, Depth + 1);
  }

  /// Print header
  if (isDoLoop() || isDoWhileLoop() || isDoMultiExitLoop()) {
    OS << "+ DO i" << NestingLevel << " = ";
    Ref = getLowerDDRef();
    Ref ? Ref->print(OS) : (void)(OS << Ref);
    OS << ", ";
    Ref = getUpperDDRef();
    Ref ? Ref->print(OS) : (void)(OS << Ref);
    OS << ", ";
    Ref = getStrideDDRef();
    Ref ? Ref->print(OS) : (void)(OS << Ref);

    OS.indent(IndentWidth);
    if (isDoLoop()) {
      OS << "<DO_LOOP>";
    } else if (isDoWhileLoop()) {
      OS << "<DO_WHILE_LOOP>";
    } else if (isDoMultiExitLoop()) {
      OS << "<DO_MULTI_EXIT_LOOP>";
    }

    OS << "\n";
  } else if (isUnknownLoop()) {
    OS << "+ UNKNOWN LOOP i" << NestingLevel << "\n";
  } else {
    llvm_unreachable("Unexpected loop type!");
  }

  /// Print children
  for (auto I = child_begin(), E = child_end(); I != E; I++) {
    I->print(OS, Depth + 1);
  }

  /// Print footer
  indent(OS, Depth);
  OS << "+ END LOOP\n";

  /// Print postexit
  for (auto I = post_begin(), E = post_end(); I != E; I++) {
    I->print(OS, Depth + 1);
  }
}

void HLLoop::setNumExits(unsigned NumEx) {
  assert(NumEx && "Number of exits cannot be zero!");
  NumExits = NumEx;
}

void HLLoop::addZttPredicate(CmpInst::Predicate Pred, RegDDRef *Ref1,
                             RegDDRef *Ref2) {
  assert(hasZtt() && "Ztt is absent!");
  Ztt->addPredicate(Pred, Ref1, Ref2);

  ztt_pred_iterator LastIt = std::prev(ztt_pred_end());

  /// Move the RegDDRefs to loop.
  setZttPredicateOperandDDRef(Ztt->removePredicateOperandDDRef(LastIt, true),
                              LastIt, true);
  setZttPredicateOperandDDRef(Ztt->removePredicateOperandDDRef(LastIt, false),
                              LastIt, false);
}

void HLLoop::removeZttPredicate(ztt_pred_iterator PredI) {
  assert(hasZtt() && "Ztt is absent!");

  /// Remove RegDDRefs from loop.
  removeZttPredicateOperandDDRef(PredI, true);
  removeZttPredicateOperandDDRef(PredI, false);

  /// Erase the DDRef slots from loop.
  RegDDRefs.erase(RegDDRefs.begin() + getNumLoopDDRefs() +
                  Ztt->getPredicateOperandDDRefOffset(PredI, true));
  RegDDRefs.erase(RegDDRefs.begin() + getNumLoopDDRefs() +
                  Ztt->getPredicateOperandDDRefOffset(PredI, true));

  /// Remove predicate from ztt.
  Ztt->removePredicate(PredI);
}

RegDDRef *HLLoop::getZttPredicateOperandDDRef(ztt_pred_iterator PredI,
                                              bool IsLHS) {
  assert(hasZtt() && "Ztt is absent!");
  return getOperandDDRefImpl(getNumLoopDDRefs() +
                             Ztt->getPredicateOperandDDRefOffset(PredI, IsLHS));
}

const RegDDRef *
HLLoop::getZttPredicateOperandDDRef(const_ztt_pred_iterator PredI,
                                    bool IsLHS) const {
  assert(hasZtt() && "Ztt is absent!");
  return getOperandDDRefImpl(getNumLoopDDRefs() +
                             Ztt->getPredicateOperandDDRefOffset(PredI, IsLHS));
}

void HLLoop::setZttPredicateOperandDDRef(RegDDRef *Ref, ztt_pred_iterator PredI,
                                         bool IsLHS) {
  assert(hasZtt() && "Ztt is absent!");
  setOperandDDRefImpl(Ref,
                      getNumLoopDDRefs() +
                          Ztt->getPredicateOperandDDRefOffset(PredI, IsLHS));
}

RegDDRef *HLLoop::removeZttPredicateOperandDDRef(ztt_pred_iterator PredI,
                                                 bool IsLHS) {
  assert(hasZtt() && "Ztt is absent!");
  auto TRef = getZttPredicateOperandDDRef(PredI, IsLHS);

  if (TRef) {
    setZttPredicateOperandDDRef(nullptr, PredI, IsLHS);
  }

  return TRef;
}

RegDDRef *HLLoop::getLowerDDRef() { return getOperandDDRefImpl(0); }

const RegDDRef *HLLoop::getLowerDDRef() const {
  return const_cast<HLLoop *>(this)->getLowerDDRef();
}

void HLLoop::setLowerDDRef(RegDDRef *Ref) {
  assert((!Ref || Ref->isSimpleRef()) && "Invalid LowerDDRef!");

  setOperandDDRefImpl(Ref, 0);
}

RegDDRef *HLLoop::removeLowerDDRef() {
  auto TRef = getLowerDDRef();

  if (TRef) {
    setLowerDDRef(nullptr);
  }

  return TRef;
}

RegDDRef *HLLoop::getUpperDDRef() { return getOperandDDRefImpl(1); }

const RegDDRef *HLLoop::getUpperDDRef() const {
  return const_cast<HLLoop *>(this)->getUpperDDRef();
}

void HLLoop::setUpperDDRef(RegDDRef *Ref) {
  assert((!Ref || Ref->isSimpleRef()) && "Invalid UpperDDRef!");

  setOperandDDRefImpl(Ref, 1);
}

RegDDRef *HLLoop::removeUpperDDRef() {
  auto TRef = getUpperDDRef();

  if (TRef) {
    setUpperDDRef(nullptr);
  }

  return TRef;
}

RegDDRef *HLLoop::getStrideDDRef() { return getOperandDDRefImpl(2); }

const RegDDRef *HLLoop::getStrideDDRef() const {
  return const_cast<HLLoop *>(this)->getStrideDDRef();
}

void HLLoop::setStrideDDRef(RegDDRef *Ref) {
  assert((!Ref || Ref->isSimpleRef()) && "Invalid StrideDDRef!");

  setOperandDDRefImpl(Ref, 2);
}

RegDDRef *HLLoop::removeStrideDDRef() {
  auto TRef = getStrideDDRef();

  if (TRef) {
    setStrideDDRef(nullptr);
  }

  return TRef;
}

void HLLoop::setZtt(HLIf *ZttIf) {
  assert(!hasZtt() && "Attempt to overwrite ztt, use removeZtt instead!");

  if (!ZttIf) {
    return;
  }

  assert((!ZttIf->hasThenChildren() && !ZttIf->hasElseChildren()) &&
         "Ztt "
         "cannot have any children!");

  Ztt = ZttIf;

  RegDDRefs.resize(getNumOperandsInternal(), nullptr);

  /// Move DDRef pointers to avoid unnecessary cloning.
  for (auto I = ztt_pred_begin(), E = ztt_pred_end(); I != E; I++) {
    setZttPredicateOperandDDRef(Ztt->removePredicateOperandDDRef(I, true), I,
                                true);
    setZttPredicateOperandDDRef(Ztt->removePredicateOperandDDRef(I, false), I,
                                false);
  }
}

HLIf *HLLoop::removeZtt() {
  assert(hasZtt() && "Loop doesn't have ztt!");

  HLIf *If = Ztt;

  /// Move Ztt DDRefs back to If.
  for (auto I = ztt_pred_begin(), E = ztt_pred_end(); I != E; I++) {
    If->setPredicateOperandDDRef(removeZttPredicateOperandDDRef(I, true), I,
                                 true);
    If->setPredicateOperandDDRef(removeZttPredicateOperandDDRef(I, false), I,
                                 false);
  }

  Ztt = nullptr;
  If->setParent(nullptr);

  resizeToNumLoopDDRefs();

  return If;
}

CanonExpr *HLLoop::getLoopCanonExpr(RegDDRef *Ref) {
  if (!Ref) {
    return nullptr;
  }

  return Ref->getSingleCanonExpr();
}

const CanonExpr *HLLoop::getLoopCanonExpr(const RegDDRef *Ref) const {
  return const_cast<HLLoop *>(this)->getLoopCanonExpr(Ref);
}

CanonExpr *HLLoop::getLowerCanonExpr() {
  return getLoopCanonExpr(getLowerDDRef());
}

const CanonExpr *HLLoop::getLowerCanonExpr() const {
  return const_cast<HLLoop *>(this)->getLowerCanonExpr();
}

CanonExpr *HLLoop::getUpperCanonExpr() {
  return getLoopCanonExpr(getUpperDDRef());
}

const CanonExpr *HLLoop::getUpperCanonExpr() const {
  return const_cast<HLLoop *>(this)->getUpperCanonExpr();
}

CanonExpr *HLLoop::getStrideCanonExpr() {
  return getLoopCanonExpr(getStrideDDRef());
}

const CanonExpr *HLLoop::getStrideCanonExpr() const {
  return const_cast<HLLoop *>(this)->getStrideCanonExpr();
}

const CanonExpr *HLLoop::getTripCountCanonExpr() const {
  /// TODO implement later
  return nullptr;
}

unsigned HLLoop::getNumOperandsInternal() const {
  return getNumLoopDDRefs() + getNumZttOperands();
}

unsigned HLLoop::getNumOperands() const { return getNumOperandsInternal(); }

unsigned HLLoop::getNumZttOperands() const {
  if (hasZtt()) {
    return Ztt->getNumOperands();
  }

  return 0;
}

void HLLoop::resizeToNumLoopDDRefs() {
  RegDDRefs.resize(getNumLoopDDRefs(), nullptr);
}

HLNode *HLLoop::getFirstPreheaderNode() {
  if (hasPreheader()) {
    return pre_begin();
  }

  return nullptr;
}

HLNode *HLLoop::getLastPreheaderNode() {
  if (hasPreheader()) {
    return std::prev(pre_end());
  }

  return nullptr;
}

HLNode *HLLoop::getFirstPostexitNode() {
  if (hasPostexit()) {
    return post_begin();
  }

  return nullptr;
}

HLNode *HLLoop::getLastPostexitNode() {
  if (hasPostexit()) {
    return std::prev(post_end());
  }

  return nullptr;
}

HLNode *HLLoop::getFirstChild() {
  if (hasChildren()) {
    return child_begin();
  }

  return nullptr;
}

HLNode *HLLoop::getLastChild() {
  if (hasChildren()) {
    return std::prev(child_end());
  }

  return nullptr;
}
