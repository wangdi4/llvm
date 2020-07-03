//===- InlineCost.h - Cost analysis for inliner -----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements heuristics for inlining decisions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INLINECOST_H
#define LLVM_ANALYSIS_INLINECOST_H

#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/CallGraphSCCPass.h"
#include "llvm/Analysis/LoopInfo.h"           // INTEL
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/ADT/SmallSet.h"                // INTEL
#include <cassert>
#include <climits>

namespace llvm {
class AssumptionCacheTracker;
class BlockFrequencyInfo;
class CallBase;
class DataLayout;
class Function;
class ProfileSummaryInfo;
class TargetTransformInfo;
class TargetLibraryInfo;

namespace InlineConstants {
// Various thresholds used by inline cost analysis.
/// Use when optsize (-Os) is specified.
const int OptSizeThreshold = 50;

/// Use when minsize (-Oz) is specified.
const int OptMinSizeThreshold = 5;

/// Use when -O3 is specified.
const int OptAggressiveThreshold = 250;

// Various magic constants used to adjust heuristics.
const int InstrCost = 5;
const int IndirectCallThreshold = 100;
const int CallPenalty = 25;
const int LastCallToStaticBonus = 15000;
const int SecondToLastCallToStaticBonus = 410; // INTEL
const int AggressiveInlineCallBonus = 5000;    // INTEL
const int BigBasicBlockPredCount = 90;         // INTEL
const int InliningHeuristicBonus = 1000;       // INTEL
const int DeepInliningHeuristicBonus = 10000;  // INTEL
const int VeryDeepInliningHeuristicBonus = 2000000000;  // INTEL
const int ColdccPenalty = 2000;
/// Do not inline functions which allocate this many bytes on the stack
/// when the caller is recursive.
const unsigned TotalAllocaSizeRecursiveCaller = 1024;
const unsigned BasicBlockSuccRatio = 210; // INTEL
/// Do not inline dynamic allocas that have been constant propagated to be
/// static allocas above this amount in bytes.
const uint64_t MaxSimplifiedDynamicAllocaToInline = 65536;
} // namespace InlineConstants

#if INTEL_CUSTOMIZATION

/// \brief A cache to save loop related info during inlining of an SCC.
///
/// This cache is used to store the DominatorTree and LoopInfo for called
/// functions which are candidates for inlining.  Unnecessarily recomputing
/// them can add significantly to compile time and memory consumption.
///
/// The DominatorTree and LoopInfo for a function is invalidated when
/// some other function is inlined into it.
///
typedef std::map<Function*, DominatorTree*> DTMap;
typedef std::map<Function*, LoopInfo*> LIMap;

class InliningLoopInfoCache {

  DTMap DTMapSCC;
  LIMap LIMapSCC;

public:

  InliningLoopInfoCache() {}

  ~InliningLoopInfoCache();

