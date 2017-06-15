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
    {WRegionNode::WRNParallel, "parallel"},
    {WRegionNode::WRNParallelLoop, "parallel loop"},
    {WRegionNode::WRNParallelSections, "parallel sections"},
    {WRegionNode::WRNParallelWorkshare, "parallel workshare"},
    {WRegionNode::WRNTeams, "teams"},
    {WRegionNode::WRNDistributeParLoop, "distribute parallel loop"},
    {WRegionNode::WRNTarget, "target"},
    {WRegionNode::WRNTargetData, "target data"},
    {WRegionNode::WRNTask, "task"},
    {WRegionNode::WRNTaskloop, "taskloop"},
    {WRegionNode::WRNVecLoop, "simd"},
    {WRegionNode::WRNWksLoop, "loop"},
    {WRegionNode::WRNSections, "sections"},
    {WRegionNode::WRNWorkshare, "workshare"},
    {WRegionNode::WRNDistribute, "distribute"},
    {WRegionNode::WRNSingle, "single"},
    {WRegionNode::WRNMaster, "master"},
    {WRegionNode::WRNAtomic, "atomic"},
    {WRegionNode::WRNBarrier, "barrier"},
    {WRegionNode::WRNCancel, "cancel"},
    {WRegionNode::WRNCritical, "critical"},
    {WRegionNode::WRNFlush, "flush"},
    {WRegionNode::WRNOrdered, "ordered"},
    {WRegionNode::WRNTaskgroup, "taskgroup"},
    {WRegionNode::WRNTaskwait, "taskwait"},
    {WRegionNode::WRNTaskyield, "taskyield"}};

// constructor for LLVM IR representation
WRegionNode::WRegionNode(unsigned SCID, BasicBlock *BB)
    : SubClassID(SCID), Attributes(0), EntryBBlock(BB) {
  setNextNumber();
  setParent(nullptr);
  setExitBBlock(nullptr);
  setIsFromHIR(false);
  resetBBSet();
}

// constructor for HIR representation
WRegionNode::WRegionNode(unsigned SCID) : SubClassID(SCID), Attributes(0) {
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
  setAttributes(W->getAttributes());
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
    return *(Children.begin());
  }
  return nullptr;
}

WRegionNode *WRegionNode::getLastChild() {
  if (hasChildren()) {
    return Children.back();
  }
  return nullptr;
}

