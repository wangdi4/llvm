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
#include "llvm/Transforms/Utils/Intel_IntrinsicUtils.h"

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
  setShared(nullptr);
  setPriv(nullptr);
  setFpriv(nullptr);
  setRed(nullptr);
  setCopyin(nullptr);
  setIf(nullptr);
  setNumThreads(nullptr);
  setDefault(WRNDefaultAbsent);
  setProcBind(WRNProcBindAbsent);
  DEBUG(dbgs() << "\nCreated WRNParallelNode<" << getNumber() << ">\n");
}

WRNParallelNode::WRNParallelNode(WRNParallelNode *W) : WRegionNode(W) {
  setAttributes(W->getAttributes());
  setShared(W->getShared());
  setPriv(W->getPriv());
  setFpriv(W->getFpriv());
  setRed(W->getRed());
  setCopyin(W->getCopyin());
  setIf(W->getIf());
  setNumThreads(W->getNumThreads());
  setDefault(W->getDefault());
  setProcBind(W->getProcBind());
  DEBUG(dbgs() << "\nCreated WRNParallelNode<" << getNumber() << ">\n");
}

void WRNParallelNode::print(formatted_raw_ostream &OS, unsigned Depth) const {
#if !INTEL_PRODUCT_RELEASE
  // TODO may need to have an extra parameter (or global) to add a fixed
  //    space for left margin at Depth=0
  std::string Indent(Depth * 2, ' ');

  OS << Indent << "BEGIN WRNParallelNode<" << getNumber() << "> {\n";

  // TODO: print data local to this ParRegion
  //       eg shared vars, priv vars, etc.
  printChildren(OS, Depth + 1);
  OS << Indent << "} END WRNParallelNode<" << getNumber() << ">\n";
#endif // !INTEL_PRODUCT_RELEASE
}

//
// Methods for WRNParallelLoopNode
//

// constructor
WRNParallelLoopNode::WRNParallelLoopNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNParallelLoop, BB), LI(Li) {
  setIsPar();
  setIsLoop();
  setShared(nullptr);
  setPriv(nullptr);
  setFpriv(nullptr);
  setRed(nullptr);
  setCopyin(nullptr);
  setIf(nullptr);
  setNumThreads(nullptr);
  setDefault(WRNDefaultAbsent);
  setProcBind(WRNProcBindAbsent);
  setCollapse(0);
  setOrdered(false);
  Loop *L = IntelGeneralUtils::getLoopFromLoopInfo(Li, BB);
  setLoop(L);

  DEBUG(dbgs() << "\nCreated WRNParallelLoopNode<" << getNumber() << ">\n");
}

WRNParallelLoopNode::WRNParallelLoopNode(WRNParallelLoopNode *W)
    : WRegionNode(W) {
  setAttributes(W->getAttributes());
  setShared(W->getShared());
  setPriv(W->getPriv());
  setFpriv(W->getFpriv());
  setRed(W->getRed());
  setCopyin(W->getCopyin());
  setIf(W->getIf());
  setNumThreads(W->getNumThreads());
  setDefault(W->getDefault());
  setProcBind(W->getProcBind());
  setSchedule(W->getSchedule());
  setCollapse(W->getCollapse());
  setOrdered(W->getOrdered());
  setLoopInfo(W->getLoopInfo());
  setLoop(W->getLoop());
  DEBUG(dbgs() << "\nCreated WRNParallelLoopNode<" << getNumber() << ">\n");
}

void WRNParallelLoopNode::print(formatted_raw_ostream &OS,
                                unsigned Depth) const {
#if !INTEL_PRODUCT_RELEASE
  // TODO may need to have an extra parameter (or global) to add a fixed
  //    space for left margin at Depth=0
  std::string Indent(Depth * 2, ' ');

  OS << Indent << "BEGIN WRNParallelLoopNode<" << getNumber() << "> {\n";
  // TODO: print data local to this ParRegion
  //       eg shared vars, priv vars, etc.

  if (auto PrivC = getPriv())
    for (PrivateItem *PrivI : PrivC->items()) {
      StringRef PrivS = IntelIntrinsicUtils::getClauseString(QUAL_OMP_PRIVATE);
      OS << Indent << "PRIVATE clause: " << PrivS << " "
         << PrivI->getOrig()->getName() << "\n";
    }

  // LinearClause *C = getLinear();
  // if (C) {
  //  OS << Indent;
  //  C->print(OS);
  // }

  OS << "\n" << Indent << "EntryBB:" << *getEntryBBlock();
  OS << "\n" << Indent << "ExitBB:" << *getExitBBlock();
  OS << "\n" << Indent << "BBlockSet dump:\n";
  if (!isBBSetEmpty())
    for (auto I = bbset_begin(), E = bbset_end(); I != E; ++I) {
      const BasicBlock *BB = *I;
      OS << Indent << *BB;
    }
  else
    OS << Indent << "No BBSet\n";

  printChildren(OS, Depth + 1);
  OS << Indent << "} END WRNParallelLoopNode<" << getNumber() << ">\n";
#endif // !INTEL_PRODUCT_RELEASE
}

