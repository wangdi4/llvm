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
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"

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

// IsInnermost flag is initialized to true, please refer to the header file.
HLLoop::HLLoop(const Loop *LLVMLoop, bool IsDoWh)
    : HLDDNode(HLNode::HLLoopVal), OrigLoop(LLVMLoop), Ztt(nullptr),
      IsDoWhile(IsDoWh), NestingLevel(0), IsInnermost(true) {
  assert(LLVMLoop && "LLVM loop cannot be null!");

  SmallVector<BasicBlock *, 8> Exits;

  initialize();
  OrigLoop->getExitingBlocks(Exits);
  setNumExits(Exits.size());
}

// IsInnermost flag is initialized to true, please refer to the header file.
HLLoop::HLLoop(HLIf *ZttIf, RegDDRef *LowerDDRef, RegDDRef *UpperDDRef,
               RegDDRef *StrideDDRef, bool IsDoWh, unsigned NumEx)
    : HLDDNode(HLNode::HLLoopVal), OrigLoop(nullptr), Ztt(nullptr),
      IsDoWhile(IsDoWh), NestingLevel(0), IsInnermost(true) {
  assert((!ZttIf || !IsDoWh) && "Do while loop cannot have ztt!");

  initialize();
  setNumExits(NumEx);

  assert(LowerDDRef && UpperDDRef && StrideDDRef &&
         "All DDRefs should be non null");

  /// Sets ztt properly, with all the ddref setup.
  setZtt(ZttIf);

  setLowerDDRef(LowerDDRef);
  setUpperDDRef(UpperDDRef);
  setStrideDDRef(StrideDDRef);

  assert(
      ((!getLowerDDRef()->containsUndef() &&
        !getUpperDDRef()->containsUndef() &&
        !getStrideDDRef()->containsUndef()) ||
       (getLowerDDRef()->containsUndef() &&
        getUpperDDRef()->containsUndef() &&
        getStrideDDRef()->containsUndef())) &&
        "Lower, Upper and Stride DDRefs "
        "should be all defined or all undefined");
}

HLLoop::HLLoop(const HLLoop &HLLoopObj, GotoContainerTy *GotoList,
               LabelMapTy *LabelMap, bool CloneChildren)
    : HLDDNode(HLLoopObj), OrigLoop(HLLoopObj.OrigLoop), Ztt(nullptr),
      IsDoWhile(HLLoopObj.IsDoWhile), NumExits(HLLoopObj.NumExits),
      NestingLevel(0), IsInnermost(true),
      IVType(HLLoopObj.IVType) {

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

  // Avoid cloning children and preheader/postexit.
  if (!CloneChildren)
    return;

  // Assert is placed here since empty loop cloning will not use it.
  assert(GotoList && " GotoList is null.");
  assert(LabelMap && " LabelMap is null.");

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
    HLNode *NewHLNode = cloneBaseImpl(&*ChildIter, GotoList, LabelMap);
    HLNodeUtils::insertAsLastChild(this, NewHLNode);
  }

  for (auto PostIter = HLLoopObj.post_begin(),
            PostIterEnd = HLLoopObj.post_end();
       PostIter != PostIterEnd; ++PostIter) {
    HLNode *NewHLNode = PostIter->clone();
    HLNodeUtils::insertAsLastPostexitNode(this, NewHLNode);
  }
}

HLLoop *HLLoop::cloneImpl(GotoContainerTy *GotoList,
                          LabelMapTy *LabelMap) const {

  // Call the Copy Constructor
  HLLoop *NewHLLoop = new HLLoop(*this, GotoList, LabelMap, true);

  return NewHLLoop;
}

HLLoop *HLLoop::clone() const {

  HLContainerTy NContainer;
  HLNodeUtils::cloneSequence(&NContainer, this);
  HLLoop *NewLoop = cast<HLLoop>(NContainer.remove(NContainer.begin()));
  return NewLoop;
}

