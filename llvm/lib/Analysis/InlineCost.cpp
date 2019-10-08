//===- InlineCost.cpp - Cost analysis for inliner -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements inline cost analysis.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/InlineCost.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/CodeMetrics.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Analysis/InstructionSimplify.h"
#include "llvm/Analysis/Intel_AggInline.h"          // INTEL
#include "llvm/Analysis/Intel_IPCloningAnalysis.h"  // INTEL
#include "llvm/Analysis/Intel_PartialInlineAnalysis.h" // INTEL
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/MemoryBuiltins.h"           // INTEL
#include "llvm/Analysis/ProfileSummaryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/GetElementPtrTypeIterator.h"
#include "llvm/IR/GlobalAlias.h"
#include "llvm/IR/InstIterator.h"                   // INTEL
#include "llvm/IR/InstVisitor.h"                    // INTEL
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Support/GenericDomTree.h"            // INTEL
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/Intel_IPCloning.h"    // INTEL
#include <algorithm>                                // INTEL
#include <queue>                                    // INTEL

using namespace llvm;
using namespace InlineReportTypes;  // INTEL
using namespace llvm::PatternMatch; // INTEL

#define DEBUG_TYPE "inline-cost"

STATISTIC(NumCallsAnalyzed, "Number of call sites analyzed");

#if INTEL_CUSTOMIZATION
extern bool llvm::IsInlinedReason(InlineReason Reason) {
  return Reason > InlrFirst && Reason < InlrLast;
}

extern bool llvm::IsNotInlinedReason(InlineReason Reason) {
  return Reason > NinlrFirst && Reason < NinlrLast;
}

static cl::opt<bool> InlineForXmain(
    "inline-for-xmain", cl::Hidden, cl::init(true),
    cl::desc("Xmain customization of inlining"));

static cl::opt<bool> DTransInlineHeuristics(
    "dtrans-inline-heuristics", cl::Hidden, cl::init(false),
    cl::desc("inlining heuristics controlled under -qopt-mem-layout-trans"));

static cl::opt<bool> EnablePreLTOInlineCost(
    "pre-lto-inline-cost", cl::Hidden, cl::init(false),
    cl::desc("Enable pre-LTO inline cost"));

// Threshold to use when optsize is specified (and there is no -inline-limit).
// CQ370998: Reduce the threshold from 75 to 15 to reduce code size.
static cl::opt<int> OptSizeThreshold(
    "inlineoptsize-threshold", cl::Hidden, cl::init(15),
    cl::desc("Threshold for inlining functions with -Os"));
#endif // INTEL_CUSTOMIZATION

static cl::opt<int> InlineThreshold(
    "inline-threshold", cl::Hidden, cl::init(225), cl::ZeroOrMore,
    cl::desc("Control the amount of inlining to perform (default = 225)"));

static cl::opt<int> HintThreshold(
    "inlinehint-threshold", cl::Hidden, cl::init(325), cl::ZeroOrMore, 
    cl::desc("Threshold for inlining functions with inline hint"));

static cl::opt<int>
    ColdCallSiteThreshold("inline-cold-callsite-threshold", cl::Hidden,
                          cl::init(45), cl::ZeroOrMore,
                          cl::desc("Threshold for inlining cold callsites"));

// We introduce this threshold to help performance of instrumentation based
// PGO before we actually hook up inliner with analysis passes such as BPI and
// BFI.
static cl::opt<int> ColdThreshold(
    "inlinecold-threshold", cl::Hidden, cl::init(45), cl::ZeroOrMore, 
    cl::desc("Threshold for inlining functions with cold attribute"));

static cl::opt<int>
    HotCallSiteThreshold("hot-callsite-threshold", cl::Hidden, cl::init(3000),
                         cl::ZeroOrMore,
                         cl::desc("Threshold for hot callsites "));

static cl::opt<int> LocallyHotCallSiteThreshold(
    "locally-hot-callsite-threshold", cl::Hidden, cl::init(525), cl::ZeroOrMore,
    cl::desc("Threshold for locally hot callsites "));

static cl::opt<int> ColdCallSiteRelFreq(
    "cold-callsite-rel-freq", cl::Hidden, cl::init(2), cl::ZeroOrMore,
    cl::desc("Maximum block frequency, expressed as a percentage of caller's "
             "entry frequency, for a callsite to be cold in the absence of "
             "profile information."));

static cl::opt<int> HotCallSiteRelFreq(
    "hot-callsite-rel-freq", cl::Hidden, cl::init(60), cl::ZeroOrMore,
    cl::desc("Minimum block frequency, expressed as a multiple of caller's "
             "entry frequency, for a callsite to be hot in the absence of "
             "profile information."));

static cl::opt<bool> OptComputeFullInlineCost(
    "inline-cost-full", cl::Hidden, cl::init(false), cl::ZeroOrMore,
    cl::desc("Compute the full inline cost of a call site even when the cost "
             "exceeds the threshold."));

#if INTEL_CUSTOMIZATION
// InliningForDeeplyNestedIfs has three possible values(BOU_UNSET is default).
// Use TRUE to force enabling of heuristic. Use FALSE to disable.
static cl::opt<cl::boolOrDefault> InliningForDeeplyNestedIfs(
    "inlining-for-deep-ifs", cl::ReallyHidden,
    cl::desc("Option that enables inlining for deeply nested IFs"));

static cl::opt<unsigned> MinDepthOfNestedIfs(
    "inlining-min-if-depth", cl::init(9), cl::ReallyHidden,
    cl::desc("Minimal depth of IF nest to trigger inlining"));

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

// Options to set the number of formal arguments, basic blocks, and loops
// for a huge function. We may suppress the inlining of functions which
// have these many arguments, basic blocks, and loops, even if they are
// have local linkage and a single call site.

static cl::opt<unsigned> HugeFunctionBasicBlockCount(
    "inlining-huge-bb-count", cl::init(90), cl::ReallyHidden,
    cl::desc("Function with this many basic blocks or more may be huge"));

static cl::opt<unsigned> HugeFunctionArgCount(
    "inlining-huge-arg-count", cl::init(8), cl::ReallyHidden,
    cl::desc("Function with this many arguments or more may be huge"));

static cl::opt<unsigned> HugeFunctionLoopCount(
    "inlining-huge-loop-count", cl::init(11), cl::ReallyHidden,
    cl::desc("Function with this many loops or more may be huge"));

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
    "inlining-dummy-args-min-callsite-count", cl::init(5),
    cl::ReallyHidden,
    cl::desc("Minimum callsite count for dummy-args function"));

#endif // INTEL_CUSTOMIZATION

namespace {

typedef SmallVector<InlineReason,2> InlineReasonVector;  // INTEL

class CallAnalyzer : public InstVisitor<CallAnalyzer, bool> {
  typedef InstVisitor<CallAnalyzer, bool> Base;
  friend class InstVisitor<CallAnalyzer, bool>;

  /// The TargetTransformInfo available for this compilation.
  const TargetTransformInfo &TTI;

  /// Getter for the cache of @llvm.assume intrinsics.
  std::function<AssumptionCache &(Function &)> &GetAssumptionCache;

  /// Getter for BlockFrequencyInfo
  Optional<function_ref<BlockFrequencyInfo &(Function &)>> &GetBFI;

  /// Profile summary information.
  ProfileSummaryInfo *PSI;

  /// The called function.
  Function &F;

  // Cache the DataLayout since we use it a lot.
  const DataLayout &DL;

  /// The OptimizationRemarkEmitter available for this compilation.
  OptimizationRemarkEmitter *ORE;

  /// The candidate callsite being analyzed. Please do not use this to do
  /// analysis in the caller function; we want the inline cost query to be
  /// easily cacheable. Instead, use the cover function paramHasAttr.
  CallBase &CandidateCall;

  /// Tunable parameters that control the analysis.
  const InlineParams &Params;

  /// Upper bound for the inlining cost. Bonuses are being applied to account
  /// for speculative "expected profit" of the inlining decision.
  int Threshold;

  /// Inlining cost measured in abstract units, accounts for all the
  /// instructions expected to be executed for a given function invocation.
  /// Instructions that are statically proven to be dead based on call-site
  /// arguments are not counted here.
  int Cost = 0;

  bool ComputeFullInlineCost;

#if INTEL_CUSTOMIZATION
  /// The cost and the threshold used for early exit during usual
  /// inlining process. Under IntelInlineReportLevel=64  we compute both
  /// "early exit" and real cost of inlining.  A value of INT_MAX indicates
  /// that a value has not yet been seen for these.  They are expected to
  /// be set at the same time, so we only need to test for EarlyExitCost.
  int EarlyExitThreshold;
  int EarlyExitCost;

  TargetLibraryInfo* TLI;
  InliningLoopInfoCache* ILIC;

  // Aggressive Analysis
  InlineAggressiveInfo *AI;
  // Set of candidate call sites for loop fusion
  SmallSet<CallBase *, 20> *CallSitesForFusion;
  // Set of candidate call sites for dtrans.
  SmallSet<Function *, 20> *FuncsForDTrans;
#endif // INTEL_CUSTOMIZATION

  bool IsCallerRecursive = false;
  bool IsRecursiveCall = false;
  bool ExposesReturnsTwice = false;
  bool HasDynamicAlloca = false;
  bool ContainsNoDuplicateCall = false;
  bool HasReturn = false;
  bool HasIndirectBr = false;
  bool HasUninlineableIntrinsic = false;
  bool InitsVargArgs = false;

  /// Number of bytes allocated statically by the callee.
  uint64_t AllocatedSize = 0;
  unsigned NumInstructions = 0;
  unsigned NumVectorInstructions = 0;

  /// Bonus to be applied when percentage of vector instructions in callee is
  /// high (see more details in updateThreshold).
  int VectorBonus = 0;
  /// Bonus to be applied when the callee has only one reachable basic block.
  int SingleBBBonus = 0;

  /// While we walk the potentially-inlined instructions, we build up and
  /// maintain a mapping of simplified values specific to this callsite. The
  /// idea is to propagate any special information we have about arguments to
  /// this call through the inlinable section of the function, and account for
  /// likely simplifications post-inlining. The most important aspect we track
  /// is CFG altering simplifications -- when we prove a basic block dead, that
  /// can cause dramatic shifts in the cost of inlining a function.
  DenseMap<Value *, Constant *> SimplifiedValues;

  /// Keep track of the values which map back (through function arguments) to
  /// allocas on the caller stack which could be simplified through SROA.
  DenseMap<Value *, Value *> SROAArgValues;

  /// The mapping of caller Alloca values to their accumulated cost savings. If
  /// we have to disable SROA for one of the allocas, this tells us how much
  /// cost must be added.
  DenseMap<Value *, int> SROAArgCosts;

  /// Keep track of values which map to a pointer base and constant offset.
  DenseMap<Value *, std::pair<Value *, APInt>> ConstantOffsetPtrs;

  /// Keep track of dead blocks due to the constant arguments.
  SetVector<BasicBlock *> DeadBlocks;

  /// The mapping of the blocks to their known unique successors due to the
  /// constant arguments.
  DenseMap<BasicBlock *, BasicBlock *> KnownSuccessors;

  /// Model the elimination of repeated loads that is expected to happen
  /// whenever we simplify away the stores that would otherwise cause them to be
  /// loads.
  bool EnableLoadElimination;
  SmallPtrSet<Value *, 16> LoadAddrSet;
  int LoadEliminationCost = 0;

  // Custom simplification helper routines.
  bool isAllocaDerivedArg(Value *V);
  bool lookupSROAArgAndCost(Value *V, Value *&Arg,
                            DenseMap<Value *, int>::iterator &CostIt);
  void disableSROA(DenseMap<Value *, int>::iterator CostIt);
  void disableSROA(Value *V);
  void findDeadBlocks(BasicBlock *CurrBB, BasicBlock *NextBB);
  void accumulateSROACost(DenseMap<Value *, int>::iterator CostIt,
                          int InstructionCost);
  void disableLoadElimination();
  bool isGEPFree(GetElementPtrInst &GEP);
  bool canFoldInboundsGEP(GetElementPtrInst &I);
  bool accumulateGEPOffset(GEPOperator &GEP, APInt &Offset);
  bool simplifyCallSite(Function *F, CallBase &Call);
  template <typename Callable>
  bool simplifyInstruction(Instruction &I, Callable Evaluate);
  ConstantInt *stripAndComputeInBoundsConstantOffsets(Value *&V);

  /// Return true if the given argument to the function being considered for
  /// inlining has the given attribute set either at the call site or the
  /// function declaration.  Primarily used to inspect call site specific
  /// attributes since these can be more precise than the ones on the callee
  /// itself.
  bool paramHasAttr(Argument *A, Attribute::AttrKind Attr);

  /// Return true if the given value is known non null within the callee if
  /// inlined through this particular callsite.
  bool isKnownNonNullInCallee(Value *V);

  /// Update Threshold based on callsite properties such as callee
  /// attributes and callee hotness for PGO builds. The Callee is explicitly
  /// passed to support analyzing indirect calls whose target is inferred by
  /// analysis.
  void updateThreshold(CallBase &Call, Function &Callee,  // INTEL
    InlineReasonVector &YesReasonVector);                 // INTEL

  /// Return true if size growth is allowed when inlining the callee at \p Call.
  bool allowSizeGrowth(CallBase &Call);

  /// Return true if \p Call is a cold callsite.
  bool isColdCallSite(CallBase &Call, BlockFrequencyInfo *CallerBFI);

  /// Return a higher threshold if \p Call is a hot callsite.
  Optional<int> getHotCallSiteThreshold(CallBase &Call,
                                        BlockFrequencyInfo *CallerBFI);

  // Custom analysis routines.
  InlineResult analyzeBlock(BasicBlock *BB,
                            SmallPtrSetImpl<const Value *> &EphValues);

  /// Handle a capped 'int' increment for Cost.
  void addCost(int64_t Inc, int64_t UpperBound = INT_MAX) {
    assert(UpperBound > 0 && UpperBound <= INT_MAX && "invalid upper bound");
    Cost = (int)std::min(UpperBound, Cost + Inc);
  }

  // Disable several entry points to the visitor so we don't accidentally use
  // them by declaring but not defining them here.
  void visit(Module *);
  void visit(Module &);
  void visit(Function *);
  void visit(Function &);
  void visit(BasicBlock *);
  void visit(BasicBlock &);

  // Provide base case for our instruction visit.
  bool visitInstruction(Instruction &I);

  // Our visit overrides.
  bool visitAlloca(AllocaInst &I);
  bool visitPHI(PHINode &I);
  bool visitGetElementPtr(GetElementPtrInst &I);
  bool visitBitCast(BitCastInst &I);
  bool visitPtrToInt(PtrToIntInst &I);
  bool visitIntToPtr(IntToPtrInst &I);
  bool visitCastInst(CastInst &I);
  bool visitUnaryInstruction(UnaryInstruction &I);
  bool visitCmpInst(CmpInst &I);
  bool visitSub(BinaryOperator &I);
  bool visitBinaryOperator(BinaryOperator &I);
  bool visitFNeg(UnaryOperator &I);
  bool visitLoad(LoadInst &I);
  bool visitStore(StoreInst &I);
  bool visitExtractValue(ExtractValueInst &I);
  bool visitInsertValue(InsertValueInst &I);
  bool visitCallBase(CallBase &Call);
  bool visitReturnInst(ReturnInst &RI);
  bool visitBranchInst(BranchInst &BI);
  bool visitSelectInst(SelectInst &SI);
  bool visitSwitchInst(SwitchInst &SI);
  bool visitIndirectBrInst(IndirectBrInst &IBI);
  bool visitResumeInst(ResumeInst &RI);
  bool visitCleanupReturnInst(CleanupReturnInst &RI);
  bool visitCatchReturnInst(CatchReturnInst &RI);
  bool visitUnreachableInst(UnreachableInst &I);
#if INTEL_CUSTOMIZATION
  bool preferDTransToInlining(CallBase &CB, bool PrepareForLTO) const;
#endif // INTEL_CUSTOMIZATION

public:
  CallAnalyzer(const TargetTransformInfo &TTI,
               std::function<AssumptionCache &(Function &)> &GetAssumptionCache,
               Optional<function_ref<BlockFrequencyInfo &(Function &)>> &GetBFI,
               ProfileSummaryInfo *PSI, OptimizationRemarkEmitter *ORE,
               Function &Callee, CallBase &Call,   // INTEL
               TargetLibraryInfo *TLI,             // INTEL
               InliningLoopInfoCache *ILIC,        // INTEL
               InlineAggressiveInfo *AI,           // INTEL
               SmallSet<CallBase *, 20> *CSForFusion, // INTEL
               SmallSet<Function *, 20> *FForDTrans, // INTEL
               const InlineParams &Params)
      : TTI(TTI), GetAssumptionCache(GetAssumptionCache), GetBFI(GetBFI),
        PSI(PSI), F(Callee), DL(F.getParent()->getDataLayout()), ORE(ORE),
        CandidateCall(Call), Params(Params), Threshold(Params.DefaultThreshold),
        ComputeFullInlineCost(OptComputeFullInlineCost ||
                              Params.ComputeFullInlineCost || ORE),
#if INTEL_CUSTOMIZATION
        EarlyExitThreshold(INT_MAX), EarlyExitCost(INT_MAX), TLI(TLI),
        ILIC(ILIC), AI(AI), CallSitesForFusion(CSForFusion),
        FuncsForDTrans(FForDTrans),
#endif // INTEL_CUSTOMIZATION
        EnableLoadElimination(true) {}

