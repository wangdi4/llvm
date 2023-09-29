#if INTEL_FEATURE_SW_DTRANS
//===-- Intel_FoldWPIntrinsic.cpp - Intrinsic wholeprogramsafe Lowering ---===//
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

#include "llvm/ADT/DenseSet.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/Intel_XmainOptLevelPass.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/WholeProgramDevirt.h"

using namespace llvm;

#define DEBUG_TYPE "intel-fold-wp-intrinsic"

// Fold the intrinsic llvm.intel.wholeprogramsafe
// into true or false depending on the result of the analysis
static bool foldIntrinsicWholeProgramSafe(Module &M, unsigned OptLevel,
                                          WholeProgramInfo *WPInfo) {

  // Name of the intrinsic for whole program safe
  StringRef WholeProgramSafeIntrinName =
      llvm::Intrinsic::getName(llvm::Intrinsic::intel_wholeprogramsafe);

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

  // Since we have whole program then proceed to change the visibility of the
  // virtual calls. The only symbols that won't be devirtualized are those
  // marked as dynamic exports.
  //
  // This check is needed for the whole program devirtualization pass. The
  // community does this check at the beginning of LTO, but it depends on an
  // assumption flag. We can replace the assumption flag with the results from
  // the analysis. For more details about this check, please refer to this
  // change set from the community: https://reviews.llvm.org/D91583.
  //
  // NOTE: We may want to revisit this in the future. There is a chance that we
  // may miss a possible vitual call that has external symbols. This is very
  // common with MS Visual Studio libraries.
  DenseSet<GlobalValue::GUID> DynamicExportSymbols;
  auto WPLinkerUtils = WPInfo->getWholeProgramLinkerUtils();
  for (auto Symbol : WPLinkerUtils->getSymbolsResolution())
    if (Symbol.isExportDynamic())
      DynamicExportSymbols.insert(GlobalValue::getGUID(Symbol.getName()));

  updateVCallVisibilityInModule(M, WPInfo->isWholeProgramSafe(),
                                DynamicExportSymbols, false,
                                [](StringRef) { return true; });

  // If whole program is safe, transform llvm.public.test.type intrinsic to 
  // llvm.test.type, otherwise remove llvm.public.test.type from the module
  updatePublicTypeTestCalls(M, WPInfo->isWholeProgramSafe());

  return true;
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
#endif // INTEL_FEATURE_SW_DTRANS
