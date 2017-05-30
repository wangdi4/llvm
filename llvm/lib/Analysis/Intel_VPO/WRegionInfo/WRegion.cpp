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
  DEBUG(dbgs() << "\nCreated WRNParallelNode<" << getNumber() << ">\n");
}

//
// Methods for WRNParallelLoopNode
//

// constructor
WRNParallelLoopNode::WRNParallelLoopNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNParallelLoop, BB), LI(Li) {
  setIsPar();
  setIsOmpLoop();
  setIf(nullptr);
  setNumThreads(nullptr);
  setDefault(WRNDefaultAbsent);
  setProcBind(WRNProcBindAbsent);
  setCollapse(0);
  setOrdered(0);

  DEBUG(dbgs() << "\nCreated WRNParallelLoopNode<" << getNumber() << ">\n");
}

//
// Methods for WRNParallelSectionsNode
//

// constructor
WRNParallelSectionsNode::WRNParallelSectionsNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNParallelSections, BB), LI(Li) {
  setIsPar();
  setIsOmpLoop();
  setIf(nullptr);
  setNumThreads(nullptr);
  setDefault(WRNDefaultAbsent);
  setProcBind(WRNProcBindAbsent);

  DEBUG(dbgs() << "\nCreated WRNParallelSectionsNode<" << getNumber() << ">\n");
}

//
// Methods for WRNParallelWorkshareNode
//

// constructor
WRNParallelWorkshareNode::WRNParallelWorkshareNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNParallelWorkshare, BB), LI(Li) {
  setIsPar();
  setIsOmpLoop();
  setIf(nullptr);
  setNumThreads(nullptr);
  setDefault(WRNDefaultAbsent);
  setProcBind(WRNProcBindAbsent);
  DEBUG(dbgs() << "\nCreated WRNParallelWorkshareNode<" << getNumber() 
                                                        << ">\n");
}

//
// Methods for WRNTeamsNode
//

// constructor
WRNTeamsNode::WRNTeamsNode(BasicBlock *BB) 
    : WRegionNode(WRegionNode::WRNTeams, BB) {
  setIsTeams();
  setThreadLimit(nullptr);
  setNumThreads(nullptr);
  setDefault(WRNDefaultAbsent);

  DEBUG(dbgs() << "\nCreated WRNTeamsNode<" << getNumber() << ">\n");
}

//
// Methods for WRNDistributeParLoopNode
//

// constructor
WRNDistributeParLoopNode::WRNDistributeParLoopNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNDistributeParLoop, BB), LI(Li) {
  setIsDistribute();
  setIsPar();
  setIsOmpLoop();
  setIf(nullptr);
  setNumThreads(nullptr);
  setDefault(WRNDefaultAbsent);
  setProcBind(WRNProcBindAbsent);
  setCollapse(0);
  setOrdered(0);

  DEBUG(dbgs() << "\nCreated WRNDistributeParLoopNode<" << getNumber() << ">\n");
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

  DEBUG(dbgs() << "\nCreated WRNTargetNode<" << getNumber() << ">\n");
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
  setNowait(false);

  DEBUG(dbgs() << "\nCreated WRNTargetDataNode<" << getNumber() << ">\n");
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

  DEBUG(dbgs() << "\nCreated WRNTaskNode<" << getNumber() << ">\n");
}

//
// Methods for WRNTaskloopNode
//

// constructor
WRNTaskloopNode::WRNTaskloopNode(BasicBlock *BB, LoopInfo *Li)
    : WRNTaskNode(BB), LI(Li) {
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

  DEBUG(dbgs() << "\nCreated WRNTaskloopNode<" << getNumber() << ">\n");
}

//
// Methods for WRNVecLoopNode
//

// constructor for LLVM IR representation
WRNVecLoopNode::WRNVecLoopNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNVecLoop, BB), LI(Li) {
  setIsOmpLoop();
  setSimdlen(0);
  setSafelen(0);
  setCollapse(0);
  setIsAutoVec(false);

  DEBUG(dbgs() << "\nCreated WRNVecLoopNode<" << getNumber() << ">\n");
}

// constructor for HIR representation
WRNVecLoopNode::WRNVecLoopNode(loopopt::HLNode *EntryHLN)
    : WRegionNode(WRegionNode::WRNVecLoop), EntryHLNode(EntryHLN) {
  setIsOmpLoop();
  setSimdlen(0);
  setSafelen(0);
  setCollapse(0);
  setIsAutoVec(false);

  setExitHLNode(nullptr);
  setHLLoop(nullptr);

  DEBUG(dbgs() << "\nCreated HIR-WRNVecLoopNode<" << getNumber() << ">\n");
}

void WRNVecLoopNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                                     bool Verbose) const {
  OS.indent(2*Depth) << "SIMDLEN: " << getSimdlen() << "\n";
  OS.indent(2*Depth) << "SAFELEN: " << getSafelen() << "\n";
}

void WRNVecLoopNode::printHIR(formatted_raw_ostream &OS, unsigned Depth,
                              bool Verbose) const {
  if (!getIsFromHIR()) // using LLVM-IR representation; no HIR to print
    return;

  OS.indent(2*Depth) << "EntryHLNode:\n";
  getEntryHLNode()->print(OS, 1);
  if (Verbose) {
    OS << "\n";
    OS.indent(2*Depth) << "HLLoop:\n";
    getHLLoop()->print(OS, 1);
  }
  OS << "\n";
  OS.indent(2*Depth) << "ExitHLNode:\n";
  getExitHLNode()->print(OS, 1);
}