//
// Methods for WRNParallelSectionsNode
//

// constructor
WRNParallelSectionsNode::WRNParallelSectionsNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNParallelSections, BB), LI(Li) {
  setIsPar();
  setIsLoop();
  setShared(nullptr);
  setPriv(nullptr);
  setFpriv(nullptr);
  setRed(nullptr);
  setCopyin(nullptr);
  setIf(nullptr);
  setNumThreads(nullptr);
  setDefault(WRNDefaultAbsent);
  setProcBind(WRNProcBindAbsent);
  Loop *L = IntelGeneralUtils::getLoopFromLoopInfo(Li, BB);
  setLoop(L);

  DEBUG(dbgs() << "\nCreated WRNParallelSectionsNode<" << getNumber() << ">\n");
}

void WRNParallelSectionsNode::print(formatted_raw_ostream &OS, unsigned Depth)
const {
#if !INTEL_PRODUCT_RELEASE
  std::string Indent(Depth * 2, ' ');
  OS << Indent << "BEGIN WRNParallelSectionsNode<" << getNumber() << "> {\n";
  printChildren(OS, Depth + 1);
  OS << Indent << "} END WRNParallelSectionsNode<" << getNumber() << ">\n";
#endif // !INTEL_PRODUCT_RELEASE
}

//
// Methods for WRNParallelWorkshareNode
//

// constructor
WRNParallelWorkshareNode::WRNParallelWorkshareNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNParallelWorkshare, BB), LI(Li) {
  setIsPar();
  setIsLoop();
  setShared(nullptr);
  setPriv(nullptr);
  setFpriv(nullptr);
  setRed(nullptr);
  setCopyin(nullptr);
  setIf(nullptr);
  setNumThreads(nullptr);
  setDefault(WRNDefaultAbsent);
  setProcBind(WRNProcBindAbsent);
  Loop *L = IntelGeneralUtils::getLoopFromLoopInfo(Li, BB);
  setLoop(L);

  DEBUG(dbgs() << "\nCreated WRNParallelWorkshareNode<" << getNumber() 
                                                        << ">\n");
}

void WRNParallelWorkshareNode::print(formatted_raw_ostream &OS, unsigned Depth)
const {
#if !INTEL_PRODUCT_RELEASE
  std::string Indent(Depth * 2, ' ');
  OS << Indent << "BEGIN WRNParallelWorkshareNode<" << getNumber() << "> {\n";
  printChildren(OS, Depth + 1);
  OS << Indent << "} END WRNParallelWorkshareNode<" << getNumber() << ">\n";
#endif // !INTEL_PRODUCT_RELEASE
}

//
// Methods for WRNTeamsNode
//

// constructor
WRNTeamsNode::WRNTeamsNode(BasicBlock *BB) 
    : WRegionNode(WRegionNode::WRNTeams, BB) {
  setIsTeams();
  setShared(nullptr);
  setPriv(nullptr);
  setFpriv(nullptr);
  setRed(nullptr);
  setThreadLimit(nullptr);
  setNumThreads(nullptr);
  setDefault(WRNDefaultAbsent);

  DEBUG(dbgs() << "\nCreated WRNTeamsNode<" << getNumber() << ">\n");
}

void WRNTeamsNode::print(formatted_raw_ostream &OS, unsigned Depth) const {
#if !INTEL_PRODUCT_RELEASE
  std::string Indent(Depth * 2, ' ');
  OS << Indent << "BEGIN WRNTeamsNode<" << getNumber() << "> {\n";
  printChildren(OS, Depth + 1);
  OS << Indent << "} END WRNTeamsNode<" << getNumber() << ">\n";
#endif // !INTEL_PRODUCT_RELEASE
}

