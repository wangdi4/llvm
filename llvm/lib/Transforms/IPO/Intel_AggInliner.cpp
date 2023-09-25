//===--------------- Intel_AggInliner.cpp --------------------------------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This implements a pass that creates a InlineAggressiveInfo object to find
// callsites that should be marked with the 'prefer-inline-aggressive'
// attribute. That attribute is then used in the inliner to inline those
// callsites.
//
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/IPO/Intel_AggInliner.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/IPO.h"

using namespace llvm;

#define DEBUG_TYPE "agginliner"

// Maximum number of callsites marked by this analysis. If it exceeds
// this threshold, this analysis is disabled.
//
static cl::opt<unsigned> InlineAggressiveCSLimit("inline-agg-callsites-limit",
                                                 cl::init(25),
                                                 cl::ReallyHidden);

// Minimum malloc size limit for Aggressive Inline Analysis. A  malloc
// call that allocates more than this limit is considered for this
// analysis.
//
static cl::opt<unsigned> InlineAggressiveMallocLimit("inline-agg-malloc-limit",
                                                     cl::init(0x6000000),
                                                     cl::ReallyHidden);

// If number of instructions in entire application is greater than
// this limit, it will not be considered for aggressive inline analysis.
//
static cl::opt<uint64_t> InlineAggressiveInstLimit("inline-agg-inst-limit",
                                                   cl::init(0x3000),
                                                   cl::ReallyHidden);

// Minimum number of calls for a consecutive call chain to be considered a long
// consecutive call chain.
static cl::opt<unsigned> InlineAggressiveLongConsecutiveCallChainSize(
    "inline-agg-long-consecutive-call-chain-size", cl::init(6),
    cl::ReallyHidden);

// Force inlining of all calls to functions found in long consecutive call
// chains, without profitability analysis.
static cl::opt<bool>
    InlineAggressiveLongConsecutiveCallChainIgnoreProfitability(
        "inline-agg-long-consecutive-call-chain-ignore-profitability",
        cl::init(false), cl::ReallyHidden);

// Bail out if trip count comparison can't complete in the specified number of
// iterations during profitability analysis of long consecutive call chain
// inlining.
static cl::opt<unsigned>
    InlineAggressiveLongConsecutiveCallChainTripCountEqualIterations(
        "inline-agg-long-consecutive-call-chain-trip-count-equal-iterations",
        cl::init(16), cl::ReallyHidden);

// Maximum number of Global variable's uses allowed.
static const unsigned MaxNumGVUses = 16;

// Maximum number of Calls allowed to inline for any Global Variable
static const unsigned MaxNumInlineCalls = 16;

// Maximum number of instructions visited to track uses of any Global Variable
static const unsigned MaxNumInstructionsVisited = 32;

// Function is not allowed to inline if size of the function exceeds this
// limit.
static const unsigned MaxNumBBInlineLimit = 32;

// Function is not considered as tiny function if size of the function exceeds
// this limit.
static const unsigned MaxNumBBTinyFuncLimit = 1;

// This class implements the long-consecutive-call-chain heuristic.
// It tries to find a function that is called consecutively for many times, and
// find all call sites of all functions that appear inside the same call
// chain to find inlining opportunities that is likely to facilitate potential
// future optimization, e.g. loop fusion.
//
// For example, if code like below appears inside the source:
//
//   ... // Not adjacent to another function call
//   func_a(data);
//   func_b(data);
//   func_c(data);
//   func_c(data);
//   func_c(data);
//   func_c(data);
//   func_d(data);
//   ... // Not adjacent to another function call
//
// And our threshold for "long consecutive call chain" is less than 5, every
// call to func_a(), func_b(), func_c() and func_d() will be checked for
// profitability and, if passed, be inlined.
//
// In detail:
// Not all functions are eligible to be inside a long consecutive call chain. We
// only handle functions whose performance is likely to be optimized in later
// passes. Such function is called a "candidate leaf function".
// A "call chain" is a series of call instructions residing in the same basic
// block, inside which each call must be a direct call to a candidate leaf
// function.
// If all calls inside a call chain has the same callee, the chain is called a
// "consecutive call chain" (abbreviated "CCChain" in the source). A consecutive
// call chain with length above a certain threshold is called a "long
// consecutive call chain".
// We find all chains containing a long consecutive call chain, and check
// whether it's profitable to inline these calls. If so, All callees of these
// call chains are promoted from "candidate leaf function" to "leaf function"
// (related to a long consecutive call chain). Bail out if any chain is not
// profitable.
// These leaf functions may also be called in a chained fashion elsewhere, these
// chains don't satisfy the conditions of a long consecutive call chain, but
// can usually be optimized in a similar manner. We also find these chains and
// check them for profitability. In this step, non-profitable calls are allowed,
// but they'll not be inlined.
class LongConsecutiveCallChainHeuristic {
  // A call chain is identified by two instructions: the Start and the End. It's
  // an [closed, open) interval, the End is not included in the chain.
  // Every instruction in a call chain must be a direct call to a candidate
  // leaf function. Since a call instruction is not a terminator, End is
  // guaranteed to be non-null.
  // A Length is also stored for convenience.
  struct CallChain {
    CallInst *Start;
    Instruction *End;
    unsigned Length;
    const Function *getCaller() const { return Start->getCaller(); }
    CallChain(CallInst *Start, Instruction *End, unsigned Length)
        : Start(Start), End(End), Length(Length) {
      assert(Start->getParent() == End->getParent() &&
             "A call chain must be contained in one basic block");
      assert(Length >= 1 && "Empty call chain is not allowed");
    }
    bool operator<(const CallChain &Other) const {
      assert(!(Start == Other.Start ^ End == Other.End) &&
             "A call instruction may be covered by at most one call chain");
      return Start < Other.Start;
    }
  };

  // Determining whether a function is a valid candidate for a call chain
  // requires some analysis, we cache the result with the Loop required by its
  // definition. The function can't be part of a call chain if the loop is
  // nullptr.
  std::map<Function *, Loop *> LeafFunctionCandidacyCache;
  // All call chains found in the module
  std::set<CallChain> CallChains;
  // Map from a call to the call chain it belongs to. It's guaranteed that every
  // call chain in CallChains has all of its call instructions present in the
  // map.
  std::map<CallInst *, const CallChain *> CallToChainMap;
  Module &M;
  std::function<void(CallBase &)> SetInline;
  std::function<void(CallBase &)> SetNoInline;
  AggInlGetLITy GetLI;
  AggInlGetSETy GetSE;

  // Determine whether a function is a valid leaf function candidate.
  bool isLeafFunctionCandidate(Function *F);
  // Analyze the module, find all long consecutive call chains, store them in a
  // set and return.
  DenseSet<const CallChain *> findChainsWithLongConsecutiveCallChain();
  // Given two calls, determine whether inlining them is profitable. CallA and
  // CallB must be two adjacent calls in the same call chain.
  bool isProfitableToInlineCallPair(CallInst *CallA, CallInst *CallB);
  // Add a custom "lccc-call-in-leaf" attribute to every direct call in leaf
  // functions that is not in a loop.
  // If there are function calls outside of the loop we're targeting in the leaf
  // function, it's likely they'll interfere with optimizations like loop
  // fusion. The attribute can tell later passes (e.g. partial inlining) to
  // optimize these calls more aggressively and hopefully eliminate them.
  // Note we put the attribute on the instructions instead of the leaf function
  // itself, or the callers of inlined call chains. Though the latter approach
  // is simpler, the attribute would be lost when the function carrying it is
  // inlined.
  void markCallsInLeafFunctions(const DenseSet<Function *> &LeafFunctions);

public:
  // This class uses SetInline and SetNoInline callback to mark call sites to be
  // or not to be inlined.
  LongConsecutiveCallChainHeuristic(Module &M,
                                    std::function<void(CallBase &)> SetInline,
                                    std::function<void(CallBase &)> SetNoInline,
                                    AggInlGetLITy GetLI, AggInlGetSETy GetSE)
      : M(M), SetInline(SetInline), SetNoInline(SetNoInline), GetLI(GetLI),
        GetSE(GetSE) {}