//
// Methods for WRNWksLoopNode
//

// constructor
WRNWksLoopNode::WRNWksLoopNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNWksLoop, BB), LI(Li) {
  setIsOmpLoop();
  setCollapse(0);
  setOrdered(0);
  setNowait(false);

  DEBUG(dbgs() << "\nCreated WRNWksLoopNode<" << getNumber() << ">\n");
}

//
// Methods for WRNSectionsNode
//

// constructor
WRNSectionsNode::WRNSectionsNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNSections, BB), LI(Li) {
  setIsOmpLoop();
  setNowait(false);

  DEBUG(dbgs() << "\nCreated WRNSectionsNode<" << getNumber() << ">\n");
}

//
// Methods for WRNWorkshareNode
//

// constructor
WRNWorkshareNode::WRNWorkshareNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNWorkshare, BB), LI(Li) {
  setIsOmpLoop();
  setNowait(false);

  DEBUG(dbgs() << "\nCreated WRNWorkshareNode<" << getNumber() << ">\n");
}

//
// Methods for WRNDistributeNode
//

// constructor
WRNDistributeNode::WRNDistributeNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNDistribute, BB), LI(Li) {
  setIsOmpLoop();
  setIsDistribute();
  setCollapse(0);
  setNowait(false);

  DEBUG(dbgs() << "\nCreated WRNDistributeNode<" << getNumber() << ">\n");
}

//
// Methods for WRNAtomicNode
//

// constructor
WRNAtomicNode::WRNAtomicNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNAtomic, BB) {
  setAtomicKind(WRNAtomicUpdate); // Default Atomic Kind is WRNAtomicUpdate
  setHasSeqCstClause(false);

  DEBUG(dbgs() << "\nCreated WRNAtomicNode<" << getNumber() << ">\n");
}

void WRNAtomicNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                                     bool Verbose) const {
  OS.indent(2*Depth) << "Atomic Kind: "
     << VPOAnalysisUtils::getClauseName(
            WRegionUtils::getClauseIdFromAtomicKind(getAtomicKind()))
     << "\n";
  OS.indent(2*Depth) << "Seq_Cst Clause: " << 
                        (getHasSeqCstClause() ? "Yes" : "No") << "\n";
}

//
// Methods for WRNBarrierNode
//

// constructor
WRNBarrierNode::WRNBarrierNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNBarrier, BB) {
  DEBUG(dbgs() << "\nCreated WRNBarrierNode <" << getNumber() << ">\n");
}


//
// Methods for WRNCancelNode
//

// constructor
WRNCancelNode::WRNCancelNode(BasicBlock *BB, bool IsCP)
    : WRegionNode(WRegionNode::WRNCancel, BB), IsCancellationPoint(IsCP) {
  setCancelKind(WRNCancelError);
  setIf(nullptr);
  DEBUG(dbgs() << "\nCreated WRNCancelNode <" << getNumber() << ">\n");
}

//
// Methods for WRNMasterNode
//

// constructor
WRNMasterNode::WRNMasterNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNMaster, BB) {
  DEBUG(dbgs() << "\nCreated WRNMasterNode <" << getNumber() << ">\n");
}

//
// Methods for WRNOrderedNode
//

// constructor
WRNOrderedNode::WRNOrderedNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNOrdered, BB) {
  setIsDoacross(false);
  setIsThreads(true);
  DEBUG(dbgs() << "\nCreated WRNOrderedNode <" << getNumber() << ">\n");
}

//
// Methods for WRNSingleNode
//

// constructor
WRNSingleNode::WRNSingleNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNSingle, BB) {
  DEBUG(dbgs() << "\nCreated WRNSingleNode <" << getNumber() << ">\n");
}

//
// Methods for WRNCriticalNode
//

// constructor
WRNCriticalNode::WRNCriticalNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNCritical, BB), UserLockName("") {
  // UserLockName is empty by default
  DEBUG(dbgs() << "\nCreated WRNCriticalNode <" << getNumber() << ">\n");
}

void WRNCriticalNode::printExtra(formatted_raw_ostream &OS, unsigned Depth,
                                                       bool Verbose) const {
  if (!UserLockName.empty())
    OS.indent(2*Depth) << "User Lock Name: " << UserLockName << "\n";
}


// constructor
WRNFlushNode::WRNFlushNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNFlush, BB) {
  DEBUG(dbgs() << "\nCreated WRNFlushNode<" << getNumber() << ">\n");
}

//
// Methods for WRNTaskgroupNode
//

// constructor
WRNTaskgroupNode::WRNTaskgroupNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNTaskgroup, BB) {
  DEBUG(dbgs() << "\nCreated WRNTaskgroupNode <" << getNumber() << ">\n");
}

//
// Methods for WRNTaskwaitNode
//

// constructor
WRNTaskwaitNode::WRNTaskwaitNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNTaskwait, BB) {
  DEBUG(dbgs() << "\nCreated WRNTaskwaitNode <" << getNumber() << ">\n");
}

//
// Methods for WRNTaskyieldNode
//

// constructor
WRNTaskyieldNode::WRNTaskyieldNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNTaskyield, BB) {
  DEBUG(dbgs() << "\nCreated WRNTaskyieldNode <" << getNumber() << ">\n");
}