//
// Methods for WRNDistributeParLoopNode
//

// constructor
WRNDistributeParLoopNode::WRNDistributeParLoopNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNDistributeParLoop, BB), LI(Li) {
  setIsDistribute();
  setIsPar();
  setIsLoop();
  setShared(nullptr);
  setPriv(nullptr);
  setFpriv(nullptr);
  setRed(nullptr);
  setCopyin(nullptr);
  setIf(nullptr);
  setNumThreads(nullptr);
  setDefault(WRNDefaultAbsent);
  setProcBind(WRNProcBindAbsent);
  setCollapse(0);
  setOrdered(false);
  Loop *L = IntelGeneralUtils::getLoopFromLoopInfo(Li, BB);
  setLoop(L);

  DEBUG(dbgs() << "\nCreated WRNDistributeParLoopNode<" << getNumber() << ">\n");
}

void WRNDistributeParLoopNode::print(formatted_raw_ostream &OS, unsigned Depth)
const {
#if !INTEL_PRODUCT_RELEASE
  std::string Indent(Depth * 2, ' ');
  OS << Indent << "BEGIN WRNDistributeParLoopNode<" << getNumber() << "> {\n";
  printChildren(OS, Depth + 1);
  OS << Indent << "} END WRNDistributeParLoopNode<" << getNumber() << ">\n";
#endif // !INTEL_PRODUCT_RELEASE
}

//
// Methods for WRNTargetNode
//

// constructor
WRNTargetNode::WRNTargetNode(BasicBlock *BB) 
    : WRegionNode(WRegionNode::WRNTarget, BB) {
  setIsTarget();
  setPriv(nullptr);
  setFpriv(nullptr);
  setMap(nullptr);
  setDepend(nullptr);
  setIsDevicePtr(nullptr);
  setIf(nullptr);
  setDevice(nullptr);
  setNowait(false);
  setDefaultmapTofromScalar(false);

  DEBUG(dbgs() << "\nCreated WRNTargetNode<" << getNumber() << ">\n");
}

void WRNTargetNode::print(formatted_raw_ostream &OS, unsigned Depth)
const {
#if !INTEL_PRODUCT_RELEASE
  std::string Indent(Depth * 2, ' ');
  OS << Indent << "BEGIN WRNTargetNode<" << getNumber() << "> {\n";
  printChildren(OS, Depth + 1);
  OS << Indent << "} END WRNTargetNode<" << getNumber() << ">\n";
#endif // !INTEL_PRODUCT_RELEASE
}

//
// Methods for WRNTargetDataNode
//

// constructor
WRNTargetDataNode::WRNTargetDataNode(BasicBlock *BB) 
    : WRegionNode(WRegionNode::WRNTargetData, BB) {
  setIsTarget();
  setMap(nullptr);
  setDepend(nullptr);
  setUseDevicePtr(nullptr);
  setIf(nullptr);
  setDevice(nullptr);
  setNowait(false);

  DEBUG(dbgs() << "\nCreated WRNTargetDataNode<" << getNumber() << ">\n");
}

void WRNTargetDataNode::print(formatted_raw_ostream &OS, unsigned Depth)
const {
#if !INTEL_PRODUCT_RELEASE
  std::string Indent(Depth * 2, ' ');
  OS << Indent << "BEGIN WRNTargetDataNode<" << getNumber() << "> {\n";
  printChildren(OS, Depth + 1);
  OS << Indent << "} END WRNTargetDataNode<" << getNumber() << ">\n";
#endif // !INTEL_PRODUCT_RELEASE
}

//
// Methods for WRNTaskNode
//

// constructor
WRNTaskNode::WRNTaskNode(BasicBlock *BB) 
    : WRegionNode(WRegionNode::WRNTask, BB) {
  setIsTask();
  setShared(nullptr);
  setPriv(nullptr);
  setFpriv(nullptr);
  setDepend(nullptr);
  setIf(nullptr);
  setFinal(nullptr);
  setPriority(nullptr);
  setDefault(WRNDefaultAbsent);
  setUntied(false);
  setMergeable(false);

  DEBUG(dbgs() << "\nCreated WRNTaskNode<" << getNumber() << ">\n");
}

