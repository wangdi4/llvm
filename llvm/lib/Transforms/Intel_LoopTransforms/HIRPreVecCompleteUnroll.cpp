//===------- HIRPreVecCompleteUnroll.cpp - pre vec complete unroll --------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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

#include "llvm/Transforms/Intel_LoopTransforms/HIRPreVecCompleteUnroll.h"
#include "HIRCompleteUnrollImpl.h"

#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"

using namespace llvm;
using namespace llvm::loopopt;

cl::opt<bool> DisablePreVecUnroll("disable-hir-pre-vec-complete-unroll",
                                  cl::desc("Disables pre vec complete unroll"),
                                  cl::Hidden, cl::init(false));

PreservedAnalyses
HIRPreVecCompleteUnrollPass::run(llvm::Function &F,
                                 llvm::FunctionAnalysisManager &AM) {
  if (DisablePreVecUnroll) {
    return PreservedAnalyses::all();
  }

  HIRCompleteUnroll(AM.getResult<HIRFrameworkAnalysis>(F),
                    AM.getResult<DominatorTreeAnalysis>(F),
                    AM.getResult<TargetIRAnalysis>(F),
                    AM.getResult<HIRLoopStatisticsAnalysis>(F),
                    AM.getResult<HIRDDAnalysisPass>(F),
                    AM.getResult<HIRSafeReductionAnalysisPass>(F), OptLevel,
                    true, PragmaOnlyUnroll)
      .run();

  return PreservedAnalyses::all();
}

namespace {

class HIRPreVecCompleteUnrollLegacyPass : public HIRCompleteUnrollLegacyPass {
public:
  static char ID;

  HIRPreVecCompleteUnrollLegacyPass(unsigned OptLevel = 0,
                                    bool PragmaOnlyUnroll = false)
      : HIRCompleteUnrollLegacyPass(ID, OptLevel, true, PragmaOnlyUnroll) {
    initializeHIRPreVecCompleteUnrollLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    if (DisablePreVecUnroll) {
      return false;
    }

    return HIRCompleteUnrollLegacyPass::runOnFunction(F);
  }
};

} // namespace

char HIRPreVecCompleteUnrollLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRPreVecCompleteUnrollLegacyPass,
                      "hir-pre-vec-complete-unroll",
                      "HIR PreVec Complete Unroll", false, false)
INITIALIZE_PASS_DEPENDENCY(OptReportOptionsPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRSafeReductionAnalysisWrapperPass)
INITIALIZE_PASS_END(HIRPreVecCompleteUnrollLegacyPass,
                    "hir-pre-vec-complete-unroll", "HIR PreVec Complete Unroll",
                    false, false)

FunctionPass *llvm::createHIRPreVecCompleteUnrollPass(unsigned OptLevel,
                                                      bool PragmaOnlyUnroll) {
  return new HIRPreVecCompleteUnrollLegacyPass(OptLevel, PragmaOnlyUnroll);
}
