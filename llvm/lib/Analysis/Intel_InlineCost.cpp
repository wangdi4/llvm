//===------- Intel_InlineCost.cpp ----------------------- -*------===//
//
// Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#if INTEL_FEATURE_SW_ADVANCED
#include "llvm/Analysis/Intel_InlineCost.h"
#endif // INTEL_FEATURE_SW_ADVANCED
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Analysis/CodeMetrics.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/Analysis/InlineCost.h"
#include "llvm/Analysis/InstructionSimplify.h"
#if INTEL_FEATURE_SW_ADVANCED
#include "llvm/Analysis/Intel_IPCloningAnalysis.h"
#endif // INTEL_FEATURE_SW_ADVANCED
#include "llvm/Analysis/Intel_OPAnalysisUtils.h"
#if INTEL_FEATURE_SW_ADVANCED
#include "llvm/Analysis/Intel_PartialInlineAnalysis.h"
#endif // INTEL_FEATURE_SW_ADVANCED
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/ProfileSummaryInfo.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/Demangle/Demangle.h"
#include "llvm/IR/AssemblyAnnotationWriter.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/GetElementPtrTypeIterator.h"
#include "llvm/IR/GlobalAlias.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/GenericDomTree.h"
#include "llvm/Support/raw_ostream.h"
#if INTEL_FEATURE_SW_ADVANCED
#include "llvm/Transforms/IPO/Intel_IPCloning.h"
#include "llvm/Transforms/IPO/Intel_InlineReportCommon.h"
#endif // INTEL_FEATURE_SW_ADVANCED
#include <algorithm>
#include <queue>

using namespace llvm;
using namespace InlineReportTypes;
using namespace llvm::PatternMatch;
using namespace llvm::vpo;

#define DEBUG_TYPE "inline-cost"

//
// Major internal options. Some of these are used in InlineCost.cpp.
//

// These options are primarily used for LIT tests.

cl::opt<bool> InlineForXmain("inline-for-xmain", cl::Hidden, cl::init(true),
                             cl::desc("Xmain customization of inlining"));

cl::opt<bool> EnablePreLTOInlineCost("pre-lto-inline-cost", cl::Hidden,
                                     cl::init(false),
                                     cl::desc("Enable pre-LTO inline cost"));

cl::opt<bool> DTransInlineHeuristics(
    "dtrans-inline-heuristics", cl::Hidden, cl::init(false),
    cl::desc("inlining heuristics controlled under -qopt-mem-layout-trans"));

cl::opt<bool> EnableLTOInlineCost("lto-inline-cost", cl::Hidden,
                                  cl::init(false),
                                  cl::desc("Enable LTO inline cost"));

cl::opt<bool>
    EnableSYCLOptimizationMode("sycl-optimization-mode", cl::Hidden,
                               cl::init(false),
                               cl::desc("Enable SYCL Optimization Mode"));