void WRNTaskNode::print(formatted_raw_ostream &OS, unsigned Depth) const {
#if !INTEL_PRODUCT_RELEASE
  std::string Indent(Depth * 2, ' ');
  OS << Indent << "BEGIN WRNTaskNode<" << getNumber() << "> {\n";
  printChildren(OS, Depth + 1);
  OS << Indent << "} END WRNTaskNode<" << getNumber() << ">\n";
#endif // !INTEL_PRODUCT_RELEASE
}

//
// Methods for WRNTaskloopNode
//

// constructor
WRNTaskloopNode::WRNTaskloopNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNTaskloop, BB), LI(Li) {
  setIsTask();
  setIsLoop();
  setShared(nullptr);
  setPriv(nullptr);
  setFpriv(nullptr);
  setLpriv(nullptr);
  setDepend(nullptr);
  setFinal(nullptr);
  setGrainsize(nullptr);
  setIf(nullptr);
  setNumTasks(nullptr);
  setPriority(nullptr);
  setDefault(WRNDefaultAbsent);
  setCollapse(0);
  setUntied(false);
  setMergeable(false);
  setNogroup(false);
  Loop *L = IntelGeneralUtils::getLoopFromLoopInfo(Li, BB);
  setLoop(L);

  DEBUG(dbgs() << "\nCreated WRNTaskloopNode<" << getNumber() << ">\n");
}

void WRNTaskloopNode::print(formatted_raw_ostream &OS, unsigned Depth) const {
#if !INTEL_PRODUCT_RELEASE
  std::string Indent(Depth * 2, ' ');
  OS << Indent << "BEGIN WRNTaskloopNode<" << getNumber() << "> {\n";
  printChildren(OS, Depth + 1);
  OS << Indent << "} END WRNTaskloopNode<" << getNumber() << ">\n";
#endif // !INTEL_PRODUCT_RELEASE
}

//
// Methods for WRNVecLoopNode
//

// constructor for LLVM IR representation
WRNVecLoopNode::WRNVecLoopNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNVecLoop, BB), LI(Li) {
  setPriv(nullptr);
  setLpriv(nullptr);
  setRed(nullptr);
  setUniform(nullptr);
  setLinear(nullptr);
  setAligned(nullptr);
  setSimdlen(0);
  setSafelen(0);
  setCollapse(0);
  setIsAutoVec(false);

  Loop *L = IntelGeneralUtils::getLoopFromLoopInfo(Li, BB);
  setLoop(L);

  DEBUG(dbgs() << "\nCreated WRNVecLoopNode<" << getNumber() << ">\n");
}

// constructor for HIR representation
WRNVecLoopNode::WRNVecLoopNode(loopopt::HLNode *EntryHLN)
    : WRegionNode(WRegionNode::WRNVecLoop), EntryHLNode(EntryHLN) {
  setPriv(nullptr);
  setLpriv(nullptr);
  setRed(nullptr);
  setUniform(nullptr);
  setLinear(nullptr);
  setAligned(nullptr);
  setSimdlen(0);
  setSafelen(0);
  setCollapse(0);
  setIsAutoVec(false);

  setExitHLNode(nullptr);
  setHLLoop(nullptr);

  DEBUG(dbgs() << "\nCreated HIR-WRNVecLoopNode<" << getNumber() << ">\n");
}

WRNVecLoopNode::WRNVecLoopNode(WRNVecLoopNode *W) : WRegionNode(W) {
  setPriv(W->getPriv());
  setLpriv(W->getLpriv());
  setRed(W->getRed());
  setUniform(W->getUniform());
  setLinear(W->getLinear());
  setAligned(W->getAligned());
  setSimdlen(W->getSimdlen());
  setSafelen(W->getSafelen());
  setCollapse(W->getCollapse());
  setIsAutoVec(W->getIsAutoVec());
  if (W->getIsFromHIR()) {
    setEntryHLNode(W->getEntryHLNode());
    setExitHLNode(W->getExitHLNode());
    setHLLoop(W->getHLLoop());
  } else {
    setLoopInfo(W->getLoopInfo());
    setLoop(W->getLoop());
  }
  DEBUG(dbgs() << "\nCreated WRNVecLoopNode<" << getNumber() << ">\n");
}

