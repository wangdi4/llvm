#if INTEL_COLLAB
//===----- WRegion.cpp - Implements the WRegion class ---------------------===//
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
//   This file implements the derived classes based on WRegionNode
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Constants.h"
#include "llvm/Analysis/Intel_VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionUtils.h"
#include "llvm/Transforms/Utils/Intel_GeneralUtils.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-wregion"

//
// Methods for WRNLoopInfo
//
//
Value* WRNLoopInfo::getNormIV(unsigned I) const {
  assert (NormIV.size() > 0 && "getNormIV: empty vector");
  assert (I < NormIV.size() && "getNormIV: bad idx");
  return NormIV[I];
}

Value* WRNLoopInfo::getNormUB(unsigned I) const {
  assert (NormUB.size() > 0 && "getNormUB: empty vector");
  assert (I < NormUB.size() && "getNormUB: bad idx");
  return NormUB[I];
}

void WRNLoopInfo::print(formatted_raw_ostream &OS, unsigned Depth,
                        unsigned Verbosity) const {
  int Ind = 2*Depth;
  Loop *L = getLoop();

  if (!L) {
    OS.indent(Ind) << "Loop is missing; may be optimized away.\n";
    return;
  }

  vpo::printBB("Loop Preheader", L->getLoopPreheader(), OS, Ind, Verbosity);
  vpo::printBB("Loop Header", L->getHeader(), OS, Ind, Verbosity);
  vpo::printBB("Loop Latch", L->getLoopLatch(), OS, Ind, Verbosity);
  vpo::printBB("Loop ZTTBB", getZTTBB(), OS, Ind, Verbosity);

  OS << "\n";
}

//
// Methods for WRNParallelNode
//

// constructor
WRNParallelNode::WRNParallelNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNParallel, BB) {
  setIsPar();
  setIf(nullptr);
  setNumThreads(nullptr);
  setDefault(WRNDefaultAbsent);
  setProcBind(WRNProcBindAbsent);
  LLVM_DEBUG(dbgs() << "\nCreated WRNParallelNode<" << getNumber() << ">\n");
}

// printer
void WRNParallelNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                 unsigned Verbosity) const {
  vpo::printExtraForParallel(this, OS, Depth, Verbosity);
  vpo::printExtraForCancellationPoints(this, OS, Depth, Verbosity);
}

//
// Methods for WRNParallelLoopNode
//

// constructor
WRNParallelLoopNode::WRNParallelLoopNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNParallelLoop, BB), WRNLI(Li) {
  setIsPar();
  setIsOmpLoop();
  setIf(nullptr);
  setNumThreads(nullptr);
  setDefault(WRNDefaultAbsent);
  setProcBind(WRNProcBindAbsent);
  setCollapse(0);
  setOrdered(-1);

  LLVM_DEBUG(dbgs() << "\nCreated WRNParallelLoopNode<" << getNumber()
                    << ">\n");
}

// printer
void WRNParallelLoopNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                     unsigned Verbosity) const {
  // Print the union of WRNParallel's and WRNWksLoop's extra fields
  // minus the Nowait field
  vpo::printExtraForParallel(this, OS, Depth, Verbosity);
  vpo::printExtraForOmpLoop(this, OS, Depth, Verbosity);
  vpo::printExtraForCancellationPoints(this, OS, Depth, Verbosity);
}

//
// Methods for WRNParallelSectionsNode
//

// constructor
WRNParallelSectionsNode::WRNParallelSectionsNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNParallelSections, BB), WRNLI(Li) {
  setIsPar();
  setIsOmpLoop();
  setIsSections();
  setIf(nullptr);
  setNumThreads(nullptr);
  setDefault(WRNDefaultAbsent);
  setProcBind(WRNProcBindAbsent);

  LLVM_DEBUG(dbgs() << "\nCreated WRNParallelSectionsNode<" << getNumber()
                    << ">\n");
}

// printer
void WRNParallelSectionsNode::printExtra(formatted_raw_ostream &OS,
                              unsigned Depth, unsigned Verbosity) const {
  // identical extra fields as WRNParallel
  vpo::printExtraForParallel(this, OS, Depth, Verbosity);
  vpo::printExtraForCancellationPoints(this, OS, Depth, Verbosity);
}