HLLoop *HLLoop::cloneEmptyLoop() const {

  // Call the Copy Constructor
  HLLoop *NewHLLoop = new HLLoop(*this, nullptr, nullptr, false);

  return NewHLLoop;
}

HLLoop *HLLoop::cloneCompleteEmptyLoop() const {

  // Call the Copy Constructor
  HLLoop *NewHLLoop = new HLLoop(*this, nullptr, nullptr, false);
  NewHLLoop->setNestingLevel(getNestingLevel());
  return NewHLLoop;
}

void HLLoop::print(formatted_raw_ostream &OS, unsigned Depth,
                   bool Detailed) const {
  const RegDDRef *Ref;

  /// Print preheader
  for (auto I = pre_begin(), E = pre_end(); I != E; I++) {
    I->print(OS, Depth + 1, false);
  }

  if (Detailed) {
    {
      indent(OS, Depth);
      OS << "+ NumExits: " << getNumExits() << "\n";
    }

    {
      indent(OS, Depth);
      OS << "+ Ztt: ";
      if (hasZtt()) {
        Ztt->printZttHeader(OS, this);
      } else {
        OS << "No";
      }
      OS << "\n";
    }

    {
      indent(OS, Depth);
      OS << "+ Innermost: " << isInnermost() << "\n";
    }
  }

  indent(OS, Depth);
  /// Print header
  if (getUpperDDRef() && (isDo() || isDoWhile() || isDoMultiExit())) {
    OS << "+ DO ";
    if (Detailed) {
      getIVType()->print(OS);
      OS << " ";
    }
    OS << "i" << NestingLevel;

    OS << " = ";
    Ref = getLowerDDRef();
    Ref ? Ref->print(OS, false) : (void)(OS << Ref);
    OS << ", ";
    Ref = getUpperDDRef();
    Ref ? Ref->print(OS, false) : (void)(OS << Ref);
    OS << ", ";
    Ref = getStrideDDRef();
    Ref ? Ref->print(OS, false) : (void)(OS << Ref);

    OS.indent(IndentWidth);
    if (isDo()) {
      OS << "<DO_LOOP>";
    } else if (isDoWhile()) {
      OS << "<DO_WHILE_LOOP>";
    } else if (isDoMultiExit()) {
      OS << "<DO_MULTI_EXIT_LOOP>";
    }

    OS << "\n";
  } else if (!getUpperDDRef() || isUnknown()) {
    OS << "+ UNKNOWN LOOP i" << NestingLevel << "\n";
  } else {
    llvm_unreachable("Unexpected loop type!");
  }

  HLDDNode::print(OS, Depth, Detailed);

  /// Print children
  for (auto I = child_begin(), E = child_end(); I != E; I++) {
    I->print(OS, Depth + 1, Detailed);
  }

  /// Print footer
  indent(OS, Depth);
  OS << "+ END LOOP\n";

  /// Print postexit
  for (auto I = post_begin(), E = post_end(); I != E; I++) {
    I->print(OS, Depth + 1, Detailed);
  }
}

void HLLoop::setNumExits(unsigned NumEx) {
  assert(NumEx && "Number of exits cannot be zero!");
  NumExits = NumEx;
}

unsigned
HLLoop::getZttPredicateOperandDDRefOffset(const_ztt_pred_iterator CPredI,
                                          bool IsLHS) const {
  assert(hasZtt() && "Ztt is absent!");
  return (getNumLoopDDRefs() +
          Ztt->getPredicateOperandDDRefOffset(CPredI, IsLHS));
}

void HLLoop::addZttPredicate(PredicateTy Pred, RegDDRef *Ref1, RegDDRef *Ref2) {
  assert(hasZtt() && "Ztt is absent!");
  Ztt->addPredicate(Pred, Ref1, Ref2);

  const_ztt_pred_iterator LastIt = std::prev(ztt_pred_end());

  /// Move the RegDDRefs to loop.
  setZttPredicateOperandDDRef(Ztt->removePredicateOperandDDRef(LastIt, true),
                              LastIt, true);
  setZttPredicateOperandDDRef(Ztt->removePredicateOperandDDRef(LastIt, false),
                              LastIt, false);
}