  DominatorTree* getDT(Function *F);
  LoopInfo* getLI(Function* F);
  void invalidateFunction(Function *F);

};

namespace InlineReportTypes {

/// \brief Inlining and non-inlining reasons
///
/// Each element of this enum is a reason why a function was or was not
/// inlined.  These are used in the inlining report. Those with names
/// beginning with Inlr are reasons FOR inlining.  Those with names
/// beginning with Ninlr are reasons for NOT inlining.
///
/// NOTE: The order of the values below is significant.  Those with a lower
/// enum value are considered more significant and will be given preference
/// when the inlining report is printed. (See the function bestInlineReason()
/// in InlineCost.cpp.
///
typedef enum {
   InlrFirst, // Just a marker placed before the first inlining reason
   InlrNoReason,
   InlrAlwaysInline,
   InlrAlwaysInlineRecursive,
   InlrInlineList,
   InlrHotProfile,
   InlrRecProClone,
   InlrHasExtractedRecursiveCall,
   InlrSingleLocalCall,
   InlrSingleBasicBlock,
   InlrSingleBasicBlockWithTest,
   InlrSingleBasicBlockWithStructTest,
   InlrEmptyFunction,
   InlrDoubleLocalCall,
   InlrDoubleNonLocalCall,
   InlrVectorBonus,
   InlrAggInline,
   InlrForFusion,
   InlrDeeplyNestedIfs,
   InlrAddressComputations,
   InlrStackComputations,
   InlrPreferPartialInline,
   InlrPassedDummyArgs,
   InlrArrayStructArgs,
   InlrPreferTileChoice,
   InlrManyRecursiveCallsSplitting,
   InlrProfitable,
   InlrLast, // Just a marker placed after the last inlining reason
   NinlrFirst, // Just a marker placed before the first non-inlining reason
   NinlrNoReason,
   NinlrNoinlineList,
   NinlrColdCC,
   NinlrColdProfile,
   NinlrDeleted,
   NinlrDuplicateCall,
   NinlrDynamicAlloca,
   NinlrExtern,
   NinlrIndirect,
   NinlrIndirectBranch,
   NinlrBlockAddress,
   NinlrCallsLocalEscape,
   NinlrCallsBranchFunnel,
   NinlrNeverInline,
   NinlrIntrinsic,
   NinlrOuterInlining,
   NinlrRecursive,
   NinlrReturnsTwice,
   NinlrTooMuchStack,
   NinlrVarargs,
   NinlrMismatchedAttributes,
   NinlrMismatchedGC,
   NinlrMismatchedPersonality,
   NinlrNoinlineAttribute,
   NinlrNoinlineCallsite,
   NinlrNoReturn,
   NinlrOptNone,
   NinlrMayBeOverriden,
   NinlrNotPossible,
   NinlrNotAlwaysInline,
   NinlrNewlyCreated,
   NinlrNotProfitable,
   NinlrOpBundles,
   NinlrMSVCEH,
   NinlrSEH,
   NinlrPreferCloning,
   NinlrNullPtrMismatch,
   NinlrPreferMultiversioning,
   NinlrPreferSOAToAOS,
   NinlrStackComputations,
   NinlrSwitchComputations,
   NinlrDelayInlineDecision,
   NinlrPreferPartialInline,
   NinlrCalleeHasExceptionHandling,
   NinlrLast // Just a marker placed after the last non-inlining reason
} InlineReason;

}

extern bool IsInlinedReason(InlineReportTypes::InlineReason Reason);
extern bool IsNotInlinedReason(InlineReportTypes::InlineReason Reason);
#endif // INTEL_CUSTOMIZATION
/// Represents the cost of inlining a function.
///
/// This supports special values for functions which should "always" or
/// "never" be inlined. Otherwise, the cost represents a unitless amount;
/// smaller values increase the likelihood of the function being inlined.
///
/// Objects of this type also provide the adjusted threshold for inlining
/// based on the information available for a particular callsite. They can be
/// directly tested to determine if inlining should occur given the cost and
/// threshold for this cost metric.
/// INTEL The Intel version is augmented with the InlineReason, which is the
/// INTEL principal reason that a call site was or was not inlined.

class InlineCost {
  enum SentinelValues { AlwaysInlineCost = INT_MIN, NeverInlineCost = INT_MAX };

  /// The estimated cost of inlining this callsite.
  int Cost = 0;

  /// The adjusted threshold against which this cost was computed.
  int Threshold = 0;

  /// Must be set for Always and Never instances.
  const char *Reason = nullptr;

  InlineReportTypes::InlineReason IntelReason; // INTEL

#if INTEL_CUSTOMIZATION
  /// \brief The cost and the threshold used for early exit from usual inlining
  /// process. A value of INT_MAX for either of these indicates that no value
  /// has been seen yet. They are expected to be set at the same time, so we
  /// need test only EarlyExitCost to see if the value of either is set yet.
  const int EarlyExitCost;
  const int EarlyExitThreshold;
#endif // INTEL_CUSTOMIZATION

