//===-- WRegionNode.cpp - Implements the WRegionNode class ----------------===//
//
//   Copyright (C) 2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
//   This file implements the WRegionNode class.
//   It's the base class for WRN graph nodes, and should never be
//   instantiated directly.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/CFG.h"
#include "llvm/IR/Dominators.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionNode.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionUtils.h"

#define DEBUG_TYPE "vpo-wrnnode"

using namespace llvm;
using namespace llvm::vpo;

unsigned WRegionNode::UniqueNum(0);

std::unordered_map<int, StringRef> llvm::vpo::WRNName = {
    {WRegionNode::WRNParallel, "Parallel"},
    {WRegionNode::WRNParallelLoop, "Parallel Loop"},
    {WRegionNode::WRNParallelSections, "Parallel Sections"},
    {WRegionNode::WRNTask, "Task"},
    {WRegionNode::WRNTaskLoop, "Task Loop"},
    {WRegionNode::WRNVecLoop, "Vec Loop"},
    {WRegionNode::WRNWksLoop, "Worksharig Loop"},
    {WRegionNode::WRNWksSections, "Worksharig Sections"},
    {WRegionNode::WRNSection, "Section"},
    {WRegionNode::WRNSingle, "Single"},
    {WRegionNode::WRNMaster, "Master"},
    {WRegionNode::WRNAtomic, "Atomic"},
    {WRegionNode::WRNBarrier, "Barrier"},
    {WRegionNode::WRNCancel, "Cancel"},
    {WRegionNode::WRNCritical, "Critical"},
    {WRegionNode::WRNFlush, "Flush"},
    {WRegionNode::WRNOrdered, "Ordered"},
    {WRegionNode::WRNTaskgroup, "Taskgroup"}};

WRegionNode *ilist_traits<WRegionNode>::createSentinel() const {
      return static_cast<WRegionNode *>(&Sentinel);
}

// constructor for LLVM IR representation
WRegionNode::WRegionNode(unsigned SCID, BasicBlock *BB)
    : SubClassID(SCID), EntryBBlock(BB) {
  setNextNumber();
  setParent(nullptr);
  setExitBBlock(nullptr);
  setIsFromHIR(false);
  resetBBSet();
}

// constructor for HIR representation
WRegionNode::WRegionNode(unsigned SCID) : SubClassID(SCID) {
  setNextNumber();
  setParent(nullptr);
  setEntryBBlock(nullptr);
  setExitBBlock(nullptr);
  resetBBSet();
  setIsFromHIR(true);
}

WRegionNode::WRegionNode(WRegionNode *W) : SubClassID(W->SubClassID) {
  setNextNumber(); // can't reuse the same number; get a new one
  setParent(W->getParent());
  setEntryBBlock(W->getEntryBBlock());
  setExitBBlock(W->getExitBBlock());
  setIsFromHIR(W->getIsFromHIR());
  resetBBSet();
  // TODO: add code to copy Children?
}

/// \brief Populates BBlockSet with BBs in the WRN from EntryBB to ExitBB.
void WRegionNode::populateBBSet(void) {
  BasicBlock *EntryBB = getEntryBBlock();
  BasicBlock *ExitBB = getExitBBlock();

  assert(EntryBB && "Missing EntryBB!");
  assert(ExitBB && "Missing ExitBB!");

  IntelGeneralUtils::collectBBSet(EntryBB, ExitBB, BBlockSet);
}

// After CFGRestructuring, the EntryBB should have a single predecessor
BasicBlock *WRegionNode::getPredBBlock() const {
  auto PredI = pred_begin(EntryBBlock);
  auto TempPredI = PredI;
  ++TempPredI;
  assert((TempPredI == pred_end(EntryBBlock)) &&
         "Region has more than one predecessor!");
  return *PredI;
}

// After CFGRestructuring, the ExitBB should have a single successor
BasicBlock *WRegionNode::getSuccBBlock() const {
  auto SuccI = succ_begin(ExitBBlock);
  auto TempSuccI = SuccI;
  ++TempSuccI;

  assert((TempSuccI == succ_end(ExitBBlock)) &&
         "Region has more than one successor!");
  return *SuccI;
}