//
// Methods for WRNParallelWorkshareNode
//

// constructor
WRNParallelWorkshareNode::WRNParallelWorkshareNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNParallelWorkshare, BB), WRNLI(Li) {
  setIsPar();
  setIsOmpLoop();
  setIf(nullptr);
  setNumThreads(nullptr);
  setDefault(WRNDefaultAbsent);
  setProcBind(WRNProcBindAbsent);
  LLVM_DEBUG(dbgs() << "\nCreated WRNParallelWorkshareNode<" << getNumber()
                    << ">\n");
}

// printer
void WRNParallelWorkshareNode::printExtra(formatted_raw_ostream &OS,
                               unsigned Depth, unsigned Verbosity) const {
  // identical extra fields as WRNParallel
  vpo::printExtraForParallel(this, OS, Depth, Verbosity);
}

//
// Methods for WRNTeamsNode
//

// constructor
WRNTeamsNode::WRNTeamsNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNTeams, BB) {
  setIsTeams();
  setThreadLimit(nullptr);
  setNumTeams(nullptr);
  setDefault(WRNDefaultAbsent);

  LLVM_DEBUG(dbgs() << "\nCreated WRNTeamsNode<" << getNumber() << ">\n");
}

// printer
void WRNTeamsNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                              unsigned Verbosity) const {
  unsigned Indent = 2 * Depth;
  vpo::printVal("THREAD_LIMIT", getThreadLimit(), OS, Indent, Verbosity);
  vpo::printVal("NUM_TEAMS", getNumTeams(), OS, Indent, Verbosity);
  vpo::printStr("DEFAULT", WRNDefaultName[getDefault()], OS, Indent,
                Verbosity);
}

//
// Methods for WRNDistributeParLoopNode
//

// constructor
WRNDistributeParLoopNode::WRNDistributeParLoopNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNDistributeParLoop, BB), WRNLI(Li) {
  setIsDistribute();
  setIsPar();
  setIsOmpLoop();
  setIf(nullptr);
  setNumThreads(nullptr);
  setDefault(WRNDefaultAbsent);
  setProcBind(WRNProcBindAbsent);
  setCollapse(0);
  setOrdered(-1);

  LLVM_DEBUG(dbgs() << "\nCreated WRNDistributeParLoopNode<" << getNumber()
                    << ">\n");
}

// printer
void WRNDistributeParLoopNode::printExtra(formatted_raw_ostream &OS,
                               unsigned Depth, unsigned Verbosity) const {
  // Similar to WRNParallelLoopNode::printExtra
  vpo::printExtraForParallel(this, OS, Depth, Verbosity);
  vpo::printExtraForOmpLoop(this, OS, Depth, Verbosity);
}

//
// Methods for WRNTargetNode
//

// constructor
WRNTargetNode::WRNTargetNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNTarget, BB) {
  setIsTarget();
  setIf(nullptr);
  setDevice(nullptr);
  setNowait(false);
  setDefaultmapTofromScalar(false);

  LLVM_DEBUG(dbgs() << "\nCreated WRNTargetNode<" << getNumber() << ">\n");
}

// printer
void WRNTargetNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                               unsigned Verbosity) const {
  vpo::printExtraForTarget(this, OS, Depth, Verbosity);
}

//
// Methods for WRNTargetDataNode
//

// constructor
WRNTargetDataNode::WRNTargetDataNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNTargetData, BB) {
  setIsTarget();
  setIf(nullptr);
  setDevice(nullptr);

  LLVM_DEBUG(dbgs() << "\nCreated WRNTargetDataNode<" << getNumber() << ">\n");
}

// printer
void WRNTargetDataNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                   unsigned Verbosity) const {
  vpo::printExtraForTarget(this, OS, Depth, Verbosity);
}

//
// Methods for WRNTargetEnterDataNode
//

