//===-------- HLLoop.cpp - Implements the HLLoop class --------------------===//
//
// Copyright (C) 2015-2021 Intel Corporation. All rights reserved.
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
#include "llvm/Analysis/Intel_OptReport/OptReportPrintUtils.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Support/CommandLine.h"
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

static cl::opt<bool>
    ExplicitLowerBoundSwitch("hir-loop-normalize-allow-explicit-lower-bound",
                             cl::init(true), cl::Hidden,
                             cl::desc("Allow creation of explicit lower bound "
                                      "instruction when normalizing the loop"));

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
      NestingLevel(0), IsInnermost(true), IVType(nullptr), HasSignedIV(false),
      DistributedForMemRec(false), LoopMetadata(LLVMLoop->getLoopID()),
      LegalMaxTripCount(0), MaxTripCountEstimate(0), MaxTCIsUsefulForDD(false),
      HasDistributePoint(false), IsUndoSinkingCandidate(false),
      IsBlocked(false), ForcedVectorWidth(0), ForcedVectorUnrollFactor(0),
      VecTag(VecTagTy::NONE) {
  assert(LLVMLoop && "LLVM loop cannot be null!");

  initialize();

  SmallVector<Loop::Edge, 8> ExitEdges;

  OrigLoop->getExitEdges(ExitEdges);
  setNumExits(ExitEdges.size());
  // If Lp has attached optreport metadata node - initialize HLoop
  // optreport with it. Otherwise it will initialize it with zero.
  // We also don't erase the opt report from LoopID. We only do that
  // at the HIRCodeGen stage, if needed.
  setOptReport(OptReport::findOptReportInLoopID(LLVMLoop->getLoopID()));

  // Drop any "llvm.loop.parallel_accesses" metadata. This metadata is not yet
  // used or preserved by HIR transformations.
  removeLoopMetadata("llvm.loop.parallel_accesses");
}