WRegionNode *WRegionNode::getFirstChild() {
  if (hasChildren()) {
    return &*(Children.begin());
  }
  return nullptr;
}

WRegionNode *WRegionNode::getLastChild() {
  if (hasChildren()) {
    return &(Children.back());
  }
  return nullptr;
}

void WRegionNode::printChildren(formatted_raw_ostream &OS,
                                unsigned Depth) const
 {
  for (auto I = wrn_child_begin(), E = wrn_child_end(); I != E; ++I) {
    I->print(OS, Depth);
  }
}

void WRegionNode::destroy() {
  // TODO: call destructor
}

void WRegionNode::destroyAll() {
  // TODO: implement this by recursive walk from top
}
void WRegionNode::dump() const {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  formatted_raw_ostream OS(dbgs());
  print(OS, 0);
#endif
}


//
// functions below are used to update WRNs with clause information
// TODO: Complete implementation as more WRN kinds are supported
//
void WRegionNode::handleQual(int ClauseID) {
  switch (ClauseID) {
  case QUAL_OMP_DEFAULT_NONE:
    setDefault(WRNDefaultNone);
    break;
  case QUAL_OMP_DEFAULT_SHARED:
    setDefault(WRNDefaultShared);
    break;
  case QUAL_OMP_DEFAULT_PRIVATE:
    setDefault(WRNDefaultPrivate);
    break;
  case QUAL_OMP_DEFAULT_FIRSTPRIVATE:
    setDefault(WRNDefaultFirstprivate);
    break;
  case QUAL_OMP_NOWAIT:
    break;
  case QUAL_OMP_UNTIED:
    break;
  case QUAL_OMP_READ_SEQ_CST:
    setHasSeqCstClause(true);
  case QUAL_OMP_READ:
    setAtomicKind(WRNAtomicRead);
    break;
  case QUAL_OMP_WRITE_SEQ_CST:
    setHasSeqCstClause(true);
  case QUAL_OMP_WRITE:
    setAtomicKind(WRNAtomicWrite);
    break;
  case QUAL_OMP_UPDATE_SEQ_CST:
    setHasSeqCstClause(true);
  case QUAL_OMP_UPDATE:
    setAtomicKind(WRNAtomicUpdate);
    break;
  case QUAL_OMP_CAPTURE_SEQ_CST:
    setHasSeqCstClause(true);
  case QUAL_OMP_CAPTURE:
    setAtomicKind(WRNAtomicCapture);
    break;
  case QUAL_OMP_MERGEABLE:
    break;
  case QUAL_OMP_NOGROUP:
    break;
  case QUAL_OMP_SCHEDULE_AUTO:
    break;
  case QUAL_OMP_SCHEDULE_RUNTIME:
    break;
  case QUAL_OMP_PROC_BIND_MASTER:
    break;
  case QUAL_OMP_PROC_BIND_CLOSE:
    break;
  case QUAL_OMP_PROC_BIND_SPREAD:
    break;
  case QUAL_LIST_END:
    break;
  default:
    llvm_unreachable("Unknown ClauseID in handleQual()");
  }
}