  InlineResult analyzeCall(CallBase &Call,                       // INTEL
                           const TargetTransformInfo &CalleeTTI, // INTEL
                           InlineReason* Reason);                // INTEL

  int getThreshold() { return Threshold; }
  int getCost() { return Cost; }
  int getEarlyExitThreshold() { return EarlyExitThreshold; }  // INTEL
  int getEarlyExitCost() { return EarlyExitCost; }            // INTEL


  // Keep a bunch of stats about the cost savings found so we can print them
  // out when debugging.
  unsigned NumConstantArgs = 0;
  unsigned NumConstantOffsetPtrArgs = 0;
  unsigned NumAllocaArgs = 0;
  unsigned NumConstantPtrCmps = 0;
  unsigned NumConstantPtrDiffs = 0;
  unsigned NumInstructionsSimplified = 0;
  unsigned SROACostSavings = 0;
  unsigned SROACostSavingsLost = 0;

  void dump();
};

} // namespace

/// Test whether the given value is an Alloca-derived function argument.
bool CallAnalyzer::isAllocaDerivedArg(Value *V) {
  return SROAArgValues.count(V);
}

/// Lookup the SROA-candidate argument and cost iterator which V maps to.
/// Returns false if V does not map to a SROA-candidate.
bool CallAnalyzer::lookupSROAArgAndCost(
    Value *V, Value *&Arg, DenseMap<Value *, int>::iterator &CostIt) {
  if (SROAArgValues.empty() || SROAArgCosts.empty())
    return false;

  DenseMap<Value *, Value *>::iterator ArgIt = SROAArgValues.find(V);
  if (ArgIt == SROAArgValues.end())
    return false;

  Arg = ArgIt->second;
  CostIt = SROAArgCosts.find(Arg);
  return CostIt != SROAArgCosts.end();
}

/// Disable SROA for the candidate marked by this cost iterator.
///
/// This marks the candidate as no longer viable for SROA, and adds the cost
/// savings associated with it back into the inline cost measurement.
void CallAnalyzer::disableSROA(DenseMap<Value *, int>::iterator CostIt) {
  // If we're no longer able to perform SROA we need to undo its cost savings
  // and prevent subsequent analysis.
  addCost(CostIt->second);
  SROACostSavings -= CostIt->second;
  SROACostSavingsLost += CostIt->second;
  SROAArgCosts.erase(CostIt);
  disableLoadElimination();
}

/// If 'V' maps to a SROA candidate, disable SROA for it.
void CallAnalyzer::disableSROA(Value *V) {
  Value *SROAArg;
  DenseMap<Value *, int>::iterator CostIt;
  if (lookupSROAArgAndCost(V, SROAArg, CostIt))
    disableSROA(CostIt);
}

/// Accumulate the given cost for a particular SROA candidate.
void CallAnalyzer::accumulateSROACost(DenseMap<Value *, int>::iterator CostIt,
                                      int InstructionCost) {
  CostIt->second += InstructionCost;
  SROACostSavings += InstructionCost;
}

void CallAnalyzer::disableLoadElimination() {
  if (EnableLoadElimination) {
    addCost(LoadEliminationCost);
    LoadEliminationCost = 0;
    EnableLoadElimination = false;
  }
}

/// Accumulate a constant GEP offset into an APInt if possible.
///
/// Returns false if unable to compute the offset for any reason. Respects any
/// simplified values known during the analysis of this callsite.
bool CallAnalyzer::accumulateGEPOffset(GEPOperator &GEP, APInt &Offset) {
  unsigned IntPtrWidth = DL.getIndexTypeSizeInBits(GEP.getType());
  assert(IntPtrWidth == Offset.getBitWidth());

  for (gep_type_iterator GTI = gep_type_begin(GEP), GTE = gep_type_end(GEP);
       GTI != GTE; ++GTI) {
    ConstantInt *OpC = dyn_cast<ConstantInt>(GTI.getOperand());
    if (!OpC)
      if (Constant *SimpleOp = SimplifiedValues.lookup(GTI.getOperand()))
        OpC = dyn_cast<ConstantInt>(SimpleOp);
    if (!OpC)
      return false;
    if (OpC->isZero())
      continue;

    // Handle a struct index, which adds its field offset to the pointer.
    if (StructType *STy = GTI.getStructTypeOrNull()) {
      unsigned ElementIdx = OpC->getZExtValue();
      const StructLayout *SL = DL.getStructLayout(STy);
      Offset += APInt(IntPtrWidth, SL->getElementOffset(ElementIdx));
      continue;
    }

    APInt TypeSize(IntPtrWidth, DL.getTypeAllocSize(GTI.getIndexedType()));
    Offset += OpC->getValue().sextOrTrunc(IntPtrWidth) * TypeSize;
  }
  return true;
}

/// Use TTI to check whether a GEP is free.
///
/// Respects any simplified values known during the analysis of this callsite.
bool CallAnalyzer::isGEPFree(GetElementPtrInst &GEP) {
  SmallVector<Value *, 4> Operands;
  Operands.push_back(GEP.getOperand(0));
  for (User::op_iterator I = GEP.idx_begin(), E = GEP.idx_end(); I != E; ++I)
    if (Constant *SimpleOp = SimplifiedValues.lookup(*I))
       Operands.push_back(SimpleOp);
     else
       Operands.push_back(*I);
  return TargetTransformInfo::TCC_Free == TTI.getUserCost(&GEP, Operands);
}

bool CallAnalyzer::visitAlloca(AllocaInst &I) {
  // Check whether inlining will turn a dynamic alloca into a static
  // alloca and handle that case.
  if (I.isArrayAllocation()) {
    Constant *Size = SimplifiedValues.lookup(I.getArraySize());
    if (auto *AllocSize = dyn_cast_or_null<ConstantInt>(Size)) {
      Type *Ty = I.getAllocatedType();
      AllocatedSize = SaturatingMultiplyAdd(
          AllocSize->getLimitedValue(), DL.getTypeAllocSize(Ty).getFixedSize(),
          AllocatedSize);
      return Base::visitAlloca(I);
    }
  }

  // Accumulate the allocated size.
  if (I.isStaticAlloca()) {
    Type *Ty = I.getAllocatedType();
    AllocatedSize = SaturatingAdd(DL.getTypeAllocSize(Ty).getFixedSize(),
                                  AllocatedSize);
  }

  // We will happily inline static alloca instructions.
  if (I.isStaticAlloca())
    return Base::visitAlloca(I);

  // FIXME: This is overly conservative. Dynamic allocas are inefficient for
  // a variety of reasons, and so we would like to not inline them into
  // functions which don't currently have a dynamic alloca. This simply
  // disables inlining altogether in the presence of a dynamic alloca.
  HasDynamicAlloca = true;
  return false;
}

bool CallAnalyzer::visitPHI(PHINode &I) {
  // FIXME: We need to propagate SROA *disabling* through phi nodes, even
  // though we don't want to propagate it's bonuses. The idea is to disable
  // SROA if it *might* be used in an inappropriate manner.

  // Phi nodes are always zero-cost.
  // FIXME: Pointer sizes may differ between different address spaces, so do we
  // need to use correct address space in the call to getPointerSizeInBits here?
  // Or could we skip the getPointerSizeInBits call completely? As far as I can
  // see the ZeroOffset is used as a dummy value, so we can probably use any
  // bit width for the ZeroOffset?
  APInt ZeroOffset = APInt::getNullValue(DL.getPointerSizeInBits(0));
  bool CheckSROA = I.getType()->isPointerTy();

  // Track the constant or pointer with constant offset we've seen so far.
  Constant *FirstC = nullptr;
  std::pair<Value *, APInt> FirstBaseAndOffset = {nullptr, ZeroOffset};
  Value *FirstV = nullptr;

  for (unsigned i = 0, e = I.getNumIncomingValues(); i != e; ++i) {
    BasicBlock *Pred = I.getIncomingBlock(i);
    // If the incoming block is dead, skip the incoming block.
    if (DeadBlocks.count(Pred))
      continue;
    // If the parent block of phi is not the known successor of the incoming
    // block, skip the incoming block.
    BasicBlock *KnownSuccessor = KnownSuccessors[Pred];
    if (KnownSuccessor && KnownSuccessor != I.getParent())
      continue;

    Value *V = I.getIncomingValue(i);
    // If the incoming value is this phi itself, skip the incoming value.
    if (&I == V)
      continue;

    Constant *C = dyn_cast<Constant>(V);
    if (!C)
      C = SimplifiedValues.lookup(V);

    std::pair<Value *, APInt> BaseAndOffset = {nullptr, ZeroOffset};
    if (!C && CheckSROA)
      BaseAndOffset = ConstantOffsetPtrs.lookup(V);

    if (!C && !BaseAndOffset.first)
      // The incoming value is neither a constant nor a pointer with constant
      // offset, exit early.
      return true;

    if (FirstC) {
      if (FirstC == C)
        // If we've seen a constant incoming value before and it is the same
        // constant we see this time, continue checking the next incoming value.
        continue;
      // Otherwise early exit because we either see a different constant or saw
      // a constant before but we have a pointer with constant offset this time.
      return true;
    }

    if (FirstV) {
      // The same logic as above, but check pointer with constant offset here.
      if (FirstBaseAndOffset == BaseAndOffset)
        continue;
      return true;
    }

    if (C) {
      // This is the 1st time we've seen a constant, record it.
      FirstC = C;
      continue;
    }

    // The remaining case is that this is the 1st time we've seen a pointer with
    // constant offset, record it.
    FirstV = V;
    FirstBaseAndOffset = BaseAndOffset;
  }

  // Check if we can map phi to a constant.
  if (FirstC) {
    SimplifiedValues[&I] = FirstC;
    return true;
  }

  // Check if we can map phi to a pointer with constant offset.
  if (FirstBaseAndOffset.first) {
    ConstantOffsetPtrs[&I] = FirstBaseAndOffset;

    Value *SROAArg;
    DenseMap<Value *, int>::iterator CostIt;
    if (lookupSROAArgAndCost(FirstV, SROAArg, CostIt))
      SROAArgValues[&I] = SROAArg;
  }

  return true;
}

/// Check we can fold GEPs of constant-offset call site argument pointers.
/// This requires target data and inbounds GEPs.
///
/// \return true if the specified GEP can be folded.
bool CallAnalyzer::canFoldInboundsGEP(GetElementPtrInst &I) {
  // Check if we have a base + offset for the pointer.
  std::pair<Value *, APInt> BaseAndOffset =
      ConstantOffsetPtrs.lookup(I.getPointerOperand());
  if (!BaseAndOffset.first)
    return false;

  // Check if the offset of this GEP is constant, and if so accumulate it
  // into Offset.
  if (!accumulateGEPOffset(cast<GEPOperator>(I), BaseAndOffset.second))
    return false;

  // Add the result as a new mapping to Base + Offset.
  ConstantOffsetPtrs[&I] = BaseAndOffset;

  return true;
}

bool CallAnalyzer::visitGetElementPtr(GetElementPtrInst &I) {
  Value *SROAArg;
  DenseMap<Value *, int>::iterator CostIt;
  bool SROACandidate =
      lookupSROAArgAndCost(I.getPointerOperand(), SROAArg, CostIt);

  // Lambda to check whether a GEP's indices are all constant.
  auto IsGEPOffsetConstant = [&](GetElementPtrInst &GEP) {
    for (User::op_iterator I = GEP.idx_begin(), E = GEP.idx_end(); I != E; ++I)
      if (!isa<Constant>(*I) && !SimplifiedValues.lookup(*I))
        return false;
    return true;
  };

  if ((I.isInBounds() && canFoldInboundsGEP(I)) || IsGEPOffsetConstant(I)) {
    if (SROACandidate)
      SROAArgValues[&I] = SROAArg;

    // Constant GEPs are modeled as free.
    return true;
  }

  // Variable GEPs will require math and will disable SROA.
  if (SROACandidate)
    disableSROA(CostIt);
  return isGEPFree(I);
}

/// Simplify \p I if its operands are constants and update SimplifiedValues.
/// \p Evaluate is a callable specific to instruction type that evaluates the
/// instruction when all the operands are constants.
template <typename Callable>
bool CallAnalyzer::simplifyInstruction(Instruction &I, Callable Evaluate) {
  SmallVector<Constant *, 2> COps;
  for (Value *Op : I.operands()) {
    Constant *COp = dyn_cast<Constant>(Op);
    if (!COp)
      COp = SimplifiedValues.lookup(Op);
    if (!COp)
      return false;
    COps.push_back(COp);
  }
  auto *C = Evaluate(COps);
  if (!C)
    return false;
  SimplifiedValues[&I] = C;
  return true;
}

bool CallAnalyzer::visitBitCast(BitCastInst &I) {
  // Propagate constants through bitcasts.
  if (simplifyInstruction(I, [&](SmallVectorImpl<Constant *> &COps) {
        return ConstantExpr::getBitCast(COps[0], I.getType());
      }))
    return true;

  // Track base/offsets through casts
  std::pair<Value *, APInt> BaseAndOffset =
      ConstantOffsetPtrs.lookup(I.getOperand(0));
  // Casts don't change the offset, just wrap it up.
  if (BaseAndOffset.first)
    ConstantOffsetPtrs[&I] = BaseAndOffset;

  // Also look for SROA candidates here.
  Value *SROAArg;
  DenseMap<Value *, int>::iterator CostIt;
  if (lookupSROAArgAndCost(I.getOperand(0), SROAArg, CostIt))
    SROAArgValues[&I] = SROAArg;

  // Bitcasts are always zero cost.
  return true;
}

bool CallAnalyzer::visitPtrToInt(PtrToIntInst &I) {
  // Propagate constants through ptrtoint.
  if (simplifyInstruction(I, [&](SmallVectorImpl<Constant *> &COps) {
        return ConstantExpr::getPtrToInt(COps[0], I.getType());
      }))
    return true;

  // Track base/offset pairs when converted to a plain integer provided the
  // integer is large enough to represent the pointer.
  unsigned IntegerSize = I.getType()->getScalarSizeInBits();
  unsigned AS = I.getOperand(0)->getType()->getPointerAddressSpace();
  if (IntegerSize >= DL.getPointerSizeInBits(AS)) {
    std::pair<Value *, APInt> BaseAndOffset =
        ConstantOffsetPtrs.lookup(I.getOperand(0));
    if (BaseAndOffset.first)
      ConstantOffsetPtrs[&I] = BaseAndOffset;
  }

  // This is really weird. Technically, ptrtoint will disable SROA. However,
  // unless that ptrtoint is *used* somewhere in the live basic blocks after
  // inlining, it will be nuked, and SROA should proceed. All of the uses which
  // would block SROA would also block SROA if applied directly to a pointer,
  // and so we can just add the integer in here. The only places where SROA is
  // preserved either cannot fire on an integer, or won't in-and-of themselves
  // disable SROA (ext) w/o some later use that we would see and disable.
  Value *SROAArg;
  DenseMap<Value *, int>::iterator CostIt;
  if (lookupSROAArgAndCost(I.getOperand(0), SROAArg, CostIt))
    SROAArgValues[&I] = SROAArg;

  return TargetTransformInfo::TCC_Free == TTI.getUserCost(&I);
}

bool CallAnalyzer::visitIntToPtr(IntToPtrInst &I) {
  // Propagate constants through ptrtoint.
  if (simplifyInstruction(I, [&](SmallVectorImpl<Constant *> &COps) {
        return ConstantExpr::getIntToPtr(COps[0], I.getType());
      }))
    return true;

  // Track base/offset pairs when round-tripped through a pointer without
  // modifications provided the integer is not too large.
  Value *Op = I.getOperand(0);
  unsigned IntegerSize = Op->getType()->getScalarSizeInBits();
  if (IntegerSize <= DL.getPointerTypeSizeInBits(I.getType())) {
    std::pair<Value *, APInt> BaseAndOffset = ConstantOffsetPtrs.lookup(Op);
    if (BaseAndOffset.first)
      ConstantOffsetPtrs[&I] = BaseAndOffset;
  }

  // "Propagate" SROA here in the same manner as we do for ptrtoint above.
  Value *SROAArg;
  DenseMap<Value *, int>::iterator CostIt;
  if (lookupSROAArgAndCost(Op, SROAArg, CostIt))
    SROAArgValues[&I] = SROAArg;

  return TargetTransformInfo::TCC_Free == TTI.getUserCost(&I);
}

bool CallAnalyzer::visitCastInst(CastInst &I) {
  // Propagate constants through casts.
  if (simplifyInstruction(I, [&](SmallVectorImpl<Constant *> &COps) {
        return ConstantExpr::getCast(I.getOpcode(), COps[0], I.getType());
      }))
    return true;

  // Disable SROA in the face of arbitrary casts we don't whitelist elsewhere.
  disableSROA(I.getOperand(0));

  // If this is a floating-point cast, and the target says this operation
  // is expensive, this may eventually become a library call. Treat the cost
  // as such.
  switch (I.getOpcode()) {
  case Instruction::FPTrunc:
  case Instruction::FPExt:
  case Instruction::UIToFP:
  case Instruction::SIToFP:
  case Instruction::FPToUI:
  case Instruction::FPToSI:
    if (TTI.getFPOpCost(I.getType()) == TargetTransformInfo::TCC_Expensive)
      addCost(InlineConstants::CallPenalty);
    break;
  default:
    break;
  }

  return TargetTransformInfo::TCC_Free == TTI.getUserCost(&I);
}

