//===------- WRegionInfo.cpp - Build WRegion Graph ---------*- C++ -*------===//
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
// This file implements the W-Region Information Graph build pass.
//
//===----------------------------------------------------------------------===//
#include "llvm/Pass.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionCollection.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionPasses.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/PostDominators.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-wrninfo"

INITIALIZE_PASS_BEGIN(WRegionInfo, "vpo-wrninfo", 
                                   "VPO Work-Region Information", false, true)
INITIALIZE_PASS_DEPENDENCY(WRegionCollection)
INITIALIZE_PASS_END(WRegionInfo, "vpo-wrninfo", 
                                 "VPO Work-Region Information", false, true)

char WRegionInfo::ID = 0;

FunctionPass *llvm::createWRegionInfoPass() { return new WRegionInfo(); }

WRegionInfo::WRegionInfo() : FunctionPass(ID) {
  // DEBUG(dbgs() << "\nStart W-Region Information Collection Pass\n\n");
  initializeWRegionInfoPass(*PassRegistry::getPassRegistry());
}

void WRegionInfo::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<WRegionCollection>();
}

bool WRegionInfo::runOnFunction(Function &F) {
  DEBUG(dbgs() << "\nENTER WRegionInfo::runOnFunction: " 
               << F.getName() << "{\n");
  Func = &F;
  WRC  = &getAnalysis<WRegionCollection>();
  DT   = WRC->getDomTree();   // propagate analysis results
  LI   = WRC->getLoopInfo();
  SE   = WRC->getSE();

  DEBUG(dbgs() << "\n}EXIT WRegionInfo::runOnFunction: " 
               << F.getName() << "\n");
  return false;
}

void WRegionInfo::buildWRGraph(WRegionCollection::InputIRKind IR) {
  DEBUG(dbgs() << "\nENTER WRegionInfo::buildWRGraph(InpuIR=" 
               << IR <<"){\n");

  WRC->buildWRGraph(IR);

  DEBUG(dbgs() << "\nRC Size = " << WRC->getWRGraphSize() << "\n");
  for (auto I = WRC->begin(), E = WRC->end(); I != E; ++I)
    DEBUG((*I)->dump());

  DEBUG(dbgs() << "\n}EXIT WRegionInfo::buildWRGraph\n");
}

void WRegionInfo::releaseMemory() {
}

void WRegionInfo::print(raw_ostream &OS, const Module *M) const {
#if !INTEL_PRODUCT_RELEASE
  formatted_raw_ostream FOS(OS);

  for (auto I = begin(), E = end(); I != E; ++I) {
    FOS << "\n";
    (*I)->print(FOS, 0);
  }
#endif // !INTEL_PRODUCT_RELEASE
}

void WRegionInfo::verifyAnalysis() const {
  // TODO: Implement later
}