namespace llvm {

#if INTEL_FEATURE_SW_ADVANCED
//
// Intel specific internal options
//

//
// Options controlling the recognition of a small application
//

// To be considered a small application, the program should have no more than
// this many 'defined' Functions.
static cl::opt<unsigned> SmallAppUserFunctionMax(
    "inlining-small-app-user-function-max", cl::init(8), cl::ReallyHidden,
    cl::desc("Maximum number of user functions in small app"));

// To be considered a small application, the program should have no more than
// this many callsites to 'defined' Functions.
static cl::opt<unsigned> SmallAppUserCallBaseMax(
    "inlining-small-app-user-callbase-max", cl::init(70), cl::ReallyHidden,
    cl::desc("Maximum number of calls to user functions in small app"));

// No more than this many additional inlinings should be performed purely
// because this is a small application.
static cl::opt<unsigned> SmallAppUserMaxExtraCallBases(
    "inlining-small-app-max-extra-callbases", cl::init(10), cl::ReallyHidden,
    cl::desc("Maximum number of extra calls to inline in small app"));

// Use this to force the opt level used for inlining heuristics in LIT tests
static cl::opt<unsigned>
    ForcedInlineOptLevel("forced-inline-opt-level", cl::init(0),
                         cl::ReallyHidden,
                         cl::desc("Forced inlining opt level"));

//
// Options controlling exceoptions for dynamic allocas
//

// Use this to control how large a dynamic alloca char array can be before
// its presence inhibits the inlining of the Function into other Functions.
static cl::opt<unsigned> DynAllocaMaxCharArraySize(
    "inlining-dyn-alloca-max-char-array-size", cl::init(4096), cl::ReallyHidden,
    cl::desc("Maximum size of char array which is dynamic alloca exception"));

//
// Options controlling the recognition of a huge function
//

// Options to set the number of formal arguments, basic blocks, and loops
// for a huge function. We may suppress the inlining of functions which
// have these many arguments, basic blocks, and loops, even if they are
// have local linkage and a single call site.
static cl::opt<unsigned> HugeFunctionBasicBlockCount(
    "inlining-huge-bb-count", cl::init(90), cl::ReallyHidden,
    cl::desc("Function with this many basic blocks or more may be huge"));

static cl::opt<unsigned> HugeODRFunctionBasicBlockCount(
#ifndef _WIN32
    "inlining-huge-odr-bb-count", cl::init(35), cl::ReallyHidden,
#else
    "inlining-huge-odr-bb-count", cl::init(25), cl::ReallyHidden,
#endif // _WIN32
    cl::desc("ODR Function with this many basic blocks or more is huge"));

static cl::opt<unsigned> HugeLTOODRFunctionBasicBlockCount(
    "inlining-huge-lto-odr-bb-count", cl::init(200), cl::ReallyHidden,
    cl::desc("LTO ODR Function with this many basic blocks or more is huge"));

static cl::opt<unsigned> HugeFunctionArgCount(
    "inlining-huge-arg-count", cl::init(8), cl::ReallyHidden,
    cl::desc("Function with this many arguments or more may be huge"));

static cl::opt<unsigned> HugeFunctionLoopCount(
    "inlining-huge-loop-count", cl::init(11), cl::ReallyHidden,
    cl::desc("Function with this many loops or more may be huge"));

//
// Options for specific non-inlining heuristics
//

// preferNotToInlineForSwitchComputations()
// Minimal number of cases in a switch to qualify for the "prefer not to
// inline for switch computations" heuristic.
static cl::opt<unsigned> MinSwitchCases(
    "inline-for-switch-min-cases", cl::Hidden, cl::init(11),
    cl::desc("Min number of switch cases required to trigger heuristic"));

//
// Options for specific inlining heuristics
//

// isProfInstrumentHotCallSite()
// Set the percentage within 100% for which any callsite is considered
// hot via instrumented profile info. (For example, a value of 85 means
// any callsite with an execution count of at least 15% (=100%-85%) of
// the hottest callsite's execution count could be considered.
static cl::opt<unsigned> ProfInstrumentHotPercentage(
    "inline-prof-instr-hot-percentage", cl::Hidden, cl::init(85),
    cl::desc("Calls within this percentage of the hottest call are hot"));

// Sets a limit on the number of callsites that can be considered hot
// due to instrumented profile execution count. For example, "5" means
// that, on entry to the inliner, only the callsites with the 5 hottest
// execution counts will be considered as having "hot" profiles. (Note
// that there could be actually be more than this number in the inline
// report if a callsite marked hot gets cloned (for example, during
// inlining).
static cl::opt<unsigned> ProfInstrumentHotCount(
    "inline-prof-instr-hot-count", cl::Hidden, cl::init(5),
    cl::desc("Number of call sites to be considered hot"));

// worthyDoubleInternalCallSite()
// worthyDoubleExternalCallSite()
// Temporary switch to control new double callsite inlining heuristics
// until tuning of loopopt is complete.
static cl::opt<bool> NewDoubleCallSiteInliningHeuristics(
    "new-double-callsite-inlining-heuristics", cl::init(false),
    cl::ReallyHidden);

// Maximum number of basic blocks allowed for inner specially handled
// double callsite external Functions.
static cl::opt<unsigned> MaxBBInnerDoubleExternalFxn(
    "max-bb-inner-double-external-fxn", cl::init(10), cl::ReallyHidden);

// Maximum number of basic blocks allowed for outer specially handled
// double callsite external Functions.
static cl::opt<unsigned> MaxBBOuterDoubleExternalFxn(
    "max-bb-outer-double-external-fxn", cl::init(30), cl::ReallyHidden);

// Minimum number of arguments in a callee candidate for inlining in one
// double callsite external Functions heuristic.
static cl::opt<unsigned> MinCalleeArgsDoubleExternal(
    "min-callee-args-double-external", cl::init(14), cl::ReallyHidden);

// Minimum number of IVDEP loops in a callee candidate for inlining in one
// double callsite external Functions heuristic.
static cl::opt<unsigned> MinCalleeIVDEPLoopsDoubleExternal(
    "min-callee-ivdep-loops-double-external", cl::init(21), cl::ReallyHidden);

// worthInliningForFusion()

// Options for early fusion heuristic
//
// Maximum number of BasicBlocks in the callee.
static cl::opt<unsigned>
     NumBBsForEarlyFusion("inline-for-early-fusion-num-bbs",
                          cl::init(30), cl::ReallyHidden);

// Minimum number of BasicBlocks in the callee that terminate in a SwitchInst.
static cl::opt<unsigned>
     NumSwitchesForEarlyFusion("inline-for-early-fusion-num-switches",
                               cl::init(11), cl::ReallyHidden);

// Minimum number of total cases in BasicBlocks in the callee that terminate
// in a SwitchInst.
static cl::opt<unsigned>
     NumCasesForEarlyFusion("inline-for-early-fusion-num-cases",
                            cl::init(25), cl::ReallyHidden);

// Minimum number of incoming values in a PHI node that feeds a single return
// value.
static cl::opt<unsigned>
     NumRetPHIInputsForEarlyFusion("inline-for-early-fusion-num-ret-phi-inputs",
                                   cl::init(11), cl::ReallyHidden);

// Options for standard fusion heuristic
//
// Number of successive callsites need to be inlined to benefit
// from regular loops fusion.
static cl::list<int> NumCallSitesForFusion("inline-for-fusion-num-callsites",
                                           cl::ReallyHidden,
                                           cl::CommaSeparated);
// Temporary switch to control new callsite inlining heuristics
// for fusion until tuning of loopopt is complete.
static cl::opt<cl::boolOrDefault>
    InliningForFusionHeuristics("inlining-for-fusion-heuristics",
                                cl::ReallyHidden);

// Minimal number of arrays in loop that should be function arguments.
static cl::opt<int> MinArgRefs(
    "inline-for-fusion-min-arg-refs", cl::Hidden, cl::init(10),
    cl::desc(
        "Min number of arguments appearing in loop candidates for fusion"));

static cl::opt<unsigned> FusionMinLoopNestDepth(
    "inline-for-fusion-loop-nest-depth", cl::Hidden, cl::init(3),
    cl::desc("Min depth of loop nest for link time inlining for fusion"));

static cl::opt<unsigned> FusionSmallAppFunctionLimit(
    "inline-for-fusion-small-app-function-limit", cl::Hidden, cl::init(10),
    cl::desc("Max user Function count in inlining for fusion"));

// worthInliningForDeeplyNestedIfs()
// InliningForDeeplyNestedIfs has three possible values(BOU_UNSET is
// default). Use TRUE to force enabling of heuristic. Use FALSE to disable.
static cl::opt<cl::boolOrDefault> InliningForDeeplyNestedIfs(
    "inlining-for-deep-ifs", cl::ReallyHidden,
    cl::desc("Option that enables inlining for deeply nested IFs"));

static cl::opt<unsigned> MinDepthOfNestedIfs(
    "inlining-min-if-depth", cl::init(9), cl::ReallyHidden,
    cl::desc("Minimal depth of IF nest to trigger inlining"));

// worthInliningForAddressComputations()
// InliningForAddressComputations has three possible values(BOU_UNSET is
// default). Use TRUE to force enabling of heuristic. Use FALSE to disable.
static cl::opt<cl::boolOrDefault> InliningForAddressComputations(
    "inlining-for-address-computations", cl::ReallyHidden,
    cl::desc("Option that enables inlining for address computations"));

static cl::opt<unsigned> InliningForACLoopDepth(
    "inlining-for-ac-loop-depth", cl::init(2), cl::ReallyHidden,
    cl::desc("Nesting level of the loop in which appears the callsite."));

static cl::opt<unsigned>
    InliningForACMinArgRefs("inlining-for-ac-min-arg-refs", cl::init(11),
                            cl::ReallyHidden,
                            cl::desc("Minimal number of arguments appearing in "
                                     "array accesses inside callee."));

// worthInliningCallSitePassedDummyArgs()
// Options to set the minimum number of formal arguments, the length of the
// formal argument series, the length of matching arguments in the sub-series,
// and the minimum number of callsites for the "dummy args" inlining
// heuristic.
static cl::opt<unsigned> DummyArgsMinArgCount(
    "inlining-dummy-args-min-arg-count", cl::init(8), cl::ReallyHidden,
    cl::desc("Minimum number of args for dummy-args function"));

static cl::opt<unsigned> DummyArgsMinSeriesLength(
    "inlining-dummy-args-min-series-length", cl::init(5), cl::ReallyHidden,
    cl::desc("Minimum length of series for dummy-args function"));

static cl::opt<unsigned> DummyArgsMinSeriesMatch(
    "inlining-dummy-args-min-series-match", cl::init(4), cl::ReallyHidden,
    cl::desc("Minimum matching length in series for dummy-args function"));

static cl::opt<unsigned> DummyArgsMinCallsiteCount(
    "inlining-dummy-args-min-callsite-count", cl::init(5), cl::ReallyHidden,
    cl::desc("Minimum callsite count for dummy-args function"));

// worthInliningForArrayStructArgs()
// Minimum number of uses all of the special array struct args must have to
// qualify a callee for inlining by the "array struct args" inline heuristic.
static cl::opt<unsigned> ArrayStructArgMinUses(
    "inline-for-array-struct-arg-min-uses", cl::Hidden, cl::init(16),
    cl::desc("Min number of uses for array-struct-arg heuristic"));

// Minimum number of args in the caller of a callsite to qualify a callsite
// for inlining by the "array struct args" inline heuristic.
static cl::opt<unsigned> ArrayStructArgMinCallerArgs(
    "inline-for-array-struct-arg-min-caller-args", cl::Hidden, cl::init(6),
    cl::desc("Min number of caller args for array-struct-arg heuristic"));

// worthInliningExposesLocalArrays()
// Options to set the minimum number of formal arguments, mimimum number
// of callsites, minimum number of array args, and the minimum number of
// array dimensions for the "exposes local arrays" inline heuristic.
// While searching back the call chain, limit the number of Functions
// we can traverse through.
//
// The goal is to inline to expose local arrays that can be collapsed by
// Loop Opt.
static cl::opt<unsigned> ExposeLocalArraysMinArgs(
    "inline-expose-local-arrays-min-args", cl::init(10), cl::ReallyHidden,
    cl::desc("Minimum args for expose local arrays candidate"));

static cl::opt<unsigned> ExposeLocalArraysMinCalls(
    "inline-expose-local-arrays-min-calls", cl::init(6), cl::ReallyHidden,
    cl::desc("Minimum calls for expose local arrays candidate"));

static cl::opt<unsigned> ExposeLocalArraysMinArrayArgs(
    "inline-expose-local-arrays-min-array-args", cl::init(2), cl::ReallyHidden,
    cl::desc("Minimum array args for expose local arrays candidate"));

static cl::opt<unsigned> ExposeLocalArraysMinDims(
    "inline-expose-local-arrays-min-dims", cl::init(3), cl::ReallyHidden,
    cl::desc("Minimum dimensions for expose local arrays candidate"));

static cl::opt<unsigned> ExposeLocalArraysMaxDepth(
    "inline-expose-local-arrays-max-depth", cl::init(5), cl::ReallyHidden,
    cl::desc("Maximum traversal depth for expose local arrays candidate"));

// worthInliningUnderTBBParallelFor()
// 
// The idea is to match a series of TBBParallelForMinCallBaseMatch calls
// to a Function which have not yet been inlined and are under a TBB parallel
// for. The calls must have at least TBBParallelForMinArgTotal actual
// arguments of which at least TBBParallelForMinArgMatch must match.
// The calls must be no more than TBBParallelForMaxDepth levels down the
// call graph from the TBB parallel for. While traversing up the call graph
// from the candidate CallBase, give up if the fan out from Function to
// CallBase users is more than TBBParallelForMaxWidth.
//
// For now, we apply this heuristic on the link step of an -flto compilation.
// Also we limit it to Modules that have at least TBBParallelForMinFuncs
// Functions. This can be loosened later, if we think that is beneficial.
//
// The goal is to provide extra inlining for Functions called under a TBB
// parallel for, because they are likely to be hot.
//
static cl::opt<unsigned> TBBParallelForMinCallBaseMatch(
    "inline-tbb-parallel-min-callbase-match", cl::init(3), cl::ReallyHidden,
    cl::desc("Min CallBase match for preferred inline under tbb parallel for"));

static cl::opt<unsigned> TBBParallelForMaxCallBaseMatch(
    "inline-tbb-parallel-max-callbase-match", cl::init(3), cl::ReallyHidden,
    cl::desc("Max CallBase match for preferred inline under tbb parallel for"));

static cl::opt<unsigned> TBBParallelForMinArgMatch(
    "inline-tbb-parallel-for-min-arg-match", cl::init(2), cl::ReallyHidden,
    cl::desc("Minimum arg match for preferred inline under tbb parallel for"));

static cl::opt<unsigned> TBBParallelForMinArgTotal(
    "inline-tbb-parallel-for-min-arg-total", cl::init(4), cl::ReallyHidden,
    cl::desc("Minimum arg total for preferred inline under tbb parallel for"));

static cl::opt<unsigned> TBBParallelForMaxDepth(
    "inline-tbb-parallel-max-depth", cl::init(2), cl::ReallyHidden,
    cl::desc("Max depth for preferred inline under tbb parallel for"));

static cl::opt<unsigned> TBBParallelForMaxWidth(
    "inline-tbb-parallel-for-max-width", cl::init(2), cl::ReallyHidden,
    cl::desc("Max width for preferred inline under tbb parallel for"));

static cl::opt<unsigned> TBBParallelForMinFuncs(
    "inline-tbb-parallel-for-min-funcs", cl::init(75000), cl::ReallyHidden,
    cl::desc("Min functions for preferred inline under tbb parallel for"));

#endif // INTEL_FEATURE_SW_ADVANCED

//
// Implementation of the Intel LoopInfo Cache (ILIC).
//

DominatorTree *InliningLoopInfoCache::getDT(Function *F) {
  auto It = DTMapSCC.find(F);
  if (It != DTMapSCC.end())
    return It->second;
  auto ret = new DominatorTree(*F);
  DTMapSCC.insert(std::make_pair(F, ret));
  return ret;
}

LoopInfo *InliningLoopInfoCache::getLI(Function *F) {
  auto It = LIMapSCC.find(F);
  if (It != LIMapSCC.end())
    return It->second;
  DominatorTree *DT = getDT(F);
  assert(DT != nullptr);
  auto ret = new LoopInfo(*DT);
  LIMapSCC.insert(std::make_pair(F, ret));
  return ret;
}

void InliningLoopInfoCache::invalidateFunction(Function *F) {
  auto DTit = DTMapSCC.find(F);
  if (DTit != DTMapSCC.end()) {
    delete DTit->second;
    DTMapSCC.erase(DTit);
  }
  auto LIit = LIMapSCC.find(F);
  if (LIit != LIMapSCC.end()) {
    delete LIit->second;
    LIMapSCC.erase(LIit);
  }
}

InliningLoopInfoCache::~InliningLoopInfoCache() {
  for (auto &DTI : DTMapSCC)
    delete DTI.second;
  DTMapSCC.clear();
  for (auto &LTI : LIMapSCC)
    delete LTI.second;
  LIMapSCC.clear();
}

#if INTEL_FEATURE_SW_ADVANCED
//
// Functions to manage profiling
//

//
// Return a profile count for 'Call' using 'PSI' if one exists.
//
static Optional<uint64_t> profInstrumentCount(ProfileSummaryInfo *PSI,
                                              CallBase &Call) {
  if (!DTransInlineHeuristics)
    return None;
  if (!PSI || !PSI->hasInstrumentationProfile())
    return None;
  MDNode *MD = Call.getMetadata(LLVMContext::MD_intel_profx);
  if (!MD)
    return None;
  assert(MD->getNumOperands() == 2);
  ConstantInt *CI = mdconst::extract<ConstantInt>(MD->getOperand(1));
  assert(CI);
  return CI->getValue().getZExtValue();
}

//
// Return the threshold over which a callsite is considered hot.
//
static uint64_t profInstrumentThreshold(ProfileSummaryInfo *PSI, Module *M) {
  static bool ComputedThreshold = false;
  static uint64_t Threshold = 0;
  if (ComputedThreshold)
    return Threshold;
  uint64_t MaxProfCount = 0;
  // Find hottest callsite and its profile count.
  std::priority_queue<uint64_t, std::vector<uint64_t>, std::greater<uint64_t>>
      ProfQueue;
  for (auto &F : M->functions()) {
    for (User *U : F.users()) {
      auto CB = dyn_cast<CallBase>(U);
      if (!CB)
        continue;
      Optional<uint64_t> ProfCount = profInstrumentCount(PSI, *CB);
      if (ProfCount == None)
        continue;
      uint64_t NewValue = ProfCount.getValue();
      if (ProfQueue.size() < ProfInstrumentHotCount) {
        ProfQueue.push(NewValue);
      } else if (NewValue > ProfQueue.top()) {
        ProfQueue.pop();
        ProfQueue.push(NewValue);
      }
      if (NewValue > MaxProfCount)
        MaxProfCount = NewValue;
    }
  }
  // Adjust the threshold by the ProfInstrumentHotPercentage.
  uint64_t PercentThreshold =
      MaxProfCount - ProfInstrumentHotPercentage * MaxProfCount / 100;
  uint64_t CountThreshold = ProfQueue.size() > 0 ? ProfQueue.top() : 0;
  Threshold = std::max(CountThreshold, PercentThreshold);
  ComputedThreshold = true;
  return Threshold;
}

//
// Return 'true' if 'CB' is a hot callsite based on 'PSI. Do not recognize
// hot callsites during the 'PrepareForLTO' phase.
//
static bool isProfInstrumentHotCallSite(CallBase &CB, ProfileSummaryInfo *PSI,
                                        bool PrepareForLTO) {
  if (!DTransInlineHeuristics || PrepareForLTO)
    return false;
  Module *M = CB.getParent()->getParent()->getParent();
  uint64_t Threshold = profInstrumentThreshold(PSI, M);
  Optional<uint64_t> ProfCount = profInstrumentCount(PSI, CB);
  if (ProfCount == None)
    return false;
  return ProfCount.getValue() >= Threshold;
}

//
// Increment 'GlobalCount' if a load of a global value appears in 'Op'.
// Increment 'ConstantCount' if an integer constant appears in 'Op'.
//
static void countGlobalsAndConstants(const Value *Op, unsigned &GlobalCount,
                                     unsigned &ConstantCount) {
  const LoadInst *LILHS = dyn_cast<LoadInst>(Op);
  if (LILHS) {
    const Value *GV = LILHS->getPointerOperand();
    if (GV && isa<GlobalValue>(GV))
      GlobalCount++;
  } else if (isa<ConstantInt>(Op))
    ConstantCount++;
}

//
// Return 'true' if a branch is based on a condition of the form:
//       global-variable .op. constant-integer
// or
//       constant-integer .op. global-variable
// The intent is that such a branch has some likelihood of being eliminated
// leaving a single basic block, and the heuristics should reflect this.
//
extern bool forgivableCondition(const Instruction *TI) {
  const BranchInst *BI = dyn_cast<BranchInst>(TI);
  if (!BI || !BI->isConditional())
    return false;
  const Value *Cond = BI->getCondition();
  const ICmpInst *ICmp = dyn_cast<ICmpInst>(Cond);
  if (!ICmp)
    return false;
  const Value *LHS = ICmp->getOperand(0);
  const Value *RHS = ICmp->getOperand(1);
  unsigned GlobalCount = 0;
  unsigned ConstantCount = 0;
  countGlobalsAndConstants(LHS, GlobalCount, ConstantCount);
  countGlobalsAndConstants(RHS, GlobalCount, ConstantCount);
  return ConstantCount == 1 && GlobalCount == 1;
}

//
// A function can be considered huge if it has too many formal arguments,
// basic blocks, and loops. We test them in this order (from cheapest to
// most expensive).
//
extern bool isHugeFunction(Function *F, InliningLoopInfoCache *ILIC,
                           const TargetTransformInfo &TTI, bool PrepareForLTO,
                           bool LinkForLTO, bool IsSYCLHost,
                           bool IsSYCLDevice) {
  if (!InlineForXmain)
    return false;
  if (F->hasLinkOnceODRLinkage() && !IsSYCLHost && !IsSYCLDevice &&
      !DTransInlineHeuristics) {
      if (!PrepareForLTO && !LinkForLTO) {
        if (F->size() > HugeODRFunctionBasicBlockCount)
          return true;
      } else {
        if (F->size() > HugeLTOODRFunctionBasicBlockCount)
          return true;
      }
  }
  if (!DTransInlineHeuristics)
    return false;
  size_t ArgSize = F->arg_size();
  if (ArgSize < HugeFunctionArgCount)
    return false;
  size_t BBCount = F->size();
  if (BBCount < HugeFunctionBasicBlockCount)
    return false;
  LoopInfo *LI = ILIC->getLI(F);
  if (!LI)
    return false;
  size_t LoopCount = std::distance(LI->begin(), LI->end());
  if (LoopCount < HugeFunctionLoopCount)
    return false;
  return true;
}

//
// Return 'true' if the program we are compiling passes some initial,
// general conditions for being a small application: A small Fortran
// program compiled on the link step with -O3 -flto -xCORE-AVX2, and
// is whole program "read".
//
static bool passesMinimalSmallAppConditions(
    CallBase &CB, const TargetTransformInfo &CalleeTTI, WholeProgramInfo *WPI,
    bool LinkForLTO, unsigned InlineOptLevel) {
  //
  // Return 'true' if the allowed number of calls to 'defined' Functions
  // is greater than 'Limit'. This will disqualify 'CB' for inlining under
  // the small application inlining heuristic.
  //
  auto SmallSizeLimitExceeded = [](Module &M, unsigned FunctionLimit,
                                   unsigned CallBaseLimit) -> bool {
    unsigned CallBaseCount = 0;
    unsigned FunctionCount = 0;
    for (auto &F : M.functions()) {
      if (F.isDeclaration())
        continue;
      if (++FunctionCount > FunctionLimit)
        return true;
      for (User *U : F.users()) {
        auto CB = dyn_cast<CallBase>(U);
        if (!CB)
          continue;
        if (++CallBaseCount > CallBaseLimit)
          return true;
      }
    }
    return false;
  };

  //
  // Return 'true' if the program we are compiling is considered a small
  // application, and therefore eligible for more inlining under the small
  // application inlining heuristic.
  //
  auto IsSmall = [&SmallSizeLimitExceeded](Module &M) -> bool {
    static bool IsAlreadyTested = false;
    static bool IsSmallApp = false;
    if (IsAlreadyTested)
      return IsSmallApp;
    IsSmallApp = !SmallSizeLimitExceeded(M, SmallAppUserFunctionMax,
                                         SmallAppUserCallBaseMax);
    IsAlreadyTested = true;
    return IsSmallApp;
  };

  if (!(LinkForLTO || EnableLTOInlineCost))
    return false;
  if (!WPI || !WPI->isWholeProgramRead())
    return false;
  if (!(ForcedInlineOptLevel >= 3 || InlineOptLevel >= 3))
    return false;
  Function *Caller = CB.getCaller();
  Function *Callee = CB.getCalledFunction();
  if (!Caller->isFortran() || !Callee->isFortran())
    return false;
  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX2;
  if (!CalleeTTI.isAdvancedOptEnabled(TTIAVX2))
    return false;
  if (!IsSmall(*(Callee->getParent())))
    return false;
  return true;
}

extern bool isDynamicAllocaException(AllocaInst &I, CallBase &CandidateCall,
                                     const InlineParams &Params,
                                     const TargetTransformInfo &CalleeTTI,
                                     WholeProgramInfo *WPI) {

  // Arguments are passed to OpenMP calls indirectly. That means, original
  // arguments are saved on stack and then addresses of the stack locations are
  // passed as arguments as shown in the example below. This change is needed
  // to enable inlining for OpenMP calls.
  //
  // Returns true if "I", which allocates either integer or pointer type,
  // is used by only StoreInst/CallBase/LoadInst/BitCastInst instructions.
  // BitCastInst: Allow only when bitcast is used by lifetime intrinsics.
  // LoadInst: Allow only when DTransInlineHeuristics is true. Also handle
  // the case where the use of the AllocInst could directly feed a lifetime
  // intrinsic, as will be the case when opaque pointers are enabled.
  //
  // Ex:
  //  %31 = alloca i32, align 4
  //  store i32 %30, i32* %31, align 4
  //  %32 = alloca double*, align 8
  //  %33 = bitcast double** %32 to i8*
  //  call void @llvm.lifetime.start.p0i8(i64 8, i8* %33)
  //  store double* %29, double** %32, align 8, !tbaa !6
  //  call void @foo(i32* nonnull %31, i32* undef, double** nonnull %32, i64 0)
  //  %34 = alloca i32, align 4
  //  store i32 4, i32* %34, align 4
  //  %35 = load i32, i32* %34, align 4
  //
  auto IsSimpleAlloca = [](AllocaInst &I) {
    Type *Ty = I.getAllocatedType();
    if (!Ty->isPointerTy() && !Ty->isIntegerTy())
      return false;
    if (I.isArrayAllocation())
      return false;
    if (vpo::VPOAnalysisUtils::seenOnJumpToEndIfClause(&I))
      return true;
    StoreInst *SingleStore = nullptr;
    bool CallOrLoadSeen = false;
    for (User *U : I.users()) {
      if (auto *SI = dyn_cast<StoreInst>(U)) {
        if (SingleStore)
          return false;
        if (SI->getPointerOperand() != &I)
          return false;
        SingleStore = SI;
      } else if (auto BC = dyn_cast<BitCastInst>(U)) {
        // Ignore if bitcast instruction is used by lifetime_start or
        // lifetime_end.
        for (auto UU : BC->users()) {
          auto ICB = dyn_cast<CallBase>(UU);
          if (!ICB || !ICB->isLifetimeStartOrEnd())
            return false;
        }
      } else if (auto CB = dyn_cast<CallBase>(U)) {
        if (!CB->isLifetimeStartOrEnd())
          CallOrLoadSeen = true;
      } else if (DTransInlineHeuristics && isa<LoadInst>(U)) {
        CallOrLoadSeen = true;
      } else {
        return false;
      }
    }
    return CallOrLoadSeen && SingleStore;
  };

  // Return 'true' if 'I' appears in a OMP_SIMD directive
  auto IsInOmpSimd = [](AllocaInst &I) -> bool {
    if (!vpo::VPOAnalysisUtils::mayHaveOpenmpDirective(*I.getFunction()))
      return false;
    for (User *U : I.users()) {
      auto II = dyn_cast<IntrinsicInst>(U);
      if (II && vpo::VPOAnalysisUtils::getDirectiveID(II) == DIR_OMP_SIMD)
        return true;
    }
    return false;
  };

  // Return 'true' if 'I' is in a BasicBlock which will be an entry block after
  // OpenMP outlining. There should be no penalty for Allocas in such blocks.
  auto IsInFirstOMPOutlineBlock = [](AllocaInst &I) -> bool {
    BasicBlock *BB = I.getParent()->getSinglePredecessor();
    if (!BB)
      return false;
    Instruction *II = BB->getTerminator()->getPrevNonDebugInstruction();
    if (!II)
      return false;
    if (!VPOAnalysisUtils::isRegionDirective(II))
      return false;
    StringRef DirString = VPOAnalysisUtils::getDirectiveString(II);
    if (!VPOAnalysisUtils::isOpenMPDirective(DirString))
      return false;
    if (!VPOAnalysisUtils::isBeginDirectiveOfRegionsNeedingOutlining(DirString))
      return false;
    return true;
  };

  // CMPLRLLVM-21826: Ignore AllocaInsts that appear in an OMP_SIMD directive
  if (IsInOmpSimd(I))
    return true;

  // In Fortran, dynamic allocas can be used to represent local arrays
  // allocated on the stack. We don't want to inhibiting inlining under
  // special circumstances. These include when we are running DTrans and
  // when we are compiling with -O3 -flto -xCORE-AVX2 on the link step.
  if (DTransInlineHeuristics ||
      passesMinimalSmallAppConditions(
          CandidateCall, CalleeTTI, WPI, Params.LinkForLTO.value_or(false),
          Params.InlineOptLevel.value_or(false))) {
    for (User *U : I.users())
      if (isa<SubscriptInst>(U))
        return true;
    if (IsInFirstOMPOutlineBlock(I))
      return true;
    if (IsSimpleAlloca(I))
      return true;
    // Tolerate a dynamic alloca representing a character array. This can
    // be generalized if we find it useful.
    if (DTransInlineHeuristics)
       if (auto ATy = dyn_cast<ArrayType>(I.getAllocatedType()))
         if (ATy->getElementType()->isIntegerTy(8) &&
             ATy->getNumElements() <= DynAllocaMaxCharArraySize)
           return true;
  }
  return false;
}

//
// Functions that implement intelWorthNotInlining()
//

static bool preferCloningToInlining(CallBase &CB, InliningLoopInfoCache &ILIC,
                                    bool PrepareForLTO) {
  // We don't need to check this if !PrepareForLTO because cloning after
  // inlining is only generic cloning, not cloning with specialization.
  if (!PrepareForLTO)
    return false;
  Function *Callee = CB.getCalledFunction();
  if (!Callee)
    return false;
  LoopInfo *LI = ILIC.getLI(Callee);
  if (!LI)
    return false;
  if (llvm::llvm_cloning_analysis::isCallCandidateForSpecialization(CB, LI))
    return true;
  return false;
}

static bool isLeafFunction(const Function &F) {

  for (const auto &I : instructions(&F)) {
    if (isa<InvokeInst>(&I))
      return false;

    if (auto CI = dyn_cast<CallInst>(&I)) {
      Function *Callee = CI->getCalledFunction();
      if (!Callee || !Callee->isIntrinsic())
        return false;
    }
  }

  return true;
}

// Check: expect the given Function
// - has 2 consecutive AND instructions;
//   and
// - each AND instruction has a value 0x07 on ones of its operands;
//
static bool has2SubInstWithValInF(Function *F, const Value *Val0,
                                  const Value *Val1) {
  for (auto &I : instructions(*F)) {
    //
    // CMPLRLLVM-20190: Skip debug info intrinsics when doing the matching
    //
    if (isa<DbgInfoIntrinsic>(&I))
      continue;
    Instruction *NextI = I.getNextNonDebugInstruction();
    if (!NextI)
      continue;

    if ((!isa<BinaryOperator>(&I)) || (!isa<BinaryOperator>(NextI)))
      continue;

    BinaryOperator *BOp = dyn_cast<BinaryOperator>(&I);
    BinaryOperator *NextBOp = dyn_cast<BinaryOperator>(NextI);

    // Check: both are bit-wise AND instructions
    if (BOp->getOpcode() != Instruction::And)
      continue;
    if (NextBOp->getOpcode() != Instruction::And)
      continue;

    // Check: each has the provided value on its operand
    bool BCond = (BOp->getOperand(0) == Val0) || (BOp->getOperand(1) == Val0);
    if (!BCond)
      continue;

    BCond =
        (NextBOp->getOperand(0) == Val1) || (NextBOp->getOperand(1) == Val1);
    if (!BCond)
      continue;

    // Good if both conditions fit:
    return true;
  }

  return false;
}

// Function checkVToArg() is overloaded with different argument lists
static bool checkVToArg(Value *, SmallPtrSetImpl<Value *> &);

// Check that at least 1 of a given BinaryOperator's operand is either an
// Argument, or can be traced to either an Argument.
static bool checkVToArg(BinaryOperator *BO,
                        SmallPtrSetImpl<Value *> &ValPtrSet) {
  assert(BO && "Expect a valid BinaryOperator *\n");

  for (unsigned I = 0, E = BO->getNumOperands(); I < E; ++I)
    if (checkVToArg(BO->getOperand(I), ValPtrSet))
      return true;

  return false;
}

// Check that at least 1 of PHINode's incoming value is either an Argument, or
// can be traced to an Argument.
static bool checkVToArg(PHINode *Phi, SmallPtrSetImpl<Value *> &ValPtrSet) {
  assert(Phi && "Expect a valid PHINode *\n");

  for (unsigned I = 0, E = Phi->getNumIncomingValues(); I < E; ++I)
    if (checkVToArg(Phi->getIncomingValue(I), ValPtrSet))
      return true;

  return false;
}

// Expect V be an Argument, or can ultimately trace back to an Argument.
//
// Support the following types only:
// - Argument
// - BinaryOperation
// - PHINode
// - CastInst
// - SelectInst
//
static bool checkVToArg(Value *V, SmallPtrSetImpl<Value *> &ValPtrSet) {
  assert(V && "Expect a valid Value *\n");

  if (isa<Argument>(V))
    return true;

  if (BinaryOperator *BO = dyn_cast<BinaryOperator>(V))
    return checkVToArg(BO, ValPtrSet);

  if (CastInst *CI = dyn_cast<CastInst>(V))
    return checkVToArg(CI->getOperand(0), ValPtrSet);

  // SelectInst: expect at least 1 operand to converge
  if (SelectInst *SI = dyn_cast<SelectInst>(V))
    return checkVToArg(SI->getTrueValue(), ValPtrSet) ||
           checkVToArg(SI->getFalseValue(), ValPtrSet);

  // PHINode: check current Phi form any potential cycle
  if (PHINode *Phi = dyn_cast<PHINode>(V))
    if (ValPtrSet.find(V) == ValPtrSet.end()) {
      ValPtrSet.insert(V);
      return checkVToArg(Phi, ValPtrSet);
    }

  return false;
}

//
// Return loop bottom-test
//
static ICmpInst *getLoopBottomTest(Loop *L) {
  auto EB = L->getExitingBlock();
  if (EB == nullptr)
    return nullptr;
  auto BI = dyn_cast<BranchInst>(EB->getTerminator());
  if (!BI || !BI->isConditional())
    return nullptr;
  auto ICmp = dyn_cast_or_null<ICmpInst>(BI->getCondition());

  return ICmp;
}

// Check that in a given loop (L):
// - BottomTest (CmpInst) exists and is a supported predicate;
// - CmpInst has only 2 operands;
// - UpperBound (UB) ultimately refers to one of F's formal;
//
static bool checkLoop(Loop *L) {
  assert(L && "Expect a valid Loop *\n");

  ICmpInst *CInst = getLoopBottomTest(L);
  if (!CInst)
    return false;

  if (!CInst->isIntPredicate())
    return false;

  if (CInst->getNumOperands() != 2)
    return false;

  // Check: comparison is <, <= or ==.
  // (since the loop has not been normalized yet)
  ICmpInst::Predicate Pred = CInst->getPredicate();
  if (!(Pred == ICmpInst::ICMP_EQ || Pred == ICmpInst::ICMP_ULT ||
        Pred == ICmpInst::ICMP_ULE || Pred == ICmpInst::ICMP_SLT ||
        Pred == ICmpInst::ICMP_SLE))
    return false;

  // Check: the loop's UpperBound traces to a formal Argument
  llvm::SmallPtrSet<Value *, 16> ValPtrSet;
  if (!checkVToArg(CInst->getOperand(1), ValPtrSet))
    return false;

  return true;
}

// Check: expect the given Function
// - has at least 1 loop nest
// - count the cases where the loop's upper bound (UB) traces back to one of
//   arguments for the given function;
//   The total count matches the expected count;
//
static bool checkLoopUBMatchArgInF(InliningLoopInfoCache &ILIC, Function *F,
                                   const unsigned ExpectedUBInArgs) {
  // Only consider candidates that have loops, otherwise there is nothing
  // to multiversion.
  LoopInfo *LI = ILIC.getLI(F);
  if (!LI || LI->empty())
    return false;
  auto Loops = LI->getLoopsInPreorder();
  unsigned Count = 0;

  // Check + Count each loop:
  for (auto L : Loops)
    if (checkLoop(L))
      ++Count;

  return (Count == ExpectedUBInArgs);
}

//
// Starts with Value *V and traces back through a series of at least
// 'MinBOpCount' but no more than 'MaxBOpCount' BinaryOperators which must
// have the form:
//   BinaryOperator W, constant or BinaryOperator constant, W
// to get to a load of a GlobalValue.  If such a GlobalValue is found, we
// return it, otherwise we return 'nullptr'.
//
static GlobalValue *traceBack(Value *V, unsigned MinBOpCount,
                              unsigned MaxBOpCount) {
  auto W = V;
  auto BOp = dyn_cast<BinaryOperator>(W);
  unsigned BOpCount = 0;
  while (BOp) {
    if (++BOpCount > MaxBOpCount)
      return nullptr;
    if (dyn_cast<ConstantInt>(BOp->getOperand(0))) {
      W = BOp->getOperand(1);
    } else if (dyn_cast<ConstantInt>(BOp->getOperand(1))) {
      W = BOp->getOperand(0);
    }
    if (!W)
      return nullptr;
    BOp = dyn_cast<BinaryOperator>(W);
  }
  if (BOpCount < MinBOpCount)
    return nullptr;
  auto LI = dyn_cast<LoadInst>(W);
  if (!LI)
    return nullptr;
  auto GV = dyn_cast<GlobalValue>(LI->getPointerOperand());
  return GV;
}

//
// Return 'true' if 'CB' should not be inlined, because it would be better
// to multiversion it. 'PrepareForLTO' is true if we are on the compile step
// of an LTO compilation.
//
static bool preferMultiversioningToInlining(
    CallBase &CB, const TargetTransformInfo &CalleeTTI,
    InliningLoopInfoCache &ILIC, bool PrepareForLTO) {

  // Right now, only the callee is tested for multiversioning.  We use
  // this set to keep track of callees that have already been tested,
  // to save compile time. We expect it to be relatively expensive to
  // test the qualifying candidates. Those that do not qualify should
  // be rejected quickly.
  static SmallPtrSet<Function *, 3> CalleeFxnPtrSet;

  // Only focus on Link time at present:
  if (PrepareForLTO)
    return false;

  // Quick tests:
  if (!DTransInlineHeuristics)
    return false;
  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX2;
  if (!CalleeTTI.isAdvancedOptEnabled(TTIAVX2))
    return false;
  Function *Callee = CB.getCalledFunction();
  if (!Callee)
    return false;
  if (CalleeFxnPtrSet.find(Callee) != CalleeFxnPtrSet.end())
    return true;

  // Check: expect Callee be a leaf function
  if (!isLeafFunction(*Callee))
    return false;

  // Check: there are 2 consecutive AND instructions, and each AND instruction
  // has a value 0x07 on its operand
  llvm::Type *I32Ty = llvm::IntegerType::getInt32Ty(Callee->getContext());
  llvm::Constant *I32Val7 = llvm::ConstantInt::get(I32Ty, 7 /*value*/, true);
  if (!has2SubInstWithValInF(Callee, I32Val7, I32Val7))
    return false;

  // Check: there are 2 loops in the given function,
  // and each Loop's UB can trace to a formal Argument.
  if (!checkLoopUBMatchArgInF(ILIC, Callee, 2))
    return false;

  CalleeFxnPtrSet.insert(Callee);
  return true;
}

//
//
// Return 'true' if 'CB' should not be inlined, because it would be better
// to perform SOAToAOS on it. 'PrepareForLTO' is true if we are on the compile
// step of an LTO compilation.
//
static bool preferDTransToInlining(CallBase &CB, bool PrepareForLTO) {
  if (!PrepareForLTO)
    return false;

  if (!DTransInlineHeuristics)
    return false;

  // The callee was marked as candidate to suppress inlining.
  if (Function *Callee = CB.getCalledFunction())
    if (Callee->hasFnAttribute("noinline-dtrans"))
      return true;

  return false;
}

// Return 'true' if 'CB' is preferred for inlining because doing so will
// enable dtrans opportunities.
static bool preferInlineDtrans(CallBase &CB) {
  return CB.hasFnAttr("prefer-inline-dtrans");
}

//
// Returns 'true' if the Function *F calls a realloc-like function, or if
// heuristically we suspect it is a realloc-like function. (In a non-LTO
// compilation, or the compile phase of an LTO, there are fewer functions with
// IR than in the link phase of an LTO compilation of the same application.
// So, a function that appears with IR in the link phase of an LTO
// compilation could be merely a declaration in the compile phase, and
// we would have no knowledge of its contents in the compile phase.)
//
static bool callsRealloc(Function *F, TargetLibraryInfo *TLI) {
  if (!F)
    return false;
  if (F->getName().contains("realloc"))
    return true;
  for (auto &I : instructions(F))
    if (const auto *const CB = dyn_cast<CallBase>(&I))
      if (getReallocatedOperand(CB, TLI))
        return true;
  return false;
}

//
// Return 'true' if Function *F should not be inlined due to its special
// manipulation of stack instructions.
//
static bool preferNotToInlineForStackComputations(Function *F,
                                                  TargetLibraryInfo *TLI) {
  //
  // Returns 'true' if the Function *F has no arguments.
  //
  auto PassesArgTest = [](Function *F) -> bool { return F->arg_size() == 0; };

  //
  // Returns 'true' if the Function *F has only one basic block.
  //
  auto PassesControlFlowTest = [](Function *F) -> bool {
    return F->size() == 1;
  };

  //
  // Returns 'true' if the Function *F passes the contents test.
  // We are looking for two stores with the forms:
  //   GlobalValue1 = BinOpTest(GlobalValue1)
  //   GlobalValue2 = BitCast(call Realloc(BitCast(GlobalValue2),
  //                                       ConstantInt * GlobalValue1)
  // where the BitCasts are optional.
  //
  auto PassesContentsTest = [](Function *F, TargetLibraryInfo *TLI) -> bool {
    unsigned StoreCount = 0;
    StoreInst *S0 = nullptr;
    for (auto &I : F->getEntryBlock()) {
      auto SI = dyn_cast<StoreInst>(&I);
      if (SI) {
        if (StoreCount == 0) {
          // Find:  GlobalValue1 = BinOpTest(GlobalValue1)
          auto GV1 = dyn_cast<GlobalValue>(SI->getPointerOperand());
          if (!GV1)
            return false;
          auto GV2 = traceBack(SI->getValueOperand(), 3, 3);
          if (GV2 != GV1)
            return false;
          S0 = SI;
          StoreCount++;
        } else if (StoreCount == 1) {
          // Find:
          //   GlobalValue2 = BitCast(call Realloc(BitCast(GlobalValue2),
          //                                       ConstantInt * GlobalValue1)
          auto SIP = SI->getPointerOperand();
          auto BCSO = dyn_cast<BitCastOperator>(SIP);
          if (BCSO)
            SIP = BCSO->getOperand(0);
          auto GV1 = dyn_cast<GlobalValue>(SIP);
          if (!GV1)
            return false;
          // Look through a possible BitCast to find the call.
          auto V = SI->getValueOperand();
          auto BCI = dyn_cast<BitCastInst>(V);
          if (BCI)
            V = BCI->getOperand(0);
          auto CI = dyn_cast<CallInst>(V);
          if (!CI)
            return false;
          // Check the call's arguments.
          if (CI->arg_size() != 2)
            return false;
          // Arg0 should be BitCast(GlobalValue2), where BitCast is optional.
          auto V0 = CI->arg_begin();
          auto LI = dyn_cast<LoadInst>(V0);
          if (!LI)
            return false;
          auto W0 = LI->getPointerOperand();
          auto BCO = dyn_cast<BitCastOperator>(W0);
          if (BCO)
            W0 = BCO->getOperand(0);
          auto GV2 = dyn_cast<GlobalValue>(W0);
          if (GV2 != GV1)
            return false;
          // Arg1 should be ConstantInt * GlobalValue1, where the multiply
          // is implemented with a shift.
          auto U = CI->arg_begin() + 1;
          auto V1 = dyn_cast<Value>(U);
          if (!V1)
            return false;
          Value *V2;
          ConstantInt *ShlC;
          if (!match(V1, m_Shl(m_Value(V2), m_ConstantInt(ShlC))))
            return false;
          auto SE = dyn_cast<SExtInst>(V2);
          if (SE)
            V2 = SE->getOperand(0);
          if (V2 != S0->getValueOperand())
            return false;
          // The call should call realloc, or at least be something we
          // suspect calls realloc.
          if (!callsRealloc(CI->getCalledFunction(), TLI))
            return false;
          StoreCount++;
        } else
          // A third store is a deal breaker.
          return false;
      }
    }
    return StoreCount == 2;
  };

  //
  // Use 'WorthyFunction' to store the single worthy Function if found.
  // Use SmallPtrSet to store those Functions that have already been tested
  // and have failed the test, so we don't need to test them again.
  //
  static Function *WorthyFunction = nullptr;
  static SmallPtrSet<Function *, 32> FunctionsTestedFail;
  if (!F || !TLI || !DTransInlineHeuristics)
    return false;
  if (WorthyFunction)
    return WorthyFunction == F;
  if (FunctionsTestedFail.count(F))
    return false;
  if (!PassesArgTest(F) || !PassesControlFlowTest(F) ||
      !PassesContentsTest(F, TLI)) {
    FunctionsTestedFail.insert(F);
    return false;
  }
  WorthyFunction = F;
  return true;
}

//
// Return 'true' if CallBase CB should not be inlined due to its special
// manipulation of stack instructions. (Note that inhibit calls into
// CB.getCaller() so that the worthy function looks relatively similar in
// both the compile and link step.
//
static bool preferNotToInlineForStackComputations(CallBase &CB,
                                                  TargetLibraryInfo *TLI) {
  return preferNotToInlineForStackComputations(CB.getCaller(), TLI) ||
         preferNotToInlineForStackComputations(CB.getCalledFunction(), TLI);
}

//
// Return 'true' if the CallBase CB should not be inlined due to having
// a special type of switch statement. (Note: this heuristic uses info
// only from the Caller, and not from the Callee of CB.)
//
static bool
preferNotToInlineForSwitchComputations(CallBase &CB,
                                       InliningLoopInfoCache &ILIC) {
  //
  // Return 'true' if the called function of the Callsite CB is a small
  // function whose basic blocks that end in a ReturnInst return the
  // result of an indirect call.
  //
  auto WorthySwitchCallSite = [](CallBase &CB) -> bool {
    auto Callee = CB.getCalledFunction();
    // Must have the IR for the callee.
    if (!Callee || Callee->isDeclaration())
      return false;
    // Callee must be small (need to limit the compile time).
    if (Callee->size() > 3)
      return false;
    unsigned ReturnCount = 0;
    for (auto &BB : *Callee) {
      auto RI = dyn_cast<ReturnInst>(BB.getTerminator());
      if (!RI)
        continue;
      ReturnCount++;
      auto V = RI->getReturnValue();
      if (!V)
        return false;
      auto ICI = dyn_cast<CallInst>(V);
      if (!ICI)
        return false;
      // Return 'true; if this is an indirect call.
      if (ICI->getCalledFunction())
        return false;
    }
    return ReturnCount > 0;
  };

  auto PreferNotToInlineCaller =
      [&WorthySwitchCallSite](Function *Caller,
                              InliningLoopInfoCache &ILIC) -> bool {
    // Limit this to Callers with a sufficiently large number of formal
    // arguments.
    if (Caller->arg_size() > 3)
      return false;
    // Look for a switch statement at the end of the entry block with a
    // sufficiently large number of cases.
    BasicBlock *EntryBlock = &(Caller->getEntryBlock());
    Instruction *TI = EntryBlock->getTerminator();
    auto SI = dyn_cast<SwitchInst>(TI);
    if (!SI)
      return false;
    if (SI->getNumCases() < MinSwitchCases)
      return false;
    auto Cond = SI->getCondition();
    // The switch statement should get its value from a call to a Function
    // that makes an indirect call to get its result.
    auto CI = dyn_cast<CallInst>(Cond);
    if (!CI)
      return false;
    if (!WorthySwitchCallSite(*CI))
      return false;
    // The actual arguments to the call should come from the formal arguments
    // to the caller, passed optionally through an all zero index GEP.
    for (unsigned I = 0; I < CI->arg_size(); I++) {
      auto W = CI->getArgOperand(I);
      auto GEPInst = dyn_cast<GetElementPtrInst>(W);
      if (GEPInst) {
        if (!GEPInst->hasAllZeroIndices())
          return false;
        W = GEPInst->getPointerOperand();
      }
      auto Arg = dyn_cast<Argument>(W);
      if (!Arg)
        return false;
    }
    // There should be a single join point that all of the switch cases
    // branch to.  The rest of the code in the caller should be covered
    // by the switch statements targets.
    unsigned Count = 0;
    auto DT = ILIC.getDT(Caller);
    for (auto &&DTN : DT->getNode(EntryBlock)->children()) {
      auto BB = DTN->getBlock();
      if (BB->getUniquePredecessor() == EntryBlock)
        continue;
      if (++Count > 1)
        return false;
      if (!dyn_cast<ReturnInst>(BB->getTerminator()))
        return false;
    }
    if (Count != 1)
      return false;
    // Reject any caller that has an invoke instruction.
    for (auto &I : instructions(Caller))
      if (isa<InvokeInst>(&I))
        return false;
    return true;
  };
  //
  // Use 'WorthyFunction' to store the single worthy Function if found.
  // Use SmallPtrSet to store those Functions that have already been tested
  // and have failed the test, so we don't need to test them again.
  //
  static Function *WorthyFunction = nullptr;
  static SmallPtrSet<Function *, 32> FunctionsTestedFail;
  Function *Caller = CB.getCaller();
  if (!DTransInlineHeuristics)
    return false;
  if (WorthyFunction == Caller)
    return true;
  if (FunctionsTestedFail.count(Caller))
    return false;
  // The first worthy function we find is the only candidate.
  if (!PreferNotToInlineCaller(Caller, ILIC)) {
    FunctionsTestedFail.insert(Caller);
    return false;
  }
  WorthyFunction = Caller;
  return true;
}

//
// Return 'true' if 'Callee' is preferred for not inlining because it is
// a recursive progression clone marked with an attribute as being
// preferred for not inlining ("prefer-noinline-rec-pro-clone").
//
static bool preferNotToInlineForRecProgressionClone(Function *Callee) {
  return Callee && Callee->hasFnAttribute("prefer-noinline-rec-pro-clone");
}

//
// Return 'true' if the CallSites of 'Caller' should not be inlined because
// we are in the 'PrepareForLTO' compile phase and delaying the inlining
// decision until the link phase will let us more easily decide whether to
// inline the caller. NOTE: This function 'preferToDelayInlineDecision' can
// also be called in the link phase.  If it is, we determine whether the
// caller should be inlined, and if so, add it on the 'QueuedCallers' set.
//
static bool
preferToDelayInlineDecision(Function *Caller, bool PrepareForLTO,
                            SmallPtrSetImpl<Function *> *QueuedCallers) {

  // Try to return a pointer to a GEP instruction which tests if a structure
  // field value is greater than 0 at the end of the Caller's entry
  // BasicBlock. If there is no such pointer, return nullptr.
  auto TestedGEP = [](Function *Caller) -> GetElementPtrInst * {
    auto EntryBlock = &(Caller->getEntryBlock());
    if (EntryBlock->size() > 5)
      return nullptr;
    auto BI = dyn_cast<BranchInst>(EntryBlock->getTerminator());
    if (!BI || BI->isUnconditional())
      return nullptr;
    auto ICI = dyn_cast<ICmpInst>(BI->getCondition());
    if (!ICI || ICI->getPredicate() != ICmpInst::ICMP_SGT)
      return nullptr;
    auto Zero = dyn_cast<ConstantInt>(ICI->getOperand(1));
    if (!Zero || !Zero->isZero())
      return nullptr;
    auto LI = dyn_cast<LoadInst>(ICI->getOperand(0));
    if (!LI)
      return nullptr;
    auto GEP = dyn_cast<GetElementPtrInst>(LI->getPointerOperand());
    if (!GEP)
      return nullptr;
    if (!isa<Argument>(GEP->getPointerOperand()))
      return nullptr;
    return GEP;
  };

  // In the 'PrepareForLTO' phase, return 'true' if the 'Caller' should not
  // have any of its callsites inlined until the link phase, because inlining
  // them will make it more difficult to determine whether that Caller should
  // be inlined during the link phase.  Then, in the link phase, return 'true'
  // if the 'Caller' should be inlined.
  auto IsDelayInlineCaller = [&TestedGEP](Function *Caller,
                                          bool PrepareForLTO) -> bool {
    if (!TestedGEP(Caller))
      return false;
    unsigned MaxSize = PrepareForLTO ? 5 : 3;
    if (Caller->size() > MaxSize)
      return false;
    auto EntryBlock = &(Caller->getEntryBlock());
    for (auto &BB : *Caller) {
      if (&BB == EntryBlock)
        continue;
      auto TI = BB.getTerminator();
      auto RI = dyn_cast<ReturnInst>(TI);
      if (RI) {
        if (!RI->getReturnValue())
          return false;
        if (BB.size() > 2)
          return false;
        continue;
      }
      auto BI = dyn_cast<BranchInst>(TI);
      if (!BI)
        return false;
      if (BI->isUnconditional())
        continue;
      if (!PrepareForLTO)
        return false;
      auto II = dyn_cast<IntrinsicInst>(BI->getCondition());
      if (!II)
        return false;
      if (II->getIntrinsicID() != Intrinsic::intel_wholeprogramsafe)
        return false;
    }
    return true;
  };

  if (!DTransInlineHeuristics)
    return false;
  if (IsDelayInlineCaller(Caller, PrepareForLTO)) {
    if (!PrepareForLTO)
      QueuedCallers->insert(Caller);
    return true;
  }
  return false;
}

// Heuristic to disable inlining of routines that move elements inside
// an array of structures. This heuristic is used only when PrepareForLTO is
// true.
//
// Ex:
//   foo(struct_a *A1, A2, A3, A4, A5, A6, A7) {
//     // Argument assignments
//     A1->field2 = A2;
//     A1->field4 = A7;
//     ...
//     for() {
//       // Copy elements inside array
//       A1[i].field1 = A1[j].field1;
//       A1[i].field2 = A1[j].field2;
//       A1[i].field3 = A1[j].field3;
//       A1[i].field4 = A1[j].field3;
//       A1[i].field5 = A1[j].field5;
//       A1[i].field6 = A1[j].field6;
//       ...
//     }
//   }
//
// Checks the following heuristics.
// 1. Callee should have at least 'CopyArrElemsMinimumArgs' arguments.
// 2. Type of first argument should be "pointer to struct that should have at
// least 'CopyArrElemsMinimumFields' fields"
// 3. Call should be in loop with minimum depth 'CopyArrElemsMinimumLoopDepth'.
// Callee should have only one loop.
// 4. In loop, copy at least 'CopyArrElemsMinimumFields' struct fields from one
// element to another element of first argument array.
// 5. Allows argument assignments to struct fields of array elements.
// 6. Except Store instructions in 4 and 5, no other instructions that can
// "write to memory" are allowed.

// Minimum limit:  Number of callee arguments.
constexpr static unsigned CopyArrElemsMinimumArgs = 7;

// Minimum limit: loop depth of Caller's BB.
constexpr static unsigned CopyArrElemsMinimumLoopDepth = 2;

// Minimum Limit: Number of fields of array element struct.
constexpr static unsigned CopyArrElemsMinimumFields = 6;

static bool preferToDelayInlineForCopyArrElems(CallBase &CB, bool PrepareForLTO,
                                               InliningLoopInfoCache &ILIC) {

  // Returns true if type of the first argument of "F" is pointer to struct.
  // Gets type of the argument from its uses. Returns false if use of
  // the argument is any instruction other than GetElementPtrInst.
  // This routine assumes "F" has at least one argument.
  auto IsFirstArgPtrToStruct = [](Function *F) {
    bool ArgUsedInGEP = false;
    Argument *Arg = F->getArg(0);
    for (User *V : Arg->users()) {
      auto GEP = dyn_cast<GetElementPtrInst>(V);
      if (!GEP || GEP->getPointerOperand() != Arg)
        return false;
      auto *STy = dyn_cast<StructType>(GEP->getSourceElementType());
      if (!STy)
        return false;
      if (STy->getNumElements() <= CopyArrElemsMinimumFields)
        return false;
      ArgUsedInGEP = true;
    }
    return ArgUsedInGEP;
  };

  // Returns true if "V" is address of an element in first argument array
  // of "F".
  // Ex:
  //    foo(struct_a *A1, ...) {
  //      ...
  //      (A1)  // Allowed pattern: address of 1st element
  //      ...
  //      (A1 + some_idx) // Allowed pattern: address of "some_idx + 1"st
  //                         element.
  //      ...
  //    }

  auto IsElementInFirstArgArray = [](Value *V, Function *F) {
    if (auto *GEP = dyn_cast<GetElementPtrInst>(V)) {
      if (GEP->getNumOperands() != 2)
        return false;
      if (!isa<StructType>(GEP->getSourceElementType()))
        return false;
      V = GEP->getOperand(0);
    }
    if (V != F->getArg(0))
      return false;
    return true;
  };

  // Returns true if "V" is field access of struct element in first argument
  // array. "Idx" is set with the field index that is accessed when it returns
  // true.
  // Ex:
  //   foo(struct_a *A1, ...) {
  //     ...
  //     (A1 + some_idx)->field5 // Allowed pattern
  //     ...
  //   }
  auto IsStructFieldInFirstArgArrayElem =
      [&IsElementInFirstArgArray](Value *V, unsigned &Idx) {
        if (auto *BC = dyn_cast<BitCastInst>(V))
          V = BC->getOperand(0);
        auto *GEP = dyn_cast<GetElementPtrInst>(V);
        if (!GEP || GEP->getNumOperands() != 3)
          return false;
        if (!IsElementInFirstArgArray(GEP->getOperand(0), GEP->getFunction()))
          return false;
        if (!isa<StructType>(GEP->getSourceElementType()))
          return false;
        auto *FIdx = dyn_cast<ConstantInt>(GEP->getOperand(2));
        if (!FIdx)
          return false;
        Idx = FIdx->getLimitedValue();
        return true;
      };

  // Returns true if "V" is load of struct element field.
  auto IsStructFieldLoad = [&IsStructFieldInFirstArgArrayElem](Value *V) {
    auto *LI = dyn_cast<LoadInst>(V);
    if (!LI)
      return false;
    unsigned LdIdx;
    if (!IsStructFieldInFirstArgArrayElem(LI->getPointerOperand(), LdIdx))
      return false;
    return true;
  };

  // Returns true if "I" is struct element field assignment.
  //  Ex:
  //  foo(struct_a *A1, A2, ..) {
  //    ...
  //    (A1 + some_idx)->field5 = A2;  // Pattern 1
  //    ...
  //    Loop:
  //      (A1 + some_idx)->field5 = (A1 + some_idx)->field4;  // Pattern 2
  //      ...
  //    goto Loop:
  //  }
  //
  // Pattern 2 stores are allowed only in loop. Field offsets of all pattern 2
  // stores are collected in "CopiedStructOffsets".

  auto IsStructFieldCopy =
      [&IsStructFieldInFirstArgArrayElem,
       &IsStructFieldLoad](Instruction &I, LoopInfo *LoopI,
                           SetVector<unsigned> &CopiedStructOffsets) {
        auto *SI = dyn_cast<StoreInst>(&I);
        if (!SI)
          return false;
        unsigned StIdx;
        if (!IsStructFieldInFirstArgArrayElem(SI->getPointerOperand(), StIdx))
          return false;
        Value *StoredVal = SI->getValueOperand();
        if (auto *BC = dyn_cast<BitCastInst>(StoredVal))
          StoredVal = BC->getOperand(0);
        else if (auto *TI = dyn_cast<TruncInst>(StoredVal))
          StoredVal = TI->getOperand(0);
        // Ok if StoredVal is just argument.
        if (isa<Argument>(StoredVal))
          return true;
        // Check for pattern 2 stores.
        if (!IsStructFieldLoad(StoredVal))
          return false;
        Loop *L = LoopI->getLoopFor(SI->getParent());
        if (!L)
          return false;
        if (!CopiedStructOffsets.insert(StIdx))
          return false;
        return true;
      };

  if (!PrepareForLTO || !DTransInlineHeuristics)
    return false;
  auto *Callee = CB.getCalledFunction();
  // Must have the IR for the callee.
  if (!Callee || Callee->isDeclaration())
    return false;
  if (Callee->arg_size() < CopyArrElemsMinimumArgs)
    return false;
  if (!IsFirstArgPtrToStruct(Callee))
    return false;
  // Callee should have one loop.
  LoopInfo *CalleeLI = ILIC.getLI(Callee);
  if (CalleeLI->size() != 1)
    return false;

  LoopInfo *CallerLI = ILIC.getLI(CB.getCaller());
  if (CallerLI->empty())
    return false;
  Loop *InnerLoop = CallerLI->getLoopFor(CB.getParent());
  if (!InnerLoop || InnerLoop->getLoopDepth() < CopyArrElemsMinimumLoopDepth)
    return false;

  // This is used to collect all fields that are stored in loop using
  // pattern 2 store.
  SetVector<unsigned> CopiedStructOffsets;
  for (auto &I : instructions(*Callee)) {
    if (isa<DbgInfoIntrinsic>(I))
      continue;
    if (IsStructFieldCopy(I, CalleeLI, CopiedStructOffsets))
      continue;
    if (I.mayWriteToMemory())
      return false;
  }
  // Make sure minimum number of fields is copied.
  if (CopiedStructOffsets.size() < CopyArrElemsMinimumFields)
    return false;
  return true;
}

//
// Return 'true' if 'Callee' is a function that is preferred to be
// partially inlined over fully inlined. This function won't be inlined
// since it is called from the function that is marked as
// "prefer-partial-inline-inlined-clone". For example:
//
// define i1 @foo(%"struct.pov::Object_Struct"*) {
//  %3 = icmp eq %"struct.pov::Object_Struct"* %0, null
//  br i1 %3, label %12, label %4
//
// ; <label>:4:
//  %5 = phi %"struct.pov::Object_Struct"* [ %9, %4 ], [ %0, %2 ]
//  %6 = Some computation
//  ...
//  br i1 %11, label %4, label %12
//
// ; <label>:12:
//  %13 = phi i1 [ true, %2 ], [ %6, %4 ]
//  ret i1 %14
// }
//
// The Intel partial inliner will create the following functions:
//
// define i1 @foo.1(%"struct::Object_Struct"*) {
// ; <label>:2:
//  %phitmp.loc = alloca i1
//  %3 = icmp eq %"struct.pov::Object_Struct"* %0, null
//  br i1 %3, label %6, label %4
//
// ; <label>:4:
//  call void @foo.1.for.body(%"struct.pov::Object_Struct"* %0, i1*phitmp.loc)
//  %phitmp.reload = load i1, i1* %phitmp.loc
//  br label %6
//
// ; <label>:6:
//  %7 = phi i1 [ true, %2 ], [ %phitmp.reload, %4 ]
//  ret i1 %7
// }
//
// define void @foo.1.for.body(%"struct::Object_Struct"*, i1* %phitmp.out) {
// newFuncRoot:
//  br label %for.body
//
// for.body.for.end_crit_edge.exitStub:
//  ret void
//
// for.body:
//  %5 = phi %"struct.pov::Object_Struct"* [ %9, %for.body ],
//                                         [ 0, %newFuncRoot ]
//  %6 = Some computation
//  ...
// %cmp = icmp eq %"struct.pov::Object_Struct"* %Var, null
//   br i1 %cmp, label %for.body.for.end_crit_edge.exitStub, label %for.body
// }
//
// All the calls to @foo will be replaced with @foo.1. The partial inliner will
// set the attribute 'prefer-partial-inline-outlined-func' on @foo.1.for.body
// and 'prefer-partial-inline-inlined-clone' on @foo.1. The inliner will fully
// inline @foo.1 and won't inline @foo.1.for.body. This creates the partial
// inlining of @foo based on the argument %1.
//
// In the example mentioned above, this function will return true for
// @foo.1.for.body since it is the function marked as
// 'prefer-partial-inline-outlined-func'.
//
static bool preferPartialInlineOutlinedFunc(Function *Callee) {
  return Callee &&
         Callee->hasFnAttribute("prefer-partial-inline-outlined-func");
}

static bool preferToIntelPartialInline(Function &F, bool PrepareForLTO,
                                       InliningLoopInfoCache &ILIC) {
  if (!PrepareForLTO || !DTransInlineHeuristics)
    return false;
  LoopInfoFuncType GetLoopInfo = [&ILIC](Function &F) -> LoopInfo & {
    return *(ILIC.getLI(&F));
  };
  return isIntelPartialInlineCandidate(&F, GetLoopInfo, PrepareForLTO);
}

//
// Return 'true' if 'I' is an instruction related to exception handling that
// will inhibit loop optimizations.
//
static bool isLoopOptInhibitingEHInst(const Instruction &I) {
  return isa<LandingPadInst>(&I) || isa<FuncletPadInst>(&I) ||
         isa<InvokeInst>(&I) || isa<ResumeInst>(&I) ||
         isa<CatchSwitchInst>(&I) || isa<CatchReturnInst>(&I) ||
         isa<CleanupReturnInst>(I);
}

//
// Return 'true' if 'CSI' is an instruction (representing a call/invoke) in a
// loop which has no instructions that will inhibit loop optimizations.
//
static bool isInNonEHLoop(Instruction &CSI, InliningLoopInfoCache &ILIC) {
  LoopInfo *LI = ILIC.getLI(CSI.getFunction());
  Loop *Loop = LI->getLoopFor(CSI.getParent());
  if (!Loop)
    return false;
  for (const BasicBlock *BB : Loop->blocks())
    for (const Instruction &I : *BB)
      if (isLoopOptInhibitingEHInst(I))
        return false;
  return true;
}

//
// Return 'true' if 'F' has a loop optimization inhibiting exception handling
// related instruction outside of a loop.
//
static bool hasLoopOptInhibitingEHInstOutsideLoop(Function *F,
                                                  InliningLoopInfoCache &ILIC) {
  if (!F)
    return false;
  LoopInfo *LI = ILIC.getLI(F);
  for (BasicBlock &BB : *F)
    if (!LI->getLoopFor(&BB))
      for (Instruction &I : BB)
        if (isLoopOptInhibitingEHInst(I))
          return true;
  return false;
}

//
// Return 'true' if 'CB' should not be inlined because it contains exception
// handling code and that will inhibit the application of loop optimizations
// to the loop in which 'CB' appears.
//
static bool preferNotToInlineEHIntoLoop(CallBase &CB,
                                        InliningLoopInfoCache &ILIC) {
  if (!DTransInlineHeuristics)
    return false;
  Function *Callee = CB.getCalledFunction();
  // Return false if CB is marked as prefer-inline-dtrans
  if (preferInlineDtrans(CB))
    return false;
  if (Callee->hasFnAttribute(Attribute::NoUnwind))
    return false;
  if (!isInNonEHLoop(CB, ILIC))
    return false;
  return hasLoopOptInhibitingEHInstOutsideLoop(Callee, ILIC);
}

extern Optional<InlineResult> intelWorthNotInlining(
    CallBase &CandidateCall, const InlineParams &Params, TargetLibraryInfo *TLI,
    const TargetTransformInfo &CalleeTTI, ProfileSummaryInfo *PSI,
    InliningLoopInfoCache *ILIC, SmallPtrSetImpl<Function *> *QueuedCallers,
    InlineReasonVector &NoReasonVector) {
  bool PrepareForLTO = Params.PrepareForLTO.value_or(false);
  Function *Callee = CandidateCall.getCalledFunction();
  if (!Callee || !InlineForXmain)
    return None;
  Optional<uint64_t> ProfCount = profInstrumentCount(PSI, CandidateCall);
  if (ProfCount && ProfCount.getValue() == 0) {
    if (!Callee->hasLinkOnceODRLinkage())
      return InlineResult::failure("not profitable")
          .setIntelInlReason(NinlrColdProfile);
    NoReasonVector.push_back(NinlrColdProfile);
  }
  if (preferCloningToInlining(CandidateCall, *ILIC, PrepareForLTO))
    return InlineResult::failure("not profitable")
        .setIntelInlReason(NinlrPreferCloning);
  if (preferMultiversioningToInlining(CandidateCall, CalleeTTI, *ILIC,
                                      PrepareForLTO))
    return InlineResult::failure("not profitable")
        .setIntelInlReason(NinlrPreferMultiversioning);
  if (preferDTransToInlining(CandidateCall, PrepareForLTO))
    return InlineResult::failure("not profitable")
        .setIntelInlReason(NinlrPreferSOAToAOS);
  if (preferNotToInlineForStackComputations(CandidateCall, TLI))
    return InlineResult::failure("not profitable")
        .setIntelInlReason(NinlrStackComputations);
  if (preferNotToInlineForSwitchComputations(CandidateCall, *ILIC))
    return InlineResult::failure("not profitable")
        .setIntelInlReason(NinlrSwitchComputations);
  if (preferNotToInlineForRecProgressionClone(Callee))
    return InlineResult::failure("recursive").setIntelInlReason(NinlrRecursive);

  if (preferToDelayInlineDecision(CandidateCall.getCaller(), PrepareForLTO,
                                  QueuedCallers)) {
    if (PrepareForLTO)
      return InlineResult::failure("not profitable")
          .setIntelInlReason(NinlrDelayInlineDecision);
    else
      return InlineResult::failure("not profitable")
          .setIntelInlReason(NinlrDelayInlineDecisionLTO);
  }

  if (preferToDelayInlineForCopyArrElems(CandidateCall, PrepareForLTO, *ILIC))
    return InlineResult::failure("not profitable")
        .setIntelInlReason(NinlrDelayInlineDecision);
  if (preferPartialInlineOutlinedFunc(Callee))
    return InlineResult::failure("not profitable")
        .setIntelInlReason(NinlrPreferPartialInline);
  if (preferToIntelPartialInline(*Callee, PrepareForLTO, *ILIC))
    return InlineResult::failure("not profitable")
        .setIntelInlReason(NinlrDelayInlineDecision);
  if (preferNotToInlineEHIntoLoop(CandidateCall, *ILIC))
    return InlineResult::failure("not profitable")
        .setIntelInlReason(NinlrCalleeHasExceptionHandling);
  if (CandidateCall.getCaller() == Callee &&
      Callee->hasFnAttribute("no-more-recursive-inlining"))
    return InlineResult::failure("recursive").setIntelInlReason(NinlrRecursive);
  return None;
}

//
// Functions that implement intelWorthInlining()
//

//
// Return 'true' if the attributes indicate that 'CB' was discovered to be
// a worthy double callsite while testing another callsite.
//
static bool hasWorthyDoubleCallSiteAttribute(CallBase &CB) {
  return CB.hasFnAttr("inline-doubleext2") || CB.hasFnAttr("inline-doubleext3");
}

//
// Return true if the caller of 'CB' has exactly two callsites or if 'CB'
// is marked as a worthy double callsite.
//
static bool isDoubleCallSite(CallBase &CB) {
  unsigned int count = 0;
  if (hasWorthyDoubleCallSiteAttribute(CB))
    return true;
  Function *F = CB.getCalledFunction();
  for (User *U : F->users()) {
    auto CBB = dyn_cast<CallBase>(U);
    if (!CBB || CBB->getCalledFunction() != F)
      continue;
    if (++count > 2)
      return false;
  }
  return count == 2;
}

//
// Return 'true' if 'CB's callee has exactly two callsites in the same
// caller, and no other uses. If so, set '*CB0' and '*CB1' to those two
// callsites.
//
static bool hasTwoCallSitesInSameCaller(CallBase &CB,
                                        CallBase **CB0, CallBase **CB1) {
  // Look for 2 calls of the callee in the caller.
  unsigned Count = 0;
  Function *Caller = CB.getCaller();
  Function *Callee = CB.getCalledFunction();
  for (User *U : Callee->users())
    if (auto CBB = dyn_cast<CallBase>(U)) {
      if (CBB->getCaller() == Caller) {
        if (++Count > 2)
          return false;
        if (Count == 1)
          *CB0 = CBB;
        else
          *CB1 = CBB;
      }
    }
  if (Count != 2)
    return false;
  return true;
}

//
// Return 'true' if this is a double callsite worth inlining.
//   (This is one of multiple double callsite heuristics.)
//
// The criteria for this heuristic are:
//  (1) Must have exactly two calls to the function in the caller
//  (2) The callee must have a single outer loop
//  (3) That loop's basic blocks must have a relatively large successor count
//
static bool worthyDoubleCallSite1(CallBase &CB, InliningLoopInfoCache &ILIC) {
  CallBase *CB0 = nullptr;
  CallBase *CB1 = nullptr;
  // Look for 2 calls of the callee in the caller.
  if (!hasTwoCallSitesInSameCaller(CB, &CB0, &CB1))
    return false;
  // Look for a single top level loop in the callee.
  LoopInfo *LI = ILIC.getLI(CB.getCalledFunction());
  unsigned count = 0;
  const Loop *L = nullptr;
  for (auto LB = LI->begin(), LE = LI->end(); LB != LE; ++LB) {
    L = *LB;
    if (++count > 1) {
      return false;
    }
  }
  if (L == nullptr) {
    return false;
  }
  // Look through the loop for a high relative successor count.
  unsigned BBCount = std::distance(L->block_begin(), L->block_end());
  // Each loop must have at least one basic block.
  assert(BBCount > 0);
  unsigned SuccCount = 0;
  for (auto BB : L->blocks()) {
    SuccCount += std::distance(succ_begin(BB), succ_end(BB));
  }
  return (100 * SuccCount / BBCount) > InlineConstants::BasicBlockSuccRatio;
}

//
// Return 'true' if the Function F has a Loop L whose trip count will be
// constant after F is inlined.
//
static bool boundConstArg(Function *F, Loop *L) {
  auto ICmp = getLoopBottomTest(L);
  if (!ICmp) {
    return false;
  }

  for (unsigned i = 0, e = ICmp->getNumOperands(); i != e; ++i) {
    auto Arg = dyn_cast<Argument>(ICmp->getOperand(i));
    if (!Arg)
      continue;
    unsigned ArgNo = Arg->getArgNo();
    for (Use &U : F->uses()) {
      if (auto CB = dyn_cast<CallBase>(U.getUser())) {
        if (!CB)
          return false;
        if (!CB->isCallee(&U))
          return false;
        if (!isa<Constant>(CB->getOperand(ArgNo)))
          return false;
      }
    }
    return true;
  }
  return false;
}

//
// Return 'true' if the Function F has a Loop L whose two inner most loops
// have trip counts that will be constant after F is inlined.
//
static bool hasConstTripCountArg(Function *F, Loop *L) {
  if (L->isInnermost() && !L->isOutermost() && boundConstArg(F, L) &&
      boundConstArg(F, L->getParentLoop()))
    return true;
  for (auto LB = L->begin(), LE = L->end(); LB != LE; ++LB)
    if (hasConstTripCountArg(F, *LB))
      return true;
  return false;
}

//
// Return 'true' if this is a double callsite worth inlining.
//   (This is one of multiple double callsite heuristics.)
//
// The criteria for this heuristic are:
//  (1) Must have exactly two calls to the function
//  (2) Must be only one loop nest
//  (3) The inner two loops of that nest must have a loop bound that
//      will be a constant after inlining is applied
//
static bool worthyDoubleCallSite2(CallBase &CB, InliningLoopInfoCache &ILIC) {
  Function *Callee = CB.getCalledFunction();
  LoopInfo *LI = ILIC.getLI(Callee);
  return std::distance(LI->begin(), LI->end()) == 1 &&
         hasConstTripCountArg(Callee, *(LI->begin()));
}

//
// Return the total number of predecessors for the basic blocks of 'F'
//
static unsigned int totalBasicBlockPredCount(Function &F) {
  unsigned int count = 0;
  for (Function::iterator BI = F.begin(), BE = F.end(); BI != BE; ++BI) {
    BasicBlock *BB = &*BI;
    count += std::distance(pred_begin(BB), pred_end(BB));
  }
  return count;
}

//
// Return 'true' if this is a double callsite worth inlining.
//   (This is one of multiple double callsite heuristics.)
//
// The criteria for this heuristic are:
//   (1) Must have exactly two calls to the function
//   (2) Call must be in a loop
//   (3) Called function must have loops
//   (4) Called function must not have any arguments
//         OR Called function must have a large enough total number
//           of predecessors
//
static bool worthyDoubleCallSite3(CallBase &CB, InliningLoopInfoCache &ILIC) {
  if (!NewDoubleCallSiteInliningHeuristics)
    return false;
  Function *Caller = CB.getCaller();
  LoopInfo *CallerLI = ILIC.getLI(Caller);
  if (!CallerLI->getLoopFor(CB.getParent()))
    return false;
  Function *Callee = CB.getCalledFunction();
  LoopInfo *CalleeLI = ILIC.getLI(Callee);
  if (CalleeLI->begin() == CalleeLI->end())
    return false;
  if (CB.arg_begin() != CB.arg_end() &&
      totalBasicBlockPredCount(*Callee) <
          InlineConstants::BigBasicBlockPredCount)
    return false;
  return true;
}

//
// Return 'true' if 'CB' worth inlining, given that it is a double callsite
// with internal linkage.
//
static bool worthyDoubleInternalCallSite(CallBase &CB,
                                         InliningLoopInfoCache &ILIC) {
  return worthyDoubleCallSite1(CB, ILIC) || worthyDoubleCallSite2(CB, ILIC) ||
         worthyDoubleCallSite3(CB, ILIC);
}

//
// Return 'true' if 'CB' and another callsite with the same caller and callee
// as well as two callsites which have 'CB's caller as their callee should
// be qualified for inlining as double callsites.
//
// For example:
//     define myouter
//        call myinner
//        call myinner
//     define myoutercaller0
//        call myouter
//     define myoutercaller1
//        call myouter
// If we return 'true', the two calls to myinner and the two calls to myouter
// will be selected for inlining.
//
// Special heuristics are employed to qualify both the inner and outer
// callsites for inlining.
//
// For the inner callsites, we look for a pair of callsites to a Function with
// exactly 9 arguments, which we break into groups of 3 (triads).
// The actual arguments in Triad 0 should all match pairwise.
// Those in Triad 1 should be related by the formula: Arg1 = Arg0 + ...
// Those in Triad 2 should be AllocInsts which are stored a floating-point
// value of 0 only once, and appear otherwise only in bitcasts to lifetime
// intrinsics or loads. Furthermore, myouter should have only 1 basic block
// and myinner should have no more than 'MaxBBInnerDoubleExternalFxn' basic
// blocks.
//
// For the outer callsites, their callers should have OpenMP directives
// and no more than 'MaxBBOuterDoubleExternalFxn' basic blocks.
//
static bool worthyDoubleExternalCallSite1(CallBase &CB, bool PrepareForLTO) {

  //
  // Return 'true' if the first three arguments of 'CB0' and 'CB1' match.
  //
  auto TestTriad0 = [](CallBase *CB0, CallBase *CB1) -> bool {
    for (unsigned I = 0; I < 3; ++I)
      if (CB0->getArgOperand(I) != CB1->getArgOperand(I))
        return false;
    return true;
  };

  //
  // Return 'true' if the middle three arguments of 'CB0' and 'CB1' have
  // the form 'Arg1 = Arg0 + ...'.
  //
  auto TestTriad1 = [](CallBase *CB0, CallBase *CB1) -> bool {
    for (unsigned I = 3; I < 6; ++I) {
      auto CBA = dyn_cast<LoadInst>(CB0->getArgOperand(I));
      Value *CBB = CB1->getArgOperand(I);
      if (!CBA) {
        CBA = dyn_cast<LoadInst>(CBB);
        if (!CBA)
          return false;
        CBB = CB0->getArgOperand(I);
      }
      Value *LD = nullptr;
      if (!match(CBB, m_FAdd(m_Specific(CBA), m_Value(LD))))
        return false;
    }
    return true;
  };

  //
  // Return 'true' if 'TV' is an AllocaInst whose use in a CallBase matches
  // the 'ArgNo' argument of 'CB', and which is stored a floating point 0 in
  // a StoreInst, and whose only other uses are bitcasts that feed lifetime
  // intrinsics or LoadInsts. Also handle the case when we have opaque
  // pointers and the AllocaInst feeds the lifetime intrinsics directly.
  //
  auto MatchesSZP = [](CallBase *CB, Value *TV, unsigned ArgNo) -> bool {
    unsigned CallBaseCount = 0;
    unsigned StoreInstCount = 0;
    auto AI = dyn_cast<AllocaInst>(TV);
    if (!AI)
      return false;
    for (User *U : AI->users()) {
      if (auto CBI = dyn_cast<CallBase>(U)) {
        if (CBI->isLifetimeStartOrEnd())
          continue;
        if (CBI != CB || TV != CB->getArgOperand(ArgNo) || CallBaseCount > 0)
          return false;
        ++CallBaseCount;
        continue;
      }
      if (auto BCI = dyn_cast<BitCastInst>(U)) {
        for (auto UU : BCI->users()) {
          auto ICB = dyn_cast<CallBase>(UU);
          if (!ICB || !ICB->isLifetimeStartOrEnd())
            return false;
        }
        continue;
      }
      if (isa<LoadInst>(U))
        continue;
      if (auto SI = dyn_cast<StoreInst>(U)) {
        auto CFP = dyn_cast<ConstantFP>(SI->getValueOperand());
        if (!CFP || !CFP->isZero() || StoreInstCount > 0)
          return false;
        ++StoreInstCount;
        continue;
      }
      return false;
    }
    return CallBaseCount == 1 && StoreInstCount == 1;
  };

  //
  // Return 'true' if the last three arguments of 'CB0' and 'CB1' all
  // match the pattern specified by 'MatchesSZP' above.
  //
  auto TestTriad2 = [&MatchesSZP](CallBase *CB0, CallBase *CB1) -> bool {
    for (unsigned I = 6; I < 9; ++I) {
      if (!MatchesSZP(CB0, CB0->getArgOperand(I), I))
        return false;
      if (!MatchesSZP(CB1, CB1->getArgOperand(I), I))
        return false;
    }
    return true;
  };

  //
  // Return 'true' if 'F' has only two callsites and no other uses and, if so,
  // set '*CB0' and '*CB1' to those uses.
  //
  auto HasOnlyTwoCallSites = [](Function &F,
                                CallBase **CB0, CallBase **CB1) -> bool {
    unsigned Count = 0;
    for (User *U : F.users()) {
      auto CB = dyn_cast<CallBase>(U);
      if (!CB)
        return false;
      if (++Count > 2)
        return false;
      if (!*CB0)
        *CB0 = CB;
      else
        *CB1 = CB;
    }
    return Count == 2;
  };

  //
  // Return 'true' if 'CB' has an OpenMp directive and no more than 'MaxBBs'
  // basic blocks.
  //
  auto IsWorthyUpperCallSite = [](CallBase &CB, unsigned MaxBBs) -> bool {
    Function *Caller = CB.getCaller();
    return vpo::VPOAnalysisUtils::mayHaveOpenmpDirective(*Caller)
        && Caller->size() <= MaxBBs;
  };

  //
  // Main code for worthyDoubleExternalCallSite1
  //
  CallBase *CB0 = nullptr;
  CallBase *CB1 = nullptr;
  //
  // Check if we have already qualified this callsite. It makes sense to
  // qualify the four callsites as a group, as we want all of them or none
  // of them. Furthermore if inlining is performed before all of the
  // callsites are qualified, the properties being qualified can change.
  // For example, if one of a pair of double callsites is inlined, the
  // remaining callsite is a single callsite rather than a double callsite.
  //
  if (CB.hasFnAttr("inline-doubleext2"))
    return true;
  if (!PrepareForLTO)
    return false;
  // Qualifications for the two inner callsites
  if (!hasTwoCallSitesInSameCaller(CB, &CB0, &CB1))
    return false;
  Function *Caller = CB.getCaller();
  if (Caller->size() != 1)
    return false;
  if (CB0->arg_size() != 9 || CB1->arg_size() != 9)
    return false;
  if (!TestTriad0(CB0, CB1) || !TestTriad1(CB0, CB1) || !TestTriad2(CB0, CB1))
    return false;
  if (CB.getCalledFunction()->size() > MaxBBInnerDoubleExternalFxn)
    return false;
  // Qualificatuions for the two outer callsites
  CallBase *CCB0 = nullptr;
  CallBase *CCB1 = nullptr;
  if (!HasOnlyTwoCallSites(*Caller, &CCB0, &CCB1))
    return false;
  if (!IsWorthyUpperCallSite(*CCB0, MaxBBOuterDoubleExternalFxn))
    return false;
  if (!IsWorthyUpperCallSite(*CCB1, MaxBBOuterDoubleExternalFxn))
    return false;
  // Qualify all callsites at once by setting the same attribute on all of them.
  CB0->addFnAttr("inline-doubleext2");
  CB1->addFnAttr("inline-doubleext2");
  CCB0->addFnAttr("inline-doubleext2");
  CCB1->addFnAttr("inline-doubleext2");
  return true;
}

//
// Return 'true' if 'CB' worth inlining, given that it is a double callsite
// with external linkage.
//
// The criteria for this heuristic are:
//   (1) Single basic block in the caller
//   (2) Single use which is not a direct invocation of the caller
//   (3) No calls to functions other than the called function
//       (except intrinsics added by using -g)
//
static bool worthyDoubleExternalCallSite2(CallBase &CB) {
  Function *Caller = CB.getCaller();
  if (!NewDoubleCallSiteInliningHeuristics)
    return false;
  if (std::distance(Caller->begin(), Caller->end()) != 1)
    return false;
  Function *Callee = CB.getCalledFunction();
  unsigned int count = 0;
  for (User *U : Caller->users()) {
    auto CBB = dyn_cast<CallBase>(U);
    if (CBB && (CBB->getCalledFunction() == Caller))
      return false;
    if (++count > 1)
      return false;
  }
  if (count != 1)
    return false;
  for (BasicBlock &BB : *Caller) {
    for (Instruction &I : BB) {
      auto CBB = dyn_cast<CallBase>(cast<Value>(&I));
      if (!CBB)
        continue;
      Function *F = CBB->getCalledFunction();
      if (F != nullptr && F->getName() == "llvm.dbg.value")
        continue;
      if (F != Callee)
        return false;
    }
  }
  return true;
}

//
// Return 'true' if 'CB' worth inlining, given that it is a double callsite
// with external linkage.
//
// The criteria for this heuristic are:
//   (1) Is a 'PrepareForLTO' compilation.
//   (2) The callee is Fortran.
//   (3) The callee has many arguments.
//   (3) Both callsites are in the same caller.
//   (4) The callee has many loops with IVDEP directive.
//
static bool worthyDoubleExternalCallSite3(CallBase &CB,
                                          bool PrepareForLTO,
                                          InliningLoopInfoCache &ILIC) {
  static Function *WorthyCallee = nullptr;
  if (CB.hasFnAttr("inline-doubleext3"))
    return true;
  if (WorthyCallee)
    return false;
  Function *Callee = CB.getCalledFunction();
  if (!PrepareForLTO || !Callee->isFortran())
     return false;
  if (Callee->arg_size() < MinCalleeArgsDoubleExternal)
    return false;
  CallBase *CB0 = nullptr;
  CallBase *CB1 = nullptr;
  if (!hasTwoCallSitesInSameCaller(CB, &CB0, &CB1))
    return false;
  LoopInfo *LI = ILIC.getLI(Callee);
  if (!LI || LI->empty())
    return false;
  bool PassedIVDEPTest = false;
  unsigned Count = 0;
  auto Loops = LI->getLoopsInPreorder();
  for (auto L : Loops)
    if (findOptionMDForLoop(L, "llvm.loop.vectorize.ivdep_back"))
      if (++Count >= MinCalleeIVDEPLoopsDoubleExternal) {
        PassedIVDEPTest = true;
        break;
      }
  if (!PassedIVDEPTest)
    return false;
  CB0->addFnAttr("inline-doubleext3");
  CB1->addFnAttr("inline-doubleext3");
  WorthyCallee = Callee;
  return true;
}

//
// Return 'true' if 'CB' worth inlining, given that it is a double callsite
// with external linkage. If we need to use the big bonus to actually get
// the callsites to inline, set '*UseBigBonus'. (NOTE: It might be useful
// to use a single big bonus consistently, but that should be tested in
// its own change set.)
//
static bool worthyDoubleExternalCallSite(CallBase &CB,
                                         bool PrepareForLTO,
                                         InliningLoopInfoCache &ILIC,
                                         bool *UseBigBonus) {
  if (worthyDoubleExternalCallSite1(CB, PrepareForLTO)) {
    *UseBigBonus = true;
    return true;
  }
  if (worthyDoubleExternalCallSite2(CB)) {
    *UseBigBonus = false;
    return true;
  }
  if (worthyDoubleExternalCallSite3(CB, PrepareForLTO, ILIC)) {
    *UseBigBonus = true;
    return true;
  }
  return false;
}

//
// Check that loop has normalized structure and constant trip count.
//
static bool isConstantTripCount(Loop *L) {

  //
  // Test that 'PHIN' is a PHINode with two incoming values, one which is
  // a ConstantInt 'Start' and the other which is a BinaryOperator. If it
  // is, return the BinaryOperator.
  //
  auto GetBOFromPHI = [](PHINode *PHIN, int64_t Start) -> BinaryOperator * {
    if (!PHIN)
      return nullptr;
    unsigned NumIn = PHIN->getNumIncomingValues();
    if (NumIn != 2)
      return nullptr;
    ConstantInt *CI = nullptr;
    BinaryOperator *BO = nullptr;
    for (unsigned I = 0; I < NumIn; ++I) {
      Value *V = PHIN->getIncomingValue(I);
      if (auto CITest = dyn_cast<ConstantInt>(V)) {
        if (CI || CITest->getSExtValue() != Start)
          return nullptr;
        CI = CITest;
        continue;
      }
      if (auto BOTest = dyn_cast<BinaryOperator>(V)) {
        if (BO)
          return nullptr;
        BO = BOTest;
        continue;
      }
      return nullptr;
    }
    return BO && CI ? BO : nullptr;
  };

  //
  // Test that 'BO' is a BinaryOperator whose operands are a PHINode and
  // a ConstantInt 'Step'. If it is, return the PHINode.
  //
  auto GetPHIFromBO = [](BinaryOperator *BO, int64_t Step) -> PHINode * {
    if (!BO)
      return nullptr;
    Value *PHITest = nullptr;
    ConstantInt *CITest = nullptr;
    if (!match(BO, m_Add(m_Value(PHITest), m_ConstantInt(CITest))))
      return nullptr;
    if (CITest->getSExtValue() != Step)
      return nullptr;
    auto PHIGood = dyn_cast<PHINode>(PHITest);
    return PHIGood;
  };

  // Test that canonical induction variable exists, and that Loop bottom
  // test has the right form.
  PHINode *IV = L->getCanonicalInductionVariable();
  if (!IV)
    return false;
  ICmpInst *CInst = getLoopBottomTest(L);
  if (!CInst || !CInst->isIntPredicate() || CInst->getNumOperands() != 2)
    return false;
  ICmpInst::Predicate Pred = CInst->getPredicate();
  if (!(Pred == ICmpInst::ICMP_EQ || Pred == ICmpInst::ICMP_ULT ||
        Pred == ICmpInst::ICMP_ULE || Pred == ICmpInst::ICMP_SLT ||
        Pred == ICmpInst::ICMP_SLE)) {
    return false;
  }

  // Use fixed constant values for the Loop induction variable's Start, Step,
  // and Stop. These can be generalized if we want a more general heuristic.
  int64_t Start = 0;
  int64_t Step = 1;
  int64_t Stop = 4;
  ConstantInt *Const = dyn_cast<ConstantInt>(CInst->getOperand(1));
  if (!Const || Const->getSExtValue() != Stop)
    return false;
  // Test that the loop increment appears in a definition cycle with a
  // PHINode and BinaryOperator increment.
  Value *IVLeft = CInst->getOperand(0);
  auto PHITest = dyn_cast<PHINode>(IVLeft);
  if (PHITest)
    return GetPHIFromBO(GetBOFromPHI(PHITest, Start), Step) == PHITest;
  auto BOTest = dyn_cast<BinaryOperator>(IVLeft);
  if (BOTest)
    return GetBOFromPHI(GetPHIFromBO(BOTest, Step), Start) == BOTest;
  return false;
}

//
// Given the Loop 'LL' and incoming 'ArgCnt', update and return 'ArgCnt'.
// 'ArgCnt' represents the number of GEPs and SubscriptInsts that have
// a pointer operand which is a function Argument.
//
static int handleLoopForFusion(Loop *LL, int ArgCnt) {
  for (auto *BB : LL->blocks()) {
    for (auto &I : *BB) {
      if (ArgCnt >= MinArgRefs)
        return ArgCnt;
      Value *PtrOp = nullptr;
      if (GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(&I)) {
        PtrOp = GEPI->getPointerOperand();
      } else if (SubscriptInst *SI = dyn_cast<SubscriptInst>(&I)) {
        PtrOp = SI->getPointerOperand();
        while ((SI = dyn_cast<SubscriptInst>(PtrOp)))
          PtrOp = SI->getPointerOperand();
      }
      if (PtrOp) {
        if (isa<PHINode>(PtrOp))
          PtrOp = dyn_cast<PHINode>(PtrOp)->getIncomingValue(0);
        if (isa<Argument>(PtrOp))
          ArgCnt++;
      }
    }
  }
  return ArgCnt;
}

//
// Given 'VecSubLoops', a vector of Loops enclosed within some loop,
// and an incoming 'ArgCnt', update and return the 'ArgCnt' by
// incrementing it for each GEP or SubscriptInst which has a
// function Argument as its pointer operand.
//
static int handleSubLoopsForFusion(std::vector<Loop *> &VecSubLoops,
                                   int ArgCnt) {
  for (int I = 0, E = VecSubLoops.size(); I < E; I++) {
    Loop *LL = VecSubLoops[I];
    ArgCnt = handleLoopForFusion(LL, ArgCnt);
    ArgCnt = handleSubLoopsForFusion(LL->getSubLoopsVector(), ArgCnt);
  }
  return ArgCnt;
}

//
// Return the depth of the deepest Loop represented in 'LI'.
//
static unsigned maxLoopDepth(LoopInfo *LI) {
  auto Loops = LI->getLoopsInPreorder();
  unsigned MaxDepth = 0;
  for (auto L : Loops) {
    unsigned CurDepth = L->getLoopDepth();
    if (CurDepth > MaxDepth)
      MaxDepth = CurDepth;
  }
  return MaxDepth;
}

/// \brief Analyze a callsite for potential inlining for fusion.
///
/// Returns true if inlining of this and a number of successive callsites
/// with the same callee would benefit from loop fusion and vectorization
/// later on.
/// Two slightly different heuristics are used, one for the PrepareForLTO
/// stage, and one for the !PrepareForLTO stage of inlining.  See the
/// comments for details.
/// In case we decide that current CB is a candidate for inlining for fusion,
/// then we store other CB to the same function in the same basic block in
/// the set of inlining candidates for fusion.
static bool worthInliningForFusion(CallBase &CB, TargetLibraryInfo *TLI,
                                   const TargetTransformInfo &CalleeTTI,
                                   InliningLoopInfoCache &ILIC,
                                   bool PrepareForLTO) {

  //
  // Return 'true' if the module 'M' has no more than 'Limit' Functions
  // with IR.
  //
  auto IsSmallApp = [](Module *M, unsigned Limit) {
    static bool Computed = false;
    static bool ComputedIsSmallApp = false;
    if (Computed)
      return ComputedIsSmallApp;
    unsigned FunctionCount = 0;
    for (auto &F : M->functions()) {
      if (F.isDeclaration())
        continue;
      if (++FunctionCount > Limit) {
        Computed = true;
        ComputedIsSmallApp = false;
        return false;
      }
    }
    Computed = true;
    ComputedIsSmallApp = true;
    return true;
  };

  //
  // Early test for inlining to promote fusion. We are looking for a
  // case where the callee is called twice in the caller, each time within
  // a single loop. The callee may be a state machine that could get fully
  // expanded after the inlining. To predict whether the callee is a
  // likely candidate, we check that a large number of BasicBlocks
  // terminate in switch statements with a large number of cases, and
  // that the callee has a single ReturnInst where the return value is
  // fed by a PHINode with a large number of inputs.
  //
  auto IsEarlyFusionCandidate = [](Function *Caller, Function *Callee,
                                   InliningLoopInfoCache &ILIC,
                                   SmallPtrSetImpl<CallBase *>
                                       &FusionCBs) -> bool {
    // The Callee is called by the Caller twice.
    unsigned CallerCount = 0;
    for (User *U : Callee->users()) {
      auto TCB = dyn_cast<CallBase>(U);
      if (!TCB || TCB->getCaller() != Caller || ++CallerCount > 2)
        return false;
      FusionCBs.insert(TCB);
    }
    if (CallerCount != 2 || Caller->hasAddressTaken())
      return false;
    // Each of the Callee's calls is enclosed in a single loop.
    LoopInfo *LI = ILIC.getLI(Caller);
    if (!LI)
      return false;
    for (CallBase *CB : FusionCBs) {
      Loop *InnerLoop = LI->getLoopFor(CB->getParent());
      if (!InnerLoop || InnerLoop->getLoopDepth() != 1)
        return false;
    }
    // Not too many BasicBlocks in the Callee.
    if (Callee->size() > NumBBsForEarlyFusion)
      return false;
    // Many of the Callee's BasicBlocks should terminate in SwitchInsts
    // with a fair number of cases. The Callee should have a single
    // ReturnInst fed by a PHINode with many inputs.
    unsigned SwitchCount = 0;
    unsigned CasesCount = 0;
    bool SawGoodReturnInst = false;
    for (auto &BB : *Callee) {
      auto TI = BB.getTerminator();
      if (auto SI = dyn_cast<SwitchInst>(TI)) {
        ++SwitchCount;
        CasesCount += SI->getNumCases();
      } else if (auto RI = dyn_cast<ReturnInst>(TI)) {
        if (SawGoodReturnInst)
          return false;
        auto PN = dyn_cast_or_null<PHINode>(RI->getReturnValue());
        if (!PN || PN->getNumIncomingValues() < NumRetPHIInputsForEarlyFusion)
          return false;
        SawGoodReturnInst = true;
      }
    }
    if (SwitchCount < NumSwitchesForEarlyFusion)
      return false;
    if (CasesCount < NumCasesForEarlyFusion)
      return false;
    return true;
  };

  Function *Caller = CB.getCaller();
  Function *Callee = CB.getCalledFunction();

  // CMPLRLLVM-28952: Early inlining for fusion heuristic test. The call
  // Callee->isPreLoopOpt() tests if LoopOpt will be run.
  SmallPtrSet<CallBase *, 2> FusionCBs;
  if (PrepareForLTO && Callee->isPreLoopOpt() &&
      !CB.hasFnAttr("inline-fusion") &&
      IsEarlyFusionCandidate(Caller, Callee, ILIC, FusionCBs)) {
    for (CallBase *CB : FusionCBs)
      CB->addFnAttr("inline-fusion");
    return true;
  }

  // The call site was marked as candidate for inlining for fusion.
  if (CB.hasFnAttr("inline-fusion")) {
    CB.removeFnAttr("inline-fusion");
    return true;
  }

  // Must have at least AVX2 for the standard heuristic.
  if (InliningForFusionHeuristics != cl::BOU_TRUE) {
    auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX2;
    if (!CalleeTTI.isAdvancedOptEnabled(TTIAVX2)) {
      LLVM_DEBUG(llvm::dbgs()
                 << "IC: No inlining for fusion. Must support at least AVX2.");
      return false;
    }
  }

  // Heuristic is enabled if option is unset or if option is set to true.
  if (((InliningForFusionHeuristics != cl::BOU_UNSET)) &&
      (InliningForFusionHeuristics != cl::BOU_TRUE)) {
    LLVM_DEBUG(llvm::dbgs() << "IC: No inlining for fusion. Option not set.");
    return false;
  }

  BasicBlock *CSBB = CB.getParent();

  //
  // CMPLRLLVM-22909: Limit the size of the application to which the
  // !PrepareForLTO heuristic can be applied to save compile time.
  //
  Module *M = CB.getFunction()->getParent();
  if (!PrepareForLTO && (!DTransInlineHeuristics ||
      !IsSmallApp(M, FusionSmallAppFunctionLimit))) {
    LLVM_DEBUG(llvm::dbgs() << "IC: No inlining for fusion. "
                               "Large app or not advanced opt enabled.");
    return false;
  }

  SmallVector<CallBase *, 20> LocalCSForFusion;
  int CSCount = 0;
  if (PrepareForLTO) {
    // Check that all call sites in the Caller, that are not to intrinsics,
    // are in the same basic block and are fusion candidates.
    for (auto &I : instructions(Caller)) {
      auto LocalCB = dyn_cast<CallBase>(&I);
      if (!LocalCB)
        continue;
      Function *CurrCallee = LocalCB->getCalledFunction();
      if (!CurrCallee) {
        LLVM_DEBUG(llvm::dbgs()
                   << "IC: No inlining for fusion: Indirect call.\n");
        return false;
      }
      if (CurrCallee->isIntrinsic())
        continue;
      if (CurrCallee != Callee) {
        LLVM_DEBUG(llvm::dbgs()
                   << "IC: No inlining for fusion: Mismatched callees.\n");
        return false;
      }
      if (I.getParent() != CSBB) {
        LLVM_DEBUG(llvm::dbgs()
                   << "IC: No inlining for fusion: Multiple basic blocks.\n");
        return false;
      }
      LocalCSForFusion.push_back(LocalCB);
      CSCount++;
    }
  } else {
    // Check that there is no indirect call in the Caller. Check that the
    // greatest number of call sites in the Caller is to the Callee.
    Function *MaxCallee = nullptr;
    SmallDenseMap<Function *, int> CallSiteCountMap;
    for (auto &I : instructions(Caller)) {
      auto LocalCB = dyn_cast<CallBase>(&I);
      if (!LocalCB || isa<IntrinsicInst>(&I))
        continue;
      Function *MyCallee = LocalCB->getCalledFunction();
      if (!MyCallee) {
        LLVM_DEBUG(llvm::dbgs()
                   << "IC: No inlining for fusion: Indirect call.\n");
        return false;
      }
      if (MyCallee->isDeclaration())
        continue;
      if (MyCallee == Callee)
        LocalCSForFusion.push_back(LocalCB);
      int MyCount = ++CallSiteCountMap[MyCallee];
      if (MyCount >= CSCount) {
        CSCount = MyCount;
        MaxCallee = MyCallee;
      }
    }
    if (MaxCallee != Callee) {
      LLVM_DEBUG(llvm::dbgs()
                 << "IC: No inlining for fusion: Not most frequent callee.\n");
      return false;
    }
  }

  // If the user doesn't specify number of call sites explicitly, then use
  // some specific number as the default value.
  if (NumCallSitesForFusion.empty()) {
    if (PrepareForLTO) {
      NumCallSitesForFusion.push_back(8);
      // TODO: NumCallSitesForFusion.push_back(2);
    } else {
      NumCallSitesForFusion.push_back(3);
    }
  }
  bool CSCountApproved = false;
  for (int CSCountVariant : NumCallSitesForFusion)
    if (CSCount == CSCountVariant) {
      CSCountApproved = true;
      break;
    }
  if (!CSCountApproved) {
    LLVM_DEBUG(
        llvm::dbgs()
        << "IC: No inlining for fusion. Inappropriate number of call sites: "
        << CSCount << "\n");
    return false;
  }

  // Check if callee has function calls inside - skip it. Make an exception
  // for intrinsics and library functions.
  for (BasicBlock &BB : *Callee) {
    for (auto &I : BB) {
      if (isa<CallInst>(I) || isa<InvokeInst>(I)) {
        CallInst *CI = dyn_cast_or_null<CallInst>(&I);
        InvokeInst *II = dyn_cast_or_null<InvokeInst>(&I);
        auto InnerFunc = CI ? CI->getCalledFunction() : II->getCalledFunction();
        if (!InnerFunc) {
          LLVM_DEBUG(llvm::dbgs() << "IC: No inlining for fusion: "
                                     " indirect call inside candidate.\n");
          return false;
        }
        LibFunc LibF;
        if (!InnerFunc->isIntrinsic() && !TLI->getLibFunc(*InnerFunc, LibF)) {
          LLVM_DEBUG(llvm::dbgs() << "IC: No inlining for fusion: "
                                     " user call inside candidate.\n");
          return false;
        }
      }
    }
  }

  // Check loops inside callee.
  LoopInfo *LI = ILIC.getLI(Callee);
  if (!LI) {
    LLVM_DEBUG(llvm::dbgs()
               << "IC: No inlining for fusion: no loop info for candidate.\n");
    return false;
  }

  int ArgCnt = 0;
  for (auto LB = LI->begin(), LE = LI->end(); LB != LE; ++LB) {
    Loop *LL = *LB;
    if (PrepareForLTO && !isConstantTripCount(LL)) {
      // Non-constant trip count. Skip inlining.
      LLVM_DEBUG(llvm::dbgs()
                 << "IC: No inlining for fusion: non-constant TC in loop.\n");
      return false;
    }
    // Check how many array refs in GEP or SubscriptInst instructions are
    // arguments of the callee.
    ArgCnt = handleLoopForFusion(LL, ArgCnt);
    if (ArgCnt >= MinArgRefs)
      break;
    if (!PrepareForLTO) {
      ArgCnt = handleSubLoopsForFusion(LL->getSubLoopsVector(), ArgCnt);
      if (ArgCnt >= MinArgRefs)
        break;
    }
  }

  // Not enough arguments-arrays were found in loop.
  if (ArgCnt < MinArgRefs) {
    LLVM_DEBUG(llvm::dbgs()
               << "IC: No inlining for fusion: not enough array refs.\n");
    return false;
  }

  if (PrepareForLTO) {
    // Require a minimum number of instructions.
    // TODO: Remove this condition once 2 becomes a legal CSCount.
    if (Caller->getInstructionCount() < 40) {
      LLVM_DEBUG(llvm::dbgs()
                 << "IC: No inlining for fusion: not enough instructions.\n");
      return false;
    }
  } else {
    // Allow only one loop nest.
    if (std::distance(LI->begin(), LI->end()) != 1) {
      LLVM_DEBUG(llvm::dbgs()
                 << "IC: No inlining for fusion: more than one loop nest.\n");
      return false;
    }
    // Loop nest must be sufficiently deep.
    if (maxLoopDepth(LI) < FusionMinLoopNestDepth) {
      LLVM_DEBUG(llvm::dbgs()
                 << "IC: No inlining for fusion: loop nest not deep enough.\n");
      return false;
    }
  }

  // Mark other inlining candidates with an attribute showing a strong
  // preference for inlining.
  for (auto LocalCB : LocalCSForFusion)
    LocalCB->addFnAttr("inline-fusion");

  return true;
}

/// \brief Calculate depth of the current BasicBlock in the given Dominator
/// tree. Map is used to store IF-depth for each block so we don't need to
/// re-calculate it multiple times.
static int calculateMaxIfDepth(BasicBlock *Node, DominatorTree *DT,
                               DenseMap<BasicBlock *, int> &Map) {
  if (!Node)
    return 0;

  DenseMap<BasicBlock *, int>::iterator It = Map.find(Node);
  if (It != Map.end())
    return It->second;

  BasicBlock *BB = Node;
  int CurrIfDepth = 0;

  while (BB) {
    if (Instruction *TermInst = BB->getTerminator()) {
      BranchInst *Inst = dyn_cast<BranchInst>(TermInst);
      if (Inst && Inst->isConditional()) {
        ++CurrIfDepth;
      }
    }
    if (DT->getNode(BB)->getIDom()) {
      BB = DT->getNode(BB)->getIDom()->getBlock();
    } else {
      break;
    }
  }

  Map.insert(std::make_pair(Node, CurrIfDepth));

  int CurrMaxIfDepth = CurrIfDepth;
  for (auto *NextNode : successors(Node)) {
    if (NextNode != Node)
      CurrMaxIfDepth =
          std::max(CurrMaxIfDepth, calculateMaxIfDepth(NextNode, DT, Map));
  }

  return CurrMaxIfDepth;
}

/// \brief Calculates the depth of the deepest 'if' statment in routine.
static int deepestIfInDomTree(DominatorTree *DT) {

  // To avoid recalculating of BasicBlock depth, it will be stored in a map.
  DenseMap<BasicBlock *, int> BBIfDepth;
  const DomTreeNodeBase<BasicBlock> *DomRoot = DT->getRootNode();

  return calculateMaxIfDepth(DomRoot->getBlock(), DT, BBIfDepth);
}

static bool worthInliningForDeeplyNestedIfs(CallBase &CB,
                                            InliningLoopInfoCache &ILIC,
                                            bool IsCallerRecursive,
                                            bool PrepareForLTO) {
  // Heuristic is enabled if option is unset and it is first inliner run
  // (on PrepareForLTO phase) OR if option is set to true.
  // CMPLRLLVM-34251: Put this heuristic under DTransInlineHeuristics test
  if (!DTransInlineHeuristics ||
      ((InliningForDeeplyNestedIfs != cl::BOU_UNSET) ||
      !PrepareForLTO) && (InliningForDeeplyNestedIfs != cl::BOU_TRUE))
    return false;

  Function *Callee = CB.getCalledFunction();
  Function *Caller = CB.getCaller();

  // Recursive callee is not allowed
  if (Caller == Callee)
    return false;

  // Caller should be recursive
  if (!IsCallerRecursive)
    return false;

  // Caller should contain 'switch' instruction.
  bool HasSwitchInst = false;
  for (BasicBlock &BB : *Caller) {
    for (auto &I : BB) {
      if (isa<SwitchInst>(I)) {
        HasSwitchInst = true;
        break;
      }
    }
  }

  if (!HasSwitchInst)
    return false;

  // Callee should have no loops.
  LoopInfo *CalleeLI = ILIC.getLI(Callee);
  if (!CalleeLI->empty())
    return false;

  // Both caller and callee should have deeply nested ifs.
  DominatorTree *CallerDT = ILIC.getDT(Caller);
  unsigned CallerIfDepth = deepestIfInDomTree(CallerDT);
  if (CallerIfDepth < MinDepthOfNestedIfs)
    return false;

  DominatorTree *CalleeDT = ILIC.getDT(Callee);
  unsigned CalleeIfDepth = deepestIfInDomTree(CalleeDT);
  if (CalleeIfDepth < MinDepthOfNestedIfs)
    return false;

  return true;
}

//
// Return 'true' if 'CB' is worth inlining due to multiple arguments being
// pointers dereferenced in callee code.
//
// The criteria for this heuristic are:
//   (1) Caller and callee have loops.
//   (2) Callsite is in loop nest of depth InliningForACLoopDepth (default 2),
//       and it is the only callsite in the loop.
//   (3) Callee's arguments are pointers dereferenced in the code multiple
//       times.
//
static bool worthInliningForAddressComputations(CallBase &CB,
                                                InliningLoopInfoCache &ILIC,
                                                bool PrepareForLTO) {
  // Heuristic is enabled if option is unset and it is first inliner run
  // (on PrepareForLTO phase) OR if option is set to true.
  if (((InliningForAddressComputations != cl::BOU_UNSET) || !PrepareForLTO) &&
      (InliningForAddressComputations != cl::BOU_TRUE))
    return false;

  Function *Callee = CB.getCalledFunction();
  Function *Caller = CB.getCaller();

  if (Caller == Callee) {
    LLVM_DEBUG(llvm::dbgs() << "IC: No inlining for AC: recursive callee.\n");
    return false;
  }

  LoopInfo *CalleeLI = ILIC.getLI(Callee);
  if (CalleeLI->empty()) {
    LLVM_DEBUG(llvm::dbgs()
               << "IC: No inlining for AC: callee has no loops.\n");
    return false;
  }

  LoopInfo *CallerLI = ILIC.getLI(Caller);
  if (CallerLI->empty()) {
    LLVM_DEBUG(llvm::dbgs()
               << "IC: No inlining for AC: caller has no loops.\n");
    return false;
  }

  Loop *InnerLoop = CallerLI->getLoopFor(CB.getParent());
  if (!InnerLoop)
    return false;

  if (InnerLoop->getLoopDepth() != InliningForACLoopDepth) {
    LLVM_DEBUG(llvm::dbgs()
               << "IC: No inlining for AC: callee is not in the loop "
                  "nest of depth InliningForACLoopDepth.\n");
    return false;
  }

  if (InnerLoop->getLoopDepth() != InliningForACLoopDepth) {
    LLVM_DEBUG(llvm::dbgs()
               << "IC: No inlining for AC: callee is not in the loop "
                  "nest of depth InliningForACLoopDepth.\n");
    return false;
  }

  int CallSiteCount = 0;
  for (auto *BB : InnerLoop->blocks()) {
    for (auto &I : *BB) {
      if (isa<CallInst>(I) || isa<InvokeInst>(I)) {
        CallInst *CI = dyn_cast_or_null<CallInst>(&I);
        InvokeInst *II = dyn_cast_or_null<InvokeInst>(&I);
        auto InnerFunc = CI ? CI->getCalledFunction() : II->getCalledFunction();
        if (!InnerFunc) {
          LLVM_DEBUG(
              llvm::dbgs()
              << "IC: No inlining for AC: indirect call inside callee.\n");
          return false;
        }
        if (!InnerFunc->isIntrinsic())
          ++CallSiteCount;

        if (CallSiteCount > 1) {
          LLVM_DEBUG(llvm::dbgs()
                     << "IC: No inlining for AC: only one callsite "
                        "allowed in the loop.\n");
          return false;
        }
      }
    }
  }

  unsigned ArgCnt = 0;
  // Check how many array refs in GEP instructions are arguments of
  // the callee.
  for (auto &BB : *Callee) {
    for (auto &I : BB) {
      if (auto *GEPI = dyn_cast<GetElementPtrInst>(&I)) {
        if (isa<Argument>(GEPI->getPointerOperand()))
          ArgCnt++;
        if (auto *Node = dyn_cast<PHINode>(GEPI->getPointerOperand()))
          for (unsigned J = 0, E = Node->getNumIncomingValues(); J < E; ++J)
            if (isa<Argument>(Node->getIncomingValue(J)))
              ArgCnt++;
      }
      if (ArgCnt > InliningForACMinArgRefs)
        break;
    }
    if (ArgCnt > InliningForACMinArgRefs)
      break;
  }

  // Not enough arguments-arrays were found in callee.
  if (ArgCnt < InliningForACMinArgRefs) {
    LLVM_DEBUG(llvm::dbgs() << "IC: No inlining for AC: not enough argument "
                               "arrays inside callee.\n");
    return false;
  }

  LLVM_DEBUG(llvm::dbgs() << "IC: Do inlining for AC.\n");
  return true;
}

//
// Return 'true' if the Function F should be inlined because it exposes some
// important stack computations. (NOTE: Right now, we only qualify such a
// function when we are not in the compile time pass of a link time
// compilation and if special DTRANS options are thrown. We also only qualify
// a single function at this time.  The heuristic can be extended if we find
// that inlining multiple functions is of value.)
//
static bool worthInliningForStackComputations(Function *F,
                                              TargetLibraryInfo *TLI,
                                              bool PrepareForLTO) {

  //
  // Return 'true' if Function *F passes the argument test.  We are looking
  // for a Function that has two arguments.  The first should be a pointer,
  // and the second should be an integer.
  //
  auto PassesArgTest = [](Function *F) -> bool {
    auto CalleeArgCount = F->arg_size();
    if (CalleeArgCount != 2)
      return false;
    Argument *Arg0 = F->arg_begin();
    if (!Arg0->getType()->isPointerTy())
      return false;
    Argument *Arg1 = F->arg_begin() + 1;
    if (!Arg1->getType()->isIntegerTy())
      return false;
    return true;
  };

  //
  // Return 'true' if the Function *F passes the control flow test.  We are
  // looking for a Function that has three BasicBlocks:
  //   EntryBB: Terminates in a > test which branches to IncBB if true and
  //     RetBB if false
  //   IncBB: Terminates in an unconditional branch to RetBB
  //   RetBB: Has only a return in it.
  //
  auto PassesControlFlowTest = [](Function *F) -> bool {
    auto CalleeBBs = F->size();
    if (CalleeBBs != 3)
      return false;
    BasicBlock *EntryBB = &(F->getEntryBlock());
    auto TI = EntryBB->getTerminator();
    auto BI = dyn_cast<BranchInst>(TI);
    if (!BI || !BI->isConditional())
      return false;
    auto IncBB = BI->getSuccessor(0);
    BasicBlock *RetBB = BI->getSuccessor(1);
    if (IncBB == EntryBB || IncBB == RetBB || EntryBB == RetBB)
      return false;
    auto IBI = dyn_cast<BranchInst>(IncBB->getTerminator());
    if (!IBI || IBI->isConditional())
      return false;
    if (IncBB->getSingleSuccessor() != RetBB)
      return false;
    if (RetBB->size() != 1)
      return false;
    if (!isa<ReturnInst>(&RetBB->front()))
      return false;
    auto *ICI = dyn_cast<ICmpInst>(BI->getCondition());
    if (!ICI || ICI->getPredicate() != ICmpInst::ICMP_SGT)
      return false;
    return true;
  };

  //
  // Returns 'true' if the Function *F passes the contents test.  Here, we are
  // looking for a test
  //   BinOpTest(GlobalStack) .SGT. GlobalMax
  // at the end of EntryBB, a StoreInst in IncBB to GlobalMax and a CallInst
  // in IncBB to a function that calls a realloc-like function. Here BinOpTest
  // is a series of BinaryOperators.
  //
  auto PassesContentsTest = [](Function *F, TargetLibraryInfo *TLI) -> bool {
    unsigned StoreCount = 0;
    unsigned CallCount = 0;
    BasicBlock *EntryBB = &(F->getEntryBlock());
    auto TI = EntryBB->getTerminator();
    // We have already tested the types of BI and ICI in passesControlFlowTest,
    // so we can use casts here instead of dyn_casts.
    auto BI = cast<BranchInst>(TI);
    auto *ICI = cast<ICmpInst>(BI->getCondition());
    auto GlobalMax = dyn_cast<LoadInst>(ICI->getOperand(1));
    if (!GlobalMax)
      return false;
    auto GlobalMaxPtr = dyn_cast<GlobalValue>(GlobalMax->getPointerOperand());
    if (!GlobalMaxPtr)
      return false;
    auto GlobalStackPtr = traceBack(ICI->getOperand(0), 1, 3);
    if (!GlobalStackPtr)
      return false;
    auto IncBB = BI->getSuccessor(0);
    for (auto &I : *IncBB) {
      auto SI = dyn_cast<StoreInst>(&I);
      if (SI) {
        if (StoreCount > 0)
          return false;
        auto GV = dyn_cast<GlobalValue>(SI->getPointerOperand());
        if (GV != GlobalMaxPtr)
          return false;
        StoreCount++;
      }
      auto CI = dyn_cast<CallInst>(&I);
      if (CI) {
        if (CallCount > 0)
          return false;
        if (!callsRealloc(CI->getCalledFunction(), TLI))
          return false;
        CallCount++;
      }
      if (StoreCount && CallCount)
        return true;
    }
    return false;
  };

  //
  // Use 'WorthyFunction' to store the single worthy Function if found.
  // Use SmallPtrSet to store those Functions that have already been tested
  // and have failed the test, so we don't need to test them again.
  //
  static Function *WorthyFunction = nullptr;
  static SmallPtrSet<Function *, 32> FunctionsTestedFail;
  if (!TLI || !DTransInlineHeuristics || PrepareForLTO)
    return false;
  if (WorthyFunction)
    return WorthyFunction == F;
  if (FunctionsTestedFail.count(F))
    return false;
  if (!PassesArgTest(F) || !PassesControlFlowTest(F) ||
      !PassesContentsTest(F, TLI)) {
    FunctionsTestedFail.insert(F);
    return false;
  }
  WorthyFunction = F;
  return true;
}

//
// Return 'true' if a CallSite with this 'Callee' should be inlined because
// it is on the 'QueuedCallers' set. NOTE: We determine whether the 'Callee'
// should be inlined before the inlining of its CallSites, because
// performing the inlining of the CallSites first makes it harder to tell
// if the caller should be inlined.
//
static bool worthInliningSingleBasicBlockWithStructTest(
    Function *Callee, bool PrepareForLTO,
    SmallPtrSetImpl<Function *> *QueuedCallers) {
  if (PrepareForLTO || !DTransInlineHeuristics)
    return false;
  return QueuedCallers->count(Callee);
}

//
// Return 'true' if 'Callee' is preferred for inlining because it is
// a recursive progression clone marked with an attribute as being
// preferred for inlining ("prefer-inline-rec-pro-clone").
//
static bool worthInliningForRecProgressionClone(Function *Callee) {
  return Callee && Callee->hasFnAttribute("prefer-inline-rec-pro-clone");
}

//
// Return 'true' if 'F' calls an Intel partial inlining inlined clone,
// which calls an Intel partial inlining outlined function, which calls
// 'F'. (Note that the functions are tested in reverse order, because it
// is cheaper in compile time to find the callers of a function than it
// it is to find the callees.
//
static bool preferPartialInlineHasExtractedRecursiveCall(Function &F,
                                                         bool PrepareForLTO) {
  static SmallPtrSet<Function *, 3> Candidates;
  static bool ScannedFunctions = false;
  static bool SawPreferPartialInlineOutlinedFunc = false;
  if (PrepareForLTO || !DTransInlineHeuristics)
    return false;
  if (ScannedFunctions)
    return SawPreferPartialInlineOutlinedFunc && Candidates.count(&F);
  Module *M = F.getParent();
  for (auto &Fx : M->functions())
    if (Fx.hasFnAttribute("prefer-partial-inline-outlined-func")) {
      SawPreferPartialInlineOutlinedFunc = true;
      for (User *U : Fx.users()) {
        auto CS1 = dyn_cast<CallBase>(U);
        if (!CS1)
          continue;
        Function *Fy = CS1->getCaller();
        if (!Fy->hasFnAttribute("prefer-partial-inline-inlined-clone"))
          continue;
        for (User *V : Fy->users()) {
          auto CS2 = dyn_cast<CallBase>(V);
          if (!CS2)
            continue;
          Function *Fz = CS2->getCaller();
          for (User *W : Fz->users()) {
            auto CS3 = dyn_cast<CallBase>(W);
            if (!CS3 || CS3->getCaller() != &Fx)
              continue;
            Candidates.insert(Fz);
          }
        }
      }
    }
  ScannedFunctions = true;
  return SawPreferPartialInlineOutlinedFunc && Candidates.count(&F);
}

//
// Return 'true' if 'Callee' is marked as
// 'prefer-partial-inline-inlined-clone'. Refer to the example in
// preferPartialInlineOutlinedFunc. This function will return true for @foo.1.
//
static bool preferPartialInlineInlinedClone(Function *Callee) {
  return Callee &&
         Callee->hasFnAttribute("prefer-partial-inline-inlined-clone");
}

//
// Return 'true' if 'F' should be inlined because it is qualified by
// the "dummy args" inlining heuristic, This heuristic qualifies
// functions:
//   (1) That have at least 'DummyArgsMinArgCount' formal arguments.
//   (2) Have a series of formal arguments which are pointers to integers
//       with at least DummyArgsMinSeriesLength arguments in the series.
//   (3) Have at least 'DummyArgsMinCallsiteCount' callsites.
//   (4) All but one of these callsites has a sub-series of matching actual
//       arguments corresponding to the formal arguments in the series in (2),
//       where the sub-series has a length of at least
//       'DummyArgsMinSeriesMatch'.
// For example:
//    int foo(int, float, int*, int*, int*, int*, int*, int)
// has 8 arguments, and a series of 5 int* arguments. Let's say it has
// the following callsites:
//    foo(2, 4.0, &myres1, &dummy, &dummy, &dummy, &dummy, 0);
//    foo(3, 5.0, &myres2, &temp, &temp, &temp, &temp, 0);
//    foo(4, 7.0, &myres3, &dummy, &dummy, &dummy, &dummy, 0);
//    foo(2, 5.0, &myres4, &dummy1, &dummy1, &dummy1, &dummy1, 0);
//    foo(6, 6.0, &myres5, &fred, &george, &mildred, 0);
// All but one of these callsites (the last) have a series of 4 int*
// arguments with the same value.  So 'foo' qualifies for inlining
// under this heuristic. (Note that the idea of matching parameters
// being an indicator of the use of dummy args is only a heuristic.)
//
// Note: This function heuristic is now qualified by a callsite specific
// heuristic below.
//
static bool worthInliningFunctionPassedDummyArgs(Function &F,
                                                 bool PrepareForLTO) {
  assert(DummyArgsMinSeriesLength <= DummyArgsMinArgCount &&
         "Series length must not exceed minimum arg count");
  assert(DummyArgsMinSeriesMatch <= DummyArgsMinSeriesLength &&
         "Series match must not exceed series length");
  static SmallPtrSet<Function *, 3> FunctionsTestedPass;
  if (PrepareForLTO || !DTransInlineHeuristics)
    return false;
  if (FunctionsTestedPass.count(&F))
    return true;
  if (F.arg_size() < DummyArgsMinArgCount)
    return false;
  // Find the series length.
  unsigned PtrToIntSeriesArgCount = 0;
  unsigned PtrToIntFirstArgNo = 0;
  for (auto &Arg : F.args()) {
    Type *PETy = inferPtrElementType(Arg);
    if (PETy && PETy->isIntegerTy()) {
      if (PtrToIntSeriesArgCount == 0)
        PtrToIntFirstArgNo = Arg.getArgNo();
      if (++PtrToIntSeriesArgCount >= DummyArgsMinSeriesLength)
        break;
    } else
      PtrToIntSeriesArgCount = 0;
  }
  if (PtrToIntSeriesArgCount < DummyArgsMinSeriesLength)
    return false;
  // Find number of callsites and the length of the series match within the
  // series.
  unsigned CallSiteCount = 0;
  unsigned BadCallSiteCount = 0;
  for (User *U : F.users()) {
    auto CS = dyn_cast<CallBase>(U);
    if (!CS)
      continue;
    ++CallSiteCount;
    unsigned E = PtrToIntFirstArgNo + PtrToIntSeriesArgCount;
    if (E > CS->arg_size() - 1)
      return false;
    unsigned MaxMatchCount = 0;
    unsigned SeriesCount = 0;
    Value *LastV = CS->getArgOperand(PtrToIntFirstArgNo);
    for (unsigned I = PtrToIntFirstArgNo + 1; I <= E; ++I) {
      Value *V = CS->getArgOperand(I);
      if (V == LastV) {
        if (++SeriesCount > MaxMatchCount)
          MaxMatchCount = SeriesCount;
      } else
        SeriesCount = 0;
      LastV = V;
    }
    if (MaxMatchCount < (DummyArgsMinSeriesMatch - 1) &&
        (++BadCallSiteCount > 1))
      return false;
  }
  if (CallSiteCount < DummyArgsMinCallsiteCount)
    return false;
  // Store the qualified function.
  FunctionsTestedPass.insert(&F);
  return true;
}

//
// Return 'true' if 'CB' should be inlined because its callee qualifies under
// "dummy args" inlining heuristic explained above, and 'CB' itself is used
// in a switch instruction.
//
static bool worthInliningCallSitePassedDummyArgs(CallBase &CB,
                                                 bool PrepareForLTO) {
  Function *Callee = CB.getCalledFunction();
  if (!worthInliningFunctionPassedDummyArgs(*Callee, PrepareForLTO))
    return false;
  for (User *U : CB.users())
    if (dyn_cast<SwitchInst>(U))
      return true;
  return false;
}

//
// Return 'true' if 'Arg' is a special "array struct arg". We are looking for
// an Argument whose type is a pointer to a structure each of whose fields is
// an single dimension array of ints or floats. Also, the lengths of the arrays
// are all the same.
//
static bool isSpecialArrayStructArg(Argument &Arg) {
  Type *PETy = inferPtrElementType(Arg);
  if (!PETy)
    return false;
  auto TyS = dyn_cast<StructType>(PETy);
  if (!TyS)
    return false;
  uint64_t ECount = 0;
  for (unsigned I = 0; I < TyS->getNumElements(); ++I) {
    llvm::Type *TyF = TyS->getElementType(I);
    auto TyFA = dyn_cast<ArrayType>(TyF);
    if (!TyFA)
      return false;
    uint64_t NE = TyFA->getNumElements();
    if (ECount == 0)
      ECount = NE;
    else if (ECount != NE)
      return false;
    llvm::Type *TyFAE = TyFA->getElementType();
    if (!TyFAE->isIntegerTy() && !TyFAE->isFloatingPointTy())
      return false;
  }
  return true;
}

//
// Return 'true' if 'CB' qualifies for inlining under the "array struct args"
// inline heuristic. The heuristic is slightly different, depending on the
// value of 'PrepareForLTO'.
//
// First, we qualify the callee. We add up the number of uses of each special
// "array struct arg". To qualify there must be at least ArrayStructArgMinUses.
//
// All callsites with this callee are qualified in the 'PrepareForLTO' phase.
// In the '!PrepareForLTO' phase, we also check if the caller has at least
// ArrayStructArgMinCallerArgs args.
//
// NOTE: As in other specialized heuristics described above, we scan all of
// the functions once and memorize the first such candidate to save compile
// time.
//
static bool worthInliningForArrayStructArgs(CallBase &CB, bool PrepareForLTO) {
  static bool ScannedFunctions = false;
  static Function *WorthyCallee = nullptr;
  if (!DTransInlineHeuristics)
    return false;
  if (!ScannedFunctions) {
    Module *M = CB.getParent()->getParent()->getParent();
    for (auto &F : M->functions()) {
      unsigned Count = 0;
      for (auto &Arg : F.args())
        if (isSpecialArrayStructArg(Arg))
          Count += std::distance(Arg.use_begin(), Arg.use_end());
      if (Count >= ArrayStructArgMinUses) {
        WorthyCallee = &F;
        break;
      }
    }
    ScannedFunctions = true;
  }
  Function *Callee = CB.getCalledFunction();
  if (Callee != WorthyCallee)
    return false;
  if (PrepareForLTO)
    return true;
  Function *Caller = CB.getCaller();
  auto AB = Caller->arg_begin();
  auto AE = Caller->arg_end();
  unsigned Diff = std::distance(AB, AE);
  return Diff >= ArrayStructArgMinCallerArgs;
}

//
// Return 'true' if 'CB' is preferred for inlining because doing so will
// enable tiling opportunities.
//
static bool preferInlineTileChoice(CallBase &CB) {
  return CB.hasFnAttr("prefer-inline-tile-choice");
}

static bool preferInlineForManyRecursiveCallsSplitting(CallBase &CB) {
  return CB.hasFnAttr("prefer-inline-mrc-split");
}

static bool preferInlineAggressive(CallBase &CB) {
  return CB.hasFnAttr("prefer-inline-aggressive");
}

//
// Return 'true' if 'CB' is worth inlining because it appears in a small
// application. We allow more inlining in small applications because the
// user will generally be more tolerant of longer compile time and less
// tolerant of lower performance.
//
static bool worthInliningForSmallApp(CallBase &CB,
                                     const TargetTransformInfo &CalleeTTI,
                                     InliningLoopInfoCache &ILIC,
                                     WholeProgramInfo *WPI, bool LinkForLTO,
                                     unsigned InlineOptLevel) {
  //
  // Return 'true' if 'CB' has a collection of no less than CandidateMin
  // callsites more than CandidateMax callsites which qualify for inlining
  // under the small application inlining heuristic. Currently, these should
  // be calls to the same Function in the same caller, all of which are in
  // loops. If we return 'true', put those calls into 'CBSet'.
  //
  auto HasSmallAppCBSet = [](CallBase &CB, InliningLoopInfoCache &ILIC,
                             SmallPtrSetImpl<CallBase *> &CBSet,
                             unsigned CandidateMin,
                             unsigned CandidateMax) -> bool {
    unsigned Count = 0;
    Function *Caller = CB.getCaller();
    Function *Callee = CB.getCalledFunction();
    for (User *U : Callee->users()) {
      auto CBX = dyn_cast<CallBase>(U);
      if (!CBX)
        continue;
      Function *CallerX = CBX->getCaller();
      if (CallerX != Caller)
        continue;
      LoopInfo *LI = ILIC.getLI(CallerX);
      if (!LI->getLoopFor(CBX->getParent()))
        continue;
      CBSet.insert(CBX);
      if (++Count > CandidateMax) {
        CBSet.clear();
        return false;
      }
    }
    if (CBSet.size() < CandidateMin) {
      CBSet.clear();
      return false;
    }
    return true;
  };

  //
  // Return 'true' if 'CB0' and 'CB1' match well enough to qualify them as
  // a pair of callsites for inlining under the small application inlining
  // heuristic. Currently, we are looking for callsites that have at least
  // one matching actual parameter and for which all other matching parameters
  // are either both fed by an AllocInst, SubscriptInst, GetElementPtrInst,
  // matching Arguments, ConstantFP or LoadInst where the operands match.
  //
  auto MatchedPair = [](CallBase &CB0, CallBase &CB1) -> bool {
    if (CB0.arg_size() != CB1.arg_size())
      return false;
    bool SawMatch = false;
    for (unsigned I = 0, E = CB0.arg_size(); I < E; I++) {
      Value *V0 = CB0.getArgOperand(I);
      Value *V1 = CB1.getArgOperand(I);
      if (V0 == V1)
        SawMatch = true;
      if (isa<AllocaInst>(V0) && isa<AllocaInst>(V1))
        continue;
      if (isa<SubscriptInst>(V0) && isa<SubscriptInst>(V1))
        continue;
      if (isa<GetElementPtrInst>(V0) && isa<GetElementPtrInst>(V1))
        continue;
      if (isa<Argument>(V0) && V0 == V1)
        continue;
      if (isa<ConstantFP>(V0) && V0 == V1)
        continue;
      if (isa<LoadInst>(V0) && isa<LoadInst>(V1)) {
        auto Op0 = cast<LoadInst>(V0)->getOperand(0);
        auto Op1 = cast<LoadInst>(V1)->getOperand(0);
        if (Op0 == Op1)
          continue;
      }
      return false;
    }
    return SawMatch;
  };

  //
  // Return 'true' if each callsite in 'CBSet' matches the adjacent callsites
  // in the set.
  //
  auto Matches = [&MatchedPair](SmallPtrSetImpl<CallBase *> &CBSet) -> bool {
    for (auto I = CBSet.begin(), E = CBSet.end(), J = I; I != E; I = J) {
      if (++J == E)
        return true;
      if (!MatchedPair(**I, **J))
        return false;
    }
    return true;
  };

  // Main code for worthInliningForSmallApp
  const unsigned CandidateMin = 2;
  const unsigned CandidateMax = 3;
  static SmallPtrSet<CallBase *, 10> QualifiedCallBases;
  if (!passesMinimalSmallAppConditions(CB, CalleeTTI, WPI, LinkForLTO,
                                       InlineOptLevel))
    return false;
  if (QualifiedCallBases.count(&CB))
    return true;
  if (QualifiedCallBases.size() + CandidateMin > SmallAppUserMaxExtraCallBases)
    return false;
  SmallPtrSet<CallBase *, 3> CBSet;
  if (!HasSmallAppCBSet(CB, ILIC, CBSet, CandidateMin, CandidateMax))
    return false;
  if (!Matches(CBSet))
    return false;
  for (CallBase *CBX : CBSet)
    QualifiedCallBases.insert(CBX);
  return true;
}

//
// Return 'true' if we have determined heuristically that 'F' should have a
// function level region in Loop Opt.
//
static bool preferFunctionLevelRegion(Function *F, bool PrepareForLTO,
                                      WholeProgramInfo *WPI) {
  if (!F || PrepareForLTO  || !DTransInlineHeuristics || !WPI ||
      !F->hasOneUse())
    return false;
  auto CB = dyn_cast<CallBase>(*(F->user_begin()));
  if (!CB)
    return false;
  return CB->getCaller() == WPI->getMainFunction();
}

//
// Return the minimum number of dimensions of the arrays accessed by 'Arg'.
// The number of dimensions is determined by counting the number of
// subscripts in a chain starting with 'Arg' down to a LoadInst or StoreInst.
//
static unsigned ArrayDimCount(Argument &Arg) {

  //
  // Return 'true' if 'U' is a LoadInst or StoreInst with 'V' as its
  // PointerOperand. As such, it would terminate a sequence of SubscriptInsts.
  //
  auto IsTerminalUserForValue = [](User *U, Value *V) -> bool {
    if (isa<LoadInst>(U))
      return true;
    auto StI = dyn_cast<StoreInst>(U);
    return StI && StI->getPointerOperand() == V;
  };

  // Use a WorkList to trace from the Arg through all of its uses recursively.
  SmallVector<std::pair<Value *, unsigned>, 8> WorkList;
  WorkList.push_back(std::make_pair(&Arg, 0));
  unsigned DimCount = 0;
  while (!WorkList.empty()) {
    auto WLP = WorkList.pop_back_val();
    Value *V = WLP.first;
    unsigned SCount = WLP.second;
    for (User *U : V->users()) {
      auto SI = dyn_cast<SubscriptInst>(U);
      if (SI && SI->getPointerOperand() == V) {
        WorkList.push_back(std::make_pair(SI, SCount + 1));
        continue;
      }
      // If at the end of a chain, record the first result, or take the
      // minimum of all results up to this point.
      if (IsTerminalUserForValue(U, V)) {
        if (!DimCount || DimCount > SCount)
          DimCount = SCount;
        continue;
      }
      // Ignore a CallBase that may be transmitting the Arg.
      if (isa<CallBase>(U))
        continue;
      // Return a conservative result for anything else.
      return 0;
    }
  }
  return DimCount;
}

//
// Return 'true' if 'Arg' represents an array with at least
// 'ExposeLocalArraysMinDims' dimensions, which is local because it is
// allocated with an AllocaInst.
//
static bool isLocalArrayExposureCandidate(Argument &Arg) {
  // Check if it is an array with enough dimensions.
  if (ArrayDimCount(Arg) < ExposeLocalArraysMinDims)
    return false;
  // Trace its uses up the call chain to find an AllocaInst.
  SmallVector<std::pair<Argument *, unsigned>, 4> WorkList;
  WorkList.push_back(std::make_pair(&Arg, 0));
  bool SawArrayTy = false;
  while (!WorkList.empty()) {
    auto VDP = WorkList.pop_back_val();
    Argument *A = VDP.first;
    unsigned Depth =  VDP.second;
    Function *F = A->getParent();
    for (User *U : F->users()) {
      auto CB = dyn_cast<CallBase>(U);
      // Exclude unusual cases where the address may be taken.
      if (!CB || CB->getCalledFunction() != F)
        return false;
      // Allow the Arg to be used in a GetElementPtrInst when it is passed
      // down to a call.
      Value *AA = CB->getArgOperand(A->getArgNo());
      if (auto GEPI = dyn_cast<GetElementPtrInst>(AA))
        AA = GEPI->getPointerOperand();
      // Found the AllocaInst for which we were looking.
      auto AI = dyn_cast<AllocaInst>(AA);
      if (AI && AI->getAllocatedType()->isArrayTy()) {
        SawArrayTy = true;
        continue;
      }
      // Traverse up the call chain to the calling Function if we haven't
      // gone up too many levels.
      if (auto FA = dyn_cast<Argument>(AA)) {
        if (Depth + 1 > ExposeLocalArraysMaxDepth)
          return false;
        WorkList.push_back(std::make_pair(FA, Depth + 1));
      }
    }
  }
  return SawArrayTy;
}

//
// Mark Functions with "prefer-expose-local-array" whose calls should be
// inlined because inlining will expose local arrays to Loop Opt's array
// contraction optimization.
//
static void localArrayExposureAnalysis(Module &M,
                                       bool PrepareForLTO,
                                       WholeProgramInfo *WPI) {
  //
  // Return 'true' if 'F' meets some minimum criteria for being marked
  // with the "prefer-expose-local-array" attribute.
  //
  auto MeetsMinimalCriteria = [](Function &F) -> bool {
    if (F.isDeclaration() || F.isIntrinsic())
      return false;
    if (F.arg_size() < ExposeLocalArraysMinArgs)
      return false;
    unsigned Count = 0;
    for (User *U : F.users()) {
      auto CB = dyn_cast<CallBase>(U);
      if (CB && CB->getCalledFunction() == &F)
        if (++Count >= ExposeLocalArraysMinCalls)
          return true;
    }
    return false;
  };

  //
  // Propagate the "prefer-expose-local-array" to wrapper Functions that
  // make calls to the main "prefer-expose-local-array" candidate Function.
  //
  auto PropagateAttribute = [](Function &F,
                               SmallPtrSetImpl<Argument *>& ArgSet) {
    for (User *U : F.users()) {
      auto CB = dyn_cast<CallBase>(U);
      if (!CB || CB->getCalledFunction() != &F)
        continue;
      Function *Caller = CB->getCaller();
      // A wrapper Function has a single BasicBlock.
      if (Caller->size() != 1)
        continue;
      if (Caller->hasFnAttribute("prefer-expose-local-arrays"))
        continue;
      for (Argument *A : ArgSet)
        if (isa<Argument>(CB->getArgOperand(A->getArgNo()))) {
          Caller->addFnAttr("prefer-expose-local-arrays");
          break;
        }
    }
  };

  //
  // Test some very minimal conditions.
  //
  if (PrepareForLTO || !DTransInlineHeuristics ||
      !WPI || !WPI->isWholeProgramRead())
    return;

  // Find the best candidate Function for the "prefer-expose-local-arrays"
  // attribute. We limit this to one candidate right now. It can be extended
  // if it is determined to be useful to do so.
  Function *FBest = nullptr;
  SmallPtrSet<Argument *, 2> ArgsBest;
  for (auto &F : M.functions()) {
    if (!F.isFortran())
      continue;
    SmallPtrSet<Argument *, 2> ArgCands;
    if (!MeetsMinimalCriteria(F))
      continue;
    unsigned Count = 0;
    for (auto &Arg : F.args()) {
      if (isLocalArrayExposureCandidate(Arg)) {
        ArgCands.insert(&Arg);
        if (++Count >= ExposeLocalArraysMinArrayArgs) {
          if (FBest)
            return;
          FBest = &F;
          ArgsBest = ArgCands;
          break;
        }
      }
    }
  }
  if (!FBest)
    return;
  // Mark the best candidate Function.
  FBest->addFnAttr("prefer-expose-local-arrays");
  // Mark the wrapper Functions enclosing it.
  PropagateAttribute(*FBest, ArgsBest);
}

//
// Return 'true' if all callsites to 'F' should be inlined because
// that might expose local arrays to Loop Opt's array contraction
// optimization.
//
static bool worthInliningExposesLocalArrays(Function &F,
                                            bool PrepareForLTO,
                                            WholeProgramInfo *WPI) {
  static bool RanArgAnalysis = false;
  // Run the analysis only once at the beginning of inline analysis.
  if (!RanArgAnalysis) {
    localArrayExposureAnalysis(*(F.getParent()), PrepareForLTO, WPI);
    RanArgAnalysis = true;
  }
  return F.hasFnAttribute("prefer-expose-local-arrays");
}

//
// Return 'true' if 'CB' should be inlined because it is under a TBB parallel
// for and therefore likely to be hot.
//
static bool worthInliningUnderTBBParallelFor(CallBase &CB,
                                             bool LinkForLTO) {

  // 
  // Return 'true' if 'F' is a TBB parallel for.
  // We use the demangler to look for a Function with the name:
  //   tbb::detail::d1::start_for<...>::execute(
  //       tbb::detail::d1::execution_data&)
  // for Linux and the name:
  //   public: virtual class tbb::detail::d1::task *__cdecl
  //       tbb::detail::d1::start_for<...>execute(
  //       struct tbb::detail::d1::execution_data &)
  // for Windows.
  //
  auto IsTBBParallelFor = [](Function &F) -> bool {
    std::string Name = demangle(F.getName().str());
    StringRef DemangledName(Name);
    char StartString[] =
#ifndef _WIN32
      "tbb::detail::d1::start_for<";
#else
      "public: virtual class tbb::detail::d1::task * "
          "__cdecl tbb::detail::d1::start_for<";
#endif // _WIN32
    char EndString[] =
#ifndef _WIN32
      ">::execute(tbb::detail::d1::execution_data&)";
#else
      ">::execute(struct tbb::detail::d1::execution_data &)";
#endif // _WIN32
    if (!DemangledName.startswith(StartString))
      return false;
    if (!DemangledName.endswith(EndString))
      return false;
    return true;
  };

  //
  // Return 'true' if 'M' has a TBB parallel for. Also, set the attribute
  // "tbb-parallel-for" on each TBB parallel for.
  //
  auto HasTBBParallelFor = [&IsTBBParallelFor](Module &M) -> bool {
    // To save compile time.
    auto NumFxns = std::distance(M.functions().begin(), M.functions().end());
    if (NumFxns < TBBParallelForMinFuncs)
      return false;
    bool RV = false;
    for (auto &F : M.functions())
      if (IsTBBParallelFor(F)) {
        F.addFnAttr("tbb-parallel-for");
        RV = true;
      }
    return RV;
  };

  //
  // Return 'true' if 'CB and 'CBU' have at least TBBParallelForMinArgTotal
  // actual arguments, have the same number of arguments, and have at least
  // TBBParallelForMinArgMatch matching arguments.
  //
  auto HasMatchingArgs = [](CallBase &CB, CallBase &CBU) -> bool {
    unsigned AS = CB.arg_size();
    if (AS != CBU.arg_size() || AS < TBBParallelForMinArgTotal)
      return false;
    unsigned Count = 0;
    for (unsigned I = 0; I < AS; ++I)
      if (CB.getArgOperand(I) == CBU.getArgOperand(I))
        if (++Count >= TBBParallelForMinArgMatch)
          return true;
    return false;
  };

  //
  // Return 'true' if 'CB' is not more than 'Depth' levels under a TBB
  // parallel for. While traversing up the call graph, limit the fan out
  // from Function to Callbase Users to be no more than TBBParallelForMaxWidth.
  //
  std::function<bool(CallBase &, unsigned)> IsUnderTBBParallelFor =
      [&IsUnderTBBParallelFor](CallBase &CB, unsigned Depth) {
    if (Depth == 0)
      return false;
    unsigned Width = 0;
    Function *Caller = CB.getCaller();
    if (Caller->hasFnAttribute("tbb-parallel-for"))
      return true;
    for (User *U : Caller->users())
      if (auto CBU = dyn_cast<CallBase>(U)) {
        if (++Width > TBBParallelForMaxWidth)
          return false;
        if (IsUnderTBBParallelFor(*CBU, Depth-1))
          return true;
      }
    return false;
  }; 

  //
  // Return 'true' if 'CB' is prefered for inlining under the TBB parallel
  // for heuristic. The caller of 'CB' should have at least
  // TBBParallelForMinCallBaseMatch calls but no more than
  // TBBParallelForMaxCallBaseMatch calls to the callee of 'CB' with
  // matching arguments.
  //
  auto PreferForInlining = [&HasMatchingArgs](CallBase &CB) -> bool {
    CallBase *PatternCB = nullptr;
    Function *PatternCallee = nullptr;
    SmallVector <CallBase *, 4> PreferInlineVector; 
    for (auto &I : instructions(*CB.getCaller()))
      if (auto CBU = dyn_cast<CallBase>(&I))
        if (!isa<IntrinsicInst>(CBU))
          if (auto Callee = CBU->getCalledFunction())
            if (!Callee->isDeclaration()) {
              // Skip candidates that do not have enough args
              if (Callee->arg_size() < TBBParallelForMinArgTotal)
                continue;
              if (!PatternCallee) {
                PatternCallee = Callee;
                PatternCB = CBU;
                PreferInlineVector.push_back(CBU);
              } else if (Callee == PatternCallee &&
                  HasMatchingArgs(*PatternCB, *CBU)) {
                PreferInlineVector.push_back(CBU);
                if (PreferInlineVector.size() > TBBParallelForMaxCallBaseMatch)
                  return false;
              } else {
                return false;
              }
            }
    if (PreferInlineVector.size() < TBBParallelForMinCallBaseMatch)
      return false;
    for (CallBase *CBU : PreferInlineVector)
      CBU->addFnAttr("prefer-inline-tbb");
    return CB.hasFnAttr("prefer-inline-tbb");
  };

  // Main code for worthInliningUnderTBBParallelFor
  static bool MarkedTBBParallelFor = false;
  static bool DoesHaveTBBParallelFor = true;
  if (!LinkForLTO)
    return false;

  if (!MarkedTBBParallelFor) {
    DoesHaveTBBParallelFor = HasTBBParallelFor(*CB.getFunction()->getParent());
    MarkedTBBParallelFor = true;
  }
  if (!DoesHaveTBBParallelFor)
    return false;
  if (CB.hasFnAttr("prefer-inline-tbb"))
    return true;
  if (!IsUnderTBBParallelFor(CB, TBBParallelForMaxDepth))
    return false;
  return PreferForInlining(CB);
}

//
// Test a series of special conditions to determine if it is worth inlining
// if any of them appear. (These have been gathered together into a single
// function to make an early exit easy to accomplish and save compile time.)
// The function returns a bonus that should be applied to the inline cost.
//
extern int intelWorthInlining(CallBase &CB, const InlineParams &Params,
                              TargetLibraryInfo *TLI,
                              const TargetTransformInfo &CalleeTTI,
                              ProfileSummaryInfo *PSI,
                              InliningLoopInfoCache *ILIC,
                              SmallPtrSetImpl<Function *> *QueuedCallers,
                              InlineReasonVector &YesReasonVector,
                              WholeProgramInfo *WPI, bool IsCallerRecursive) {
  bool PrepareForLTO = Params.PrepareForLTO.value_or(false);
  bool LinkForLTO = Params.LinkForLTO.value_or(false);
  unsigned InlineOptLevel = Params.InlineOptLevel.value_or(0);
  Function *F = CB.getCalledFunction();
  if (!F)
    return 0;
  if (isProfInstrumentHotCallSite(CB, PSI, PrepareForLTO)) {
    YesReasonVector.push_back(InlrHotProfile);
    return -InlineConstants::InliningHeuristicBonus;
  }
  if (isDoubleCallSite(CB)) {
    // If there are two calls of the function, the cost of inlining it may
    // drop, but less dramatically.
    if (F->hasLocalLinkage() || F->hasLinkOnceODRLinkage()) {
      if (worthyDoubleInternalCallSite(CB, *ILIC)) {
        YesReasonVector.push_back(InlrDoubleLocalCall);
        return -InlineConstants::SecondToLastCallToStaticBonus;
      }
    }
    bool UseBigBonus = false;
    if (worthyDoubleExternalCallSite(CB, PrepareForLTO, *ILIC,
                                     &UseBigBonus)) {
      YesReasonVector.push_back(InlrDoubleNonLocalCall);
      if (UseBigBonus)
        return -InlineConstants::InliningHeuristicBonus;
      return -InlineConstants::SecondToLastCallToStaticBonus;
    }
  }
  if (worthInliningForFusion(CB, TLI, CalleeTTI, *ILIC, PrepareForLTO)) {
    YesReasonVector.push_back(InlrForFusion);
    Function *Caller = CB.getCaller();
    if (preferFunctionLevelRegion(Caller, PrepareForLTO, WPI))
      if (!Caller->hasFnAttribute("prefer-function-level-region"))
        Caller->addFnAttr("prefer-function-level-region");
    return -InlineConstants::InliningHeuristicBonus;
  }
  if (worthInliningForDeeplyNestedIfs(CB, *ILIC, IsCallerRecursive,
                                      PrepareForLTO)) {
    YesReasonVector.push_back(InlrDeeplyNestedIfs);
    return -InlineConstants::InliningHeuristicBonus;
  }
  if (worthInliningForAddressComputations(CB, *ILIC, PrepareForLTO)) {
    YesReasonVector.push_back(InlrAddressComputations);
    return -InlineConstants::InliningHeuristicBonus;
  }
  if (worthInliningForStackComputations(F, TLI, PrepareForLTO)) {
    YesReasonVector.push_back(InlrStackComputations);
    return -InlineConstants::InliningHeuristicBonus;
    ;
  }
  if (worthInliningSingleBasicBlockWithStructTest(F, PrepareForLTO,
                                                  QueuedCallers)) {
    YesReasonVector.push_back(InlrSingleBasicBlockWithStructTest);
    return -InlineConstants::InliningHeuristicBonus;
    ;
  }
  if (worthInliningForRecProgressionClone(F)) {
    YesReasonVector.push_back(InlrRecProClone);
    return -InlineConstants::InliningHeuristicBonus;
  }
  if (preferPartialInlineHasExtractedRecursiveCall(*F, PrepareForLTO)) {
    YesReasonVector.push_back(InlrHasExtractedRecursiveCall);
    return -InlineConstants::InliningHeuristicBonus;
  }
  if (preferPartialInlineInlinedClone(F)) {
    YesReasonVector.push_back(InlrPreferPartialInline);
    return -InlineConstants::InliningHeuristicBonus;
  }
  if (worthInliningCallSitePassedDummyArgs(CB, PrepareForLTO)) {
    YesReasonVector.push_back(InlrPassedDummyArgs);
    return -InlineConstants::InliningHeuristicBonus;
  }
  if (worthInliningForArrayStructArgs(CB, PrepareForLTO)) {
    YesReasonVector.push_back(InlrArrayStructArgs);
    return -InlineConstants::InliningHeuristicBonus;
  }
  if (preferInlineTileChoice(CB)) {
    YesReasonVector.push_back(InlrPreferTileChoice);
    return -InlineConstants::InliningHeuristicBonus;
  }
  if (preferInlineForManyRecursiveCallsSplitting(CB)) {
    YesReasonVector.push_back(InlrManyRecursiveCallsSplitting);
    return -InlineConstants::InliningHeuristicBonus;
  }
  if (preferInlineAggressive(CB)) {
    YesReasonVector.push_back(InlrAggInline);
    return -InlineConstants::InliningHeuristicBonus;
  }
  if (preferInlineDtrans(CB)) {
    YesReasonVector.push_back(InlrDTransInline);
    return -InlineConstants::InliningHeuristicBonus;
  }
  if (worthInliningForSmallApp(CB, CalleeTTI, *ILIC, WPI, LinkForLTO,
                               InlineOptLevel)) {
    YesReasonVector.push_back(InlrHasSmallAppBudget);
    return -InlineConstants::InliningHeuristicBonus;
  }
  if (worthInliningExposesLocalArrays(*F, PrepareForLTO, WPI)) {
    YesReasonVector.push_back(InlrExposesLocalArrays);
    return -InlineConstants::InliningHeuristicBonus;
  }
  if (worthInliningUnderTBBParallelFor(CB, LinkForLTO)) {
    YesReasonVector.push_back(InlrUnderTBBParallelFor);
    return -InlineConstants::InliningHeuristicBonus;
  }
  return 0;
}

extern bool intelNotHotCallee(Function &F, bool PrepareForLTO) {
  if (!PrepareForLTO)
    return false;
  // Check if 'F' is a multiple callsite function.
  unsigned Count = 0;
  for (User *U : F.users())
    if (isa<CallBase>(U))
      if (++Count > 1)
        break;
  if (Count <= 1)
    return false;
  // Check if some call inside 'F' is to a declaration
  // that appears only once in this translation unit.
  for (auto &I : instructions(F)) {
    auto CB = dyn_cast<CallBase>(&I);
    if (!CB || isa<IntrinsicInst>(CB))
      continue;
    Function *Callee = CB->getCalledFunction();
    if (!Callee || !Callee->isDeclaration())
      continue;
    Count = 0;
    for (User *U : Callee->users())
      if (isa<CallBase>(U))
        if (++Count >= 2)
          break;
    if (Count == 1)
      return true;
  }
  return false;
}

extern bool intelCallTerminatesUnreachable(CallBase &CB) {
  Function *F = CB.getCalledFunction();
  if (!F)
    return false;
  return DTransInlineHeuristics && F->size() == 1 &&
      isa<UnreachableInst>(F->getEntryBlock().getTerminator());
}
#endif // INTEL_FEATURE_SW_ADVANCED
} // end namespace llvm
