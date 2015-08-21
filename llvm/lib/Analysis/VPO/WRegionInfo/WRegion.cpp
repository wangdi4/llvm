//===----- WRegion.cpp - Implements the WRegion class ---------------------===//
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
//   This file implements the derived classes based on WRegionNode
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-wregion"

//
// Methods for WRegion
//

// constructors
WRegion::WRegion(unsigned SCID) : WRegionNode(SCID) {}
WRegion::WRegion(WRegionNode *W) : WRegionNode(W)   {}


WRegionNode *WRegion::getFirstChild() {
  if (hasChildren()) {
    return Children.begin();
  }
  return nullptr;
}

WRegionNode *WRegion::getLastChild() {
  if (hasChildren()) {
    return &(Children.back());
  }
  return nullptr;
}

void WRegion::printChildren(formatted_raw_ostream &OS, unsigned Depth) const {
  for (auto I = wrn_child_begin(), E = wrn_child_end(); I != E; ++I) {
    I->print(OS, Depth);
  }
}

//
// Methods for WRNParallelNode
//

// constructor
WRNParallelNode::WRNParallelNode() : WRegion(WRegionNode::WRNParallel)
{
  setShared(nullptr);
  setPriv(nullptr);
  setFpriv(nullptr);
  setRed(nullptr);
  setCopyin(nullptr);
  setIf(nullptr);
  setNumThreads(nullptr);
  setDefault(VPODefaultFalse);
  setProcBind(VPOProcBindFalse);
  DEBUG(dbgs() << "\nCreated WRNParallelNode<" << getNumber() <<">\n");
}

WRNParallelNode::WRNParallelNode(WRNParallelNode *W) : WRegion(W)
{
  setShared(W->getShared());
  setPriv(W->getPriv());
  setFpriv(W->getFpriv());
  setRed(W->getRed());
  setCopyin(W->getCopyin());
  setIf(W->getIf());
  setNumThreads(W->getNumThreads());
  setDefault(W->getDefault());
  setProcBind(W->getProcBind());
  DEBUG(dbgs() << "\nCreated WRNParallelNode<" << getNumber() <<">\n");
}



void WRNParallelNode::print(formatted_raw_ostream &OS, unsigned Depth) const {
  //TODO may need to have an extra parameter (or global) to add a fixed
  //    space for left margin at Depth=0
  std::string SpaceString(Depth*2, ' ');

  OS << SpaceString << "BEGIN WRNParallelNode<" << getNumber() << "> {\n";

    // TODO: print data local to this ParRegion
    //       eg shared vars, priv vars, etc.
    printChildren(OS, Depth+1);
  OS << SpaceString << "} END WRNParallelNode<" << getNumber() << ">\n";
}


//
// Methods for WRNVecLoopNode
//

// constructor
WRNVecLoopNode::WRNVecLoopNode() : WRegion(WRegionNode::WRNVecLoop)
{
  setPriv(nullptr);
  setLpriv(nullptr);
  setRed(nullptr);
  setLinear(nullptr);
  setAligned(nullptr);
  setSimdlen(0);
  setSafelen(0);
  setCollapse(0);
  setIsAutoVec(false);
  setLoopInfo(nullptr);
  DEBUG(dbgs() << "\nCreated WRNVecLoopNode<" << getNumber() <<">\n");
}

WRNVecLoopNode::WRNVecLoopNode(WRNVecLoopNode *W) : WRegion(W)
{
  setPriv(W->getPriv());
  setLpriv(W->getLpriv());
  setRed(W->getRed());
  setLinear(W->getLinear());
  setAligned(W->getAligned());
  setSimdlen(W->getSimdlen());
  setSafelen(W->getSafelen());
  setCollapse(W->getCollapse());
  setIsAutoVec(W->getIsAutoVec());
  setLoopInfo(W->getLoopInfo());
  DEBUG(dbgs() << "\nCreated WRNVecLoopNode<" << getNumber() <<">\n");
}


void WRNVecLoopNode::print(formatted_raw_ostream &OS, unsigned Depth) const 
{
  std::string SpaceString(Depth*2, ' ');
  OS << SpaceString << "BEGIN WRNVecLoopNode<" << getNumber() << "> {\n";

  // TODO: print data local to this VecLoop 

#if 1
    DEBUG(dbgs()<< "EntryBB:"  << *getEntryBBlock());
    DEBUG(dbgs()<< "\nExitBB:"  << *getExitBBlock());
    DEBUG(dbgs()<< "\nBBlockSet dump:\n");
    auto BBSet = getBBlockSet();
    if (BBSet && !(BBSet->empty())) {
      for (auto I=BBSet->begin(),E=BBSet->end(); I!=E; ++I) {
        const BasicBlock *BB = *I;
        DEBUG(dbgs() << *BB);
      }
    }
    else {
      DEBUG(dbgs() << "No BBSet\n");
    }
#endif

  printChildren(OS, Depth+1);
  OS << SpaceString << "} END WRNVecLoopNode<" << getNumber() << ">\n";
}
