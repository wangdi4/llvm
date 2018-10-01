//===- InlineCost.cpp - Cost analysis for inliner -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
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
#include "llvm/Analysis/LoopInfo.h" // INTEL
#include "llvm/Analysis/ProfileSummaryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/GetElementPtrTypeIterator.h"
#include "llvm/IR/GlobalAlias.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Operator.h"
#include "llvm/Transforms/IPO/Intel_IPCloning.h" // INTEL
#include "llvm/Support/GenericDomTree.h" // INTEL
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace InlineReportTypes; // INTEL

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
    "inlinehint-threshold", cl::Hidden, cl::init(325),
    cl::desc("Threshold for inlining functions with inline hint"));

static cl::opt<int>
    ColdCallSiteThreshold("inline-cold-callsite-threshold", cl::Hidden,
                          cl::init(45),
                          cl::desc("Threshold for inlining cold callsites"));

// We introduce this threshold to help performance of instrumentation based
// PGO before we actually hook up inliner with analysis passes such as BPI and
// BFI.
static cl::opt<int> ColdThreshold(
    "inlinecold-threshold", cl::Hidden, cl::init(45),
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
    cl::desc("Maxmimum block frequency, expressed as a percentage of caller's "
             "entry frequency, for a callsite to be cold in the absence of "
             "profile information."));

static cl::opt<int> HotCallSiteRelFreq(
    "hot-callsite-rel-freq", cl::Hidden, cl::init(60), cl::ZeroOrMore,
    cl::desc("Minimum block frequency, expressed as a multiple of caller's "
             "entry frequency, for a callsite to be hot in the absence of "
             "profile information."));

static cl::opt<bool> OptComputeFullInlineCost(
    "inline-cost-full", cl::Hidden, cl::init(false),
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
  CallSite CandidateCS;

  /// Tunable parameters that control the analysis.
  const InlineParams &Params;

  int Threshold;

  int Cost;
  bool ComputeFullInlineCost;

#if INTEL_CUSTOMIZATION
  /// The cost and the threshold used for early exit during usual
  /// inlining process. Under IntelInlineReportLevel=64  we compute both
  /// "early exit" and real cost of inlining.  A value of INT_MAX indicates
  /// that a value has not yet been seen for these.  They are expected to
  /// be set at the same time, so we only need to test for EarlyExitCost.
  int EarlyExitThreshold;
  int EarlyExitCost;

  InliningLoopInfoCache* ILIC;

  // Aggressive Analysis
  InlineAggressiveInfo *AI;
  // Set of candidate call sites for loop fusion
  SmallSet<CallSite, 20> *CallSitesForFusion;
#endif // INTEL_CUSTOMIZATION

  bool IsCallerRecursive;
  bool IsRecursiveCall;
  bool ExposesReturnsTwice;
  bool HasDynamicAlloca;
  bool ContainsNoDuplicateCall;
  bool HasReturn;
  bool HasIndirectBr;
  bool HasUninlineableIntrinsic;
  bool InitsVargArgs;

  /// Number of bytes allocated statically by the callee.
  uint64_t AllocatedSize;
  unsigned NumInstructions, NumVectorInstructions;
  int VectorBonus, TenPercentVectorBonus;
  // Bonus to be applied when the callee has only one reachable basic block.
  int SingleBBBonus;

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
  int LoadEliminationCost;

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
  bool simplifyCallSite(Function *F, CallSite CS);
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
  void updateThreshold(CallSite CS, Function &Callee,     // INTEL
    InlineReasonVector &YesReasonVector);                 // INTEL

  /// Return true if size growth is allowed when inlining the callee at CS.
  bool allowSizeGrowth(CallSite CS);

  /// Return true if \p CS is a cold callsite.
  bool isColdCallSite(CallSite CS, BlockFrequencyInfo *CallerBFI);

  /// Return a higher threshold if \p CS is a hot callsite.
  Optional<int> getHotCallSiteThreshold(CallSite CS,
                                        BlockFrequencyInfo *CallerBFI);

  // Custom analysis routines.
  InlineResult analyzeBlock(BasicBlock *BB,
                            SmallPtrSetImpl<const Value *> &EphValues);

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
  bool visitLoad(LoadInst &I);
  bool visitStore(StoreInst &I);
  bool visitExtractValue(ExtractValueInst &I);
  bool visitInsertValue(InsertValueInst &I);
  bool visitCallSite(CallSite CS);
  bool visitReturnInst(ReturnInst &RI);
  bool visitBranchInst(BranchInst &BI);
  bool visitSelectInst(SelectInst &SI);
  bool visitSwitchInst(SwitchInst &SI);
  bool visitIndirectBrInst(IndirectBrInst &IBI);
  bool visitResumeInst(ResumeInst &RI);
  bool visitCleanupReturnInst(CleanupReturnInst &RI);
  bool visitCatchReturnInst(CatchReturnInst &RI);
  bool visitUnreachableInst(UnreachableInst &I);

public:
  CallAnalyzer(const TargetTransformInfo &TTI,
               std::function<AssumptionCache &(Function &)> &GetAssumptionCache,
               Optional<function_ref<BlockFrequencyInfo &(Function &)>> &GetBFI,
               ProfileSummaryInfo *PSI, OptimizationRemarkEmitter *ORE,
               Function &Callee, CallSite CSArg,   // INTEL
               InliningLoopInfoCache *ILIC,        // INTEL
               InlineAggressiveInfo *AI,           // INTEL
               SmallSet<CallSite, 20> *CSForFusion, // INTEL
               const InlineParams &Params)
      : TTI(TTI), GetAssumptionCache(GetAssumptionCache), GetBFI(GetBFI),
        PSI(PSI), F(Callee), DL(F.getParent()->getDataLayout()), ORE(ORE),
        CandidateCS(CSArg), Params(Params), Threshold(Params.DefaultThreshold),
#if INTEL_CUSTOMIZATION
        Cost(0), ComputeFullInlineCost(OptComputeFullInlineCost ||
            Params.ComputeFullInlineCost.getValueOr(false) || ORE),
        ILIC(ILIC), AI(AI), CallSitesForFusion(CSForFusion),
#endif // INTEL_CUSTOMIZATION
        IsCallerRecursive(false), IsRecursiveCall(false),
        ExposesReturnsTwice(false), HasDynamicAlloca(false),
        ContainsNoDuplicateCall(false), HasReturn(false), HasIndirectBr(false),
        HasUninlineableIntrinsic(false), InitsVargArgs(false), AllocatedSize(0),
        NumInstructions(0), NumVectorInstructions(0), VectorBonus(0),
        SingleBBBonus(0), EnableLoadElimination(true), LoadEliminationCost(0),
        NumConstantArgs(0), NumConstantOffsetPtrArgs(0), NumAllocaArgs(0),
        NumConstantPtrCmps(0), NumConstantPtrDiffs(0),
        NumInstructionsSimplified(0), SROACostSavings(0),
        SROACostSavingsLost(0) {}

  InlineResult analyzeCall(CallSite CS, InlineReason* Reason); // INTEL

  int getThreshold() { return Threshold; }
  int getCost() { return Cost; }
  int getEarlyExitThreshold() { return EarlyExitThreshold; }  // INTEL
  int getEarlyExitCost() { return EarlyExitCost; }            // INTEL


  // Keep a bunch of stats about the cost savings found so we can print them
  // out when debugging.
  unsigned NumConstantArgs;
  unsigned NumConstantOffsetPtrArgs;
  unsigned NumAllocaArgs;
  unsigned NumConstantPtrCmps;
  unsigned NumConstantPtrDiffs;
  unsigned NumInstructionsSimplified;
  unsigned SROACostSavings;
  unsigned SROACostSavingsLost;

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
  Cost += CostIt->second;
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
    Cost += LoadEliminationCost;
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
          AllocSize->getLimitedValue(), DL.getTypeAllocSize(Ty), AllocatedSize);
      return Base::visitAlloca(I);
    }
  }

  // Accumulate the allocated size.
  if (I.isStaticAlloca()) {
    Type *Ty = I.getAllocatedType();
    AllocatedSize = SaturatingAdd(DL.getTypeAllocSize(Ty), AllocatedSize);
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
  // Propagate constants through ptrtoint.
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
      Cost += InlineConstants::CallPenalty;
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

  // Disable any SROA on the argument to arbitrary unary operators.
  disableSROA(Operand);

  return false;
}

