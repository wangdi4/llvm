//===------- Intel_AggInline.cpp - Aggressive Inline Analysis -*------===//
//
// Copyright (C) 2016-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// This file does Aggressive Inline Analysis to expose uses of global
// pointers to malloc'ed memory. It helps data-transformations to easily
// analyze all uses of global pointers in single routine.
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_AggInline.h"
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

using namespace llvm;

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

// Function is not allowed to inline if size of the function exceeds this limit.
static const unsigned MaxNumBBInlineLimit = 32;

// Function is not considered as tiny function if size of the function exceeds
// this limit.
static const unsigned MaxNumBBTinyFuncLimit = 1;

#define DEBUG_TYPE "inlineaggressiveanalysis"

INITIALIZE_PASS_BEGIN(InlineAggressiveWrapperPass, "inlineaggressiveanalysis",
                      "inline aggressive analysis", false, false)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(InlineAggressiveWrapperPass, "inlineaggressiveanalysis",
                    "inline aggressive analysis", false, false)

char InlineAggressiveWrapperPass::ID = 0;

ModulePass *llvm::createInlineAggressiveWrapperPassPass() {
  return new InlineAggressiveWrapperPass();
}

InlineAggressiveWrapperPass::InlineAggressiveWrapperPass() : ModulePass(ID) {
  initializeInlineAggressiveWrapperPassPass(*PassRegistry::getPassRegistry());
}

bool InlineAggressiveWrapperPass::doFinalization(Module &M) {
  Result.reset();
  return false;
}

bool InlineAggressiveWrapperPass::runOnModule(Module &M) {
  auto &WPA = getAnalysis<WholeProgramWrapperPass>();
  auto GetTLI = [this](const Function &F) -> const TargetLibraryInfo & {
    return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
  };

  Result.reset(new InlineAggressiveInfo(
      InlineAggressiveInfo::runImpl(M, WPA.getResult(), GetTLI)));
  return false;
}

InlineAggressiveInfo::InlineAggressiveInfo(AggInlGetTLITy GetTLI)
    : GetTLI(GetTLI) {
  AggInlCalls.clear();
}

InlineAggressiveInfo::InlineAggressiveInfo(InlineAggressiveInfo &&Arg)
    : AggInlCalls(std::move(Arg.AggInlCalls)) {}

InlineAggressiveInfo::~InlineAggressiveInfo() {}

// Returns true if Aggressive Inline occured.
//
bool InlineAggressiveInfo::isAggInlineOccured(void) {
  if (AggInlCalls.size())
    return true;
  return false;
}

// Mark 'CS' as Inlined-Call by inserting 'CS' into AggInlCalls if
// it is already not there.
//
void InlineAggressiveInfo::setAggInlInfoForCallSite(CallBase &CB) {
  if (isCallInstInAggInlList(CB))
    return;
  AggInlCalls.push_back(&CB);
}

// Returns true if 'CS' is marked as inlined-call i.e 'CS' is in
// AggInlCall.
//
bool InlineAggressiveInfo::isCallInstInAggInlList(CallBase &CB) {
  for (unsigned i = 0, e = AggInlCalls.size(); i != e; ++i) {
    if (AggInlCalls[i] == &CB)
      return true;
  }
  return false;
}

// Returns true if there are no calls to user defined routines in
// callee of 'CS'. This function is used to prove that formals of
// a routine are not escaped to any other user defined routines.
//
static bool noCallsToUserDefinedRoutinesInCallee(CallBase &CB) {
  Function *F = CB.getCalledFunction();
  if (!F)
    return false;
  for (inst_iterator II = inst_begin(F), E = inst_end(F); II != E; ++II) {
    auto CB = dyn_cast<CallBase>(&*II);
    if (!CB)
      continue;
    Function *Callee = CB->getCalledFunction();
    if (!Callee || !Callee->isDeclaration()) {
      return false;
    }
  }
  return true;
}

// Mark all callsites of 'F' as aggressive-inlined-calls.
//
bool InlineAggressiveInfo::setAggInlineInfoForAllCallSites(Function *F) {
  for (auto *UR : F->users()) {
    if (!isa<CallInst>(UR)) {
      return false;
    }
    auto CB1 = cast<CallBase>(UR);
    setAggInlInfoForCallSite(*CB1);
  }
  return true;
}

