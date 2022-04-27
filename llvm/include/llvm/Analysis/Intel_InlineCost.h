#if INTEL_FEATURE_SW_ADVANCED
//===-------------- Intel_InlineCost.h -----------------------------------===//
//
// Copyright (C) 2020-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// Intel-specific inline cost analysis: This file should include functions
// that are large code modifications to the llorg inlining analysis in
// InlineCost.cpp. Small modifications to existing llorg code in InlineCost.cpp
// should be made directly in InlineCost.cpp. This will minimize the problems
// that occur during a pulldown.
//===----------------------------------------------------------------------===//
//
#ifndef LLVM_ANALYSIS_INTELINLINECOST_H
#define LLVM_ANALYSIS_INTELINLINECOST_H

#include "llvm/ADT/SmallSet.h"
#include "llvm/Analysis/InlineCost.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Instructions.h"

//
// NOTE: Some of these functions have a number of parameters which appear
// as private members of the CallAnalyzer and InlineCostCallAnalyzer classes.
// It might be useful to rewrite these as member functions of those classes
// to avoid the parameter passing.
//

using namespace llvm;

namespace llvm {

class TargetLibraryInfo;
class TargetTransformInfo;
class ProfileSummaryInfo;

using namespace InlineReportTypes;

// Return 'true' if 'F' is a huge function. In particular, huge functions
// are less preferred for inlining, even if there is only one callsite.
extern bool isHugeFunction(Function *F, InliningLoopInfoCache *ILIC);

// Return 'true' if 'TI' is an Instruction used in a 'forgivableCondition'.
// A function which would be a single basic block except for the forgivable
// condition will be treated for heuristic purposes as if it has a single
// basic block.
extern bool forgivableCondition(const Instruction *TI);

// Return 'true' if 'I' is a dynamic AllocaInst in the called function 'F' of
// 'CandidateCall', but that we should not inhibit the inlining of 'F'
// simply because 'I' appears in 'F'.
extern bool isDynamicAllocaException(AllocaInst &I, CallBase &CandidateCall,
                                     const InlineParams &Params,
                                     const TargetTransformInfo &CalleeTTI,
                                     WholeProgramInfo *WPI);

// We would wrap the following function declaration inside INTEL FEATURE SW
// DTRANS, but the current FIF tool does not permit nesting of that within
// INTEL FEATURE SW ADVANCED.

// Set inline/noninline attributes for DTrans inlining heuristics.
extern void collectDTransFuncs(Module &M, const InlineParams &Params);

// Return an 'InlineResult' if 'CandidateCall' is a callsite that is not
// worth inlining according to some special Intel inlining heuristic.
// If there is no special reason to inhibit the inlining, return 'None'.
extern Optional<InlineResult> intelWorthNotInlining(
    CallBase &CandidateCall, const InlineParams &Params, TargetLibraryInfo *TLI,
    const TargetTransformInfo &CalleeTTI, ProfileSummaryInfo *PSI,
    InliningLoopInfoCache *ILIC, SmallPtrSetImpl<Function *> *QueuedCallers,
    InlineReasonVector &NoReasonVector);

// If 'CB' is a callsite for which an Intel inlining heuristic demonstrates
// a good reason for inlining it, return a (negative) bonus that can be added
// to the cost of inlining the callsite. This will make it much more likely
// that 'CB' will be inlined. If no such reason exists, return 0.
extern int intelWorthInlining(CallBase &CB, const InlineParams &Params,
                              TargetLibraryInfo *TLI,
                              const TargetTransformInfo &CalleeTTI,
                              ProfileSummaryInfo *PSI,
                              InliningLoopInfoCache *ILIC,
                              SmallPtrSetImpl<Function *> *QueuedCallers,
                              InlineReasonVector &YesReasonVector,
                              WholeProgramInfo *WPI, bool IsCallerRecursive);

} // end namespace llvm

#endif // LLVM_ANALYSIS_INTELINLINECOST_H
#endif // INTEL_FEATURE_SW_ADVANCED