void WRegionNode::handleQualOpnd(int ClauseID, Value *V) {
  // for clauses whose parameter are constant integer exprs,
  // we store the information as an int rather than a Value*,
  // so we must extract the integer N from V and store N.
  int64_t N = -1;
  ConstantInt *CI = dyn_cast<ConstantInt>(V);
  if (CI != nullptr) {
    N = *((CI->getValue()).getRawData());
  }

  switch (ClauseID) {
  case QUAL_OMP_SIMDLEN:
    assert(N > 0 && "SIMDLEN must be positive");
    setSimdlen(N);
    break;
  case QUAL_OMP_SAFELEN:
    assert(N > 0 && "SAFELEN must be positive");
    setSafelen(N);
    break;
  case QUAL_OMP_COLLAPSE:
    assert(N > 0 && "COLLAPSE parameter must be positive");
    setCollapse(N);
    break;
  case QUAL_OMP_IF:
    setIf(V);
    break;
  case QUAL_OMP_NUM_THREADS:
    setNumThreads(V);
    break;
  case QUAL_OMP_ORDERED:
    break;
  case QUAL_OMP_FINAL:
    break;
  case QUAL_OMP_GRAINSIZE:
    break;
  case QUAL_OMP_NUM_TASKS:
    break;
  case QUAL_OMP_PRIORITY:
    break;
  case QUAL_OMP_NUM_TEAMS:
    break;
  case QUAL_OMP_THREAD_LIMIT:
    break;
  case QUAL_OMP_DIST_SCHEDULE_STATIC:
    break;
  case QUAL_OMP_SCHEDULE_STATIC:
    break;
  case QUAL_OMP_SCHEDULE_DYNAMIC:
    break;
  case QUAL_OMP_SCHEDULE_GUIDED:
    break;
  default:
    llvm_unreachable("Unknown ClauseID in handleQualOpnd()");
  }
}

#if 1
// TODO: investigate/fix this build issue
// Moved this here from WRegionUtils.cpp
// Having this in WRegionUtils.cpp caused link error:
//   undefined reference to `llvm::vpo::Clause<llvm::vpo::PrivateItem>*
//   llvm::vpo::WRegionUtils::extractQualOpndList<llvm::vpo::PrivateItem>(
//   llvm::IntrinsicInst*, llvm::vpo::Clause<llvm::vpo::PrivateItem>*)'

template <typename ClauseTy>
ClauseTy *WRegionUtils::extractQualOpndList(IntrinsicInst *Call, ClauseTy *C) {
  if (C == nullptr) {
    StringRef DirString = VPOUtils::getDirectiveMetadataString(Call);
    int ClauseID = VPOUtils::getClauseID(DirString);
    C = new ClauseTy();
    C->setClauseID(ClauseID);
  }

  // Skip argument(0) as it is the metadata
  for (unsigned I = 1; I < Call->getNumArgOperands(); ++I) {
    Value *V = Call->getArgOperand(I);
    C->add(V);
  }
  return C;
}

ReductionClause *WRegionUtils::extractReductionOpndList(IntrinsicInst *Call, 
                                                        ReductionClause *C,
                                                        int ReductionKind) {
  if (C == nullptr) {
    C = new ReductionClause();
    C->setClauseID(QUAL_OMP_REDUCTION_ADD); // dummy reduction op
  }

  // Skip argument(0) as it is the metadata
  for (unsigned I = 1; I < Call->getNumArgOperands(); ++I) {
    Value *V = Call->getArgOperand(I);
    C->add(V);
    ReductionItem *RI = C->back();
    RI->setType((ReductionItem::WRNReductionKind)ReductionKind);
  }
  return C;
}
#endif


//
// The code below was trying to get initializer and combiner from the LLVM IR.
// However, only UDR requires initializer and combiner function pointers, and
// the front-end has to provide them. 
// For standard OpenMP reduction operations(ie, not UDR), the combiner and 
// initializer operations are implied by the reduction operation and type
// of the reduction variable.
// Therefore, we should never have to use the code below.
//
#if 0
/// \brief Fill reduction info in ReductionItem \pRI
static void setReductionItem(ReductionItem *RI, IntrinsicInst *Call) {
  auto usedInOnlyOnePhiNode = [](Value *V) {
    PHINode *Phi = 0;
    for (auto U : V->users())
      if (isa<PHINode>(U)) {
        if (Phi) // More than one Phi node
          return (PHINode *)nullptr;
        Phi = cast<PHINode>(U);
      }
    return Phi;
  };

  Value *RedVarPtr = RI->getOrig();      
  assert(isa<PointerType>(RedVarPtr->getType()) &&
         "Variable specified in Reduction directive should be a pointer");

  for (auto U : RedVarPtr->users()) {
    if (!isa<LoadInst>(U))
      continue;
    if (auto PhiNode = usedInOnlyOnePhiNode(U)) {
      RI->setInitializer(U);
      if (PhiNode->getIncomingValue(0) == U)
        RI->setCombiner(PhiNode->getIncomingValue(1));
      else
        RI->setCombiner(PhiNode->getIncomingValue(0));
      break;
    }
  }
  assert(RI->getInitializer() && RI->getCombiner() &&
         "Reduction Item is not initialized");
  StringRef DirString = VPOUtils::getDirectiveMetadataString(Call);
  int ReductionClauseID = VPOUtils::getClauseID(DirString);
  RI->setType(ReductionItem::getKindFromClauseId(ReductionClauseID));
}
#endif


