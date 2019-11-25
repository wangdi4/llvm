//===- Mem2Reg.cpp - The -mem2reg pass, a wrapper around the Utils lib ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass is a simple pass wrapper around the PromoteMemToReg function call
// exposed by the Utils library.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/Mem2Reg.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AssumptionCache.h"
#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/Intel_WP.h"
#endif // INTEL_CUSTOMIZATION
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include <vector>

using namespace llvm;

#define DEBUG_TYPE "mem2reg"

STATISTIC(NumPromoted, "Number of alloca's promoted");

#if INTEL_CUSTOMIZATION
static bool promoteMemoryToRegisterForBB(Function &F, DominatorTree &DT,
                                         AssumptionCache &AC, BasicBlock &BB)
#endif // INTEL_CUSTOMIZATION
{
  std::vector<AllocaInst *> Allocas;
#if !INTEL_CUSTOMIZATION
  BasicBlock &BB = F.getEntryBlock(); // Get the entry node for the function
#endif // INTEL_CUSTOMIZATION
  bool Changed = false;
  while (true) {
    Allocas.clear();

    // Find allocas that are safe to promote, by looking at all instructions in
    // the entry node
    for (BasicBlock::iterator I = BB.begin(), E = --BB.end(); I != E; ++I)
      if (AllocaInst *AI = dyn_cast<AllocaInst>(I)) // Is it an alloca?
        if (isAllocaPromotable(AI))
          Allocas.push_back(AI);

    if (Allocas.empty())
      break;

    PromoteMemToReg(Allocas, DT, &AC);
    NumPromoted += Allocas.size();
    Changed = true;
  }
  return Changed;
}

#if INTEL_CUSTOMIZATION
static bool promoteMemoryToRegister(Function &F, DominatorTree &DT,
                                    AssumptionCache &AC, bool AllBBs) {
  if (!AllBBs) {
    BasicBlock &BB = F.getEntryBlock(); // Get the entry node for the function
    return promoteMemoryToRegisterForBB(F, DT, AC, BB);
  }

  bool Changed = false;
  for (auto &BB : F) {
    Changed = promoteMemoryToRegisterForBB(F, DT, AC, BB) || Changed;
  }
  return Changed;
}
#endif // INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
PreservedAnalyses PromotePass::run(Function &F, FunctionAnalysisManager &AM,
                                   bool AllBBs)
#endif // INTEL_CUSTOMIZATION
{
  auto &DT = AM.getResult<DominatorTreeAnalysis>(F);
  auto &AC = AM.getResult<AssumptionAnalysis>(F);
#if INTEL_CUSTOMIZATION
  if (!promoteMemoryToRegister(F, DT, AC, AllBBs))
    return PreservedAnalyses::all();
#endif // INTEL_CUSTOMIZATION

  auto PA = PreservedAnalyses();        // INTEL
  PA.preserve<WholeProgramAnalysis>();  // INTEL
  PA.preserve<GlobalsAA>();             // INTEL
  PA.preserve<AndersensAA>();           // INTEL

  PA.preserveSet<CFGAnalyses>();
  return PA;
}

namespace {

struct PromoteLegacyPass : public FunctionPass {
  // Pass identification, replacement for typeid
  static char ID;

#if INTEL_CUSTOMIZATION
  bool Unskippable;
  bool AllBBs;
  PromoteLegacyPass(bool Unskippable = false, bool AllBBs = false)
      : FunctionPass(ID), Unskippable(Unskippable), AllBBs(AllBBs) {
    initializePromoteLegacyPassPass(*PassRegistry::getPassRegistry());
  }
#endif // INTEL_CUSTOMIZATION

  // runOnFunction - To run this pass, first we calculate the alloca
  // instructions that are safe for promotion, then we promote each one.
  bool runOnFunction(Function &F) override {

#if INTEL_CUSTOMIZATION
    if (!Unskippable && skipFunction(F))
      return false;
#endif // INTEL_CUSTOMIZATION

    DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    AssumptionCache &AC =
        getAnalysis<AssumptionCacheTracker>().getAssumptionCache(F);
#if INTEL_CUSTOMIZATION
    return promoteMemoryToRegister(F, DT, AC, AllBBs);
#endif // INTEL_CUSTOMIZATION
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<AssumptionCacheTracker>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.setPreservesCFG();
#if INTEL_CUSTOMIZATION
    AU.addPreserved<AndersensAAWrapperPass>();
    AU.addPreserved<GlobalsAAWrapperPass>();
#endif // INTEL_CUSTOMIZATION
    AU.addPreserved<GlobalsAAWrapperPass>();       // INTEL
    AU.addPreserved<WholeProgramWrapperPass>();    // INTEL
  }
};

} // end anonymous namespace

char PromoteLegacyPass::ID = 0;

INITIALIZE_PASS_BEGIN(PromoteLegacyPass, "mem2reg", "Promote Memory to "
                                                    "Register",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(AssumptionCacheTracker)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AndersensAAWrapperPass)    // INTEL
INITIALIZE_PASS_END(PromoteLegacyPass, "mem2reg", "Promote Memory to Register",
                    false, false)

// createPromoteMemoryToRegister - Provide an entry point to create this pass.
#if INTEL_CUSTOMIZATION
FunctionPass *llvm::createPromoteMemoryToRegisterPass(bool Unskippable,
                                                      bool AllBBs) {
  return new PromoteLegacyPass(Unskippable, AllBBs);
}
#endif // INTEL_CUSTOMIZATION
