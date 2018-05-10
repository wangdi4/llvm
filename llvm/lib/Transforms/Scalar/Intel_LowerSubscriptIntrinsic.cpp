//===- Intel_LowerSubscriptIntrinsic.cpp - Lower llvm.intel.subscript intrinsic ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass lowers the 'llvm.intel.subscript' intrinsic to explicit address
// computations.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/Intel_LowerSubscriptIntrinsic.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/IntrinsicInst.h"

#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/Utils/Local.h"
#include "llvm/Pass.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Transforms/Scalar.h"

using namespace llvm;

static cl::opt<bool> EnableSubscriptLowering(
    "enable-subscript-lowering", cl::Hidden, cl::init(true),
    cl::desc("Enable lowering llvm.intel.subscript lowering to pointer arithmetic"));

static bool lowerIntrinsics(Function &F) {

  if (!EnableSubscriptLowering)
    return false;

  Module *M = F.getParent();
  const DataLayout &DL = M->getDataLayout();

  bool Changed = false;
  for (BasicBlock &BB : F) {
    // Replace llvm.intel.subscript intrinsics.
    for (auto BI = BB.begin(), BE = BB.end(); BI != BE;) {
      Instruction &Inst = *BI++;
      SubscriptInst *CI = dyn_cast<SubscriptInst>(&Inst);
      if (!CI)
        continue;

      IRBuilder<> Builder(CI);
      Value *Offset[] = {EmitSubsOffset(&Builder, DL, CI)};
      CI->replaceAllUsesWith(
          Builder.CreateInBoundsGEP(CI->getPointerOperand(), Offset));
      salvageDebugInfo(*CI);
      CI->eraseFromParent();

      Changed = true;
    }
  }
  return Changed;
}

PreservedAnalyses
LowerSubscriptIntrinsicPass::run(Function &M, FunctionAnalysisManager &FM) {
  if (!lowerIntrinsics(M))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  // PA.preserve<AAManager>();    depends on other analyses.
  // PA.preserve<MemorySSAAnalysis>(); depends on GlobalsAA.
  // PA.preserve<BasicAA>();      done in later patch.
  // PA.preserve<GlobalsAA>();    done in later patch.
  // PA.preserve<AndersensAA>();  done in later patch.
  // PA.preserve<InlineAggAnalysis>(); postponed, very specific.
  // PA.preserve<ScalarEvolutionAnalysis>(); postponed.
  // PA.preserve<DependenceAnalysis>(); postponed.
  // PA.preserve<MemoryDependenceAnalysis>(); postponed.
  // PA.preserve<SCEVAA>(); postponed.
  PA.preserveSet<CFGAnalyses>();
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

namespace {
class LowerSubscriptIntrinsicLegacyPass : public FunctionPass {
public:
  static char ID;

  LowerSubscriptIntrinsicLegacyPass() : FunctionPass(ID) {
    initializeLowerSubscriptIntrinsicLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesCFG();
    AU.addPreserved<WholeProgramWrapperPass>();
  }

  bool runOnFunction(Function &F) override { return lowerIntrinsics(F); }

};
} // end anonymous namespace

char LowerSubscriptIntrinsicLegacyPass::ID = 0;

INITIALIZE_PASS(LowerSubscriptIntrinsicLegacyPass,
                "lower-subscript", "Subscript Intrinsic Lowering",
                false, false)

FunctionPass *llvm::createLowerSubscriptIntrinsicLegacyPass() {
  return new LowerSubscriptIntrinsicLegacyPass();
}

