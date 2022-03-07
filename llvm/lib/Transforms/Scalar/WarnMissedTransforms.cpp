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
//===- LoopTransformWarning.cpp -  ----------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Emit warnings if forced code transformations have not been performed.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/WarnMissedTransforms.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Support/CommandLine.h" // INTEL_CUSTOMIZATION

using namespace llvm;

#define DEBUG_TYPE "transform-warning"

#if INTEL_CUSTOMIZATION
static cl::opt<bool> VPlanSIMDAssertDefault(
    "vplan-simd-assert-default", cl::init(false),
    cl::desc("Emits assert if pragma simd loop is not vectorized by VPlan"));
#endif

/// Emit warnings for forced (i.e. user-defined) loop transformations which have
/// still not been performed.
#if INTEL_CUSTOMIZATION
static void
warnAboutLoopLeftoverTransformations(Loop *L, Function *F,
                                     OptimizationRemarkEmitter *ORE) {
#else
static void warnAboutLeftoverTransformations(Loop *L,
                                             OptimizationRemarkEmitter *ORE) {
#endif
  if (hasUnrollTransformation(L) == TM_ForcedByUser) {
    LLVM_DEBUG(dbgs() << "Leftover unroll transformation\n");
    ORE->emit(
        DiagnosticInfoOptimizationFailure(DEBUG_TYPE,
                                          "FailedRequestedUnrolling",
                                          L->getStartLoc(), L->getHeader())
        << "loop not unrolled: the optimizer was unable to perform the "
           "requested transformation; the transformation might be disabled or "
           "specified as part of an unsupported transformation ordering");
  }

  if (hasUnrollAndJamTransformation(L) == TM_ForcedByUser) {
    LLVM_DEBUG(dbgs() << "Leftover unroll-and-jam transformation\n");
    ORE->emit(
        DiagnosticInfoOptimizationFailure(DEBUG_TYPE,
                                          "FailedRequestedUnrollAndJamming",
                                          L->getStartLoc(), L->getHeader())
        << "loop not unroll-and-jammed: the optimizer was unable to perform "
           "the requested transformation; the transformation might be disabled "
           "or specified as part of an unsupported transformation ordering");
  }

  if (hasVectorizeTransformation(L) == TM_ForcedByUser) {
    LLVM_DEBUG(dbgs() << "Leftover vectorization transformation\n");
    Optional<ElementCount> VectorizeWidth =
        getOptionalElementCountLoopAttribute(L);
    Optional<int> InterleaveCount =
        getOptionalIntLoopAttribute(L, "llvm.loop.interleave.count");

    if (!VectorizeWidth || VectorizeWidth->isVector())
#if INTEL_CUSTOMIZATION
    {
      if (VPlanSIMDAssertDefault)
        F->getContext().diagnose(DiagnosticInfoUnsupported(
            *F,
             "loop not vectorized: the optimizer was unable to perform the "
             "requested transformation; the transformation might be disabled "
             "or specified as part of an unsupported transformation ordering",
             L->getStartLoc()));
      else
#endif // INTEL_CUSTOMIZATION
      ORE->emit(
          DiagnosticInfoOptimizationFailure(DEBUG_TYPE,
                                            "FailedRequestedVectorization",
                                            L->getStartLoc(), L->getHeader())
          << "loop not vectorized: the optimizer was unable to perform the "
             "requested transformation; the transformation might be disabled "
             "or specified as part of an unsupported transformation ordering");
    } // INTEL
    else if (InterleaveCount.getValueOr(0) != 1)
      ORE->emit(
          DiagnosticInfoOptimizationFailure(DEBUG_TYPE,
                                            "FailedRequestedInterleaving",
                                            L->getStartLoc(), L->getHeader())
          << "loop not interleaved: the optimizer was unable to perform the "
             "requested transformation; the transformation might be disabled "
             "or specified as part of an unsupported transformation ordering");
  }

  if (hasDistributeTransformation(L) == TM_ForcedByUser) {
    LLVM_DEBUG(dbgs() << "Leftover distribute transformation\n");
    ORE->emit(
        DiagnosticInfoOptimizationFailure(DEBUG_TYPE,
                                          "FailedRequestedDistribution",
                                          L->getStartLoc(), L->getHeader())
        << "loop not distributed: the optimizer was unable to perform the "
           "requested transformation; the transformation might be disabled or "
           "specified as part of an unsupported transformation ordering");
  }
}

static void warnAboutLeftoverTransformations(Function *F, LoopInfo *LI,
                                             OptimizationRemarkEmitter *ORE) {
  for (auto *L : LI->getLoopsInPreorder())
#if INTEL_CUSTOMIZATION
    warnAboutLoopLeftoverTransformations(L, F, ORE);
#else
    warnAboutLeftoverTransformations(L, ORE);
#endif
}

// New pass manager boilerplate
PreservedAnalyses
WarnMissedTransformationsPass::run(Function &F, FunctionAnalysisManager &AM) {
  // Do not warn about not applied transformations if optimizations are
  // disabled.
  if (F.hasOptNone())
    return PreservedAnalyses::all();

  auto &ORE = AM.getResult<OptimizationRemarkEmitterAnalysis>(F);
  auto &LI = AM.getResult<LoopAnalysis>(F);

  warnAboutLeftoverTransformations(&F, &LI, &ORE);

  return PreservedAnalyses::all();
}

// Legacy pass manager boilerplate
namespace {
class WarnMissedTransformationsLegacy : public FunctionPass {
public:
  static char ID;

  explicit WarnMissedTransformationsLegacy() : FunctionPass(ID) {
    initializeWarnMissedTransformationsLegacyPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F))
      return false;

    auto &ORE = getAnalysis<OptimizationRemarkEmitterWrapperPass>().getORE();
    auto &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

    warnAboutLeftoverTransformations(&F, &LI, &ORE);
    return false;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<OptimizationRemarkEmitterWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();

    AU.setPreservesAll();
  }
};
} // end anonymous namespace

char WarnMissedTransformationsLegacy::ID = 0;

INITIALIZE_PASS_BEGIN(WarnMissedTransformationsLegacy, "transform-warning",
                      "Warn about non-applied transformations", false, false)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(OptimizationRemarkEmitterWrapperPass)
INITIALIZE_PASS_END(WarnMissedTransformationsLegacy, "transform-warning",
                    "Warn about non-applied transformations", false, false)

Pass *llvm::createWarnMissedTransformationsPass() {
  return new WarnMissedTransformationsLegacy();
}