void HLLoop::removeZttPredicate(const_ztt_pred_iterator CPredI) {
  assert(hasZtt() && "Ztt is absent!");

  // Remove RegDDRefs from loop.
  removeZttPredicateOperandDDRef(CPredI, true);
  removeZttPredicateOperandDDRef(CPredI, false);

  // Erase the DDRef slots from loop.
  // Since erasing from the vector leads to shifting of elements, it is better
  // to erase in reverse order.
  RegDDRefs.erase(RegDDRefs.begin() +
                  getZttPredicateOperandDDRefOffset(CPredI, false));
  RegDDRefs.erase(RegDDRefs.begin() +
                  getZttPredicateOperandDDRefOffset(CPredI, true));

  // Remove predicate from ztt.
  Ztt->removePredicate(CPredI);
}

void HLLoop::replaceZttPredicate(const_ztt_pred_iterator CPredI,
                                 PredicateTy NewPred) {
  assert(hasZtt() && "Ztt is absent!");
  Ztt->replacePredicate(CPredI, NewPred);
}

RegDDRef *HLLoop::getZttPredicateOperandDDRef(const_ztt_pred_iterator CPredI,
                                              bool IsLHS) const {
  assert(hasZtt() && "Ztt is absent!");
  return getOperandDDRefImpl(getZttPredicateOperandDDRefOffset(CPredI, IsLHS));
}

void HLLoop::setZttPredicateOperandDDRef(RegDDRef *Ref,
                                         const_ztt_pred_iterator CPredI,
                                         bool IsLHS) {
  assert(hasZtt() && "Ztt is absent!");
  setOperandDDRefImpl(Ref, getZttPredicateOperandDDRefOffset(CPredI, IsLHS));
}

RegDDRef *HLLoop::removeZttPredicateOperandDDRef(const_ztt_pred_iterator CPredI,
                                                 bool IsLHS) {
  assert(hasZtt() && "Ztt is absent!");
  auto TRef = getZttPredicateOperandDDRef(CPredI, IsLHS);

  if (TRef) {
    setZttPredicateOperandDDRef(nullptr, CPredI, IsLHS);
  }

  return TRef;
}

bool HLLoop::isZttOperandDDRef(const RegDDRef *Ref) const {
  assert(Ref->getHLDDNode() && (cast<HLLoop>(Ref->getHLDDNode()) == this) &&
         "Ref does not belong to this loop!");

  auto It = std::find(ztt_ddref_begin(), ztt_ddref_end(), Ref);

  return (It != ztt_ddref_end());
}

RegDDRef *HLLoop::getLowerDDRef() { return getOperandDDRefImpl(0); }

const RegDDRef *HLLoop::getLowerDDRef() const {
  return const_cast<HLLoop *>(this)->getLowerDDRef();
}

void HLLoop::setLowerDDRef(RegDDRef *Ref) {
  assert((!Ref || Ref->isScalarRef()) && "Invalid LowerDDRef!");

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
  assert((!Ref || Ref->isScalarRef()) && "Invalid UpperDDRef!");

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
  assert((!Ref || Ref->isScalarRef()) && "Invalid StrideDDRef!");

  setOperandDDRefImpl(Ref, 2);
}

void HLLoop::setLLVMLoop(const Loop *LLVMLoop) { OrigLoop = LLVMLoop; }

RegDDRef *HLLoop::removeStrideDDRef() {
  auto TRef = getStrideDDRef();

  if (TRef) {
    setStrideDDRef(nullptr);
  }

  return TRef;
}

const Loop *HLLoop::removeLLVMLoop() {
  auto OrigLoop = getLLVMLoop();

  if (OrigLoop) {
    setLLVMLoop(nullptr);
  }

  return OrigLoop;
}