// IsInnermost flag is initialized to true, please refer to the header file.
HLLoop::HLLoop(HLNodeUtils &HNU, HLIf *ZttIf, RegDDRef *LowerDDRef,
               RegDDRef *UpperDDRef, RegDDRef *StrideDDRef, unsigned NumEx)
    : HLDDNode(HNU, HLNode::HLLoopVal), OrigLoop(nullptr), Ztt(nullptr),
      NestingLevel(0), IsInnermost(true), HasSignedIV(false),
      DistributedForMemRec(false), LoopMetadata(nullptr), LegalMaxTripCount(0),
      MaxTripCountEstimate(0), MaxTCIsUsefulForDD(false),
      HasDistributePoint(false), IsUndoSinkingCandidate(false),
      IsBlocked(false), ForcedVectorWidth(0), ForcedVectorUnrollFactor(0),
      VecTag(VecTagTy::NONE) {
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
      IVType(HLLoopObj.IVType), HasSignedIV(HLLoopObj.HasSignedIV),
      LiveInSet(HLLoopObj.LiveInSet), LiveOutSet(HLLoopObj.LiveOutSet),
      DistributedForMemRec(HLLoopObj.DistributedForMemRec),
      LoopMetadata(HLLoopObj.LoopMetadata),
      LegalMaxTripCount(HLLoopObj.LegalMaxTripCount),
      MaxTripCountEstimate(HLLoopObj.MaxTripCountEstimate),
      MaxTCIsUsefulForDD(HLLoopObj.MaxTCIsUsefulForDD),
      CmpDbgLoc(HLLoopObj.CmpDbgLoc), BranchDbgLoc(HLLoopObj.BranchDbgLoc),
      HasDistributePoint(HLLoopObj.HasDistributePoint),
      IsUndoSinkingCandidate(HLLoopObj.IsUndoSinkingCandidate),
      IsBlocked(HLLoopObj.IsBlocked),
      ForcedVectorWidth(HLLoopObj.ForcedVectorWidth),
      ForcedVectorUnrollFactor(HLLoopObj.ForcedVectorUnrollFactor),
      VecTag(HLLoopObj.VecTag),
      PrefetchingInfoVec(HLLoopObj.PrefetchingInfoVec) {

  initialize();

  /// Clone the Ztt
  if (HLLoopObj.hasZtt()) {
    setZtt(HLLoopObj.Ztt->clone());

    auto ZttRefIt = HLLoopObj.ztt_ddref_begin();

    for (auto ZIt = ztt_pred_begin(), EZIt = ztt_pred_end(); ZIt != EZIt;
         ++ZIt) {
      setLHSZttPredicateOperandDDRef((*ZttRefIt)->clone(), ZIt);
      ++ZttRefIt;
      setRHSZttPredicateOperandDDRef((*ZttRefIt)->clone(), ZIt);
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
  HasSignedIV = Lp.HasSignedIV;
  DistributedForMemRec = Lp.DistributedForMemRec;
  LoopMetadata = Lp.LoopMetadata;
  LegalMaxTripCount = Lp.LegalMaxTripCount;
  MaxTripCountEstimate = Lp.MaxTripCountEstimate;
  MaxTCIsUsefulForDD = Lp.MaxTCIsUsefulForDD;
  HasDistributePoint = Lp.HasDistributePoint;
  IsUndoSinkingCandidate = Lp.IsUndoSinkingCandidate;
  IsBlocked = Lp.IsBlocked;
  ForcedVectorWidth = Lp.ForcedVectorWidth;
  ForcedVectorUnrollFactor = Lp.ForcedVectorUnrollFactor;
  VecTag = Lp.VecTag;

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
  if (Parent && (getTopSortNum() == 0 ||
                 this != HLNodeUtils::getFirstLexicalChild(Parent, this))) {
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
  OS << "+ HasSignedIV: " << (hasSignedIV() ? "Yes\n" : "No\n");

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

  if (BlockingInfo) {
    OS << "\n";
    indent(OS, Depth);

    OS << "+ Blocking levels and factors:";
    for (auto &LevelFactorPair : BlockingInfo->LevelsAndFactors) {
      OS << "(" << LevelFactorPair.first << ",";
      LevelFactorPair.second->print(OS, false);
      OS << ") ";
    }

    OS << "\n";
    indent(OS, Depth);
    OS << "+ Blocking privates:";

    First = true;
    for (auto *Private : BlockingInfo->Privates) {
      if (!First) {
        OS << ", ";
      }
      Private->print(OS, false);
      First = false;
    }
  }

  if (!PrefetchingInfoVec.empty()) {
    OS << "\n";
    indent(OS, Depth);
    OS << "+ Prefetching directives:";

    First = true;
    for (auto &Info : PrefetchingInfoVec) {
      if (!First) {
        OS << ", ";
      }
      OS << "{";
      Info.Var->print(OS, false);

      if (!Info.Dist) {
        OS << ":"
           << "disable"
           << "}";
      } else {
        OS << ":" << Info.Hint << ":" << Info.Dist << "}";
      }

      First = false;
    }
  }

  OS << "\n";
#endif // INTEL_PRODUCT_RELEASE
}

void HLLoop::printDirectives(formatted_raw_ostream &OS, unsigned Depth) const {
  if (ParTraits != nullptr)
    OS << " <parallel>";

  // Implementation of isSIMD() requires top sort numbers,
  // so we skip it when they are not available.
  if (isAttached() && (getTopSortNum() != 0) && isSIMD()) {
    OS << " <simd>";
  }

  // Print the vectorization tag, if any.
  switch (getVecTag()) {
  case VecTagTy::AUTOVEC:
    OS << " <auto-vectorized>";
    break;
  case VecTagTy::SIMD:
    OS << " <simd-vectorized>";
    break;
  case VecTagTy::PEEL:
    OS << " <vector-peel>";
    break;
  case VecTagTy::REMAINDER:
    OS << " <vector-remainder>";
    break;
  case VecTagTy::NONE:
    break;
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

  if (LegalMaxTripCount) {
    OS << "  <LEGAL_MAX_TC = " << LegalMaxTripCount << ">";
  }

  if (getMVTag()) {
    OS << "  <MVTag: " << getMVTag();

    auto &Delinearized = getMVDelinearizableBlobIndices();
    if (!Delinearized.empty()) {
      auto &BU = getBlobUtils();

      OS << ", Delinearized: ";
      for (auto I = Delinearized.begin(), E = Delinearized.end(); I != E; ++I) {
        BU.printBlob(OS, BU.getBlob(*I));
        if (I + 1 != E) {
          OS << ", ";
        }
      }
    }
    OS << ">";
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
  if (Parent && (getTopSortNum() == 0 ||
                 this != HLNodeUtils::getLastLexicalChild(Parent, this))) {
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
  OptReport OR = getOptReport();

  OptReportUtils::printNodeHeaderAndOrigin(OS, 0, OR, getDebugLoc());

  if (OR)
    OptReportUtils::printOptReport(OS, 0, OR);

  OptReportUtils::printNodeFooter(OS, 0, OR);

  if (OR && OR.nextSibling())
    OptReportUtils::printEnclosedOptReport(OS, 0, OR.nextSibling());
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
  setLHSZttPredicateOperandDDRef(Ztt->removeLHSPredicateOperandDDRef(LastIt),
                                 LastIt);
  setRHSZttPredicateOperandDDRef(Ztt->removeRHSPredicateOperandDDRef(LastIt),
                                 LastIt);
}

void HLLoop::removeZttPredicate(const_ztt_pred_iterator CPredI) {
  assert(hasZtt() && "Ztt is absent!");

  // Remove RegDDRefs from loop.
  removeLHSZttPredicateOperandDDRef(CPredI);
  removeRHSZttPredicateOperandDDRef(CPredI);

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
    setLHSZttPredicateOperandDDRef(Ztt->removeLHSPredicateOperandDDRef(I), I),
        setRHSZttPredicateOperandDDRef(Ztt->removeRHSPredicateOperandDDRef(I),
                                       I);
  }
}

HLIf *HLLoop::removeZtt() {

  if (!hasZtt()) {
    return nullptr;
  }

  HLIf *If = Ztt;

  /// Move Ztt DDRefs back to If.
  for (auto I = ztt_pred_begin(), E = ztt_pred_end(); I != E; I++) {
    If->setLHSPredicateOperandDDRef(removeLHSZttPredicateOperandDDRef(I), I);
    If->setRHSPredicateOperandDDRef(removeRHSZttPredicateOperandDDRef(I), I);
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

  assert(CanonExpr::isValidLinearDefLevel(NewLevel) &&
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

void HLLoop::removePreheader() { HLNodeUtils::remove(pre_begin(), pre_end()); }

void HLLoop::removePostexit() { HLNodeUtils::remove(post_begin(), post_end()); }

static void promoteDemoteBlobs(RegDDRef *Ref, unsigned Level, int Increment) {
  if (Ref->isSelfBlob()) {
    unsigned DefLevel = Ref->getSingleCanonExpr()->getDefinedAtLevel();

    if (DefLevel != NonLinearLevel && DefLevel >= Level) {
      Ref->getSingleCanonExpr()->setDefinedAtLevel(DefLevel + Increment);
    }

  } else {
    for (auto *BRef : make_range(Ref->blob_begin(), Ref->blob_end())) {
      auto BlobLevel = BRef->getDefinedAtLevel();

      if (BlobLevel != NonLinearLevel && BlobLevel >= Level) {
        BRef->setDefinedAtLevel(BlobLevel + Increment);
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
    promoteDemoteBlobs(Ref, Level, -1);

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

void HLLoop::replaceByFirstIteration(bool ExtractPostexit) {
  unsigned Level = getNestingLevel();
  const RegDDRef *LB = getLowerDDRef();

  bool HaveExplicitLB = false;
  SmallPtrSet<HLLoop *, 8> InnerLoops;

  extractZtt(Level - 1);
  extractPreheader();
  if (ExtractPostexit) {
    extractPostexit();
  }

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
          DDRefUtils::replaceIVByCanonExpr(Ref, Level, IVReplacement,
                                           HasSignedIV, false);
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

  for (auto *ZttRef : make_range(ztt_ddref_begin(), ztt_ddref_end())) {
    (void)ZttRef;
    assert(!ZttRef->isNonLinear() &&
           "Ztt ref is not expected to be non-linear!");
  }

  assert((!getParentLoop() ||
          (getNestingLevel() == getParentLoop()->getNestingLevel() + 1)) &&
         "If it's not a top-level loop its nesting level should be +1");
  assert((getParentLoop() || getNestingLevel() == 1) &&
         "Top level loops should have 1st nesting level");

  assert(hasChildren() &&
         "Found an empty Loop, assumption that there should be no empty loops");
}

static inline bool nodeIsDirective(const HLNode *Node, int DirectiveID) {
  const HLInst *I = dyn_cast<HLInst>(Node);
  // Loop, IF, Switch, etc.
  if (!I)
    return false;

  return I->isDirective(DirectiveID);
}

static const HLInst *getDirectiveFromNode(const HLNode *Node, int DirectiveID) {
  while ((Node = Node->getPrevNode())) {
    if (nodeIsDirective(Node, DirectiveID)) {
      return cast<HLInst>(Node);
    }
  }
  return nullptr;
}

const HLInst *HLLoop::getDirective(int DirectiveID) const {
  // Allow SIMD loop detection if directive is inside loop's Preheader.
  if (hasPreheader()) {
    auto *LastPreheaderNode = getLastPreheaderNode();

    if (nodeIsDirective(LastPreheaderNode, DirectiveID))
      return cast<HLInst>(LastPreheaderNode);

    const HLInst *DirInst =
        getDirectiveFromNode(LastPreheaderNode, DirectiveID);
    if (DirInst)
      return DirInst;
  }

  // Allow SIMD loop detection if directive is sibling node to HLLoop.
  if (const HLInst *DirInst = getDirectiveFromNode(this, DirectiveID)) {
    return DirInst;
  }

  // Allow SIMD loop detection inside if conditions inside SIMD region
  if (auto *Parent = dyn_cast<HLIf>(getParent())) {
    return getDirectiveFromNode(Parent, DirectiveID);
  }

  return nullptr;
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

    assert(!RRef->isMemRef() && "non-terminal ref not expected in ztt!");

    if (RRef->getSingleCanonExpr()->hasIV()) {
      return true;
    }
  }

  return false;
}

void HLLoop::addRemoveLoopMetadataImpl(ArrayRef<MDNode *> MDs,
                                       StringRef RemoveID,
                                       MDNode **ExternalLoopMetadata) {
  assert((MDs.empty() || RemoveID.empty()) &&
         "Simultaneous addition and removal not expected!");

  LLVMContext &Context = getHLNodeUtils().getHIRFramework().getContext();

  // Reserve space for the unique identifier
  SmallVector<Metadata *, 4> NewMDs(1);

  bool IsAddition = !MDs.empty();
  bool FoundRemoveID = false;

  MDNode *ExistingLoopMD =
      ExternalLoopMetadata ? *ExternalLoopMetadata : getLoopMetadata();

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

  MDNode *NewLoopMD = nullptr;
  NewMDs.append(MDs.begin(), MDs.end());

  if (NewMDs.size() > 1) {
    NewLoopMD = MDNode::get(Context, NewMDs);
    NewLoopMD->replaceOperandWith(0, NewLoopMD);
  }

  if (ExternalLoopMetadata) {
    *ExternalLoopMetadata = NewLoopMD;
  } else {
    NewLoopMD ? setLoopMetadata(NewLoopMD) : clearLoopMetadata();
  }
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

void HLLoop::markLLVMLoopDoNotUnroll() {
  auto *LoopMD = OrigLoop->getLoopID();

  removeLoopMetadata("llvm.loop.unroll.enable", &LoopMD);
  removeLoopMetadata("llvm.loop.unroll.count", &LoopMD);

  LLVMContext &Context = getHLNodeUtils().getHIRFramework().getContext();
  addLoopMetadata(
      MDNode::get(Context, MDString::get(Context, "llvm.loop.unroll.disable")),
      &LoopMD);

  OrigLoop->setLoopID(LoopMD);
}

void HLLoop::markDoNotUnrollAndJam() {
  removeLoopMetadata("llvm.loop.unroll_and_jam.enable");
  removeLoopMetadata("llvm.loop.unroll_and_jam.count");

  LLVMContext &Context = getHLNodeUtils().getHIRFramework().getContext();
  addLoopMetadata(MDNode::get(
      Context, MDString::get(Context, "llvm.loop.unroll_and_jam.disable")));
}

void HLLoop::markDoNotBlock() {
  // Currently, the only user of this util is HIRInterLoopBlocking.
  // Because inter loop blocking is avoided over loops with any loop blocking
  // pragma, this assertion can be added.
  // Without this assertion, careful checks might be needed to see if
  // the existing pragma information is conflicting with noblock_loop.
  // Also, BlockingInfo is a vector containing information for this loop and
  // all its children loops. Exisiting pragma could be a vector of the size
  // larger than one.
  assert(!BlockingInfo);

  // Set information strictly only for the loop, not for any children loops.
  // Factor 0 indicates noblock.
  BlockingInfo.reset(new BlockingPragmaInfo);
  BlockingInfo->LevelsAndFactors.push_back(
    std::make_pair(1 /* Level */, getDDRefUtils().createConstDDRef(
        Type::getInt32Ty(getHLNodeUtils().getContext()), 0) /* Factor */));
}

bool HLLoop::canNormalize(const CanonExpr *LowerCE,
                          bool AllowExplicitBoundInst) const {

  if (isUnknown()) {
    return false;
  }

  // We can always normalize using explicit bound inst.
  if (ExplicitLowerBoundSwitch && AllowExplicitBoundInst) {
    return true;
  }

  // If LB not supplied, get it from Loop
  // For stripmining code, the LB is constructed later in the loop
  // we know it can be normalized
  if (!LowerCE) {
    LowerCE = getLowerCanonExpr();
  }

  assert(CanonExprUtils::mergeable(LowerCE, getUpperCanonExpr(), false) &&
         "Lower and Upper are expected to be always mergeable");

  if (LowerCE->isIntConstant() ||
      LowerCE->canConvertToStandAloneBlobOrConstant()) {
    return true;
  }

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

static bool normalizeCE(const HLLoop *Lp, CanonExpr *CE,
                        CanonExpr *NormalizedIV, const CanonExpr *LowerBlob) {

  unsigned Level = Lp->getNestingLevel();

  if (!CE->hasIV(Level)) {
    return false;
  }

  bool LowerIsConst = Lp->getLowerCanonExpr()->isIntConstant();

  if (LowerIsConst) {
    // The CEs are either properly mergeable or LowerCE is a mergeable constant.
    // Because we add an IV to the constant LowerCE is can make it
    // non-mergeable.
    // For ex.: LowerCE: i64 7       - can merge with a constant
    //          NormalizedIV:   i64 i1 + 7  - type conflict i32/i64.
    //          CE:      sext.i32.i64(i1 + %61 + 8)
    // To avoid artificial assertion in the replaceIVByCanonExpr() we set the
    // correct src type to the NormalizedIV.
    NormalizedIV->setSrcType(CE->getSrcType());
  }

  bool IsSigned = Lp->hasSignedIV();

  if (CanonExprUtils::replaceIVByCanonExpr(CE, Level, NormalizedIV, IsSigned,
                                           true)) {
    return true;
  }

  assert(!LowerIsConst &&
         "[HIR-NORMALIZE] Replacement of IV failed for constant lower");
  assert(LowerBlob && "[HIR-NORMALIZE] LowerBlob is nullptr");

  // Create temporary normalized IV in CE's src type by using conversion to
  // standalone blob.
  std::unique_ptr<CanonExpr> TmpNormalizedIV(LowerBlob->clone());

  // Extend or truncate to CE type.
  TmpNormalizedIV->setDestType(CE->getSrcType()->getScalarType());
  bool Res = TmpNormalizedIV->convertToStandAloneBlobOrConstant();
  (void)Res;
  assert(Res && "[HIR-NORMALIZE] Conversion to stand alone blob failed");

  int64_t Stride;
  Lp->getStrideCanonExpr()->isIntConstant(&Stride);

  TmpNormalizedIV->addIV(Level, InvalidBlobIndex, Stride, false);

  Res = CanonExprUtils::replaceIVByCanonExpr(CE, Level, TmpNormalizedIV.get(),
                                             IsSigned, true);
  (void)Res;
  assert(Res && "[HIR-NORMALIZE] replacement of IV by lower failed");
  return true;
}

bool HLLoop::normalize(bool AllowExplicitBoundInst) {
  if (isNormalized()) {
    return true;
  }

  if (!canNormalize(nullptr, AllowExplicitBoundInst)) {
    DEBUG_NORMALIZE(dbgs() << "[HIR-NORMALIZE] Can not normalize loop "
                           << getNumber() << "\n");
    return false;
  }

  DEBUG_NORMALIZE(dbgs() << "[HIR-NORMALIZE] Before:\n");
  DEBUG_NORMALIZE(dump());

  unsigned Level = getNestingLevel();
  RegDDRef *LowerRef = getLowerDDRef();
  CanonExpr *LowerCE = LowerRef->getSingleCanonExpr();

  // Single blob which represents the lower.
  CanonExpr *LowerBlob = nullptr;

  if (!LowerCE->isIntConstant()) {

    // If explicit instruction is allowed, it is better to create it upfront
    // otherwise loop invariant operations in the lower get embedded inside the
    // normalized CEs in the loop body and may not exist in a hoistable form.
    if (ExplicitLowerBoundSwitch && AllowExplicitBoundInst) {
      HLInst *LBCopyInst =
          getHLNodeUtils().createCopyInst(removeLowerDDRef(), "lb");
      HLNodeUtils::insertAsLastPreheaderNode(this, LBCopyInst);

      LBCopyInst->getRvalDDRef()->makeConsistent();

      LowerRef = LBCopyInst->getLvalDDRef()->clone();

      LowerCE = LowerRef->getSingleCanonExpr();
      LowerCE->setDefinedAtLevel(Level - 1);

      setLowerDDRef(LowerRef);

      LowerBlob = LowerCE;

    } else {
      LowerBlob = LowerCE->clone();
      LowerBlob->convertToStandAloneBlobOrConstant();
      // This produces the desired extension on lower for normalized IV
      // creation. It is a small hack to not set this multiple times on new
      // normalized IV creation inside normalizeCE().
      LowerBlob->setExtType(HasSignedIV);
    }
  }

  RegDDRef *UpperRef = getUpperDDRef();

  // Clone is required as we will be updating upper ref and will be using
  // original ref to make it consistent.
  std::unique_ptr<RegDDRef> UpperRefClone(UpperRef->clone());

  CanonExpr *UpperCE = UpperRef->getSingleCanonExpr();

  // New Upper = (U - L) / S
  if (!CanonExprUtils::subtract(UpperCE, LowerCE, false)) {
    llvm_unreachable("[HIR-NORMALIZE] Can not subtract L from U");
  }

  CanonExpr *StrideCE = getStrideCanonExpr();
  int64_t Stride;
  StrideCE->isIntConstant(&Stride);

  UpperCE->divide(Stride);
  UpperCE->simplify(true, true);

  UpperRef->makeConsistent({UpperRefClone.get(), LowerRef}, Level);
  SmallVector<const RegDDRef *, 2> Aux = {LowerRef, UpperRefClone.get()};

  // NormalizedIV = S * IV + L
  std::unique_ptr<CanonExpr> NormalizedIV(LowerCE->clone());
  NormalizedIV->addIV(Level, InvalidBlobIndex, Stride, false);

  SmallVector<unsigned, 2> LowerRefSymbases;
  LowerRef->populateTempBlobSymbases(LowerRefSymbases);

  ForEach<HLDDNode>::visitRange(
      child_begin(), child_end(),
      [this, &NormalizedIV, &Aux, &LowerBlob, Level,
       &LowerRefSymbases](HLDDNode *Node) {
        bool NodeModified = false;

        for (RegDDRef *Ref :
             llvm::make_range(Node->ddref_begin(), Node->ddref_end())) {
          for (CanonExpr *CE :
               llvm::make_range(Ref->canon_begin(), Ref->canon_end())) {
            NodeModified |= normalizeCE(this, CE, NormalizedIV.get(), LowerBlob);
          }

          Ref->makeConsistent(Aux, IsInnermost ? Level : NonLinearLevel);
        }

        // Add livein symbases to parent loops used in current loop lower bound.
        if (NodeModified && !LowerRefSymbases.empty()) {
          for (auto *ParentLoop = Node->getParentLoop(); ParentLoop != this;
               ParentLoop = ParentLoop->getParentLoop()) {
            ParentLoop->addLiveInTemp(LowerRefSymbases);
          }
        }
      });

  StrideCE->setConstant(1);

  LowerCE->clear();
  LowerRef->makeConsistent({}, Level);

  DEBUG_NORMALIZE(dbgs() << "[HIR-NORMALIZE] After:\n");
  DEBUG_NORMALIZE(dump());

  LoopsNormalized++;

  return true;
}

bool HLLoop::canStripmine(unsigned StripmineSize,
                          bool AllowExplicitBoundInst) const {
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
  if (!canNormalize(CE, AllowExplicitBoundInst)) {
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

bool HLLoop::canPeelFirstIteration() const {
  if (!isUnknown()) {
    return true;
  }

  // Handle any bottom test where the operands are linear at level.
  // For example-
  //
  // + UNKNOWN LOOP i1
  // | L:
  // | <i1 = 0>
  // |
  // | if (i1 < A[0]) {
  // |   < i1 = i1 + 1>
  // |   goto L;
  // | }
  //
  // In general, any bottom test which can be modified to act as the ZTT
  // condition for rest of the loop iterations after peeling can be handled.
  //
  // After peeling the loops will look like this-
  //
  // + UNKNOWN LOOP i1        // Peel loop
  // | L.peel:
  // | <i1 = 0>
  // |
  // | if (undef false undef) {   // 'false' bottom test to force loop exit.
  // |   < i1 = i1 + 1>
  // |   goto L.peel;
  // | }
  //
  // if (0 < A[0]) {          // ZTT formed by replacing IV by 0 in bottom test.
  //   + UNKNOWN LOOP i1      // Modified main loop
  //   | L:
  //   | <i1 = 0>
  //   |
  //   | if (i1 + 1 < A[0]) {     // Shifted IV by 1.
  //   |   < i1 = i1 + 1>
  //   |   goto L;
  //   | }
  // }

  auto *BottomTest = getBottomTest();

  unsigned Level = getNestingLevel();

  for (auto *Ref :
       make_range(BottomTest->ddref_begin(), BottomTest->ddref_end())) {
    if (!Ref->isLinearAtLevel(Level)) {
      return false;
    }
  }

  return true;
}

HLLoop *HLLoop::peelFirstIteration(bool UpdateMainLoop) {
  assert(isNormalized() && "Unsupported loop in 1st iteration peeling!");

  // When 'UpdateMainLoop' is false, the peel loop executes a redundant
  // iteration which is also executed by the main loop. Since this is always
  // doable, we don't need to check canPeelFirstIteration().
  if (UpdateMainLoop && !canPeelFirstIteration()) {
    return nullptr;
  }

  bool IsUnknown = isUnknown();

  // Extract Ztt, preheader and postexit from this loop before cloning it so
  // that the peel loop doesn't include them.
  extractZttPreheaderAndPostexit();

  HLLoop *PeelLoop = clone();
  HLNodeUtils::insertBefore(this, PeelLoop);

  if (IsUnknown) {
    // Change the peel loop's bottom test condition to false to force loop to
    // exit after one iteration.
    auto *PeelBottomTest = PeelLoop->getBottomTest();
    auto *PredI = PeelBottomTest->pred_begin();
    PeelBottomTest->replacePredicate(PredI, PredicateTy::FCMP_FALSE);

    auto *LHS = PeelBottomTest->getLHSPredicateOperandDDRef(PredI);
    auto *UndefOp = getDDRefUtils().createUndefDDRef(LHS->getDestType());

    PeelBottomTest->setLHSPredicateOperandDDRef(UndefOp, PredI);
    PeelBottomTest->setRHSPredicateOperandDDRef(UndefOp->clone(), PredI);

  } else {
    // Since the loop is normalized, set peel loop UB to 0 so that it executes
    // just one iteration.
    PeelLoop->getUpperDDRef()->clear();
  }

  // Update this loop's UB and DDRefs to avoid execution of peeled iteration
  // only if UpdateMainLoop is true.
  if (UpdateMainLoop) {
    if (!IsUnknown) {
      auto *Upper = getUpperDDRef();
      Upper->getSingleCanonExpr()->addConstant(-1, true /*IsMath*/);
      Upper->makeConsistent();

      shiftLoopBodyRegDDRefs(1);

      // Original loop requires a new ztt because it may only have a single
      // iteration, now executed by the peel loop.
      createZtt(false /*IsOverWrite*/, hasSignedIV());

    } else {
      // Clone of the bottom test can act as the ztt for rest of the iterations
      // if we replace IV by zero.
      auto *Ztt = getBottomTest()->cloneEmpty();

      unsigned Level = getNestingLevel();
      for (auto *Ref : make_range(Ztt->ddref_begin(), Ztt->ddref_end())) {
        Ref->replaceIVByConstant(Level, 0);
        Ref->makeConsistent({}, Level - 1);
      }

      HLNodeUtils::insertBefore(this, Ztt);
      HLNodeUtils::moveAsFirstThenChild(Ztt, this);

      shiftLoopBodyRegDDRefs(1);
    }
  }

  HLNodeUtils::addCloningInducedLiveouts(PeelLoop, this);

  return PeelLoop;
}

void HLLoop::undefInitializeUnconditionalLiveoutTemps() {
  SmallSet<unsigned, 4> LiveoutOnlyTemps;

  for (auto SB : make_range(live_out_begin(), live_out_end())) {
    // Temps which are also livein are not a concern.
    if (!isLiveIn(SB)) {
      LiveoutOnlyTemps.insert(SB);
    }
  }

  if (LiveoutOnlyTemps.empty()) {
    return;
  }

  auto &HNU = getHLNodeUtils();
  auto &DDRU = getDDRefUtils();
  auto &BU = getBlobUtils();
  auto *FirstChild = getFirstChild();

  for (auto &Node : make_range(child_begin(), child_end())) {
    auto *Inst = dyn_cast<HLInst>(&Node);

    if (!Inst) {
      continue;
    }

    auto *LvalRef = Inst->getLvalDDRef();

    if (!LvalRef) {
      continue;
    }

    unsigned SB = LvalRef->getSymbase();

    if (!LiveoutOnlyTemps.count(SB) ||
        !HLNodeUtils::postDominates(Inst, FirstChild)) {
      continue;
    }

    auto *LiveoutTempRef =
        DDRU.createSelfBlobRef(BU.findOrInsertTempBlobIndex(SB));
    auto *UndefRef = DDRU.createUndefDDRef(LiveoutTempRef->getDestType());

    auto *InitInst = HNU.createCopyInst(UndefRef, "undefinit", LiveoutTempRef);

    HLNodeUtils::insertBefore(this, InitInst);
  }
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
      PeelFactorShr->getLvalDDRef()->clone(), hasSignedIV());
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
  extractZttPreheaderAndPostexit();

  undefInitializeUnconditionalLiveoutTemps();

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

OptReport OptReportTraits<HLLoop>::getOrCreatePrevOptReport(
    HLLoop &Loop, const OptReportBuilder &Builder) {

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

OptReport OptReportTraits<HLLoop>::getOrCreateParentOptReport(
    HLLoop &Loop, const OptReportBuilder &Builder) {
  if (HLLoop *Dest = Loop.getParentLoop())
    return Builder(*Dest).getOrCreateOptReport();

  if (HLRegion *Dest = Loop.getParentRegion())
    return Builder(*Dest).getOrCreateOptReport();

  llvm_unreachable("Failed to find a parent");
}

void OptReportTraits<HLLoop>::traverseChildNodesBackward(HLLoop &Loop,
                                                         NodeVisitorTy Func) {
  struct LoopVisitor : public HLNodeVisitorBase {
    using NodeVisitorTy = OptReportTraits<HLLoop>::NodeVisitorTy;
    NodeVisitorTy Func;

    LoopVisitor(NodeVisitorTy Func) : Func(Func) {}
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

void HLLoop::addInt32LoopMetadata(StringRef ID, unsigned Value) {
  LLVMContext &Context = getHLNodeUtils().getHIRFramework().getContext();

  auto *TCNode = ConstantAsMetadata::get(
      ConstantInt::get(Type::getInt32Ty(Context), Value));

  auto MNode = MDNode::get(Context, {MDString::get(Context, ID), TCNode});

  addLoopMetadata(MNode);
}

void HLLoop::setPragmaBasedMinimumTripCount(unsigned MinTripCount) {
  addInt32LoopMetadata("llvm.loop.intel.loopcount_minimum", MinTripCount);
}

void HLLoop::setPragmaBasedMaximumTripCount(unsigned MaxTripCount) {
  addInt32LoopMetadata("llvm.loop.intel.loopcount_maximum", MaxTripCount);
}

void HLLoop::setPragmaBasedAverageTripCount(unsigned AvgTripCount) {
  addInt32LoopMetadata("llvm.loop.intel.loopcount_average", AvgTripCount);
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

void HLLoop::promoteNestingLevel(unsigned StartLevel) {
  SmallVector<const RegDDRef *, 32> LVals;

  // Use pre-header lval to make body def levels consistent.
  for (auto &Node : make_range(pre_begin(), pre_end())) {
    auto &Inst = cast<HLInst>(Node);
    if (Inst.hasLval()) {
      LVals.push_back(Inst.getLvalDDRef());
    }
  }

  ForEach<RegDDRef>::visitRange(child_begin(), child_end(), [&](RegDDRef *Ref) {
    Ref->promoteIVs(StartLevel);
    promoteDemoteBlobs(Ref, StartLevel, 1);

    Ref->makeConsistent(LVals, StartLevel + 1);
  });
}

/// Returns true if \p SC has either nsw or nuw flag.
static bool isNoWrapIV(const SCEV *SC, const Loop *Lp) {
  auto *AddRec = dyn_cast<SCEVAddRecExpr>(SC);

  if (!AddRec || !AddRec->isAffine() || (AddRec->getLoop() != Lp)) {
    return false;
  }

  return (AddRec->hasNoSignedWrap() || AddRec->hasNoUnsignedWrap());
}

/// Returns true if the IV of \p Lp has either nsw or nuw flag.
static bool hasNoWrapIV(const Loop *Lp, ScalarEvolution &SE) {

  auto *ExitingBB = Lp->getExitingBlock();

  if (!ExitingBB) {
    return false;
  }

  auto *BrInst = dyn_cast<BranchInst>(ExitingBB->getTerminator());

  if (!BrInst || !BrInst->isConditional()) {
    return false;
  }

  auto *Cmp = dyn_cast<ICmpInst>(BrInst->getCondition());

  if (!Cmp) {
    return false;
  }

  auto *SC0 = SE.getSCEV(Cmp->getOperand(0));
  auto *SC1 = SE.getSCEV(Cmp->getOperand(1));

  return (isNoWrapIV(SC0, Lp) || isNoWrapIV(SC1, Lp));
}

/// Returns true if \p Lp has ZTT in LLVM IR.
static bool hasIRZtt(const Loop *Lp, DominatorTree &DT, ScalarEvolution &SE) {

  // Go through dominating blocks of loop with single predecessors to look for
  // ZTT so we know which path (true/false) the loop is in.
  for (auto *DomNode = DT.getNode(Lp->getLoopPreheader()); DomNode != nullptr;
       DomNode = DomNode->getIDom()) {
    auto *BB = DomNode->getBlock();
    auto *DomBB = BB->getSinglePredecessor();

    if (!DomBB) {
      continue;
    }

    auto *BrInst = dyn_cast<BranchInst>(DomBB->getTerminator());

    if (!BrInst || !BrInst->isConditional()) {
      continue;
    }

    bool IsFalseBranch = (BrInst->getSuccessor(0) != BB);

    if (SE.isLoopZtt(Lp, BrInst, IsFalseBranch)) {
      return true;
    }
  }

  return false;
}

bool HLLoop::canTripCountEqualIVTypeRangeSize() const {
  assert(!isUnknown() && "Countable loop expected!");

  // Trip count is known to be less than type range in the following cases-
  // Ztt disallows this case.
  if (hasSignedIV() || isConstTripLoop() || hasZtt()) {
    return false;
  }

  bool IsDoLoop = isDo();
  // If LegalMaxTripCount <= UMAX(IVType), then we know the trip count cannot be
  // equal to type range. Cannot use legal max trip count for multi-exit loops
  // as it may have been refined by ScalarEvolution using early-exits or using
  // nowrap based semantics which may not apply if the loop exits using the
  // early-exit.
  if (IsDoLoop) {
    uint64_t LegalMaxTC = getLegalMaxTripCount();
    if (LegalMaxTC != 0) {
      APInt MaxTypeVal = APInt::getMaxValue(IVType->getScalarSizeInBits());

      if (LegalMaxTC <= MaxTypeVal.getZExtValue()) {
        return false;
      }
    }
  }

  auto *Lp = getLLVMLoop();

  if (!Lp) {
    return true;
  }

  auto &HIRF = getHLNodeUtils().getHIRFramework();
  auto &SE = HIRF.getScopedSE().getOrigSE();

  if (IsDoLoop) {
    // If single exit loops have nowrap IV, then the trip count cannot be
    // equal to type range as it requires wrapping IV.
    // TODO: ScalarEvolution should refine MaxBackedgeTakenCount using nowrap
    // logic which will then be set in LegalMaxTripCount field of HLLoop.
    if (hasNoWrapIV(Lp, SE)) {
      return false;
    }
  }

  // Ztt may not be part of the region (especially for multi-exit loops) or may
  // have been extracted by a transformation. This check helps catch more cases.
  return !hasIRZtt(Lp, HIRF.getDomTree(), SE);
}