bool CallAnalyzer::visitUnaryInstruction(UnaryInstruction &I) {
  Value *Operand = I.getOperand(0);
  if (simplifyInstruction(I, [&](SmallVectorImpl<Constant *> &COps) {
        return ConstantFoldInstOperands(&I, COps[0], DL);
      }))
    return true;

  // Disable any SROA on the argument to arbitrary unary instructions.
  disableSROA(Operand);

  return false;
}

bool CallAnalyzer::paramHasAttr(Argument *A, Attribute::AttrKind Attr) {
  return CandidateCall.paramHasAttr(A->getArgNo(), Attr);
}

bool CallAnalyzer::isKnownNonNullInCallee(Value *V) {
  // Does the *call site* have the NonNull attribute set on an argument?  We
  // use the attribute on the call site to memoize any analysis done in the
  // caller. This will also trip if the callee function has a non-null
  // parameter attribute, but that's a less interesting case because hopefully
  // the callee would already have been simplified based on that.
  if (Argument *A = dyn_cast<Argument>(V))
    if (paramHasAttr(A, Attribute::NonNull))
      return true;

  // Is this an alloca in the caller?  This is distinct from the attribute case
  // above because attributes aren't updated within the inliner itself and we
  // always want to catch the alloca derived case.
  if (isAllocaDerivedArg(V))
    // We can actually predict the result of comparisons between an
    // alloca-derived value and null. Note that this fires regardless of
    // SROA firing.
    return true;

  return false;
}

bool CallAnalyzer::allowSizeGrowth(CallBase &Call) {
  // If the normal destination of the invoke or the parent block of the call
  // site is unreachable-terminated, there is little point in inlining this
  // unless there is literally zero cost.
  // FIXME: Note that it is possible that an unreachable-terminated block has a
  // hot entry. For example, in below scenario inlining hot_call_X() may be
  // beneficial :
  // main() {
  //   hot_call_1();
  //   ...
  //   hot_call_N()
  //   exit(0);
  // }
  // For now, we are not handling this corner case here as it is rare in real
  // code. In future, we should elaborate this based on BPI and BFI in more
  // general threshold adjusting heuristics in updateThreshold().
  if (InvokeInst *II = dyn_cast<InvokeInst>(&Call)) {
    if (isa<UnreachableInst>(II->getNormalDest()->getTerminator()))
      return false;
  } else if (isa<UnreachableInst>(Call.getParent()->getTerminator()))
    return false;

  return true;
}

bool CallAnalyzer::isColdCallSite(CallBase &Call,
                                  BlockFrequencyInfo *CallerBFI) {
  // If global profile summary is available, then callsite's coldness is
  // determined based on that.
  if (PSI && PSI->hasProfileSummary())
    return PSI->isColdCallSite(CallSite(&Call), CallerBFI);

  // Otherwise we need BFI to be available.
  if (!CallerBFI)
    return false;

  // Determine if the callsite is cold relative to caller's entry. We could
  // potentially cache the computation of scaled entry frequency, but the added
  // complexity is not worth it unless this scaling shows up high in the
  // profiles.
  const BranchProbability ColdProb(ColdCallSiteRelFreq, 100);
  auto CallSiteBB = Call.getParent();
  auto CallSiteFreq = CallerBFI->getBlockFreq(CallSiteBB);
  auto CallerEntryFreq =
      CallerBFI->getBlockFreq(&(Call.getCaller()->getEntryBlock()));
  return CallSiteFreq < CallerEntryFreq * ColdProb;
}

Optional<int>
CallAnalyzer::getHotCallSiteThreshold(CallBase &Call,
                                      BlockFrequencyInfo *CallerBFI) {

  // If global profile summary is available, then callsite's hotness is
  // determined based on that.
  if (PSI && PSI->hasProfileSummary() &&
      PSI->isHotCallSite(CallSite(&Call), CallerBFI))
    return Params.HotCallSiteThreshold;

  // Otherwise we need BFI to be available and to have a locally hot callsite
  // threshold.
  if (!CallerBFI || !Params.LocallyHotCallSiteThreshold)
    return None;

  // Determine if the callsite is hot relative to caller's entry. We could
  // potentially cache the computation of scaled entry frequency, but the added
  // complexity is not worth it unless this scaling shows up high in the
  // profiles.
  auto CallSiteBB = Call.getParent();
  auto CallSiteFreq = CallerBFI->getBlockFreq(CallSiteBB).getFrequency();
  auto CallerEntryFreq = CallerBFI->getEntryFreq();
  if (CallSiteFreq >= CallerEntryFreq * HotCallSiteRelFreq)
    return Params.LocallyHotCallSiteThreshold;

  // Otherwise treat it normally.
  return None;
}

void CallAnalyzer::updateThreshold(CallBase &Call, Function &Callee, // INTEL
  InlineReasonVector &YesReasonVector) {                             // INTEL

#if INTEL_CUSTOMIZATION
  // A function can be considered huge if it has too many formal arguments,
  // basic blocks, and loops. We test them in this order (from cheapest to
  // most expensive).
  auto IsHugeFunction = [this](Function *F) {
    if (!InlineForXmain || !DTransInlineHeuristics)
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
  };
#endif // INTEL_CUSTOMIZATION

  // If no size growth is allowed for this inlining, set Threshold to 0.
  if (!allowSizeGrowth(Call)) {
    Threshold = 0;
    return;
  }

  Function *Caller = Call.getCaller();

  // return min(A, B) if B is valid.
  auto MinIfValid = [](int A, Optional<int> B) {
    return B ? std::min(A, B.getValue()) : A;
  };

  // return max(A, B) if B is valid.
  auto MaxIfValid = [](int A, Optional<int> B) {
    return B ? std::max(A, B.getValue()) : A;
  };

  // Various bonus percentages. These are multiplied by Threshold to get the
  // bonus values.
  // SingleBBBonus: This bonus is applied if the callee has a single reachable
  // basic block at the given callsite context. This is speculatively applied
  // and withdrawn if more than one basic block is seen.
  //
  // LstCallToStaticBonus: This large bonus is applied to ensure the inlining
  // of the last call to a static function as inlining such functions is
  // guaranteed to reduce code size.
  //
  // These bonus percentages may be set to 0 based on properties of the caller
  // and the callsite.
  int SingleBBBonusPercent = 50;
  int VectorBonusPercent = TTI.getInlinerVectorBonusPercent();
  int LastCallToStaticBonus = InlineConstants::LastCallToStaticBonus;

  // Lambda to set all the above bonus and bonus percentages to 0.
  auto DisallowAllBonuses = [&]() {
    SingleBBBonusPercent = 0;
    VectorBonusPercent = 0;
    LastCallToStaticBonus = 0;
  };

  // Use the OptMinSizeThreshold or OptSizeThreshold knob if they are available
  // and reduce the threshold if the caller has the necessary attribute.
  if (Caller->hasMinSize()) {
    Threshold = MinIfValid(Threshold, Params.OptMinSizeThreshold);
    // For minsize, we want to disable the single BB bonus and the vector
    // bonuses, but not the last-call-to-static bonus. Inlining the last call to
    // a static function will, at the minimum, eliminate the parameter setup and
    // call/return instructions.
    SingleBBBonusPercent = 0;
    VectorBonusPercent = 0;
  } else if (Caller->hasOptSize())
    Threshold = MinIfValid(Threshold, Params.OptSizeThreshold);

  // Adjust the threshold based on inlinehint attribute and profile based
  // hotness information if the caller does not have MinSize attribute.
  if (!Caller->hasMinSize()) {
    if (Callee.hasFnAttribute(Attribute::InlineHint) ||    // INTEL
        Call.hasFnAttr(Attribute::InlineHint) ||             // INTEL
        Call.hasFnAttr("inline-hint-recursive"))             // INTEL
      Threshold = MaxIfValid(Threshold, Params.HintThreshold);

    // FIXME: After switching to the new passmanager, simplify the logic below
    // by checking only the callsite hotness/coldness as we will reliably
    // have local profile information.
    //
    // Callsite hotness and coldness can be determined if sample profile is
    // used (which adds hotness metadata to calls) or if caller's
    // BlockFrequencyInfo is available.
    BlockFrequencyInfo *CallerBFI = GetBFI ? &((*GetBFI)(*Caller)) : nullptr;
    auto HotCallSiteThreshold = getHotCallSiteThreshold(Call, CallerBFI);
    if (!Caller->hasOptSize() && HotCallSiteThreshold) {
      LLVM_DEBUG(dbgs() << "Hot callsite.\n");
      // FIXME: This should update the threshold only if it exceeds the
      // current threshold, but AutoFDO + ThinLTO currently relies on this
      // behavior to prevent inlining of hot callsites during ThinLTO
      // compile phase.
      Threshold = HotCallSiteThreshold.getValue();
    } else if (isColdCallSite(Call, CallerBFI)) {
      LLVM_DEBUG(dbgs() << "Cold callsite.\n");
      // Do not apply bonuses for a cold callsite including the
      // LastCallToStatic bonus. While this bonus might result in code size
      // reduction, it can cause the size of a non-cold caller to increase
      // preventing it from being inlined.
      DisallowAllBonuses();
      Threshold = MinIfValid(Threshold, Params.ColdCallSiteThreshold);
    } else if (PSI) {
      // Use callee's global profile information only if we have no way of
      // determining this via callsite information.
      if (PSI->isFunctionEntryHot(&Callee)) {
        LLVM_DEBUG(dbgs() << "Hot callee.\n");
        // If callsite hotness can not be determined, we may still know
        // that the callee is hot and treat it as a weaker hint for threshold
        // increase.
        Threshold = MaxIfValid(Threshold, Params.HintThreshold);
      } else if (PSI->isFunctionEntryCold(&Callee)) {
        LLVM_DEBUG(dbgs() << "Cold callee.\n");
        // Do not apply bonuses for a cold callee including the
        // LastCallToStatic bonus. While this bonus might result in code size
        // reduction, it can cause the size of a non-cold caller to increase
        // preventing it from being inlined.
        DisallowAllBonuses();
        Threshold = MinIfValid(Threshold, Params.ColdThreshold);
      }
    }
  }

  // Finally, take the target-specific inlining threshold multiplier into
  // account.
  Threshold *= TTI.getInliningThresholdMultiplier();

  SingleBBBonus = Threshold * SingleBBBonusPercent / 100;
  VectorBonus = Threshold * VectorBonusPercent / 100;

  bool OnlyOneCallAndLocalLinkage =
       (F.hasLocalLinkage()                                     // INTEL
         || (InlineForXmain && F.hasLinkOnceODRLinkage())) &&   // INTEL
       F.hasOneUse() && &F == Call.getCalledFunction() &&       // INTEL
       !IsHugeFunction(&F);                                     // INTEL
  // If there is only one call of the function, and it has internal linkage,
  // the cost of inlining it drops dramatically. It may seem odd to update
  // Cost in updateThreshold, but the bonus depends on the logic in this method.
  // INTEL CQ370998: Added link once ODR linkage case.
  if (OnlyOneCallAndLocalLinkage) {                 // INTEL
    Cost -= LastCallToStaticBonus;
    YesReasonVector.push_back(InlrSingleLocalCall); // INTEL
  }                                                 // INTEL
}

bool CallAnalyzer::visitCmpInst(CmpInst &I) {
  Value *LHS = I.getOperand(0), *RHS = I.getOperand(1);
  // First try to handle simplified comparisons.
  if (simplifyInstruction(I, [&](SmallVectorImpl<Constant *> &COps) {
        return ConstantExpr::getCompare(I.getPredicate(), COps[0], COps[1]);
      }))
    return true;

  if (I.getOpcode() == Instruction::FCmp)
    return false;

  // Otherwise look for a comparison between constant offset pointers with
  // a common base.
  Value *LHSBase, *RHSBase;
  APInt LHSOffset, RHSOffset;
  std::tie(LHSBase, LHSOffset) = ConstantOffsetPtrs.lookup(LHS);
  if (LHSBase) {
    std::tie(RHSBase, RHSOffset) = ConstantOffsetPtrs.lookup(RHS);
    if (RHSBase && LHSBase == RHSBase) {
      // We have common bases, fold the icmp to a constant based on the
      // offsets.
      Constant *CLHS = ConstantInt::get(LHS->getContext(), LHSOffset);
      Constant *CRHS = ConstantInt::get(RHS->getContext(), RHSOffset);
      if (Constant *C = ConstantExpr::getICmp(I.getPredicate(), CLHS, CRHS)) {
        SimplifiedValues[&I] = C;
        ++NumConstantPtrCmps;
        return true;
      }
    }
  }

  // If the comparison is an equality comparison with null, we can simplify it
  // if we know the value (argument) can't be null
  if (I.isEquality() && isa<ConstantPointerNull>(I.getOperand(1)) &&
      isKnownNonNullInCallee(I.getOperand(0))) {
    bool IsNotEqual = I.getPredicate() == CmpInst::ICMP_NE;
    SimplifiedValues[&I] = IsNotEqual ? ConstantInt::getTrue(I.getType())
                                      : ConstantInt::getFalse(I.getType());
    return true;
  }
  // Finally check for SROA candidates in comparisons.
  Value *SROAArg;
  DenseMap<Value *, int>::iterator CostIt;
  if (lookupSROAArgAndCost(I.getOperand(0), SROAArg, CostIt)) {
    if (isa<ConstantPointerNull>(I.getOperand(1))) {
      accumulateSROACost(CostIt, InlineConstants::InstrCost);
      return true;
    }

    disableSROA(CostIt);
  }

  return false;
}

bool CallAnalyzer::visitSub(BinaryOperator &I) {
  // Try to handle a special case: we can fold computing the difference of two
  // constant-related pointers.
  Value *LHS = I.getOperand(0), *RHS = I.getOperand(1);
  Value *LHSBase, *RHSBase;
  APInt LHSOffset, RHSOffset;
  std::tie(LHSBase, LHSOffset) = ConstantOffsetPtrs.lookup(LHS);
  if (LHSBase) {
    std::tie(RHSBase, RHSOffset) = ConstantOffsetPtrs.lookup(RHS);
    if (RHSBase && LHSBase == RHSBase) {
      // We have common bases, fold the subtract to a constant based on the
      // offsets.
      Constant *CLHS = ConstantInt::get(LHS->getContext(), LHSOffset);
      Constant *CRHS = ConstantInt::get(RHS->getContext(), RHSOffset);
      if (Constant *C = ConstantExpr::getSub(CLHS, CRHS)) {
        SimplifiedValues[&I] = C;
        ++NumConstantPtrDiffs;
        return true;
      }
    }
  }

  // Otherwise, fall back to the generic logic for simplifying and handling
  // instructions.
  return Base::visitSub(I);
}

bool CallAnalyzer::visitBinaryOperator(BinaryOperator &I) {
  Value *LHS = I.getOperand(0), *RHS = I.getOperand(1);
  Constant *CLHS = dyn_cast<Constant>(LHS);
  if (!CLHS)
    CLHS = SimplifiedValues.lookup(LHS);
  Constant *CRHS = dyn_cast<Constant>(RHS);
  if (!CRHS)
    CRHS = SimplifiedValues.lookup(RHS);

  Value *SimpleV = nullptr;
  if (auto FI = dyn_cast<FPMathOperator>(&I))
    SimpleV = SimplifyBinOp(I.getOpcode(), CLHS ? CLHS : LHS,
                            CRHS ? CRHS : RHS, FI->getFastMathFlags(), DL);
  else
    SimpleV =
        SimplifyBinOp(I.getOpcode(), CLHS ? CLHS : LHS, CRHS ? CRHS : RHS, DL);

  if (Constant *C = dyn_cast_or_null<Constant>(SimpleV))
    SimplifiedValues[&I] = C;

  if (SimpleV)
    return true;

  // Disable any SROA on arguments to arbitrary, unsimplified binary operators.
  disableSROA(LHS);
  disableSROA(RHS);

  // If the instruction is floating point, and the target says this operation
  // is expensive, this may eventually become a library call. Treat the cost
  // as such. Unless it's fneg which can be implemented with an xor.
  using namespace llvm::PatternMatch;
  if (I.getType()->isFloatingPointTy() &&
      TTI.getFPOpCost(I.getType()) == TargetTransformInfo::TCC_Expensive &&
      !match(&I, m_FNeg(m_Value())))
    addCost(InlineConstants::CallPenalty);

  return false;
}

bool CallAnalyzer::visitFNeg(UnaryOperator &I) {
  Value *Op = I.getOperand(0);
  Constant *COp = dyn_cast<Constant>(Op);
  if (!COp)
    COp = SimplifiedValues.lookup(Op);

  Value *SimpleV = SimplifyFNegInst(COp ? COp : Op,
                                    cast<FPMathOperator>(I).getFastMathFlags(),
                                    DL);

  if (Constant *C = dyn_cast_or_null<Constant>(SimpleV))
    SimplifiedValues[&I] = C;

  if (SimpleV)
    return true;

  // Disable any SROA on arguments to arbitrary, unsimplified fneg.
  disableSROA(Op);

  return false;
}