  bool run();
};

// A SimpleValueMap is just a map from Value to Value. It's used to help doing
// argument substitution for analysis of call sites. Because the IR is not
// changed during analysis there is no need to use a ValueMap.
using SimpleValueMap = DenseMap<Value *, Value *>;

// Return the mapped value of V in Map if exists, or V itself if not.
static const Value *tryMapValue(const Value *V, const SimpleValueMap &Map) {
  auto It = Map.find(V);
  if (It == Map.end())
    return V;
  return It->second;
}

// Given ArgMapLHS and ArgMapRHS constructed from two function call sites in
// Caller function, and two values LHS and RHS extracted from their callees, try
// to determine whether they're possibly equal by traversing the instructions
// producing LHS and RHS recursively and comparing them.
// The length of traversal is limited by the Iterations variable.
// Currently it covers constants, arguments, GEPs, and loads. Potential mutation
// of memory between loads is not taken into consideration.
static bool checkValueStructuralEqualityAcrossCall(
    const Value *LHS, const Value *RHS, const SimpleValueMap &ArgMapLHS,
    const SimpleValueMap &ArgMapRHS, unsigned &Iterations, Function *Caller) {
  if (Iterations == 0)
    return false;
  Iterations -= 1;

  if (LHS->getType() != RHS->getType())
    return false;

  LHS = tryMapValue(LHS, ArgMapLHS);
  RHS = tryMapValue(RHS, ArgMapRHS);

  auto LI = dyn_cast<Instruction>(LHS);
  auto RI = dyn_cast<Instruction>(RHS);

  if (LHS == RHS) {
    if (isa<Constant>(LHS))
      return true;
    if (LI && LI->getFunction() == Caller)
      return true;
    if (auto LArg = dyn_cast<Argument>(LHS))
      if (LArg->getParent() == Caller)
        return true;
  }

  if (!LI || !RI || LI->getOpcode() != RI->getOpcode())
    return false;

  if (auto LGEP = dyn_cast<GetElementPtrInst>(LI)) {
    auto RGEP = cast<GetElementPtrInst>(RI);
    if (LGEP->getNumIndices() != RGEP->getNumIndices() ||
        LGEP->isInBounds() != RGEP->isInBounds())
      return false;
    for (const auto &It : llvm::zip_equal(LGEP->indices(), RGEP->indices()))
      if (!checkValueStructuralEqualityAcrossCall(
              std::get<0>(It), std::get<1>(It), ArgMapLHS, ArgMapRHS,
              Iterations, Caller))
        return false;
    return checkValueStructuralEqualityAcrossCall(
        LGEP->getPointerOperand(), RGEP->getPointerOperand(), ArgMapLHS,
        ArgMapRHS, Iterations, Caller);
  }

  if (auto LLoad = dyn_cast<LoadInst>(LI))
    return checkValueStructuralEqualityAcrossCall(
        LLoad->getPointerOperand(), cast<LoadInst>(RI)->getPointerOperand(),
        ArgMapLHS, ArgMapRHS, Iterations, Caller);

  return false;
}

// Given ArgMapLHS and ArgMapRHS constructed from two function call sites in
// Caller function, and two SCEVs LHS and RHS constructed from their callees,
// try to determine whether they're possibly equal by comparing the expression
// structure of the two SCEVs.
// The length of the process is limited by the Iterations variable.
// Currently it covers SCEVConstant, SCEVCast, and SCEVCommutativeExpr, and uses
// checkValueStructuralEqualityAcrossCall for SCEVUnknown.
// Note it's only intended for simple cases of comparing trip counts. Against
// SCEVCommutativeExpr we just check for the equality of nth operand of LHS with
// the nth operand of RHS. We don't really check commutatively for simplicity.
static bool checkSCEVStructuralEqualityAcrossCall(
    const SCEV *LHS, const SCEV *RHS, const SimpleValueMap &ArgMapLHS,
    const SimpleValueMap &ArgMapRHS, unsigned &Iterations, Function *Caller) {
  if (Iterations == 0)
    return false;
  Iterations -= 1;

  if (isa<SCEVCouldNotCompute>(LHS) || isa<SCEVCouldNotCompute>(RHS))
    return false;

  if (LHS->getSCEVType() != RHS->getSCEVType() ||
      LHS->getType() != RHS->getType())
    return false;

  if (auto LConstant = dyn_cast<SCEVConstant>(LHS))
    return LConstant->getAPInt() == cast<SCEVConstant>(RHS)->getAPInt();

  if (auto LUnknown = dyn_cast<SCEVUnknown>(LHS))
    return checkValueStructuralEqualityAcrossCall(
        LUnknown->getValue(), cast<SCEVUnknown>(RHS)->getValue(), ArgMapLHS,
        ArgMapRHS, Iterations, Caller);

  if (auto LCast = dyn_cast<SCEVCastExpr>(LHS))
    return checkSCEVStructuralEqualityAcrossCall(
        LCast->getOperand(), cast<SCEVCastExpr>(RHS)->getOperand(), ArgMapLHS,
        ArgMapRHS, Iterations, Caller);

  if (auto LComm = dyn_cast<SCEVCommutativeExpr>(LHS)) {
    auto RComm = cast<SCEVCommutativeExpr>(RHS);
    if (LComm->getNumOperands() != RComm->getNumOperands() ||
        LComm->getNoWrapFlags() != RComm->getNoWrapFlags())
      return false;
    for (const auto &It : llvm::zip_equal(LComm->operands(), RComm->operands()))
      if (!checkSCEVStructuralEqualityAcrossCall(std::get<0>(It),
                                                 std::get<1>(It), ArgMapLHS,
                                                 ArgMapRHS, Iterations, Caller))
        return false;
    return true;
  }

  return false;
}

// Construct a SimpleValueMap containing the mapping of formal parameter to
// actual parameter for a function call.
static void populateArgumentMapping(SimpleValueMap &Out, CallInst *CI) {
  Function *Fn = CI->getCalledFunction();
  assert(!Fn->isVarArg() && "Unexpected callee with varargs");
  assert(Fn->arg_size() == CI->arg_size() &&
         "A call instruction should have the same number of arguments as its "
         "callee");
  for (const auto &It : llvm::zip_equal(Fn->args(), CI->args()))
    Out[&std::get<0>(It)] = std::get<1>(It);
}