void WRegionNode::printChildren(formatted_raw_ostream &OS,
                                unsigned Depth) const
 {
#if !INTEL_PRODUCT_RELEASE
  for (auto I = wrn_child_begin(), E = wrn_child_end(); I != E; ++I) {
    (*I)->print(OS, Depth);
  }
#endif // !INTEL_PRODUCT_RELEASE
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
  case QUAL_OMP_DEFAULTMAP_TOFROM_SCALAR:
    setDefaultmapTofromScalar(true);
    break;
  case QUAL_OMP_NOWAIT:
    setNowait(true);
    break;
  case QUAL_OMP_UNTIED:
    setUntied(true);
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
    setMergeable(true);
    break;
  case QUAL_OMP_NOGROUP:
    setNogroup(true);
    break;
  case QUAL_OMP_PROC_BIND_MASTER:
    setProcBind(WRNProcBindMaster);
    break;
  case QUAL_OMP_PROC_BIND_CLOSE:
    setProcBind(WRNProcBindClose);
    break;
  case QUAL_OMP_PROC_BIND_SPREAD:
    setProcBind(WRNProcBindSpread);
    break;
  case QUAL_OMP_ORDERED_THREADS:
    setIsDoacross(false);
    setIsThreads(true);
    break;
  case QUAL_OMP_ORDERED_SIMD:
    setIsDoacross(false);
    setIsThreads(false);
    break;
  case QUAL_OMP_DEPEND_SOURCE:
    setIsDoacross(true);
    setIsDepSource(true);
    break;
  case QUAL_OMP_CANCEL_PARALLEL:
    setCancelKind(WRNCancelParallel);
    break;
  case QUAL_OMP_CANCEL_LOOP:
    setCancelKind(WRNCancelLoop);
    break;
  case QUAL_OMP_CANCEL_SECTIONS:
    setCancelKind(WRNCancelSections);
    break;
  case QUAL_OMP_CANCEL_TASKGROUP:
    setCancelKind(WRNCancelTaskgroup);
    break;
  case QUAL_LIST_END: //TODO: remove this obsolete case
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
  case QUAL_OMP_NAME: {
    // The operand is expected to be a constant string. Example:
    // call void @llvm.intel.directive.qual.opnd.a9i8(metadata
    // !"QUAL.OMP.NAME", [9 x i8] c"lock_name")
    ConstantDataSequential *CD = dyn_cast<ConstantDataSequential>(V);
    assert((CD != nullptr && (CD->isString() || CD->isCString())) &&
           "QUAL_OMP_NAME opnd should be a constant string.");

    if (CD->isCString()) // Process as C string first, so that the nul
                         // bytes at the end are ignored. (e.g. c"lock_name\00")
      setUserLockName(CD->getAsCString());
    else if (CD->isString()) // Process as a regular string. (e.g. c"lock_name")
      setUserLockName(CD->getAsString());

  } break;
  case QUAL_OMP_NUM_THREADS:
    setNumThreads(V);
    break;
  case QUAL_OMP_ORDERED:
    assert(N > 0 && "ORDERED parameter must be positive");
    setOrdered(N);
    break;
  case QUAL_OMP_FINAL:
    setFinal(V);
    break;
  case QUAL_OMP_GRAINSIZE:
    setGrainsize(V);
    break;
  case QUAL_OMP_NUM_TASKS:
    setNumTasks(V);
    break;
  case QUAL_OMP_PRIORITY:
    setPriority(V);
    break;
  case QUAL_OMP_NUM_TEAMS:
    setNumTeams(V);
    break;
  case QUAL_OMP_THREAD_LIMIT:
    setThreadLimit(V);
    break;
  case QUAL_OMP_DEVICE:
    setDevice(V);
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
    StringRef DirString = VPOAnalysisUtils::getDirectiveMetadataString(Call);
    int ClauseID = VPOAnalysisUtils::getClauseID(DirString);
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

void WRegionUtils::extractScheduleOpndList(ScheduleClause & Sched,
                                           IntrinsicInst *Call,
                                           WRNScheduleKind Kind) {
  // save the schedule kind
  Sched.setKind(Kind);

  // extract and save the chunk size expr 
  Value *V = Call->getArgOperand(2);
  Sched.setChunkExpr(V);

  // if chunk size is constant, extract and save the constant chunk size
  int64_t ChunkSize = 0;
  ConstantInt *CI = dyn_cast<ConstantInt>(V);
  if (CI != nullptr) { 
    ChunkSize = *((CI->getValue()).getRawData());
    DEBUG(dbgs() << " Schedule chunk size is constant: " << ChunkSize << "\n");
  }
  Sched.setChunk(ChunkSize);

  // extract and save the schedule modifiers 
  StringRef ModifierString = 
                           VPOAnalysisUtils::getScheduleModifierMDString(Call);
  DEBUG(dbgs() << " Schedule Modifier Argument: " << ModifierString << "\n");

  SmallVector<StringRef, 4> ModifierSubString;
  ModifierString.split(ModifierSubString, ".");

  DEBUG(dbgs() << "   Modifier SubString: " << ModifierSubString[0] << "\n");
  DEBUG(dbgs() << "   Modifier SubString: " << ModifierSubString[1] << "\n");

  for (int i=0; i<2; i++) {
    // Max number of substrings is 2, because monotonic and nonmonotonic
    // are mutually exclusive
    if (ModifierSubString[i] == "MONOTONIC") {
      Sched.setIsSchedMonotonic(true);
    } else if (ModifierSubString[i] == "NONMONOTONIC") {
      Sched.setIsSchedNonmonotonic(true);
    } else if (ModifierSubString[i] == "SIMD") {
      Sched.setIsSchedSimd(true);
    };
  }

  return;
}

MapClause *WRegionUtils::extractMapOpndList(IntrinsicInst *Call, 
                                            MapClause *C,
                                            unsigned MapKind) {
  if (C == nullptr) {
    C = new MapClause();
    C->setClauseID(QUAL_OMP_MAP_TO); // dummy map clause id; details are in 
                                     // the MapKind of each list item
  }

  // Skip argument(0) as it is the metadata
  for (unsigned I = 1; I < Call->getNumArgOperands(); ++I) {
    Value *V = Call->getArgOperand(I);
    C->add(V);
    MapItem *MI = C->back();
    MI->setMapKind(MapKind);
  }
  return C;
}

DependClause *WRegionUtils::extractDependOpndList(IntrinsicInst *Call, 
                                                  DependClause *C,
                                                  bool IsIn) {
  if (C == nullptr) {
    C = new DependClause();
    C->setClauseID(QUAL_OMP_DEPEND_IN); // dummy depend clause id; 
  }

  // Skip argument(0) as it is the metadata
  for (unsigned I = 1; I < Call->getNumArgOperands(); ++I) {
    Value *V = Call->getArgOperand(I);

    //TODO: Parse array section arguments.
    //      Currently only scalar vars are supported.
    C->add(V);
    DependItem *DI = C->back();
    DI->setIsIn(IsIn);
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
  StringRef DirString = VPOAnalysisUtils::getDirectiveMetadataString(Call);
  int ReductionClauseID = VPOAnalysisUtils::getClauseID(DirString);
  RI->setType(ReductionItem::getKindFromClauseId(ReductionClauseID));
}
#endif


//
// TODO1: This implementation does not yet support nonPOD and array section
//        clause items. It also does not support the optional arguments at
//        the end of linear and aligned clauses.  We'll do that later.
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
  case QUAL_OMP_COPYIN: {
    CopyinClause *C =
      WRegionUtils::extractQualOpndList<CopyinClause>(Call, getCopyin());
    setCopyin(C);
    break;
  }
  case QUAL_OMP_COPYPRIVATE: {
    CopyprivateClause *C =
      WRegionUtils::extractQualOpndList<CopyprivateClause>(Call, getCpriv());
    setCpriv(C);
    break;
  }
  case QUAL_OMP_DEPEND_IN:
  case QUAL_OMP_DEPEND_OUT:
  case QUAL_OMP_DEPEND_INOUT: {
    bool IsIn = ClauseID==QUAL_OMP_DEPEND_IN;
    DependClause *C =
      WRegionUtils::extractDependOpndList(Call, getDepend(), IsIn);
    setDepend(C);
    break;
  }
  case QUAL_OMP_DEPEND_SINK: {
    setIsDoacross(true);
    DepSinkClause *C =
      WRegionUtils::extractQualOpndList<DepSinkClause>(Call, getDepSink());
    setDepSink(C);
    break;
  }
  case QUAL_OMP_IS_DEVICE_PTR: {
    IsDevicePtrClause *C=WRegionUtils::extractQualOpndList<IsDevicePtrClause>
                         (Call, getIsDevicePtr());
    setIsDevicePtr(C);
    break;
  }
  case QUAL_OMP_USE_DEVICE_PTR: {
    UseDevicePtrClause *C=WRegionUtils::extractQualOpndList<UseDevicePtrClause>
                          (Call, getUseDevicePtr());
    setUseDevicePtr(C);
    break;
  }
  case QUAL_OMP_TO:
  case QUAL_OMP_FROM:
  case QUAL_OMP_MAP_TO:
  case QUAL_OMP_MAP_FROM:
  case QUAL_OMP_MAP_TOFROM:
  case QUAL_OMP_MAP_ALLOC:
  case QUAL_OMP_MAP_RELEASE:
  case QUAL_OMP_MAP_DELETE:
  case QUAL_OMP_MAP_ALWAYS_TO:
  case QUAL_OMP_MAP_ALWAYS_FROM:
  case QUAL_OMP_MAP_ALWAYS_TOFROM:
  case QUAL_OMP_MAP_ALWAYS_ALLOC:
  case QUAL_OMP_MAP_ALWAYS_RELEASE:
  case QUAL_OMP_MAP_ALWAYS_DELETE: {
    unsigned MapKind = MapItem::getMapKindFromClauseId(ClauseID);
    MapClause *C =
      WRegionUtils::extractMapOpndList(Call, getMap(), MapKind);
    setMap(C);
    break;
  }
  case QUAL_OMP_UNIFORM: {
    UniformClause *C =
      WRegionUtils::extractQualOpndList<UniformClause>(Call, getUniform());
    setUniform(C);
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
  case QUAL_OMP_FLUSH: {
    FlushSet *C =
      WRegionUtils::extractQualOpndList<FlushSet>(Call, getFlush());
    setFlush(C);
    break;
  }
  case QUAL_OMP_SCHEDULE_AUTO: {
    WRegionUtils::extractScheduleOpndList(getSchedule(), Call, 
                                          WRNScheduleAuto);
    break;
  }
  case QUAL_OMP_SCHEDULE_DYNAMIC: {
    WRegionUtils::extractScheduleOpndList(getSchedule(), Call, 
                                          WRNScheduleDynamic);
    break;
  }
  case QUAL_OMP_SCHEDULE_GUIDED: {
    WRegionUtils::extractScheduleOpndList(getSchedule(), Call, 
                                          WRNScheduleGuided);
    break;
  }
  case QUAL_OMP_SCHEDULE_RUNTIME: {
    WRegionUtils::extractScheduleOpndList(getSchedule(), Call, 
                                          WRNScheduleRuntime);
    break;
  }
  case QUAL_OMP_DIST_SCHEDULE_STATIC:
  case QUAL_OMP_SCHEDULE_STATIC: {
    WRegionUtils::extractScheduleOpndList(getSchedule(), Call, 
                                          WRNScheduleStatic);
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
  // Error: simd WRNs do not take SHARED clauses.
  llvm_unreachable("Unexpected clause encountered!");
#endif
}

void WRegionNode::errorClause(int ClauseID) const {
  StringRef ClauseName = VPOAnalysisUtils::getClauseName(ClauseID);
  errorClause(ClauseName);
}