bool CallAnalyzer::visitLoad(LoadInst &I) {
  Value *SROAArg;
  DenseMap<Value *, int>::iterator CostIt;
  if (lookupSROAArgAndCost(I.getPointerOperand(), SROAArg, CostIt)) {
    if (I.isSimple()) {
      accumulateSROACost(CostIt, InlineConstants::InstrCost);
      return true;
    }

    disableSROA(CostIt);
  }

  // If the data is already loaded from this address and hasn't been clobbered
  // by any stores or calls, this load is likely to be redundant and can be
  // eliminated.
  if (EnableLoadElimination &&
      !LoadAddrSet.insert(I.getPointerOperand()).second && I.isUnordered()) {
    LoadEliminationCost += InlineConstants::InstrCost;
    return true;
  }

  return false;
}

bool CallAnalyzer::visitStore(StoreInst &I) {
  Value *SROAArg;
  DenseMap<Value *, int>::iterator CostIt;
  if (lookupSROAArgAndCost(I.getPointerOperand(), SROAArg, CostIt)) {
    if (I.isSimple()) {
      accumulateSROACost(CostIt, InlineConstants::InstrCost);
      return true;
    }

    disableSROA(CostIt);
  }

  // The store can potentially clobber loads and prevent repeated loads from
  // being eliminated.
  // FIXME:
  // 1. We can probably keep an initial set of eliminatable loads substracted
  // from the cost even when we finally see a store. We just need to disable
  // *further* accumulation of elimination savings.
  // 2. We should probably at some point thread MemorySSA for the callee into
  // this and then use that to actually compute *really* precise savings.
  disableLoadElimination();
  return false;
}

bool CallAnalyzer::visitExtractValue(ExtractValueInst &I) {
  // Constant folding for extract value is trivial.
  if (simplifyInstruction(I, [&](SmallVectorImpl<Constant *> &COps) {
        return ConstantExpr::getExtractValue(COps[0], I.getIndices());
      }))
    return true;

  // SROA can look through these but give them a cost.
  return false;
}

bool CallAnalyzer::visitInsertValue(InsertValueInst &I) {
  // Constant folding for insert value is trivial.
  if (simplifyInstruction(I, [&](SmallVectorImpl<Constant *> &COps) {
        return ConstantExpr::getInsertValue(/*AggregateOperand*/ COps[0],
                                            /*InsertedValueOperand*/ COps[1],
                                            I.getIndices());
      }))
    return true;

  // SROA can look through these but give them a cost.
  return false;
}

/// Try to simplify a call site.
///
/// Takes a concrete function and callsite and tries to actually simplify it by
/// analyzing the arguments and call itself with instsimplify. Returns true if
/// it has simplified the callsite to some other entity (a constant), making it
/// free.
bool CallAnalyzer::simplifyCallSite(Function *F, CallBase &Call) {
  // FIXME: Using the instsimplify logic directly for this is inefficient
  // because we have to continually rebuild the argument list even when no
  // simplifications can be performed. Until that is fixed with remapping
  // inside of instsimplify, directly constant fold calls here.
  if (!canConstantFoldCallTo(&Call, F))
    return false;

  // Try to re-map the arguments to constants.
  SmallVector<Constant *, 4> ConstantArgs;
  ConstantArgs.reserve(Call.arg_size());
  for (Value *I : Call.args()) {
    Constant *C = dyn_cast<Constant>(I);
    if (!C)
      C = dyn_cast_or_null<Constant>(SimplifiedValues.lookup(I));
    if (!C)
      return false; // This argument doesn't map to a constant.

    ConstantArgs.push_back(C);
  }
  if (Constant *C = ConstantFoldCall(&Call, F, ConstantArgs)) {
    SimplifiedValues[&Call] = C;
    return true;
  }

  return false;
}

bool CallAnalyzer::visitCallBase(CallBase &Call) {
  if (Call.hasFnAttr(Attribute::ReturnsTwice) &&
      !F.hasFnAttribute(Attribute::ReturnsTwice)) {
    // This aborts the entire analysis.
    ExposesReturnsTwice = true;
    return false;
  }
  if (isa<CallInst>(Call) && cast<CallInst>(Call).cannotDuplicate())
    ContainsNoDuplicateCall = true;

  if (Function *F = Call.getCalledFunction()) {
    // When we have a concrete function, first try to simplify it directly.
    if (simplifyCallSite(F, Call))
      return true;

    // Next check if it is an intrinsic we know about.
    // FIXME: Lift this into part of the InstVisitor.
    if (IntrinsicInst *II = dyn_cast<IntrinsicInst>(&Call)) {
      switch (II->getIntrinsicID()) {
      default:
        if (!Call.onlyReadsMemory() && !isAssumeLikeIntrinsic(II))
          disableLoadElimination();
        return Base::visitCallBase(Call);

      case Intrinsic::load_relative:
        // This is normally lowered to 4 LLVM instructions.
        addCost(3 * InlineConstants::InstrCost);
        return false;

      case Intrinsic::memset:
      case Intrinsic::memcpy:
      case Intrinsic::memmove:
        disableLoadElimination();
        // SROA can usually chew through these intrinsics, but they aren't free.
        return false;
      case Intrinsic::icall_branch_funnel:
      case Intrinsic::localescape:
        HasUninlineableIntrinsic = true;
        return false;
      case Intrinsic::vastart:
        InitsVargArgs = true;
        return false;
      }
    }

    if (F == Call.getFunction()) {
      // This flag will fully abort the analysis, so don't bother with anything
      // else.
      IsRecursiveCall = true;
      return false;
    }

    if (TTI.isLoweredToCall(F)) {
      // We account for the average 1 instruction per call argument setup
      // here.
      addCost(Call.arg_size() * InlineConstants::InstrCost);

      // Everything other than inline ASM will also have a significant cost
      // merely from making the call.
      if (!isa<InlineAsm>(Call.getCalledValue()))
        addCost(InlineConstants::CallPenalty);
    }

    if (!Call.onlyReadsMemory())
      disableLoadElimination();
    return Base::visitCallBase(Call);
  }

  // Otherwise we're in a very special case -- an indirect function call. See
  // if we can be particularly clever about this.
  Value *Callee = Call.getCalledValue();

  // First, pay the price of the argument setup. We account for the average
  // 1 instruction per call argument setup here.
  addCost(Call.arg_size() * InlineConstants::InstrCost);

  // Next, check if this happens to be an indirect function call to a known
  // function in this inline context. If not, we've done all we can.
  Function *F = dyn_cast_or_null<Function>(SimplifiedValues.lookup(Callee));
  if (!F) {
    if (!Call.onlyReadsMemory())
      disableLoadElimination();
    return Base::visitCallBase(Call);
  }

  // If we have a constant that we are calling as a function, we can peer
  // through it and see the function target. This happens not infrequently
  // during devirtualization and so we want to give it a hefty bonus for
  // inlining, but cap that bonus in the event that inlining wouldn't pan
  // out. Pretend to inline the function, with a custom threshold.
  auto IndirectCallParams = Params;
  IndirectCallParams.DefaultThreshold = InlineConstants::IndirectCallThreshold;
  CallAnalyzer CA(TTI, GetAssumptionCache, GetBFI, PSI, ORE, *F, Call,
                  TLI, ILIC, AI, CallSitesForFusion,   // INTEL
                  FuncsForDTrans,                  //INTEL
                  IndirectCallParams);
  if (CA.analyzeCall(Call, TTI, nullptr)) { // INTEL
    // We were able to inline the indirect call! Subtract the cost from the
    // threshold to get the bonus we want to apply, but don't go below zero.
    Cost -= std::max(0, CA.getThreshold() - CA.getCost());
  }

  if (!F->onlyReadsMemory())
    disableLoadElimination();
  return Base::visitCallBase(Call);
}

bool CallAnalyzer::visitReturnInst(ReturnInst &RI) {
  // At least one return instruction will be free after inlining.
  bool Free = !HasReturn;
  HasReturn = true;
  return Free;
}

bool CallAnalyzer::visitBranchInst(BranchInst &BI) {
  // We model unconditional branches as essentially free -- they really
  // shouldn't exist at all, but handling them makes the behavior of the
  // inliner more regular and predictable. Interestingly, conditional branches
  // which will fold away are also free.
  return BI.isUnconditional() || isa<ConstantInt>(BI.getCondition()) ||
         dyn_cast_or_null<ConstantInt>(
             SimplifiedValues.lookup(BI.getCondition()));
}

bool CallAnalyzer::visitSelectInst(SelectInst &SI) {
  bool CheckSROA = SI.getType()->isPointerTy();
  Value *TrueVal = SI.getTrueValue();
  Value *FalseVal = SI.getFalseValue();

  Constant *TrueC = dyn_cast<Constant>(TrueVal);
  if (!TrueC)
    TrueC = SimplifiedValues.lookup(TrueVal);
  Constant *FalseC = dyn_cast<Constant>(FalseVal);
  if (!FalseC)
    FalseC = SimplifiedValues.lookup(FalseVal);
  Constant *CondC =
      dyn_cast_or_null<Constant>(SimplifiedValues.lookup(SI.getCondition()));

  if (!CondC) {
    // Select C, X, X => X
    if (TrueC == FalseC && TrueC) {
      SimplifiedValues[&SI] = TrueC;
      return true;
    }

    if (!CheckSROA)
      return Base::visitSelectInst(SI);

    std::pair<Value *, APInt> TrueBaseAndOffset =
        ConstantOffsetPtrs.lookup(TrueVal);
    std::pair<Value *, APInt> FalseBaseAndOffset =
        ConstantOffsetPtrs.lookup(FalseVal);
    if (TrueBaseAndOffset == FalseBaseAndOffset && TrueBaseAndOffset.first) {
      ConstantOffsetPtrs[&SI] = TrueBaseAndOffset;

      Value *SROAArg;
      DenseMap<Value *, int>::iterator CostIt;
      if (lookupSROAArgAndCost(TrueVal, SROAArg, CostIt))
        SROAArgValues[&SI] = SROAArg;
      return true;
    }

    return Base::visitSelectInst(SI);
  }

  // Select condition is a constant.
  Value *SelectedV = CondC->isAllOnesValue()
                         ? TrueVal
                         : (CondC->isNullValue()) ? FalseVal : nullptr;
  if (!SelectedV) {
    // Condition is a vector constant that is not all 1s or all 0s.  If all
    // operands are constants, ConstantExpr::getSelect() can handle the cases
    // such as select vectors.
    if (TrueC && FalseC) {
      if (auto *C = ConstantExpr::getSelect(CondC, TrueC, FalseC)) {
        SimplifiedValues[&SI] = C;
        return true;
      }
    }
    return Base::visitSelectInst(SI);
  }

  // Condition is either all 1s or all 0s. SI can be simplified.
  if (Constant *SelectedC = dyn_cast<Constant>(SelectedV)) {
    SimplifiedValues[&SI] = SelectedC;
    return true;
  }

  if (!CheckSROA)
    return true;

  std::pair<Value *, APInt> BaseAndOffset =
      ConstantOffsetPtrs.lookup(SelectedV);
  if (BaseAndOffset.first) {
    ConstantOffsetPtrs[&SI] = BaseAndOffset;

    Value *SROAArg;
    DenseMap<Value *, int>::iterator CostIt;
    if (lookupSROAArgAndCost(SelectedV, SROAArg, CostIt))
      SROAArgValues[&SI] = SROAArg;
  }

  return true;
}

bool CallAnalyzer::visitSwitchInst(SwitchInst &SI) {
  // We model unconditional switches as free, see the comments on handling
  // branches.
  if (isa<ConstantInt>(SI.getCondition()))
    return true;
  if (Value *V = SimplifiedValues.lookup(SI.getCondition()))
    if (isa<ConstantInt>(V))
      return true;

  // Assume the most general case where the switch is lowered into
  // either a jump table, bit test, or a balanced binary tree consisting of
  // case clusters without merging adjacent clusters with the same
  // destination. We do not consider the switches that are lowered with a mix
  // of jump table/bit test/binary search tree. The cost of the switch is
  // proportional to the size of the tree or the size of jump table range.
  //
  // NB: We convert large switches which are just used to initialize large phi
  // nodes to lookup tables instead in simplify-cfg, so this shouldn't prevent
  // inlining those. It will prevent inlining in cases where the optimization
  // does not (yet) fire.

  // Maximum valid cost increased in this function.
  int CostUpperBound = INT_MAX - InlineConstants::InstrCost - 1;

  // Exit early for a large switch, assuming one case needs at least one
  // instruction.
  // FIXME: This is not true for a bit test, but ignore such case for now to
  // save compile-time.
  int64_t CostLowerBound =
      std::min((int64_t)CostUpperBound,
               (int64_t)SI.getNumCases() * InlineConstants::InstrCost + Cost);

#if INTEL_CUSTOMIZATION
  if (CostLowerBound > Threshold) {
    if (EarlyExitCost == INT_MAX) {
      EarlyExitCost = CostLowerBound;
      EarlyExitThreshold = Threshold;
    }
  }
#endif // INTEL_CUSTOMIZATION

  unsigned JumpTableSize = 0;
  unsigned NumCaseCluster =
      TTI.getEstimatedNumberOfCaseClusters(SI, JumpTableSize);

  // If suitable for a jump table, consider the cost for the table size and
  // branch to destination.
  if (JumpTableSize) {
    int64_t JTCost = (int64_t)JumpTableSize * InlineConstants::InstrCost +
                     4 * InlineConstants::InstrCost;

    addCost(JTCost, (int64_t)CostUpperBound);
    return false;
  }

  // Considering forming a binary search, we should find the number of nodes
  // which is same as the number of comparisons when lowered. For a given
  // number of clusters, n, we can define a recursive function, f(n), to find
  // the number of nodes in the tree. The recursion is :
  // f(n) = 1 + f(n/2) + f (n - n/2), when n > 3,
  // and f(n) = n, when n <= 3.
  // This will lead a binary tree where the leaf should be either f(2) or f(3)
  // when n > 3.  So, the number of comparisons from leaves should be n, while
  // the number of non-leaf should be :
  //   2^(log2(n) - 1) - 1
  //   = 2^log2(n) * 2^-1 - 1
  //   = n / 2 - 1.
  // Considering comparisons from leaf and non-leaf nodes, we can estimate the
  // number of comparisons in a simple closed form :
  //   n + n / 2 - 1 = n * 3 / 2 - 1
  if (NumCaseCluster <= 3) {
    // Suppose a comparison includes one compare and one conditional branch.
    addCost(NumCaseCluster * 2 * InlineConstants::InstrCost);
    return false;
  }

  int64_t ExpectedNumberOfCompare = 3 * (int64_t)NumCaseCluster / 2 - 1;
  int64_t SwitchCost =
      ExpectedNumberOfCompare * 2 * InlineConstants::InstrCost;

  addCost(SwitchCost, (int64_t)CostUpperBound);
  return false;
}

bool CallAnalyzer::visitIndirectBrInst(IndirectBrInst &IBI) {
  // We never want to inline functions that contain an indirectbr.  This is
  // incorrect because all the blockaddress's (in static global initializers
  // for example) would be referring to the original function, and this
  // indirect jump would jump from the inlined copy of the function into the
  // original function which is extremely undefined behavior.
  // FIXME: This logic isn't really right; we can safely inline functions with
  // indirectbr's as long as no other function or global references the
  // blockaddress of a block within the current function.
  HasIndirectBr = true;
  return false;
}

bool CallAnalyzer::visitResumeInst(ResumeInst &RI) {
  // FIXME: It's not clear that a single instruction is an accurate model for
  // the inline cost of a resume instruction.
  return false;
}

bool CallAnalyzer::visitCleanupReturnInst(CleanupReturnInst &CRI) {
  // FIXME: It's not clear that a single instruction is an accurate model for
  // the inline cost of a catchret instruction.
  return false;
}

bool CallAnalyzer::visitCatchReturnInst(CatchReturnInst &CRI) {
  // FIXME: It's not clear that a single instruction is an accurate model for
  // the inline cost of a cleanupret instruction.
  return false;
}

bool CallAnalyzer::visitUnreachableInst(UnreachableInst &I) {
  // FIXME: It might be reasonably to discount the cost of instructions leading
  // to unreachable as they have the lowest possible impact on both runtime and
  // code size.
  return true; // No actual code is needed for unreachable.
}

bool CallAnalyzer::visitInstruction(Instruction &I) {
  // Some instructions are free. All of the free intrinsics can also be
  // handled by SROA, etc.
  if (TargetTransformInfo::TCC_Free == TTI.getUserCost(&I))
    return true;

  // We found something we don't understand or can't handle. Mark any SROA-able
  // values in the operand list as no longer viable.
  for (User::op_iterator OI = I.op_begin(), OE = I.op_end(); OI != OE; ++OI)
    disableSROA(*OI);

  return false;
}