// constructor
WRNTargetEnterDataNode::WRNTargetEnterDataNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNTargetEnterData, BB) {
  setIsTarget();
  setIf(nullptr);
  setDevice(nullptr);
  setNowait(false);

  LLVM_DEBUG(dbgs() << "\nCreated WRNTargetEnterDataNode<" << getNumber()
                    << ">\n");
}

// printer
void WRNTargetEnterDataNode::printExtra(formatted_raw_ostream &OS,
                                        unsigned Depth,
                                        unsigned Verbosity) const {
  vpo::printExtraForTarget(this, OS, Depth, Verbosity);
}

//
// Methods for WRNTargetExitDataNode
//

// constructor
WRNTargetExitDataNode::WRNTargetExitDataNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNTargetExitData, BB) {
  setIsTarget();
  setIf(nullptr);
  setDevice(nullptr);
  setNowait(false);

  LLVM_DEBUG(dbgs() << "\nCreated WRNTargetExitDataNode<" << getNumber()
                    << ">\n");
}

// printer
void WRNTargetExitDataNode::printExtra(formatted_raw_ostream &OS,
                                       unsigned Depth,
                                       unsigned Verbosity) const {
  vpo::printExtraForTarget(this, OS, Depth, Verbosity);
}

//
// Methods for WRNTargetUpdateNode
//

// constructor
WRNTargetUpdateNode::WRNTargetUpdateNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNTargetUpdate, BB) {
  setIsTarget();
  setIf(nullptr);
  setDevice(nullptr);
  setNowait(false);

  LLVM_DEBUG(dbgs() << "\nCreated WRNTargetUpdateNode<" << getNumber()
                    << ">\n");
}

// printer
void WRNTargetUpdateNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                     unsigned Verbosity) const {
  vpo::printExtraForTarget(this, OS, Depth, Verbosity);
}

//
// Methods for WRNTaskNode
//

// constructor
WRNTaskNode::WRNTaskNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNTask, BB) {
  setIsTask();
  setIf(nullptr);
  setFinal(nullptr);
  setPriority(nullptr);
  setDefault(WRNDefaultAbsent);
  setUntied(false);
  setMergeable(false);
  setTaskFlag(WRNTaskFlag::Tied);

  LLVM_DEBUG(dbgs() << "\nCreated WRNTaskNode<" << getNumber() << ">\n");
}

// printer
void WRNTaskNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                             unsigned Verbosity) const {
  vpo::printExtraForTask(this, OS, Depth, Verbosity);
  vpo::printExtraForCancellationPoints(this, OS, Depth, Verbosity);
}

//
// Methods for WRNTaskloopNode
//

// constructor
WRNTaskloopNode::WRNTaskloopNode(BasicBlock *BB, LoopInfo *Li)
    : WRNTaskNode(BB), WRNLI(Li) {
  setWRegionKindID(WRegionNode::WRNTaskloop);
  setIsTask();
  setIsOmpLoop();
  setGrainsize(nullptr);
  setIf(nullptr);
  setNumTasks(nullptr);
  setSchedCode(0);
  setCollapse(0);
  setNogroup(false);
  setTaskFlag(WRNTaskFlag::Tied);
  // These are done in WRNTaskNode's constructor
  //   setFinal(nullptr);
  //   setPriority(nullptr);
  //   setDefault(WRNDefaultAbsent);
  //   setUntied(false);
  //   setMergeable(false);

  LLVM_DEBUG(dbgs() << "\nCreated WRNTaskloopNode<" << getNumber() << ">\n");
}

//
// Methods for WRNVecLoopNode
//

// constructor for LLVM IR representation
#if INTEL_CUSTOMIZATION
WRNVecLoopNode::WRNVecLoopNode(BasicBlock *BB, LoopInfo *Li,
                               const bool isAutoVec)
#else
WRNVecLoopNode::WRNVecLoopNode(BasicBlock *BB, LoopInfo *Li)
#endif // INTEL_CUSTOMIZATION
    : WRegionNode(WRegionNode::WRNVecLoop, BB), WRNLI(Li) {
  setIsOmpLoop();
  setSimdlen(0);
  setSafelen(0);
  setCollapse(0);
#if INTEL_CUSTOMIZATION
  setIsAutoVec(isAutoVec);
  setIgnoreProfitability(false);
#endif // INTEL_CUSTOMIZATION

  LLVM_DEBUG(dbgs() << "\nCreated WRNVecLoopNode<" << getNumber() << ">\n");
}

