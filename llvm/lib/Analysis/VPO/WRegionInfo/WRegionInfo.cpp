//===------- WRegionInfo.cpp - Build WRegion Graph ---------*- C++ -*------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the W-Region Information Graph build pass.
//
//===----------------------------------------------------------------------===//
#include "llvm/Pass.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegion.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionCollection.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionPasses.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/PostDominators.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-wrninfo"

INITIALIZE_PASS_BEGIN(WRegionInfo, "vpo-wrninfo", 
                                   "VPO Work-Region Information", false, true)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(PostDominatorTree)
INITIALIZE_PASS_DEPENDENCY(WRegionCollection)
INITIALIZE_PASS_END(WRegionInfo, "vpo-wrninfo", 
                                 "VPO Work-Region Information", false, true)

char WRegionInfo::ID = 0;

FunctionPass *llvm::createWRegionInfoPass() { return new WRegionInfo(); }

WRegionInfo::WRegionInfo() : FunctionPass(ID) {
  DEBUG(dbgs() << "\nStart W-Region Information Collection Pass\n\n");
  initializeWRegionInfoPass(*PassRegistry::getPassRegistry());
}

void WRegionInfo::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<DominatorTreeWrapperPass>();
  AU.addRequiredTransitive<PostDominatorTree>();
  AU.addRequiredTransitive<WRegionCollection>();
}

void WRegionInfo::doFillUpWRegionInfo(WRegionCollection *R) {
  //for (auto &I : R->getWRegions()) {
   DEBUG(dbgs() << "fill Up W-Region Info After WRegionCollection\n");
  //}
}

bool WRegionInfo::runOnFunction(Function &F) {
  this->Func = &F;

  DEBUG(dbgs() << "W-Region Information Collection Start\n");

  DT     = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  PDT    = &getAnalysis<PostDominatorTree>();
  WRC    = &getAnalysis<WRegionCollection>();

  // Set collected WRegions. This is to be properly fixed once WRegion
  // passes are fully implemented.
  setWRegions();

  doFillUpWRegionInfo(WRC);

  DEBUG(dbgs() << "W-Region Information Collection End\n");

  return false;
}

void WRegionInfo::releaseMemory() {
  WRegions.clear();
#if 0
  /// Destroy all WRegionNodes.
  WRegionUtils::destroyAll();
#endif
}

void WRegionInfo::print(raw_ostream &OS, const Module *M) const {
  formatted_raw_ostream FOS(OS);

  for (auto I = begin(), E = end(); I != E; ++I) {
    FOS << "\n";
    I->print(FOS, 0);
  }
}

void WRegionInfo::verifyAnalysis() const {
  // TODO: Implement later
}

void WRegionInfo::setWRegions() {
  DEBUG(dbgs() << "\nRC Size = " << WRC->getWRegionListSize() << "\n");

  for (auto I = WRC->begin(), E = WRC->end(); I != E; ++I) {

    // Naive Copy 
    if (WRegion *WNode = dyn_cast<WRegion>(I)) {
      BasicBlock *EntryBB   = WNode->getEntryBBlock(); 
      BasicBlock *ExitBB    = WNode->getExitBBlock();
      auto &BBSet           = WNode->getBBlockSet();
      LoopInfo const *LoopI = WNode->getLoopInfo();

      // New Node Added to List
      WRegion *NewNode = new WRegion(EntryBB, ExitBB, BBSet, LoopI);
      WRegions.push_back(NewNode); 
    }
  }
}
