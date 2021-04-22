//===------ HIRPostVecCompleteUnroll.cpp - post vec complete unroll -------===//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
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

#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#if INTEL_INCLUDE_DTRANS
#include "Intel_DTrans/Analysis/DTransImmutableAnalysis.h"
#include "Intel_DTrans/DTransCommon.h"
#endif // INTEL_INCLUDE_DTRANS

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

#if INTEL_INCLUDE_DTRANS
  auto &MAMProxy = AM.getResult<ModuleAnalysisManagerFunctionProxy>(F);
#endif // INTEL_INCLUDE_DTRANS
  HIRCompleteUnroll(
      HIRF, AM.getResult<DominatorTreeAnalysis>(F),
      AM.getResult<TargetIRAnalysis>(F),
      AM.getResult<HIRLoopStatisticsAnalysis>(F),
      AM.getResult<HIRDDAnalysisPass>(F),
      AM.getResult<HIRSafeReductionAnalysisPass>(F),
#if INTEL_INCLUDE_DTRANS
      MAMProxy.getCachedResult<DTransImmutableAnalysis>(*F.getParent()),
#endif // INTEL_INCLUDE_DTRANS
      OptLevel, false, PragmaOnlyUnroll)
      .run();

  return PreservedAnalyses::all();
}

namespace {

class HIRPostVecCompleteUnrollLegacyPass : public HIRCompleteUnrollLegacyPass {
public:
  static char ID;

  HIRPostVecCompleteUnrollLegacyPass(unsigned OptLevel = 0,
                                     bool PragmaOnlyUnroll = false)
      : HIRCompleteUnrollLegacyPass(ID, OptLevel, false, PragmaOnlyUnroll) {
    initializeHIRPostVecCompleteUnrollLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    if (DisablePostVecUnroll) {
      return false;
    }

    return HIRCompleteUnrollLegacyPass::runOnFunction(F);
  }
};
} // namespace

char HIRPostVecCompleteUnrollLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRPostVecCompleteUnrollLegacyPass,
                      "hir-post-vec-complete-unroll",
                      "HIR PostVec Complete Unroll", false, false)
INITIALIZE_PASS_DEPENDENCY(OptReportOptionsPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRSafeReductionAnalysisWrapperPass)
#if INTEL_INCLUDE_DTRANS
INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
#endif // INTEL_INCLUDE_DTRANS
INITIALIZE_PASS_END(HIRPostVecCompleteUnrollLegacyPass,
                    "hir-post-vec-complete-unroll",
                    "HIR PostVec Complete Unroll", false, false)

FunctionPass *llvm::createHIRPostVecCompleteUnrollPass(unsigned OptLevel,
                                                       bool PragmaOnlyUnroll) {
  return new HIRPostVecCompleteUnrollLegacyPass(OptLevel, PragmaOnlyUnroll);
}