/// Analyze a basic block for its contribution to the inline cost.
///
/// This method walks the analyzer over every instruction in the given basic
/// block and accounts for their cost during inlining at this callsite. It
/// aborts early if the threshold has been exceeded or an impossible to inline
/// construct has been detected. It returns false if inlining is no longer
/// viable, and true if inlining remains viable.
InlineResult
CallAnalyzer::analyzeBlock(BasicBlock *BB,
                           SmallPtrSetImpl<const Value *> &EphValues) {
  for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ++I) {
    // FIXME: Currently, the number of instructions in a function regardless of
    // our ability to simplify them during inline to constants or dead code,
    // are actually used by the vector bonus heuristic. As long as that's true,
    // we have to special case debug intrinsics here to prevent differences in
    // inlining due to debug symbols. Eventually, the number of unsimplified
    // instructions shouldn't factor into the cost computation, but until then,
    // hack around it here.
    if (isa<DbgInfoIntrinsic>(I))
      continue;

    // Skip ephemeral values.
    if (EphValues.count(&*I))
      continue;

    ++NumInstructions;
    if (isa<ExtractElementInst>(I) || I->getType()->isVectorTy())
      ++NumVectorInstructions;

    // If the instruction simplified to a constant, there is no cost to this
    // instruction. Visit the instructions using our InstVisitor to account for
    // all of the per-instruction logic. The visit tree returns true if we
    // consumed the instruction in any way, and false if the instruction's base
    // cost should count against inlining.
    if (Base::visit(&*I))
      ++NumInstructionsSimplified;
    else
      addCost(InlineConstants::InstrCost);

    using namespace ore;
    // If the visit this instruction detected an uninlinable pattern, abort.
    InlineResult IR;
    if (IsRecursiveCall)
      IR = "recursive";
    else if (ExposesReturnsTwice)
      IR = "exposes returns twice";
    else if (HasDynamicAlloca)
      IR = "dynamic alloca";
    else if (HasIndirectBr)
      IR = "indirect branch";
    else if (HasUninlineableIntrinsic)
      IR = "uninlinable intrinsic";
    else if (InitsVargArgs)
      IR = "varargs";
    if (!IR) {
      if (ORE)
        ORE->emit([&]() {
          return OptimizationRemarkMissed(DEBUG_TYPE, "NeverInline",
                                          &CandidateCall)
                 << NV("Callee", &F) << " has uninlinable pattern ("
                 << NV("InlineResult", IR.message)
                 << ") and cost is not fully computed";
        });
      return IR;
    }

    // If the caller is a recursive function then we don't want to inline
    // functions which allocate a lot of stack space because it would increase
    // the caller stack usage dramatically.
    if (IsCallerRecursive &&
        AllocatedSize > InlineConstants::TotalAllocaSizeRecursiveCaller) {
      InlineResult IR = "recursive and allocates too much stack space";
      if (ORE)
        ORE->emit([&]() {
          return OptimizationRemarkMissed(DEBUG_TYPE, "NeverInline",
                                          &CandidateCall)
                 << NV("Callee", &F) << " is " << NV("InlineResult", IR.message)
                 << ". Cost is not fully computed";
        });
      return IR;
    }

    // Check if we've passed the maximum possible threshold so we don't spin in
    // huge basic blocks that will never inline.
#if INTEL_CUSTOMIZATION
    if (Cost >= Threshold) {
      if (!ComputeFullInlineCost)
         return false;
      if (EarlyExitCost == INT_MAX) {
         EarlyExitCost = Cost;
         EarlyExitThreshold = Threshold;
      }
    }
#endif // INTEL_CUSTOMIZATION
  }

  return true;
}

#if INTEL_CUSTOMIZATION
///
/// \brief Find the best inlining or non-inlining reason
///
/// Given a 'DefaultReason' and a vector of inlining/non-inlining reasons,
/// return the best reason among all of them.  Inlining/Non-inlining reasons
/// are considered better if their corresponding enum value is lower.
///

static InlineReason bestInlineReason(const InlineReasonVector& ReasonVector,
  InlineReason DefaultReason)
{
  InlineReason Reason = DefaultReason;
  for (unsigned i = 0; i < ReasonVector.size(); i++) {
    if (ReasonVector[i] < Reason) {
       Reason = ReasonVector[i];
    }
  }
  return Reason;
}
#endif // INTEL_CUSTOMIZATION

/// Compute the base pointer and cumulative constant offsets for V.
///
/// This strips all constant offsets off of V, leaving it the base pointer, and
/// accumulates the total constant offset applied in the returned constant. It
/// returns 0 if V is not a pointer, and returns the constant '0' if there are
/// no constant offsets applied.
ConstantInt *CallAnalyzer::stripAndComputeInBoundsConstantOffsets(Value *&V) {
  if (!V->getType()->isPointerTy())
    return nullptr;

  unsigned AS = V->getType()->getPointerAddressSpace();
  unsigned IntPtrWidth = DL.getIndexSizeInBits(AS);
  APInt Offset = APInt::getNullValue(IntPtrWidth);

  // Even though we don't look through PHI nodes, we could be called on an
  // instruction in an unreachable block, which may be on a cycle.
  SmallPtrSet<Value *, 4> Visited;
  Visited.insert(V);
  do {
    if (GEPOperator *GEP = dyn_cast<GEPOperator>(V)) {
      if (!GEP->isInBounds() || !accumulateGEPOffset(*GEP, Offset))
        return nullptr;
      V = GEP->getPointerOperand();
    } else if (Operator::getOpcode(V) == Instruction::BitCast) {
      V = cast<Operator>(V)->getOperand(0);
    } else if (GlobalAlias *GA = dyn_cast<GlobalAlias>(V)) {
      if (GA->isInterposable())
        break;
      V = GA->getAliasee();
    } else {
      break;
    }
    assert(V->getType()->isPointerTy() && "Unexpected operand type!");
  } while (Visited.insert(V).second);

  Type *IntPtrTy = DL.getIntPtrType(V->getContext(), AS);
  return cast<ConstantInt>(ConstantInt::get(IntPtrTy, Offset));
}

