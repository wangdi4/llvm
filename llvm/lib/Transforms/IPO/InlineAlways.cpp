//===- InlineAlways.cpp - Code to inline always_inline functions ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements a custom inliner that handles only functions that
// are marked as "always inline".
//
//===----------------------------------------------------------------------===//

#ifdef INTEL_CUSTOMIZATION
#include "llvm/Transforms/IPO/InlinerPass.h"
#endif // INTEL_CUSTOMIZATION
#include "llvm/Transforms/IPO.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/InlineCost.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#ifdef INTEL_CUSTOMIZATION
#include "llvm/Support/Debug.h"
#endif // INTEL_CUSTOMIZATION

using namespace llvm;
#ifdef INTEL_CUSTOMIZATION
using namespace InlineReportTypes;
#endif // INTEL_CUSTOMIZATION

#define DEBUG_TYPE "inline"

namespace {

/// \brief Inliner pass which only handles "always inline" functions.
class AlwaysInliner : public Inliner {
  InlineCostAnalysis *ICA;

#ifdef INTEL_SPECIFIC_IL0_BACKEND
  // This is used to enable/disable standard inliner pass for
  // AlwaysInline attribute and perform it only for inline functions
  // specifically marked with "INTEL_ALWAYS_INLINE".
  bool Il0BackendMode;
#endif // INTEL_SPECIFIC_IL0_BACKEND

public:
  // Use extremely low threshold.
  AlwaysInliner() : Inliner(ID, -2000000000, /*InsertLifetime*/ true),
                    ICA(nullptr) {
    initializeAlwaysInlinerPass(*PassRegistry::getPassRegistry());
#ifdef INTEL_SPECIFIC_IL0_BACKEND
    Il0BackendMode = false;
#endif // INTEL_SPECIFIC_IL0_BACKEND
  }

  AlwaysInliner(bool InsertLifetime)
      : Inliner(ID, -2000000000, InsertLifetime), ICA(nullptr) {
    initializeAlwaysInlinerPass(*PassRegistry::getPassRegistry());
#ifdef INTEL_SPECIFIC_IL0_BACKEND
    Il0BackendMode = false;
#endif // INTEL_SPECIFIC_IL0_BACKEND
  }

#ifdef INTEL_SPECIFIC_IL0_BACKEND
  AlwaysInliner(bool InsertLifetime, bool Il0BackendMode)
      : Inliner(ID, -2000000000, InsertLifetime), ICA(nullptr) {
    initializeAlwaysInlinerPass(*PassRegistry::getPassRegistry());
    this->Il0BackendMode = Il0BackendMode;
  }
#endif // INTEL_SPECIFIC_IL0_BACKEND

  static char ID; // Pass identification, replacement for typeid

  InlineCost getInlineCost(CallSite CS) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnSCC(CallGraphSCC &SCC) override;

  using llvm::Pass::doFinalization;
  bool doFinalization(CallGraph &CG) override {
#ifdef INTEL_CUSTOMIZATION
    bool ReturnValue = removeDeadFunctions(CG, /*AlwaysInlineOnly=*/ true);
    getReport().print();
    removeDeletableFunctions();
    return ReturnValue;
#else
    return removeDeadFunctions(CG, /*AlwaysInlineOnly=*/ true);
#endif // INTEL_CUSTOMIZATION
  }
};

}

char AlwaysInliner::ID = 0;
INITIALIZE_PASS_BEGIN(AlwaysInliner, "always-inline",
                "Inliner for always_inline functions", false, false)
INITIALIZE_AG_DEPENDENCY(AliasAnalysis)
INITIALIZE_PASS_DEPENDENCY(AssumptionCacheTracker)
INITIALIZE_PASS_DEPENDENCY(CallGraphWrapperPass)
INITIALIZE_PASS_DEPENDENCY(InlineCostAnalysis)
INITIALIZE_PASS_END(AlwaysInliner, "always-inline",
                "Inliner for always_inline functions", false, false)

Pass *llvm::createAlwaysInlinerPass() { return new AlwaysInliner(); }

Pass *llvm::createAlwaysInlinerPass(bool InsertLifetime) {
  return new AlwaysInliner(InsertLifetime);
}

#ifdef INTEL_SPECIFIC_IL0_BACKEND
Pass *llvm::createAlwaysInlinerPass(bool InsertLifetime, bool Il0BackendMode) {
  return new AlwaysInliner(InsertLifetime, Il0BackendMode);
}
#endif // INTEL_SPECIFIC_IL0_BACKEND

/// \brief Get the inline cost for the always-inliner.
///
/// The always inliner *only* handles functions which are marked with the
/// attribute to force inlining. As such, it is dramatically simpler and avoids
/// using the powerful (but expensive) inline cost analysis. Instead it uses
/// a very simple and boring direct walk of the instructions looking for
/// impossible-to-inline constructs.
///
/// Note, it would be possible to go to some lengths to cache the information
/// computed here, but as we only expect to do this for relatively few and
/// small functions which have the explicit attribute to force inlining, it is
/// likely not worth it in practice.
InlineCost AlwaysInliner::getInlineCost(CallSite CS) {
  Function *Callee = CS.getCalledFunction();

#ifdef INTEL_SPECIFIC_IL0_BACKEND
  // Only specially marked functions are inlined here.
  // The rest always_inline functions are processed by the IL0 backend.
  // This is necessary due to current CilkPlus implementation, where front-end
  // emits some code outlined, but it has to be inlined to have valid
  // debug info in IL0 and also IL0 backend does not inline back functions
  // with call to Cilk's setjmp.
  if (Il0BackendMode) {
    if (Callee && !Callee->isDeclaration() &&
        Callee->hasFnAttribute("INTEL_ALWAYS_INLINE") &&
        ICA->isInlineViable(*Callee))
      return InlineCost::getAlways();
    return InlineCost::getNever();
  }
#endif // INTEL_SPECIFIC_IL0_BACKEND

  // Only inline direct calls to functions with always-inline attributes
  // that are viable for inlining. FIXME: We shouldn't even get here for
  // declarations.
#ifdef INTEL_CUSTOMIZATION
  InlineReason Reason; 
  if (Callee && !Callee->isDeclaration() &&
      CS.hasFnAttr(Attribute::AlwaysInline)) { 
      if (ICA->isInlineViable(*Callee, Reason)) {
        return InlineCost::getAlways(InlrAlwaysInline);
      }
      return InlineCost::getNever(Reason); 
  } 
  return InlineCost::getNever(NinlrNotAlwaysInline);
#else 
  if (Callee && !Callee->isDeclaration() &&
      CS.hasFnAttr(Attribute::AlwaysInline) &&
      ICA->isInlineViable(*Callee))
    return InlineCost::getAlways();

  return InlineCost::getNever();
#endif // INTEL_CUSTOMIZATION
}

bool AlwaysInliner::runOnSCC(CallGraphSCC &SCC) {
  ICA = &getAnalysis<InlineCostAnalysis>();
  return Inliner::runOnSCC(SCC);
}

void AlwaysInliner::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<InlineCostAnalysis>();
  Inliner::getAnalysisUsage(AU);
}
