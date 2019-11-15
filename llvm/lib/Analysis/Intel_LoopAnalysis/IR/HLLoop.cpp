//===-------- HLLoop.cpp - Implements the HLLoop class --------------------===//
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
// This file implements the HLLoop class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLLoop.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/CanonExprUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/ForEach.h"
#include "llvm/Analysis/Intel_OptReport/LoopOptReportPrintUtils.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

// Next Option is used for performance headroom finding and stress testing
static cl::opt<bool> AssumeIVDEPInnermostLoop(
    "hir-assume-ivdep-innermost-loop", cl::init(false), cl::Hidden,
    cl::desc("Assumes IVDEP is on for innermost loop"));

#define DEBUG_NORMALIZE(X) DEBUG_WITH_TYPE("hir-loop-normalize", X)

llvm::Statistic LoopsNormalized = {"hir-loop-normalize", "LoopsNormalized",
                                   "Loops normalized On-Demand"};

void HLLoop::initialize() {
  unsigned NumOp;

  ChildBegin = Children.end();
  PostexitBegin = Children.end();

  /// This call is to get around calling virtual functions in the constructor.
  NumOp = getNumOperandsInternal();

  RegDDRefs.resize(NumOp, nullptr);
}

// IsInnermost flag is initialized to true, please refer to the header file.
HLLoop::HLLoop(HLNodeUtils &HNU, const Loop *LLVMLoop)
    : HLDDNode(HNU, HLNode::HLLoopVal), OrigLoop(LLVMLoop), Ztt(nullptr),
      NestingLevel(0), IsInnermost(true), IVType(nullptr), IsNSW(false),
      DistributedForMemRec(false), LoopMetadata(LLVMLoop->getLoopID()),
      MaxTripCountEstimate(0), MaxTCIsUsefulForDD(false),
      HasDistributePoint(false), IsUndoSinkingCandidate(false) {
  assert(LLVMLoop && "LLVM loop cannot be null!");

  initialize();

  SmallVector<Loop::Edge, 8> ExitEdges;

  OrigLoop->getExitEdges(ExitEdges);
  setNumExits(ExitEdges.size());
  // If Lp has attached optreport metadata node - initialize HLoop
  // optreport with it. Otherwise it will initialize it with zero.
  // We also don't erase the opt report from LoopID. We only do that
  // at the HIRCodeGen stage, if needed.
  setOptReport(LoopOptReport::findOptReportInLoopID(LLVMLoop->getLoopID()));
}

// IsInnermost flag is initialized to true, please refer to the header file.
HLLoop::HLLoop(HLNodeUtils &HNU, HLIf *ZttIf, RegDDRef *LowerDDRef,
               RegDDRef *UpperDDRef, RegDDRef *StrideDDRef, unsigned NumEx)
    : HLDDNode(HNU, HLNode::HLLoopVal), OrigLoop(nullptr), Ztt(nullptr),
      NestingLevel(0), IsInnermost(true), IsNSW(false),
      DistributedForMemRec(false), LoopMetadata(nullptr),
      MaxTripCountEstimate(0), MaxTCIsUsefulForDD(false),
      HasDistributePoint(false), IsUndoSinkingCandidate(false) {
  initialize();
  setNumExits(NumEx);

  assert(LowerDDRef && UpperDDRef && StrideDDRef &&
         "All DDRefs should be non null");

  /// Sets ztt properly, with all the ddref setup.
  setZtt(ZttIf);

  setLowerDDRef(LowerDDRef);
  setUpperDDRef(UpperDDRef);
  setStrideDDRef(StrideDDRef);

  setIVType(LowerDDRef->getDestType());
}

HLLoop::HLLoop(const HLLoop &HLLoopObj)
    : HLDDNode(HLLoopObj), OrigLoop(HLLoopObj.OrigLoop), Ztt(nullptr),
      NumExits(HLLoopObj.NumExits), NestingLevel(0), IsInnermost(true),
      IVType(HLLoopObj.IVType), IsNSW(HLLoopObj.IsNSW),
      LiveInSet(HLLoopObj.LiveInSet), LiveOutSet(HLLoopObj.LiveOutSet),
      DistributedForMemRec(HLLoopObj.DistributedForMemRec),
      LoopMetadata(HLLoopObj.LoopMetadata),
      MaxTripCountEstimate(HLLoopObj.MaxTripCountEstimate),
      MaxTCIsUsefulForDD(HLLoopObj.MaxTCIsUsefulForDD),
      CmpDbgLoc(HLLoopObj.CmpDbgLoc), BranchDbgLoc(HLLoopObj.BranchDbgLoc),
      HasDistributePoint(HLLoopObj.HasDistributePoint),
      IsUndoSinkingCandidate(HLLoopObj.IsUndoSinkingCandidate) {

  initialize();

  /// Clone the Ztt
  if (HLLoopObj.hasZtt()) {
    setZtt(HLLoopObj.Ztt->clone());

    auto ZttRefIt = HLLoopObj.ztt_ddref_begin();

    for (auto ZIt = ztt_pred_begin(), EZIt = ztt_pred_end(); ZIt != EZIt;
         ++ZIt) {
      setZttPredicateOperandDDRef((*ZttRefIt)->clone(), ZIt, true);
      ++ZttRefIt;
      setZttPredicateOperandDDRef((*ZttRefIt)->clone(), ZIt, false);
      ++ZttRefIt;
    }
  }

  /// Clone loop RegDDRefs
  setLowerDDRef(HLLoopObj.getLowerDDRef()->clone());
  setUpperDDRef(HLLoopObj.getUpperDDRef(true)->clone());
  setStrideDDRef(HLLoopObj.getStrideDDRef()->clone());
}

HLLoop &HLLoop::operator=(HLLoop &&Lp) {
  OrigLoop = Lp.OrigLoop;
  IVType = Lp.IVType;
  IsNSW = Lp.IsNSW;
  DistributedForMemRec = Lp.DistributedForMemRec;
  LoopMetadata = Lp.LoopMetadata;
  MaxTripCountEstimate = Lp.MaxTripCountEstimate;
  MaxTCIsUsefulForDD = Lp.MaxTCIsUsefulForDD;
  HasDistributePoint = Lp.HasDistributePoint;
  IsUndoSinkingCandidate = Lp.IsUndoSinkingCandidate;

  // LiveInSet/LiveOutSet do not need to be moved as they depend on the lexical
  // order of HLLoops which remains the same as before.

  removeZtt();

  if (Lp.hasZtt()) {
    setZtt(Lp.removeZtt());
  }

  setLowerDDRef(Lp.removeLowerDDRef());
  setUpperDDRef(Lp.removeUpperDDRef());
  setStrideDDRef(Lp.removeStrideDDRef());

  return *this;
}

HLLoop *HLLoop::cloneImpl(GotoContainerTy *GotoList, LabelMapTy *LabelMap,
                          HLNodeMapper *NodeMapper) const {

  HLLoop *NewHLLoop = cloneEmpty();

  // Assert is placed here since empty loop cloning will not use it.
  assert(GotoList && " GotoList is null.");
  assert(LabelMap && " LabelMap is null.");

  /// Loop over children, preheader and postexit
  for (auto PreIter = this->pre_begin(), PreIterEnd = this->pre_end();
       PreIter != PreIterEnd; ++PreIter) {
    HLNode *NewHLNode = cloneBaseImpl(&*PreIter, nullptr, nullptr, NodeMapper);
    HLNodeUtils::insertAsLastPreheaderNode(NewHLLoop, NewHLNode);
  }

  // Clone the children.
  // The goto target label's will not be updated and would be done by caller.
  for (auto ChildIter = this->child_begin(), ChildIterEnd = this->child_end();
       ChildIter != ChildIterEnd; ++ChildIter) {
    HLNode *NewHLNode =
        cloneBaseImpl(&*ChildIter, GotoList, LabelMap, NodeMapper);
    HLNodeUtils::insertAsLastChild(NewHLLoop, NewHLNode);
  }

  for (auto PostIter = this->post_begin(), PostIterEnd = this->post_end();
       PostIter != PostIterEnd; ++PostIter) {
    HLNode *NewHLNode = cloneBaseImpl(&*PostIter, nullptr, nullptr, NodeMapper);
    HLNodeUtils::insertAsLastPostexitNode(NewHLLoop, NewHLNode);
  }

  return NewHLLoop;
}