//
// TODO1: This implementation does not yet support nonPOD and array section
//        clause items. It also does not support the optional arguments at
//        the end of linear and aligned clauses.  We'll do that later.
//
// TODO2: Complete the cases in the switch to handle all list-type clauses
//
void WRegionNode::handleQualOpndList(int ClauseID, IntrinsicInst *Call) {
  switch (ClauseID) {
  case QUAL_OMP_SHARED: {
    SharedClause *C =
        WRegionUtils::extractQualOpndList<SharedClause>(Call, getShared());
    setShared(C);
    break;
  }
  case QUAL_OMP_PRIVATE: {
    PrivateClause *C =
        WRegionUtils::extractQualOpndList<PrivateClause>(Call, getPriv());
    setPriv(C);
    break;
  }
  case QUAL_OMP_FIRSTPRIVATE: {
    FirstprivateClause *C =
        WRegionUtils::extractQualOpndList<FirstprivateClause>(Call, getFpriv());
    setFpriv(C);
    break;
  }
  case QUAL_OMP_LASTPRIVATE: {
    LastprivateClause *C =
        WRegionUtils::extractQualOpndList<LastprivateClause>(Call, getLpriv());
    setLpriv(C);
    break;
  }
  case QUAL_OMP_LINEAR: {
    LinearClause *C =
        WRegionUtils::extractQualOpndList<LinearClause>(Call, getLinear());
    setLinear(C);
    break;
  }
  case QUAL_OMP_ALIGNED: {
    AlignedClause *C =
        WRegionUtils::extractQualOpndList<AlignedClause>(Call, getAligned());
    setAligned(C);
    break;
  }
  case QUAL_OMP_REDUCTION_ADD:
  case QUAL_OMP_REDUCTION_SUB:
  case QUAL_OMP_REDUCTION_MUL:
  case QUAL_OMP_REDUCTION_AND:
  case QUAL_OMP_REDUCTION_OR:
  case QUAL_OMP_REDUCTION_XOR:
  case QUAL_OMP_REDUCTION_BAND:
  case QUAL_OMP_REDUCTION_BOR:
  case QUAL_OMP_REDUCTION_UDR: {
    int ReductionKind = ReductionItem::getKindFromClauseId(ClauseID);
    assert(ReductionKind > 0 && "Bad reduction operation");
    ReductionClause *C =
        WRegionUtils::extractReductionOpndList(Call, getRed(), ReductionKind);
    setRed(C);

    //don't call this: 
    // setReductionItem(C->back(), Call);
    break;
  }
  default:
    llvm_unreachable("Unknown ClauseID in handleQualOpndList()");
    break;
  }
}

StringRef WRegionNode::getName() const {
  // good return llvm::vpo::WRNName[getWRegionKindID()];
  return WRNName[getWRegionKindID()];
}

void WRegionNode::errorClause(StringRef ClauseName) const {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  formatted_raw_ostream OS(dbgs());
  OS << "Error: " << getName() << " WRNs do not take " << ClauseName
     << " clauses.\n";
  // Example:
  // Error: Vec Loop WRNs do not take SHARED clauses.
  llvm_unreachable("Unexpected clause encountered!");
#endif
}

void WRegionNode::errorClause(int ClauseID) const {
  StringRef ClauseName = VPOUtils::getClauseName(ClauseID);
  errorClause(ClauseName);
}
