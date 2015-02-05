//===- RegionIdentification.cpp - Identifies HIR Regions *- C++ -*---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HIR Region Identification pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/Analysis/Intel_LoopAnalysis/RegionIdentification.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-region"

/// TODO: look into why this doesn't work as a substitute of using RegisterPass
/// directly.
//INITIALIZE_PASS_BEGIN(RegionIdentification, "hir-region",
//                    "HIR Region Identification", false, true)
//INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
//INITIALIZE_PASS_DEPENDENCY(LoopInfo)
//INITIALIZE_PASS_DEPENDENCY(LoopSimplify)
//INITIALIZE_PASS_DEPENDENCY(LCSSA)
//INITIALIZE_PASS_DEPENDENCY(ScalarEvolution)
//INITIALIZE_PASS_END(RegionIdentification, "hir-region",
//                    "HIR Region Identification", false, true)

static RegisterPass<RegionIdentification> X("hir-region",
                    "HIR Region Identification", false, true);

char RegionIdentification::ID = 0;

FunctionPass *llvm::createRegionIdentificationPass() { 
  return new RegionIdentification(); 
}

RegionIdentification::RegionIdentification()
  : FunctionPass(ID) {
// Required with INITIALIZE_PASS* macros
// initializeRegionIdentificationPass(*PassRegistry::getPassRegistry());
}

void RegionIdentification::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<LoopInfo>();
  AU.addRequiredTransitive<DominatorTreeWrapperPass>();
  AU.addRequiredTransitive<ScalarEvolution>();
}

bool RegionIdentification::isCandidateLoop(Loop& Lp) {

  /// Return false if we cannot handle this loop
  if (!Lp.isLoopSimplifyForm()) {
    return false;
  }

  /// Return false if it contains inner loops.
  if (!Lp.empty()) {
    return false;
  }

  const SCEV *BETC = SE->getBackedgeTakenCount(&Lp);

  /// Return false if the trip count of the loop is not computable.
  if (isa<SCEVCouldNotCompute>(BETC)) {
    return false;
  }

  return true;
}
 
void RegionIdentification::formRegions() {

  /// Create regions out of outermost stand-alone loops in the function.
  /// Needs to be refined to look into inner loops.
  for (LoopInfo::iterator I = LI->begin(), E = LI->end(); I != E; ++I) {

    /// Will be extended to include multiple loops in the same region.
    if (isCandidateLoop(**I)) {
      RegionBBlocks.push_back( std::make_pair( (*I)->getHeader(), 
        std::set<BasicBlock*>((*I)->getBlocks().begin(), 
          (*I)->getBlocks().end()) ) );
    }
  }
}

bool RegionIdentification::runOnFunction(Function &F) {
  this->Func = &F;

  LI = &getAnalysis<LoopInfo>();
  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  SE = &getAnalysis<ScalarEvolution>();

  formRegions();

  return false;
}

void RegionIdentification::releaseMemory() {
  RegionBBlocks.clear();
}

void RegionIdentification::print(raw_ostream &OS, const Module* M) 
  const { 
  ///TODO: implement later
}

void RegionIdentification::verifyAnalysis() const {
  ///TODO: implement later
}