  // Trivial constructor, interesting logic in the factory functions below.

#if INTEL_CUSTOMIZATION
  InlineCost(int Cost, int Threshold, const char* Reason = nullptr,
    InlineReportTypes::InlineReason IntelReason
    = InlineReportTypes::NinlrNoReason, int EarlyExitCost = INT_MAX,
    int EarlyExitThreshold = INT_MAX) :
    Cost(Cost), Threshold(Threshold), Reason(Reason), IntelReason(IntelReason),
    EarlyExitCost(EarlyExitCost), EarlyExitThreshold(EarlyExitThreshold) {
    assert((isVariable() || Reason) &&
            "Reason must be provided for Never or Always");
  }
#endif // INTEL_CUSTOMIZATION

public:
  static InlineCost get(int Cost, int Threshold) {
    assert(Cost > AlwaysInlineCost && "Cost crosses sentinel value");
    assert(Cost < NeverInlineCost && "Cost crosses sentinel value");
    return InlineCost(Cost, Threshold);
  }
#if INTEL_CUSTOMIZATION
  static InlineCost get(int Cost, int Threshold, const char* Reason,
    InlineReportTypes::InlineReason IntelReason, int EarlyExitCost,
    int EarlyExitThreshold) {
    assert(Cost > AlwaysInlineCost && "Cost crosses sentinel value");
    assert(Cost < NeverInlineCost && "Cost crosses sentinel value");
    return InlineCost(Cost, Threshold, Reason, IntelReason, EarlyExitCost,
        EarlyExitThreshold);
  }
#endif // INTEL_CUSTOMIZATION
  static InlineCost getAlways(const char *Reason) {
    return InlineCost(AlwaysInlineCost, 0, Reason);
  }
  static InlineCost getNever(const char *Reason) {
    return InlineCost(NeverInlineCost, 0, Reason);
  }
#if INTEL_CUSTOMIZATION
  static InlineCost getAlways(const char* Reason,
                              InlineReportTypes::InlineReason IntelReason) {
    return InlineCost(AlwaysInlineCost, 0, Reason, IntelReason);
  }
  static InlineCost getNever(const char* Reason,
                             InlineReportTypes::InlineReason IntelReason) {
    return InlineCost(NeverInlineCost, 0, Reason, IntelReason);
  }
#endif // INTEL_CUSTOMIZATION

  /// Test whether the inline cost is low enough for inlining.
  explicit operator bool() const { return Cost < Threshold; }

  bool isAlways() const { return Cost == AlwaysInlineCost; }
  bool isNever() const { return Cost == NeverInlineCost; }
  bool isVariable() const { return !isAlways() && !isNever(); }

  /// Get the inline cost estimate.
  /// It is an error to call this on an "always" or "never" InlineCost.
  int getCost() const {
    assert(isVariable() && "Invalid access of InlineCost");
    return Cost;
  }

  /// Get the threshold against which the cost was computed
  int getThreshold() const {
    assert(isVariable() && "Invalid access of InlineCost");
    return Threshold;
  }

  /// Get the reason of Always or Never.
  const char *getReason() const {
    assert((Reason || isVariable()) &&
           "InlineCost reason must be set for Always or Never");
    return Reason;
  }