bool LongConsecutiveCallChainHeuristic::isProfitableToInlineCallPair(
    CallInst *CallA, CallInst *CallB) {
  if (InlineAggressiveLongConsecutiveCallChainIgnoreProfitability)
    return true;

  // By definition, both calls must call a leaf function, every leaf function
  // must have one and only one top-level loop. Get the trip counts of the
  // loops, represented in SCEV, and use checkSCEVStructuralEqualityAcrossCall
  // to check whether they're equal.
  assert(CallA->getNextNonDebugInstruction() == CallB &&
         "Calls in a call chain should be adjacent to each other");
  Function *ParentFn = CallA->getCaller();
  Function *FnA = CallA->getCalledFunction();
  Loop *LoopA = LeafFunctionCandidacyCache[FnA];
  Function *FnB = CallB->getCalledFunction();
  Loop *LoopB = LeafFunctionCandidacyCache[FnB];
  assert(LoopA && LoopB && "Functions in a call chain should've been cached");

  ScalarEvolution &SEA = GetSE(*FnA);
  const SCEV *TCA =
      SEA.getTripCountFromExitCount(SEA.getBackedgeTakenCount(LoopA));
  ScalarEvolution &SEB = GetSE(*FnB);
  const SCEV *TCB =
      SEB.getTripCountFromExitCount(SEB.getBackedgeTakenCount(LoopB));
  SimpleValueMap ArgMapA;
  populateArgumentMapping(ArgMapA, CallA);
  SimpleValueMap ArgMapB;
  populateArgumentMapping(ArgMapB, CallB);

  unsigned Iterations =
      InlineAggressiveLongConsecutiveCallChainTripCountEqualIterations;
  bool Result = checkSCEVStructuralEqualityAcrossCall(
      TCA, TCB, ArgMapA, ArgMapB, Iterations, ParentFn);
  LLVM_DEBUG(dbgs() << "AggInl: In function " << ParentFn->getName()
                    << ", profitability " << Result
                    << " to inline pair of calls:\n"
                    << *CallA << "\n"
                    << *CallB << "\nSCEV for trip counts " << *TCA << " AND "
                    << *TCB << "\n");
  return Result;
}

bool LongConsecutiveCallChainHeuristic::isLeafFunctionCandidate(Function *F) {
  auto CacheIt = LeafFunctionCandidacyCache.find(F);
  if (CacheIt != LeafFunctionCandidacyCache.end())
    return CacheIt->second;

  // Current qualification for a candidate leaf function is very naive. More
  // analysis is done when checking for the profitability of inlining each call
  // chain.
  // Functions with variadic arguments are not accepted because they're rare and
  // make later analysis more complex.
  if (F->isDeclaration() || F->isVarArg()) {
    LeafFunctionCandidacyCache[F] = nullptr;
    return false;
  }
  const LoopInfo &LI = GetLI(*F);
  // The function should have one and only one top-level loop.
  if (LI.size() != 1) {
    LeafFunctionCandidacyCache[F] = nullptr;
    return false;
  }

  LeafFunctionCandidacyCache[F] = *LI.begin();
  return true;
}

DenseSet<const LongConsecutiveCallChainHeuristic::CallChain *>
LongConsecutiveCallChainHeuristic::findChainsWithLongConsecutiveCallChain() {
  DenseSet<const CallChain *> Result;
  // For every candidate leaf function in the module, build call chains by
  // finding all its call sites and then finding their adjacent calls.
  for (auto &F : M) {
    if (!isLeafFunctionCandidate(&F))
      continue;

    for (User *U : F.users()) {
      auto CI = dyn_cast<CallInst>(U);
      if (!CI || CI->getCalledFunction() != &F)
        continue;
      // Skip if the call chain it belongs to is already constructed (i.e. the
      // call has been visited before)
      if (CallToChainMap.find(CI) != CallToChainMap.end())
        continue;

      auto IsEligibleInstForCallChain = [this](Instruction *I) {
        auto CI = dyn_cast<CallInst>(I);
        if (!CI)
          return false;
        auto Callee = CI->getCalledFunction();
        if (!Callee || !isLeafFunctionCandidate(Callee))
          return false;
        return true;
      };

      // Start from the call, walk its adjacent instructions in both directions
      // to build its call chain.
      unsigned ChainLength = 1;
      CallInst *ChainStart = CI;
      while (Instruction *Prev = ChainStart->getPrevNonDebugInstruction()) {
        if (!IsEligibleInstForCallChain(Prev))
          break;
        assert(CallToChainMap.find(CI) == CallToChainMap.end() &&
               "The same call is visited twice while building call chain");
        ChainLength += 1;
        ChainStart = cast<CallInst>(Prev);
      }
      Instruction *ChainEnd = CI;
      while (Instruction *Next = ChainEnd->getNextNonDebugInstruction()) {
        if (!IsEligibleInstForCallChain(Next))
          break;
        assert(CallToChainMap.find(CI) == CallToChainMap.end() &&
               "The same call is visited twice while building call chain");
        ChainLength += 1;
        ChainEnd = Next;
      }
      ChainEnd = ChainEnd->getNextNonDebugInstruction();
      const CallChain &Chain =
          *CallChains.emplace(ChainStart, ChainEnd, ChainLength).first;

      // Compute the length for the longest consecutive call chain inside the
      // chain. Put it into Result if the length is above a threshold.
      Function *CalleeWithLongestCCChain = nullptr;
      // The variable is only for debug message
      (void)CalleeWithLongestCCChain;
      unsigned LongestCCChainLength = 0;
      Function *CalleeForCurrentCCChain = nullptr;
      unsigned CurrentCCChainLength = 0;
      for (Instruction *Curr = ChainStart; Curr != ChainEnd;
           Curr = Curr->getNextNonDebugInstruction()) {
        auto CurrCall = cast<CallInst>(Curr);
        auto Callee = CurrCall->getCalledFunction();
        assert(Callee &&
               "An instruction in a call chain must be a direct call");
        if (Callee != CalleeForCurrentCCChain) {
          if (LongestCCChainLength < CurrentCCChainLength) {
            CalleeWithLongestCCChain = CalleeForCurrentCCChain;
            LongestCCChainLength = CurrentCCChainLength;
          }
          CalleeForCurrentCCChain = Callee;
          CurrentCCChainLength = 1;
        } else {
          CurrentCCChainLength += 1;
        }
        CallToChainMap.emplace(CurrCall, &Chain);
      }
      if (LongestCCChainLength < CurrentCCChainLength) {
        CalleeWithLongestCCChain = CalleeForCurrentCCChain;
        LongestCCChainLength = CurrentCCChainLength;
      }

      if (LongestCCChainLength >=
          InlineAggressiveLongConsecutiveCallChainSize) {
        LLVM_DEBUG(
            dbgs() << "AggInl: long consecutive call chain located inside "
                   << CI->getFunction()->getName() << ", total length "
                   << ChainLength << ", the longest consecutive chain has "
                   << LongestCCChainLength << " calls to "
                   << CalleeWithLongestCCChain->getName()
                   << ", chain starts from " << *ChainStart << "\n");
        Result.insert(&Chain);
      }
    }
  }
  return Result;
}

void LongConsecutiveCallChainHeuristic::markCallsInLeafFunctions(
    const DenseSet<Function *> &LeafFunctions) {
  for (auto F : LeafFunctions) {
    Loop *L = LeafFunctionCandidacyCache[F];
    assert(L && "Leaf functions should've been cached");
    for (auto &BB : *F) {
      if (L->contains(&BB))
        continue;
      for (auto &I : BB) {
        auto CI = dyn_cast<CallInst>(&I);
        if (CI && CI->getCalledFunction())
          CI->addFnAttr("lccc-call-in-leaf");
      }
    }
  }
}