void HLLoop::setZtt(HLIf *ZttIf) {
  assert(!hasZtt() && "Attempt to overwrite ztt, use removeZtt instead!");

  if (!ZttIf) {
    return;
  }

  assert((!ZttIf->hasThenChildren() && !ZttIf->hasElseChildren()) &&
         "Ztt cannot have any children!");

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
  return const_cast<HLLoop *>(this)
      ->getLoopCanonExpr(const_cast<RegDDRef *>(Ref));
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

CanonExpr *HLLoop::getTripCountCanonExpr() const {

  if (isUnknown() || !getStrideDDRef()->isIntConstant())
    return nullptr;

  // UB and LB should be scalar refs in the current design.
  assert(getUpperDDRef()->isScalarRef() && getLowerDDRef()->isScalarRef() &&
         " Upper Bound and Lower Bound should be terminal or constant.");

  CanonExpr *Result = nullptr;
  const CanonExpr *UBCE = getUpperCanonExpr();
  // For normalized loop, TC = (UB+1).
  if (isNormalized()) {
    Result = UBCE->clone();
    Result->addConstant(1);
    return Result;
  }

  // TripCount Canon Expr = (UB-LB+Stride)/Stride;
  int64_t StrideConst = getStrideCanonExpr()->getConstant();
  Result = CanonExprUtils::cloneAndSubtract(UBCE, getLowerCanonExpr());
  Result->addConstant(StrideConst);
  Result->multiplyDenominator(StrideConst, true);
  return Result;
}

void HLLoop::updateTripCountBlobDDRefs(RegDDRef *TripRef) const {
  SmallVector<BlobDDRef *, 6> NewBlobRefs;

  TripRef->updateBlobDDRefs(NewBlobRefs);

  // No new blobs to update level for.
  if (NewBlobRefs.empty()) {
    return;
  }

  for (auto &I : NewBlobRefs) {
    unsigned Index = I->getBlobIndex();
    int Level = 0;

    if (getUpperDDRef()->findBlobLevel(Index, &Level) ||
        getLowerDDRef()->findBlobLevel(Index, &Level) ||
        getStrideDDRef()->findBlobLevel(Index, &Level)) {
      (Level == -1) ? I->setNonLinear() : I->setDefinedAtLevel(Level);
    } else {
      llvm_unreachable("Trip count blob not found in lower or stride DDRef!");
    }
  }
}

RegDDRef *HLLoop::getTripCountDDRef() const {

  CanonExpr *TripCE = getTripCountCanonExpr();
  RegDDRef *TripRef =
      DDRefUtils::createScalarRegDDRef(getUpperDDRef()->getSymbase(), TripCE);

  updateTripCountBlobDDRefs(TripRef);

  return TripRef;
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
    return &*pre_begin();
  }

  return nullptr;
}

HLNode *HLLoop::getLastPreheaderNode() {
  if (hasPreheader()) {
    return &*(std::prev(pre_end()));
  }

  return nullptr;
}

HLNode *HLLoop::getFirstPostexitNode() {
  if (hasPostexit()) {
    return &*post_begin();
  }

  return nullptr;
}

HLNode *HLLoop::getLastPostexitNode() {
  if (hasPostexit()) {
    return &*(std::prev(post_end()));
  }

  return nullptr;
}

HLNode *HLLoop::getFirstChild() {
  if (hasChildren()) {
    return &*child_begin();
  }

  return nullptr;
}

HLNode *HLLoop::getLastChild() {
  if (hasChildren()) {
    return &*(std::prev(child_end()));
  }

  return nullptr;
}

bool HLLoop::isNormalized() const {
  if (isUnknown()) {
    return false;
  }

  int64_t LBConst = 0, StepConst = 0;

  if (!getLowerDDRef()->isIntConstant(&LBConst) ||
      !getStrideDDRef()->isIntConstant(&StepConst)) {
    return false;
  }

  if (LBConst != 0 || StepConst != 1) {
    return false;
  }

  return true;
}