HLLoop *HLLoop::clone(HLNodeMapper *NodeMapper) const {
  return cast<HLLoop>(HLNode::clone(NodeMapper));
}

HLLoop *HLLoop::cloneEmpty() const {
  // Call the Copy Constructor
  return new HLLoop(*this);
}

void HLLoop::printPreheader(formatted_raw_ostream &OS, unsigned Depth,
                            bool Detailed) const {
#if !INTEL_PRODUCT_RELEASE
  auto Parent = getParent();

  // If a previous node exists, add a newline.
  if (Parent && (this != getHLNodeUtils().getFirstLexicalChild(Parent, this))) {
    indent(OS, Depth);
    OS << "\n";
  }

  for (auto I = pre_begin(), E = pre_end(); I != E; I++) {
    I->print(OS, Depth + 1, Detailed);
  }
#endif // !INTEL_PRODUCT_RELEASE
}

void HLLoop::printDetails(formatted_raw_ostream &OS, unsigned Depth,
                          bool Detailed) const {
#if !INTEL_PRODUCT_RELEASE

  if (!Detailed) {
    return;
  }

  indent(OS, Depth);

  OS << "+ Ztt: ";

  if (hasZtt()) {
    Ztt->printZttHeader(OS, this);
  } else {
    OS << "No";
  }
  OS << "\n";

  indent(OS, Depth);
  OS << "+ NumExits: " << getNumExits() << "\n";

  indent(OS, Depth);
  OS << "+ Innermost: " << (isInnermost() ? "Yes\n" : "No\n");

  indent(OS, Depth);
  OS << "+ NSW: " << (isNSW() ? "Yes\n" : "No\n");

  bool First = true;

  indent(OS, Depth);
  OS << "+ LiveIn symbases: ";
  for (auto I = live_in_begin(), E = live_in_end(); I != E; ++I) {
    if (!First) {
      OS << ", ";
    }
    OS << *I;
    First = false;
  }

  OS << "\n";

  First = true;

  indent(OS, Depth);
  OS << "+ LiveOut symbases: ";
  for (auto I = live_out_begin(), E = live_out_end(); I != E; ++I) {
    if (!First) {
      OS << ", ";
    }
    OS << *I;
    First = false;
  }

  OS << "\n";

  indent(OS, Depth);
  OS << "+ Loop metadata:";
  if (auto Node = getLoopMetadata()) {
    RegDDRef::MDNodesTy Nodes = {
        RegDDRef::MDPairTy{LLVMContext::MD_loop, Node}};
    getDDRefUtils().printMDNodes(OS, Nodes);
  } else {
    OS << " No";
  }
  OS << "\n";
#endif // INTEL_PRODUCT_RELEASE
}

void HLLoop::printDirectives(formatted_raw_ostream &OS, unsigned Depth) const {
  if (ParTraits != nullptr)
    OS << " <parallel>";

  if (isAttached() && isSIMD()) {
    OS << " <simd>";
  }

  // Some of the pragma checks require trip count information,
  // so we skip them if it isn't present yet.
  if (!getStrideDDRef()) {
    return;
  }

  unsigned Count = getUnrollPragmaCount();
  if (hasUnrollEnablingPragma()) {
    OS << " <unroll";
    if (Count) {
      OS << " = " << Count;
    }
    OS << ">";
  } else if (hasUnrollDisablingPragma()) {
    OS << " <nounroll>";
  }

  Count = getUnrollAndJamPragmaCount();
  if (hasUnrollAndJamEnablingPragma()) {
    OS << " <unroll and jam";
    if (Count) {
      OS << " = " << Count;
    }
    OS << ">";
  } else if (hasUnrollAndJamDisablingPragma()) {
    OS << " <nounroll and jam>";
  }

  Count = getVectorizePragmaWidth();
  if (hasVectorizeEnablingPragma()) {
    OS << " <vectorize";
    if (Count) {
      OS << " = " << Count;
    }
    OS << ">";
  } else if (hasVectorizeDisablingPragma()) {
    OS << " <novectorize>";
  }

  if (hasVectorizeIVDepLoopPragma() || hasVectorizeIVDepBackPragma()) {
    OS << " <ivdep>";
  }

  if (getPragmaBasedMinimumTripCount(Count)) {
    OS << " <min_trip_count = " << Count << ">";
  }

  if (getPragmaBasedAverageTripCount(Count)) {
    OS << " <avg_trip_count = " << Count << ">";
  }

  if (getPragmaBasedMaximumTripCount(Count)) {
    OS << " <max_trip_count = " << Count << ">";
  }

  if (hasFusionEnablingPragma()) {
    OS << " <force fusion>";
  }

  if (hasFusionDisablingPragma()) {
    OS << " <no fusion>";
  }

  SmallVector<unsigned, 4> TripCounts;

  if (getPragmaBasedLikelyTripCounts(TripCounts)) {
    OS << " <trip_counts = ";

    bool FirstTC = true;
    for (auto TC : TripCounts) {
      if (!FirstTC) {
        OS << ", ";
      }
      OS << TC;
      FirstTC = false;
    }
    OS << ">";
  }
}

void HLLoop::printHeader(formatted_raw_ostream &OS, unsigned Depth,
                         bool Detailed) const {
#if !INTEL_PRODUCT_RELEASE
  const RegDDRef *Ref;

  printDetails(OS, Depth, Detailed);

  indent(OS, Depth);

  if (isDo() || isDoMultiExit()) {
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
    } else {
      OS << "<DO_MULTI_EXIT_LOOP>";
    }

  } else if (isUnknown()) {
    OS << "+ UNKNOWN LOOP i" << NestingLevel;
  } else {
    llvm_unreachable("Unexpected loop type!");
  }

  if (MaxTripCountEstimate) {
    OS << "  <MAX_TC_EST = " << MaxTripCountEstimate << ">";
  }

  if (getMVTag()) {
    OS << "  <MVTag: " << getMVTag() << ">";
  }

  printDistributePoint(OS);
  printDirectives(OS, Depth);

  OS << "\n";

  HLDDNode::print(OS, Depth, Detailed);
#endif // !INTEL_PRODUCT_RELEASE
}

void HLLoop::printBody(formatted_raw_ostream &OS, unsigned Depth,
                       bool Detailed) const {
#if !INTEL_PRODUCT_RELEASE

  for (auto I = child_begin(), E = child_end(); I != E; I++) {
    I->print(OS, Depth + 1, Detailed);
  }
#endif // !INTEL_PRODUCT_RELEASE
}

void HLLoop::printFooter(formatted_raw_ostream &OS, unsigned Depth) const {
#if !INTEL_PRODUCT_RELEASE
  indent(OS, Depth);
  OS << "+ END LOOP\n";
#endif // !INTEL_PRODUCT_RELEASE
}

