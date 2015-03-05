//===-------- HLLoop.cpp - Implements the HLLoop class --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HLLoop class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Debug.h"
#include "llvm/IR/Intel_LoopIR/HLLoop.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

HLLoop::HLLoop(HLIf *ZttIf, DDRef *LowerDDRef, DDRef *TripCountDDRef,
               DDRef *StrideDDRef, bool IsDoWh, unsigned NumEx)
    : HLDDNode(HLNode::HLLoopVal), Ztt(ZttIf), IsDoWhile(IsDoWh),
      NestingLevel(0), IsInnermost(true) {
  assert((!ZttIf || !IsDoWh) && "Do while loop cannot have ztt!");
  assert((!IsDoWh || NumEx == 1) && "Do while loop cannot have multiple "
                                    "exits!");
  assert(((LowerDDRef && TripCountDDRef && StrideDDRef) ||
          (!LowerDDRef && !TripCountDDRef && !StrideDDRef)) &&
         "Inconsistent loop DDRefs");
  unsigned NumOp;

  ChildBegin = Children.end();
  PostexitBegin = Children.end();

  // This call is to get around calling virtual functions in the constructor.
  NumOp = getNumOperandsInternal();

  if (NumOp > DDRefs.size()) {
    DDRefs.resize(NumOp, nullptr);
  }

  setNumExits(NumEx);

  /// Sets ztt properly, with all the ddref setup.
  setZtt(ZttIf);

  setLowerDDRef(LowerDDRef);
  setTripCountDDRef(TripCountDDRef);
  setStrideDDRef(StrideDDRef);
}

HLLoop::HLLoop(const HLLoop &HLLoopObj)
    : HLDDNode(HLLoopObj), Ztt(nullptr), IsDoWhile(HLLoopObj.IsDoWhile),
      NumExits(HLLoopObj.NumExits), NestingLevel(0),
      IsInnermost(HLLoopObj.IsInnermost) {

  const DDRef *Ref;

  ChildBegin = Children.end();
  PostexitBegin = Children.end();

  /// Clone the Ztt
  if (HLLoopObj.hasZtt()) {
    setZtt(HLLoopObj.Ztt->clone());
  }

  /// Clone loop DDRefs
  setLowerDDRef((Ref = HLLoopObj.getLowerDDRef()) ? Ref->clone() : nullptr);
  setTripCountDDRef((Ref = HLLoopObj.getTripCountDDRef()) ? Ref->clone()
                                                          : nullptr);
  setStrideDDRef((Ref = HLLoopObj.getStrideDDRef()) ? Ref->clone() : nullptr);

  /// Loop over children, preheader and postexit
  for (auto PreIter = HLLoopObj.pre_begin(), PreIterEnd = HLLoopObj.pre_end();
       PreIter != PreIterEnd; ++PreIter) {
    HLNode *NewHLNode = PreIter->clone();
    HLNodeUtils::insertAsLastPreheaderNode(this, NewHLNode);
  }

  for (auto ChildIter = HLLoopObj.child_begin(),
            ChildIterEnd = HLLoopObj.child_end();
       ChildIter != ChildIterEnd; ++ChildIter) {
    HLNode *NewHLNode = ChildIter->clone();
    HLNodeUtils::insertAsLastChild(this, NewHLNode);
  }

  for (auto PostIter = HLLoopObj.post_begin(),
            PostIterEnd = HLLoopObj.post_end();
       PostIter != PostIterEnd; ++PostIter) {
    HLNode *NewHLNode = PostIter->clone();
    HLNodeUtils::insertAsLastPostexitNode(this, NewHLNode);
  }
}

HLLoop *HLLoop::clone() const {

  /// Call the Copy Constructor
  HLLoop *NewHLLoop = new HLLoop(*this);

  return NewHLLoop;
}

void HLLoop::setNumExits(unsigned NumEx) {
  assert(NumEx && "Number of exits cannot be zero!");
  NumExits = NumEx;
}

DDRef *HLLoop::getLowerDDRef() { return getOperandDDRefImpl(0); }

const DDRef *HLLoop::getLowerDDRef() const {
  return const_cast<HLLoop *>(this)->getLowerDDRef();
}

void HLLoop::setLowerDDRef(DDRef *Ref) {
  assert((!Ref || !isa<RegDDRef>(Ref) ||
          ((cast<RegDDRef>(Ref)->getNumDimensions() == 1) &&
           !cast<RegDDRef>(Ref)->hasGEPInfo())) &&
         "Invalid LowerDDRef!");

  setOperandDDRefImpl(Ref, 0);
}

DDRef *HLLoop::removeLowerDDRef() {
  auto TRef = getLowerDDRef();

  if (TRef) {
    setLowerDDRef(nullptr);
  }

  return TRef;
}