bool HLLoop::isConstTripLoop(int64_t *TripCnt) const {

  bool RetVal = false;
  CanonExpr *TripCExpr = getTripCountCanonExpr();
  if (TripCExpr && TripCExpr->isIntConstant(TripCnt)) {
    assert((!TripCnt || (*TripCnt != 0)) && " Zero Trip Loop found.");
    RetVal = true;
  }

  // Free the canon expr.
  if (TripCExpr) {
    CanonExprUtils::destroy(TripCExpr);
  }

  return RetVal;
}

// This will create the Ztt for the loop.
void HLLoop::createZtt(bool IsOverwrite) {

  assert((!hasZtt() || IsOverwrite) && "Overwriting existing Ztt.");
  // Don't generate Ztt for Const trip loops.
  RegDDRef *TripRef = getTripCountDDRef();
  assert(TripRef && " Trip Count DDRef is null.");
  if (TripRef->getSingleCanonExpr()->isIntConstant()) {
    DDRefUtils::destroy(TripRef);
    return;
  }

  // (Trip > 0)
  RegDDRef *ZeroDD = DDRefUtils::createConstDDRef(TripRef->getDestType(), 0);
  HLIf *ZttIf = HLNodeUtils::createHLIf(CmpInst::ICMP_UGT, TripRef, ZeroDD);
  setZtt(ZttIf);
}

void HLLoop::verify() const {
  HLDDNode::verify();

  assert(
      ((!getLowerDDRef()->containsUndef() &&
        !getUpperDDRef()->containsUndef() &&
        !getStrideDDRef()->containsUndef()) ||
       (getLowerDDRef()->containsUndef() && getUpperDDRef()->containsUndef() &&
        getStrideDDRef()->containsUndef())) &&
      "Lower, Upper and Stride DDRefs "
      "should be all defined or all undefined");

  assert(!getLowerDDRef()->getSingleCanonExpr()->isNonLinear() &&
         "Loop lower cannot be non-linear!");
  assert(!getUpperDDRef()->getSingleCanonExpr()->isNonLinear() &&
         "Loop upper cannot be non-linear!");
  assert(!getStrideDDRef()->getSingleCanonExpr()->isNonLinear() &&
         "Loop stride cannot be non-linear!");

  assert(getUpperDDRef()->getSrcType()->isIntegerTy() &&
         "Invalid loop upper type!");

  // TODO: Implement special case as ZTT's DDRefs are attached to node
  // if (Ztt) {
  //  Ztt->verify();
  //}

  for (auto I = pre_begin(), E = pre_end(); I != E; ++I) {
    assert(isa<HLInst>(*I) && "All nodes in preheader must be HLInst");
  }

  for (auto I = post_begin(), E = post_end(); I != E; ++I) {
    assert(isa<HLInst>(*I) && "All nodes in postexit must be HLInst");
  }

  assert((!getParentLoop() ||
          (getNestingLevel() == getParentLoop()->getNestingLevel() + 1)) &&
         "If it's not a top-level loop its nesting level should be +1");
  assert((getParentLoop() || getNestingLevel() == 1) &&
         "Top level loops should have 1st nesting level");
}

bool HLLoop::isSIMD() const {
  HLContainerTy::const_iterator Iter(*this);
  auto First = HLNodeUtils::getFirstLexicalChild(getParent(), this); 
  HLContainerTy::const_iterator FIter(*First);

  while (Iter != FIter) {
    --Iter;
    const HLInst *I = dyn_cast<HLInst>(Iter);
    if (!I)
      return false; // Loop, IF, Switch, etc.
    Intrinsic::ID IntrinID;
    if (!I->isIntrinCall(IntrinID) ||
        !vpo::VPOUtils::isIntelDirectiveOrClause(IntrinID))
      return false; // Expecting just directives and clauses between SIMD
                    // and Loop.
    if (I->isSIMDDirective())
      return true;
  }
  
  return false;
}