void HLLoop::printPostexit(formatted_raw_ostream &OS, unsigned Depth,
                           bool Detailed) const {
#if !INTEL_PRODUCT_RELEASE

  for (auto I = post_begin(), E = post_end(); I != E; I++) {
    I->print(OS, Depth + 1, Detailed);
  }

  auto Parent = getParent();

  // If a next node exists, add a newline.
  if (Parent && (this != getHLNodeUtils().getLastLexicalChild(Parent, this))) {
    indent(OS, Depth);
    OS << "\n";
  }
#endif // !INTEL_PRODUCT_RELEASE
}

void HLLoop::print(formatted_raw_ostream &OS, unsigned Depth,
                   bool Detailed) const {
#if !INTEL_PRODUCT_RELEASE

  printPreheader(OS, Depth, Detailed);

  printHeader(OS, Depth, Detailed);

  printBody(OS, Depth, Detailed);

  printFooter(OS, Depth);

  printPostexit(OS, Depth, Detailed);
#endif // !INTEL_PRODUCT_RELEASE
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void HLLoop::dumpOptReport() const {
#if !INTEL_PRODUCT_RELEASE
  formatted_raw_ostream OS(dbgs());
  LoopOptReport OptReport = getOptReport();

  OptReportUtils::printLoopHeaderAndOrigin(OS, 0, OptReport, getDebugLoc());

  if (OptReport)
    OptReportUtils::printOptReport(OS, 0, OptReport);

  OptReportUtils::printLoopFooter(OS, 0);

  if (OptReport && OptReport.nextSibling())
    OptReportUtils::printEnclosedOptReport(OS, 0, OptReport.nextSibling());
#endif // !INTEL_PRODUCT_RELEASE
}
#endif

unsigned
HLLoop::getZttPredicateOperandDDRefOffset(const_ztt_pred_iterator CPredI,
                                          bool IsLHS) const {
  assert(hasZtt() && "Ztt is absent!");
  return (getNumLoopDDRefs() +
          Ztt->getPredicateOperandDDRefOffset(CPredI, IsLHS));
}

void HLLoop::addZttPredicate(const HLPredicate &Pred, RegDDRef *Ref1,
                             RegDDRef *Ref2) {
  assert(hasZtt() && "Ztt is absent!");
  Ztt->addPredicate(Pred, Ref1, Ref2);

  const_ztt_pred_iterator LastIt = std::prev(ztt_pred_end());

  RegDDRefs.resize(getNumOperandsInternal(), nullptr);

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
                                 const HLPredicate &NewPred) {
  assert(hasZtt() && "Ztt is absent!");
  Ztt->replacePredicate(CPredI, NewPred);
}

void HLLoop::replaceZttPredicate(const_ztt_pred_iterator CPredI,
                                 PredicateTy NewPred) {
  assert(hasZtt() && "Ztt is absent!");
  Ztt->replacePredicate(CPredI, NewPred);
}

void HLLoop::invertZttPredicate(const_ztt_pred_iterator CPredI) {
  assert(hasZtt() && "Ztt is absent!");
  Ztt->invertPredicate(CPredI);
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

RegDDRef *HLLoop::removeLowerDDRef() {
  auto TRef = getLowerDDRef();

  if (TRef) {
    setLowerDDRef(nullptr);
  }

  return TRef;
}

RegDDRef *HLLoop::removeUpperDDRef() {
  auto TRef = getUpperDDRef();

  if (TRef) {
    setUpperDDRef(nullptr);
  }

  return TRef;
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
         "Ztt cannot have any children!");

  Ztt = ZttIf;
  Ztt->setParent(this);

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

  if (!hasZtt()) {
    return nullptr;
  }

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

CanonExpr *HLLoop::getTripCountCanonExpr() const {
  if (isUnknown()) {
    return nullptr;
  }

  CanonExpr *Result = nullptr;
  const CanonExpr *UBCE = getUpperCanonExpr();
  // For normalized loop, TC = (UB+1).
  if (isNormalized()) {
    Result = UBCE->clone();
    Result->addConstant(1, true);
    return Result;
  }

  // TripCount Canon Expr = (UB-LB+Stride)/Stride;
  int64_t StrideConst = getStrideCanonExpr()->getConstant();
  Result = getCanonExprUtils().cloneAndSubtract(UBCE, getLowerCanonExpr());
  assert(Result && " Trip Count computation failed.");

  Result->divide(StrideConst);
  Result->addConstant(StrideConst, true);
  Result->simplify(true, true);
  return Result;
}

RegDDRef *HLLoop::getTripCountDDRef(unsigned NestingLevel) const {
  SmallVector<const RegDDRef *, 4> LoopRefs;

  CanonExpr *TripCE = getTripCountCanonExpr();
  if (!TripCE) {
    return nullptr;
  }

  RegDDRef *TripRef = getDDRefUtils().createScalarRegDDRef(
      getUpperDDRef()->getSymbase(), TripCE);

  LoopRefs.push_back(getLowerDDRef());
  LoopRefs.push_back(getStrideDDRef());
  LoopRefs.push_back(getUpperDDRef());

  // Default argument case.
  if ((MaxLoopNestLevel + 1) == NestingLevel) {
    NestingLevel = getNestingLevel() - 1;
  }

  TripRef->makeConsistent(LoopRefs, NestingLevel);

  return TripRef;
}

unsigned HLLoop::getNumZttOperands() const {
  if (hasZtt()) {
    return Ztt->getNumOperands();
  }

  return 0;
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
    // Unknown loop is always normalized.
    return true;
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

bool HLLoop::isKnownZttPredicate(bool *IsTrue) const {
  if (!hasZtt()) {
    return false;
  }

  if (HLNodeUtils::isKnownPredicateRange(
          ztt_pred_begin(), ztt_pred_end(),
          std::bind(&HLLoop::getZttPredicateOperandDDRef, this,
                    std::placeholders::_1, std::placeholders::_2),
          IsTrue)) {
    return true;
  }

  return false;
}

bool HLLoop::isConstTripLoop(uint64_t *TripCnt) const {
  if (isUnknown()) {
    return false;
  }

  bool ConstantTripLoop = false;
  int64_t TC;

  if (isNormalized()) {
    const CanonExpr *UpperBound = getUpperCanonExpr();

    if (UpperBound->isIntConstant(&TC)) {
      ConstantTripLoop = true;
      TC += 1;
    }
  } else {
    if (CanonExprUtils::getConstDistance(getUpperCanonExpr(),
                                         getLowerCanonExpr(), &TC)) {
      TC /= getStrideCanonExpr()->getConstant();
      TC += 1;

      ConstantTripLoop = true;
    }
  }

  assert((!ConstantTripLoop || (TC != 0)) && " Zero Trip Loop found!");

  if (ConstantTripLoop && TripCnt) {
    // This signed to unsigned conversion should be safe as all the negative
    // trip counts which fit in signed 64 bits have been converted to postive
    // integers by parser. Reinterpreting negative signed 64 values (which are
    // outside the range) as an unsigned 64 bit value is correct for trip
    // counts.
    *TripCnt = TC;
  }

  return ConstantTripLoop;
}

void HLLoop::createZtt(RegDDRef *LHS, PredicateTy Pred, RegDDRef *RHS,
                       bool IsOverwrite) {
  assert((!hasZtt() || IsOverwrite) && "Overwriting existing Ztt.");

  // Don't generate Ztt for Const trip loops.
  if (isConstTripLoop()) {
    return;
  }

  setZtt(getHLNodeUtils().createHLIf(Pred, LHS, RHS));
}

// This will create the Ztt for the loop.
void HLLoop::createZtt(bool IsOverwrite, bool IsSigned) {

  assert((!hasZtt() || IsOverwrite) && "Overwriting existing Ztt.");

  if (hasZtt()) {
    removeZtt();
  }

  // Don't generate Ztt for Const trip loops.
  if (isConstTripLoop()) {
    return;
  }

  // Trip > 0
  RegDDRef *LBRef = getLowerDDRef()->clone();
  RegDDRef *UBRef = getUpperDDRef()->clone();

  // The ZTT will look like [ LB < UB + 1 ]. This form is the safest one as UB
  // can not be MAX_VALUE and it's safe to add 1. Transformations are free to do
  // UB - 1.
  UBRef->getSingleCanonExpr()->addConstant(1, true);

  HLIf *ZttIf = getHLNodeUtils().createHLIf(
      IsSigned ? PredicateTy::ICMP_SLT : PredicateTy::ICMP_ULT, LBRef, UBRef);
  setZtt(ZttIf);

  // The following call is required because self-blobs do not have BlobDDRefs.
  // +1 operation could make non-self blob a self-blob and wise versa.
  // For example if UB is (%b - 1) or (%b).
  SmallVector<const RegDDRef *, 1> Aux = {getUpperDDRef()};
  UBRef->makeConsistent(Aux, getNestingLevel());
}

HLIf *HLLoop::extractZtt(unsigned NewLevel) {
  // Default value of NewLevel is NonLinearLevel.

  if (!hasZtt()) {
    return nullptr;
  }

  HLIf *Ztt = removeZtt();

  HLNodeUtils::insertBefore(this, Ztt);
  HLNodeUtils::moveAsFirstThenChild(Ztt, this);

  if (NewLevel == NonLinearLevel) {
    NewLevel = getNestingLevel() - 1;
  }

  assert(CanonExprUtils::isValidLinearDefLevel(NewLevel) &&
         "Invalid nesting level.");

  std::for_each(Ztt->ddref_begin(), Ztt->ddref_end(),
                [NewLevel](RegDDRef *Ref) { Ref->updateDefLevel(NewLevel); });

  return Ztt;
}

void HLLoop::extractPreheader() {

  if (!hasPreheader()) {
    return;
  }

  extractZtt();

  HLNodeUtils::moveBefore(this, pre_begin(), pre_end());
}

void HLLoop::extractPostexit() {

  if (!hasPostexit()) {
    return;
  }

  extractZtt();

  HLNodeUtils::moveAfter(this, post_begin(), post_end());
}

void HLLoop::extractPreheaderAndPostexit() {
  extractPreheader();
  extractPostexit();
}

void HLLoop::removePreheader() { HLNodeUtils::remove(pre_begin(), pre_end()); }

void HLLoop::removePostexit() { HLNodeUtils::remove(post_begin(), post_end()); }

static void demoteBlobs(RegDDRef *Ref, unsigned Level) {
  if (Ref->isSelfBlob()) {
    unsigned DefLevel = Ref->getSingleCanonExpr()->getDefinedAtLevel();

    if (DefLevel != NonLinearLevel && DefLevel >= Level) {
      Ref->getSingleCanonExpr()->setDefinedAtLevel(DefLevel - 1);
    }

  } else {
    for (auto *BRef : make_range(Ref->blob_begin(), Ref->blob_end())) {
      auto BlobLevel = BRef->getDefinedAtLevel();

      if (BlobLevel != NonLinearLevel && BlobLevel >= Level) {
        BRef->setDefinedAtLevel(BlobLevel - 1);
      }
    }
  }
}

// Helper for replaceByFirstIteration().
// \p Ref lies in \p ReplaceLp which is going is being replaced by its body.
// \p MergedRef substituted ReplaceLp's IV in Ref.
// \p InnerLoops is the sets of inner loops of ReplaceLp which have been
// visited/processed.
static unsigned demoteRef(RegDDRef *Ref, const HLLoop *ReplaceLp,
                          const RegDDRef *MergedRef,
                          SmallPtrSet<HLLoop *, 8> &InnerLoops) {

  // Nothing special needs to be done for innermost loops.
  if (ReplaceLp->isInnermost()) {
    return ReplaceLp->getNestingLevel();
  }

  auto *Node = Ref->getHLDDNode();
  auto *ParentLp = dyn_cast<HLLoop>(Node);

  if (!ParentLp) {
    ParentLp = Node->getLexicalParentLoop();
  }

  if (ParentLp != ReplaceLp) {
    unsigned Level = ReplaceLp->getNestingLevel();

    // Demote IVs for all inner loop levels.
    Ref->demoteIVs(Level + 1);

    // When we replace outer loop by first iteration, the definition level
    // of blobs defined inside this loop reduces by 1.
    // The use of these blobs in inner loop refs need to be updated.
    demoteBlobs(Ref, Level);

    // When IV is substituted in inner loop, temps in merged ref need to be
    // marked as livein to that loop.
    auto *TmpLp = ParentLp;

    while (TmpLp != ReplaceLp && !InnerLoops.count(TmpLp)) {

      for (auto *LowerBRef :
           make_range(MergedRef->blob_begin(), MergedRef->blob_end())) {
        TmpLp->addLiveInTemp(LowerBRef->getSymbase());
      }

      InnerLoops.insert(TmpLp);
      TmpLp = TmpLp->getParentLoop();
    }
  }

  return ParentLp->getNestingLevel();
}

void HLLoop::replaceByFirstIteration() {
  unsigned Level = getNestingLevel();
  const RegDDRef *LB = getLowerDDRef();

  bool HaveExplicitLB = false;
  SmallPtrSet<HLLoop *, 8> InnerLoops;

  extractZtt(Level - 1);
  extractPreheaderAndPostexit();

  ForEach<RegDDRef>::visitRange(
      child_begin(), child_end(),
      [this, Level, &LB, &HaveExplicitLB, &InnerLoops](RegDDRef *Ref) {
        if (Ref->hasIV(Level)) {
          const CanonExpr *IVReplacement = LB->getSingleCanonExpr();

          if (!HaveExplicitLB && !DDRefUtils::canReplaceIVByCanonExpr(
                                     Ref, Level, IVReplacement, false)) {
            // Insert explicit LB copy statement before loop.
            HLInst *LBCopy =
                getHLNodeUtils().createCopyInst(getLowerDDRef()->clone(), "lb");
            HLNodeUtils::insertBefore(this, LBCopy);
            LB = LBCopy->getLvalDDRef();
            IVReplacement = LB->getSingleCanonExpr();
            HaveExplicitLB = true;
          }

          // Expected to be always successful.
          DDRefUtils::replaceIVByCanonExpr(Ref, Level, IVReplacement, IsNSW,
                                           false);
        }

        unsigned RefLevel = demoteRef(Ref, this, LB, InnerLoops);

        // New level of ref decreases by 1.
        Ref->makeConsistent({LB}, RefLevel - 1);
      });

  // To minimize the possibility of topsort numbers re-computation, detach the
  // loop before moving the body nodes.
  HLNode *Marker = getHLNodeUtils().getOrCreateMarkerNode();
  HLNodeUtils::replace(this, Marker);

  HLNodeUtils::moveAfter(Marker, child_begin(), child_end());
  HLNodeUtils::remove(Marker);
}

void HLLoop::verify() const {
  HLDDNode::verify();

  assert(getLowerDDRef() && "Null lower ref not expected!");
  assert(getUpperDDRef(true) && "Null upper ref not expected");
  assert(getStrideDDRef() && "Null stride ref not expected!");

  if (isUnknown()) {
    assert(getHeaderLabel() && "Could not find header label of unknown loop!");
    assert(getBottomTest() && "Could not find bottom test of unknown loop!");
    assert(!hasZtt() && "ZTT not expected for unknown loops!");

  } else {
    auto StrideCE = getStrideDDRef()->getSingleCanonExpr();
    (void)StrideCE;

    assert(!getLowerDDRef()->getSingleCanonExpr()->isNonLinear() &&
           "Loop lower cannot be non-linear!");
    assert(!getUpperDDRef()->getSingleCanonExpr()->isNonLinear() &&
           "Loop upper cannot be non-linear!");
    assert(!StrideCE->isNonLinear() && "Loop stride cannot be non-linear!");

    int64_t Val;
    assert((StrideCE->isIntConstant(&Val) && (Val > 0)) &&
           "Loop stride expected to be a postive integer!");
    (void)Val;

    assert(getUpperDDRef()->getSrcType()->isIntegerTy() &&
           "Invalid loop upper type!");
  }

  // TODO: Implement special case as ZTT's DDRefs are attached to node
  // if (Ztt) {
  //  Ztt->verify();
  //}

  assert((!getParentLoop() ||
          (getNestingLevel() == getParentLoop()->getNestingLevel() + 1)) &&
         "If it's not a top-level loop its nesting level should be +1");
  assert((getParentLoop() || getNestingLevel() == 1) &&
         "Top level loops should have 1st nesting level");

  assert(hasChildren() &&
         "Found an empty Loop, assumption that there should be no empty loops");
}

bool HLLoop::hasDirective(int DirectiveID) const {
  HLContainerTy::const_iterator Iter(*this);
  auto First = getHLNodeUtils().getFirstLexicalChild(getParent(), this);
  HLContainerTy::const_iterator FIter(*First);

  while (Iter != FIter) {
    --Iter;
    const HLInst *I = dyn_cast<HLInst>(Iter);

    // Loop, IF, Switch, etc.
    if (!I) {
      return false;
    }

    if (I->isDirective(DirectiveID)) {
      return true;
    }
  }

  return false;
}

bool HLLoop::hasVectorizeIVDepPragma() const {
  return hasVectorizeIVDepLoopPragma() || hasVectorizeIVDepBackPragma() ||
         (AssumeIVDEPInnermostLoop && isInnermost());
}

bool HLLoop::isTriangularLoop() const {

  const CanonExpr *LB = getLowerCanonExpr();
  const CanonExpr *UB = getUpperCanonExpr();
  if (LB->hasIV() || UB->hasIV()) {
    return true;
  }

  for (auto I = ztt_ddref_begin(), E1 = ztt_ddref_end(); I != E1; ++I) {
    const RegDDRef *RRef = *I;

    assert(RRef->isTerminalRef() && "non-terminal ref not expected in ztt!");

    if (RRef->getSingleCanonExpr()->hasIV()) {
      return true;
    }
  }

  return false;
}

void HLLoop::addRemoveLoopMetadataImpl(ArrayRef<MDNode *> MDs,
                                       StringRef RemoveID) {
  assert((MDs.empty() || RemoveID.empty()) &&
         "Simultaneous addition and removal not expected!");

  LLVMContext &Context = getHLNodeUtils().getHIRFramework().getContext();

  // Reserve space for the unique identifier
  SmallVector<Metadata *, 4> NewMDs(1);

  bool IsAddition = !MDs.empty();
  bool FoundRemoveID = false;

  MDNode *ExistingLoopMD = getLoopMetadata();

  if (ExistingLoopMD) {
    // TODO: add tests for this part of code after enabling generation of HIR
    // for loops with pragmas.
    for (unsigned I = 1, E = ExistingLoopMD->getNumOperands(); I < E; ++I) {
      Metadata *RawMD = ExistingLoopMD->getOperand(I);

      MDNode *MD = dyn_cast<MDNode>(RawMD);
      if (!MD || MD->getNumOperands() == 0) {
        // Unconditionally copy unknown metadata.
        NewMDs.push_back(RawMD);
        continue;
      }

      const MDString *Id = dyn_cast<MDString>(MD->getOperand(0));

      if (!Id) {
        // Do not handle non-string identifiers. Unconditionally copy metadata.
        NewMDs.push_back(MD);
        continue;
      }

      StringRef IdRef = Id->getString();

      if (IsAddition) {
        // Check if the metadata will be redefined by the new one.
        bool IsRedefined =
            std::any_of(MDs.begin(), MDs.end(), [IdRef](MDNode *NewMD) {
              const MDString *NewId = dyn_cast<MDString>(NewMD->getOperand(0));
              assert(NewId && "Added metadata should contain string "
                              "identifier as a first operand");

              if (NewId->getString().equals(IdRef)) {
                return true;
              }

              return false;
            });

        // Do not copy redefined metadata.
        if (IsRedefined) {
          continue;
        }

      } else if (IdRef.equals(RemoveID)) {
        FoundRemoveID = true;
        continue;
      }

      NewMDs.push_back(MD);
    }
  }

  // We were in removal node and did not find RemoveID so there is nothing to
  // do.
  if (!IsAddition && !FoundRemoveID) {
    return;
  }

  NewMDs.append(MDs.begin(), MDs.end());

  MDNode *NewLoopMD = MDNode::get(Context, NewMDs);
  NewLoopMD->replaceOperandWith(0, NewLoopMD);
  setLoopMetadata(NewLoopMD);
}

void HLLoop::markDoNotVectorize() {
  LLVMContext &Context = getHLNodeUtils().getHIRFramework().getContext();

  Metadata *One =
      ConstantAsMetadata::get(ConstantInt::get(Type::getInt32Ty(Context), 1));

  Metadata *MDVectorWidth[] = {
      MDString::get(Context, "llvm.loop.vectorize.width"), One};
  Metadata *MDInterleaveCount[] = {
      MDString::get(Context, "llvm.loop.interleave.count"), One};

  MDNode *MDs[] = {MDNode::get(Context, MDVectorWidth),
                   MDNode::get(Context, MDInterleaveCount)};

  addLoopMetadata(MDs);
}

void HLLoop::markDoNotUnroll() {
  removeLoopMetadata("llvm.loop.unroll.enable");
  removeLoopMetadata("llvm.loop.unroll.count");

  LLVMContext &Context = getHLNodeUtils().getHIRFramework().getContext();
  addLoopMetadata(
      MDNode::get(Context, MDString::get(Context, "llvm.loop.unroll.disable")));
}

void HLLoop::markDoNotUnrollAndJam() {
  removeLoopMetadata("llvm.loop.unroll_and_jam.enable");
  removeLoopMetadata("llvm.loop.unroll_and_jam.count");

  LLVMContext &Context = getHLNodeUtils().getHIRFramework().getContext();
  addLoopMetadata(MDNode::get(
      Context, MDString::get(Context, "llvm.loop.unroll_and_jam.disable")));
}

bool HLLoop::canNormalize(const CanonExpr *LowerCE) const {

  if (isUnknown()) {
    return false;
  }

  // If LB not supplied, get it from Loop
  // For stripmining code, the LB is constructed later in the loop
  // we know it can be normalized
  if (!LowerCE) {
    LowerCE = getLowerCanonExpr();
  }

  assert(CanonExprUtils::mergeable(LowerCE, getUpperCanonExpr(), false) &&
         "Lower and Upper are expected to be always mergeable");

  unsigned Level = getNestingLevel();

  bool Mergeable = true;
  ForEach<const HLDDNode>::visitRange(
      child_begin(), child_end(),
      [LowerCE, Level, &Mergeable](const HLDDNode *Node) {
        for (const RegDDRef *Ref :
             llvm::make_range(Node->ddref_begin(), Node->ddref_end())) {
          for (const CanonExpr *CE :
               llvm::make_range(Ref->canon_begin(), Ref->canon_end())) {
            if (!CE->hasIV(Level)) {
              continue;
            }

            if (!CanonExprUtils::mergeable(CE, LowerCE, true)) {
              Mergeable = false;
              return;
            }
          }
        }
      });

  return Mergeable;
}

bool HLLoop::normalize() {
  if (isNormalized()) {
    return true;
  }

  if (!canNormalize()) {
    DEBUG_NORMALIZE(dbgs() << "[HIR-NORMALIZE] Can not normalize loop "
                           << getNumber() << "\n");
    return false;
  }

  CanonExpr *LowerCE = getLowerCanonExpr();
  CanonExpr *StrideCE = getStrideCanonExpr();

  DEBUG_NORMALIZE(dbgs() << "[HIR-NORMALIZE] Before:\n");
  DEBUG_NORMALIZE(dump());

  int64_t Stride;
  StrideCE->isIntConstant(&Stride);

  RegDDRef *UpperRef = getUpperDDRef();
  RegDDRef *LowerRef = getLowerDDRef();

  // Clone is required as we will be updating upper ref and will be using
  // original ref to make it consistent.
  std::unique_ptr<RegDDRef> UpperRefClone(UpperRef->clone());
  SmallVector<const RegDDRef *, 2> Aux = {LowerRef, UpperRefClone.get()};

  CanonExpr *UpperCE = getUpperCanonExpr();

  // New Upper = (U - L) / S
  if (!CanonExprUtils::subtract(UpperCE, LowerCE, false)) {
    llvm_unreachable("[HIR-NORMALIZE] Can not subtract L from U");
  }

  UpperCE->divide(Stride);
  UpperCE->simplify(true, true);

  unsigned Level = getNestingLevel();

  // NewIV = S * IV + L
  std::unique_ptr<CanonExpr> NewIV(LowerCE->clone());
  NewIV->addIV(Level, InvalidBlobIndex, Stride, false);

  bool IsNSW = isNSW();
  auto UpdateCE = [&NewIV, Level, IsNSW](CanonExpr *CE) {
    if (!CE->hasIV(Level)) {
      return;
    }

    // The CEs are either properly mergeable or LowerCE is a mergeable constant.
    // Because we add an IV to the constant LowerCE is can make it
    // non-mergeable.
    // For ex.: LowerCE: i64 7       - can merge with a constant
    //          NewIV:   i64 i1 + 7  - type conflict i32/i64.
    //          CE:      sext.i32.i64(i1 + %61 + 8)
    // To avoid artificial assertion in the replaceIVByCanonExpr() we set the
    // correct src type to the NewIV.
    NewIV->setSrcType(CE->getSrcType());

    if (!CanonExprUtils::replaceIVByCanonExpr(CE, Level, NewIV.get(), IsNSW,
                                              true)) {
      llvm_unreachable("[HIR-NORMALIZE] Can not replace IV by Lower");
    }
  };

  ForEach<HLDDNode>::visitRange(
      child_begin(), child_end(),
      [this, &UpdateCE, &Aux, Level](HLDDNode *Node) {
        for (RegDDRef *Ref :
             llvm::make_range(Node->ddref_begin(), Node->ddref_end())) {
          for (CanonExpr *CE :
               llvm::make_range(Ref->canon_begin(), Ref->canon_end())) {
            UpdateCE(CE);
          }

          Ref->makeConsistent(Aux, IsInnermost ? Level : NonLinearLevel);
        }
      });

  StrideCE->setConstant(1);

  UpperRef->makeConsistent(Aux, Level);

  LowerCE->clear();
  LowerRef->makeConsistent({}, Level);

  DEBUG_NORMALIZE(dbgs() << "[HIR-NORMALIZE] After:\n");
  DEBUG_NORMALIZE(dump());

  LoopsNormalized++;

  return true;
}

bool HLLoop::canStripmine(unsigned StripmineSize) const {
  assert(isNormalized() &&
         "Loop needs stripmine are expected to be normalized");

  if (!isStripmineRequired(StripmineSize)) {
    return true;
  }

  unsigned Level = getNestingLevel();
  if (Level == MaxLoopNestLevel) {
    return false;
  }

  bool Result = true;
  // Check out if loop can be mormalized before proceeding
  // Need to create a new LB

  const CanonExpr *LBCE = getLowerDDRef()->getSingleCanonExpr();

  CanonExpr *CE = LBCE->clone();
  CE->clear();

  //  64*i1
  CE->setIVConstCoeff(Level, StripmineSize);
  if (!canNormalize(CE)) {
    Result = false;
  }

  getCanonExprUtils().destroy(CE);
  return Result;
}

bool HLLoop::isStripmineRequired(unsigned StripmineSize) const {
  assert(isNormalized() &&
         "Loop needs stripmine are expected to be normalized");

  uint64_t TripCount;
  return !isConstTripLoop(&TripCount) || (TripCount > StripmineSize);
}

HLIf *HLLoop::getBottomTest() {
  if (!isUnknown()) {
    return nullptr;
  }

  auto LastChild = getLastChild();

  assert(LastChild && isa<HLIf>(LastChild) &&
         "Could not find bottom test for unknown loop!");

  return cast<HLIf>(LastChild);
}

HLLabel *HLLoop::getHeaderLabel() {
  if (!isUnknown()) {
    return nullptr;
  }

  auto FirstChild = getFirstChild();

  assert(FirstChild && isa<HLLabel>(FirstChild) &&
         "Could not find bottom test for unknown loop!");

  return cast<HLLabel>(FirstChild);
}

MDNode *HLLoop::getLoopStringMetadata(StringRef Name) const {
  if (!LoopMetadata) {
    return nullptr;
  }

  for (unsigned I = 1, E = LoopMetadata->getNumOperands(); I < E; ++I) {
    MDNode *MD = dyn_cast<MDNode>(LoopMetadata->getOperand(I));
    if (!MD) {
      continue;
    }

    MDString *StrMD = dyn_cast<MDString>(MD->getOperand(0));

    if (!StrMD) {
      continue;
    }

    if (Name.equals(StrMD->getString()))
      return MD;
  }

  return nullptr;
}

bool HLLoop::hasCompleteUnrollEnablingPragma() const {
  uint64_t TC;
  if (!isConstTripLoop(&TC)) {
    return false;
  }

  // llvm.loop.unroll.enable is a no-op for complete unroll as we still need to
  // heuristically decide the unroll factor.
  if (getLoopStringMetadata("llvm.loop.unroll.full")) {
    return true;
  }

  // Unroll if loop's trip count is less than unroll count.
  auto PragmaTC = getUnrollPragmaCount();

  return (TC <= PragmaTC);
}

bool HLLoop::hasCompleteUnrollDisablingPragma() const {

  if (getLoopStringMetadata("llvm.loop.unroll.disable")) {
    return true;
  }

  auto PragmaTC = getUnrollPragmaCount();

  if (PragmaTC) {
    uint64_t TC;

    if (!isConstTripLoop(&TC) || (PragmaTC < TC)) {
      return true;
    }
  }

  return false;
}

bool HLLoop::hasGeneralUnrollDisablingPragma() const {

  if (getLoopStringMetadata("llvm.loop.unroll.disable") ||
      getLoopStringMetadata("llvm.loop.unroll.runtime.disable") ||
      // 'full' metadata only implies complete unroll, not partial unroll.
      getLoopStringMetadata("llvm.loop.unroll.full")) {
    return true;
  }

  unsigned PragmaCount = getUnrollPragmaCount();

  // Unroll count of 1 also qualifies as disabling pragma.
  return (PragmaCount == 1);
}

bool HLLoop::hasVectorizeEnablingPragma() const {
  // The logic is complicated due to the fact that both
  // "llvm.loop.vectorize.width" and "llvm.loop.vectorize.enable" can be used as
  // vectorization enablers/disablers.

  auto *EnableMD = getLoopStringMetadata("llvm.loop.vectorize.enable");

  if (EnableMD &&
      mdconst::extract<ConstantInt>(EnableMD->getOperand(1))->isZero()) {
    return false;
  }

  auto *WidthMD = getLoopStringMetadata("llvm.loop.vectorize.width");

  if (WidthMD &&
      mdconst::extract<ConstantInt>(WidthMD->getOperand(1))->isOne()) {
    return false;
  }

  return (EnableMD || WidthMD);
}

bool HLLoop::hasVectorizeDisablingPragma() const {
  // Return true if either the loop has "llvm.loop.vectorize.width" metadata
  // with width of 1 or it has "llvm.loop.vectorize.enable" metadata with
  // boolean operand set to false.
  auto *MD = getLoopStringMetadata("llvm.loop.vectorize.width");

  if (MD && mdconst::extract<ConstantInt>(MD->getOperand(1))->isOne()) {
    return true;
  }

  MD = getLoopStringMetadata("llvm.loop.vectorize.enable");

  return MD && mdconst::extract<ConstantInt>(MD->getOperand(1))->isZero();
}

bool HLLoop::hasFusionEnablingPragma() const {
  return !hasFusionDisablingPragma() &&
         (getLoopStringMetadata("llvm.loop.fusion.enable") ||
          getLoopStringMetadata("llvm.loop.fusion.full"));
}

bool HLLoop::hasFusionDisablingPragma() const {
  return getLoopStringMetadata("llvm.loop.fusion.disable");
}

struct EarlyExitCollector final : public HLNodeVisitorBase {
  SmallVectorImpl<HLGoto *> &Gotos;
  unsigned MaxTopSortNum;

public:
  EarlyExitCollector(SmallVectorImpl<HLGoto *> &Gotos, HLLoop *Lp)
      : Gotos(Gotos) {
    assert(Lp && "Lp cannot be null\n");
    MaxTopSortNum = Lp->getMaxTopSortNum();
  }

  void visit(HLGoto *Goto) {
    if (Goto->isExternal() ||
        Goto->getTargetLabel()->getTopSortNum() > MaxTopSortNum) {
      Gotos.push_back(Goto);
    }
  };

  void visit(HLNode *Node){};
  void postVisit(HLNode *Node){};
};

void HLLoop::shiftLoopBodyRegDDRefs(int64_t Amount) {
  const unsigned LoopLevel = getNestingLevel();

  ForEach<HLDDNode>::visitRange(
      child_begin(), child_end(), [LoopLevel, Amount](HLDDNode *Node) {
        for (RegDDRef *Ref :
             llvm::make_range(Node->ddref_begin(), Node->ddref_end())) {
          Ref->shift(LoopLevel, Amount);
        }
      });
}

HLLoop *HLLoop::peelFirstIteration(bool UpdateMainLoop) {
  assert(!isUnknown() && isNormalized() &&
         "Unsupported loop in 1st iteration peeling!");

  // Extract Ztt, preheader and postexit from this loop before cloning it so
  // that the peel loop doesn't include them.
  extractZtt();
  extractPreheaderAndPostexit();

  HLLoop *PeelLoop = clone();
  getHLNodeUtils().insertBefore(this, PeelLoop);

  // Since the loop is normalized, set peel loop UB to 0 so that it executes
  // just one iteration.
  PeelLoop->getUpperDDRef()->clear();

  // Update this loop's UB and DDRefs to avoid execution of peeled iteration
  // only if UpdateMainLoop is true.
  if (UpdateMainLoop) {
    getUpperCanonExpr()->addConstant(-1, true /*IsMath*/);
    shiftLoopBodyRegDDRefs(1);

    // Original loop requires a new ztt because the it may only have a single
    // iteration, now executed by the peel loop.
    createZtt(false /*IsOverWrite*/, isNSW());
  }

  return PeelLoop;
}

HLLoop *HLLoop::generatePeelLoop(const RegDDRef *PeelArrayRef, unsigned VF) {
  assert(!isUnknown() && isNormalized() && "Unsupported loop for peeling.");
  assert(PeelArrayRef && PeelArrayRef->isMemRef() &&
         PeelArrayRef->isLinearAtLevel(getNestingLevel()) &&
         "Unsupported PeelArrayRef.");

  auto &CEU = getCanonExprUtils();
  auto &HNU = getHLNodeUtils();
  auto &DDU = getDDRefUtils();
  HLContainerTy PeelLoopInsts;

  // Assuming that this is a full vector register, then alignment needed for
  // accesses to PeelArrayRef is computed as-
  // needed_alignment = VF * sizeof(PeelArrayRef)
  // For example, if PeelArrayRef contains 8-byte pointers and VF=4 the needed
  // alignment will be 32.
  unsigned PeelArrayItemSize =
      CEU.getTypeSizeInBytes(PeelArrayRef->getDestType());
  unsigned NeededAlignment = VF * PeelArrayItemSize;
  const CanonExpr *PeelArrayBase = PeelArrayRef->getBaseCE();
  Type *IntTy = IntegerType::get(
      CEU.getContext(), CEU.getTypeSizeInBits(PeelArrayBase->getSrcType()));

  // Normalization check for main loop before peeling
  RegDDRef *OrigLB = getLowerDDRef();
  RegDDRef *DummyTemp = HNU.createTemp(IntTy);
  setLowerDDRef(DummyTemp);
  if (!canNormalize()) {
    setLowerDDRef(OrigLB);
    assert(false && "Loop cannot be normalized.");
    // Bailout initiation for prod builds
    return nullptr;
  }
  setLowerDDRef(OrigLB);

  // Next we find the current alignment of PeelArrayRef's base pointer which is
  // obtained by-
  // %alignment = &(%arr)[0] & (needed_alignment - 1)
  // Here %arr is array that we align access to via the peel loop.
  RegDDRef *PeelArrayBaseRef = PeelArrayRef->clone();
  PeelArrayBaseRef->replaceIVByConstant(getNestingLevel(), 0);
  PeelArrayBaseRef->setAddressOf(true);
  HLInst *PeelArrayBaseCast =
      HNU.createPtrToInt(IntTy, PeelArrayBaseRef, "arr.base.cast");
  PeelLoopInsts.push_back(*PeelArrayBaseCast);
  HLInst *Alignment = HNU.createAnd(
      PeelArrayBaseCast->getLvalDDRef()->clone(),
      DDU.createConstDDRef(IntTy, NeededAlignment - 1), "alignment");
  PeelLoopInsts.push_back(*Alignment);

  // Finally the peeling factor or number of peel iterations is computed as-
  // %peel_fctr = (needed_alignment - %alignment) >> log2(sizeof(PeelArrayRef))
  HLInst *PeelFactorSub =
      HNU.createSub(DDU.createConstDDRef(IntTy, NeededAlignment),
                    Alignment->getLvalDDRef()->clone(), "peel.factor");
  PeelLoopInsts.push_back(*PeelFactorSub);
  HLInst *PeelFactorShr = HNU.createAShr(
      PeelFactorSub->getLvalDDRef()->clone(),
      DDU.createConstDDRef(IntTy, Log2_64(PeelArrayItemSize)), "peel.factor");
  PeelLoopInsts.push_back(*PeelFactorShr);

  // Clamp down peeling factor if it is greater than loop's TC.
  // %peel_fctr = (TC <= %peel_fctr) ? TC : %peel_fctr
  HLInst *PeelFactorMin = HNU.createMin(
      getTripCountDDRef()->clone(), PeelFactorShr->getLvalDDRef()->clone(),
      PeelFactorShr->getLvalDDRef()->clone(), isNSW());
  PeelLoopInsts.push_back(*PeelFactorMin);
  auto PeelFactorSym = PeelFactorMin->getLvalDDRef()->getSymbase();

  // TODO: Consider truncating PeelFactor to upper DDRef type size in case of
  // mismatch. It is known that PeelFactor is a small number (atmost 63) for
  // char arrays. Currently the mismatch scenario is avoided by idiom
  // recognition. if (PeelFactorShr->getLvalDDRef()->getDestType() !=
  //    getUpperDDRef()->getDestType()) {
  // }

  // Extract Ztt, preheader and postexit from this loop before cloning it so
  // that the peel loop doesn't include them.
  extractZtt();
  extractPreheaderAndPostexit();

  HLLoop *PeelLoop = clone();
  PeelLoop->setMaxTripCountEstimate(NeededAlignment - 1);
  PeelLoop->addLiveInTemp(PeelFactorSym);

  // Updates to cloned peel loop.
  unsigned LoopLevel = getNestingLevel();

  // Upper bound of peel loop will be PeelFactor-1
  auto *PeelLpUB = PeelFactorMin->getLvalDDRef()->clone();
  auto *PeelLpUBCanonExpr = PeelLpUB->getSingleCanonExpr();
  PeelLpUB->addBlobDDRef(PeelLpUBCanonExpr->getSingleBlobIndex(),
                         LoopLevel - 1);
  PeelLpUBCanonExpr->setDefinedAtLevel(LoopLevel - 1);
  PeelLpUBCanonExpr->addConstant(-1, true /*IsMath*/);
  PeelLpUB->setSymbase(GenericRvalSymbase);
  PeelLoop->setUpperDDRef(PeelLpUB);

  // RT check for peel iterations
  HLPredicate Pred(PredicateTy::ICMP_NE);
  HLIf *If =
      getHLNodeUtils().createHLIf(Pred, PeelFactorMin->getLvalDDRef()->clone(),
                                  DDU.createConstDDRef(IntTy, 0));
  HLNodeUtils::insertAsFirstThenChild(If, PeelLoop);

  PeelLoopInsts.push_back(*If);
  HLNodeUtils::insertBefore(this, &PeelLoopInsts);

  // Changes to the current loop, which will become main loop.
  // Lower bound of main loop will be PeelFactor
  auto *MainLpLB = PeelFactorMin->getLvalDDRef()->clone();
  MainLpLB->getSingleCanonExpr()->setDefinedAtLevel(LoopLevel - 1);
  setLowerDDRef(MainLpLB);

  addLiveInTemp(PeelFactorSym);

  // Original loop requires a new ztt since all iterations might be executed in
  // peel loop
  createZtt(false /*Overwrite*/);

  // Since LB of main loop was changed, normalize the loop so that it starts
  // from 0. NOTE: this is needed, since downstream utility to create main &
  // remainder loop assume normalized scalar loops.
  normalize();

  return PeelLoop;
}

void HLLoop::populateEarlyExits(SmallVectorImpl<HLGoto *> &Gotos) {
  if (getNumExits() == 1) {
    return;
  }

  // Collect Gotos in the Loop
  EarlyExitCollector EEC(Gotos, this);

  HLNodeUtils::visitRange(EEC, getFirstChild(), getLastChild());

  assert((Gotos.size() == getNumExits() - 1) && "Mismatch in number of exits!");
}

LoopOptReport LoopOptReportTraits<HLLoop>::getOrCreatePrevOptReport(
    HLLoop &Loop, const LoopOptReportBuilder &Builder) {

  struct PrevLoopFinder : public HLNodeVisitorBase {
    const HLLoop *FoundLoop = nullptr;
    const HLNode *FirstNode;

    PrevLoopFinder(const HLNode *F) : FirstNode(F) {}
    bool isDone() const { return FoundLoop; }
    void visit(const HLLoop *Lp) {
      if (Lp != FirstNode && Lp->getTopSortNum() < FirstNode->getTopSortNum())
        FoundLoop = Lp;
    }
    void visit(const HLNode *Node) {}
    void postVisit(const HLNode *Node) {}
  };

  PrevLoopFinder PLF(&Loop);
  const HLNode *FirstNode;
  const HLNode *LastNode;
  const HLLoop *ParentLoop = Loop.getParentLoop();
  if (ParentLoop) {
    FirstNode = ParentLoop->getFirstChild();
    LastNode = Loop.getHLNodeUtils().getImmediateChildContainingNode(ParentLoop,
                                                                     &Loop);

  } else {
    const HLRegion *ParentRegion = Loop.getParentRegion();
    FirstNode = ParentRegion->getFirstChild();
    LastNode = Loop.getHLNodeUtils().getImmediateChildContainingNode(
        ParentRegion, &Loop);
  }

  HLNodeUtils::visitRange<true, false, false>(PLF, FirstNode, LastNode);
  if (!PLF.FoundLoop)
    return nullptr;

  HLLoop &Lp = const_cast<HLLoop &>(*PLF.FoundLoop);
  return Builder(Lp).getOrCreateOptReport();
}

LoopOptReport LoopOptReportTraits<HLLoop>::getOrCreateParentOptReport(
    HLLoop &Loop, const LoopOptReportBuilder &Builder) {
  if (HLLoop *Dest = Loop.getParentLoop())
    return Builder(*Dest).getOrCreateOptReport();

  if (HLRegion *Dest = Loop.getParentRegion())
    return Builder(*Dest).getOrCreateOptReport();

  llvm_unreachable("Failed to find a parent");
}

void LoopOptReportTraits<HLLoop>::traverseChildLoopsBackward(
    HLLoop &Loop, LoopVisitorTy Func) {
  struct LoopVisitor : public HLNodeVisitorBase {
    using LoopVisitorTy = LoopOptReportTraits<HLLoop>::LoopVisitorTy;
    LoopVisitorTy Func;

    LoopVisitor(LoopVisitorTy Func) : Func(Func) {}
    void postVisit(HLLoop *Lp) { Func(*Lp); }
    void visit(const HLNode *Node) {}
    void postVisit(const HLNode *Node) {}
  };

  if (Loop.hasChildren()) {
    LoopVisitor LV(Func);
    HLNodeUtils::visitRange<true, false, false>(LV, Loop.getFirstChild(),
                                                Loop.getLastChild());
  }
}

static void addTripCountMetadata(HLLoop *Loop, StringRef MetadataTy,
                                 unsigned TripCount) {
  LLVMContext &Context = Loop->getHLNodeUtils().getHIRFramework().getContext();

  auto *TCNode = ConstantAsMetadata::get(
      ConstantInt::get(Type::getInt32Ty(Context), TripCount));

  auto MNode =
      MDNode::get(Context, {MDString::get(Context, MetadataTy), TCNode});

  Loop->addLoopMetadata(MNode);
}

void HLLoop::setPragmaBasedMinimumTripCount(unsigned MinTripCount) {
  addTripCountMetadata(this, "llvm.loop.intel.loopcount_minimum", MinTripCount);
}

void HLLoop::setPragmaBasedMaximumTripCount(unsigned MaxTripCount) {
  addTripCountMetadata(this, "llvm.loop.intel.loopcount_maximum", MaxTripCount);
}

void HLLoop::setPragmaBasedAverageTripCount(unsigned AvgTripCount) {
  addTripCountMetadata(this, "llvm.loop.intel.loopcount_average", AvgTripCount);
}

void HLLoop::dividePragmaBasedTripCount(unsigned Factor) {
  unsigned TC;

  if (getPragmaBasedMinimumTripCount(TC)) {
    setPragmaBasedMinimumTripCount(TC / Factor);
  }

  if (getPragmaBasedMaximumTripCount(TC)) {
    setPragmaBasedMaximumTripCount(TC / Factor);
  }

  if (getPragmaBasedAverageTripCount(TC)) {
    setPragmaBasedAverageTripCount(TC / Factor);
  }
}