void WRNVecLoopNode::print(formatted_raw_ostream &OS, unsigned Depth) const {
#if !INTEL_PRODUCT_RELEASE
  std::string Indent(Depth * 2, ' ');
  OS << Indent << "\nBEGIN WRNVecLoopNode<" << getNumber() << "> {\n";

  OS << Indent << "SIMDLEN clause: " << getSimdlen() << "\n";

  if (auto RC = getRed())
    for (ReductionItem *RI : RC->items()) {
      ReductionItem::WRNReductionKind RType = RI->getType();
      int RedClauseID = ReductionItem::getClauseIdFromKind(RType);
      StringRef RedStr = IntelIntrinsicUtils::getClauseString(RedClauseID);
      OS << Indent << "REDUCTION clause: " << RedStr << " "
         << RI->getOrig()->getName() << "\n";
    }
  UniformClause *CU = getUniform();
  if (CU) {
    OS << Indent;
    CU->print(OS);
  }
  LinearClause *C = getLinear();
  if (C) {
    OS << Indent;
    C->print(OS);
  }

  if (getIsFromHIR()) {
    OS << "\n" << Indent << "EntryHLNode:\n";
    getEntryHLNode()->print(OS, 1);
    OS << "\n" << Indent << "HLLoop:\n";
    getHLLoop()->print(OS, 1);
    OS << "\n" << Indent << "ExitHLNode:\n";
    getExitHLNode()->print(OS, 1);
  } else {
    OS << "\n" << Indent << "EntryBB:" << *getEntryBBlock();
    OS << "\n" << Indent << "ExitBB:" << *getExitBBlock();
    OS << "\n" << Indent << "BBlockSet dump:\n";
    if (!isBBSetEmpty())
      for (auto I = bbset_begin(), E = bbset_end(); I != E; ++I) {
        const BasicBlock *BB = *I;
        OS << Indent << *BB;
      }
    else
      OS << Indent << "No BBSet\n";
  }
  printChildren(OS, Depth + 1);
  OS << Indent << "} END WRNVecLoopNode<" << getNumber() << ">\n\n";
#endif // !INTEL_PRODUCT_RELEASE
}

//
// Methods for WRNWksLoopNode
//

// constructor
WRNWksLoopNode::WRNWksLoopNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNWksLoop, BB), LI(Li) {
  setIsLoop();
  setPriv(nullptr);
  setFpriv(nullptr);
  setLpriv(nullptr);
  setRed(nullptr);
  setLinear(nullptr);
  setCollapse(0);
  setOrdered(0);
  setNowait(false);
  Loop *L = IntelGeneralUtils::getLoopFromLoopInfo(Li, BB);
  setLoop(L);

  DEBUG(dbgs() << "\nCreated WRNWksLoopNode<" << getNumber() << ">\n");
}

void WRNWksLoopNode::print(formatted_raw_ostream &OS, unsigned Depth) const {
#if !INTEL_PRODUCT_RELEASE
  std::string Indent(Depth * 2, ' ');
  OS << Indent << "BEGIN WRNWksLoopNode<" << getNumber() << "> {\n";
  printChildren(OS, Depth + 1);
  OS << Indent << "} END WRNWksLoopNode<" << getNumber() << ">\n";
#endif // !INTEL_PRODUCT_RELEASE
}

//
// Methods for WRNSectionsNode
//

// constructor
WRNSectionsNode::WRNSectionsNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNSections, BB), LI(Li) {
  setIsLoop();
  setPriv(nullptr);
  setFpriv(nullptr);
  setLpriv(nullptr);
  setRed(nullptr);
  setNowait(false);
  Loop *L = IntelGeneralUtils::getLoopFromLoopInfo(Li, BB);
  setLoop(L);

  DEBUG(dbgs() << "\nCreated WRNSectionsNode<" << getNumber() << ">\n");
}

void WRNSectionsNode::print(formatted_raw_ostream &OS, unsigned Depth) const
{
#if !INTEL_PRODUCT_RELEASE
  std::string Indent(Depth * 2, ' ');
  OS << Indent << "BEGIN WRNSectionsNode<" << getNumber() << "> {\n";
  printChildren(OS, Depth + 1);
  OS << Indent << "} END WRNSectionsNode<" << getNumber() << ">\n";
#endif // !INTEL_PRODUCT_RELEASE
}

//
// Methods for WRNWorkshareNode
//

// constructor
WRNWorkshareNode::WRNWorkshareNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNWorkshare, BB), LI(Li) {
  setIsLoop();
  setNowait(false);
  Loop *L = IntelGeneralUtils::getLoopFromLoopInfo(Li, BB);
  setLoop(L);

  DEBUG(dbgs() << "\nCreated WRNWorkshareNode<" << getNumber() << ">\n");
}