bool CallAnalyzer::paramHasAttr(Argument *A, Attribute::AttrKind Attr) {
  return CandidateCS.paramHasAttr(A->getArgNo(), Attr);
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

bool CallAnalyzer::allowSizeGrowth(CallSite CS) {
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
  Instruction *Instr = CS.getInstruction();
  if (InvokeInst *II = dyn_cast<InvokeInst>(Instr)) {
    if (isa<UnreachableInst>(II->getNormalDest()->getTerminator()))
      return false;
  } else if (isa<UnreachableInst>(Instr->getParent()->getTerminator()))
    return false;

  return true;
}

bool CallAnalyzer::isColdCallSite(CallSite CS, BlockFrequencyInfo *CallerBFI) {
  // If global profile summary is available, then callsite's coldness is
  // determined based on that.
  if (PSI && PSI->hasProfileSummary())
    return PSI->isColdCallSite(CS, CallerBFI);

  // Otherwise we need BFI to be available.
  if (!CallerBFI)
    return false;

  // Determine if the callsite is cold relative to caller's entry. We could
  // potentially cache the computation of scaled entry frequency, but the added
  // complexity is not worth it unless this scaling shows up high in the
  // profiles.
  const BranchProbability ColdProb(ColdCallSiteRelFreq, 100);
  auto CallSiteBB = CS.getInstruction()->getParent();
  auto CallSiteFreq = CallerBFI->getBlockFreq(CallSiteBB);
  auto CallerEntryFreq =
      CallerBFI->getBlockFreq(&(CS.getCaller()->getEntryBlock()));
  return CallSiteFreq < CallerEntryFreq * ColdProb;
}

Optional<int>
CallAnalyzer::getHotCallSiteThreshold(CallSite CS,
                                      BlockFrequencyInfo *CallerBFI) {

  // If global profile summary is available, then callsite's hotness is
  // determined based on that.
  if (PSI && PSI->hasProfileSummary() && PSI->isHotCallSite(CS, CallerBFI))
    return Params.HotCallSiteThreshold;

  // Otherwise we need BFI to be available and to have a locally hot callsite
  // threshold.
  if (!CallerBFI || !Params.LocallyHotCallSiteThreshold)
    return None;

  // Determine if the callsite is hot relative to caller's entry. We could
  // potentially cache the computation of scaled entry frequency, but the added
  // complexity is not worth it unless this scaling shows up high in the
  // profiles.
  auto CallSiteBB = CS.getInstruction()->getParent();
  auto CallSiteFreq = CallerBFI->getBlockFreq(CallSiteBB).getFrequency();
  auto CallerEntryFreq = CallerBFI->getEntryFreq();
  if (CallSiteFreq >= CallerEntryFreq * HotCallSiteRelFreq)
    return Params.LocallyHotCallSiteThreshold;

  // Otherwise treat it normally.
  return None;
}

void CallAnalyzer::updateThreshold(CallSite CS, Function &Callee, // INTEL
  InlineReasonVector &YesReasonVector) {                          // INTEL
  // If no size growth is allowed for this inlining, set Threshold to 0.
  if (!allowSizeGrowth(CS)) {
    Threshold = 0;
    return;
  }

  Function *Caller = CS.getCaller();

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
  // Vector bonuses: We want to more aggressively inline vector-dense kernels
  // and apply this bonus based on the percentage of vector instructions. A
  // bonus is applied if the vector instructions exceed 50% and half that amount
  // is applied if it exceeds 10%. Note that these bonuses are some what
  // arbitrary and evolved over time by accident as much as because they are
  // principled bonuses.
  // FIXME: It would be nice to base the bonus values on something more
  // scientific.
  //
  // LstCallToStaticBonus: This large bonus is applied to ensure the inlining
  // of the last call to a static function as inlining such functions is
  // guaranteed to reduce code size.
  //
  // These bonus percentages may be set to 0 based on properties of the caller
  // and the callsite.
  int SingleBBBonusPercent = 50;
  int VectorBonusPercent = 150;
  int LastCallToStaticBonus = InlineConstants::LastCallToStaticBonus;

  // Lambda to set all the above bonus and bonus percentages to 0.
  auto DisallowAllBonuses = [&]() {
    SingleBBBonusPercent = 0;
    VectorBonusPercent = 0;
    LastCallToStaticBonus = 0;
  };

  // Use the OptMinSizeThreshold or OptSizeThreshold knob if they are available
  // and reduce the threshold if the caller has the necessary attribute.
  if (Caller->optForMinSize()) {
    Threshold = MinIfValid(Threshold, Params.OptMinSizeThreshold);
    // For minsize, we want to disable the single BB bonus and the vector
    // bonuses, but not the last-call-to-static bonus. Inlining the last call to
    // a static function will, at the minimum, eliminate the parameter setup and
    // call/return instructions.
    SingleBBBonusPercent = 0;
    VectorBonusPercent = 0;
  } else if (Caller->optForSize())
    Threshold = MinIfValid(Threshold, Params.OptSizeThreshold);

  // Adjust the threshold based on inlinehint attribute and profile based
  // hotness information if the caller does not have MinSize attribute.
  if (!Caller->optForMinSize()) {
    if (Callee.hasFnAttribute(Attribute::InlineHint) ||    // INTEL
        CS.hasFnAttr(Attribute::InlineHint) ||             // INTEL
        CS.hasFnAttr(Attribute::InlineHintRecursive))      // INTEL
      Threshold = MaxIfValid(Threshold, Params.HintThreshold);

    // FIXME: After switching to the new passmanager, simplify the logic below
    // by checking only the callsite hotness/coldness as we will reliably
    // have local profile information.
    //
    // Callsite hotness and coldness can be determined if sample profile is
    // used (which adds hotness metadata to calls) or if caller's
    // BlockFrequencyInfo is available.
    BlockFrequencyInfo *CallerBFI = GetBFI ? &((*GetBFI)(*Caller)) : nullptr;
    auto HotCallSiteThreshold = getHotCallSiteThreshold(CS, CallerBFI);
    if (!Caller->optForSize() && HotCallSiteThreshold) {
      LLVM_DEBUG(dbgs() << "Hot callsite.\n");
      // FIXME: This should update the threshold only if it exceeds the
      // current threshold, but AutoFDO + ThinLTO currently relies on this
      // behavior to prevent inlining of hot callsites during ThinLTO
      // compile phase.
      Threshold = HotCallSiteThreshold.getValue();
    } else if (isColdCallSite(CS, CallerBFI)) {
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
       F.hasOneUse() &&  &F == CS.getCalledFunction();          // INTEL
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
    SimpleV = SimplifyFPBinOp(I.getOpcode(), CLHS ? CLHS : LHS,
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
  // as such.
  if (I.getType()->isFloatingPointTy() &&
      TTI.getFPOpCost(I.getType()) == TargetTransformInfo::TCC_Expensive)
    Cost += InlineConstants::CallPenalty;

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
bool CallAnalyzer::simplifyCallSite(Function *F, CallSite CS) {
  // FIXME: Using the instsimplify logic directly for this is inefficient
  // because we have to continually rebuild the argument list even when no
  // simplifications can be performed. Until that is fixed with remapping
  // inside of instsimplify, directly constant fold calls here.
  if (!canConstantFoldCallTo(CS, F))
    return false;

  // Try to re-map the arguments to constants.
  SmallVector<Constant *, 4> ConstantArgs;
  ConstantArgs.reserve(CS.arg_size());
  for (CallSite::arg_iterator I = CS.arg_begin(), E = CS.arg_end(); I != E;
       ++I) {
    Constant *C = dyn_cast<Constant>(*I);
    if (!C)
      C = dyn_cast_or_null<Constant>(SimplifiedValues.lookup(*I));
    if (!C)
      return false; // This argument doesn't map to a constant.

    ConstantArgs.push_back(C);
  }
  if (Constant *C = ConstantFoldCall(CS, F, ConstantArgs)) {
    SimplifiedValues[CS.getInstruction()] = C;
    return true;
  }

  return false;
}

bool CallAnalyzer::visitCallSite(CallSite CS) {
  if (CS.hasFnAttr(Attribute::ReturnsTwice) &&
      !F.hasFnAttribute(Attribute::ReturnsTwice)) {
    // This aborts the entire analysis.
    ExposesReturnsTwice = true;
    return false;
  }
  if (CS.isCall() && cast<CallInst>(CS.getInstruction())->cannotDuplicate())
    ContainsNoDuplicateCall = true;

  if (Function *F = CS.getCalledFunction()) {
    // When we have a concrete function, first try to simplify it directly.
    if (simplifyCallSite(F, CS))
      return true;

    // Next check if it is an intrinsic we know about.
    // FIXME: Lift this into part of the InstVisitor.
    if (IntrinsicInst *II = dyn_cast<IntrinsicInst>(CS.getInstruction())) {
      switch (II->getIntrinsicID()) {
      default:
        if (!CS.onlyReadsMemory() && !isAssumeLikeIntrinsic(II))
          disableLoadElimination();
        return Base::visitCallSite(CS);

      case Intrinsic::load_relative:
        // This is normally lowered to 4 LLVM instructions.
        Cost += 3 * InlineConstants::InstrCost;
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

    if (F == CS.getInstruction()->getFunction()) {
      // This flag will fully abort the analysis, so don't bother with anything
      // else.
      IsRecursiveCall = true;
      return false;
    }

    if (TTI.isLoweredToCall(F)) {
      // We account for the average 1 instruction per call argument setup
      // here.
      Cost += CS.arg_size() * InlineConstants::InstrCost;

      // Everything other than inline ASM will also have a significant cost
      // merely from making the call.
      if (!isa<InlineAsm>(CS.getCalledValue()))
        Cost += InlineConstants::CallPenalty;
    }

    if (!CS.onlyReadsMemory())
      disableLoadElimination();
    return Base::visitCallSite(CS);
  }

  // Otherwise we're in a very special case -- an indirect function call. See
  // if we can be particularly clever about this.
  Value *Callee = CS.getCalledValue();

  // First, pay the price of the argument setup. We account for the average
  // 1 instruction per call argument setup here.
  Cost += CS.arg_size() * InlineConstants::InstrCost;

  // Next, check if this happens to be an indirect function call to a known
  // function in this inline context. If not, we've done all we can.
  Function *F = dyn_cast_or_null<Function>(SimplifiedValues.lookup(Callee));
  if (!F) {
    if (!CS.onlyReadsMemory())
      disableLoadElimination();
    return Base::visitCallSite(CS);
  }

  // If we have a constant that we are calling as a function, we can peer
  // through it and see the function target. This happens not infrequently
  // during devirtualization and so we want to give it a hefty bonus for
  // inlining, but cap that bonus in the event that inlining wouldn't pan
  // out. Pretend to inline the function, with a custom threshold.
  auto IndirectCallParams = Params;
  IndirectCallParams.DefaultThreshold = InlineConstants::IndirectCallThreshold;
  CallAnalyzer CA(TTI, GetAssumptionCache, GetBFI, PSI, ORE, *F, CS,
                  ILIC, AI, CallSitesForFusion, IndirectCallParams); // INTEL
  if (CA.analyzeCall(CS, nullptr)) { // INTEL
    // We were able to inline the indirect call! Subtract the cost from the
    // threshold to get the bonus we want to apply, but don't go below zero.
    Cost -= std::max(0, CA.getThreshold() - CA.getCost());
  }

  if (!F->onlyReadsMemory())
    disableLoadElimination();
  return Base::visitCallSite(CS);
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
    if (!ComputeFullInlineCost) {
      Cost = CostLowerBound;
      return false;
    }
    if (EarlyExitCost == INT_MAX) {
      EarlyExitCost = Cost;
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

    Cost = std::min((int64_t)CostUpperBound, JTCost + Cost);
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
    Cost += NumCaseCluster * 2 * InlineConstants::InstrCost;
    return false;
  }

  int64_t ExpectedNumberOfCompare = 3 * (int64_t)NumCaseCluster / 2 - 1;
  int64_t SwitchCost =
      ExpectedNumberOfCompare * 2 * InlineConstants::InstrCost;

  Cost = std::min((int64_t)CostUpperBound, SwitchCost + Cost);
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
      Cost += InlineConstants::InstrCost;

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
                                          CandidateCS.getInstruction())
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
                                          CandidateCS.getInstruction())
                 << NV("Callee", &F) << " is " << NV("InlineResult", IR.message)
                 << ". Cost is not fully computed";
        });
      return IR;
    }

    // Check if we've past the maximum possible threshold so we don't spin in
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
static bool forgivableCondition(TerminatorInst* TI) {
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
static bool worthyDoubleCallSite1(CallSite &CS, InliningLoopInfoCache &ILIC) {
   Function *Caller = CS.getCaller();
   Function *Callee = CS.getCalledFunction();
   // Look for 2 calls of the callee in the caller.
   unsigned count = 0;
   for (Use &U : Callee->uses()) {
      if (auto CS = ImmutableCallSite(U.getUser())) {
        if (CS.getCaller() == Caller) {
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
      if (auto CS = ImmutableCallSite(U.getUser())) {
        if (!CS)
          return false;
        if (!CS.isCallee(&U))
          return false;
        auto I = CS.getInstruction();
        if (!isa<Constant>(I->getOperand(ArgNo)))
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
static bool worthyDoubleCallSite2(CallSite &CS, InliningLoopInfoCache& ILIC) {
  Function *Callee = CS.getCalledFunction();
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
static bool worthyDoubleCallSite3(CallSite &CS, InliningLoopInfoCache& ILIC) {
  if (!NewDoubleCallSiteInliningHeuristics)
    return false;
  Function *Caller = CS.getCaller();
  LoopInfo *CallerLI = ILIC.getLI(Caller);
  if (!CallerLI->getLoopFor(CS.getInstruction()->getParent()))
    return false;
  Function *Callee = CS.getCalledFunction();
  LoopInfo *CalleeLI = ILIC.getLI(Callee);
  if (CalleeLI->begin() == CalleeLI->end())
    return false;
  if (CS.arg_begin() != CS.arg_end()
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
    CallSite Site(U);
    if (!Site || Site.getCalledFunction() != F)
      continue;
    if (++count > 2)
      return false;
  }
  return count == 2;
}

//
// Return 'true' if 'CS' worth inlining, given that it is a double callsite
// with internal linkage.
//
static bool worthyDoubleInternalCallSite(CallSite &CS,
  InliningLoopInfoCache& ILIC)
{
  return worthyDoubleCallSite1(CS, ILIC) || worthyDoubleCallSite2(CS, ILIC)
    || worthyDoubleCallSite3(CS, ILIC);
}

//
// Return 'true' if 'CS' worth inlining, given that it is a double callsite
// with external linkage.
//
// The criteria for this heuristic are:
//   (1) Single basic block in the caller
//   (2) Single use which is not a direct invocation of the caller
//   (3) No calls to functions other than the called function
//       (except intrinsics added by using -g)
//
static bool worthyDoubleExternalCallSite(CallSite &CS) {
  Function *Caller = CS.getCaller();
  if (!NewDoubleCallSiteInliningHeuristics)
    return false;
  if (std::distance(Caller->begin(), Caller->end()) != 1)
    return false;
  Function *Callee = CS.getCalledFunction();
  unsigned int count = 0;
  for (User *U : Caller->users()) {
    CallSite Site(U);
    if (Site && Site.getCalledFunction() == Caller)
      return false;
    if (++count > 1)
      return false;
  }
  if (count != 1)
    return false;
  for (BasicBlock &BB : *Caller) {
    for (Instruction &I : BB) {
      CallSite CS(cast<Value>(&I));
      if (!CS)
        continue;
      Function *F = CS.getCalledFunction();
      if (F != nullptr && F->getName() == "llvm.dbg.value")
        continue;
      if (F != Callee)
        return false;
    }
  }
  return true;
}

static bool preferCloningToInlining(CallSite& CS,
                                    InliningLoopInfoCache& ILIC,
                                    bool PrepareForLTO) {
  // As a workaround, commenting out the below source line until changes
  // in BackendUtil.cpp, which sets PrepareForLTO flag, will be checked
  // into xmain branch. BackendUtil.cpp is not branch on ltoprof-xmain.
  // if (!PrepareForLTO) return false;
  Function *Callee = CS.getCalledFunction();
  if (!Callee) return false;
  LoopInfo *LI = ILIC.getLI(Callee);
  if (!LI) return false;
  if (llvm::llvm_cloning_analysis::isCallCandidateForSpecialization(CS, LI))
    return true;
  return false;
}

//
// Return 'true' if 'SI' is a store of a function pointer which appears in a
// series of 3 assignments to successive fields of a structure instance.
// Like:
//
//   typedef int (*FP)();
//
//   typedef struct {
//     FP field1;
//     FP field2;
//     FP field3;
//   } MYSTRUCT;
//   MYSTRUCT *ptr;
//
//   ptr->field1 = FunctionPtr1;
//   ptr->field2 = FunctionPtr2;
//   ptr->field3 = FunctionPtr3;
//
static bool inSpecialFxnPtrStructAssignCluster(StoreInst *SI,
                                               SmallPtrSetImpl<Function *>
                                                   &CalleeFxnPtrSet) {
  static SmallPtrSet<Function *, 3> FxnPtrSet;
    // Stores the function pointers in the series
  static SmallPtrSet<StoreInst *, 3> StoreInstSet;
    // Stores the StoreInsts series
  if (!StoreInstSet.empty()) {
    // If we have already computed the series of function pointers and
    // stores, just query whether 'SI' is one of them.
    return StoreInstSet.find(SI) != StoreInstSet.end();
  }
  // Scan for a series of GetElementPtrInsts followed by StoreInsts, like:
  //
  // %fp01 = getelementptr inbounds %MYSTRUCT, %MYSTRUCT* %0, i32 0, i32 0
  // store i32 ()* @fp1, i32 ()** %fp1, %fp01, align 8
  // %fp02 = getelementptr inbounds %MYSTRUCT, %MYSTRUCT* %0, i32 0, i32 1
  // store i32 ()* @fp2, i32 ()** %fp2, %fp02, align 8
  // %fp03 = getelementptr inbounds %MYSTRUCT, %MYSTRUCT* %0, i32 0, i32 2
  //  store i32 ()* @fp3, i32 ()** %fp3, %fp03, align 8
  BasicBlock *BB = SI->getParent();
  Value *GEPIPointerOperand = nullptr;
  GetElementPtrInst *NGI = nullptr;
  bool FoundStoreMatch = false;
  int ExpectedCount = 0;
  for (auto &I : *BB) {
    // {NGI, NSI} will be the current GetElementPtrInst and StoreInst pair
    if (!NGI || !NGI->hasAllConstantIndices()) {
      NGI = dyn_cast<GetElementPtrInst>(&I);
      continue;
    }
    auto NSI = dyn_cast<StoreInst>(&I);
    if (!NSI)
      break;
    if (NSI == SI)
      FoundStoreMatch = true;
    // Ensure that NGI feeds NSI
    if (GEPIPointerOperand == nullptr)
      GEPIPointerOperand = NGI->getPointerOperand();
    // Ensure that NGI has the form:
    //   getelementptr inbounds %MYSTRUCT, %MYSTRUCT* %0, i32 0, i32 X
    // where X goes from 0 to 1 to 2
    else if (GEPIPointerOperand != NGI->getPointerOperand())
      break;
    if (!cast<ConstantInt>(NGI->getOperand(1))->isZeroValue())
      break;
    if (cast<ConstantInt>(NGI->getOperand(2))->getSExtValue() != ExpectedCount)
      break;
    if (NSI->getPointerOperand() != NGI)
      break;
    auto F = dyn_cast<Function>(NSI->getValueOperand());
    if (!F)
      break;
    // Save the candidate pair
    StoreInstSet.insert(NSI);
    FxnPtrSet.insert(F);
    ++ExpectedCount;
    if (FoundStoreMatch && (ExpectedCount == 3)) {
      // Make each of the Functions found in the series preferred for
      // multiversioning
      for (auto Fxn : FxnPtrSet)
        CalleeFxnPtrSet.insert(Fxn);
      return true;
    }
    NGI = nullptr;
  }
  // We didn't find a full series, so clear out what we have found so far.
  StoreInstSet.clear();
  FxnPtrSet.clear();
  return false;
}

//
// Return 'true' if 'F' is called by exactly two Functions
//
static bool isCalledByTwoFxns(Function *F)
{
  SmallPtrSet<Function *, 2> CallerSet;
  for (User *U : F->users()) {
    CallSite Site(U);
    if (!Site || Site.getCalledFunction() != F)
      continue;
    if (CallerSet.insert(Site.getCaller()).second && CallerSet.size() > 2)
      return false;
  }
  return CallerSet.size() == 2;
}

//
// Return 'true' if 'CS' should not be inlined, because it would be better
// to multiversion it. 'PrepareForLTO' is true if we are on the compile step
// of an LTO compilation.
//
static bool preferMultiversioningToInlining(CallSite& CS,
                                            InliningLoopInfoCache& ILIC,
                                            bool PrepareForLTO) {

  // Right now, only the callee is tested for multiversioning.  We use
  // this set to keep track of callees that have already been tested,
  // to save compile time. We expect it to be relatively expensive to
  // test the qualifying candidates. Those that do not qualify should
  // be rejected quickly.
  static SmallPtrSet<Function *, 3> CalleeFxnPtrSet;

  // Functions that were not called by exactly two functions when we first
  // tested them. Inlining can change the number of functions which call
  // a function, as can indirect call specialization.
  static SmallPtrSet<Function *, 10> NotCalledByTwoFxnPtrSet;

  if (!DTransInlineHeuristics)
    return false;
  Function* Callee = CS.getCalledFunction();
  if (!Callee)
    return false;
  if (CalleeFxnPtrSet.find(Callee) != CalleeFxnPtrSet.end())
    return true;
  // Must be called by exactly two Functions.
  if (NotCalledByTwoFxnPtrSet.find(Callee) != NotCalledByTwoFxnPtrSet.end())
    return false;
  if (!isCalledByTwoFxns(Callee)) {
    NotCalledByTwoFxnPtrSet.insert(Callee);
    return false;
  }
  // Only consider candidates that have loops, otherwise there is nothing
  // to multiversion.
  LoopInfo *CalleeLI = ILIC.getLI(Callee);
  if (!CalleeLI || CalleeLI->empty())
    return false;
  // Special criteria that narrow the candidate list.
  bool FoundNonCallUser = false;
  for (User *U : Callee->users()) {
    CallSite Site(U);
    if (!Site || Site.getCalledFunction() != Callee)
      continue;
    Function *Caller = Site.getCaller();
    for (User *UU : Caller->users()) {
      // After LTO pass indirect call resolution, some candidates may
      // appear as direct calls, and we don't want to penalize them for that.
      if (!PrepareForLTO && (isa<CallInst>(UU) || isa<InvokeInst>(UU)))
        continue;
      auto SI = dyn_cast<StoreInst>(UU);
      if (!SI)
        return false;
      // Look for a list of function pointers assigned to successive
      // fields in the same structure instance.
      if (!inSpecialFxnPtrStructAssignCluster(SI, CalleeFxnPtrSet))
        return false;
      FoundNonCallUser = true;
    }
  }
  if (!FoundNonCallUser)
    return false;
  CalleeFxnPtrSet.insert(Callee);
  return true;
}

//
// Return 'true' if 'CS' is worth inlining due to multiple arguments being
// pointers dereferenced in callee code.
//
// The criteria for this heuristic are:
//   (1) Caller and callee have loops.
//   (2) Callsite is in loop nest of depth InliningForACLoopDepth (default 2),
//       and it is the only callsite in the loop.
//   (3) Callee's arguments are pointers dereferenced in the code multiple
//       times.
//
static bool worthInliningForAddressComputations(CallSite &CS,
                                                InliningLoopInfoCache &ILIC,
                                                bool PrepareForLTO) {
  // Heuristic is enabled if option is unset and it is first inliner run
  // (on PrepareForLTO phase) OR if option is set to true.
  if (((InliningForAddressComputations != cl::BOU_UNSET) || !PrepareForLTO) &&
      (InliningForAddressComputations != cl::BOU_TRUE))
    return false;

  Function *Callee = CS.getCalledFunction();
  Function *Caller = CS.getCaller();

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

  Loop *InnerLoop = CallerLI->getLoopFor(CS.getInstruction()->getParent());
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
  if (LimTC < 3) {
    // small TC - return false
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

// Minimal number of successive callsites need to be inlined to benefit
// from loops fusion.
static cl::opt<int>
    MinCallSitesForFusion("inline-for-fusion-min-callsites", cl::Hidden,
                          cl::init(2),
                          cl::desc("Min number of calls inlined for fusion"));

// Maximal number of successive callsites need to be inlined to benefit
// from loops fusion.
static cl::opt<int>
    MaxCallSitesForFusion("inline-for-fusion-max-callsites", cl::Hidden,
                          cl::init(20),
                          cl::desc("Max number of calls inlined for fusion"));

// Temporary switch to control new callsite inlining heuristics
// for fusion until tuning of loopopt is complete.
static cl::opt<bool>
    InliningForFusionHeuristics("inlining-for-fusion-heuristics",
                                cl::init(false), cl::ReallyHidden);

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
/// In case we decide that current CS is a candidate for inlining for fusion,
/// then we store other CS to the same function in the same basic block in
/// the set of inlining candidates for fusion.
static bool worthInliningForFusion(CallSite &CS, InliningLoopInfoCache &ILIC,
                                   SmallSet<CallSite, 20> *CallSitesForFusion) {
  if (!InliningForFusionHeuristics) {
    return false;
  }

  if (!CallSitesForFusion) {
    LLVM_DEBUG(llvm::dbgs()
               << "IC: No inlining for fusion: no call site candidates.\n");
    return false;
  }

  // The call site was stored as candidate for inlining for fusion.
  if (CallSitesForFusion->count(CS)) {
    CallSitesForFusion->erase(CS);
    return true;
  }

  Function *Callee = CS.getCalledFunction();
  BasicBlock *CSBB = CS->getParent();

  SmallVector<CallSite, 20> LocalCSForFusion;
  // Go through each successive call in basic block and find all callsites.
  int CSCount = 1;
  bool CallSiteCountFlag = false;
  for (auto I = CSBB->begin(), E = CSBB->end(); I != E; ++I) {
    CallSite LocalCS(cast<Value>(I));
    if (!LocalCS) {
      continue;
    }
    if (LocalCS == CS) {
      CallSiteCountFlag = true;
      continue;
    }
    Function *CurrCallee = LocalCS.getCalledFunction();
    if (CallSiteCountFlag) {
      if (CurrCallee == Callee) {
        CSCount++;
        LocalCSForFusion.push_back(LocalCS);
      } else {
        CallSiteCountFlag = false;
      }
    }
  }

  // If number of successive calls is relatively small or too big
  // then skip inlining
  if ((CSCount < MinCallSitesForFusion) || (CSCount > MaxCallSitesForFusion)) {
    LLVM_DEBUG(
        llvm::dbgs()
        << "IC: No inlining for fusion: number of candidates is out of range:"
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
        if (ArgCnt > MinArgRefs) {
          break;
        }
        if (GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(&I)) {
          Value *PtrOp = GEPI->getPointerOperand();
          if (PtrOp) {
            if (isa<PHINode>(PtrOp)) {
              PtrOp = dyn_cast<PHINode>(PtrOp)->getIncomingValue(0);
            }
            if (isa<Argument>(PtrOp))
              ArgCnt++;
          }
        }
      }
      if (ArgCnt > MinArgRefs) {
        break;
      }
    }
    if (ArgCnt > MinArgRefs) {
      break;
    }
  }

  // Not enough arguments-arrays were found in loop.
  if (ArgCnt < MinArgRefs) {
    LLVM_DEBUG(llvm::dbgs()
               << "IC: No inlining for fusion: not enough array refs.\n");
    return false;
  }

  // Store other inlining candidates in a special map.
  if (CallSitesForFusion) {
    for (auto LocalCS : LocalCSForFusion) {
      CallSitesForFusion->insert(LocalCS);
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
    if (TerminatorInst *TermInst = BB->getTerminator()) {
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
static bool worthInliningForDeeplyNestedIfs(CallSite &CS,
                                            InliningLoopInfoCache &ILIC,
                                            bool IsCallerRecursive,
                                            bool PrepareForLTO) {
  // Heuristic is enabled if option is unset and it is first inliner run
  // (on PrepareForLTO phase) OR if option is set to true.
  if (((InliningForDeeplyNestedIfs != cl::BOU_UNSET) || !PrepareForLTO) &&
      (InliningForDeeplyNestedIfs != cl::BOU_TRUE))
    return false;

  Function *Callee = CS.getCalledFunction();
  Function *Caller = CS.getCaller();

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
    // A CFG edge is dead if the predecessor is dead or the predessor has a
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

InlineResult CallAnalyzer::analyzeCall(CallSite CS,            // INTEL
                                       InlineReason* Reason) { // INTEL
  ++NumCallsAnalyzed;
  InlineReason TempReason = NinlrNoReason; // INTEL
  InlineReason* ReasonAddr = Reason == nullptr ? &TempReason : Reason; // INTEL
  InlineReasonVector YesReasonVector; // INTEL
  InlineReasonVector NoReasonVector; // INTEL
  TempReason = NinlrNoReason; // INTEL
  EarlyExitCost = INT_MAX; // INTEL
  EarlyExitThreshold = INT_MAX; // INTEL

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
  updateThreshold(CS, F, YesReasonVector); // INTEL

  // INTEL  CQ378383: Tolerate a single "forgivable" condition when optimizing
  // INTEL  for size. In this case, we delay subtracting out the single basic
  // INTEL  block bonus until we see a second branch with multiple targets.
  bool SeekingForgivable = CS.getCaller()->optForSize(); // INTEL
  bool FoundForgivable = false;                          // INTEL
  bool SubtractedBonus = false;                          // INTEL
  bool PrepareForLTO = Params.PrepareForLTO.getValueOr(false); // INTEL

  // Speculatively apply all possible bonuses to Threshold. If cost exceeds
  // this Threshold any time, and cost cannot decrease, we can stop processing
  // the rest of the function body.
  Threshold += (SingleBBBonus + VectorBonus);
#if INTEL_CUSTOMIZATION
  if (InlineForXmain &&
      preferCloningToInlining(CS, *ILIC, PrepareForLTO)) {
    *ReasonAddr = NinlrPreferCloning;
    return "prefer cloning";
  }
  if (InlineForXmain &&
      preferMultiversioningToInlining(CS, *ILIC, PrepareForLTO)) {
    *ReasonAddr = NinlrPreferMultiversioning;
    return false;
  }
#endif // INTEL_CUSTOMIZATION

  // Give out bonuses for the callsite, as the instructions setting them up
  // will be gone after inlining.
  Cost -= getCallsiteCost(CS, DL);

#if INTEL_CUSTOMIZATION
  Function *Caller = CS.getInstruction()->getFunction();
  // Check if the caller function is recursive itself.
  for (User *U : Caller->users()) {
    CallSite Site(U);
    if (!Site)
      continue;
    Instruction *I = Site.getInstruction();
    if (I->getFunction() == Caller) {
      IsCallerRecursive = true;
      break;
    }
  }

  if (InlineForXmain) {
    if (&F == CS.getCalledFunction()) {
      if (isDoubleCallSite(&F)) {
        // If there are two calls of the function, the cost of inlining it may
        // drop, but less dramatically.
        if (F.hasLocalLinkage() || F.hasLinkOnceODRLinkage()) {
           if (worthyDoubleInternalCallSite(CS, *ILIC)) {
             Cost -= InlineConstants::SecondToLastCallToStaticBonus;
             YesReasonVector.push_back(InlrDoubleLocalCall);
           }
        }
        else {
           if (worthyDoubleExternalCallSite(CS)) {
             Cost -= InlineConstants::SecondToLastCallToStaticBonus;
             YesReasonVector.push_back(InlrDoubleNonLocalCall);
           }
        }
      }
      if (worthInliningForFusion(CS, *ILIC, CallSitesForFusion)) {
        Cost -= InlineConstants::InliningHeuristicBonus;
        YesReasonVector.push_back(InlrForFusion);
      }
      if (worthInliningForDeeplyNestedIfs(CS, *ILIC, IsCallerRecursive,
                                          PrepareForLTO)) {
        Cost -= InlineConstants::InliningHeuristicBonus;
        YesReasonVector.push_back(InlrDeeplyNestedIfs);
      }
      if (worthInliningForAddressComputations(CS, *ILIC, PrepareForLTO)) {
        Cost -= InlineConstants::InliningHeuristicBonus;
        YesReasonVector.push_back(InlrAddressComputations);
      }
    }
  }

  // Use InlineAggressiveInfo to expose uses of global ptrs
  if (InlineForXmain && AI != nullptr && AI->isCallInstInAggInlList(CS)) {
    Cost -= InlineConstants::AggressiveInlineCallBonus;
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
  CallSite::arg_iterator CAI = CS.arg_begin();
  for (Function::arg_iterator FAI = F.arg_begin(), FAE = F.arg_end();
       FAI != FAE; ++FAI, ++CAI) {
    assert(CAI != CS.arg_end());
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

    // Disallow inlining a blockaddress. A blockaddress only has defined
    // behavior for an indirect branch in the same function, and we do not
    // currently support inlining indirect branches. But, the inliner may not
    // see an indirect branch that ends up being dead code at a particular call
    // site. If the blockaddress escapes the function, e.g., via a global
    // variable, inlining may lead to an invalid cross-function reference.
    if (BB->hasAddressTaken()) { // INTEL
      *ReasonAddr = NinlrBlockAddress; // INTEL
      return "blockaddress";
    }

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

    TerminatorInst *TI = BB->getTerminator();

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
          Cost -= InlineConstants::InstrCost;
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
    YesReasonVector.push_back(InlrAlmostSingleBasicBlock);
#endif // INTEL_CUSTOMIZATION

  bool OnlyOneCallAndLocalLinkage =
       (F.hasLocalLinkage()                                     // INTEL
         || (InlineForXmain && F.hasLinkOnceODRLinkage())) &&   // INTEL
       F.hasOneUse() &&  &F == CS.getCalledFunction();          // INTEL
  // If this is a noduplicate call, we can still inline as long as
  // inlining this would cause the removal of the caller (so the instruction
  // is not actually duplicated, just moved).
  if (!OnlyOneCallAndLocalLinkage && ContainsNoDuplicateCall) { // INTEL
    *ReasonAddr = NinlrDuplicateCall; // INTEL
    return "noduplicate";
  } // INTEL

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

int llvm::getCallsiteCost(CallSite CS, const DataLayout &DL) {
  int Cost = 0;
  for (unsigned I = 0, E = CS.arg_size(); I != E; ++I) {
    if (CS.isByValArgument(I)) {
      // We approximate the number of loads and stores needed by dividing the
      // size of the byval type by the target's pointer size.
      PointerType *PTy = cast<PointerType>(CS.getArgument(I)->getType());
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
    CallSite CS, const InlineParams &Params, TargetTransformInfo &CalleeTTI,
    std::function<AssumptionCache &(Function &)> &GetAssumptionCache,
    Optional<function_ref<BlockFrequencyInfo &(Function &)>> GetBFI,
    InliningLoopInfoCache *ILIC, // INTEL
    InlineAggressiveInfo *AI,    // INTEL
    SmallSet<CallSite, 20> *CallSitesForFusion, // INTEL
    ProfileSummaryInfo *PSI, OptimizationRemarkEmitter *ORE) {
  return getInlineCost(CS, CS.getCalledFunction(), Params, CalleeTTI,
                       GetAssumptionCache, GetBFI, ILIC, AI, // INTEL
                       CallSitesForFusion, PSI, ORE);// INTEL
}

InlineCost llvm::getInlineCost(
    CallSite CS, Function *Callee, const InlineParams &Params,
    TargetTransformInfo &CalleeTTI,
    std::function<AssumptionCache &(Function &)> &GetAssumptionCache,
    Optional<function_ref<BlockFrequencyInfo &(Function &)>> GetBFI,
    InliningLoopInfoCache *ILIC,    // INTEL
    InlineAggressiveInfo *AI,       // INTEL
    SmallSet<CallSite, 20> *CallSitesForFusion, // INTEL
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
  for (unsigned I = 0, E = CS.arg_size(); I != E; ++I)
    if (CS.isByValArgument(I)) {
      PointerType *PTy = cast<PointerType>(CS.getArgument(I)->getType());
      if (PTy->getAddressSpace() != AllocaAS)
        return llvm::InlineCost::getNever("byval arguments without alloca"
                                          " address space");
    }

  // Calls to functions with always-inline attributes should be inlined
  // whenever possible.

  if (CS.hasFnAttr(Attribute::AlwaysInline)) {
#if INTEL_CUSTOMIZATION
    InlineReason Reason = InlrNoReason;
    if (isInlineViable(*Callee, Reason))
      return llvm::InlineCost::getAlways("always inline attribute",
                                         InlrAlwaysInline);
    assert(IsNotInlinedReason(Reason));
    return llvm::InlineCost::getNever("inapplicable always inline attribute",
                                      Reason);
  }
  if (CS.hasFnAttr(Attribute::AlwaysInlineRecursive)) {
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
  Function *Caller = CS.getCaller();
  if (!functionsHaveCompatibleAttributes(Caller, Callee, CalleeTTI))
    return llvm::InlineCost::getNever("conflicting attributes",   // INTEL
                                      NinlrMismatchedAttributes); // INTEL

  // Don't inline this call if the caller has the optnone attribute.
  if (Caller->hasFnAttribute(Attribute::OptimizeNone))
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
      CS.isNoInline()) { // INTEL
#if INTEL_CUSTOMIZATION
    if (Callee->isInterposable()) {
      return llvm::InlineCost::getNever("interposable", NinlrMayBeOverriden);
    }
    if (Callee->hasFnAttribute(Attribute::NoInline)) {
      return llvm::InlineCost::getNever("noinline function attribute",
                                        NinlrNoinlineAttribute);
    }
    if (CS.isNoInline()) {
      return llvm::InlineCost::getNever("noinline call site attribute",
                                        NinlrNoinlineCallsite);
    }
#endif // INTEL_CUSTOMIZATION
  } // INTEL

  LLVM_DEBUG(llvm::dbgs() << "      Analyzing call of " << Callee->getName()
                          << "... (caller:" << Caller->getName() << ")\n");

  CallAnalyzer CA(CalleeTTI, GetAssumptionCache, GetBFI, PSI, ORE, *Callee, CS,
                  ILIC, AI, CallSitesForFusion, Params);  // INTEL
#if INTEL_CUSTOMIZATION
  InlineReason Reason = InlrNoReason;
  InlineResult ShouldInline = CA.analyzeCall(CS, &Reason);
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

bool llvm::isInlineViable(Function &F, // INTEL
                          InlineReason& Reason) { // INTEL
  bool ReturnsTwice = F.hasFnAttribute(Attribute::ReturnsTwice);
  for (Function::iterator BI = F.begin(), BE = F.end(); BI != BE; ++BI) {
    // Disallow inlining of functions which contain indirect branches or
    // blockaddresses.
    if (isa<IndirectBrInst>(BI->getTerminator())
      || BI->hasAddressTaken()) { // INTEL
#if INTEL_CUSTOMIZATION
      if (isa<IndirectBrInst>(BI->getTerminator())) {
        Reason = NinlrIndirectBranch;
        return false;
      }
      if (BI->hasAddressTaken()) {
        Reason = NinlrBlockAddress;
        return false;
      }
#endif // INTEL_CUSTOMIZATION
    } // INTEL

    for (auto &II : *BI) {
      CallSite CS(&II);
      if (!CS)
        continue;

      // Disallow recursive calls.
      if (&F == CS.getCalledFunction()) { // INTEL
        Reason = NinlrRecursive; // INTEL
        return false;
      } // INTEL

      // Disallow calls which expose returns-twice to a function not previously
      // attributed as such.
      if (!ReturnsTwice && CS.isCall() &&
          cast<CallInst>(CS.getInstruction())->canReturnTwice()) { // INTEL
        Reason = NinlrReturnsTwice; // INTEL
        return false;
      } // INTEL

      if (CS.getCalledFunction())
        switch (CS.getCalledFunction()->getIntrinsicID()) {
        default:
          break;
        // Disallow inlining of @llvm.icall.branch.funnel because current
        // backend can't separate call targets from call arguments.
        case llvm::Intrinsic::icall_branch_funnel:
        // Disallow inlining functions that call @llvm.localescape. Doing this
        // correctly would require major changes to the inliner.
        case llvm::Intrinsic::localescape:
          Reason = NinlrCallsLocalEscape; // INTEL
          return false; // INTEL
        // Disallow inlining of functions that initialize VarArgs with va_start.
        case llvm::Intrinsic::vastart:
          Reason = NinlrVarargs; // INTEL
          return false;
        }
    }
  }

  return true;
}

// APIs to create InlineParams based on command line flags and/or other
// parameters.

InlineParams llvm::getInlineParams(int Threshold) {
  InlineParams Params;
  Params.PrepareForLTO = false;     // INTEL

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
