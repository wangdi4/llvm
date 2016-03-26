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
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionUtils.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"
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
  // TODO may need to have an extra parameter (or global) to add a fixed
  //    space for left margin at Depth=0
  std::string Indent(Depth * 2, ' ');

  OS << Indent << "BEGIN WRNParallelNode<" << getNumber() << "> {\n";

  // TODO: print data local to this ParRegion
  //       eg shared vars, priv vars, etc.
  printChildren(OS, Depth + 1);
  OS << Indent << "} END WRNParallelNode<" << getNumber() << ">\n";
}

//
// Methods for WRNParallelLoopNode
//

// constructor
WRNParallelLoopNode::WRNParallelLoopNode(BasicBlock *BB, LoopInfo *Li)
    : WRegionNode(WRegionNode::WRNParallelLoop, BB), LI(Li) {
  setShared(nullptr);
  setPriv(nullptr);
  setFpriv(nullptr);
  setRed(nullptr);
  setCopyin(nullptr);
  setIf(nullptr);
  setNumThreads(nullptr);
  setDefault(WRNDefaultAbsent);
  setProcBind(WRNProcBindAbsent);
  setSchedule(WRNScheduleStaticEven);
  setCollapse(0);
  setOrdered(false);
  Loop *L = IntelGeneralUtils::getLoopFromLoopInfo(Li, BB);
  setLoop(L);

  DEBUG(dbgs() << "\nCreated WRNParallelLoopNode<" << getNumber() << ">\n");
}

WRNParallelLoopNode::WRNParallelLoopNode(WRNParallelLoopNode *W)
    : WRegionNode(W) {
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
  // TODO may need to have an extra parameter (or global) to add a fixed
  //    space for left margin at Depth=0
  std::string Indent(Depth * 2, ' ');

  OS << Indent << "BEGIN WRNParallelLoopNode<" << getNumber() << "> {\n";
  // TODO: print data local to this ParRegion
  //       eg shared vars, priv vars, etc.
  printChildren(OS, Depth + 1);
  OS << Indent << "} END WRNParallelLoopNode<" << getNumber() << ">\n";
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
}