void WRNWorkshareNode::print(formatted_raw_ostream &OS, unsigned Depth) const{
#if !INTEL_PRODUCT_RELEASE
  std::string Indent(Depth * 2, ' ');
  OS << Indent << "BEGIN WRNWorkshareNode<" << getNumber() << "> {\n";
  printChildren(OS, Depth + 1);
  OS << Indent << "} END WRNWorkshareNode<" << getNumber() << ">\n";
#endif // !INTEL_PRODUCT_RELEASE
}

//
// Methods for WRNDistributeNode
//

// constructor
WRNDistributeNode::WRNDistributeNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNDistribute, BB), LI(Li) {
  setIsLoop();
  setIsDistribute();
  setPriv(nullptr);
  setFpriv(nullptr);
  setLpriv(nullptr);
  setCollapse(0);
  setNowait(false);
  Loop *L = IntelGeneralUtils::getLoopFromLoopInfo(Li, BB);
  setLoop(L);

  DEBUG(dbgs() << "\nCreated WRNDistributeNode<" << getNumber() << ">\n");
}

void WRNDistributeNode::print(formatted_raw_ostream &OS, unsigned Depth) const{
#if !INTEL_PRODUCT_RELEASE
  std::string Indent(Depth * 2, ' ');
  OS << Indent << "BEGIN WRNDistributeNode<" << getNumber() << "> {\n";
  printChildren(OS, Depth + 1);
  OS << Indent << "} END WRNDistributeNode<" << getNumber() << ">\n";
#endif // !INTEL_PRODUCT_RELEASE
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

WRNAtomicNode::WRNAtomicNode(WRNAtomicNode *W) : WRegionNode(W) {
  setAtomicKind(W->getAtomicKind());
  setHasSeqCstClause(W->getHasSeqCstClause());

  DEBUG(dbgs() << "\nCreated WRNAtomicNode<" << getNumber() << ">\n");
}

void WRNAtomicNode::print(formatted_raw_ostream &OS, unsigned Depth) const {
#if !INTEL_PRODUCT_RELEASE
  std::string Indent(Depth * 2, ' ');

  OS << Indent << "BEGIN WRNAtomicNode<" << getNumber() << "> {\n";
  OS << Indent << "Atomic Kind: "
     << VPOAnalysisUtils::getClauseName(
            WRegionUtils::getClauseIdFromAtomicKind(getAtomicKind()))
     << "\n";
  OS << Indent << "Seq_Cst Clause: " << (getHasSeqCstClause() ? "Yes" : "No")
     << "\n";
  OS << "\n" << Indent << "EntryBB:" << *getEntryBBlock();
  OS << "\n" << Indent << "ExitBB:" << *getExitBBlock();
  OS << "\n" << Indent << "BBlockSet dump:\n";

  if (!isBBSetEmpty())
    for (auto I = bbset_begin(), E = bbset_end(); I != E; ++I) {
      const BasicBlock *BB = *I;
      OS << Indent << *BB;
    }
  else
    OS << Indent << "No BBSet\n";

  OS << Indent << "} END WRNAtomicNode<" << getNumber() << ">\n";
#endif // !INTEL_PRODUCT_RELEASE
}

//
// Methods for WRNBarrierNode
//

// constructor
WRNBarrierNode::WRNBarrierNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNBarrier, BB) {
  DEBUG(dbgs() << "\nCreated WRNBarrierNode <" << getNumber() << ">\n");
}

void WRNBarrierNode::print(formatted_raw_ostream &OS, unsigned Depth) const {
#if !INTEL_PRODUCT_RELEASE
  std::string Indent(Depth * 2, ' ');
  OS << Indent << "\nBEGIN WRNBarrierNode<" << getNumber() << "> {\n";
  printChildren(OS, Depth + 1);
  OS << Indent << "} END WRNBarrierNode <" << getNumber() << ">\n\n";
#endif // !INTEL_PRODUCT_RELEASE
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

void WRNCancelNode::print(formatted_raw_ostream &OS, unsigned Depth) const {
#if !INTEL_PRODUCT_RELEASE
  std::string Indent(Depth * 2, ' ');
  OS << Indent << "\nBEGIN WRNCancelNode<" << getNumber() << "> {\n";
  printChildren(OS, Depth + 1);
  OS << Indent << "} END WRNCancelNode <" << getNumber() << ">\n\n";
#endif // !INTEL_PRODUCT_RELEASE
}

//
// Methods for WRNMasterNode
//

// constructor
WRNMasterNode::WRNMasterNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNMaster, BB) {
  DEBUG(dbgs() << "\nCreated WRNMasterNode <" << getNumber() << ">\n");
}

