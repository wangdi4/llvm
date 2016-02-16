//===- InlineCost.h - Cost analysis for inliner -----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements heuristics for inlining decisions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INLINECOST_H
#define LLVM_ANALYSIS_INLINECOST_H

#include "llvm/Analysis/CallGraphSCCPass.h" // INTEL 
#include <cassert>
#include <climits>

namespace llvm {
class AssumptionCacheTracker;
class CallSite;
class DataLayout;
class Function;
class TargetTransformInfo;


namespace InlineConstants {
  // Various magic constants used to adjust heuristics.
  const int InstrCost = 5;
  const int IndirectCallThreshold = 100;
  const int CallPenalty = 25;
  const int LastCallToStaticBonus = -15000;
  const int ColdccPenalty = 2000;
  const int NoreturnPenalty = 10000;
  /// Do not inline functions which allocate this many bytes on the stack
  /// when the caller is recursive.
  const unsigned TotalAllocaSizeRecursiveCaller = 1024;
}

#ifdef INTEL_CUSTOMIZATION
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
   InlrSingleLocalCall,
   InlrSingleBasicBlock,
   InlrAlmostSingleBasicBlock,
   InlrEmptyFunction,
   InlrVectorBonus,
   InlrProfitable,
   InlrLast, // Just a marker placed after the last inlining reason
   NinlrFirst, // Just a marker placed before the first non-inlining reason
   NinlrNoReason,
   NinlrColdCC,
   NinlrDeleted,
   NinlrDuplicateCall,
   NinlrDynamicAlloca,
   NinlrExtern,
   NinlrIndirect,
   NinlrIndirectBranch,
   NinlrBlockAddress,
   NinlrCallsLocalEscape,
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
   NinlrLast // Just a marker placed after the last non-inlining reason
} InlineReason;

}

extern bool IsInlinedReason(InlineReportTypes::InlineReason Reason);
extern bool IsNotInlinedReason(InlineReportTypes::InlineReason Reason);
#endif // INTEL_CUSTOMIZATION

/// \brief Represents the cost of inlining a function.
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
  enum SentinelValues {
    AlwaysInlineCost = INT_MIN,
    NeverInlineCost = INT_MAX
  };

  /// \brief The estimated cost of inlining this callsite.
  const int Cost;

  /// \brief The adjusted threshold against which this cost was computed.
  const int Threshold;

  InlineReportTypes::InlineReason Reason; // INTEL

  // Trivial constructor, interesting logic in the factory functions below.

  InlineCost(int Cost, int Threshold, InlineReportTypes::InlineReason Reason 
    = InlineReportTypes::NinlrNoReason) : Cost(Cost), Threshold(Threshold), 
    Reason(Reason) {} // INTEL 

public:
  static InlineCost get(int Cost, int Threshold) {
    assert(Cost > AlwaysInlineCost && "Cost crosses sentinel value");
    assert(Cost < NeverInlineCost && "Cost crosses sentinel value");
    return InlineCost(Cost, Threshold);
  }
#ifdef INTEL_CUSTOMIZATION
  static InlineCost get(int Cost, int Threshold, 
    InlineReportTypes::InlineReason Reason) {
    assert(Cost > AlwaysInlineCost && "Cost crosses sentinel value");
    assert(Cost < NeverInlineCost && "Cost crosses sentinel value");
    return InlineCost(Cost, Threshold, Reason);
  }
#endif // INTEL_CUSTOMIZATION
  static InlineCost getAlways() {
    return InlineCost(AlwaysInlineCost, 0);
  }
  static InlineCost getNever() {
    return InlineCost(NeverInlineCost, 0);
  }
#ifdef INTEL_CUSTOMIZATION
  static InlineCost getAlways(InlineReportTypes::InlineReason Reason) {
    return InlineCost(AlwaysInlineCost, 0, Reason);
  }
  static InlineCost getNever(InlineReportTypes::InlineReason Reason) {
    return InlineCost(NeverInlineCost, 0, Reason);
  }
#endif // INTEL_CUSTOMIZATION

  /// \brief Test whether the inline cost is low enough for inlining.
  explicit operator bool() const {
    return Cost < Threshold;
  }

  bool isAlways() const { return Cost == AlwaysInlineCost; }
  bool isNever() const { return Cost == NeverInlineCost; }
  bool isVariable() const { return !isAlways() && !isNever(); }

  /// \brief Get the inline cost estimate.
  /// It is an error to call this on an "always" or "never" InlineCost.
  int getCost() const {
    assert(isVariable() && "Invalid access of InlineCost");
    return Cost;
  }

  /// \brief Get the cost delta from the threshold for inlining.
  /// Only valid if the cost is of the variable kind. Returns a negative
  /// value if the cost is too high to inline.
  int getCostDelta() const { return Threshold - getCost(); }

#ifdef INTEL_CUSTOMIZATION 
  InlineReportTypes::InlineReason getInlineReason() const 
    { return Reason; }
  void setInlineReason(InlineReportTypes::InlineReason MyReason) 
    { Reason = MyReason; }
#endif // INTEL_CUSTOMIZATION

};

/// \brief Get an InlineCost object representing the cost of inlining this
/// callsite.
///
/// Note that threshold is passed into this function. Only costs below the
/// threshold are computed with any accuracy. The threshold can be used to
/// bound the computation necessary to determine whether the cost is
/// sufficiently low to warrant inlining.
///
/// Also note that calling this function *dynamically* computes the cost of
/// inlining the callsite. It is an expensive, heavyweight call.
InlineCost getInlineCost(CallSite CS, int Threshold,
                         TargetTransformInfo &CalleeTTI,
                         AssumptionCacheTracker *ACT);

/// \brief Get an InlineCost with the callee explicitly specified.
/// This allows you to calculate the cost of inlining a function via a
/// pointer. This behaves exactly as the version with no explicit callee
/// parameter in all other respects.
//
InlineCost getInlineCost(CallSite CS, Function *Callee, int Threshold,
                         TargetTransformInfo &CalleeTTI,
                         AssumptionCacheTracker *ACT);

/// \brief Minimal filter to detect invalid constructs for inlining.
bool isInlineViable(Function &Callee,                         // INTEL
                    InlineReportTypes::InlineReason& Reason); // INTEL
}
#endif
