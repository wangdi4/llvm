//===-- WRegionNode.cpp - Implements the WRegionNode class ----------------===//
//
//   Copyright (C) 2015 Intel Corporation. All rights reserved.
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
#include "llvm/Analysis/VPO/WRegionInfo/WRegionNode.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"

#define DEBUG_TYPE "vpo-wrnnode"

using namespace llvm;
using namespace llvm::vpo;

WRContainerTy llvm::vpo::WRegions;

unsigned WRegionNode::UniqueNum(0);

WRegionNode::WRegionNode(unsigned SCID) : SubClassID(SCID)  {
  setNextNumber();
  setParent(nullptr);
  setEntryBBlock(nullptr);
  setExitBBlock(nullptr);
  setBBlockSet(nullptr);
  setIsFromHIR(false);
}

WRegionNode::WRegionNode(WRegionNode *W)
    : SubClassID(W->SubClassID) {
  setNextNumber();   // can't reuse the same number; get a new one
  setParent(W->getParent());
  setEntryBBlock(W->getEntryBBlock()); setExitBBlock(W->getExitBBlock());
  setBBlockSet(W->getBBlockSet());
  setIsFromHIR(W->getIsFromHIR());
  //TODO: add code to copy Children?
}

void WRegionNode::doPreOrderSubCFGVisit(
  BasicBlock    *BB,
  BasicBlock    *ExitBB,
  SmallPtrSetImpl<BasicBlock*> *PreOrderTreeVisited
)
{
  if (!PreOrderTreeVisited->count(BB)) {

    // DEBUG(dbgs()<< "DUMP PreOrder Tree Visiting :"  << *BB);
    PreOrderTreeVisited->insert(BB);

    for (succ_iterator I = succ_begin(BB), 
                       E = succ_end(BB); I != E; ++I) {
      if (*I != ExitBB) {
        doPreOrderSubCFGVisit(*I, ExitBB, PreOrderTreeVisited);
      }
    }

  }
  return;
}

/// \brief Populates BBlockSet with BBs in the WRN from EntryBB to ExitBB.
void WRegionNode::populateBBlockSet(void)
{
  BasicBlock *EntryBB = getEntryBBlock();
  BasicBlock *ExitBB  = getExitBBlock();
  setBBlockSet(nullptr);

  if (!EntryBB || !ExitBB)
    return;

  SmallPtrSet<BasicBlock*, 16> PreOrderTreeVisited;

  doPreOrderSubCFGVisit(EntryBB, ExitBB, &PreOrderTreeVisited);

  /// Added ExitBBlock to Pre-Order Tree
  PreOrderTreeVisited.insert(ExitBB);

  WRegionBSetTy * BBSet = new (WRegionBSetTy);

  // for (SmallPtrSetIterator<BasicBlock *> I = PreOrderTreeVisited.begin(), 
  for (auto I = PreOrderTreeVisited.begin(), 
            E = PreOrderTreeVisited.end(); I != E; ++I) {
    BasicBlock *BB = *I;

    // DEBUG(dbgs()<< "SHOW BBSet BBLOCK Insert Ordering :"  << *BB);

    /// Populate BBlockSet for the Region/Loop
    BBSet->push_back(BB);
  }
  setBBlockSet(BBSet);
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

bool WRegionNode::hasChildren() const { 
  const WRegion* W = static_cast<const WRegion*>(this);
  return !(W->hasChildren()); 
}

/// \brief Returns the number of children.
unsigned WRegionNode::getNumChildren() const { 
  const WRegion* W = static_cast<const WRegion*>(this);
  return W->getNumChildren(); 
  return 0;
}

/// \brief Return address of the Children container
WRContainerTy &WRegionNode::getChildren() { 
  WRegion* W = static_cast<WRegion*>(this);
  return W->getChildren(); 
}

WRegionNode *WRegionNode::getFirstChild() {
  WRegion* W = static_cast<WRegion*>(this);
  return W->getFirstChild();
}

WRegionNode *WRegionNode::getLastChild() {
  WRegion* W = static_cast<WRegion*>(this);
  return W->getLastChild();
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

void WRegionNode::printChildren(formatted_raw_ostream &OS, 
                                unsigned Depth) const {
  const WRegion* W = static_cast<const WRegion*>(this);
  W->printChildren(OS, Depth);
}