  /// Get the cost delta from the threshold for inlining.
  /// Only valid if the cost is of the variable kind. Returns a negative
  /// value if the cost is too high to inline.
  int getCostDelta() const { return Threshold - getCost(); }

#if INTEL_CUSTOMIZATION
  InlineReportTypes::InlineReason getInlineReason() const
    { return IntelReason; }
  void setInlineReason(InlineReportTypes::InlineReason MyReason)
    { IntelReason = MyReason; }
  int getEarlyExitCost() const
    { return EarlyExitCost; }
  int getEarlyExitThreshold() const
    { return EarlyExitThreshold; }
#endif // INTEL_CUSTOMIZATION

};

/// InlineResult is basically true or false. For false results the message
/// describes a reason.
class InlineResult {
  const char *Message = nullptr;
  InlineReportTypes::InlineReason IntelInlReason =    // INTEL
      InlineReportTypes::InlineReason::NinlrNoReason; // INTEL
  InlineResult(const char *Message = nullptr) : Message(Message) {}

public:
#if INTEL_CUSTOMIZATION
  static InlineResult success() {
    InlineResult IR;
    return IR.setIntelInlReason(InlineReportTypes::InlineReason::InlrNoReason);
  }
#endif // INTEL_CUSTOMIZATION
  static InlineResult failure(const char *Reason) {
    return InlineResult(Reason);
  }
  bool isSuccess() const { return Message == nullptr; }
  const char *getFailureReason() const {
    assert(!isSuccess() &&
           "getFailureReason should only be called in failure cases");
    return Message;
  }
#if INTEL_CUSTOMIZATION
  InlineReportTypes::InlineReason getIntelInlReason() { return IntelInlReason; }
  InlineResult &setIntelInlReason(InlineReportTypes::InlineReason IR) {
    IntelInlReason = IR;
    return *this;
  }
#endif // INTEL_CUSTOMIZATION
};

/// Thresholds to tune inline cost analysis. The inline cost analysis decides
/// the condition to apply a threshold and applies it. Otherwise,
/// DefaultThreshold is used. If a threshold is Optional, it is applied only
/// when it has a valid value. Typically, users of inline cost analysis
/// obtain an InlineParams object through one of the \c getInlineParams methods
/// and pass it to \c getInlineCost. Some specialized versions of inliner
/// (such as the pre-inliner) might have custom logic to compute \c InlineParams
/// object.

struct InlineParams {
  /// The default threshold to start with for a callee.
  int DefaultThreshold = -1;

  /// Threshold to use for callees with inline hint.
  Optional<int> HintThreshold;

  /// Threshold to use for cold callees.
  Optional<int> ColdThreshold;

  /// Threshold to use when the caller is optimized for size.
  Optional<int> OptSizeThreshold;

  /// Threshold to use when the caller is optimized for minsize.
  Optional<int> OptMinSizeThreshold;

  /// Threshold to use when the callsite is considered hot.
  Optional<int> HotCallSiteThreshold;

#if INTEL_CUSTOMIZATION
  /// This flag indicates that it is LTO compile phase. This flag is
  /// set when PrepareForLTO flag in PassManagerBuilder is true. .
  Optional<bool> PrepareForLTO;
#endif // INTEL_CUSTOMIZATION

  /// Threshold to use when the callsite is considered hot relative to function
  /// entry.
  Optional<int> LocallyHotCallSiteThreshold;

  /// Threshold to use when the callsite is considered cold.
  Optional<int> ColdCallSiteThreshold;

  /// Compute inline cost even when the cost has exceeded the threshold.
  Optional<bool> ComputeFullInlineCost;