// Mark 'CS' as aggressive-inlined-calls and all callsites of callee of
// 'CS' as aggressive-inlined-calls.
//
bool InlineAggressiveInfo::setAggInlineInfo(CallBase &CB) {
  setAggInlInfoForCallSite(CB);
  Function *Callee = CB.getCalledFunction();
  if (!Callee)
    return false;
  return setAggInlineInfoForAllCallSites(Callee);
}

// Propagate AggInfo from callsites to called functions recursively.
//
bool InlineAggressiveInfo::propagateAggInlineInfoCall(CallBase &CB) {

  Function *Callee = CB.getCalledFunction();

  if (!Callee || Callee->isDeclaration()) {
    return false;
  }
  bool DoInline = false;
  for (inst_iterator II = inst_begin(Callee), E = inst_end(Callee); II != E;
       ++II) {

    auto CB1 = dyn_cast<CallBase>(&*II);
    if (!CB1)
      continue;
    if (propagateAggInlineInfoCall(*CB1)) {
      setAggInlineInfo(*CB1);
      DoInline = true;
    }
  }
  if (isCallInstInAggInlList(CB))
    DoInline = true;

  return DoInline;
}

// Set AggInfo to any callsite that eventually calls a callsite, which
// is marked as Inlined-call for Aggressive Analysis.
//
bool InlineAggressiveInfo::propagateAggInlineInfo(Function *F) {
  for (inst_iterator II = inst_begin(F), E = inst_end(F); II != E; ++II) {
    auto CB = dyn_cast<CallBase>(&*II);
    if (!CB)
      continue;
    if (propagateAggInlineInfoCall(*CB))
      setAggInlineInfo(*CB);
  }

  // Check a limit on number of callsites that marked as inlined-calls.
  if (AggInlCalls.size() > InlineAggressiveCSLimit)
    return false;

  // Disable Aggressive analysis if any callsite is marked as both NoInline
  // and aggressive-inlined-call.
  for (unsigned i = 0, e = AggInlCalls.size(); i != e; ++i) {
    auto CB = cast<CallBase>(AggInlCalls[i]);
    if (CB->isNoInline())
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

// Returns true if 'SI' store instruction is saving 'V' in formal
// argument of current routine.
//
//   Ex:
//      LBM_allocateGrid(double** %ptr)
//      store i8* %V, i8** %ptr
//
static bool isValueSavedInArg(Value *V, StoreInst *SI) {
  if (V != SI->getOperand(0))
    return false;
  Value *V2 = SI->getOperand(1);
  if (Operator::getOpcode(V2) == Instruction::BitCast)
    V2 = cast<Operator>(V2)->getOperand(0);
  if (!isa<Argument>(V2))
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

  // Just limit to single formal pointer parameter for now
  FunctionType *FTy = F.getFunctionType();
  if (!FTy->getReturnType()->isVoidTy())
    return false;
  if (FTy->getNumParams() != 1)
    return false;
  if (!FTy->getParamType(0)->isPointerTy())
    return false;

  Value *V = &CB;

  bool malloc_saved_in_arg = false;
  for (auto *I : V->users()) {
    if (isa<ICmpInst>(I)) {
      // Ignore use in ICmp
      continue;
    }
    if (GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(I)) {
      // %add.ptr = getelementptr inbounds i8, i8* %call, i64 0
      // %1 = bitcast double** %ptr to i8**
      // store i8* %add.ptr, i8** %1
      if (!GEPI->hasOneUse()) {
        return false;
      }
      if (StoreInst *SI = dyn_cast<StoreInst>(*GEPI->user_begin())) {
        if (isValueSavedInArg(GEPI, SI))
          malloc_saved_in_arg = true;
        else
          return false;
      } else
        return false;
    } else if (StoreInst *SI = dyn_cast<StoreInst>(I)) {
      // store i8* %call, i8** %0
      if (isValueSavedInArg(V, SI))
        malloc_saved_in_arg = true;
      else
        return false;
    } else {
      return false;
    }
  }
  return malloc_saved_in_arg;
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

// It tracks all uses of global variables in 'Globals' and it basically
// does two main things:
//
//   1. Mark callsites as aggressive-inlined-calls if dereference or
//      address of global variables in 'Globals' are passed as arguments.
//        Ex:
//             LBM_allocateGrid(@srcGrid))
//
//             %0 = load @srcGrid,
//             %arraydecay = getelementptr %0, i64 0,
//             LBM_initializeGrid(double* %arraydecay)
//
//   2. Mark all callsites of a routine if it has any references to the
//      globals variables in 'Globals'
//      Ex:
//             LBM_swapGrids()  {
//                 ...
//                 store i64 %1, @srcGrid to i64*
//                 ...
//             }
//
bool InlineAggressiveInfo::trackUsesofAllocatedGlobalVariables(
    std::vector<GlobalVariable *> &Globals) {
  for (unsigned i = 0, e = Globals.size(); i != e; ++i) {
    GlobalVariable *GV = Globals[i];
    for (User *U1 : GV->users()) {

      while (!isa<CallInst>(U1)) {
        //   %3 = load [208000000 x double]*, [208000000 x double]** @srcGrid
        //   %arraydecay2 = getelementptr  %3, i64 0, i64 0
        //   @LBM_loadObstacleFile(double* %arraydecay2, i8* nonnull %2) #5
        //
        if (!U1->hasOneUse()) {
          break;
        }
        if (Operator::getOpcode(U1) == Instruction::BitCast ||
            Operator::getOpcode(U1) == Instruction::Load ||
            Operator::getOpcode(U1) == Instruction::GetElementPtr) {
          U1 = *U1->user_begin();
        } else {
          LLVM_DEBUG(dbgs() << " Skipped AggInl ... unexpeced use of global\n"
                            << "      " << *U1 << "\n");
          return false;
        }
      }

      assert(U1 && "Expecting use");

      if (CallInst *CI2 = dyn_cast<CallInst>(U1)) {
        auto CB1 = cast<CallBase>(CI2);
        // Mark callsite as aggressive-inlined-call only if callee
        // doesn't have any calls to user defined routines so that
        // we can ignore propagating formals to other calls in Callee.
        //
        if (!noCallsToUserDefinedRoutinesInCallee(*CB1)) {
          LLVM_DEBUG(dbgs()
                     << " Skipped AggInl ... global may be escaped in callee\n"
                     << "      " << CB1 << "\n");
          return false;
        }
        LLVM_DEBUG(dbgs() << "AggInl:  Marking callsite for inline  \n"
                          << "      " << CB1 << "\n");
        setAggInlInfoForCallSite(*CB1);
        continue;
      }

      // Process all uses if user has multiple uses.
      for (User *U2 : U1->users()) {
        if (Operator::getOpcode(U2) == Instruction::Store) {
          //  LBM_swapGrids()  {
          //     ...
          //     store i64 %1, @srcGrid to i64*)
          Function *F1 = cast<Instruction>(U2)->getParent()->getParent();
          LLVM_DEBUG(dbgs() << "AggInl:  Marking all callsites of "
                            << F1->getName() << "\n");
          setAggInlineInfoForAllCallSites(F1);
        } else if (CallInst *CI2 = dyn_cast<CallInst>(U2)) {
          // %7 = load @srcGrid,
          // %arraydecay8 = getelementptr %7, i64 0
          // LBM_initializeSpecialCellsForChannel(double* %arraydecay8)
          // LBM_initializeSpecialCellsForLDC(double* %arraydecay8)
          auto CB1 = cast<CallBase>(CI2);
          if (!noCallsToUserDefinedRoutinesInCallee(*CB1)) {
            LLVM_DEBUG(dbgs()
                       << " Skipped AggInl ...global may be escaped in callee\n"
                       << "      " << CB1 << "\n");
            return false;
          }
          LLVM_DEBUG(dbgs() << "AggInl:  Marking callsite for inline  \n"
                            << "      " << CB1 << "\n");
          setAggInlInfoForCallSite(*CB1);
        } else if (Operator::getOpcode(U2) == Instruction::Load) {
          for (User *U3 : U2->users()) {
            if (Operator::getOpcode(U3) != Instruction::Store) {
              LLVM_DEBUG(dbgs()
                         << " Skipped AggInl ... unexpected use of global\n"
                         << "      " << *U3 << "\n");
              return false;
            }
            Function *F1 = cast<Instruction>(U3)->getParent()->getParent();
            LLVM_DEBUG(dbgs() << "AggInl:  Marking all callsites of "
                              << F1->getName() << "\n");
            setAggInlineInfoForAllCallSites(F1);
          }
        } else {
          LLVM_DEBUG(dbgs() << " Skipped AggInl ... unexpected use of global\n"
                            << "      " << *U2 << "\n");
          return false;
        }
      }
    }
  }
  return true;
}

InlineAggressiveInfo InlineAggressiveInfo::runImpl(Module &M,
                                                   WholeProgramInfo &WPI,
                                                   AggInlGetTLITy GetTLI) {
  InlineAggressiveInfo Result(GetTLI);
  if (!WPI.isWholeProgramSafe()) {
    LLVM_DEBUG(dbgs() << " Skipped AggInl ... Whole Program NOT safe \n");
    return Result;
  }
  Result.analyzeModule(M);
  return Result;
}

// Inline Analysis to expose uses of global pointers to malloc'ed memory.
bool InlineAggressiveInfo::analyzeHugeMallocGlobalPointersHeuristic(Module &M) {
  Function *AllocRtn = nullptr;
  Function *MainRtn = nullptr;

  uint64_t TotalInstCount = 0;

  AggInlCalls.clear();

  for (Function &F : M) {

    if (F.isDeclaration() || F.isIntrinsic())
      continue;

    if (!F.doesNotRecurse()) {
      LLVM_DEBUG(dbgs() << " Skipped AggInl ..." << F.getName()
                        << " is recursive \n");
      return false;
    }

    if (F.getName() == "main")
      MainRtn = &F;

    for (inst_iterator II = inst_begin(F), E = inst_end(F); II != E; ++II) {

      TotalInstCount++;
      if (isa<InvokeInst>(&*II)) {
        LLVM_DEBUG(dbgs() << " Skipped AggInl ... InvokeInst is seen");
        return false;
      }
      const CallInst *CI = dyn_cast<CallInst>(&*II);
      if (!CI || !isa<CallInst>(CI))
        continue;
      Function *Callee = CI->getCalledFunction();

      if (Callee == nullptr) {
        LLVM_DEBUG(dbgs() << " Skipped AggInl ... Indirect call is seen");
        return false;
      }
      if (!isMallocAllocatingHugeMemory(CI)) {
        continue;
      }

      auto CB = cast<CallBase>(&*II);
      if (isMallocAddressSavedInArg(F, *CB)) {
        if (AllocRtn != nullptr) {
          LLVM_DEBUG(dbgs()
                     << " Skipped AggInl ... Found more than 1 malloc routine");
          return false;
        }
        AllocRtn = &F;
      }
    }

    if (TotalInstCount > InlineAggressiveInstLimit) {
      LLVM_DEBUG(dbgs() << " Skipped AggInl ... too many instructions");
      return false;
    }
  }
  LLVM_DEBUG(dbgs() << " Total inst: " << TotalInstCount << "\n");

  if (MainRtn == nullptr || AllocRtn == nullptr) {
    LLVM_DEBUG(dbgs() << " Skipped AggInl ... No main/malloc routine found");
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
    LLVM_DEBUG(dbgs() << " Skipped AggInl ... No Allocated Globals found");
    return false;
  }

  LLVM_DEBUG({
    dbgs() << "AggInl:  collected globals \n";
    for (unsigned i = 0, e = AllocatedGlobals.size(); i != e; ++i)
      dbgs() << "      " << *AllocatedGlobals[i] << "\n";
  });

  if (!trackUsesofAllocatedGlobalVariables(AllocatedGlobals)) {
    AggInlCalls.clear();
    LLVM_DEBUG(
        dbgs()
        << " Skipped AggInl ... can't track uses of Allocated Globals\n");
    return false;
  }

  if (!propagateAggInlineInfo(MainRtn)) {
    AggInlCalls.clear();
    LLVM_DEBUG(
        dbgs() << " Skipped AggInl ... can't propagate Agg Inline info\n");
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

  std::function<bool(Value *, SmallPtrSet<CallBase *, MaxNumInlineCalls> &,
                     SmallPtrSet<Value *, MaxNumInstructionsVisited> &)>
      TrackUses;

  // Returns true if "Call" is a wrapper to either Alloc or Free call.
  auto IsTinyAllocFreeCall = [this](const CallBase *Call) {
    auto &TLI = GetTLI(*Call->getFunction());
    // Check for alloc call.
    if (isNoAliasFn(Call, &TLI))
      return true;

    Function *F = Call->getCalledFunction();
    // Returns false if F is not small routine or has any calls except "free".
    if (!F || F->isDeclaration() || F->size() > MaxNumBBTinyFuncLimit)
      return false;
    for (const auto &I : instructions(F)) {
      if (isa<DbgInfoIntrinsic>(&I))
        continue;
      if (auto *Call = dyn_cast<CallBase>(&I))
        if (!isFreeCall(Call, &TLI))
          return false;
    }
    return true;
  };

  // Returns true if F doesn't have any user calls except tiny alloc/free
  // wrapper calls. Get almost leaf info from AlmostLeafFunctionMap if already
  // computed.
  auto IsAlmostLeafFunction =
      [&IsTinyAllocFreeCall](
          Function *F, DenseMap<Function *, bool> &AlmostLeafFunctionMap) {
        bool IsLeaf = true;

        if (AlmostLeafFunctionMap.find(F) != AlmostLeafFunctionMap.end()) {
          IsLeaf = AlmostLeafFunctionMap[F];
        } else {
          for (const auto &I : instructions(F)) {
            if (auto *Call = dyn_cast<CallBase>(&I)) {
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
          }
          AlmostLeafFunctionMap[F] = IsLeaf;
        }
        return IsLeaf;
      };

  // Returns false if any call in InlineCalls is indirect/not-leaf.
  auto InlineCallsOkay =
      [&IsAlmostLeafFunction](
          SmallPtrSet<CallBase *, MaxNumInlineCalls> &InlineCalls,
          DenseMap<Function *, bool> &AlmostLeafFunctionMap) {
        if (InlineCalls.size() > MaxNumInlineCalls)
          return false;

        for (auto *CB : InlineCalls) {
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
          if (!IsAlmostLeafFunction(F, AlmostLeafFunctionMap))
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
      [&TrackUses](Value *V,
                   SmallPtrSet<CallBase *, MaxNumInlineCalls> &InlineCalls,
                   SmallPtrSet<Value *, MaxNumInstructionsVisited> &Visited) {
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
            InlineCalls.insert(cast<CallBase>(I));
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
  auto AnalyzeGV = [this, &TrackUses](
                       Value *V,
                       SmallPtrSet<CallBase *, MaxNumInlineCalls> &InlineCalls,
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
        } else if (isNoAliasFn(ValOp, &GetTLI(*SI->getFunction()))) {
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
  // This map is used to avoid recomputation of leaf info.
  DenseMap<Function *, bool> AlmostLeafFunctionMap;

  LLVM_DEBUG(
      dbgs() << " Started AggInl SingleAccessFunctionGlobalVar Analysis\n");

  for (GlobalVariable &GV : M.globals()) {
    if (!GV.hasLocalLinkage())
      continue;
    // Skip if GV has many uses.
    if (GV.hasNUsesOrMore(MaxNumGVUses))
      continue;

    SmallPtrSet<CallBase *, MaxNumInlineCalls> InlineCalls;
    InlineCalls.clear();
    Function *AccessFunction = nullptr;
    if (!AnalyzeGV(&GV, InlineCalls, AccessFunction))
      continue;

    LLVM_DEBUG(dbgs() << "   GV selected as candidate: " << GV.getName()
                      << "\n");

    if (!InlineCallsOkay(InlineCalls, AlmostLeafFunctionMap)) {
      LLVM_DEBUG(dbgs() << "   Ignored GV ... calls are not okay to inline \n");
      continue;
    }
    for (auto *CB : InlineCalls)
      InlineCallsInFunction[AccessFunction].insert(CB);
  }
  for (auto TPair : InlineCallsInFunction) {
    LLVM_DEBUG(dbgs() << "  Function: " << TPair.first->getName() << "\n");
    if (TPair.second.size() > MaxNumInlineCalls) {
      LLVM_DEBUG(
          dbgs() << "    Not inlining any calls ...exceeding heuristic. \n");
      continue;
    }
    LLVM_DEBUG(dbgs() << "  Inlining calls \n");
    for (auto *CB : TPair.second) {
      LLVM_DEBUG(dbgs() << "     " << *CB << "\n");
      setAggInlInfoForCallSite(*CB);
    }
  }
  if (AggInlCalls.size())
    return true;
  return false;
}

bool InlineAggressiveInfo::analyzeModule(Module &M) {
  if (analyzeHugeMallocGlobalPointersHeuristic(M))
    return true;
  if (analyzeSingleAccessFunctionGlobalVarHeuristic(M))
    return true;
  return false;
}

// This analysis depends on WholeProgramAnalysis. Analysis info is not
// modified by any other pass.
//
void InlineAggressiveWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<WholeProgramWrapperPass>();
  AU.addRequired<TargetLibraryInfoWrapperPass>();
}

char InlineAggAnalysis::PassID;

// Provide a definition for the static class member used to identify passes.
AnalysisKey InlineAggAnalysis::Key;

InlineAggressiveInfo InlineAggAnalysis::run(Module &M,
                                            AnalysisManager<Module> &AM) {

  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function *>(&F)));
  };
  return InlineAggressiveInfo::runImpl(M, AM.getResult<WholeProgramAnalysis>(M),
                                       GetTLI);
}