bool LongConsecutiveCallChainHeuristic::run() {
  LLVM_DEBUG(dbgs() << "AggInl: LongConsecutiveCallChainHeuristic\n");
  // First, find all long consecutive call chains.
  DenseSet<const CallChain *> ChainsWithLCCC =
      findChainsWithLongConsecutiveCallChain();
  if (ChainsWithLCCC.empty())
    return false;

  // Every callee function in the long chains is a leaf function.
  DenseSet<Function *> LeafFunctions;
  // Store all call sites to be inlined.
  // Their callers are also stored to help determine whether to call chains with
  // a single call.
  DenseSet<CallInst *> InlinedCallSites;
  DenseSet<const Function *> CallersOfInlinedCallSites;

  // Find all profitable pairs to inline in a call chain and do the bookkeeping.
  auto CheckProfitabilityForCallChain =
      [this, &InlinedCallSites,
       &CallersOfInlinedCallSites](const CallChain *Chain) {
        unsigned NumInlinedPairs = 0;
        for (CallInst *Curr = Chain->Start;
             Curr->getNextNonDebugInstruction() != Chain->End;
             Curr = cast<CallInst>(Curr->getNextNonDebugInstruction())) {
          auto NextCI = cast<CallInst>(Curr->getNextNonDebugInstruction());
          if (!isProfitableToInlineCallPair(Curr, NextCI))
            continue;
          NumInlinedPairs += 1;
          InlinedCallSites.insert(Curr);
          InlinedCallSites.insert(NextCI);
        }
        if (NumInlinedPairs)
          CallersOfInlinedCallSites.insert(Chain->getCaller());
        LLVM_DEBUG(dbgs() << "AggInl: LCCC in " << Chain->getCaller()->getName()
                          << ", found " << NumInlinedPairs
                          << " profitable pairs among " << Chain->Length
                          << " calls in call chain from " << *Chain->Start
                          << " TO " << *Chain->End << "\n");
        return NumInlinedPairs;
      };

  // Then check every long chain found above for profitability. If there are
  // multiple long chains, every chain needs to be profitable for the heuristic
  // to continue.
  DenseSet<const CallChain *> VisitedCallChains;
  for (auto Chain : ChainsWithLCCC) {
    VisitedCallChains.insert(Chain);
    for (Instruction *I = Chain->Start; I != Chain->End;
         I = I->getNextNonDebugInstruction())
      LeafFunctions.insert(cast<CallInst>(I)->getCalledFunction());
    if (CheckProfitabilityForCallChain(Chain) != Chain->Length - 1)
      return false;
  }

  markCallsInLeafFunctions(LeafFunctions);

  // We've found the leaf functions from the long chains. Check profitability of
  // inlining for every call chain that calls a leaf function.
  // The profitability check requires at least two calls, if there is only one
  // call, we then check whether there is any other call chain that passes the
  // check in the same function.
  DenseSet<CallInst *> SoleLeafCalls;
  for (auto LeafFunction : LeafFunctions)
    for (auto U : LeafFunction->users()) {
      auto CI = dyn_cast<CallInst>(U);
      if (!CI || CI->getCalledFunction() != LeafFunction)
        continue;
      const CallChain *Chain = CallToChainMap[CI];
      assert(
          Chain &&
          "Expected call chain to be present for every call of leaf function");

      if (VisitedCallChains.find(Chain) != VisitedCallChains.end())
        continue;
      VisitedCallChains.insert(Chain);

      if (Chain->Length == 1)
        SoleLeafCalls.insert(CI);
      else
        CheckProfitabilityForCallChain(Chain);
    }
  for (auto CI : SoleLeafCalls)
    if (CallersOfInlinedCallSites.find(CI->getCaller()) !=
        CallersOfInlinedCallSites.end())
      InlinedCallSites.insert(CI);

  for (CallInst *CI : InlinedCallSites)
    SetInline(*CI);

  // It's possible that a leaf function becomes recursive after inlining. The
  // default setting of LLVM's inliner forbids all inlining of all recursive
  // functions. To ensure more leaf functions are inlined, detect recursive
  // call chains like func_a() -> func_b() -> func_a() and prevent inlining
  // of func_b().
  // A complete analysis would build the SCC of leaf functions, but that's
  // too heavy for our current need. For the time being we just check one level
  // upward.
  for (auto LeafFunction : LeafFunctions) {
    for (auto U : LeafFunction->users()) {
      auto CI = dyn_cast<CallInst>(U);
      if (!CI || CI->getCalledFunction() != LeafFunction)
        continue;
      Function *Caller = CI->getCaller();
      for (auto UOfCaller : Caller->users()) {
        auto CallOfCaller = dyn_cast<CallInst>(UOfCaller);
        if (CallOfCaller && CallOfCaller->getCalledFunction() == Caller &&
            CallOfCaller->getCaller() == LeafFunction)
          SetNoInline(*CallOfCaller);
      }
    }
  }

  return true;
}

InlineAggressiveInfo::InlineAggressiveInfo(AggInlGetTLITy GetTLI,
                                           AggInlGetLITy GetLI,
                                           AggInlGetSETy GetSE)
    : GetTLI(GetTLI), GetLI(GetLI), GetSE(GetSE) {
  AggInlCalls.clear();
}

InlineAggressiveInfo::InlineAggressiveInfo(InlineAggressiveInfo &&Arg)
    : AggInlCalls(std::move(Arg.AggInlCalls)) {}

InlineAggressiveInfo::~InlineAggressiveInfo() {}

// Mark 'CB' as Inlined-Call by inserting 'CB' into AggInlCalls if
// it is already not there. Return 'true' if the process succeeded.
// If 'Recursive', propagate the setting up the call graph toward
// '@main'.
//
bool InlineAggressiveInfo::setAggInlInfoForCallSite(CallBase &CB,
                                                    bool Recursive) {
  Function *F = CB.getCalledFunction();
  if (F && !F->isDeclaration() && !F->isIntrinsic()) {
    if (AggInlCalls.insert(&CB))
      LLVM_DEBUG(dbgs() << "AggInl: Inserting: " << CB << "\n");
    else
      return true;
  }
  if (!Recursive)
    return true;
  return setAggInlInfoForCallSites(*CB.getCaller());
}

// All CallBases to 'F' as Inlined-Call by inserting them into AggInlCalls if
// if they are not already not there. Return 'true' if the process succeeded.
//
bool InlineAggressiveInfo::setAggInlInfoForCallSites(Function &F) {
  for (User *U : F.users()) {
    auto CCB = dyn_cast<CallBase>(U);
    if (!CCB)
      return false;
    if (!setAggInlInfoForCallSite(*CCB, true /*Recursive*/))
      return false;
  }
  return true;
}

// Returns true if 'CI' is a malloc call and it is allocating more
// than 'InlineAggressiveMallocLimit' bytes.
//
static bool isMallocAllocatingHugeMemory(const CallInst *CI) {
  Function *Callee = CI->getCalledFunction();
  StringRef FnName = Callee->getName();
  if (FnName != "malloc")
    return false;
  Value *MallocArg = CI->getArgOperand(0);
  ConstantInt *ConstInt = dyn_cast<ConstantInt>(MallocArg);
  if (!ConstInt)
    return false;
  if (ConstInt->getValue().getZExtValue() < InlineAggressiveMallocLimit)
    return false;

  return true;
}

