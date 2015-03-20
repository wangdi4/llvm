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

  DDRefs.resize(NumOp, nullptr);
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

HLLoop::HLLoop(HLIf *ZttIf, DDRef *LowerDDRef, DDRef *TripCountDDRef,
               DDRef *StrideDDRef, bool IsDoWh, unsigned NumEx)
    : HLDDNode(HLNode::HLLoopVal), OrigLoop(nullptr), Ztt(nullptr),
      IsDoWhile(IsDoWh), NestingLevel(0), IsInnermost(true) {
  assert((!ZttIf || !IsDoWh) && "Do while loop cannot have ztt!");

  initialize();
  setNumExits(NumEx);

  assert(((LowerDDRef && TripCountDDRef && StrideDDRef) ||
          (!LowerDDRef && !TripCountDDRef && !StrideDDRef)) &&
         "Inconsistent loop DDRefs!");

  /// Sets ztt properly, with all the ddref setup.
  setZtt(ZttIf);

  setLowerDDRef(LowerDDRef);
  setTripCountDDRef(TripCountDDRef);
  setStrideDDRef(StrideDDRef);
}

HLLoop::HLLoop(const HLLoop &HLLoopObj)
    : HLDDNode(HLLoopObj), OrigLoop(HLLoopObj.OrigLoop), Ztt(nullptr),
      IsDoWhile(HLLoopObj.IsDoWhile), NumExits(HLLoopObj.NumExits),
      NestingLevel(0), IsInnermost(HLLoopObj.IsInnermost) {

  const DDRef *Ref;

  initialize();

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
  assert(!hasZtt() && "Attempt to overwrite ztt, use removeZtt instead!");

  if (!ZttIf) {
    return;
  }

  assert((!ZttIf->hasThenChildren() && !ZttIf->hasElseChildren()) &&
         "Ztt "
         "cannot have any children!");

  Ztt = ZttIf;

  unsigned NumOp = ZttIf->getNumOperands();

  DDRefs.resize(getNumOperandsInternal(), nullptr);

  /// Copy DDRef pointers to avoid unnecessary cloning.
  for (unsigned I = 0; I < NumOp; I++) {
    setZttOperandDDRef(ZttIf->getOperandDDRef(I), I);
  }
}

HLIf *HLLoop::removeZtt() {
  assert(hasZtt() && "Loop doesn't have ztt!");

  HLIf *If;

  If = Ztt;
  Ztt = nullptr;
  If->setParent(nullptr);

  /// Set Ztt DDRefs' Node back to Ztt.
  for (unsigned I = 0; I < getNumZttOperands(); I++) {
    if (auto TRef = getZttOperandDDRef(I)) {
      setNode(TRef, If);
    }
  }

  resizeToNumLoopDDRefs();

  return If;
}

CanonExpr *HLLoop::getLoopCanonExpr(DDRef *Ref) {
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

const CanonExpr *HLLoop::getLoopCanonExpr(const DDRef *Ref) const {
  return const_cast<HLLoop *>(this)->getLoopCanonExpr(Ref);
}

CanonExpr *HLLoop::getLowerCanonExpr() {
  return getLoopCanonExpr(getLowerDDRef());
}

const CanonExpr *HLLoop::getLowerCanonExpr() const {
  return const_cast<HLLoop *>(this)->getLowerCanonExpr();
}

CanonExpr *HLLoop::getTripCountCanonExpr() {
  return getLoopCanonExpr(getTripCountDDRef());
}

const CanonExpr *HLLoop::getTripCountCanonExpr() const {
  return const_cast<HLLoop *>(this)->getTripCountCanonExpr();
}

CanonExpr *HLLoop::getStrideCanonExpr() {
  return getLoopCanonExpr(getStrideDDRef());
}

const CanonExpr *HLLoop::getStrideCanonExpr() const {
  return const_cast<HLLoop *>(this)->getStrideCanonExpr();
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
