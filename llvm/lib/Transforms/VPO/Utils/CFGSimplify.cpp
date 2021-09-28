#if INTEL_COLLAB
//===------- CFGSimplify.cpp - Simplifies CFG after Paropt -*- C++ -*------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This files implements the CFGSimplify pass that cleans up CFG after
/// different Paropt transformations. For example, during the lowering
/// of OpenMP directives, Paropt may introduce redundant empty blocks,
/// which it does not keep track of for the sake of simplified implementation.
/// These empty blocks may affect the debugging of the original program.
/// CFGSimplify pass cleans-up the functions that were processed by Paropt
/// in any way. This pass is unskippable, because it must be run even at O0
/// to preserve the debuggability of the original program.
/// This pass uses "processed-by-vpo" Function attribute to detect the Functions
/// that were modified by Paropt and only simplifies CFG for these Functions.
///
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/DomTreeUpdater.h"
#include "llvm/InitializePasses.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/VPO/Utils/CFGSimplify.h"
#include "llvm/Transforms/VPO/VPOPasses.h"

using namespace llvm;

#define DEBUG_TYPE "vpo-cfg-simplify"
#define PASS_NAME "VPO CFG simplification"

namespace {
class VPOCFGSimplify : public FunctionPass {
public:
  static char ID;

  VPOCFGSimplify() : FunctionPass(ID) {
    initializeVPOCFGSimplifyPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return PASS_NAME; }

  bool runOnFunction(Function &F) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addPreserved<DominatorTreeWrapperPass>();
  }
};
}

char VPOCFGSimplify::ID = 0;
INITIALIZE_PASS(VPOCFGSimplify, DEBUG_TYPE, PASS_NAME, false, false)

FunctionPass *llvm::createVPOCFGSimplifyPass() {
  return new VPOCFGSimplify();
}

static bool simplifyCFG(Function &F, DominatorTree *DT) {
  // If the Function was not marked by any VPO pass, then
  // we skip the CFG simplification for it to avoid side-effects
  // caused just by adding -fiopenmp option.
  if (!F.hasFnAttribute("processed-by-vpo")) {
    LLVM_DEBUG(dbgs() << "VPOCFGSimplify did nothing.\n");
    return false;
  }

  DomTreeUpdater DTU(DT, DomTreeUpdater::UpdateStrategy::Eager);
  bool Changed = false;
  bool LocalChange = true;

  while (LocalChange) {
    LocalChange = false;

    for (Function::iterator BBIt = F.begin(); BBIt != F.end(); ) {
      BasicBlock &BB = *BBIt++;
      if (DT) {
        assert(
            !DTU.isBBPendingDeletion(&BB) &&
            "Should not end up trying to simplify blocks marked for removal.");
        // Make sure that the advanced iterator does not point at the blocks
        // that are marked for removal, skip over all such blocks.
        while (BBIt != F.end() && DTU.isBBPendingDeletion(&*BBIt))
          ++BBIt;
      }

      BasicBlock *PredBB = BB.getUniquePredecessor();
      if (!PredBB || !PredBB->getUniqueSuccessor())
        continue;

      Instruction *PredBBLastInst = PredBB->getTerminator();
      assert(PredBBLastInst && "Malformed basic block without a terminator.");
      Instruction *BBFirstInst = &*BB.begin();

      if (PredBBLastInst->hasMetadataOtherThanDebugLoc() ||
          BBFirstInst->hasMetadataOtherThanDebugLoc() ||
          PredBBLastInst->getDebugLoc() != BBFirstInst->getDebugLoc())
        continue;

      LocalChange |= MergeBlockIntoPredecessor(&BB, DT ? &DTU : nullptr);
    }

    Changed |= LocalChange;
  }

  LLVM_DEBUG(dbgs() << "VPOCFGSimplify changed function: " <<
             F.getName() << "\n");
  return Changed;
}

bool VPOCFGSimplify::runOnFunction(Function &F) {
  auto *DTWP = getAnalysisIfAvailable<DominatorTreeWrapperPass>();
  DominatorTree *DT = DTWP ? &DTWP->getDomTree() : nullptr;
  return simplifyCFG(F, DT);
}

PreservedAnalyses VPOCFGSimplifyPass::run(
    Function &F, FunctionAnalysisManager &FAM) {
  auto *DT = FAM.getCachedResult<DominatorTreeAnalysis>(F);

  bool Changed = simplifyCFG(F, DT);
  if (!Changed)
    return PreservedAnalyses::all();

  auto PA = PreservedAnalyses();
  PA.preserve<DominatorTreeAnalysis>();
  return PA;
}
#endif // INTEL_COLLAB
