//===- AlwaysInliner.cpp - Code to inline always_inline functions ----------===//
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
//===- InlineAlways.cpp - Code to inline always_inline functions ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements a custom inliner that handles only functions that
// are marked as "always inline".
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/InlineCost.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/ProfileSummaryInfo.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/IPO/Inliner.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

#if INTEL_CUSTOMIZATION
#include "llvm/Support/CommandLine.h"
#endif // INTEL_CUSTOMIZATION

using namespace llvm;
using namespace InlineReportTypes; // INTEL

#define DEBUG_TYPE "inline"

#if INTEL_CUSTOMIZATION
extern cl::opt<unsigned> IntelInlineReportLevel;

AlwaysInlinerPass::AlwaysInlinerPass(bool InsertLifetime)
    : InsertLifetime(InsertLifetime) {
  Report = getInlineReport();
  MDReport = getMDInlineReport();
}

AlwaysInlinerPass::~AlwaysInlinerPass() { getReport()->testAndPrint(this); }

#endif // INTEL_CUSTOMIZATION

bool AlwaysInlineImpl(
    Module &M, bool InsertLifetime, ProfileSummaryInfo &PSI,
    function_ref<AssumptionCache &(Function &)> GetAssumptionCache,
    function_ref<AAResults &(Function &)> GetAAR,
#if INTEL_CUSTOMIZATION
    function_ref<BlockFrequencyInfo &(Function &)> GetBFI, InlineReport *IR,
    InlineReportBuilder *MDIR) {
#endif // INTEL_CUSTOMIZATION
  SmallSetVector<CallBase *, 16> Calls;
  bool Changed = false;
  SmallVector<Function *, 16> InlinedFunctions;
  for (Function &F : M) {
    // When callee coroutine function is inlined into caller coroutine function
    // before coro-split pass,
    // coro-early pass can not handle this quiet well.
    // So we won't inline the coroutine function if it have not been unsplited
    if (F.isPresplitCoroutine())
      continue;

    if (!F.isDeclaration() && isInlineViable(F).isSuccess()) {
      Calls.clear();

      for (User *U : F.users())
        if (auto *CB = dyn_cast<CallBase>(U))
          if (CB->getCalledFunction() == &F &&
                CB->hasFnAttr(Attribute::AlwaysInline) &&
                !CB->getAttributes().hasFnAttr(Attribute::NoInline))
              Calls.insert(CB);

      for (CallBase *CB : Calls) {
        Function *Caller = CB->getCaller();
        OptimizationRemarkEmitter ORE(Caller);
        DebugLoc DLoc = CB->getDebugLoc();
        BasicBlock *Block = CB->getParent();

#if INTEL_CUSTOMIZATION
        getInlineReport()->beginUpdate(CB);
        getMDInlineReport()->beginUpdate(CB);
        // if only the callee has the 'alwaysinline' attribute, then set inline
        // reason to "callee always inline", otherwise "callsite always inline"
        // is used.
        if (!CB->hasFnAttrOnCallsite(Attribute::AlwaysInline)) {
          llvm::setMDReasonIsInlined(CB, InlrAlwaysInline);
          getInlineReport()->setReasonIsInlined(CB, InlrAlwaysInline);
        } else {
          llvm::setMDReasonIsInlined(CB, InlrCSAlwaysInline);
          getInlineReport()->setReasonIsInlined(CB, InlrCSAlwaysInline);
        }
        InlineFunctionInfo IFI(GetAssumptionCache, &PSI,
#endif // INTEL_CUSTOMIZATION
                               GetBFI ? &GetBFI(*Caller) : nullptr,
                               GetBFI ? &GetBFI(F) : nullptr);

#if INTEL_CUSTOMIZATION
        InlineResult Res = InlineFunction(
            *CB, IFI, getInlineReport(), getMDInlineReport(),
            /*MergeAttributes=*/true, &GetAAR(F), InsertLifetime);
#endif // INTEL_CUSTOMIZATION
        if (!Res.isSuccess()) {
#if INTEL_CUSTOMIZATION
          InlineReason Reason = Res.getIntelInlReason();
          getInlineReport()->setReasonNotInlined(CB, Reason);
          getInlineReport()->endUpdate();
          llvm::setMDReasonNotInlined(CB, Reason);
          getMDInlineReport()->endUpdate();
#endif // INTEL_CUSTOMIZATION
          ORE.emit([&]() {
            return OptimizationRemarkMissed(DEBUG_TYPE, "NotInlined", // INTEL
                                            CB->getDebugLoc(), CB->getParent()) // INTEL
                   << "'" << ore::NV("Callee", &F) << "' is not inlined into '"
                   << ore::NV("Caller", Caller)
                   << "': " << ore::NV("Reason", Res.getFailureReason());
          });
          continue;
        }

#if INTEL_CUSTOMIZATION
        getInlineReport()->inlineCallSite();
        getInlineReport()->endUpdate();
        getMDInlineReport()->inlineCallSite();
        getMDInlineReport()->endUpdate();
#endif // INTEL_CUSTOMIZATION

        emitInlinedIntoBasedOnCost(
            ORE, DLoc, Block, F, *Caller,
            InlineCost::getAlways("always inline attribute"),
            /*ForProfileContext=*/false, DEBUG_TYPE);

        Changed = true;
      }

      if (F.hasFnAttribute(Attribute::AlwaysInline)) {
        // Remember to try and delete this function afterward. This both avoids
        // re-walking the rest of the module and avoids dealing with any
        // iterator invalidation issues while deleting functions.
        InlinedFunctions.push_back(&F);
      }
    }
  }

  // Remove any live functions.
  erase_if(InlinedFunctions, [&](Function *F) {
    F->removeDeadConstantUsers();
    return !F->isDefTriviallyDead();
  });

  // Delete the non-comdat ones from the module and also from our vector.
  auto NonComdatBegin = partition(
      InlinedFunctions, [&](Function *F) { return F->hasComdat(); });
  for (Function *F : make_range(NonComdatBegin, InlinedFunctions.end())) {
    M.getFunctionList().erase(F);
    Changed = true;
  }
  InlinedFunctions.erase(NonComdatBegin, InlinedFunctions.end());

  if (!InlinedFunctions.empty()) {
    // Now we just have the comdat functions. Filter out the ones whose comdats
    // are not actually dead.
    filterDeadComdatFunctions(InlinedFunctions);
    // The remaining functions are actually dead.
    for (Function *F : InlinedFunctions) {
      M.getFunctionList().erase(F);
      Changed = true;
    }
  }
  return Changed;
}

