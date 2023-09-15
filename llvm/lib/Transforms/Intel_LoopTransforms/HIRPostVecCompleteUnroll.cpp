//===------ HIRPostVecCompleteUnroll.cpp - post vec complete unroll -------===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// Wrapper over HIRCompleteUnroll pass. This is executed after vectorizer. The
// idea is to unroll loops which have not been vectorized. The profitability
// threshold is therefore smaller than prevec complete unroll.
//===----------------------------------------------------------------------===//
//

#include "llvm/Transforms/Intel_LoopTransforms/HIRPostVecCompleteUnrollPass.h"
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

cl::opt<bool>
    DisablePostVecUnroll("disable-hir-post-vec-complete-unroll",
                         cl::desc("Disables post vec complete unroll"),
                         cl::Hidden, cl::init(false));

PreservedAnalyses HIRPostVecCompleteUnrollPass::runImpl(
    llvm::Function &F, llvm::FunctionAnalysisManager &AM, HIRFramework &HIRF) {
  if (DisablePostVecUnroll) {
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
          OptLevel, false, PragmaOnlyUnroll)
          .run();

  return PreservedAnalyses::all();
}
