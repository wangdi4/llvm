//===------- HIRPreVecCompleteUnroll.cpp - pre vec complete unroll --------===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// Wrapper over HIRCompleteUnroll pass. This is executed before vectorizer. The
// idea is to unroll loopnests which seems to be highly profitable before they
// get to the vectorizer so the profitability threshold is set to a higher
// value.
//===----------------------------------------------------------------------===//
//

#include "llvm/Transforms/Intel_LoopTransforms/HIRPreVecCompleteUnrollPass.h"
#include "HIRCompleteUnroll.h"

#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#if INTEL_FEATURE_SW_DTRANS
#include "Intel_DTrans/Analysis/DTransImmutableAnalysis.h"
#endif // INTEL_FEATURE_SW_DTRANS

using namespace llvm;
using namespace llvm::loopopt;

cl::opt<bool> DisablePreVecUnroll("disable-hir-pre-vec-complete-unroll",
                                  cl::desc("Disables pre vec complete unroll"),
                                  cl::Hidden, cl::init(false));

PreservedAnalyses HIRPreVecCompleteUnrollPass::runImpl(
    llvm::Function &F, llvm::FunctionAnalysisManager &AM, HIRFramework &HIRF) {
  if (DisablePreVecUnroll) {
    return PreservedAnalyses::all();
  }

#if INTEL_FEATURE_SW_DTRANS
  auto &MAMProxy = AM.getResult<ModuleAnalysisManagerFunctionProxy>(F);
#endif // INTEL_FEATURE_SW_DTRANS
  ModifiedHIR =
      HIRCompleteUnroll(
          HIRF, AM.getResult<DominatorTreeAnalysis>(F),
          AM.getResult<TargetIRAnalysis>(F),
          AM.getResult<TargetLibraryAnalysis>(F),
          AM.getResult<HIRLoopStatisticsAnalysis>(F),
          AM.getResult<HIRDDAnalysisPass>(F),
          AM.getResult<HIRSafeReductionAnalysisPass>(F),
#if INTEL_FEATURE_SW_DTRANS
          MAMProxy.getCachedResult<DTransImmutableAnalysis>(*F.getParent()),
#endif // INTEL_FEATURE_SW_DTRANS
          OptLevel, true, PragmaOnlyUnroll)
          .run();

  return PreservedAnalyses::all();
}
