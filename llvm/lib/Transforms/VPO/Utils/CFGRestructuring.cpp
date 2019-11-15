#if INTEL_COLLAB
//===---------- CFGRestructuring.cpp - Restructures CFG *- C++ -*----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the CFGRestructuring member function of VPOUtils class.
/// The function restructures the CFG for a routine, where each directive for
/// Cilk, OpenMP, Offload, Vectorization is put into a standalone basic block.
/// This is a pre-required process for constructing WRegion for a routine.
///
//===----------------------------------------------------------------------===//

#include <set>
#include "llvm/IR/Function.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/VPO/Utils/CFGRestructuring.h"
#include "llvm/Transforms/VPO/VPOPasses.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/InitializePasses.h"
#include "llvm/ADT/Twine.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-cfgrestructuring"

/// \brief Auxiliary routine to split a BB at SplitPoint
static void splitBB(Instruction *SplitPoint, DominatorTree *DT, LoopInfo *LI,
                    StringRef &NewName, unsigned &Counter) {
  BasicBlock *OrigBB = SplitPoint->getParent();
  BasicBlock *NewBB = SplitBlock(OrigBB, SplitPoint, DT, LI);
  NewBB->setName(NewName + "." + Twine(++Counter));
}

/// Directives are represented with @llvm.directive.region.entry/exit
/// intrinsics. WRegion Construction requires that each such intrinsic be on
/// a BasicBlock by itself, with no other instructions in the same BB except
/// for the terminating unconditional branch.
/// This is done by splitting the BB before and after the intrinsic.
void VPOUtils::CFGRestructuring(Function &F, DominatorTree *DT, LoopInfo *LI) {

  LLVM_DEBUG(dbgs() << "VPO CFG Restructuring \n");

  // Find all the intrinsic calls representing directive begin/end, and store
  // them in the set InstructionsToSplit.
  SmallVector<Instruction *, 8> InstructionsToSplit;
  InstructionsToSplit.clear();
  for (Function::iterator B = F.begin(), BE = F.end(); B != BE; ++B)
    for (BasicBlock::iterator I = B->begin(), IE = B->end(); I != IE; ++I)
      if (VPOAnalysisUtils::isRegionDirective(&*I))
        InstructionsToSplit.push_back(&*I);

  BasicBlock *FunctionEntryBB = &(F.getEntryBlock());
  unsigned Counter = 0; // Used to create unique names for newly created BBs

  // Go through InstructionsToSplit to split the BBs according to the
  // aforementioned rules.
  for (Instruction *I : InstructionsToSplit) {

    StringRef DirString = VPOAnalysisUtils::getDirectiveString(I);
    if (!VPOAnalysisUtils::isOpenMPDirective(DirString))
      // Do not process unrelated directives as they may not need restructuring
      continue;

    // Get the basic block where this instruction resides in.
    BasicBlock *BB = I->getParent();
    bool IsFunctionEntry = (BB == FunctionEntryBB);

    // Split before I.
    // Optimization: skip this if I is BB's first instruction && BB has only
    // one predecessor && BB is not FunctionEntryBB.
    if (IsFunctionEntry ||
        (I != &(BB->front())) ||
        (std::distance(pred_begin(BB), pred_end(BB))>1))
      splitBB(I, DT, LI, DirString, Counter);

    if (VPOAnalysisUtils::isBeginDirectiveOfRegionsNeedingOutlining(DirString))
      // For regions that need outlining, create an extra empty BBlock before
      // the region entry's BBlock, otherwise code-generation for the region may
      // hinder the Loop recognition of any outer loop construct, if present.
      // Check par_in_section.ll Lit test for reference.
      splitBB(I, DT, LI, DirString, Counter);

    // Split after I.
    BasicBlock::iterator Inst(I);
    Instruction *SplitPoint = &*(++Inst);
    // Optimization: skip this if I's successor is an unconditional branch
    // instruction.
    BranchInst *BI = dyn_cast<BranchInst>(SplitPoint);
    if (BI && BI->isUnconditional())
      continue; // skip; don't split
    splitBB(SplitPoint, DT, LI, DirString, Counter);
  }
}

using namespace llvm;
using namespace llvm::vpo;

namespace {
class VPOCFGRestructuring : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid
  VPOCFGRestructuringPass Impl;

  VPOCFGRestructuring() : FunctionPass(ID) {
    initializeVPOCFGRestructuringPass(*PassRegistry::getPassRegistry());
  }
  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};
} // anonymous namespace

INITIALIZE_PASS_BEGIN(VPOCFGRestructuring, "vpo-cfg-restructuring",
                      "VPO CFG Restructuring", false, false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_END(VPOCFGRestructuring, "vpo-cfg-restructuring",
                    "VPO CFGRestructuring", false, false)

char VPOCFGRestructuring::ID = 0;

bool VPOCFGRestructuring::runOnFunction(Function &F) {
  if (VPOAnalysisUtils::skipFunctionForOpenmp(F) && skipFunction(F))
    return false;

  auto DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  auto LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

  return Impl.runImpl(F, DT, LI);
}

bool VPOCFGRestructuringPass::runImpl(Function &F, DominatorTree *DT,
                                      LoopInfo *LI) {

  VPOUtils::CFGRestructuring(F, DT, LI);

  return true;
}

void VPOCFGRestructuring::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<LoopInfoWrapperPass>();
}

PreservedAnalyses VPOCFGRestructuringPass::run(Function &F,
                                               FunctionAnalysisManager &AM) {
  auto &DT = AM.getResult<DominatorTreeAnalysis>(F);
  auto &LI = AM.getResult<LoopAnalysis>(F);

  bool Changed = runImpl(F, &DT, &LI);

  if (!Changed)
    return PreservedAnalyses::all();

  return PreservedAnalyses::none();
}

FunctionPass *llvm::createVPOCFGRestructuringPass() {
  return new VPOCFGRestructuring();
}
#endif // INTEL_COLLAB