DDRef *HLLoop::getTripCountDDRef() { return getOperandDDRefImpl(1); }

const DDRef *HLLoop::getTripCountDDRef() const {
  return const_cast<HLLoop *>(this)->getTripCountDDRef();
}

void HLLoop::setTripCountDDRef(DDRef *Ref) {
  assert((!Ref || !isa<RegDDRef>(Ref) ||
          ((cast<RegDDRef>(Ref)->getNumDimensions() == 1) &&
           !cast<RegDDRef>(Ref)->hasGEPInfo())) &&
         "Invalid TripCountDDRef!");

  setOperandDDRefImpl(Ref, 1);
}

DDRef *HLLoop::removeTripCountDDRef() {
  auto TRef = getTripCountDDRef();

  if (TRef) {
    setTripCountDDRef(nullptr);
  }

  return TRef;
}

DDRef *HLLoop::getStrideDDRef() { return getOperandDDRefImpl(2); }

const DDRef *HLLoop::getStrideDDRef() const {
  return const_cast<HLLoop *>(this)->getStrideDDRef();
}

void HLLoop::setStrideDDRef(DDRef *Ref) {
  assert((!Ref || !isa<RegDDRef>(Ref) ||
          ((cast<RegDDRef>(Ref)->getNumDimensions() == 1) &&
           !cast<RegDDRef>(Ref)->hasGEPInfo())) &&
         "Invalid StrideDDRef!");

  setOperandDDRefImpl(Ref, 2);
}

DDRef *HLLoop::removeStrideDDRef() {
  auto TRef = getStrideDDRef();

  if (TRef) {
    setStrideDDRef(nullptr);
  }

  return TRef;
}

DDRef *HLLoop::getZttOperandDDRef(unsigned OperandNum) {
  assert(OperandNum < getNumZttOperands() && "Operand is out of range!");

  return getOperandDDRefImpl(OperandNum + getNumLoopDDRefs());
}

const DDRef *HLLoop::getZttOperandDDRef(unsigned OperandNum) const {
  return const_cast<HLLoop *>(this)->getZttOperandDDRef(OperandNum);
}

void HLLoop::setZttOperandDDRef(DDRef *Ref, unsigned OperandNum) {
  assert(OperandNum < getNumZttOperands() && "Operand is out of range!");

  setOperandDDRefImpl(Ref, OperandNum + getNumLoopDDRefs());
}

DDRef *HLLoop::removeZttOperandDDRef(unsigned OperandNum) {
  auto TRef = getZttOperandDDRef(OperandNum);

  if (TRef) {
    setZttOperandDDRef(nullptr, OperandNum);
  }

  return TRef;
}

void HLLoop::setZtt(HLIf *ZttIf) {
  assert(!hasZtt() && "Attempt to overwrite ztt!");

  if (!ZttIf) {
    return;
  }

  assert((ZttIf->isThenEmpty() && ZttIf->isElseEmpty()) && "Ztt cannot have "
                                                           "any children!");

  Ztt = ZttIf;

  unsigned NumOp = ZttIf->getNumOperands();

  /// Copy DDRef pointers to avoid unnecessary cloning.
  for (unsigned I = 0; I < NumOp; I++) {
    setZttOperandDDRef(ZttIf->getOperandDDRef(I), I);
  }
}

HLIf *HLLoop::removeZtt() {
  assert(hasZtt() && "Loop doesn't have ztt!");

  HLIf *If;

  If = Ztt;
  If->setParent(nullptr);

  /// Set Ztt DDRefs' Node back to Ztt.
  for (unsigned I = 0; I < getNumZttOperands(); I++) {
    if (auto TRef = getZttOperandDDRef(I)) {
      setNode(TRef, Ztt);
    }
  }

  resizeToNumLoopDDRefs();

  return If;
}

const CanonExpr *HLLoop::getLoopCanonExpr(const DDRef *Ref) const {
  if (!Ref) {
    return nullptr;
  }

  if (auto CRef = dyn_cast<ConstDDRef>(Ref)) {
    return CRef->getCanonExpr();
  } else if (auto RRef = dyn_cast<RegDDRef>(Ref)) {
    return RRef->getSingleCanonExpr();
  } else {
    llvm_unreachable("Unexpected condition!");
  }

  return nullptr;
}

const CanonExpr *HLLoop::getLowerCanonExpr() const {
  return getLoopCanonExpr(getLowerDDRef());
}

const CanonExpr *HLLoop::getTripCountCanonExpr() const {
  return getLoopCanonExpr(getTripCountDDRef());
}

const CanonExpr *HLLoop::getStrideCanonExpr() const {
  return getLoopCanonExpr(getStrideDDRef());
}

const CanonExpr *HLLoop::getUpperCanonExpr() const {
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
  DDRefs.resize(getNumLoopDDRefs(), nullptr);
}