WRNMasterNode::WRNMasterNode(WRNMasterNode *W) : WRegionNode(W) {
  DEBUG(dbgs() << "\nCreated WRNMasterNode<" << getNumber() << ">\n");
}

void WRNMasterNode::print(formatted_raw_ostream &OS, unsigned Depth) const {
#if !INTEL_PRODUCT_RELEASE
  std::string Indent(Depth * 2, ' ');
  OS << Indent << "\nBEGIN WRNMasterNode<" << getNumber() << "> {\n";

  if (!isBBSetEmpty())
    for (auto I = bbset_begin(), E = bbset_end(); I != E; ++I) {
      const BasicBlock *BB = *I;
      OS << Indent << *BB;
    }
  else
    OS << Indent << "No BBSet\n";

  printChildren(OS, Depth + 1);
  OS << Indent << "} END WRNMasterNode <" << getNumber() << ">\n\n";
#endif // !INTEL_PRODUCT_RELEASE
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

WRNOrderedNode::WRNOrderedNode(WRNOrderedNode *W) : WRegionNode(W) {
  setIsDoacross(W->getIsDoacross());
  setIsThreads(W->getIsThreads());
  setIsDepSource(W->getIsDepSource());
  setDepSink(W->getDepSink());
  DEBUG(dbgs() << "\nCreated WRNOrderedNode<" << getNumber() << ">\n");
}

void WRNOrderedNode::print(formatted_raw_ostream &OS, unsigned Depth) const {
#if !INTEL_PRODUCT_RELEASE
  std::string Indent(Depth * 2, ' ');
  OS << Indent << "\nBEGIN WRNOrderedNode<" << getNumber() << "> {\n";

  if (!isBBSetEmpty())
    for (auto I = bbset_begin(), E = bbset_end(); I != E; ++I) {
      const BasicBlock *BB = *I;
      OS << Indent << *BB;
    }
  else
    OS << Indent << "No BBSet\n";

  printChildren(OS, Depth + 1);
  OS << Indent << "} END WRNOrderedNode <" << getNumber() << ">\n\n";
#endif // !INTEL_PRODUCT_RELEASE
}

//
// Methods for WRNSingleNode
//

// constructor
WRNSingleNode::WRNSingleNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNSingle, BB) {
  DEBUG(dbgs() << "\nCreated WRNSingleNode <" << getNumber() << ">\n");
}

WRNSingleNode::WRNSingleNode(WRNSingleNode *W) : WRegionNode(W) {
  DEBUG(dbgs() << "\nCreated WRNSingleNode<" << getNumber() << ">\n");
}

void WRNSingleNode::print(formatted_raw_ostream &OS, unsigned Depth) const {
#if !INTEL_PRODUCT_RELEASE
  std::string Indent(Depth * 2, ' ');
  OS << Indent << "\nBEGIN WRNSingleNode<" << getNumber() << "> {\n";

  if (!isBBSetEmpty())
    for (auto I = bbset_begin(), E = bbset_end(); I != E; ++I) {
      const BasicBlock *BB = *I;
      OS << Indent << *BB;
    }
  else
    OS << Indent << "No BBSet\n";

  printChildren(OS, Depth + 1);
  OS << Indent << "} END WRNSingleNode <" << getNumber() << ">\n\n";
#endif // !INTEL_PRODUCT_RELEASE
}

//
// Methods for WRNCriticalNode
//

// constructor
WRNCriticalNode::WRNCriticalNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNCritical, BB), UserLockName("") {
  // UserLockName is empty be default
  DEBUG(dbgs() << "\nCreated WRNCriticalNode <" << getNumber() << ">\n");
}

WRNCriticalNode::WRNCriticalNode(WRNCriticalNode *W)
    : WRegionNode(W), UserLockName(W->UserLockName) {
  DEBUG(dbgs() << "\nCreated WRNCriticalNode<" << getNumber() << ">\n");
}