  /// Indicate whether we should allow inline deferral.
  Optional<bool> EnableDeferral = true;
};

/// Generate the parameters to tune the inline cost analysis based only on the
/// commandline options.
InlineParams getInlineParams();

/// Generate the parameters to tune the inline cost analysis based on command
/// line options. If -inline-threshold option is not explicitly passed,
/// \p Threshold is used as the default threshold.
InlineParams getInlineParams(int Threshold);

/// Generate the parameters to tune the inline cost analysis based on command
/// line options. If -inline-threshold option is not explicitly passed,
/// the default threshold is computed from \p OptLevel and \p SizeOptLevel.
/// An \p OptLevel value above 3 is considered an aggressive optimization mode.
/// \p SizeOptLevel of 1 corresponds to the -Os flag and 2 corresponds to
/// the -Oz flag.
InlineParams getInlineParams(unsigned OptLevel, unsigned SizeOptLevel);

#if INTEL_CUSTOMIZATION
/// Generate the parameters to tune the inline cost analysis based on command
/// line options. It does exactly same as what "getInlineParams(unsigned
/// OptLevel, unsigned SizeOptLevel)" routine does except it also sets
/// PrepareForLTO flag in InlineParams based on \p PrepareForLTO.
InlineParams getInlineParams(unsigned OptLevel, unsigned SizeOptLevel,
                             bool PrepareForLTO);
#endif // INTEL_CUSTOMIZATION

/// Return the cost associated with a callsite, including parameter passing
/// and the call/return instruction.
int getCallsiteCost(CallBase &Call, const DataLayout &DL);

/// Get an InlineCost object representing the cost of inlining this
/// callsite.
///
/// Note that a default threshold is passed into this function. This threshold
/// could be modified based on callsite's properties and only costs below this
/// new threshold are computed with any accuracy. The new threshold can be
/// used to bound the computation necessary to determine whether the cost is
/// sufficiently low to warrant inlining.
///
/// Also note that calling this function *dynamically* computes the cost of
/// inlining the callsite. It is an expensive, heavyweight call.
InlineCost
getInlineCost(CallBase &Call, const InlineParams &Params,
              TargetTransformInfo &CalleeTTI,
              function_ref<AssumptionCache &(Function &)> GetAssumptionCache,
              function_ref<const TargetLibraryInfo &(Function &)> GetTLI,
              function_ref<BlockFrequencyInfo &(Function &)> GetBFI = nullptr,
              ProfileSummaryInfo *PSI = nullptr,
              OptimizationRemarkEmitter *ORE = nullptr,               // INTEL
              InliningLoopInfoCache *ILIC = nullptr);                 // INTEL

/// Get an InlineCost with the callee explicitly specified.
/// This allows you to calculate the cost of inlining a function via a
/// pointer. This behaves exactly as the version with no explicit callee
/// parameter in all other respects.
//
InlineCost
getInlineCost(CallBase &Call, Function *Callee, const InlineParams &Params,
              TargetTransformInfo &CalleeTTI,
              function_ref<AssumptionCache &(Function &)> GetAssumptionCache,
              function_ref<const TargetLibraryInfo &(Function &)> GetTLI,
              function_ref<BlockFrequencyInfo &(Function &)> GetBFI = nullptr,
              ProfileSummaryInfo *PSI = nullptr,
              OptimizationRemarkEmitter *ORE = nullptr,               // INTEL
              InliningLoopInfoCache *ILIC = nullptr);                 // INTEL

/// Returns InlineResult::success() if the call site should be always inlined
/// because of user directives, and the inlining is viable. Returns
/// InlineResult::failure() if the inlining may never happen because of user
/// directives or incompatibilities detectable without needing callee traversal.
/// Otherwise returns None, meaning that inlining should be decided based on
/// other criteria (e.g. cost modeling).
Optional<InlineResult> getAttributeBasedInliningDecision(
    CallBase &Call, Function *Callee, TargetTransformInfo &CalleeTTI,
    function_ref<const TargetLibraryInfo &(Function &)> GetTLI);

/// Get the cost estimate ignoring thresholds. This is similar to getInlineCost
/// when passed InlineParams::ComputeFullInlineCost, or a non-null ORE. It
/// uses default InlineParams otherwise.
/// Contrary to getInlineCost, which makes a threshold-based final evaluation of
/// should/shouldn't inline, captured in InlineResult, getInliningCostEstimate
/// returns:
/// - None, if the inlining cannot happen (is illegal)
/// - an integer, representing the cost.
Optional<int> getInliningCostEstimate(
    CallBase &Call, TargetTransformInfo &CalleeTTI,
    function_ref<AssumptionCache &(Function &)> GetAssumptionCache,
    function_ref<BlockFrequencyInfo &(Function &)> GetBFI = nullptr,
    ProfileSummaryInfo *PSI = nullptr,
    OptimizationRemarkEmitter *ORE = nullptr,               // INTEL
    TargetLibraryInfo *TLI = nullptr,                       // INTEL
    InliningLoopInfoCache *ILIC = nullptr);                 // INTEL

/// Minimal filter to detect invalid constructs for inlining.
InlineResult isInlineViable(Function &Callee);

// This pass is used to annotate instructions during the inline process for
// debugging and analysis. The main purpose of the pass is to see and test
// inliner's decisions when creating new optimizations to InlineCost.
struct InlineCostAnnotationPrinterPass
    : PassInfoMixin<InlineCostAnnotationPrinterPass> {
  raw_ostream &OS;

public:
  explicit InlineCostAnnotationPrinterPass(raw_ostream &OS) : OS(OS) {}
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};
} // namespace llvm

#endif
