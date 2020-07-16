//===----------------------- HIRSumWindowReuse.cpp ------------------------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
/// \file
/// This file implements a sum window reuse pass which transforms certain
/// sliding window sums to avoid re-loading and re-adding overlapping terms
/// between outer loop iterations.
///
/// For example:
///
/// \code
/// + DO i1 = 0, N-K, 1
/// |   %sum = 0.0;
/// |
/// |   + DO i2 = 0, K-1, 1
/// |   |   %sum = %sum + (%A)[i1 + i2];
/// |   + END LOOP
/// |
/// |   (%B)[i1] = %sum;
/// + END LOOP
///
/// ===>
///
///   %sum = 0.0;
/// + DO i1 = 0, N-K, 1
/// |
/// |   if (i1 == 0)
/// |   {
/// |      + DO i2 = 0, K-1, 1
/// |      |   %sum = %sum + (%A)[i2];
/// |      + END LOOP
/// |   }
/// |   else
/// |   {
/// |      %sum = %sum - (%A)[i1 - 1];
/// |      %sum = %sum + (%A)[i1 + K-1];
/// |   }
/// |
/// |   (%B)[i1] = %sum;
/// + END LOOP
/// \endcode
///
/// To understand this transform, it can be helpful to visualize the sliding
/// window sum. This table shows the terms in each sum calculated in the example
/// above (for N=7, K=4):
///
/// |      | A[0] | A[1] | A[2] | A[3] | A[4] | A[5] | A[6] |
/// | ---: | :--: | :--: | :--: | :--: | :--: | :--: | :--: |
/// | i1=0 |  +   |  +   |  +   |  +   |      |      |      |
/// | i1=1 |      |  +   |  +   |  +   |  +   |      |      |
/// | i1=2 |      |      |  +   |  +   |  +   |  +   |      |
/// | i1=3 |      |      |      |  +   |  +   |  +   |  +   |
///
/// For `i1=0`, the sum is calculated as `S0=A[0]+A[1]+A[2]+A[3]` regardless of
/// whether this transform has applied. However, the window for the `i1=1` sum
/// has a significant overlap with the `i1=0` sum's window, and when this
/// transform applies it can be calculated as `S1=S0-A[0]+A[4]` rather than
/// `S1=A[1]+A[2]+A[3]+A[4]`, reusing the terms in the overlap between the
/// window via the `S0` value that was calculated in the previous iteration.
/// For these types of sliding windows, this re-use can be very beneficial for
/// reducing the amount of computation needed for successive sums.
///
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/Intel_LoopTransforms/HIRSumWindowReuse.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"

#define OPT_SWITCH "hir-sum-window-reuse"
#define OPT_DESC "HIR Sum Window Reuse"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass{"disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass")};

static bool runHIRSumWindowReuse(HIRFramework &, HIRDDAnalysis &) {
  if (DisablePass) {
    LLVM_DEBUG(dbgs() << OPT_DESC " Disabled\n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "[NYI] Initial placeholder pass for " OPT_DESC "\n");

  return false;
}

namespace {

/// A wrapper for running HIRSumWindowReuse with the old pass manager.
class HIRSumWindowReuseLegacyPass : public HIRTransformPass {
public:
  static char ID;
  HIRSumWindowReuseLegacyPass() : HIRTransformPass{ID} {
    initializeHIRSumWindowReuseLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<HIRFrameworkWrapperPass>();
    AU.addRequired<HIRDDAnalysisWrapperPass>();
    AU.setPreservesAll();
  }
};

} // namespace

char HIRSumWindowReuseLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRSumWindowReuseLegacyPass, OPT_SWITCH, OPT_DESC, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_END(HIRSumWindowReuseLegacyPass, OPT_SWITCH, OPT_DESC, false,
                    false)

FunctionPass *llvm::createHIRSumWindowReusePass() {
  return new HIRSumWindowReuseLegacyPass{};
}

bool HIRSumWindowReuseLegacyPass::runOnFunction(Function &F) {
  if (skipFunction(F)) {
    return false;
  }

  return runHIRSumWindowReuse(getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
                              getAnalysis<HIRDDAnalysisWrapperPass>().getDDA());
}

PreservedAnalyses
HIRSumWindowReusePass::run(Function &F, llvm::FunctionAnalysisManager &AM) {
  runHIRSumWindowReuse(AM.getResult<HIRFrameworkAnalysis>(F),
                       AM.getResult<HIRDDAnalysisPass>(F));
  return PreservedAnalyses::all();
}