#if INTEL_CUSTOMIZATION
// constructor for HIR representation
WRNVecLoopNode::WRNVecLoopNode(loopopt::HLNode *EntryHLN, const bool isAutoVec)
                                      : WRegionNode(WRegionNode::WRNVecLoop),
                                        WRNLI(nullptr), EntryHLNode(EntryHLN) {
  setIsOmpLoop();
  setSimdlen(0);
  setSafelen(0);
  setCollapse(0);
  setIsAutoVec(isAutoVec);
  setIgnoreProfitability(false);

  setExitHLNode(nullptr);
  setHLLoop(nullptr);

  LLVM_DEBUG(dbgs() << "\nCreated HIR-WRNVecLoopNode<" << getNumber() << ">\n");
}
#endif // INTEL_CUSTOMIZATION

// Specify namespace for the template instantiation or the build will fail
namespace llvm {
namespace vpo {
template <> Loop *WRNVecLoopNode::getTheLoop<Loop>() const {
  return getWRNLoopInfo().getLoop();
}
#if INTEL_CUSTOMIZATION
template <>
loopopt::HLLoop *WRNVecLoopNode::getTheLoop<loopopt::HLLoop>() const {
  return getHLLoop();
}
#endif // INTEL_CUSTOMIZATION
} // vpo
} // llvm

// printer
void WRNVecLoopNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                unsigned Verbosity) const {
  unsigned Indent = 2 * Depth;
  vpo::printInt("SIMDLEN", getSimdlen(), OS, Indent, Verbosity);
  vpo::printInt("SAFELEN", getSafelen(), OS, Indent, Verbosity);
  vpo::printInt("COLLAPSE", getCollapse(), OS, Indent, Verbosity);
}

#if INTEL_CUSTOMIZATION
void WRNVecLoopNode::printHIR(formatted_raw_ostream &OS, unsigned Depth,
                              unsigned Verbosity) const {
  if (!getIsFromHIR()) // using LLVM-IR representation; no HIR to print
    return;

  OS.indent(2*Depth) << "EntryHLNode:\n";
  getEntryHLNode()->print(OS, 1);
  if (Verbosity > 0) {
    OS << "\n";
    OS.indent(2*Depth) << "HLLoop:\n";
    getHLLoop()->print(OS, 1);
  }
  OS << "\n";
  OS.indent(2*Depth) << "ExitHLNode:\n";
  getExitHLNode()->print(OS, 1);
}
#endif // INTEL_CUSTOMIZATION

//
// Methods for WRNWksLoopNode
//

// constructor
WRNWksLoopNode::WRNWksLoopNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNWksLoop, BB), WRNLI(Li) {
  setIsOmpLoop();
  setCollapse(0);
  setOrdered(-1);
  setNowait(false);

  LLVM_DEBUG(dbgs() << "\nCreated WRNWksLoopNode<" << getNumber() << ">\n");
}

// printer
void WRNWksLoopNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                unsigned Verbosity) const {
  vpo::printExtraForOmpLoop(this, OS, Depth, Verbosity);
  vpo::printExtraForCancellationPoints(this, OS, Depth, Verbosity);
}

//
// Methods for WRNSectionsNode
//

// constructor
WRNSectionsNode::WRNSectionsNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNSections, BB), WRNLI(Li) {
  setIsOmpLoop();
  setIsSections();
  setNowait(false);

  LLVM_DEBUG(dbgs() << "\nCreated WRNSectionsNode<" << getNumber() << ">\n");
}

// printer
void WRNSectionsNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                 unsigned Verbosity) const {
  vpo::printBool("NOWAIT", getNowait(), OS, 2*Depth, Verbosity);
  vpo::printExtraForCancellationPoints(this, OS, Depth, Verbosity);
}

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
//
// Methods for WRNSectionNode
//

// constructor
WRNSectionNode::WRNSectionNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNSection, BB) {
  LLVM_DEBUG(dbgs() << "\nCreated WRNSectionNode<" << getNumber() << ">\n");
}
#endif // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION

