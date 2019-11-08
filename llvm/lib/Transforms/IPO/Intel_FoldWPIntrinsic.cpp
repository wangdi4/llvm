//===-- Intel_FoldWPIntrinsic.cpp - Intrinsic wholeprogramsafe Lowering -*--===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// This pass traverses through the IR and replaces the calls to the intrinsic
// llvm.intel.wholeprogramsafe with true if whole program safe was detected.
// Else, replace the calls with false. The intrinsic
// llvm.intel.wholeprogramsafe should be removed completely after this process
// since it won't be lowered. See the language reference manual for more
// information.

#include "llvm/Transforms/IPO/Intel_FoldWPIntrinsic.h"

#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/Intel_XmainOptLevelPass.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/IPO.h"

using namespace llvm;

#define DEBUG_TYPE "intel-fold-wp-intrinsic"

// Name of the intrinsic for whole program safe
static cl::opt<StringRef> WholeProgramSafeIntrinName(
    "whole-program-safe-intrinsic", cl::init("llvm.intel.wholeprogramsafe"),
    cl::ReallyHidden);

// Fold the intrinsic llvm.intel.wholeprogramsafe
// into true or false depending on the result of the analysis
static bool foldIntrinsicWholeProgramSafe(Module &M, unsigned OptLevel,
                                          WholeProgramInfo *WPInfo) {

  Function *WhPrIntrin = M.getFunction(WholeProgramSafeIntrinName);

  if (!WhPrIntrin) {
    LLVM_DEBUG({
      dbgs() << "Intrinsic " << WholeProgramSafeIntrinName << " not found\n";
    });
    return false;
  }

  LLVMContext &Context = M.getContext();

  // If the optimization level is 0 then we are going to take the path
  // when whole program is not safe. This means that any optimization
  // wrapped in the intrinsic llvm.intel.wholeprogramsafe won't be
  // applied (e.g. devirtualization).
  bool WPResult = WPInfo->isWholeProgramSafe() && OptLevel > 0;
  ConstantInt *InitVal = (WPResult ?
                          ConstantInt::getTrue(Context) :
                          ConstantInt::getFalse(Context));

  LLVM_DEBUG({
    dbgs() << "Lowering intrinsic " << WhPrIntrin->getName()
           << " into " << (WPResult ? "TRUE (1)" : "FALSE (0)") << "\n";
  });

  while (!WhPrIntrin->use_empty()){
    // The intrinsic llvm.intel.wholeprogramsafe is only supported for
    // CallInst instructions. The only intrinsics that are allowed in
    // InvokeInst are: donothing, patchpoint, statepoint, coro_resume
    // and coro_destroy.
    CallInst *IntrinCall = cast<CallInst>(WhPrIntrin->user_back());

    LLVM_DEBUG({
      dbgs() << "  Intrinsic folded in function "
             << IntrinCall->getCaller()->getName() << "\n";
    });

    IntrinCall->replaceAllUsesWith(InitVal);
    IntrinCall->eraseFromParent();
  }

  assert(WhPrIntrin->use_empty() && "Whole Program Analysis: intrinsic"
          " llvm.intel.wholeprogramsafe wasn't removed correctly. There"
          "are call sites available.");

  WhPrIntrin->eraseFromParent();

  assert(!M.getFunction(WholeProgramSafeIntrinName) &&
          "Whole Program Analysis: intrinsic llvm.intel.wholeprogramsafe"
          " wasn't removed correctly. The function definition still available.");

  return true;
}

namespace {

struct IntelFoldWPIntrinsicLegacyPass : public ModulePass {
public:
  static char ID; // Pass identification, replacement for typeid
  IntelFoldWPIntrinsicLegacyPass() : ModulePass(ID) {
    initializeIntelFoldWPIntrinsicLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addPreserved<WholeProgramWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addRequired<XmainOptLevelWrapperPass>();
  }

  bool runOnModule(Module &M) override {

    WholeProgramInfo *WPInfo =
        &getAnalysis<WholeProgramWrapperPass>().getResult();

    // NOTE: The legacy pass manager uses two variables to represent the
    // optimization levels:
    //
    //   - OptLevel: stores the optimization level
    //               0 = -O0, 1 = -O1, 2 = -O2, 3 = -O3
    //
    //   - SizeLevel: stores if we are optimizing for size
    //               0 = no, 1 = Os, 2 = Oz
    //
    // The values of OptLevel can be 0, 1, 2 or 3.
    unsigned OptLevel = getAnalysis<XmainOptLevelWrapperPass>().getOptLevel();

    return foldIntrinsicWholeProgramSafe(M, OptLevel, WPInfo);
  }
};

} // namespace

char IntelFoldWPIntrinsicLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(IntelFoldWPIntrinsicLegacyPass,
                      "intel-fold-wp-intrinsic", "Intel fold WP intrinsic",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_DEPENDENCY(XmainOptLevelWrapperPass)
INITIALIZE_PASS_END(IntelFoldWPIntrinsicLegacyPass,
                    "intel-fold-wp-intrinsic", "Intel fold WP intrinsic",
                    false, false)

ModulePass *llvm::createIntelFoldWPIntrinsicLegacyPass() {
  return new IntelFoldWPIntrinsicLegacyPass();
}

IntelFoldWPIntrinsicPass::IntelFoldWPIntrinsicPass() {}

PreservedAnalyses IntelFoldWPIntrinsicPass::run(Module &M,
                                                ModuleAnalysisManager &AM) {

  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  // NOTE: The new pass manager uses an enum to represent the
  // optimization levels:
  //
  //   - PassBuilder::OptimizationLevel
  //
  //     PassBuilder::OptimizationLevel::O0 = -O0
  //     PassBuilder::OptimizationLevel::O1 = -O1
  //     PassBuilder::OptimizationLevel::O2 = -O2
  //     PassBuilder::OptimizationLevel::O3 = -O3
  //     PassBuilder::OptimizationLevel::Os = -Os
  //     PassBuilder::OptimizationLevel::Oz = -Oz
  //
  // The values of OptLevel can be 0, 1, 2, 3, 4 or 5. The options -Os and
  // -Oz are optimization level but for size. The compiler will treat these
  // two levels as -O2 but without increasing the size of the code. The main
  // difference between -Os and -Oz is that the second one does more aggressive
  // optimizations related to size.
  unsigned OptLevel = AM.getResult<XmainOptLevelAnalysis>(M).getOptLevel();

  if (!foldIntrinsicWholeProgramSafe(M, OptLevel, &WPInfo))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  PA.preserve<TargetLibraryAnalysis>();

  return PA;
}