struct AlwaysInlinerLegacyPass : public ModulePass {
  bool InsertLifetime;

  AlwaysInlinerLegacyPass()
      : AlwaysInlinerLegacyPass(/*InsertLifetime*/ true) {}

  AlwaysInlinerLegacyPass(bool InsertLifetime)
      : ModulePass(ID), InsertLifetime(InsertLifetime) {
    initializeAlwaysInlinerLegacyPassPass(*PassRegistry::getPassRegistry());
  }

#if INTEL_CUSTOMIZATION
  ~AlwaysInlinerLegacyPass() {
    getInlineReport()->testAndPrint(this);
  }
#endif // INTEL_CUSTOMIZATION

  bool runOnModule(Module &M) override {

    auto &PSI = getAnalysis<ProfileSummaryInfoWrapperPass>().getPSI();
    auto GetAAR = [&](Function &F) -> AAResults & {
      return getAnalysis<AAResultsWrapperPass>(F).getAAResults();
    };
    auto GetAssumptionCache = [&](Function &F) -> AssumptionCache & {
      return getAnalysis<AssumptionCacheTracker>().getAssumptionCache(F);
    };

    return AlwaysInlineImpl(M, InsertLifetime, PSI, GetAssumptionCache, GetAAR,
#if INTEL_CUSTOMIZATION
                            /*GetBFI*/ nullptr, getInlineReport(),
                            getMDInlineReport());
#endif // INTEL_CUSTOMIZATION
  }

  static char ID; // Pass identification, replacement for typeid

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<AssumptionCacheTracker>();
    AU.addRequired<AAResultsWrapperPass>();
    AU.addRequired<ProfileSummaryInfoWrapperPass>();
  }
};

char AlwaysInlinerLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(AlwaysInlinerLegacyPass, "always-inline",
                      "Inliner for always_inline functions", false, false)
INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AssumptionCacheTracker)
INITIALIZE_PASS_DEPENDENCY(ProfileSummaryInfoWrapperPass)
INITIALIZE_PASS_END(AlwaysInlinerLegacyPass, "always-inline",
                    "Inliner for always_inline functions", false, false)

Pass *llvm::createAlwaysInlinerLegacyPass(bool InsertLifetime) {
  return new AlwaysInlinerLegacyPass(InsertLifetime);
}

#if INTEL_CUSTOMIZATION
/// Main run interface method.  We override here to avoid calling skipSCC().
class UnskippableAlwaysInlinerLegacyPass : public AlwaysInlinerLegacyPass {
public:
  UnskippableAlwaysInlinerLegacyPass(bool InsertLietime)
      : AlwaysInlinerLegacyPass(InsertLietime) {}
};

Pass *llvm::createUnskippableAlwaysInlinerLegacyPass(bool InsertLifetime) {
  return new UnskippableAlwaysInlinerLegacyPass(InsertLifetime);
}
#endif // INTEL_CUSTOMIZATION

PreservedAnalyses AlwaysInlinerPass::run(Module &M,
                                         ModuleAnalysisManager &MAM) {
  FunctionAnalysisManager &FAM =
      MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetAssumptionCache = [&](Function &F) -> AssumptionCache & {
    return FAM.getResult<AssumptionAnalysis>(F);
  };
  auto GetBFI = [&](Function &F) -> BlockFrequencyInfo & {
    return FAM.getResult<BlockFrequencyAnalysis>(F);
  };
  auto GetAAR = [&](Function &F) -> AAResults & {
    return FAM.getResult<AAManager>(F);
  };
  auto &PSI = MAM.getResult<ProfileSummaryAnalysis>(M);
  getInlineReport()->beginModule(this); // INTEL
  bool Changed = AlwaysInlineImpl(M, InsertLifetime, PSI, GetAssumptionCache,
#if INTEL_CUSTOMIZATION
                                  GetAAR, GetBFI, getReport(), getMDReport());
#endif // INTEL_CUSTOMIZATION

  getInlineReport()->endModule(); // INTEL
  return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