//
// Methods for WRNWorkshareNode
//

// constructor
WRNWorkshareNode::WRNWorkshareNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNWorkshare, BB), WRNLI(Li) {
  setIsOmpLoop();
  setNowait(false);

  LLVM_DEBUG(dbgs() << "\nCreated WRNWorkshareNode<" << getNumber() << ">\n");
}

// printer
void WRNWorkshareNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                  unsigned Verbosity) const {
  vpo::printBool("NOWAIT", getNowait(), OS, 2*Depth, Verbosity);
}

//
// Methods for WRNDistributeNode
//

// constructor
WRNDistributeNode::WRNDistributeNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNDistribute, BB), WRNLI(Li) {
  setIsOmpLoop();
  setIsDistribute();
  setCollapse(0);

  LLVM_DEBUG(dbgs() << "\nCreated WRNDistributeNode<" << getNumber() << ">\n");
}

// printer
void WRNDistributeNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                   unsigned Verbosity) const {
  vpo::printInt("COLLAPSE", getCollapse(), OS, 2*Depth, Verbosity);
}

//
// Methods for WRNAtomicNode
//

// constructor
WRNAtomicNode::WRNAtomicNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNAtomic, BB) {
  setAtomicKind(WRNAtomicUpdate); // Default Atomic Kind is WRNAtomicUpdate
  setHasSeqCstClause(false);

  LLVM_DEBUG(dbgs() << "\nCreated WRNAtomicNode<" << getNumber() << ">\n");
}

//printer
void WRNAtomicNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                               unsigned Verbosity) const {
  unsigned Indent = 2 * Depth;
  vpo::printStr("ATOMIC KIND", WRNAtomicName[getAtomicKind()], OS, Indent,
                Verbosity);
  vpo::printBool("SEQ_CST", getHasSeqCstClause(), OS, Indent, Verbosity);
}

//
// Methods for WRNBarrierNode
//

// constructor
WRNBarrierNode::WRNBarrierNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNBarrier, BB) {
  LLVM_DEBUG(dbgs() << "\nCreated WRNBarrierNode <" << getNumber() << ">\n");
}

//
// Methods for WRNCancelNode
//

// constructor
WRNCancelNode::WRNCancelNode(BasicBlock *BB, bool IsCP)
    : WRegionNode(WRegionNode::WRNCancel, BB), IsCancellationPoint(IsCP) {
  setCancelKind(WRNCancelError);
  setIf(nullptr);
  LLVM_DEBUG(dbgs() << "\nCreated WRNCancelNode <" << getNumber() << ">\n");
}

//printer
void WRNCancelNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                               unsigned Verbosity) const {
  unsigned Indent = 2 * Depth;
  vpo::printBool("IS CANCELLATION POINT", getIsCancellationPoint(), OS, Indent,
                 Verbosity);
  vpo::printStr("CONSTRUCT TO CANCEL", WRNCancelName[getCancelKind()], OS,
                Indent, Verbosity);
  vpo::printVal("IF_EXPR", getIf(), OS, Indent, Verbosity);
}

//
// Methods for WRNMasterNode
//

// constructor
WRNMasterNode::WRNMasterNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNMaster, BB) {
  LLVM_DEBUG(dbgs() << "\nCreated WRNMasterNode <" << getNumber() << ">\n");
}

//
// Methods for WRNOrderedNode
//

// constructor
WRNOrderedNode::WRNOrderedNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNOrdered, BB) {
  setIsDoacross(false);
  setIsThreads(true);
  LLVM_DEBUG(dbgs() << "\nCreated WRNOrderedNode <" << getNumber() << ">\n");
}

//printer
void WRNOrderedNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                unsigned Verbosity) const {
  unsigned Indent = 2 * Depth;
  bool IsDoacross = getIsDoacross();
  // vpo::printBool("IS DOACROSS", IsDoacross, OS, Indent, Verbosity);

  if (IsDoacross) // Depend clauses present for DoAcross
    vpo::printBool("DEPEND(SOURCE)", getIsDepSource(), OS, Indent, Verbosity);
  else {
    // No Depend clauses => not for DoAcross
    StringRef Kind = getIsThreads() ? "THREADS" : "SIMD";
    vpo::printStr("KIND", Kind, OS, Indent, Verbosity);
  }
}