// Returns true if return address of malloc is saved in formal argument
// of current routine and not escaped to other places.
// Example for allowed case:
//
//      LBM_allocateGrid(double** %ptr)
//      %call = call noalias i8* @malloc()
//      %0 = bitcast double** %ptr to i8**
//      store i8* %call, i8** %0
//      %tobool = icmp eq i8* %call, null
//      %add.ptr = getelementptr inbounds i8, i8* %call, i64
//      %1 = bitcast double** %ptr to i8**
//      store i8* %add.ptr, i8** %1
//
static bool isMallocAddressSavedInArg(Function &F, CallBase &CB) {

  // Returns true if A == V. This routine ignores BitCastInst.
  auto IsArgument = [](Value *V, Argument *A) {
    Value *ArgV = V;
    if (auto *BC = dyn_cast<BitCastInst>(ArgV))
      ArgV = BC->getOperand(0);
    return ArgV == A;
  };

  // Just limit to single formal pointer parameter for now
  FunctionType *FTy = F.getFunctionType();
  if (!FTy->getReturnType()->isVoidTy())
    return false;
  if (FTy->getNumParams() != 1)
    return false;
  if (!FTy->getParamType(0)->isPointerTy())
    return false;

  Value *V = &CB;
  Argument *A = F.getArg(0);

  // Memory allocation pointer is stored to Argument.
  // Ex: *Arg = CB;
  StoreInst *StoreI = nullptr;

  // Incremented memory allocation pointer is stored to Argument
  // Ex: *Arg = CB + some_constant;
  StoreInst *StoreWithIncI = nullptr;
  SmallVector<Value *, 16> WorkList;
  // Incremented values from "CB".
  // Ex: *ptr += 2000;
  DenseMap<Value *, int64_t> PtrAliasOffsetMap;
  WorkList.push_back(V);
  PtrAliasOffsetMap[V] = 0;
  while (!WorkList.empty()) {
    Value *V = WorkList.pop_back_val();
    for (auto &UI : V->uses()) {
      Value *Ptr = UI.getUser();
      // Ignore ICmp instructions.
      if (isa<ICmpInst>(Ptr))
        continue;
      if (isa<BitCastInst>(Ptr)) {
        WorkList.push_back(Ptr);
        // No Change in offset.
        PtrAliasOffsetMap[Ptr] = PtrAliasOffsetMap[V];
      } else if (auto *GEPI = dyn_cast<GetElementPtrInst>(Ptr)) {
        if (!GEPI || GEPI->getNumIndices() != 1)
          return false;
        auto CInt = dyn_cast<ConstantInt>(GEPI->getOperand(1));
        if (!CInt)
          return false;

        WorkList.push_back(Ptr);
        // Increase offset for Ptr.
        PtrAliasOffsetMap[Ptr] = PtrAliasOffsetMap[V] + CInt->getSExtValue();
      } else if (auto *SI = dyn_cast<StoreInst>(Ptr)) {
        if (!IsArgument(SI->getPointerOperand(), A))
          return false;
        if (SI->getValueOperand() != V)
          return false;
        if (PtrAliasOffsetMap[V] == 0) {
          // Storing "CB" to the Arg.
          if (StoreI)
            return false;
          StoreI = SI;
        } else {
          // Storing "CB + some_const" to the Arg.
          if (StoreWithIncI)
            return false;
          StoreWithIncI = SI;
        }
      } else {
        return false;
      }
    }
  }
  // Recognize it as Allocation routine only if CB is used only by
  // StoreI and StoreWithIncI.
  if (!StoreI || !StoreWithIncI)
    return false;
  return true;
}

// Collect global variable pointers that allocated memory using
// AllocRtn.
//   Ex:
//      LBM_allocateGrid(@srcGrid to double**))
//      LBM_allocateGrid(@dstGrid to double**))
//
// Returns false if AllocRtn is used to allocate memory other than global
// variables. All collected variables are inserted into 'Globals'.
//
static bool collectMemoryAllocatedGlobVarsUsingAllocRtn(
    Function *MallocRtn, std::vector<GlobalVariable *> &Globals) {

  for (Use &U : MallocRtn->uses()) {
    User *UR = U.getUser();
    if (!isa<CallInst>(UR)) {
      return false;
    }
    auto CB1 = cast<CallBase>(UR);
    if (!CB1->isCallee(&U))
      return false;

    Value *Glob = *(CB1->arg_begin());
    if (Operator::getOpcode(Glob) == Instruction::BitCast)
      Glob = cast<Operator>(Glob)->getOperand(0);
    ;

    if (GlobalVariable *GV = dyn_cast<GlobalVariable>(Glob)) {
      Globals.push_back(GV);
    } else {
      return false;
    }
  }
  return true;
}

using Item = std::tuple<Value *, Value *, unsigned>;

//
// For Value 'X' potentially used as an actual argument to 'CB',
// return the corresponding Argument of the callee, if it can be
// uniquely identified. Otherwise return 'nullptr'.
//
static Argument *getFormal(Value *X, CallBase *CB) {
  Function *F = CB->getCalledFunction();
  if (!F || CB->arg_size() != F->arg_size())
    return nullptr;
  bool FoundIndex = false;
  unsigned Index = 0;
  for (unsigned I = 0, E = CB->arg_size(); I < E; ++I) {
    if (CB->getArgOperand(I) == X) {
      if (!FoundIndex) {
        Index = I;
        FoundIndex = true;
      } else {
        return nullptr;
      }
    }
  }
  return FoundIndex ? F->getArg(Index) : nullptr;
}