#if INTEL_CUSTOMIZATION
//
// Increment 'GlobalCount' if a load of a global value appears in 'Op'.
// Increment 'ConstantCount' if an integer constant appears in 'Op'.
//
static void countGlobalsAndConstants(Value* Op, unsigned& GlobalCount,
  unsigned& ConstantCount)
{
  LoadInst *LILHS = dyn_cast<LoadInst>(Op);
  if (LILHS) {
    Value *GV = LILHS->getPointerOperand();
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
static bool forgivableCondition(Instruction* TI) {
  BranchInst *BI = dyn_cast<BranchInst>(TI);
  if (!BI || !BI->isConditional())
    return false;
  Value *Cond = BI->getCondition();
  ICmpInst *ICmp = dyn_cast<ICmpInst>(Cond);
  if (!ICmp)
    return false;
  Value* LHS = ICmp->getOperand(0);
  Value* RHS = ICmp->getOperand(1);
  unsigned GlobalCount = 0;
  unsigned ConstantCount = 0;
  countGlobalsAndConstants(LHS, GlobalCount, ConstantCount);
  countGlobalsAndConstants(RHS, GlobalCount, ConstantCount);
  return ConstantCount == 1 && GlobalCount == 1;
}

DominatorTree* InliningLoopInfoCache::getDT(Function* F) {
  auto It = DTMapSCC.find(F);
  if (It != DTMapSCC.end())
    return It->second;
  auto ret = new DominatorTree(*F);
  DTMapSCC.insert(std::make_pair(F, ret));
  return ret;
}

LoopInfo* InliningLoopInfoCache::getLI(Function* F) {
  auto It = LIMapSCC.find(F);
  if (It != LIMapSCC.end())
    return It->second;
  DominatorTree* DT = getDT(F);
  assert(DT != nullptr);
  auto ret = new LoopInfo(*DT);
  LIMapSCC.insert(std::make_pair(F, ret));
  return ret;
}

void InliningLoopInfoCache::invalidateFunction(Function* F) {
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
  for (auto &DTI: DTMapSCC)
    delete DTI.second;
  DTMapSCC.clear();
  for (auto &LTI: LIMapSCC)
    delete LTI.second;
  LIMapSCC.clear();
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
   Function *Caller = CB.getCaller();
   Function *Callee = CB.getCalledFunction();
   // Look for 2 calls of the callee in the caller.
   unsigned count = 0;
   for (Use &U : Callee->uses()) {
      if (auto CBB = dyn_cast<CallBase>(U.getUser())) {
        if (CBB && (CBB->getCaller() == Caller)) {
          if (++count > 2) {
            return false;
          }
        }
      }
   }
   if (count != 2) {
      return false;
   }
   // Look for a single top level loop in the callee.
   LoopInfo *LI = ILIC.getLI(Callee);
   count = 0;
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
  if (L->empty() && L->getParentLoop()
    && boundConstArg(F, L) && boundConstArg(F, L->getParentLoop()))
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
static bool worthyDoubleCallSite2(CallBase &CB, InliningLoopInfoCache& ILIC) {
  Function *Callee = CB.getCalledFunction();
  LoopInfo *LI = ILIC.getLI(Callee);
  return std::distance(LI->begin(), LI->end()) == 1
    && hasConstTripCountArg(Callee, *(LI->begin()));
}

//
// Return the total number of predecessors for the basic blocks of 'F'
//
static unsigned int totalBasicBlockPredCount(Function &F)
{
  unsigned int count = 0;
  for (Function::iterator BI = F.begin(), BE = F.end(); BI != BE; ++BI) {
    BasicBlock *BB = &*BI;
    count += std::distance(pred_begin(BB), pred_end(BB));
  }
  return count;
}

//
// Temporary switch to control new double callsite inlining heuristics
// until tuning of loopopt is complete.
//
static cl::opt<bool> NewDoubleCallSiteInliningHeuristics
  ("new-double-callsite-inlining-heuristics",
   cl::init(false), cl::ReallyHidden);

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
static bool worthyDoubleCallSite3(CallBase &CB, InliningLoopInfoCache& ILIC) {
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
  if (CB.arg_begin() != CB.arg_end()
    && totalBasicBlockPredCount(*Callee)
    < InlineConstants::BigBasicBlockPredCount)
    return false;
  return true;
}

//
// Return true if 'F' has exactly two callsites
//
static bool isDoubleCallSite(Function *F)
{
  unsigned int count = 0;
  for (User *U : F->users()) {
    auto CB = dyn_cast<CallBase>(U);
    if (!CB || CB->getCalledFunction() != F)
      continue;
    if (++count > 2)
      return false;
  }
  return count == 2;
}

//
// Return 'true' if 'CB' worth inlining, given that it is a double callsite
// with internal linkage.
//
static bool worthyDoubleInternalCallSite(CallBase &CB,
  InliningLoopInfoCache& ILIC)
{
  return worthyDoubleCallSite1(CB, ILIC) || worthyDoubleCallSite2(CB, ILIC) ||
    worthyDoubleCallSite3(CB, ILIC);
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
static bool worthyDoubleExternalCallSite(CallBase &CB) {
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

static bool preferCloningToInlining(CallBase &CB,
                                    InliningLoopInfoCache& ILIC,
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

// Check: expect the given Function
// - has 2 consecutive AND instructions;
//   and
// - each AND instruction has a value 0x07 on ones of its operands;
//
static bool has2SubInstWithValInF(Function *F, const Value *Val0,
                                  const Value *Val1) {

  for (inst_iterator I = inst_begin(F), E = std::prev(inst_end(F)); I != E;
       ++I) {

    auto NextI = std::next(I);

    if ((!isa<BinaryOperator>(&*I)) || (!isa<BinaryOperator>(&*NextI)))
      continue;

    BinaryOperator *BOp = dyn_cast<BinaryOperator>(&*I);
    BinaryOperator *NextBOp = dyn_cast<BinaryOperator>(&*NextI);

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

//
// Return 'true' if 'CB' should not be inlined, because it would be better
// to multiversion it. 'PrepareForLTO' is true if we are on the compile step
// of an LTO compilation.
//
static bool preferMultiversioningToInlining(CallBase &CB,
                                            const TargetTransformInfo
                                                &CalleeTTI,
                                            InliningLoopInfoCache &ILIC,
                                            bool PrepareForLTO) {

  // Right now, only the callee is tested for multiversioning.  We use
  // this set to keep track of callees that have already been tested,
  // to save compile time. We expect it to be relatively expensive to
  // test the qualifying candidates. Those that do not qualify should
  // be rejected quickly.
  static SmallPtrSet<Function *, 3> CalleeFxnPtrSet;

  //Only focus on Link time at present:
  if (PrepareForLTO)
    return false;

  // Quick tests:
  if (!DTransInlineHeuristics)
    return false;
  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasAVX2;
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

// Check that loop has normalized structure and constant trip count.
static bool isConstantTripCount(Loop *L) {
  // Get canonical IV.
  PHINode *IV = L->getCanonicalInductionVariable();
  if (!IV) {
    return false;
  }

  ICmpInst *CInst = getLoopBottomTest(L);
  if (!CInst) {
    return false;
  }

  if (!CInst->isIntPredicate()) {
    return false;
  }

  int NumOps = CInst->getNumOperands();
  if (NumOps != 2) {
    return false;
  }

  // Check that condition is <, <= or ==.
  ICmpInst::Predicate Pred = CInst->getPredicate();
  if (!(Pred == ICmpInst::ICMP_EQ || Pred == ICmpInst::ICMP_ULT ||
        Pred == ICmpInst::ICMP_ULE || Pred == ICmpInst::ICMP_SLT ||
        Pred == ICmpInst::ICMP_SLE)) {
    return false;
  }

  // First operand should be IV. Second should be positive int constant.
  Value *IVInc = CInst->getOperand(0);
  ConstantInt *Const = dyn_cast<ConstantInt>(CInst->getOperand(1));

  if (!IVInc || !Const) {
    // IV or TC are not available - return false
    return false;
  }

  const APInt &ConstValue = Const->getValue();
  if (!ConstValue.isStrictlyPositive()) {
    // non-positive TC - return false
    return false;
  }

  uint64_t LimTC = ConstValue.getLimitedValue();
  // Currently allow only TC=4.
  if (LimTC != 4) {
    return false;
  }

  unsigned IncomingValuesCnt = IV->getNumIncomingValues();
  for (unsigned i = 0; i < IncomingValuesCnt; ++i) {
    if (IVInc == IV->getIncomingValue(i)) {
      // Found IV in bottom test - return true
      return true;
    }
  }

  return false;
}

// Minimal number of arrays in loop that should be function arguments.
static cl::opt<int> MinArgRefs(
    "inline-for-fusion-min-arg-refs", cl::Hidden, cl::init(10),
    cl::desc(
        "Min number of arguments appearing in loop candidates for fusion"));

// Number of successive callsites need to be inlined to benefit
// from loops fusion.
static cl::list<int> NumCallSitesForFusion("inline-for-fusion-num-callsites",
                                           cl::ReallyHidden,
                                           cl::CommaSeparated);

// Temporary switch to control new callsite inlining heuristics
// for fusion until tuning of loopopt is complete.
static cl::opt<cl::boolOrDefault>
    InliningForFusionHeuristics("inlining-for-fusion-heuristics",
                                cl::ReallyHidden);

/// \brief Analyze a callsite for potential inlining for fusion.
///
/// Returns true if inlining of this and a number of successive callsites
/// with the same callee would benefit from loop fusion and vectorization
/// later on.
/// The criteria for this heuristic are:
/// 1) Multiple callsites of the same callee in one basic block.
/// 2) Loops inside callee should have constant trip count and have
///    no calles inside.
/// 3) Array accesses inside loops should corespond to callee arguments.
/// In case we decide that current CB is a candidate for inlining for fusion,
/// then we store other CB to the same function in the same basic block in
/// the set of inlining candidates for fusion.
static bool worthInliningForFusion(CallBase &CB,
                                   const TargetTransformInfo &CalleeTTI,
                                   InliningLoopInfoCache &ILIC,
                                   SmallSet<CallBase *, 20> *CallSitesForFusion,
                                   bool PrepareForLTO) {

  if (InliningForFusionHeuristics != cl::BOU_TRUE) {
    auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasAVX2;
    if (!CalleeTTI.isAdvancedOptEnabled(TTIAVX2))
      return false;
  }

  // Heuristic is enabled if option is unset and it is first inliner run
  // (on PrepareForLTO phase) OR if option is set to true.
  if (((InliningForFusionHeuristics != cl::BOU_UNSET) || !PrepareForLTO) &&
      (InliningForFusionHeuristics != cl::BOU_TRUE))
    return false;

  if (!CallSitesForFusion) {
    LLVM_DEBUG(llvm::dbgs()
               << "IC: No inlining for fusion: no call site candidates.\n");
    return false;
  }

  // The call site was stored as candidate for inlining for fusion.
  if (CallSitesForFusion->count(&CB)) {
    CallSitesForFusion->erase(&CB);
    return true;
  }

  Function *Caller = CB.getCaller();
  Function *Callee = CB.getCalledFunction();
  BasicBlock *CSBB = CB.getParent();

  SmallVector<CallBase *, 20> LocalCSForFusion;
  // Check that all call sites in the Caller, that are not to intrinsics, are in
  // the same basic block and are fusion candidates.
  int CSCount = 0;
  for (auto &I : instructions(Caller)) {
    auto LocalCB = dyn_cast<CallBase>(&I);
    if (!LocalCB)
      continue;

    Function *CurrCallee = LocalCB->getCalledFunction();
    if (!CurrCallee) {
      return false;
    }

    if (CurrCallee->isIntrinsic())
      continue;
    if (CurrCallee != Callee) {
      return false;
    }

    if (I.getParent() != CSBB) {
      return false;
    }

    LocalCSForFusion.push_back(LocalCB);
    CSCount++;
  }

  // If the user doesn't specify number of call sites explicitly then use 8
  // (and 2 later) as a default value,
  if (NumCallSitesForFusion.empty()) {
    NumCallSitesForFusion.push_back(8);
    // TODO:   NumCallSitesForFusion.push_back(2);
  }

  // If call site candidates are the only calls in the caller function and the
  // number of successive calls doesn't fit into the option - skip
  // transformation.
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

  // Check if callee has function calls inside - skip it.
  for (BasicBlock &BB : *Callee) {
    for (auto &I : BB) {
      if (isa<CallInst>(I) || isa<InvokeInst>(I)) {
        CallInst *CI = dyn_cast_or_null<CallInst>(&I);
        InvokeInst *II = dyn_cast_or_null<InvokeInst>(&I);
        auto InnerFunc = CI ? CI->getCalledFunction() : II->getCalledFunction();
        if (!InnerFunc || !InnerFunc->isIntrinsic()) {
          LLVM_DEBUG(llvm::dbgs()
                     << "IC: No inlining for fusion: call inside candidate.\n");
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
    if (!isConstantTripCount(LL)) {
      // Non-constant trip count. Skip inlining.
      LLVM_DEBUG(llvm::dbgs()
                 << "IC: No inlining for fusion: non-constant TC in loop.\n");
      return false;
    }
    // Check how many array refs in GEP instructions are arguments of
    // the callee.
    for (auto *BB : LL->blocks()) {
      for (auto &I : *BB) {
        if (ArgCnt > MinArgRefs)
          break;
        if (GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(&I)) {
          Value *PtrOp = GEPI->getPointerOperand();
          if (PtrOp) {
            if (isa<PHINode>(PtrOp))
              PtrOp = dyn_cast<PHINode>(PtrOp)->getIncomingValue(0);
            if (isa<Argument>(PtrOp))
              ArgCnt++;
          }
        }
      }
      if (ArgCnt > MinArgRefs)
        break;
    }
    if (ArgCnt > MinArgRefs)
      break;
  }

  // Not enough arguments-arrays were found in loop.
  if (ArgCnt < MinArgRefs) {
    LLVM_DEBUG(llvm::dbgs()
               << "IC: No inlining for fusion: not enough array refs.\n");
    return false;
  }

  // TODO: Remove this condition once 2 becomes a legal CSCount.
  if (Caller->getInstructionCount() < 40)
    return false;

  // Store other inlining candidates in a special map.
  if (CallSitesForFusion) {
    for (auto LocalCB : LocalCSForFusion) {
      CallSitesForFusion->insert(LocalCB);
    }
  }

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

/// \brief Analyze a callsite for potential inlining for deeply nested ifs.
///
/// The criteria for this heuristic are:
/// 1) Works on PrepareForLTO phase only.
/// 2) Recursive Caller, non-recursive callee.
/// 3) Caller has 'switch' instruction.
/// 4) Callee has no loops.
/// 5) Both caller and callee contain depeply nested ifs.
static bool worthInliningForDeeplyNestedIfs(CallBase &CB,
                                            InliningLoopInfoCache &ILIC,
                                            bool IsCallerRecursive,
                                            bool PrepareForLTO) {
  // Heuristic is enabled if option is unset and it is first inliner run
  // (on PrepareForLTO phase) OR if option is set to true.
  if (((InliningForDeeplyNestedIfs != cl::BOU_UNSET) || !PrepareForLTO) &&
      (InliningForDeeplyNestedIfs != cl::BOU_TRUE))
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
// Return 'true' if 'CB' should not be inlined, because it would be better
// to perform SOAToAOS on it. 'PrepareForLTO' is true if we are on the compile
// step of an LTO compilation.
//
bool CallAnalyzer::preferDTransToInlining(CallBase &CB,
                                          bool PrepareForLTO) const {
  if (!PrepareForLTO)
    return false;

  if (!DTransInlineHeuristics)
    return false;

  if (!FuncsForDTrans) {
    LLVM_DEBUG(
        llvm::dbgs()
        << "IC: inlining for dtrans: no candidates to suppress inline.\n");
    return false;
  }

  // The callee was stored as candidate to suppress inlining.
  if (Function *Callee = CB.getCalledFunction())
    if (FuncsForDTrans->count(Callee))
      return true;

  return false;
}
#endif // INTEL_CUSTOMIZATION

/// Find dead blocks due to deleted CFG edges during inlining.
///
/// If we know the successor of the current block, \p CurrBB, has to be \p
/// NextBB, the other successors of \p CurrBB are dead if these successors have
/// no live incoming CFG edges.  If one block is found to be dead, we can
/// continue growing the dead block list by checking the successors of the dead
/// blocks to see if all their incoming edges are dead or not.
void CallAnalyzer::findDeadBlocks(BasicBlock *CurrBB, BasicBlock *NextBB) {
  auto IsEdgeDead = [&](BasicBlock *Pred, BasicBlock *Succ) {
    // A CFG edge is dead if the predecessor is dead or the predecessor has a
    // known successor which is not the one under exam.
    return (DeadBlocks.count(Pred) ||
            (KnownSuccessors[Pred] && KnownSuccessors[Pred] != Succ));
  };

  auto IsNewlyDead = [&](BasicBlock *BB) {
    // If all the edges to a block are dead, the block is also dead.
    return (!DeadBlocks.count(BB) &&
            llvm::all_of(predecessors(BB),
                         [&](BasicBlock *P) { return IsEdgeDead(P, BB); }));
  };

  for (BasicBlock *Succ : successors(CurrBB)) {
    if (Succ == NextBB || !IsNewlyDead(Succ))
      continue;
    SmallVector<BasicBlock *, 4> NewDead;
    NewDead.push_back(Succ);
    while (!NewDead.empty()) {
      BasicBlock *Dead = NewDead.pop_back_val();
      if (DeadBlocks.insert(Dead))
        // Continue growing the dead block lists.
        for (BasicBlock *S : successors(Dead))
          if (IsNewlyDead(S))
            NewDead.push_back(S);
    }
  }
}

#if INTEL_CUSTOMIZATION
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
    if (isReallocLikeFn(&I, TLI))
      return true;
  return false;
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
  auto PassesContentsTest = [](Function *F,
      TargetLibraryInfo *TLI) -> bool {
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
// Return 'true' if Function *F should not be inlined due to its special
// manipulation of stack instructions.
//
static bool preferNotToInlineForStackComputations(Function *F,
                                                  TargetLibraryInfo *TLI) {
  //
  // Returns 'true' if the Function *F has no arguments.
  //
  auto PassesArgTest = [](Function *F) -> bool {
    return F->arg_size() == 0;
  };

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
  auto PassesContentsTest = [](Function *F,
      TargetLibraryInfo *TLI) -> bool {
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
   return preferNotToInlineForStackComputations(CB.getCaller(), TLI)
     || preferNotToInlineForStackComputations(CB.getCalledFunction(), TLI);
}

// Minimal number of cases in a switch to qualify for the "prefer not to
// inline for switch computations" heuristic.
static cl::opt<unsigned> MinSwitchCases(
    "inline-for-switch-min-cases", cl::Hidden, cl::init(11),
    cl::desc("Min number of switch cases required to trigger heuristic"));

//
// Return 'true' if the CallBase CB should not be inlined due to having
// a special type of switch statement. (Note: this heuristic uses info
// only from the Caller, and not from the Callee of CB.)
//
static bool preferNotToInlineForSwitchComputations(CallBase &CB,
                                                   InliningLoopInfoCache &ILIC) {
  //
  // Return 'true' if the called function of the Callsite CB is a small
  // function whose basic blocks that end in a ReturnInst return the
  // result of an indirect call.
  //
  auto WorthySwitchCallSite = [] (CallBase &CB) -> bool {
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
     [&WorthySwitchCallSite](Function *Caller, InliningLoopInfoCache &ILIC)
     -> bool {
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
    for (unsigned I = 0; I < CI->getNumArgOperands(); I++) {
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
    for (auto &DTN : DT->getNode(EntryBlock)->getChildren()) {
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
  return Callee && Callee->hasFnAttribute(
    "prefer-partial-inline-outlined-func");
}

//
// Return 'true' if 'Callee' is marked as
// 'prefer-partial-inline-inlined-clone'. Refer to the example in
// preferPartialInlineOutlinedFunc. This function will return true for @foo.1.
//
static bool preferPartialInlineInlinedClone(Function *Callee) {
  return Callee && Callee->hasFnAttribute(
    "prefer-partial-inline-inlined-clone");
}

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
  for (auto &Fx :  M->functions())
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
  return  SawPreferPartialInlineOutlinedFunc && Candidates.count(&F);
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
    Type *Ty = Arg.getType();
    if (Ty->isPointerTy() && Ty->getPointerElementType()->isIntegerTy()) {
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
    if (E > CS->getNumArgOperands() - 1)
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
// Return 'true' if the CallSites of 'Caller' should not be inlined because
// we are in the 'PrepareForLTO' compile phase and delaying the inlining
// decision until the link phase will let us more easily decide whether to
// inline the caller. NOTE: This function 'preferToDelayInlineDecision' can
// also be called in the link phase.  If it is, we determine whether the
// caller should be inlined, and if so, add it on the 'QueuedCallers' set.
//
static bool preferToDelayInlineDecision(Function *Caller,
                                        bool PrepareForLTO,
                                        SmallPtrSetImpl<Function *>
                                            &QueuedCallers) {

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
      QueuedCallers.insert(Caller);
    return true;
  }
  return false;
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
// Return 'true' if a CallSite with this 'Callee' should be inlined because
// it is on the 'QueuedCallers' set. NOTE: We determine whether the 'Callee'
// should be inlined before the inlining of its CallSites, because
// performing the inlining of the CallSites first makes it harder to tell
// if the caller should be inlined.
//
static bool worthInliningSingleBasicBlockWithStructTest(Function *Callee,
                                                        bool PrepareForLTO,
                                                        SmallPtrSetImpl
                                                            <Function *>
                                                            *QueuedCallers) {
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
static bool isInNonEHLoop(Instruction &CSI,
                          InliningLoopInfoCache &ILIC) {
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
  if (Callee->hasFnAttribute(Attribute::NoUnwind))
    return false;
  if (!isInNonEHLoop(CB, ILIC))
    return false;
  return hasLoopOptInhibitingEHInstOutsideLoop(Callee, ILIC);
}

//
// Minimum number of uses all of the special array struct args must have to
// qualify a callee for inlining by the "array struct args" inline heuristic.
//
static cl::opt<unsigned> ArrayStructArgMinUses(
    "inline-for-array-struct-arg-min-uses", cl::Hidden, cl::init(16),
    cl::desc("Min number of uses for array-struct-arg heuristic"));

//
// Minimum number of args in the caller of a callsite to qualify a callsite
// for inlining by the "array struct args" inline heuristic.
//
static cl::opt<unsigned> ArrayStructArgMinCallerArgs(
    "inline-for-array-struct-arg-min-caller-args", cl::Hidden, cl::init(6),
    cl::desc("Min number of caller args for array-struct-arg heuristic"));

//
// Return 'true' if 'Arg' is a special "array struct arg". We are looking for
// an Argument whose type is a pointer to a structure each of whose fields is
// an single dimension array of ints or floats. Also, the lengths of the arrays
// are all the same.
//
static bool isSpecialArrayStructArg(Argument &Arg) {
  llvm::Type *Ty = Arg.getType();
  if (!Ty->isPointerTy())
    return false;
  llvm::Type *TyE = Ty->getPointerElementType();
  auto TyS = dyn_cast<StructType>(TyE);
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
static bool worthInliningForArrayStructArgs(CallBase &CB,
                                            bool PrepareForLTO) {
  static bool ScannedFunctions = false;
  static Function *WorthyCallee = nullptr;
  if (!DTransInlineHeuristics)
    return false;
  if (!ScannedFunctions) {
    Module *M = CB.getParent()->getParent()->getParent();
    for (auto &F :  M->functions()) {
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
// Set the percentage within 100% for which any callsite is considered
// hot via instrumented profile info. (For example, a value of 85 means
// any callsite with an execution count of at least 15% (=100%-85%) of
// the hottest callsite's execution count could be considered.
//
static cl::opt<unsigned> ProfInstrumentHotPercentage(
    "inline-prof-instr-hot-percentage", cl::Hidden, cl::init(85),
    cl::desc("Calls within this percentage of the hottest call are hot"));

//
// Sets a limit on the number of callsites that can be considered hot
// due to instrumented profile execution count. For example, "5" means
// that, on entry to the inliner, only the callsites with the 5 hottest
// execution counts will be considered as having "hot" profiles. (Note
// that there could be actually be more than this number in the inline
// report if a callsite marked hot gets cloned (for example, during
// inlining).
//
static cl::opt<unsigned> ProfInstrumentHotCount(
    "inline-prof-instr-hot-count", cl::Hidden, cl::init(5),
    cl::desc("Number of call sites to be considered hot"));

//
// Return the threshold over which a callsite is considered hot.
//
static uint64_t profInstrumentThreshold(ProfileSummaryInfo *PSI,
                                        Module *M) {
  static bool ComputedThreshold = false;
  static uint64_t Threshold = 0;
  if (ComputedThreshold)
    return Threshold;
  uint64_t MaxProfCount = 0;
  // Find hottest callsite and its profile count.
  std::priority_queue<uint64_t, std::vector<uint64_t>, std::greater<uint64_t>>
      ProfQueue;
  for (auto &F :  M->functions()) {
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
  uint64_t PercentThreshold = MaxProfCount -
      ProfInstrumentHotPercentage * MaxProfCount / 100;
  uint64_t CountThreshold = ProfQueue.size() > 0 ? ProfQueue.top() : 0;
  Threshold = std::max(CountThreshold, PercentThreshold);
  ComputedThreshold = true;
  return Threshold;
}

//
// Return 'true' if 'CB' is a hot callsite based on 'PSI. Do not recognize
// hot callsites during the 'PrepareForLTO' phase.
//
static bool isProfInstrumentHotCallSite(CallBase &CB,
                                        ProfileSummaryInfo *PSI,
                                        bool PrepareForLTO)
{
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
// Test a series of special conditions to determine if it is worth inlining
// if any of them appear. (These have been gathered together into a single
// function to make an early exit easy to accomplish and save compile time.)
// The function returns a bonus that should be applied to the inline cost.
//
static int worthInliningUnderSpecialCondition(CallBase &CB,
                                               TargetLibraryInfo &TLI,
                                               const TargetTransformInfo
                                                   &CalleeTTI,
                                               InliningLoopInfoCache &ILIC,
                                               ProfileSummaryInfo *PSI,
                                               bool PrepareForLTO,
                                               bool IsCallerRecursive,
                                               SmallSet<CallBase *, 20>
                                                   *CallSitesForFusion,
                                               SmallPtrSetImpl<Function *>
                                                   *QueuedCallers,
                                               InlineReasonVector
                                                   &YesReasonVector) {
  Function *F = CB.getCalledFunction();
  if (!F)
    return 0;
  if (isProfInstrumentHotCallSite(CB, PSI, PrepareForLTO)) {
    YesReasonVector.push_back(InlrHotProfile);
    return -InlineConstants::DeepInliningHeuristicBonus;
  }
  if (isDoubleCallSite(F)) {
    // If there are two calls of the function, the cost of inlining it may
    // drop, but less dramatically.
    if (F->hasLocalLinkage() || F->hasLinkOnceODRLinkage()) {
       if (worthyDoubleInternalCallSite(CB, ILIC)) {
         YesReasonVector.push_back(InlrDoubleLocalCall);
         return -InlineConstants::SecondToLastCallToStaticBonus;
       }
    }
    if (worthyDoubleExternalCallSite(CB)) {
      YesReasonVector.push_back(InlrDoubleNonLocalCall);
      return -InlineConstants::SecondToLastCallToStaticBonus;
    }
  }
  if (worthInliningForFusion(CB, CalleeTTI, ILIC, CallSitesForFusion,
    PrepareForLTO)) {
    YesReasonVector.push_back(InlrForFusion);
    return -InlineConstants::InliningHeuristicBonus;;
  }
  if (worthInliningForDeeplyNestedIfs(CB, ILIC, IsCallerRecursive,
    PrepareForLTO)) {
    YesReasonVector.push_back(InlrDeeplyNestedIfs);
    return -InlineConstants::InliningHeuristicBonus;
  }
  if (worthInliningForAddressComputations(CB, ILIC, PrepareForLTO)) {
    YesReasonVector.push_back(InlrAddressComputations);
    return -InlineConstants::InliningHeuristicBonus;
  }
  if (worthInliningForStackComputations(F, &TLI, PrepareForLTO)) {
    YesReasonVector.push_back(InlrStackComputations);
    return -InlineConstants::InliningHeuristicBonus;;
  }
  if (worthInliningSingleBasicBlockWithStructTest(F, PrepareForLTO,
      QueuedCallers)) {
    YesReasonVector.push_back(InlrSingleBasicBlockWithStructTest);
    return -InlineConstants::InliningHeuristicBonus;;
  }
  if (worthInliningForRecProgressionClone(F)) {
    YesReasonVector.push_back(InlrRecProClone);
    return -InlineConstants::VeryDeepInliningHeuristicBonus;
  }
  if (preferPartialInlineHasExtractedRecursiveCall(*F, PrepareForLTO)) {
    YesReasonVector.push_back(InlrHasExtractedRecursiveCall);
    return -InlineConstants::DeepInliningHeuristicBonus;
  }
  if (preferPartialInlineInlinedClone(F)) {
    YesReasonVector.push_back(InlrPreferPartialInline);
    return -InlineConstants::DeepInliningHeuristicBonus;
  }
  if (worthInliningCallSitePassedDummyArgs(CB, PrepareForLTO)) {
    YesReasonVector.push_back(InlrPassedDummyArgs);
    return -InlineConstants::DeepInliningHeuristicBonus;
  }
  if (worthInliningForArrayStructArgs(CB, PrepareForLTO)) {
    YesReasonVector.push_back(InlrArrayStructArgs);
    return -InlineConstants::DeepInliningHeuristicBonus;
  }
  return 0;
}

#endif // INTEL_CUSTOMIZATION

/// Analyze a call site for potential inlining.
///
/// Returns true if inlining this call is viable, and false if it is not
/// viable. It computes the cost and adjusts the threshold based on numerous
/// factors and heuristics. If this method returns false but the computed cost
/// is below the computed threshold, then inlining was forcibly disabled by
/// some artifact of the routine.
///
/// INTEL The Intel version also sets the value of *Reason to be the principal
/// INTEL the call site would be inlined or not inlined.

#if INTEL_CUSTOMIZATION
InlineResult CallAnalyzer::analyzeCall(CallBase &Call,
                                       const TargetTransformInfo &CalleeTTI,
                                       InlineReason *Reason) {
#endif // INTEL_CUSTOMIZATION
  ++NumCallsAnalyzed;
  InlineReason TempReason = NinlrNoReason; // INTEL
  InlineReason* ReasonAddr = Reason == nullptr ? &TempReason : Reason; // INTEL
  InlineReasonVector YesReasonVector; // INTEL
  InlineReasonVector NoReasonVector; // INTEL
  TempReason = NinlrNoReason; // INTEL
  EarlyExitCost = INT_MAX; // INTEL
  EarlyExitThreshold = INT_MAX; // INTEL
  static SmallPtrSet<Function *, 10> QueuedCallers; // INTEL

  // Perform some tweaks to the cost and threshold based on the direct
  // callsite information.

  // We want to more aggressively inline vector-dense kernels, so up the
  // threshold, and we'll lower it if the % of vector instructions gets too
  // low. Note that these bonuses are some what arbitrary and evolved over time
  // by accident as much as because they are principled bonuses.
  //
  // FIXME: It would be nice to remove all such bonuses. At least it would be
  // nice to base the bonus values on something more scientific.
  assert(NumInstructions == 0);
  assert(NumVectorInstructions == 0);

  // Update the threshold based on callsite properties
  updateThreshold(Call, F, YesReasonVector); // INTEL

  // While Threshold depends on commandline options that can take negative
  // values, we want to enforce the invariant that the computed threshold and
  // bonuses are non-negative.
#if INTEL_CUSTOMIZATION
  // There is nothing in updateThreshold() to put a lower limit of 0 on the
  // Threshold when -inline-threshold is specified as < 0. Commenting this
  // out, at least for now. The same goes for the SingleBBBonus and VectorBonus,
  // which are derived from Threshold.
  // assert(Threshold >= 0);
  // assert(SingleBBBonus >= 0);
  // assert(VectorBonus >= 0);
#endif // INTEL_CUSTOMIZATION

  // INTEL  CQ378383: Tolerate a single "forgivable" condition when optimizing
  // INTEL  for size. In this case, we delay subtracting out the single basic
  // INTEL  block bonus until we see a second branch with multiple targets.
  bool SeekingForgivable = Call.getCaller()->hasOptSize(); // INTEL
  bool FoundForgivable = false;                          // INTEL
  bool SubtractedBonus = false;                          // INTEL
  bool PrepareForLTO = Params.PrepareForLTO.getValueOr(false); // INTEL

  // Speculatively apply all possible bonuses to Threshold. If cost exceeds
  // this Threshold any time, and cost cannot decrease, we can stop processing
  // the rest of the function body.
  Threshold += (SingleBBBonus + VectorBonus);
#if INTEL_CUSTOMIZATION
  Function *Callee = Call.getCalledFunction();
  if (Callee && InlineForXmain) {
    Optional<uint64_t> ProfCount = profInstrumentCount(PSI, Call);
    if (ProfCount && ProfCount.getValue() == 0) {
      if (!Callee->hasLinkOnceODRLinkage()) {
        *ReasonAddr = NinlrColdProfile;
        return false;
      }
      NoReasonVector.push_back(NinlrColdProfile);
    }
    if (preferCloningToInlining(Call, *ILIC, PrepareForLTO)) {
      *ReasonAddr = NinlrPreferCloning;
      return "prefer cloning";
    }
    if (preferMultiversioningToInlining(Call, CalleeTTI, *ILIC,
        PrepareForLTO)) {
      *ReasonAddr = NinlrPreferMultiversioning;
      return false;
    }
    if (preferDTransToInlining(Call, PrepareForLTO)) {
      *ReasonAddr = NinlrPreferSOAToAOS;
      return false;
    }
    if (preferNotToInlineForStackComputations(Call, TLI)) {
      *ReasonAddr = NinlrStackComputations;
      return false;
    }
    if (preferNotToInlineForSwitchComputations(Call, *ILIC)) {
      *ReasonAddr = NinlrSwitchComputations;
      return false;
    }
    if (preferNotToInlineForRecProgressionClone(Callee)) {
      *ReasonAddr = NinlrRecursive;
      return false;
    }
    if (preferToDelayInlineDecision(Call.getCaller(), PrepareForLTO,
        QueuedCallers)) {
      *ReasonAddr = NinlrDelayInlineDecision;
      return false;
    }
    if (preferPartialInlineOutlinedFunc(Callee)) {
      *ReasonAddr = NinlrPreferPartialInline;
      return false;
    }
    if (preferToIntelPartialInline(*Callee, PrepareForLTO, *ILIC)) {
      *ReasonAddr = NinlrDelayInlineDecision;
      return false;
    }
    if (preferNotToInlineEHIntoLoop(Call, *ILIC)) {
      *ReasonAddr = NinlrCalleeHasExceptionHandling;
      return false;
    }
    if (Call.getCaller() == Callee &&
      Callee->hasFnAttribute("no-more-recursive-inlining")) {
      *ReasonAddr = NinlrRecursive;
      return false;
    }
  }
#endif // INTEL_CUSTOMIZATION

  // Give out bonuses for the callsite, as the instructions setting them up
  // will be gone after inlining.
  addCost(-getCallsiteCost(Call, DL));

#if INTEL_CUSTOMIZATION
  Function *Caller = Call.getFunction();
  // Check if the caller function is recursive itself.
  for (User *U : Caller->users()) {
    CallBase *Call = dyn_cast<CallBase>(U);
    if (Call && Call->getFunction() == Caller) {
      IsCallerRecursive = true;
      break;
    }
  }

  if (InlineForXmain) {
    if (&F == Call.getCalledFunction()) {
      addCost(worthInliningUnderSpecialCondition(
          Call, *TLI, CalleeTTI, *ILIC, PSI, PrepareForLTO, IsCallerRecursive,
          CallSitesForFusion, &QueuedCallers, YesReasonVector));
    }
  }

  // Use InlineAggressiveInfo to expose uses of global ptrs
  if (InlineForXmain && AI != nullptr && AI->isCallInstInAggInlList(Call)) {
    addCost(-InlineConstants::AggressiveInlineCallBonus);
    YesReasonVector.push_back(InlrAggInline);
  }
#endif // INTEL_CUSTOMIZATION

  // If this function uses the coldcc calling convention, prefer not to inline
  // it.
  if (F.getCallingConv() == CallingConv::Cold) { // INTEL
    Cost += InlineConstants::ColdccPenalty;
    NoReasonVector.push_back(NinlrColdCC); // INTEL
  } // INTEL

#if INTEL_CUSTOMIZATION
  if (Cost >= Threshold) {
    if (!ComputeFullInlineCost) {
      *ReasonAddr = bestInlineReason(NoReasonVector, NinlrNotProfitable);
      return "high cost";
    }
    if (EarlyExitCost == INT_MAX) {
      EarlyExitCost = Cost;
      EarlyExitThreshold = Threshold;
    }
 }
#endif // INTEL_CUSTOMIZATION

  if (F.empty()) { // INTEL
    *ReasonAddr = InlrEmptyFunction; // INTEL
    return true;
  } // INTEL

  // Populate our simplified values by mapping from function arguments to call
  // arguments with known important simplifications.
  auto CAI = Call.arg_begin();
  for (Function::arg_iterator FAI = F.arg_begin(), FAE = F.arg_end();
       FAI != FAE; ++FAI, ++CAI) {
    assert(CAI != Call.arg_end());
    if (Constant *C = dyn_cast<Constant>(CAI))
      SimplifiedValues[&*FAI] = C;

    Value *PtrArg = *CAI;
    if (ConstantInt *C = stripAndComputeInBoundsConstantOffsets(PtrArg)) {
      ConstantOffsetPtrs[&*FAI] = std::make_pair(PtrArg, C->getValue());

      // We can SROA any pointer arguments derived from alloca instructions.
      if (isa<AllocaInst>(PtrArg)) {
        SROAArgValues[&*FAI] = PtrArg;
        SROAArgCosts[PtrArg] = 0;
      }
    }
  }
  NumConstantArgs = SimplifiedValues.size();
  NumConstantOffsetPtrArgs = ConstantOffsetPtrs.size();
  NumAllocaArgs = SROAArgValues.size();

  // FIXME: If a caller has multiple calls to a callee, we end up recomputing
  // the ephemeral values multiple times (and they're completely determined by
  // the callee, so this is purely duplicate work).
  SmallPtrSet<const Value *, 32> EphValues;
  CodeMetrics::collectEphemeralValues(&F, &GetAssumptionCache(F), EphValues);

  // The worklist of live basic blocks in the callee *after* inlining. We avoid
  // adding basic blocks of the callee which can be proven to be dead for this
  // particular call site in order to get more accurate cost estimates. This
  // requires a somewhat heavyweight iteration pattern: we need to walk the
  // basic blocks in a breadth-first order as we insert live successors. To
  // accomplish this, prioritizing for small iterations because we exit after
  // crossing our threshold, we use a small-size optimized SetVector.
  typedef SetVector<BasicBlock *, SmallVector<BasicBlock *, 16>,
                    SmallPtrSet<BasicBlock *, 16>>
      BBSetVector;
  BBSetVector BBWorklist;
  BBWorklist.insert(&F.getEntryBlock());
  bool SingleBB = true;
  // Note that we *must not* cache the size, this loop grows the worklist.
  for (unsigned Idx = 0; Idx != BBWorklist.size(); ++Idx) {
    // Bail out the moment we cross the threshold. This means we'll under-count
    // the cost, but only when undercounting doesn't matter.
#if INTEL_CUSTOMIZATION
    if (Cost >= Threshold) {
      if (!ComputeFullInlineCost)
        break;
      if (EarlyExitCost == INT_MAX) {
        EarlyExitCost = Cost;
        EarlyExitThreshold = Threshold;
      }
    }
#endif // INTEL_CUSTOMIZATION

    BasicBlock *BB = BBWorklist[Idx];
    if (BB->empty())
      continue;

    // Disallow inlining a blockaddress with uses other than strictly callbr.
    // A blockaddress only has defined behavior for an indirect branch in the
    // same function, and we do not currently support inlining indirect
    // branches.  But, the inliner may not see an indirect branch that ends up
    // being dead code at a particular call site. If the blockaddress escapes
    // the function, e.g., via a global variable, inlining may lead to an
    // invalid cross-function reference.
    // FIXME: pr/39560: continue relaxing this overt restriction.
    if (BB->hasAddressTaken())
      for (User *U : BlockAddress::get(&*BB)->users())
        if (!isa<CallBrInst>(*U)) { // INTEL
          *ReasonAddr = NinlrBlockAddress; // INTEL
          return "blockaddress used outside of callbr";
        } // INTEL

    // Analyze the cost of this block. If we blow through the threshold, this
    // returns false, and we can bail on out.
#if INTEL_CUSTOMIZATION
    InlineResult IR = analyzeBlock(BB, EphValues);
    if (!IR) {
      *ReasonAddr = NinlrNotProfitable;
      if (IsRecursiveCall) {
        *ReasonAddr = NinlrRecursive;
      }
      if (ExposesReturnsTwice) {
        *ReasonAddr = NinlrReturnsTwice;
      }
      if (HasDynamicAlloca) {
        *ReasonAddr = NinlrDynamicAlloca;
      }
      if (HasIndirectBr) {
        *ReasonAddr = NinlrIndirectBranch;
      }
      if (HasUninlineableIntrinsic) {
        *ReasonAddr = NinlrCallsLocalEscape;
      }
      if (IsCallerRecursive &&
          AllocatedSize > InlineConstants::TotalAllocaSizeRecursiveCaller) {
        *ReasonAddr = NinlrTooMuchStack;
      }
      if (!ComputeFullInlineCost || (*ReasonAddr) != NinlrNotProfitable)
        return IR;
      if (EarlyExitCost == INT_MAX) {
        EarlyExitCost = Cost;
        EarlyExitThreshold = Threshold;
      }
    }
#endif // INTEL_CUSTOMIZATION

    Instruction *TI = BB->getTerminator();

    // Add in the live successors by first checking whether we have terminator
    // that may be simplified based on the values simplified by this call.
    if (BranchInst *BI = dyn_cast<BranchInst>(TI)) {
      if (BI->isConditional()) {
        Value *Cond = BI->getCondition();
        if (ConstantInt *SimpleCond =
                dyn_cast_or_null<ConstantInt>(SimplifiedValues.lookup(Cond))) {
          BasicBlock *NextBB = BI->getSuccessor(SimpleCond->isZero() ? 1 : 0);
          BBWorklist.insert(NextBB);
          KnownSuccessors[BB] = NextBB;
          findDeadBlocks(BB, NextBB);
          continue;
        }
      }
    } else if (SwitchInst *SI = dyn_cast<SwitchInst>(TI)) {
      Value *Cond = SI->getCondition();
      if (ConstantInt *SimpleCond =
              dyn_cast_or_null<ConstantInt>(SimplifiedValues.lookup(Cond))) {
        BasicBlock *NextBB = SI->findCaseValue(SimpleCond)->getCaseSuccessor();
        BBWorklist.insert(NextBB);
        KnownSuccessors[BB] = NextBB;
        findDeadBlocks(BB, NextBB);
        continue;
      }
    }

    // If we're unable to select a particular successor, just count all of
    // them.
    for (unsigned TIdx = 0, TSize = TI->getNumSuccessors(); TIdx != TSize;
         ++TIdx)
      BBWorklist.insert(TI->getSuccessor(TIdx));

    // If we had any successors at this point, than post-inlining is likely to
    // have them as well. Note that we assume any basic blocks which existed
    // due to branches or switches which folded above will also fold after
    // inlining.
#if INTEL_CUSTOMIZATION
    if (InlineForXmain) {
      if (TI->getNumSuccessors() > 1) {
        if (SeekingForgivable && forgivableCondition(TI)) {
          FoundForgivable = true;
          addCost(-InlineConstants::InstrCost);
        }
        else {
          if (!SubtractedBonus) {
            SubtractedBonus = true;
            Threshold -= SingleBBBonus;
          }
          FoundForgivable = false;
        }
        SingleBB = false;
      }
    }
    else {
      if (SingleBB && TI->getNumSuccessors() > 1) {
        // Take off the bonus we applied to the threshold.
        Threshold -= SingleBBBonus;
        SingleBB = false;
      }
    }
#endif // INTEL_CUSTOMIZATION
  }

#if INTEL_CUSTOMIZATION
  if (SingleBB)
    YesReasonVector.push_back(InlrSingleBasicBlock);
  else if (FoundForgivable)
    YesReasonVector.push_back(InlrSingleBasicBlockWithTest);
#endif // INTEL_CUSTOMIZATION

  bool OnlyOneCallAndLocalLinkage =
      (F.hasLocalLinkage()                                  // INTEL
       || (InlineForXmain && F.hasLinkOnceODRLinkage())) && // INTEL
      F.hasOneUse() && &F == Call.getCalledFunction();      // INTEL
  // If this is a noduplicate call, we can still inline as long as
  // inlining this would cause the removal of the caller (so the instruction
  // is not actually duplicated, just moved).
  if (!OnlyOneCallAndLocalLinkage && ContainsNoDuplicateCall) { // INTEL
    *ReasonAddr = NinlrDuplicateCall; // INTEL
    return "noduplicate";
  } // INTEL

  // Loops generally act a lot like calls in that they act like barriers to
  // movement, require a certain amount of setup, etc. So when optimising for
  // size, we penalise any call sites that perform loops. We do this after all
  // other costs here, so will likely only be dealing with relatively small
  // functions (and hence DT and LI will hopefully be cheap).
  if (Caller->hasMinSize()) {
    DominatorTree DT(F);
    LoopInfo LI(DT);
    int NumLoops = 0;
    for (Loop *L : LI) {
      // Ignore loops that will not be executed
      if (DeadBlocks.count(L->getHeader()))
        continue;
      NumLoops++;
    }
    addCost(NumLoops * InlineConstants::CallPenalty);
  }

  // We applied the maximum possible vector bonus at the beginning. Now,
  // subtract the excess bonus, if any, from the Threshold before
  // comparing against Cost.
  if (NumVectorInstructions <= NumInstructions / 10)
    Threshold -= VectorBonus;
  else if (NumVectorInstructions <= NumInstructions / 2)
    Threshold -= VectorBonus/2;

#if INTEL_CUSTOMIZATION
  if (NumVectorInstructions > NumInstructions / 10) {
    YesReasonVector.push_back(InlrVectorBonus);
  }
  bool IsProfitable = Cost < std::max(1, Threshold);
  if (IsProfitable) {
    *ReasonAddr = bestInlineReason(YesReasonVector, InlrProfitable);
  }
  else {
    *ReasonAddr = bestInlineReason(NoReasonVector, NinlrNotProfitable);
  }
  if (!IsProfitable)
    return "not profitable";
  return true;
#endif // INTEL_CUSTOMIZATION
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
/// Dump stats about this call's analysis.
LLVM_DUMP_METHOD void CallAnalyzer::dump() {
#define DEBUG_PRINT_STAT(x) dbgs() << "      " #x ": " << x << "\n"
  DEBUG_PRINT_STAT(NumConstantArgs);
  DEBUG_PRINT_STAT(NumConstantOffsetPtrArgs);
  DEBUG_PRINT_STAT(NumAllocaArgs);
  DEBUG_PRINT_STAT(NumConstantPtrCmps);
  DEBUG_PRINT_STAT(NumConstantPtrDiffs);
  DEBUG_PRINT_STAT(NumInstructionsSimplified);
  DEBUG_PRINT_STAT(NumInstructions);
  DEBUG_PRINT_STAT(SROACostSavings);
  DEBUG_PRINT_STAT(SROACostSavingsLost);
  DEBUG_PRINT_STAT(LoadEliminationCost);
  DEBUG_PRINT_STAT(ContainsNoDuplicateCall);
  DEBUG_PRINT_STAT(Cost);
  DEBUG_PRINT_STAT(Threshold);
#undef DEBUG_PRINT_STAT
}
#endif

/// Test that there are no attribute conflicts between Caller and Callee
///        that prevent inlining.
static bool functionsHaveCompatibleAttributes(Function *Caller,
                                              Function *Callee,
                                              TargetTransformInfo &TTI) {
  return TTI.areInlineCompatible(Caller, Callee) &&
         AttributeFuncs::areInlineCompatible(*Caller, *Callee);
}

int llvm::getCallsiteCost(CallBase &Call, const DataLayout &DL) {
  int Cost = 0;
  for (unsigned I = 0, E = Call.arg_size(); I != E; ++I) {
    if (Call.isByValArgument(I)) {
      // We approximate the number of loads and stores needed by dividing the
      // size of the byval type by the target's pointer size.
      PointerType *PTy = cast<PointerType>(Call.getArgOperand(I)->getType());
      unsigned TypeSize = DL.getTypeSizeInBits(PTy->getElementType());
      unsigned AS = PTy->getAddressSpace();
      unsigned PointerSize = DL.getPointerSizeInBits(AS);
      // Ceiling division.
      unsigned NumStores = (TypeSize + PointerSize - 1) / PointerSize;

      // If it generates more than 8 stores it is likely to be expanded as an
      // inline memcpy so we take that as an upper bound. Otherwise we assume
      // one load and one store per word copied.
      // FIXME: The maxStoresPerMemcpy setting from the target should be used
      // here instead of a magic number of 8, but it's not available via
      // DataLayout.
      NumStores = std::min(NumStores, 8U);

      Cost += 2 * NumStores * InlineConstants::InstrCost;
    } else {
      // For non-byval arguments subtract off one instruction per call
      // argument.
      Cost += InlineConstants::InstrCost;
    }
  }
  // The call instruction also disappears after inlining.
  Cost += InlineConstants::InstrCost + InlineConstants::CallPenalty;
  return Cost;
}

InlineCost llvm::getInlineCost(
    CallBase &Call, const InlineParams &Params, TargetTransformInfo &CalleeTTI,
    std::function<AssumptionCache &(Function &)> &GetAssumptionCache,
    Optional<function_ref<BlockFrequencyInfo &(Function &)>> GetBFI,
    TargetLibraryInfo *TLI,      // INTEL
    InliningLoopInfoCache *ILIC, // INTEL
    InlineAggressiveInfo *AI,    // INTEL
    SmallSet<CallBase *, 20> *CallSitesForFusion, // INTEL
    SmallSet<Function *, 20> *FuncsForDTrans, // INTEL
    ProfileSummaryInfo *PSI, OptimizationRemarkEmitter *ORE) {
  return getInlineCost(Call, Call.getCalledFunction(), Params, CalleeTTI,
                       GetAssumptionCache, GetBFI, TLI, ILIC, AI, // INTEL
                       CallSitesForFusion,                        // INTEL
                       FuncsForDTrans, PSI, ORE);             // INTEL
}

InlineCost llvm::getInlineCost(
    CallBase &Call, Function *Callee, const InlineParams &Params,
    TargetTransformInfo &CalleeTTI,
    std::function<AssumptionCache &(Function &)> &GetAssumptionCache,
    Optional<function_ref<BlockFrequencyInfo &(Function &)>> GetBFI,
    TargetLibraryInfo *TLI,         // INTEL
    InliningLoopInfoCache *ILIC,    // INTEL
    InlineAggressiveInfo *AI,       // INTEL
    SmallSet<CallBase *, 20> *CallSitesForFusion, // INTEL
    SmallSet<Function *, 20> *FuncsForDTrans, // INTEL
    ProfileSummaryInfo *PSI, OptimizationRemarkEmitter *ORE) {

  // Cannot inline indirect calls.
  if (!Callee)
    return llvm::InlineCost::getNever("indirect call", NinlrIndirect); // INTEL

  // Never inline calls with byval arguments that does not have the alloca
  // address space. Since byval arguments can be replaced with a copy to an
  // alloca, the inlined code would need to be adjusted to handle that the
  // argument is in the alloca address space (so it is a little bit complicated
  // to solve).
  unsigned AllocaAS = Callee->getParent()->getDataLayout().getAllocaAddrSpace();
  for (unsigned I = 0, E = Call.arg_size(); I != E; ++I)
    if (Call.isByValArgument(I)) {
      PointerType *PTy = cast<PointerType>(Call.getArgOperand(I)->getType());
      if (PTy->getAddressSpace() != AllocaAS)
        return llvm::InlineCost::getNever("byval arguments without alloca"
                                          " address space");
    }

  // Calls to functions with always-inline attributes should be inlined
  // whenever possible.

  if (Call.hasFnAttr(Attribute::AlwaysInline)) {
#if INTEL_CUSTOMIZATION
    InlineReason Reason = InlrNoReason;
    auto IsViable = isInlineViable(*Callee, Reason);
    if (IsViable)
      return llvm::InlineCost::getAlways("always inline attribute",
                                         InlrAlwaysInline);
    assert(IsNotInlinedReason(Reason));
    return llvm::InlineCost::getNever(IsViable.message,
                                      Reason);
  }
  if (Call.hasFnAttr("always-inline-recursive")) {
    InlineReason Reason = InlrNoReason;
    if (isInlineViable(*Callee, Reason))
      return llvm::InlineCost::getAlways("always inline recursive attribute",
                                         InlrAlwaysInlineRecursive);
    assert(IsNotInlinedReason(Reason));
    return llvm::InlineCost::getNever(
        "inapplicable always inline recursive attribute", Reason);
  }
#endif // INTEL_CUSTOMIZATION

  // Never inline functions with conflicting attributes (unless callee has
  // always-inline attribute).
  Function *Caller = Call.getCaller();
  if (!functionsHaveCompatibleAttributes(Caller, Callee, CalleeTTI))
    return llvm::InlineCost::getNever("conflicting attributes",   // INTEL
                                      NinlrMismatchedAttributes); // INTEL

  // Don't inline this call if the caller has the optnone attribute.
  if (Caller->hasOptNone())
    return llvm::InlineCost::getNever("optnone attribute",  // INTEL
                                      NinlrOptNone);        // INTEL

  // Don't inline a function that treats null pointer as valid into a caller
  // that does not have this attribute.
  if (!Caller->nullPointerIsDefined() && Callee->nullPointerIsDefined())
    return llvm::InlineCost::getNever(                             // INTEL
        "nullptr definitions incompatible", NinlrNullPtrMismatch); // INTEL

  // Don't inline functions which can be interposed at link-time.  Don't inline
  // functions marked noinline or call sites marked noinline.
  // Note: inlining non-exact non-interposable functions is fine, since we know
  // we have *a* correct implementation of the source level function.
  if (Callee->isInterposable() || Callee->hasFnAttribute(Attribute::NoInline) ||
      Call.isNoInline()) { // INTEL
#if INTEL_CUSTOMIZATION
    if (Callee->isInterposable()) {
      return llvm::InlineCost::getNever("interposable", NinlrMayBeOverriden);
    }
    if (Callee->hasFnAttribute(Attribute::NoInline)) {
      return llvm::InlineCost::getNever("noinline function attribute",
                                        NinlrNoinlineAttribute);
    }
    if (Call.isNoInline()) {
      return llvm::InlineCost::getNever("noinline call site attribute",
                                        NinlrNoinlineCallsite);
    }
#endif // INTEL_CUSTOMIZATION
  } // INTEL

  LLVM_DEBUG(llvm::dbgs() << "      Analyzing call of " << Callee->getName()
                          << "... (caller:" << Caller->getName() << ")\n");

  CallAnalyzer CA(CalleeTTI, GetAssumptionCache, GetBFI, PSI, ORE, *Callee,
                  Call, TLI, ILIC, AI, CallSitesForFusion,   // INTEL
                  FuncsForDTrans, Params);               // INTEL
#if INTEL_CUSTOMIZATION
  InlineReason Reason = InlrNoReason;
  InlineResult ShouldInline = CA.analyzeCall(Call, CalleeTTI, &Reason);
  assert(Reason != InlrNoReason);
#endif // INTEL_CUSTOMIZATION

  LLVM_DEBUG(CA.dump());

  // Check if there was a reason to force inlining or no inlining.
  if (!ShouldInline && CA.getCost() < CA.getThreshold())
    return InlineCost::getNever(ShouldInline.message, Reason); // INTEL
  if (ShouldInline && CA.getCost() >= CA.getThreshold())
    return InlineCost::getAlways("empty function", Reason); // INTEL

  return llvm::InlineCost::get(CA.getCost(),            // INTEL
    CA.getThreshold(), nullptr, Reason,                 // INTEL
    CA.getEarlyExitCost(), CA.getEarlyExitThreshold()); // INTEL
}

InlineResult llvm::isInlineViable(Function &F, // INTEL
                                  InlineReason& Reason) { // INTEL
  bool ReturnsTwice = F.hasFnAttribute(Attribute::ReturnsTwice);
  for (Function::iterator BI = F.begin(), BE = F.end(); BI != BE; ++BI) {
    // Disallow inlining of functions which contain indirect branches.
    if (isa<IndirectBrInst>(BI->getTerminator())) { // INTEL
      Reason = NinlrIndirectBranch; // INTEL
      return "contains indirect branches";
    } // INTEL

    // Disallow inlining of blockaddresses which are used by non-callbr
    // instructions.
    if (BI->hasAddressTaken())
      for (User *U : BlockAddress::get(&*BI)->users())
        if (!isa<CallBrInst>(*U)) { // INTEL
          Reason = NinlrBlockAddress; // INTEL
          return "blockaddress used outside of callbr";
        } // INTEL

    for (auto &II : *BI) {
      CallBase *Call = dyn_cast<CallBase>(&II);
      if (!Call)
        continue;

      // Disallow recursive calls.
      if (&F == Call->getCalledFunction()) { // INTEL
        Reason = NinlrRecursive; // INTEL
        return "recursive call";
      } // INTEL

      // Disallow calls which expose returns-twice to a function not previously
      // attributed as such.
      if (!ReturnsTwice && isa<CallInst>(Call) &&
          cast<CallInst>(Call)->canReturnTwice()) { // INTEL
        Reason = NinlrReturnsTwice; // INTEL
        return "exposes returns-twice attribute";
      } // INTEL

      if (Call->getCalledFunction())
        switch (Call->getCalledFunction()->getIntrinsicID()) {
        default:
          break;
        // Disallow inlining of @llvm.icall.branch.funnel because current
        // backend can't separate call targets from call arguments.
        case llvm::Intrinsic::icall_branch_funnel:
          Reason = NinlrCallsLocalEscape; // INTEL
          return "disallowed inlining of @llvm.icall.branch.funnel";
        // Disallow inlining functions that call @llvm.localescape. Doing this
        // correctly would require major changes to the inliner.
        case llvm::Intrinsic::localescape:
          Reason = NinlrCallsLocalEscape; // INTEL
          return "disallowed inlining of @llvm.localescape";
        // Disallow inlining of functions that initialize VarArgs with va_start.
        case llvm::Intrinsic::vastart:
          Reason = NinlrVarargs; // INTEL
          return "contains VarArgs initialized with va_start";
        }
    }
  }

  return true;
}

// APIs to create InlineParams based on command line flags and/or other
// parameters.

InlineParams llvm::getInlineParams(int Threshold) {
  InlineParams Params;
  Params.PrepareForLTO = EnablePreLTOInlineCost;     // INTEL

  // This field is the threshold to use for a callee by default. This is
  // derived from one or more of:
  //  * optimization or size-optimization levels,
  //  * a value passed to createFunctionInliningPass function, or
  //  * the -inline-threshold flag.
  //  If the -inline-threshold flag is explicitly specified, that is used
  //  irrespective of anything else.
  if (InlineThreshold.getNumOccurrences() > 0)
    Params.DefaultThreshold = InlineThreshold;
  else
    Params.DefaultThreshold = Threshold;

  // Set the HintThreshold knob from the -inlinehint-threshold.
  Params.HintThreshold = HintThreshold;

  // Set the HotCallSiteThreshold knob from the -hot-callsite-threshold.
  Params.HotCallSiteThreshold = HotCallSiteThreshold;

  // If the -locally-hot-callsite-threshold is explicitly specified, use it to
  // populate LocallyHotCallSiteThreshold. Later, we populate
  // Params.LocallyHotCallSiteThreshold from -locally-hot-callsite-threshold if
  // we know that optimization level is O3 (in the getInlineParams variant that
  // takes the opt and size levels).
  // FIXME: Remove this check (and make the assignment unconditional) after
  // addressing size regression issues at O2.
  if (LocallyHotCallSiteThreshold.getNumOccurrences() > 0)
    Params.LocallyHotCallSiteThreshold = LocallyHotCallSiteThreshold;

  // Set the ColdCallSiteThreshold knob from the -inline-cold-callsite-threshold.
  Params.ColdCallSiteThreshold = ColdCallSiteThreshold;

  // Set the OptMinSizeThreshold and OptSizeThreshold params only if the
  // -inlinehint-threshold commandline option is not explicitly given. If that
  // option is present, then its value applies even for callees with size and
  // minsize attributes.
  // If the -inline-threshold is not specified, set the ColdThreshold from the
  // -inlinecold-threshold even if it is not explicitly passed. If
  // -inline-threshold is specified, then -inlinecold-threshold needs to be
  // explicitly specified to set the ColdThreshold knob
  if (InlineThreshold.getNumOccurrences() == 0) {
    Params.OptMinSizeThreshold = InlineConstants::OptMinSizeThreshold;
    Params.OptSizeThreshold = InlineForXmain
     ? OptSizeThreshold : InlineConstants::OptSizeThreshold; // INTEL
    Params.ColdThreshold = ColdThreshold;
  } else if (ColdThreshold.getNumOccurrences() > 0) {
    Params.ColdThreshold = ColdThreshold;
  }
  return Params;
}

InlineParams llvm::getInlineParams() {
  return getInlineParams(InlineThreshold);
}

// Compute the default threshold for inlining based on the opt level and the
// size opt level.
static int computeThresholdFromOptLevels(unsigned OptLevel,
                                         unsigned SizeOptLevel) {
  if (OptLevel > 2)
    return InlineConstants::OptAggressiveThreshold;
  if (SizeOptLevel == 1) // -Os
    return InlineForXmain
      ? OptSizeThreshold : InlineConstants::OptSizeThreshold; // INTEL
  if (SizeOptLevel == 2) // -Oz
    return InlineConstants::OptMinSizeThreshold;
  return InlineThreshold;
}

InlineParams llvm::getInlineParams(unsigned OptLevel, unsigned SizeOptLevel) {
  auto Params =
      getInlineParams(computeThresholdFromOptLevels(OptLevel, SizeOptLevel));
  // At O3, use the value of -locally-hot-callsite-threshold option to populate
  // Params.LocallyHotCallSiteThreshold. Below O3, this flag has effect only
  // when it is specified explicitly.
  if (OptLevel > 2)
    Params.LocallyHotCallSiteThreshold = LocallyHotCallSiteThreshold;
  return Params;
}

#if INTEL_CUSTOMIZATION
// This routine does exactly same as what "getInlineParams(unsigned OptLevel,
// unsigned SizeOptLevel)" function does except it also sets PrepareForLTO
// flag in "InlineParams" based on "PrepareForLTO".
InlineParams llvm::getInlineParams(unsigned OptLevel, unsigned SizeOptLevel,
                                   bool PrepareForLTO) {
  InlineParams InlParams;

  InlParams = getInlineParams(OptLevel, SizeOptLevel);
  InlParams.PrepareForLTO = PrepareForLTO;
  return InlParams;
}
#endif // INTEL_CUSTOMIZATION