//
// Methods for WRNSingleNode
//

// constructor
WRNSingleNode::WRNSingleNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNSingle, BB), Nowait(false) {
  LLVM_DEBUG(dbgs() << "\nCreated WRNSingleNode <" << getNumber() << ">\n");
}

// printer
void WRNSingleNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                               unsigned Verbosity) const {
  vpo::printBool("NOWAIT", getNowait(), OS, 2*Depth, Verbosity);
}

//
// Methods for WRNCriticalNode
//

// constructor
WRNCriticalNode::WRNCriticalNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNCritical, BB), UserLockName("") {
  // UserLockName is empty by default
  LLVM_DEBUG(dbgs() << "\nCreated WRNCriticalNode <" << getNumber() << ">\n");
}

// printer
void WRNCriticalNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                 unsigned Verbosity) const {
  unsigned Indent = 2 * Depth;
  StringRef Name = UserLockName.empty() ?  "UNSPECIFIED" : getUserLockName();
  vpo::printStr("User Lock Name", Name, OS, Indent, Verbosity);
  //TODO: Add HINT
}

// constructor
WRNFlushNode::WRNFlushNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNFlush, BB) {
  LLVM_DEBUG(dbgs() << "\nCreated WRNFlushNode<" << getNumber() << ">\n");
}

//
// Methods for WRNTaskgroupNode
//

// constructor
WRNTaskgroupNode::WRNTaskgroupNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNTaskgroup, BB) {
  LLVM_DEBUG(dbgs() << "\nCreated WRNTaskgroupNode <" << getNumber() << ">\n");
}

//
// Methods for WRNTaskwaitNode
//

// constructor
WRNTaskwaitNode::WRNTaskwaitNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNTaskwait, BB) {
  LLVM_DEBUG(dbgs() << "\nCreated WRNTaskwaitNode <" << getNumber() << ">\n");
}

//
// Methods for WRNTaskyieldNode
//

// constructor
WRNTaskyieldNode::WRNTaskyieldNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNTaskyield, BB) {
  LLVM_DEBUG(dbgs() << "\nCreated WRNTaskyieldNode <" << getNumber() << ">\n");
}



//
// Auxiliary print routines
//

// Print the fields common to WRNs for which getIsPar()==true.
// Possible constructs are: WRNParallel, WRNParallelLoop,
//                          WRNParallelSections, WRNParallelWorkshare,
// The fields to print are: IfExpr, NumThreads, Default, ProcBind
void vpo::printExtraForParallel(WRegionNode const *W,
                                formatted_raw_ostream &OS, int Depth,
                                unsigned Verbosity) {
  assert(W->getIsPar() &&
         "printExtraForParallel is for WRNs with getIsPar()==true");
  unsigned Indent = 2 * Depth;
  vpo::printVal("IF_EXPR", W->getIf(), OS, Indent, Verbosity);
  vpo::printVal("NUM_THREADS", W->getNumThreads(), OS, Indent, Verbosity);
  vpo::printStr("DEFAULT", WRNDefaultName[W->getDefault()], OS, Indent,
                Verbosity);
  vpo::printStr("PROCBIND", WRNProcBindName[W->getProcBind()], OS, Indent,
                Verbosity);
}

void vpo::printExtraForCancellationPoints(WRegionNode const *W,
                                          formatted_raw_ostream &OS, int Depth,
                                          unsigned Verbosity) {

  if (!W->canHaveCancellationPoints())
    return;

  unsigned Indent = 2 * Depth;
  auto &CPs = W->getCancellationPoints();
  vpo::printValList("CANCELLATION.POINTS",
                    SmallVector<Value *, 8>(CPs.begin(), CPs.end()), OS, Indent,
                    Verbosity);
}

