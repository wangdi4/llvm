//===- Intel_InlineReportCommon.h - Inlining report utils ------*- C++ -*-===//
//
// Copyright (C) 2019-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines various enums and utilities needed to represent an inlining
// report.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_INTEL_INLINEREPORTCOMMON_H
#define LLVM_TRANSFORMS_IPO_INTEL_INLINEREPORTCOMMON_H

#include "llvm/Analysis/InlineCost.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/FormattedStream.h"

namespace llvm {

namespace InlineReportTypes {

typedef enum {
  Basic = 0x1,     // Print basic information like what was inlined
  Reasons = 0x2,   // Add reasons for inlining or not inlining
  SameLine = 0x4,  // Put the reasons and the call site on the same lime
  LineCol = 0x8,   // Print the line and column of the call sites
                   //   if we had appropriate source position information
  File = 0x10,     // Print the file of the call sites
  Linkage = 0x20,  // Print linkage info for routines and call sites:
                   //   L: local (F.hasLocalLinkage())
                   //   O: link once ODR (one definition rule)
                   //     (F.hasLinkOnceODRLinkage())
                   //   X: available externally (and generally not emitted)
                   //     (F.hasAvailableExternallyLinkage())
                   //   A: alternate (something other than L, O, or X)
  EarlyExitCost = 0x40,    // Print the early exit cost vs. threshold info,
                           //   where applicable
  BasedOnMetadata = 0x80,  // Create metadata-based inline report
  CompositeReport = 0x100, // Create composite inline report for an -flto
                           //   compilation
  DontSkipIntrin = 0x200,  // Do create the inlining report info for the
                           //   special intrinsic call sites
  Language = 0x400,     // Print the source language C for C/C++ and F for Fortran
  Options = 0x800,      // Print the inlining option values
  RealCost = 0x1000,    // Print both early exit and real inlining costs
  DeadStatics = 0x2000, // Print dead static functions
  Externs = 0x4000,     // Print external function callsites
  Indirects = 0x8000    // Print indirect function callsites
} InlineReportOptions;
}

// The reasons that a call site is inlined or not inlinined fall into
// several categories.  These are indicated by the InlPtrType for each
// reason.
//
// The simplest category is those reasons which are absolute: we inlined
// or didn't inline the call site exactly because of this.  In this case,
// printing a simple text string suffices to describe why the call site
// was or was not inlined.  These reasons have the InlPrtType InlPtrSimple.
//
// Sometimes, however, the real reason a call site was or was not inlined
// is because the values of the cost and the threshold for that call site.
// In these cases, if the cost <= threshold, the inlining was done, but
// if cost > threshold it was not.  But there are often large bonuses and
// penalities that contribute to the value of the cost and/or threshold.
// In such cases, reporting the principal reason the cost and/or threshold
// was adjusted provides a more meaningful reason than simply citing the
// cost and threshold numbers, and we do that.  These reasons have the
// InlPtrType InlPtrCost.
//
// Finally, there are some reasons that can't be adequately displayed by
// either of the above two techniques.  These have the InlPtrType
// InlPrtSpecial.  The handling of them is done directly within the report
// printing functions themselves.

typedef enum {
  InlPrtNone,   // Used for sentinels and the generic value "InlrNoReason"
                //   No text is expected to be printed for these.
  InlPrtSimple, // Print only the text for the (non-)inlining reason
  InlPrtCost,   // Print the text and cost info for the (non-)inlining reason
  InlPrtSpecial // The function InlineReportCallSite::print needs to have
                //   special cased code to handle it
} InlPrtType;

typedef struct {
  InlPrtType Type;     // Classification of inlining reason
  const char *Message; // Text message for inlining reason (or nullptr)
} InlPrtRecord;

///
/// \brief A table of entries, one for each possible (non-)inlining reason
///
const static InlPrtRecord InlineReasonText[] = {
    // InlrFirst,
    {InlPrtNone, nullptr},
    // InlrNoReason,
    {InlPrtNone, nullptr},
    // InlrAlwaysInlineRecursive,
    {InlPrtSimple, "Callee is always inline (recursive)"},
    // InlrAlwaysInline,
    {InlPrtSimple, "Callee is always inline"},
    // InlrInlineList,
    {InlPrtSimple, "Callsite on inline list"},
    // InlrHotProfile,
    {InlPrtCost, "Callsite has hot profile"},
    // InlrHotCallsite,
    {InlPrtCost, "Callsite is hot"},
    // InlrHotCallee,
    {InlPrtCost, "Callee is hot"},
    // InlrRecProClone
    {InlPrtCost, "Callee is recursive progression clone"},
    // InlrHasExtractedRecursiveCall
    {InlPrtCost, "Callee has extracted recursive call"},
    // InlrSingleLocalCall,
    {InlPrtCost, "Callee has single callsite and local linkage"},
    // InlrSingleBasicBlock,
    {InlPrtCost, "Callee is single basic block"},
    // InlrSingleBasicBlockWithTest,
    {InlPrtCost, "Callee is single basic block with test"},
    // InlrSingleBasicBlockWithStructTest,
    {InlPrtCost, "Callee is single basic block with structure test"},
    // InlrEmptyFunction,
    {InlPrtCost, "Callee is empty"},
    // InlrDoubleLocalCall,
    {InlPrtCost, "Callee has double callsite and local linkage"},
    // InlrDoubleNonLocalCall,
    {InlPrtCost, "Callee has double callsite without local linkage"},
    // InlrVectorBonus,
    {InlPrtCost, "Callee has vector instructions"},
    // InlrAggInline,
    {InlPrtCost, "Aggressive inline to expose uses of global ptrs"},
    // InlrDTransInline,
    {InlPrtCost, "inline to enable DTrans"},
    // InlrForFusion,
    {InlPrtCost,
     "Callee has multiple callsites with loops that could be fused"},
    // InlrDeeplyNestedIfs,
    {InlPrtCost, "Callee was inlined due to deeply nested ifs"},
    // InlrAddressComputations,
    {InlPrtCost, "Inlining for complicated address computations"},
    // InlrStackComputations,
    {InlPrtCost, "Callee has key stack computations"},
    // InlrPreferPartialInline
    {InlPrtCost, "Preferred for partial inlining"},
    // InlrPassedDummyArgs
    {InlPrtCost, "Callee has callsites with dummy args"},
    // InlrArrayStructArgs
    {InlPrtCost, "Callee has callsites with array struct args"},
    // InlrPreferTileChoice
    {InlPrtCost, "Callsite inlined to enable tiling"},
    // InlrManyRecursiveCallsSplitting
    {InlPrtCost, "Callsite inlined for many recursive calls splitting"},
    // InlrHasSmallAppBudget
    {InlPrtCost, "Has inline budget for small application"},
    // InlrExposesLocalArrays
    {InlPrtCost, "Exposes local arrays"},
    // InlrUnderTBBParallelFor
    {InlPrtCost, "Under TBB parallel for"},
    // InlrProfitable,
    {InlPrtCost, "Inlining is profitable"},
    // InlrLast,
    {InlPrtNone, nullptr},
    // NinlrFirst,
    {InlPrtNone, nullptr},
    // NinlrNoReason,
    {InlPrtSimple, "Not tested for inlining"},
    // NinlrNoinlineList,
    {InlPrtSimple, "Callsite on noinline list"},
    // NinlrColdCC,
    {InlPrtCost, "Callee has cold calling convention"},
    // NinlrColdProfile,
    {InlPrtCost, "Callsite has cold profile"},
    // NinlrColdCallsite,
    {InlPrtCost, "Callsite is cold"},
    // NinlrColdCallee,
    {InlPrtCost, "Callee is cold"},
    // NinlrDeleted,
    {InlPrtSpecial, nullptr},
    // NinlrDuplicateCall,
    {InlPrtSimple, "Callee cannot be called more than once"},
    // NinlrDynamicAlloca,
    {InlPrtCost, "Callee has dynamic alloca"},
    // NinlrExtern,
    {InlPrtSpecial, nullptr},
    // NinlrIndirect,
    {InlPrtSpecial, "Call site is indirect"},
    // NinlrIndirectBranch,
    {InlPrtCost, "Callee has indirect branch"},
    // NinlrBlockAddress,
    {InlPrtCost, "Callee has block address"},
    // NinlrCallsLocalEscape,
    {InlPrtCost, "Callee calls localescape"},
    // NinlrCallsBranchFunnel,
    {InlPrtCost, "Callee calls branch funnel"},
    // NinlrNeverInline,
    {InlPrtSimple, "Callee is never inline"},
    // NinlrIntrinsic,
    {InlPrtSimple, "Callee is intrinsic"},
    // NinlrOuterInlining,
    {InlPrtSpecial, "High outer inlining cost"},
    // NinlrRecursive,
    {InlPrtSimple, "Callee has recursion"},
    // NinlrReturnsTwice,
    {InlPrtSimple, "Callee has returns twice instruction"},
    // NinlrTooMuchStack,
    {InlPrtCost, "Callee uses too much stack space"},
    // NinlrVarargs,
    {InlPrtSimple, "Callee is varargs"},
    // NinlrMismatchedAttributes,
    {InlPrtSimple, "Caller/Callee mismatched attributes"},
    // NinlrMismatchedGC
    {InlPrtSimple, "Caller/Callee garbage collector mismatch"},
    // NinlrMismatchedPersonality,
    {InlPrtSimple, "Caller/Callee personality mismatch"},
    // NinlrNoinlineAttribute
    {InlPrtSimple, "Callee has noinline attribute"},
    // NinlrNoinlineCallsite,
    {InlPrtSimple, "Callsite is noinline"},
    // NinlrNoReturn,
    {InlPrtSimple, "Callee is noreturn"},
    // NinlrOptNone,
    {InlPrtSimple, "Callee is opt none"},
    // NinlrMayBeOverriden,
    {InlPrtSimple, "Callee may be overriden"},
    // NinlrNotPossible,
    {InlPrtSimple, "Not legal to inline"},
    // NinlrNotAlwaysInline,
    {InlPrtSimple, "Callee is not always_inline"},
    // NinlrNewlyCreated,
    {InlPrtSimple, "Newly created callsite"},
    // NinlrNotProfitable,
    {InlPrtCost, "Inlining is not profitable"},
    // NinlrOpBundles,
    {InlPrtSimple, "Cannot inline call with operand bundle"},
    // NinlrMSVCEH,
    {InlPrtSimple, "Microsoft EH prevents inlining"},
    // NinlrSEH,
    {InlPrtSimple, "Structured EH prevents inlining"},
    // NinlrPreferCloning,
    {InlPrtSimple, "Callsite preferred for cloning"},
    // NinlrNullPtrMismatch,
    {InlPrtSimple, "Caller/callee null pointer mismatch"},
    // NinlrPreferMultiversioning,
    {InlPrtSimple, "Callsite preferred for multiversioning"},
    // NinlrPreferSOAToAOS,
    {InlPrtSimple, "Callsite preferred for SOA-to-AOS"},
    // NinlrStackComputations
    {InlPrtSimple, "Callsite has key stack computations"},
    // NinlrSwitchComputations
    {InlPrtSimple, "Callsite has key switch computations"},
    // NinlrDelayInlineDecision
    {InlPrtSimple, "Inline decision is delayed until link time"},
    // NinlrPreferPartialInline
    {InlPrtSimple, "Outlined function from partial inlining"},
    // NinlrCalleeHasExceptionHandling
    {InlPrtSimple, "Callee has exception handling"},
    // NinlrIsCrossLanguage
    {InlPrtSimple, "Caller and Callee have different source languages"},
    // NinlrNotMandatory
    {InlPrtSimple, "Inlining is not mandatory"},
    // NinlrUnsplitCoroutineCall
    {InlPrtSimple, "Unsplit coroutine call"},
    // NinlrByvalArgsWithoutAllocaAS
    {InlPrtSimple, "Call has byval argument without alloca address space"},
    // NinlrMultiversionedCallsite
    {InlPrtSimple, "Multiversioned callsite"},
    // NinlrDelayInlineDecisionLTO
    {InlPrtSimple, "Inline decision is delayed"},
    // NinlrLast
    {InlPrtNone, nullptr}};

static_assert(sizeof(InlineReasonText) ==
              sizeof(InlPrtRecord) * (InlineReportTypes::NinlrLast + 1),
              "Missing report message");

// Print indent
void printIndentCount(formatted_raw_ostream &OS, unsigned indentCount);
// Get string value from metadata consuming 'Front' of the MDString
StringRef getOpStr(Metadata *Node, StringRef Front);
// Get integer value from metadata consuming 'Front' of the MDString
void getOpVal(Metadata *Node, StringRef Front, int64_t *Val);
// Print the inlining option values
void printOptionValues(formatted_raw_ostream &OS, unsigned OptLevel = 0,
                       unsigned SizeLevel = 0);
// Print function inline report
void printFunctionInlineReport(Function *F, unsigned Level);
// Print call site inline report
void printCallSiteInlineReport(Instruction *I, unsigned Level);
// Skip some llvm-specific intrinsics to make inline report shorter.
bool shouldSkipIntrinsic(IntrinsicInst *I);
} // namespace llvm

#endif // LLVM_TRANSFORMS_IPO_INTEL_INLINEREPORTCOMMON_H