void WRNCriticalNode::print(formatted_raw_ostream &OS, unsigned Depth) const {
#if !INTEL_PRODUCT_RELEASE
  std::string Indent(Depth * 2, ' ');
  OS << Indent << "\nBEGIN WRNCriticalNode<" << getNumber() << "> {\n";

  if (!UserLockName.empty())
    OS << Indent << "\nUser Lock Name: " << UserLockName << "\n";

  if (!isBBSetEmpty())
    for (auto I = bbset_begin(), E = bbset_end(); I != E; ++I) {
      const BasicBlock *BB = *I;
      OS << Indent << *BB;
    }
  else
    OS << Indent << "No BBSet\n";

  printChildren(OS, Depth + 1);
  OS << Indent << "} END WRNCriticalNode <" << getNumber() << ">\n\n";
#endif // !INTEL_PRODUCT_RELEASE
}

// constructor
WRNFlushNode::WRNFlushNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNFlush, BB) {
  setFlush(nullptr);
  DEBUG(dbgs() << "\nCreated WRNFlushNode<" << getNumber() << ">\n");
}

WRNFlushNode::WRNFlushNode(WRNFlushNode *W) : WRegionNode(W) {
  setFlush(W->getFlush());
  DEBUG(dbgs() << "\nCreated WRNFlushNode<" << getNumber() << ">\n");
}

void WRNFlushNode::print(formatted_raw_ostream &OS, unsigned Depth) const {
#if !INTEL_PRODUCT_RELEASE
  // TODO may need to have an extra parameter (or global) to add a fixed
  //    space for left margin at Depth=0
  std::string Indent(Depth * 2, ' ');

  OS << Indent << "BEGIN WRNFlushNode<" << getNumber() << "> {\n";

  // TODO: print data local to this Flush Set
  printChildren(OS, Depth + 1);
  OS << Indent << "} END WRNFlushNode<" << getNumber() << ">\n";
#endif // !INTEL_PRODUCT_RELEASE
}

//
// Methods for WRNTaskgroupNode
//

// constructor
WRNTaskgroupNode::WRNTaskgroupNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNTaskgroup, BB) {
  DEBUG(dbgs() << "\nCreated WRNTaskgroupNode <" << getNumber() << ">\n");
}

void WRNTaskgroupNode::print(formatted_raw_ostream &OS, unsigned Depth) const {
#if !INTEL_PRODUCT_RELEASE
  std::string Indent(Depth * 2, ' ');
  OS << Indent << "\nBEGIN WRNTaskgroupNode<" << getNumber() << "> {\n";
  printChildren(OS, Depth + 1);
  OS << Indent << "} END WRNTaskgroupNode <" << getNumber() << ">\n\n";
#endif // !INTEL_PRODUCT_RELEASE
}

//
// Methods for WRNTaskwaitNode
//

// constructor
WRNTaskwaitNode::WRNTaskwaitNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNTaskwait, BB) {
  DEBUG(dbgs() << "\nCreated WRNTaskwaitNode <" << getNumber() << ">\n");
}

void WRNTaskwaitNode::print(formatted_raw_ostream &OS, unsigned Depth) const {
#if !INTEL_PRODUCT_RELEASE
  std::string Indent(Depth * 2, ' ');
  OS << Indent << "\nBEGIN WRNTaskwaitNode<" << getNumber() << "> {\n";
  printChildren(OS, Depth + 1);
  OS << Indent << "} END WRNTaskwaitNode <" << getNumber() << ">\n\n";
#endif // !INTEL_PRODUCT_RELEASE
}

//
// Methods for WRNTaskyieldNode
//

// constructor
WRNTaskyieldNode::WRNTaskyieldNode(BasicBlock *BB)
    : WRegionNode(WRegionNode::WRNTaskyield, BB) {
  DEBUG(dbgs() << "\nCreated WRNTaskyieldNode <" << getNumber() << ">\n");
}

void WRNTaskyieldNode::print(formatted_raw_ostream &OS, unsigned Depth) const {
#if !INTEL_PRODUCT_RELEASE
  std::string Indent(Depth * 2, ' ');
  OS << Indent << "\nBEGIN WRNTaskyieldNode<" << getNumber() << "> {\n";
  printChildren(OS, Depth + 1);
  OS << Indent << "} END WRNTaskyieldNode <" << getNumber() << ">\n\n";
#endif // !INTEL_PRODUCT_RELEASE
}