// Print the fields common to some WRNs for which getIsOmpLoop()==true.
// Possible constructs are: WRNParallelLoop, WRNDistributeParLoop, WRNWksLoop
// The fields to print are: Collapse, Ordered, Nowait
void vpo::printExtraForOmpLoop(WRegionNode const *W, formatted_raw_ostream &OS,
                               int Depth, unsigned Verbosity) {
  assert(W->getIsOmpLoop() &&
         "printExtraForOmpLoop is for WRNs with getIsOmpLoop()==true");
  unsigned Indent = 2 * Depth;
  vpo::printInt("COLLAPSE", W->getCollapse(), OS, Indent, Verbosity);
  if (W->getOrdered() > 0)
    vpo::printInt("ORDERED(N)", W->getOrdered(), OS, Indent, Verbosity);
  else
    vpo::printBool("ORDERED", W->getOrdered() == 0, OS, Indent, Verbosity);

  // WRNs with getIsPar()==true don't have the Nowait clause
  if (!(W->getIsPar()))
    vpo::printBool("NOWAIT", W->getNowait(), OS, Indent, Verbosity);
}

// Print the fields common to WRNs for which getIsTarget()==true.
// Possible constructs are: WRNTarget, WRNTargetData, WRNTargetUpdate
// The fields to print are: IfExpr, Device, Nowait
// Additionally, for WRNTarget also print the Defaultmap clause
void vpo::printExtraForTarget(WRegionNode const *W, formatted_raw_ostream &OS,
                              int Depth, unsigned Verbosity) {
  assert(W->getIsTarget() &&
         "printExtraForTarget is for WRNs with getIsTarget()==true");
  unsigned Indent = 2 * Depth;
  vpo::printVal("IF_EXPR", W->getIf(), OS, Indent, Verbosity);
  vpo::printVal("DEVICE", W->getDevice(), OS, Indent, Verbosity);

  // All target constructs but WRNTargetData can have the NOWAIT clause
  if (!isa<WRNTargetDataNode>(W))
    vpo::printBool("NOWAIT", W->getNowait(), OS, Indent, Verbosity);

  // Only WRNTarget can have the defaultmap(tofrom:scalar) clause
  if (isa<WRNTargetNode>(W)) {
    StringRef Str = W->getDefaultmapTofromScalar() ?
                    "TOFROM:SCALAR" : "UNSPECIFIED";
    vpo::printStr("DEFAULTMAP", Str, OS, Indent, Verbosity);
  }
}

// Print the fields common to WRNs for which getIsTask()==true.
// Possible constructs are: WRNTask, WRNTaskloop
// The fields to print are:
//          IfExpr, Default, Final, Priority, Untied, Mergeable
// Additionally, for WRNTaskloop also print these:
//          Grainsize, NumTasks, Collapse, Nogroup
void vpo::printExtraForTask(WRegionNode const *W, formatted_raw_ostream &OS,
                            int Depth, unsigned Verbosity) {
  assert(W->getIsTask() &&
         "printExtraForTarget is for WRNs with getIsTask()==true");
  unsigned Indent = 2 * Depth;
  vpo::printVal("IF_EXPR", W->getIf(), OS, Indent, Verbosity);
  vpo::printStr("DEFAULT", WRNDefaultName[W->getDefault()], OS, Indent);
  vpo::printVal("FINAL", W->getFinal(), OS, Indent, Verbosity);
  vpo::printVal("PRIORITY", W->getPriority(), OS, Indent, Verbosity);
  vpo::printBool("UNTIED", W->getUntied(), OS, Indent, Verbosity);
  vpo::printBool("MERGEABLE", W->getMergeable(), OS, Indent, Verbosity);

  // WRNTaskloop has a few more additional fields to print
  if (isa<WRNTaskloopNode>(W)) {
    vpo::printVal("GRAINSIZE", W->getGrainsize(), OS, Indent, Verbosity);
    vpo::printVal("NUM_TASKS", W->getNumTasks(), OS, Indent, Verbosity);
    vpo::printInt("COLLAPSE", W->getCollapse(), OS, Indent, Verbosity);
    vpo::printBool("NOGROUP", W->getNogroup(), OS, Indent, Verbosity);
  }
}
#endif // INTEL_COLLAB
