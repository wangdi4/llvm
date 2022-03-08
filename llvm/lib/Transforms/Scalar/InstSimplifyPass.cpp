//===- InstSimplifyPass.cpp -----------------------------------------------===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/InstSimplifyPass.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/InstructionSimplify.h"
#include "llvm/Analysis/Intel_WP.h"         // INTEL
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h" // INTEL
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/Local.h"

using namespace llvm;

#define DEBUG_TYPE "instsimplify"

STATISTIC(NumSimplified, "Number of redundant instructions removed");

static bool runImpl(Function &F, const SimplifyQuery &SQ,
                    OptimizationRemarkEmitter *ORE) {
  SmallPtrSet<const Instruction *, 8> S1, S2, *ToSimplify = &S1, *Next = &S2;
  bool Changed = false;

  do {
    for (BasicBlock &BB : F) {
      // Unreachable code can take on strange forms that we are not prepared to
      // handle. For example, an instruction may have itself as an operand.
      if (!SQ.DT->isReachableFromEntry(&BB))
        continue;

      SmallVector<WeakTrackingVH, 8> DeadInstsInBB;
      for (Instruction &I : BB) {
        // The first time through the loop, ToSimplify is empty and we try to
        // simplify all instructions. On later iterations, ToSimplify is not
        // empty and we only bother simplifying instructions that are in it.
        if (!ToSimplify->empty() && !ToSimplify->count(&I))
          continue;

        // Don't waste time simplifying dead/unused instructions.
        if (isInstructionTriviallyDead(&I)) {
          DeadInstsInBB.push_back(&I);
          Changed = true;
        } else if (!I.use_empty()) {
          if (Value *V = SimplifyInstruction(&I, SQ, ORE)) {
            // Mark all uses for resimplification next time round the loop.
            for (User *U : I.users())
              Next->insert(cast<Instruction>(U));
            I.replaceAllUsesWith(V);
            ++NumSimplified;
            Changed = true;
            // A call can get simplified, but it may not be trivially dead.
            if (isInstructionTriviallyDead(&I))
              DeadInstsInBB.push_back(&I);
          }
        }
      }
      RecursivelyDeleteTriviallyDeadInstructions(DeadInstsInBB, SQ.TLI);
    }

    // Place the list of instructions to simplify on the next loop iteration
    // into ToSimplify.
    std::swap(ToSimplify, Next);
    Next->clear();
  } while (!ToSimplify->empty());

  return Changed;
}

namespace {
struct InstSimplifyLegacyPass : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid
  InstSimplifyLegacyPass() : FunctionPass(ID) {
    initializeInstSimplifyLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addPreserved<WholeProgramWrapperPass>(); // INTEL
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<AssumptionCacheTracker>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<TargetTransformInfoWrapperPass>(); // INTEL
    AU.addRequired<OptimizationRemarkEmitterWrapperPass>();
  }

  /// Remove instructions that simplify.
  bool runOnFunction(Function &F) override {
    if (skipFunction(F))
      return false;

    const DominatorTree *DT =
        &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    const TargetLibraryInfo *TLI =
        &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    AssumptionCache *AC =
        &getAnalysis<AssumptionCacheTracker>().getAssumptionCache(F);
    OptimizationRemarkEmitter *ORE =
        &getAnalysis<OptimizationRemarkEmitterWrapperPass>().getORE();
    const DataLayout &DL = F.getParent()->getDataLayout();
#if INTEL_CUSTOMIZATION
    const TargetTransformInfo *TTI =
        &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
    const SimplifyQuery SQ(DL, TLI, DT, AC, nullptr, true, true, TTI);
#endif // INTEL_CUSTOMIZATION
    return runImpl(F, SQ, ORE);
  }
};

#if INTEL_CUSTOMIZATION
class UnskippableInstSimplifyLegacyPass : public InstSimplifyLegacyPass {
  bool skipFunction(const Function &F) const override { return false; }
};
#endif // INTEL_CUSTOMIZATION
} // namespace

char InstSimplifyLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(InstSimplifyLegacyPass, "instsimplify",
                      "Remove redundant instructions", false, false)
INITIALIZE_PASS_DEPENDENCY(AssumptionCacheTracker)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass) // INTEL
INITIALIZE_PASS_DEPENDENCY(OptimizationRemarkEmitterWrapperPass)
INITIALIZE_PASS_END(InstSimplifyLegacyPass, "instsimplify",
                    "Remove redundant instructions", false, false)

// Public interface to the simplify instructions pass.
FunctionPass *llvm::createInstSimplifyLegacyPass() {
  return new InstSimplifyLegacyPass();
}

#if INTEL_CUSTOMIZATION
FunctionPass *llvm::createUnskippableInstSimplifyLegacyPass() {
  return new UnskippableInstSimplifyLegacyPass();
}
#endif // INTEL_CUSTOMIZATION

PreservedAnalyses InstSimplifyPass::run(Function &F,
                                        FunctionAnalysisManager &AM) {
  auto &DT = AM.getResult<DominatorTreeAnalysis>(F);
  auto &TLI = AM.getResult<TargetLibraryAnalysis>(F);
#if INTEL_CUSTOMIZATION
  auto &TTI = AM.getResult<TargetIRAnalysis>(F);
#endif // INTEL_CUSTOMIZATION
  auto &AC = AM.getResult<AssumptionAnalysis>(F);
  auto &ORE = AM.getResult<OptimizationRemarkEmitterAnalysis>(F);
  const DataLayout &DL = F.getParent()->getDataLayout();
#if INTEL_CUSTOMIZATION
  // Pass TTI to support target-specific opts.
  const SimplifyQuery SQ(
    DL,
    &TLI,
    &DT,
    &AC,
    nullptr, /* CXTI */
    true, /* UseInstrInfo */
    true, /*CanUseUndef */
    &TTI);
#endif // INTEL_CUSTOMIZATION
  bool Changed = runImpl(F, SQ, &ORE);
  if (!Changed)
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  PA.preserve<WholeProgramAnalysis>();  // INTEL
  return PA;
}
