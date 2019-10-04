//===- InlineSimple.cpp - Code to perform simple function inlining --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements bottom-up inlining of functions into callees.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/InlineCost.h"
#include "llvm/Analysis/ProfileSummaryInfo.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/Inliner.h"
#include "llvm/Transforms/IPO/Intel_InlineReport.h"          // INTEL

using namespace llvm;

#if INTEL_CUSTOMIZATION
using namespace InlineReportTypes;

extern cl::opt<unsigned> IntelInlineReportLevel;
#endif // INTEL_CUSTOMIZATION

#define DEBUG_TYPE "inline"

namespace {

/// Actual inliner pass implementation.
///
/// The common implementation of the inlining logic is shared between this
/// inliner pass and the always inliner pass. The two passes use different cost
/// analyses to determine when to inline.
class SimpleInliner : public LegacyInlinerBase {

  InlineParams Params;

public:
  SimpleInliner() : LegacyInlinerBase(ID), Params(llvm::getInlineParams()) {
    initializeSimpleInlinerPass(*PassRegistry::getPassRegistry());
  }

  explicit SimpleInliner(InlineParams Params)
      : LegacyInlinerBase(ID), Params(std::move(Params)) {
    initializeSimpleInlinerPass(*PassRegistry::getPassRegistry());
  }

  static char ID; // Pass identification, replacement for typeid

  InlineCost getInlineCost(CallSite CS) override {
    Function *Callee = CS.getCalledFunction();
    TargetTransformInfo &TTI = TTIWP->getTTI(*Callee);
    TargetLibraryInfo &TLI = TLIWP->getTLI(*Callee); // INTEL

    bool RemarksEnabled = false;
    const auto &BBs = CS.getCaller()->getBasicBlockList();
    if (!BBs.empty()) {
      auto DI = OptimizationRemark(DEBUG_TYPE, "", DebugLoc(), &BBs.front());
      if (DI.isEnabled())
        RemarksEnabled = true;
    }
    OptimizationRemarkEmitter ORE(CS.getCaller());

    std::function<AssumptionCache &(Function &)> GetAssumptionCache =
        [&](Function &F) -> AssumptionCache & {
      return ACT->getAssumptionCache(F);
    };

#if INTEL_CUSTOMIZATION
    auto *Agg = getAnalysisIfAvailable<InlineAggressiveWrapperPass>();
    InlineAggressiveInfo *AggI = Agg ? &Agg->getResult() : nullptr;
    if (IntelInlineReportLevel & InlineReportOptions::RealCost)
      Params.ComputeFullInlineCost = true;
#endif // INTEL_CUSTOMIZATION

    return llvm::getInlineCost(
        cast<CallBase>(*CS.getInstruction()), Params, TTI, GetAssumptionCache,
        /*GetBFI=*/None, &TLI, ILIC, AggI, &CallSitesForFusion,     // INTEL
        &FuncsForDTrans, PSI, RemarksEnabled ? &ORE : nullptr); // INTEL
  }

  bool runOnSCC(CallGraphSCC &SCC) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;

private:
  TargetTransformInfoWrapperPass *TTIWP;
  TargetLibraryInfoWrapperPass *TLIWP;    // INTEL

#if INTEL_CUSTOMIZATION
  const InlineParams *getInlineParams() const override { return &Params; }
#endif // INTEL_CUSTOMIZATION
};

} // end anonymous namespace

char SimpleInliner::ID = 0;
INITIALIZE_PASS_BEGIN(SimpleInliner, "inline", "Function Integration/Inlining",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(AssumptionCacheTracker)
INITIALIZE_PASS_DEPENDENCY(CallGraphWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ProfileSummaryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(InlineAggressiveWrapperPass)        // INTEL
INITIALIZE_PASS_END(SimpleInliner, "inline", "Function Integration/Inlining",
                    false, false)

Pass *llvm::createFunctionInliningPass() { return new SimpleInliner(); }

Pass *llvm::createFunctionInliningPass(int Threshold) {
  return new SimpleInliner(llvm::getInlineParams(Threshold));
}

#if INTEL_CUSTOMIZATION
Pass *llvm::createFunctionInliningPass(unsigned OptLevel,
                                       unsigned SizeOptLevel,
                                       bool DisableInlineHotCallSite,
                                       bool PrepareForLTO)
{
  auto Param = llvm::getInlineParams(OptLevel, SizeOptLevel, PrepareForLTO);
  if (DisableInlineHotCallSite)
    Param.HotCallSiteThreshold = 0;
  return new SimpleInliner(Param);
}
#endif // INTEL_CUSTOMIZATION

Pass *llvm::createFunctionInliningPass(InlineParams &Params) {
  return new SimpleInliner(Params);
}

bool SimpleInliner::runOnSCC(CallGraphSCC &SCC) {
  TTIWP = &getAnalysis<TargetTransformInfoWrapperPass>();
  TLIWP = &getAnalysis<TargetLibraryInfoWrapperPass>(); // INTEL
  return LegacyInlinerBase::runOnSCC(SCC);
}

void SimpleInliner::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<TargetTransformInfoWrapperPass>();
  AU.addUsedIfAvailable<InlineAggressiveWrapperPass>();        // INTEL
  LegacyInlinerBase::getAnalysisUsage(AU);
}
