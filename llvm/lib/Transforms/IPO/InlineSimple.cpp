//===- InlineSimple.cpp - Code to perform simple function inlining --------===//
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
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"                        // INTEL
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

#if INTEL_CUSTOMIZATION
  ~SimpleInliner() {
    getReport()->testAndPrint(this);
  }
#endif // INTEL_CUSTOMIZATION
  static char ID; // Pass identification, replacement for typeid

  InlineCost getInlineCost(CallBase &CB) override {
    Function *Callee = CB.getCalledFunction();
    TargetTransformInfo &TTI = TTIWP->getTTI(*Callee);

    bool RemarksEnabled = false;
    const auto &BBs = CB.getCaller()->getBasicBlockList();
    if (!BBs.empty()) {
      auto DI = OptimizationRemark(DEBUG_TYPE, "", DebugLoc(), &BBs.front());
      if (DI.isEnabled())
        RemarksEnabled = true;
    }
    OptimizationRemarkEmitter ORE(CB.getCaller());

    std::function<AssumptionCache &(Function &)> GetAssumptionCache =
        [&](Function &F) -> AssumptionCache & {
      return ACT->getAssumptionCache(F);
    };

#if INTEL_CUSTOMIZATION
    if (IntelInlineReportLevel & InlineReportOptions::RealCost)
      Params.ComputeFullInlineCost = true;
#endif // INTEL_CUSTOMIZATION

    return llvm::getInlineCost(CB, Params, TTI, GetAssumptionCache, GetTLI,
                               /*GetBFI=*/nullptr, PSI,
                               RemarksEnabled ? &ORE : nullptr, // INTEL
                               ILIC, WPI);                      // INTEL
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
                                       bool PrepareForLTO,
                                       bool LinkForLTO)
{
  auto Param = llvm::getInlineParams(OptLevel, SizeOptLevel, PrepareForLTO,
      LinkForLTO);
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
#if INTEL_CUSTOMIZATION
  TLIWP = &getAnalysis<TargetLibraryInfoWrapperPass>();
  getInlineReport()->beginSCC(SCC, this);
  getMDInlineReport()->beginSCC(SCC);
  bool RV = LegacyInlinerBase::runOnSCC(SCC);
  getInlineReport()->endSCC();
  return RV;
#endif // INTEL_CUSTOMIZATION
}

void SimpleInliner::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<TargetTransformInfoWrapperPass>();
  LegacyInlinerBase::getAnalysisUsage(AU);
}
