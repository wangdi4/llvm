//===--------------- Intel_AggInliner.cpp --------------------------------===//
//
// Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
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
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/MemoryBuiltins.h"
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

InlineAggressiveInfo::InlineAggressiveInfo(AggInlGetTLITy GetTLI)
    : GetTLI(GetTLI) {
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
                                                   AggInlGetTLITy GetTLI) {
  InlineAggressiveInfo Result(GetTLI);
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
  for (auto TPair : InlineCallsInFunction) {
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

  auto ShouldSetNoRecurse = [](Function &F) -> bool {
    if (F.isDeclaration())
      return false;
    if (F.doesNotRecurse())
      return false;
    if (F.size() > MaxNumBBTinyFuncLimit)
      return false;
    for (auto &I : instructions(F))
      if (isa<CallBase>(&I))
        return false;
    return true;
  };

  for (auto &F : M.functions())
    if (ShouldSetNoRecurse(F)) {
      F.setDoesNotRecurse();
      LLVM_DEBUG(dbgs() << "AggInl: Setting NoRecurse on: " << F.getName()
                        << "\n");
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
  return false;
}

void InlineAggressiveInfo::addInliningAttributes() {
  for (unsigned I = 0, E = AggInlCalls.size(); I != E; ++I) {
    CallBase *CB = AggInlCalls[I];
    CB->addFnAttr("prefer-inline-aggressive");
  }
}

namespace {

class AggInlinerLegacyPass : public ModulePass {
  std::unique_ptr<InlineAggressiveInfo> Result;

public:
  static char ID; // Pass identification, replacement for typeid
  AggInlinerLegacyPass(void) : ModulePass(ID) {
    initializeAggInlinerLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
  }

  bool runOnModule(Module &M) override {
    auto &WPA = getAnalysis<WholeProgramWrapperPass>();
    auto GetTLI = [this](const Function &F) -> const TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };
    if (skipModule(M))
      return false;
    Result.reset(new InlineAggressiveInfo(
        InlineAggressiveInfo::runImpl(M, WPA.getResult(), GetTLI)));
    return false;
  }

  bool doFinalization(Module &M) override {
    Result.reset();
    return false;
  }
};

} // namespace

INITIALIZE_PASS_BEGIN(AggInlinerLegacyPass, "agginliner", "AggInliner", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(AggInlinerLegacyPass, "agginliner", "AggInliner", false,
                    false)

char AggInlinerLegacyPass::ID = 0;

ModulePass *llvm::createAggInlinerLegacyPass(void) {
  return new AggInlinerLegacyPass();
}

AggInlinerPass::AggInlinerPass(void) {}

char AggInlinerPass::PassID;

PreservedAnalyses AggInlinerPass::run(Module &M, ModuleAnalysisManager &AM) {
  std::unique_ptr<InlineAggressiveInfo> Result;
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function *>(&F)));
  };
  Result.reset(new InlineAggressiveInfo(InlineAggressiveInfo::runImpl(
      M, AM.getResult<WholeProgramAnalysis>(M), GetTLI)));
  return PreservedAnalyses::all();
}