// Track the use of the GlobalVariables in 'GVs' and mark any CallBase for
// aggressive inlining that is reachable from them. Return 'true' if the
// process is successful.
//
bool InlineAggressiveInfo::trackUsesOfAGVs(std::vector<GlobalVariable *> &GVs) {
  SmallVector<Item, 10> Worklist;
  SmallSet<Value *, 10> Visited;

  //
  // Add another Worklist element and put out a trace message.
  //
  auto TrackAndPrint = [&Worklist](Value *VS, Value *VD, unsigned IL) {
    Worklist.push_back({VS, VD, IL});
    LLVM_DEBUG(dbgs() << "TrackAggInl: FROM " << *VS << "\n");
    LLVM_DEBUG(dbgs() << "TrackAggInl: TO   " << *VD << "\n");
  };

  //
  // Main code for trackUsesOfAGVs()
  //
  for (unsigned I = 0, E = GVs.size(); I != E; ++I) {
    GlobalVariable *GV = GVs[I];
    //
    // Start by loading the Users of the GVs. Any Function is which they
    // appear, except @main, should be marked for aggressive inlining.
    //
    for (User *U : GV->users())
      TrackAndPrint(GV, U, 0);
    //
    // Propagate through the def/use chains.
    //
    while (!Worklist.empty()) {
      Item IT = Worklist.pop_back_val();
      Value *X = std::get<0>(IT);
      Value *V = std::get<1>(IT);
      unsigned IL = std::get<2>(IT);
      if (!Visited.insert(V).second)
        continue;
      //
      // Do not trace through LoadInsts at more than one level of
      // dereferencing.
      //
      auto LI = dyn_cast<LoadInst>(V);
      if (LI && LI->getPointerOperand() == X)
        if (++IL > 1)
          continue;
      //
      // Each CallBase encountered should be aggressively inlined. Mark
      // it for that, and load the Worklist with new items propagating
      // down from the formal arguments.
      //
      if (auto CB = dyn_cast<CallBase>(V)) {
        if (AggInlCalls.size() >= InlineAggressiveCSLimit) {
          LLVM_DEBUG(dbgs() << "TrackAggInl: Too many CallBases\n");
          return false;
        }
        if (!setAggInlInfoForCallSite(*CB, true /*Recursive*/)) {
          LLVM_DEBUG(dbgs() << "TrackAggInl: Could not set AggInfo for " << *CB
                            << "\n");
          return false;
        }
        Argument *A = getFormal(X, CB);
        if (!A) {
          LLVM_DEBUG(dbgs() << "TrackAggInl: NO FUNCTION OR UNIQUE FORMAL\n");
          return false;
        }
        LLVM_DEBUG(dbgs() << "TrackAggInl: ARG: " << *A << " "
                          << CB->getCalledFunction()->getName() << "\n");
        for (User *U : A->users())
          TrackAndPrint(A, U, IL);
        //
        // Storing back GV kills the analysis. Bail out.
        //
      } else if (auto SI = dyn_cast<StoreInst>(V)) {
        if (SI->getValueOperand() == GV) {
          LLVM_DEBUG(dbgs() << "TrackAggInl: StoreInst stores GlobalValue "
                            << *GV << "\n");
          return false;
        }
        //
        // Ensure that all callsites on any path from @main to this
        // Instruction get inlined.
        //
      } else if (auto II = dyn_cast<Instruction>(V)) {
        Function *F = II->getFunction();
        if (!setAggInlInfoForCallSites(*F)) {
          LLVM_DEBUG(dbgs() << "TrackAggInl: Could not set AggInfo for "
                            << F->getName() << "\n");
          return false;
        }
      }
      //
      // Normal propagation from def to use.
      //
      for (User *U : V->users())
        TrackAndPrint(V, U, IL);
    }
  }
  return true;
}

InlineAggressiveInfo InlineAggressiveInfo::runImpl(Module &M,
                                                   WholeProgramInfo &WPI,
                                                   AggInlGetTLITy GetTLI,
                                                   AggInlGetLITy GetLI,
                                                   AggInlGetSETy GetSE) {
  InlineAggressiveInfo Result(GetTLI, GetLI, GetSE);
  if (!WPI.isWholeProgramSafe()) {
    LLVM_DEBUG(dbgs() << " Skipped AggInl ... Whole Program NOT safe\n");
    return Result;
  }
  Result.setNoRecurseOnTinyFunctions(M);
  Result.analyzeModule(M);
  return Result;
}

// Inline Analysis to expose uses of global pointers to malloc'ed memory.
bool InlineAggressiveInfo::analyzeHugeMallocGlobalPointersHeuristic(Module &M) {
  Function *AllocRtn = nullptr;
  Function *MainRtn = nullptr;

  LLVM_DEBUG(dbgs() << "AggInl: HugeMallocGlobalPointersHeuristic\n");

  uint64_t TotalInstCount = 0;

  AggInlCalls.clear();

  for (Function &F : M) {

    if (F.isDeclaration() || F.isIntrinsic())
      continue;

    StringRef FName = F.getName();
    // Instead of using general isMainEntryPoint/getMainFunction to allow
    // multiple variants of “main”, allowing only “main” variant for now.
    // “norecurse” attribute is checked for all routines including “main”.
    // Currently, “norecurse” attribute is set for only “main” variant. Skip
    // unused functions besides "main" since we already checked for
    // WholeProgramSafe.
    if (F.hasMetadata("llvm.acd.clone"))
      FName = FName.take_front(FName.find('.'));
    if (FName != "main" && F.hasNUses(0))
      continue;

    if (!F.doesNotRecurse()) {
      LLVM_DEBUG(dbgs() << " Skipped AggInl ..." << F.getName()
                        << " is recursive\n");
      return false;
    }

    if (FName == "main")
      MainRtn = &F;

    for (auto &II : instructions(F)) {
      TotalInstCount++;
      if (isa<InvokeInst>(&II)) {
        LLVM_DEBUG(dbgs() << " Skipped AggInl ... InvokeInst is seen\n");
        return false;
      }
      auto CI = dyn_cast<CallInst>(&II);
      if (!CI)
        continue;
      Function *Callee = CI->getCalledFunction();

      if (!Callee) {
        LLVM_DEBUG(dbgs() << " Skipped AggInl ... Indirect call is seen\n");
        return false;
      }
      if (!isMallocAllocatingHugeMemory(CI)) {
        continue;
      }

      if (isMallocAddressSavedInArg(F, *CI)) {
        if (AllocRtn != nullptr) {
          LLVM_DEBUG(dbgs() << " Skipped AggInl ... "
                            << "Found more than 1 malloc routine\n");
          return false;
        }
        AllocRtn = &F;
      }
    }

    if (TotalInstCount > InlineAggressiveInstLimit) {
      LLVM_DEBUG(dbgs() << " Skipped AggInl ... too many instructions\n");
      return false;
    }
  }
  LLVM_DEBUG(dbgs() << " Total inst: " << TotalInstCount << "\n");

  if (MainRtn == nullptr || AllocRtn == nullptr) {
    LLVM_DEBUG(dbgs() << " Skipped AggInl ... No main/malloc routine found\n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "AggInl: " << AllocRtn->getName()
                    << " malloc routine found\n");

  std::vector<GlobalVariable *> AllocatedGlobals;
  AllocatedGlobals.clear();

  if (!collectMemoryAllocatedGlobVarsUsingAllocRtn(AllocRtn,
                                                   AllocatedGlobals)) {
    LLVM_DEBUG(
        dbgs()
        << " Skipped AggInl ... Not able to collect Allocated Globals\n");
    return false;
  }

  if (AllocatedGlobals.empty()) {
    LLVM_DEBUG(dbgs() << " Skipped AggInl ... No Allocated Globals found\n");
    return false;
  }

  LLVM_DEBUG({
    dbgs() << "AggInl:  collected globals\n";
    for (unsigned i = 0, e = AllocatedGlobals.size(); i != e; ++i)
      dbgs() << "      " << *AllocatedGlobals[i] << "\n";
  });

  if (!trackUsesOfAGVs(AllocatedGlobals)) {
    AggInlCalls.clear();
    LLVM_DEBUG(
        dbgs()
        << " Skipped AggInl ... can't track uses of Allocated Globals\n");
    return false;
  }

  LLVM_DEBUG({
    dbgs() << "AggInl:  All CallSites marked for inline after propagation\n";
    for (unsigned i = 0, e = AggInlCalls.size(); i != e; ++i)
      dbgs() << "      " << *AggInlCalls[i] << "\n";
  });

  MainRtn->addFnAttr("may_have_huge_local_malloc");
  return true;
}

// This does inline analysis to expose uses of global variable that is
// accessed in only one function and is escaped to other functions as
// arguments of calls.
//
//  Ex:
//     foo() {
//       static double* grad;
//
//       some_free_call(grad);
//       grad = nullptr;
//       ...
//       grad = some_memory_alloc();
//       ...
//       bar1(grad);
//       ...
//       new_grad = PHI(grad, ...);
//       ...
//       bar2(new_grad);
//       ...
//     }
//
// This analysis tries to inline bar1 and bar2 so that all uses of "grad"
// will be exposed in "foo".
//
// These are the main points of the analysis:
//   1. Global variable has to be accessed in a single function.
//   2. The variable is assigned with newly allocated memory. nullptr
//      assignment is also allowed.
//   3. Track all uses of the variable and collect all calls where the
//      variable is escaped as argument.
//   4. Make sure all the calls are direct calls.
//   5. Make sure callees don't have any other calls except calls
//      to small functions.
//
bool InlineAggressiveInfo::analyzeSingleAccessFunctionGlobalVarHeuristic(
    Module &M) {

  // Returns true if "Call" is a wrapper to either Alloc or Free call.
  auto IsTinyAllocFreeCall = [this](const CallBase *Call) {
    auto &TLI = GetTLI(*Call->getFunction());
    // Check for alloc call.
    if (isNoAliasCall(Call))
      return true;

    Function *F = Call->getCalledFunction();
    // Returns false if F is not small routine or has any calls except "free".
    if (!F || F->isDeclaration() || F->size() > MaxNumBBTinyFuncLimit)
      return false;
    for (const auto &I : instructions(F)) {
      if (isa<DbgInfoIntrinsic>(&I))
        continue;
      if (auto *Call = dyn_cast<CallBase>(&I))
        if (!getFreedOperand(Call, &TLI))
          return false;
    }
    return true;
  };

  // For the GlobalVar aliased to the Argument 'A' of 'F', create a
  // list of CallBases in 'F' for which a Use (defined recursively) of
  // that Argument will escape out of the 'F' and put them in 'EscCBs'.
  auto FindEscCBs = [](Function *F, Argument *A,
                       SmallPtrSet<CallBase *, 4> &EscCBs) {
    SmallVector<Value *, 10> Worklist;
    SmallSet<Value *, 10> Visited;
    for (User *U : A->users())
      Worklist.push_back(U);
    while (!Worklist.empty()) {
      Value *V = Worklist.pop_back_val();
      if (!Visited.insert(V).second)
        continue;
      if (auto CB = dyn_cast<CallBase>(V)) {
        EscCBs.insert(CB);
      }
      for (User *U : V->users())
        Worklist.push_back(U);
    }
  };

  using ALFMap = DenseMap<Function *, SmallPtrSet<Argument *, 4>>;

  // Returns true if 'F' doesn't have any user calls except tiny alloc/free
  // wrapper calls. Ignore calls that are not reachable from 'A'. Get almost
  // leaf info from AlmostLeafFunctionMap if already computed.
  auto IsAlmostLeafFunction = [&IsTinyAllocFreeCall, &FindEscCBs](
                                  Function *F, Argument *A, ALFMap &IsALFMap,
                                  ALFMap &IsNotALFMap) -> bool {
    bool IsLeaf = true;
    if (!A)
      return false;
    // Test the maps to see if we already have computed the answer.
    auto It = IsALFMap.find(F);
    if (It != IsALFMap.end() && It->second.count(A))
      return true;
    It = IsNotALFMap.find(F);
    if (It != IsNotALFMap.end() && It->second.count(A))
      return false;
    // If not, compute the answer.
    SmallPtrSet<CallBase *, 4> EscCBs;
    FindEscCBs(F, A, EscCBs);
    for (auto Call : EscCBs) {
      // Tiny alloc/free wrapper calls are allowed.
      if (IsTinyAllocFreeCall(Call))
        continue;
      // Any call to user function (except tiny alloc/free wrapper
      // calls) is considered as not leaf.
      Function *Callee = Call->getCalledFunction();
      if (!Callee || !Callee->isDeclaration()) {
        IsLeaf = false;
        break;
      }
    }
    // Store the answer for future calls to IsAlmostLeafFunction.
    if (IsLeaf)
      IsALFMap[F].insert(A);
    else
      IsNotALFMap[F].insert(A);
    return IsLeaf;
  };

  using ICItem = std::pair<CallBase *, Argument *>;
  using ICItemSet = std::set<ICItem>;
  std::function<bool(Value *, ICItemSet &,
                     SmallPtrSet<Value *, MaxNumInstructionsVisited> &)>
      TrackUses;

  // Returns false if any call in InlineCalls is indirect/not-leaf.
  auto InlineCallsOkay = [&IsAlmostLeafFunction](ICItemSet &InlineCalls,
                                                 ALFMap &IsALFMap,
                                                 ALFMap &IsNotALFMap) {
    if (InlineCalls.size() > MaxNumInlineCalls)
      return false;

    for (auto &ICI : InlineCalls) {
      CallBase *CB = ICI.first;
      Argument *A = ICI.second;
      Function *F = CB->getCalledFunction();
      // Indirect call is not allowed.
      if (!F)
        return false;

      if (F->isDeclaration())
        continue;

      // Don't allow big functions.
      if (F->size() > MaxNumBBInlineLimit)
        return false;

      // Make sure they are almost leaf.
      if (!IsAlmostLeafFunction(F, A, IsALFMap, IsNotALFMap))
        return false;
    }
    return true;
  };

  // Tracks uses of "V" recursively. This routine supports only limited
  // instruction types. Returns false if use of "V" is used by unexpected
  // instruction. All calls, which use "V" directly or indirectly, are
  // collected in "InlineCalls". "Visited" is used to control recursion and
  // reduce compile-time.
  TrackUses =
      [&TrackUses](Value *V, ICItemSet &InlineCalls,
                   SmallPtrSet<Value *, MaxNumInstructionsVisited> &Visited) {
        CallBase *CB = nullptr;
        if (!Visited.insert(V).second)
          return true;
        if (Visited.size() >= MaxNumInstructionsVisited ||
            V->hasNUsesOrMore(MaxNumInstructionsVisited))
          return false;

        for (auto *UR : V->users()) {
          auto *I = dyn_cast<Instruction>(UR);
          if (!I)
            return false;
          switch (I->getOpcode()) {
          case Instruction::Call:
            CB = cast<CallBase>(I);
            InlineCalls.insert({CB, getFormal(V, CB)});
            break;

          case Instruction::Load:
          case Instruction::Store:
          case Instruction::ICmp:
            break;

          case Instruction::BitCast:
          case Instruction::GetElementPtr:
          case Instruction::PHI:
            if (!TrackUses(I, InlineCalls, Visited))
              return false;
            break;

          default:
            return false;
          }
        }
        return true;
      };

  // Analyze all accesses of "V", which is Global Variable.
  //  1. Returns false if all uses of "V" are not in single function.
  //     "AccessFunction" is the function where V is used.
  //  2. Newly allocated memory is stored to V. nullptr assignment is ignored.
  //     No other stores are allowed.
  //  3. All loads of "V" are tracked using TrackUses.
  //
  auto AnalyzeGV = [&TrackUses](Value *V, ICItemSet &InlineCalls,
                                Function *&AccessFunction) -> bool {
    StoreInst *NullPtrStore = nullptr;
    StoreInst *AllocPtrStore = nullptr;
    SmallPtrSet<Value *, MaxNumInstructionsVisited> Visited;
    Visited.clear();
    for (auto *UR : V->users()) {
      Instruction *I = dyn_cast<Instruction>(UR);
      if (!I)
        return false;
      Function *F = I->getFunction();
      if (!AccessFunction)
        AccessFunction = F;
      else if (AccessFunction != F)
        return false;
      if (auto *LI = dyn_cast<LoadInst>(I)) {
        if (!TrackUses(LI, InlineCalls, Visited))
          return false;
      } else if (auto *SI = dyn_cast<StoreInst>(I)) {
        Value *ValOp = SI->getValueOperand();
        if (isa<Constant>(ValOp) && cast<Constant>(ValOp)->isNullValue()) {
          if (NullPtrStore)
            return false;
          NullPtrStore = SI;
        } else if (isNoAliasCall(ValOp)) {
          if (AllocPtrStore)
            return false;
          AllocPtrStore = SI;
          // Track uses of allocated pointer.
          if (!TrackUses(ValOp, InlineCalls, Visited))
            return false;
        } else {
          return false;
        }
      } else {
        return false;
      }
    }
    if (!AccessFunction || !AllocPtrStore || InlineCalls.size() == 0)
      return false;

    return true;
  };

  // Used to map function and calls that will be inlined into the function.
  DenseMap<Function *, SmallPtrSet<CallBase *, MaxNumInlineCalls>>
      InlineCallsInFunction;
  // These maps are used to avoid recomputation of leaf info. 'IsALFMap'
  // saves Arguments for which {Function *, Argument *} qualified as almost
  // leaf function for all callsites reachable from Argument, 'IsNotALFMap'
  // is for those Arguments for which {Function *, Argument *} did not
  // qualify.
  ALFMap IsALFMap;
  ALFMap IsNotALFMap;

  LLVM_DEBUG(dbgs() << "AggInl: SingleAccessFunctionGlobalVarHeuristic\n");

  for (GlobalVariable &GV : M.globals()) {
    if (!GV.hasLocalLinkage())
      continue;
    // Skip if GV has many uses.
    if (GV.hasNUsesOrMore(MaxNumGVUses))
      continue;

    ICItemSet InlineCalls;
    InlineCalls.clear();
    Function *AccessFunction = nullptr;
    if (!AnalyzeGV(&GV, InlineCalls, AccessFunction))
      continue;

    LLVM_DEBUG(dbgs() << "   GV selected as candidate: " << GV.getName()
                      << "\n");

    if (!InlineCallsOkay(InlineCalls, IsALFMap, IsNotALFMap)) {
      LLVM_DEBUG(dbgs() << "   Ignored GV ... calls are not okay to inline\n");
      continue;
    }
    for (auto &ICI : InlineCalls) {
      CallBase *CB = ICI.first;
      InlineCallsInFunction[AccessFunction].insert(CB);
    }
  }
  for (const auto &TPair : InlineCallsInFunction) {
    LLVM_DEBUG(dbgs() << "  Function: " << TPair.first->getName() << "\n");
    if (TPair.second.size() > MaxNumInlineCalls) {
      LLVM_DEBUG(
          dbgs() << "    Not inlining any calls ...exceeding heuristic.\n");
      continue;
    }
    LLVM_DEBUG(dbgs() << "  Inlining calls\n");
    for (auto *CB : TPair.second) {
      LLVM_DEBUG(dbgs() << "     " << *CB << "\n");
      if (!setAggInlInfoForCallSite(*CB, false /*Recursive*/)) {
        LLVM_DEBUG(dbgs() << "   Could not set AggInfo for " << *CB << "\n");
        return false;
      }
    }
  }
  if (AggInlCalls.size())
    return true;
  return false;
}

//
// Currently in FunctionAttrs.cpp, weakODR functions will not be
// marked as NoRecurse, because they might be pre-empted by a
// strong version. That will not happen if we detect "whole program
// seen". So we will mark some such tiny Functions as NoRecurse
// here, because the aggressive inlining analysis may test if
// Functions in the call graph are recursive. The analysis we are
// doing here is limited to tiny functions to save compile time. We
// can do a more general implementation if it is found to be useful.
//
void InlineAggressiveInfo::setNoRecurseOnTinyFunctions(Module &M) {

  auto ShouldSetNoRecurse = [this](Function &F) -> bool {
    if (F.isDeclaration())
      return false;
    if (F.doesNotRecurse())
      return false;
    if (F.size() > MaxNumBBTinyFuncLimit)
      return false;
    for (auto &I : instructions(F))
      if (auto CB = dyn_cast<CallBase>(&I)) {
        if (isa<IntrinsicInst>(CB))
          continue;
        auto Callee = CB->getCalledFunction();
        if (!Callee)
          return false;
        LibFunc TheLibFunc;
        const TargetLibraryInfo &TLI = GetTLI(*const_cast<Function *>(Callee));
        if (TLI.getLibFunc(Callee->getName(), TheLibFunc) &&
            TLI.has(TheLibFunc))
          continue;
        if (!Callee->doesNotRecurse())
          return false;
      }
    return true;
  };

  SetVector<Function *> Worklist;
  for (auto &F : M.functions())
    Worklist.insert(&F);
  while (!Worklist.empty()) {
    Function *F = Worklist.pop_back_val();
    if (ShouldSetNoRecurse(*F)) {
      F->setDoesNotRecurse();
      LLVM_DEBUG(dbgs() << "AggInl: Setting NoRecurse on: " << F->getName()
                        << "\n");
      for (User *U : F->users())
        if (auto CB = dyn_cast<CallBase>(U))
          if (CB->getCalledFunction() == F)
            Worklist.insert(CB->getCaller());
    }
  }
}

bool InlineAggressiveInfo::analyzeModule(Module &M) {
  if (analyzeHugeMallocGlobalPointersHeuristic(M)) {
    addInliningAttributes();
    return true;
  }
  if (analyzeSingleAccessFunctionGlobalVarHeuristic(M)) {
    addInliningAttributes();
    return true;
  }
  LongConsecutiveCallChainHeuristic LCCCA(
      M,
      [this](CallBase &CB) {
        setAggInlInfoForCallSite(CB, false /*Recursive*/);
      },
      [](CallBase &CB) { CB.setIsNoInline(); }, GetLI, GetSE);
  if (LCCCA.run()) {
    addInliningAttributes();
    return true;
  }
  return false;
}

void InlineAggressiveInfo::addInliningAttributes() {
  for (unsigned I = 0, E = AggInlCalls.size(); I != E; ++I) {
    CallBase *CB = AggInlCalls[I];
    CB->addFnAttr("prefer-inline-aggressive");
  }
}

AggInlinerPass::AggInlinerPass(void) {}

PreservedAnalyses AggInlinerPass::run(Module &M, ModuleAnalysisManager &AM) {
  std::unique_ptr<InlineAggressiveInfo> Result;
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function *>(&F)));
  };
  auto GetLI = [&FAM](const Function &F) -> LoopInfo & {
    return FAM.getResult<LoopAnalysis>(*(const_cast<Function *>(&F)));
  };
  auto GetSE = [&FAM](const Function &F) -> ScalarEvolution & {
    return FAM.getResult<ScalarEvolutionAnalysis>(
        *(const_cast<Function *>(&F)));
  };
  Result.reset(new InlineAggressiveInfo(InlineAggressiveInfo::runImpl(
      M, AM.getResult<WholeProgramAnalysis>(M), GetTLI, GetLI, GetSE)));
  return PreservedAnalyses::all();
}
