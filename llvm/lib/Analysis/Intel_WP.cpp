//===------- Intel_WP.cpp - Whole Program Analysis -*------===//
//
// Copyright (C) 2016-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// This file does whole-program-analysis using TargetLibraryInfo.
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_WP.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

using namespace llvm;
using namespace llvm::llvm_intel_wp_analysis;

#define DEBUG_TYPE "whole-program-analysis"
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
#define USING_DEBUG_MODE 1
#endif // NDEBUG || LLVM_ENABLE_DUMP

namespace llvm {
namespace llvm_intel_wp_analysis {
// If it is true, compiler assumes that source files in the current
// compilation have entire program.
bool AssumeWholeProgram = false;
} // namespace llvm_intel_wp_analysis
} // namespace llvm

static cl::opt<bool, true>
    AssumeWholeProgramOpt("whole-program-assume", cl::ReallyHidden,
                          cl::location(AssumeWholeProgram));

// Flag to print the libfuncs found by whole program analysis.
static cl::opt<bool> WholeProgramTraceLibFuncs("whole-program-trace-libfuncs",
                                               cl::init(false),
                                               cl::ReallyHidden);

// Flag for printing the symbols that weren't internalized in the module
static cl::opt<bool>
    WholeProgramTraceVisibility("whole-program-trace-visibility",
                                cl::init(false), cl::ReallyHidden);

// Flag for printing the symbols resolution found by the linker
static cl::opt<bool> WholeProgramReadTrace("whole-program-read-trace",
                                           cl::init(false), cl::ReallyHidden);

// Flag asserts that whole program will be detected.
static cl::opt<bool> WholeProgramAssert("whole-program-assert", cl::init(false),
                                        cl::ReallyHidden);

// If it is true, compiler assumes that whole program has been read (linker can
// reach all symbols).
static cl::opt<bool> AssumeWholeProgramRead("whole-program-assume-read",
                                            cl::init(false), cl::ReallyHidden);

// The compiler will assume that all symbols have hidden visibility if turned on
static cl::opt<bool> AssumeWholeProgramHidden("whole-program-assume-hidden",
                                              cl::init(false),
                                              cl::ReallyHidden);

// If it is true, compiler assumes that program is linked as executable.
static cl::opt<bool>
    AssumeWholeProgramExecutable("whole-program-assume-executable",
                                 cl::init(false), cl::ReallyHidden);

// Flag to get whole program advanced optimization computation trace.
static cl::opt<bool>
    WholeProgramAdvanceOptTrace("whole-program-advanced-opt-trace",
                                cl::init(false), cl::ReallyHidden);

INITIALIZE_PASS_BEGIN(WholeProgramWrapperPass, "wholeprogramanalysis",
                      "Whole program analysis", false, false)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_END(WholeProgramWrapperPass, "wholeprogramanalysis",
                    "Whole program analysis", false, false)

char WholeProgramWrapperPass::ID = 0;

ModulePass*
llvm::createWholeProgramWrapperPassPass(WholeProgramUtils WPUtils) {
  return new WholeProgramWrapperPass(std::move(WPUtils));
}

WholeProgramWrapperPass::WholeProgramWrapperPass()
    : ModulePass(ID) {
  initializeWholeProgramWrapperPassPass(*PassRegistry::getPassRegistry());
}

WholeProgramWrapperPass::WholeProgramWrapperPass(WholeProgramUtils WPUtils)
    : ModulePass(ID), WPUtils(std::move(WPUtils)) {
  initializeWholeProgramWrapperPassPass(*PassRegistry::getPassRegistry());
}

bool WholeProgramWrapperPass::doFinalization(Module &M) {
  Result.reset();
  return false;
}

bool WholeProgramWrapperPass::runOnModule(Module &M) {
  auto GTTI = [this](Function &F) -> TargetTransformInfo & {
    return this->getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
  };

  auto GetTLI = [this](Function &F) -> TargetLibraryInfo & {
    return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
  };

  WholeProgramInfo *WPAResult = new WholeProgramInfo(&M, GetTLI, GTTI,
                                                     &WPUtils);
  WPAResult->analyzeModule();

  Result.reset(WPAResult);

  return false;
}

WholeProgramInfo::WholeProgramInfo(Module *M,
    std::function<const TargetLibraryInfo &(Function &F)> GetTLI,
    function_ref<TargetTransformInfo &(Function &)> GTTI,
    WholeProgramUtils *WPUtils) : WPUtils(WPUtils) {
  // WholeProgramSafe: WholeProgramRead && WholeProgramSeen &&
  //                   LinkingForExecutable
  WholeProgramSafe = false;

  // WholeProgramSeen: All functions with definition (IR) where internalized,
  //                   those who weren't internalized or are declaration must
  //                   be LibFuncs or intrinsic functions. Also, main must
  //                   be found.
  WholeProgramSeen = false;

  // WholeProgramHidden: This is part of WholeProgramSeen, it basically means
  //                     that all functions with definition (IR) where
  //                     internalized and those who weren't internalized must
  //                     be identified as LibFuncs. This is is useful when
  //                     debugging transformations that require internalization,
  //                     for example: devirtualization.
  WholeProgramHidden = true;

  // Another important term:
  // WholeProgramRead: All symbols were resolved by the linker.

  this->M = M;
  this->GetTLI = GetTLI;
  this->GTTI = GTTI;

  auto EE = TargetTransformInfo::AdvancedOptLevel::AO_TargetNumLevels;
  unsigned E = static_cast<unsigned>(EE);
  for (unsigned I = 0; I < E; ++I)
    IsAdvancedOptEnabled[I] = true;
}

WholeProgramInfo::~WholeProgramInfo() {}

#ifdef USING_DEBUG_MODE
// Function to print the trace for whole program. Types of traces:
//
//   Simple: just print the missing aliases, missing libfuncs and functions
//           that weren't internalized
//           -mllvm -debug-only=whole-program-analysis
//
//   Library Functions: just print the library functions that were found
//                      and the missing ones
//                      -mllvm -debug-only=whole-program-analysis
//                      -mllvm -whole-program-trace-libfuncs
//
//   Visible Outside LTO: just print the functions that weren't internalized
//                        -mllvm -debug-only=whole-program-analysis
//                        -mllvm -whole-program-trace-visibility
//
//   Whole Program Read: just print the result from the whole program read
//                       analysis
//                       -mllvm -debug-only=whole-program-analysis
//                       -mllvm -whole-program-read-trace
//
//   Full: Print all
//         -mllvm -debug-only=whole-program-analysis
//         -mllvm -whole-program-trace-libfuncs
//         -mllvm -whole-program-trace-visibility
//         -mllvm -whole-program-read-trace
//
// TODO: These opt variables need to be converted to debug-only style.
void WholeProgramInfo::printWholeProgramTrace() {

  auto PrintAliases = [this](void) {
    dbgs() << "  ALIASES UNRESOLVED: " << AliasesNotFound.size() << "\n";
    for (auto GA : AliasesNotFound)
      dbgs() << "    " << GA->getName() << "\n";
  };

  auto PrintLibFuncsFound = [this](void) {
    dbgs() << "  LIBFUNCS FOUND: " << LibFuncsFound.size() << "\n";
    for (auto LibFunc : LibFuncsFound)
      dbgs() << "    " << LibFunc->getName() << "\n";
  };

  auto PrintLibFuncsNotFound = [this](void) {
    dbgs() << "  LIBFUNCS NOT FOUND: " << LibFuncsNotFound.size() << "\n";
    for (auto LibFunc : LibFuncsNotFound)
      dbgs() << "    " << LibFunc->getName() << "\n";
  };

  auto PrintVisibility = [this](void) {
    dbgs() << "  VISIBLE OUTSIDE LTO: " << VisibleFunctions.size() << "\n";
    for (auto VisibleFunc : VisibleFunctions)
      dbgs() << "    " << VisibleFunc->getName() << "\n";
  };

  auto PrintWPResult = [this](void) {
    dbgs() << "  WHOLE PROGRAM RESULT: \n";
    dbgs() << "    MAIN DEFINITION: " << (MainDefSeen ? " " : " NOT ")
           << "DETECTED \n";

    if (!AssumeWholeProgram) {
      dbgs() << "    LINKING AN EXECUTABLE: "
             << (isLinkedAsExecutable() ? " " : " NOT ") << "DETECTED\n";
      dbgs() << "    WHOLE PROGRAM READ: "
             << (isWholeProgramRead() ? " " : " NOT ") << "DETECTED \n";
      dbgs() << "    WHOLE PROGRAM SEEN: "
             << (isWholeProgramSeen() ? " " : " NOT ") << "DETECTED \n";
    } else {
      dbgs() << "    WHOLE PROGRAM ASSUME IS ENABLED\n";
    }
    dbgs() << "    WHOLE PROGRAM SAFE: " << (WholeProgramSafe ? " " : " NOT ")
           << "DETECTED\n";
  };

  bool Simple = !WholeProgramTraceLibFuncs && !WholeProgramTraceVisibility &&
                !WholeProgramReadTrace;

  bool Full = (WholeProgramTraceLibFuncs && WholeProgramTraceVisibility &&
               WholeProgramReadTrace);

  dbgs() << "\nWHOLE-PROGRAM-ANALYSIS: ";

  // Just the missing libfuncs or those functions that are external
  if (Simple)
    dbgs() << "SIMPLE ANALYSIS";

  // Print all
  else if (Full)
    dbgs() << "FULL ANALYSIS";

  // Print the libfuncs found and not found
  else if (WholeProgramTraceLibFuncs)
    dbgs() << "LIBRARY FUNCTIONS TRACE";

  // Print the functions that weren't internalized
  else if (WholeProgramTraceVisibility)
    dbgs() << "EXTERNAL FUNCTIONS TRACE";

  else if (WholeProgramReadTrace)
    dbgs() << "WHOLE PROGRAM READ TRACE";

  dbgs() << "\n\n";

  if (Simple || Full) {
    dbgs() << "  UNRESOLVED CALLSITES: " << UnresolvedCallsCount << "\n";
  }

  if (Simple || Full)
    PrintAliases();

  if (Full || WholeProgramTraceLibFuncs) {
    dbgs() << "  TOTAL LIBFUNCS: "
           << LibFuncsNotFound.size() + LibFuncsFound.size() << "\n";
    PrintLibFuncsFound();
  }

  if (Simple || Full || WholeProgramTraceLibFuncs)
    PrintLibFuncsNotFound();

  if (Simple || Full || WholeProgramTraceVisibility)
    PrintVisibility();

  if (Full || WholeProgramReadTrace)
    WPUtils->PrintSymbolsResolution();

  PrintWPResult();
}
#endif // USING_DEBUG_MODE

void WholeProgramInfo::analyzeModule() {

  wholeProgramAllExternsAreIntrins();

  computeIsAdvancedOptEnabled();
  computeIsLibIRCAllowedEverywhere();
}

// This analysis depends on TargetLibraryInfo and TargetTransformInfo.
// Analysis info is not modified by any other pass.
void WholeProgramWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.addRequired<TargetTransformInfoWrapperPass>();
}

// Collect the following globals from the Module M and store them in the
// SetVector LLVMSpecialGlobalVars:
//
//   llvm.used:          Treat the symbol as if it needs to be retained in the
//                       module, even if it appears it could be removed.
//   llvm.compiler.used: Same as llvm.used but the compiler shouldn't touch the
//                         function
//   llvm.global_ctors:  Everything executed before main
//   llvm.global_dtors:  Everything executed after main
//
// These globals are arrays that contain Functions needed by the whole program
// analysis but we can't reach them from main.
void WholeProgramInfo::collectLLVMSpecialGlobalVars(
    SetVector<const GlobalVariable *> &LLVMSpecialGlobalVars) {

  auto CollectGlobal = [this, &LLVMSpecialGlobalVars](StringRef GlobName) {
    if (const GlobalVariable *Glob = M->getGlobalVariable(GlobName))
      LLVMSpecialGlobalVars.insert(Glob);
  };

  CollectGlobal("llvm.used");
  CollectGlobal("llvm.compiler.used");
  CollectGlobal("llvm.global_ctors");
  CollectGlobal("llvm.global_dtors");
}

// Return true if traversing the users, recursively, of the input value V we
// can reach a function stored in FuncsCollected or a global variable stored in
// LLVMSpecialGlobalVars. In simple words, we are going to check if the
// functions in FuncsCollected or if the global variables in
// LLVMSpecialGlobalVars will use the Value V indirectly.
//
// The set ValueChecked is for storing data that has been checked already to
// prevent infinite loops.
static bool
hasMeaningfulUse(const Value *V, SetVector<const Function *> &FuncsCollected,
                  SetVector<const Value *> &ValueChecked,
                  SetVector<const GlobalVariable *> &LLVMSpecialGlobalVars) {
  if (!V)
    return false;

  if (V->user_empty())
    return false;

  if (!ValueChecked.insert(V))
    return false;

  for (const User *U : V->users()) {

    const Value *Val = cast<const Value>(U);
    if (ValueChecked.count(Val) > 0)
      continue;

    // If there is an instruction then collect the function and check it
    if (auto *Inst = dyn_cast<const Instruction>(U)) {
      const Function *F = Inst->getFunction();
      if (FuncsCollected.count(F) > 0)
        return true;

      const Value *FuncVal = cast<const Value>(F);
      if (hasMeaningfulUse(FuncVal, FuncsCollected, ValueChecked,
                            LLVMSpecialGlobalVars))
        return true;

      continue;
    } else if (auto *Glob = dyn_cast<const GlobalVariable>(U)) {
      // Check if the User is one of the special global variables
      if (LLVMSpecialGlobalVars.count(Glob) > 0)
        return true;
    }

    if (hasMeaningfulUse(Val, FuncsCollected, ValueChecked,
                          LLVMSpecialGlobalVars))
      return true;
  }

  return false;
}

// Return true if the input function is one of the following:
//
//   * is main (or one of its form)
//   * is not external (internalized symbol and is not main)
//   * is a library function
//   * is a linker added symbol (will be treated as libfunc)
//   * is an intrinsic
//   * is a branch funnel
//
// Else, return false.
bool WholeProgramInfo::isValidFunction(const Function *F) {

#ifdef USING_DEBUG_MODE
  // Store the input function in the corresponding SetVector for
  // debugging purposes:
  //
  //   * LibFuncsFound: the function was found in the Target Library
  //                    Analysis (LibFunc), is a known LLVM intrinsic
  //                    function or is a branch funnel
  //   * LibFuncsNotFound: a declaration that wasn't resolved
  //   * VisibleFunctions: a function with IR that wasn't internalized and
  //                       isn't a LibFunc
  auto UpdateWPDebugResults = [this, F](bool Result) {
    if (Result) {
      LibFuncsFound.insert(F);
    } else {
      if (F->isDeclaration())
        LibFuncsNotFound.insert(F);
      else
        VisibleFunctions.insert(F);
      UnresolvedCallsCount++;
    }
  };
#endif // USING_DEBUG_MODE

  // Branch funnel functions are wrappers to the branch funnel intrinsics.
  // These are created during devirtualization. The purpose of the branch
  // funnel intrinsic is to contain all the possible targets for a virtual
  // call, which could be used in a manner similar to multiversioning. Then
  // the virtual call will be replaced with a direct call to the wrapper
  // function. This is an example of a branch funnel wrapper function:
  //
  //  define hidden void @__typeid__ZTS4Base_0_branch_funnel(i8* nest, ...) {
  //    musttail call void (...)
  //    @llvm.icall.branch.funnel(i8* %0, i8* getelementptr
  //       (i8, i8* bitcast ([3 x i8*]* @_ZTV7Derived.0 to i8*), i64 16),
  //        i1 (%class.Derived*)* @_ZN7Derived3fooEv,
  //        i8* getelementptr (i8, i8* bitcast ([3 x i8*]* @_ZTV8Derived2.0 to
  //        i8*), i64 16), i1 (%class.Derived2*)* @_ZN8Derived23fooEv, ...)
  //    ret void
  //  }
  //
  // The linkage type for these functions is ExternalLinkage. We can treat
  // these wrapper functions as libfuncs since they only wrap an intrinsic.
  // Also, if devirtualization ran then it means that we achieved whole
  // program already and all the functions are internal to the module during
  // LTO.
  auto IsBranchFunnelFunc = [F]() {
    // If there is no IR then there is nothing to check
    if (F->isDeclaration())
      return false;

    // Return type will always be void
    if (!F->getReturnType()->isVoidTy())
      return false;

    // Only one basic block
    if (F->size() != 1)
      return false;

    const BasicBlock &BB = F->front();

    // Branch funnels contains only 2 instructions:
    //   call to intrinsic
    //   return void
    if (BB.size() != 2)
      return false;

    const CallBase *FunnelCall = dyn_cast<CallBase>(&(BB.front()));
    const ReturnInst *Ret = dyn_cast<ReturnInst>(&(BB.back()));

    if (!FunnelCall || !Ret)
      return false;

    // Check that the return type is void
    if (!Ret->getType()->isVoidTy())
      return false;

    const Function *Func = FunnelCall->getCalledFunction();

    if (!Func || !Func->isIntrinsic())
      return false;

    // Calls the intrinsic to branch funnel
    if (Func->getIntrinsicID() == llvm::Intrinsic::icall_branch_funnel)
      return true;

    return false;
  };

  // The only symbols that should be external are main, library functions
  // (LibFuncs) or special symbols added by the linker.
  if (F->hasLocalLinkage())
    return true;

  StringRef SymbolName = F->getName();
  if (WPUtils->isLinkerAddedSymbol(SymbolName))
    return true;

  if (WPUtils->isMainEntryPoint(SymbolName))
    return true;

  bool FuncResolved = false;
  LibFunc TheLibFunc;
  const TargetLibraryInfo &TLI = GetTLI(*const_cast<Function *>(F));
  if ((TLI.getLibFunc(F->getName(), TheLibFunc) && TLI.has(TheLibFunc)) ||
      F->isIntrinsic() || IsBranchFunnelFunc())
    FuncResolved = true;
  else if (!F->isDeclaration())
    WholeProgramHidden = false;

#ifdef USING_DEBUG_MODE
  UpdateWPDebugResults(FuncResolved);
#endif // USING_DEBUG_MODE

  // Function F is a function that needs to be resolved
  return FuncResolved;
}

// Walk the instructions in Function F and store the callees in CallsitesFuncs.
// The TargetLibraryInfo GetTLI is used to check if the called functions are
// LibFuncs or not.
bool WholeProgramInfo::collectAndResolveCallSites(
    const Function *F, std::queue<const Function *> &CallsitesFuncs) {

  // Return true if the input Callee is a valid function for whole program,
  // else return false.
  auto CheckCallee = [this, F](const Function *Callee) -> bool {
    bool FuncResolved = isValidFunction(Callee);

    //
    // If the Caller is not Fortran, but the Callee is Fortran, do not
    // recognize this as a call to a libFunc. In other words, a libFunc
    // that is marked as Fortran (like those from the Fortran runtime
    // library) can only be recognized as a Fortran libFunc if it is called
    // by a Caller that was compiled by the Fortran front end.
    //
    // This mimics the behavior of the IL0 compiler, where only the Fortran
    // front end could recognize calls to the Fortran runtime library as
    // intrinsics.
    //
    if (FuncResolved && Callee->isFortran() && !F->isFortran()) {
#ifdef USING_DEBUG_MODE
      LibFuncsFound.remove(Callee);

      if (Callee->isDeclaration())
        LibFuncsNotFound.insert(Callee);
      else
        VisibleFunctions.insert(Callee);

      ++UnresolvedCallsCount;
#endif // USING_DEBUG_MODE
      return false;
    }

    return FuncResolved;
  };

  bool AllCallSitesResolved = true;

  for (auto &II : instructions(F)) {
    const CallBase *CS = dyn_cast<CallBase>(&II);
    if (!CS)
      continue;

    const Value *Arg = CS->getCalledOperand()->stripPointerCasts();

    if (const Function *Callee = dyn_cast<const Function>(Arg)) {
      if (!Callee->isDeclaration())
        CallsitesFuncs.push(Callee);

      AllCallSitesResolved &= CheckCallee(Callee);

#ifndef USING_DEBUG_MODE
      // If we are not debugging then we can return early
      if (!AllCallSitesResolved)
        return false;
#endif // USING_DEBUG_MODE
    }
  }

  return AllCallSitesResolved;
}

// Return true if all Functions in the Module M were resolved, else return
// false. We recognize a function as "resolved" if it has one of the
// following properties:
//
//   * is main (or one of its form)
//   * is not external (internalized symbol and is not main)
//   * is a library function
//   * is a linker added symbol (will be treated as libfunc)
//   * is an intrinsic
//   * is a branch funnel
//
// Else return false.
bool WholeProgramInfo::analyzeAndResolveFunctions() {

  // Given a Function F, return true if it can be resolved. Also collect
  // the called functions and check if they can be resolved too. These called
  // functions will be stored in FuncsCollected. Else return false.
  auto AnalyzeFunction =
      [this](const Function *F,
                     SetVector<const Function *> &FuncsCollected) -> bool {
    bool AllResolved = true;
    if (!isValidFunction(F)) {
      AllResolved = false;
#ifndef USING_DEBUG_MODE
      return false;
#endif // USING_DEBUG_MODE
    }

    std::queue<const Function *> CallsitesFuncs;
    CallsitesFuncs.push(F);

    while (!CallsitesFuncs.empty()) {
      const Function *CurrFunc = CallsitesFuncs.front();
      CallsitesFuncs.pop();

      if (!FuncsCollected.insert(CurrFunc))
        continue;

      if (CurrFunc->isDeclaration())
        continue;

      AllResolved &=
          collectAndResolveCallSites(CurrFunc, CallsitesFuncs);
#ifndef USING_DEBUG_MODE
      // If we are not debugging then we can return early
      if (!AllResolved)
        return false;
#endif // USING_DEBUG_MODE
    }
    return AllResolved;
  };

  bool Resolved = true;
  MainDefSeen = false;
#ifdef USING_DEBUG_MODE
  UnresolvedCallsCount = 0;
#endif // USING_DEBUG_MODE

  SetVector<const Function *> AddressTakenFuncs;
  const Function *MainFunc = getMainFunction();

  // If main wasn't found then there is no need to keep checking.
  if (MainFunc)
    MainDefSeen = true;
  else
    return false;

  // Find the special global variables
  collectLLVMSpecialGlobalVars(LLVMSpecialGlobalVars);

  // Collect all the functions that can be reached by walking main
  Resolved &= AnalyzeFunction(MainFunc, FuncsCollected);

#ifndef USING_DEBUG_MODE
  // Return early if we are not debugging
  if (!Resolved)
    return false;
#endif // USING_DEBUG_MODE

  // Find all address taken functions that we missed when main was checked
  // (indirect calls).
  for (auto &F : *M)
    if (F.hasAddressTaken() && !F.user_empty() && FuncsCollected.count(&F) == 0)
      AddressTakenFuncs.insert(&F);

  // Check which address taken functions can reach any function in
  // FuncsCollected or the global variables in LLVMSpecialGlobalVars by
  // traversing the users.
  bool KeepWalking = true;
  while (KeepWalking) {
    unsigned OldSize = FuncsCollected.size();
    for (auto *ATFunc : AddressTakenFuncs) {
      if (FuncsCollected.count(ATFunc) > 0)
        continue;

      SetVector<const Value *> DataChecked;
      if (hasMeaningfulUse(ATFunc, FuncsCollected, DataChecked,
                            LLVMSpecialGlobalVars)) {

        Resolved &= AnalyzeFunction(ATFunc, FuncsCollected);
#ifndef USING_DEBUG_MODE
        // Return early if we are not debugging
        if (!Resolved)
          return false;
#endif // USING_DEBUG_MODE
      }
    }
    KeepWalking = OldSize != FuncsCollected.size();
  }

  return Resolved;
}

// Given a Value, check if it is a Function or a GlobalVariable, if so insert
// it in Aliasees. If not, then check if it is a Constant and recurse with
// the Operands as input. The DataChecked vector is used to prevent infinite
// recursion.
static void CollectAliasedInformation(const Value *BaseVal,
                                      SetVector<const Value*> &Aliasees,
                                      SetVector<const Value*> &DataChecked) {

  // Return true if ConstVal is a Function or a GlobalAlias, and insert it
  // in Aliasees. Also, return true if ConstVal is in Aliasees. Else, return
  // false.
  auto CheckValue = [&Aliasees](const Value* ConstVal) -> bool {
    const Value *Val = ConstVal->stripPointerCasts();

    if (Aliasees.count(Val))
      return true;

    if (isa<const Function>(Val) || isa<const GlobalAlias>(Val))
      return Aliasees.insert(Val);

    return false;
  };

  if (!BaseVal)
    return;

  if (!DataChecked.insert(BaseVal))
    return;

  // Check if BaseVal is a Function or a GlobalAlias
  if (CheckValue(BaseVal) || !isa<const Constant>(BaseVal))
    return;

  // Functions and GlobalAliases are Constants. We are just going to traverse
  // through any constant that holds them (array, structure, etc.).
  auto *BaseConst = cast<const Constant>(BaseVal);
  for (unsigned I = 0, E = BaseConst->getNumOperands(); I < E; I++) {
    const Value *OP = BaseConst->getOperand(I);
    CollectAliasedInformation(OP, Aliasees, DataChecked);
  }
}

// Given a GlobalAlias GA, return true if the alias was resolved, else return
// false. An alias is identified as resolved if:
//
//   * All functions in the alias must be internal functions or libfuncs
//   * All aliases in the alias must be resolved too
//   * There is no recursion in the alias
//
// The GlobalAlias OriginalGA is used for checking any recursion (Alias A
// points to Alias B which points to Alias A).
bool WholeProgramInfo::isValidAlias(const GlobalAlias *GA,
                                    const GlobalAlias *OriginalGA) {

  if (!GA || !OriginalGA)
    return false;

  // If the alias was collected before then there is nothing to check
  if (AliasesFound.count(GA))
    return true;

  const Value *BaseObject = cast<const GlobalValue>(GA->getAliaseeObject());

  // Collect what GA is aliasing
  SetVector<const Value*> Aliasees;
  SetVector<const Value*> DataChecked;
  CollectAliasedInformation(BaseObject, Aliasees, DataChecked);

  // If GA or OriginalGA are in the vector of aliasees, then return false.
  // This means that we found some sort of recursion.
  if (Aliasees.count(GA) > 0 || Aliasees.count(OriginalGA))
    return false;

  // Go through all the Functions and GlobalAliases collected.
  for (auto *Aliasee : Aliasees) {
    // A Function must have local linkage with IR or must be a libfunc
    if (auto *Func = dyn_cast<const Function>(Aliasee)) {
      if (!isValidFunction(Func))
        return false;
    }
    else {
      // A GlobalAlias must be resolveed too
      auto *GlobAlias = cast<const GlobalAlias>(Aliasee);
      if (AliasesFound.count(GlobAlias))
        continue;
      if (!isValidAlias(GlobAlias, OriginalGA))
        return false;
    }
  }

  AliasesFound.insert(GA);
  return true;
}

// Return true if all the aliases in the Module were resolved. An alias is
// "resolved" if it has the following properties:
//
//   * aliasees that are functions must be identified as "resolved"
//   * aliasees that are aliases must be resolved
//
// Else, return false.
bool WholeProgramInfo::analyzeAndResolveAliases() {

  // Walk through all aliases to find unresolved aliases.
  bool AllAliasesResolved = true;
  for (auto &GA : M->aliases()) {

    SetVector<const Value *> DataChecked;
    // Skip those aliases that won't be used in the program
    if (!hasMeaningfulUse(&GA, FuncsCollected, DataChecked,
                           LLVMSpecialGlobalVars))
      continue;

    bool AliasResolved = isValidAlias(&GA, &GA);

    if (!AliasResolved) {
#ifdef USING_DEBUG_MODE
      // Store the unresolved aliases if we are debugging
      AliasesNotFound.insert(&GA);
#else
      // Return early if we aren't debugging
      return false;
#endif // USING_DEBUG_MODE
    }

    AllAliasesResolved &= AliasResolved;
  }

  return AllAliasesResolved;
}

// Detect whole program using intrinsic table.
//
void WholeProgramInfo::wholeProgramAllExternsAreIntrins() {

  // If whole program assert is enabled and FunctionsResolved
  // is false then exit with an error message. This is to
  // catch when whole program wasn't achieved.
  auto AssertWholeProgram = [](bool FunctionsResolved) -> void {
    if (!WholeProgramAssert || FunctionsResolved)
      return;

    // NOTE: Although this function is used for asserting if whole program
    // was achieved, it doesn't use an actual c assert to abort the compilation.
    // The issue is that the c assert function calls abort, which produces some
    // messages that could be misleading while debugging. Instead, we are going
    // to report the failure using the errs function and then call exit.
    errs() << "Whole-Program-Analysis: Did not detect whole program\n";
    errs().flush();
    exit(1);
  };

  // This analysis walks the module and checks that all the functions were
  // resolved and all libfuncs are in the libfuncs table.
  bool AllResolved = analyzeAndResolveFunctions();
  AssertWholeProgram(AllResolved);

  // This analysis checks the aliases. There is a chance that an alias is
  // pointing to multiple functions and we didn't reach these functions by
  // walking the module. In this case we need to check all the functions that
  // the alias is pointing to and make sure that they were resolved. If at
  // least one function is not resolved, then there is no whole program.
  // This is common with MS Visual Studio libraries.
  AllResolved &= analyzeAndResolveAliases();
  AssertWholeProgram(AllResolved);

  if (AllResolved)
    WholeProgramSeen = true;

  WholeProgramSafe =
      isWholeProgramSeen() && isWholeProgramRead() && isLinkedAsExecutable();

  LLVM_DEBUG(printWholeProgramTrace(););
}

// Compute and store the value of isAdvancedOptEnabled(AO) for all
// possible values of the argument AO.
void WholeProgramInfo::computeIsAdvancedOptEnabled() {
  for (Function &F : *M) {
    if (F.isDeclaration())
      continue;
    TargetTransformInfo &TTI = GTTI(F);
    auto EE = TargetTransformInfo::AdvancedOptLevel::AO_TargetNumLevels;
    unsigned E = static_cast<unsigned>(EE);
    for (unsigned I = 0; I < E; ++I) {
      auto II = static_cast<TargetTransformInfo::AdvancedOptLevel>(I);
      IsAdvancedOptEnabled[I] &= TTI.isAdvancedOptEnabled(II);
    }
  }

  // How to print this trace: -mllvm -debug-only=whole-program-analysis
  //                          -mllvm -whole-program-advanced-opt-trace
  LLVM_DEBUG({
    if (WholeProgramAdvanceOptTrace) {
      auto &Enabled = IsAdvancedOptEnabled;
      if (Enabled
              [TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelSSE42])
        dbgs() << "Target has Intel SSE42\n";
      if (Enabled[TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX])
        dbgs() << "Target has Intel AVX\n";
      if (Enabled[TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX2])
        dbgs() << "Target has Intel AVX2\n";
      if (Enabled
              [TargetTransformInfo::AdvancedOptLevel::AO_TargetHasGenericAVX2])
        dbgs() << "Target has generic AVX2\n";
      if (Enabled
              [TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX512])
        dbgs() << "Target has Intel AVX512\n";
    }
  });
}

void WholeProgramInfo::computeIsLibIRCAllowedEverywhere() {
  IsLibIRCAllowedEverywhere = true;
  for (Function &F : *M) {
    if (F.isDeclaration())
      continue;
    TargetTransformInfo &TTI = GTTI(F);
    if (!TTI.isLibIRCAllowed()) {
      IsLibIRCAllowedEverywhere = false;
      if (!WholeProgramAdvanceOptTrace)
        return;
      LLVM_DEBUG(dbgs() << "LibIRC not allowed on " << F.getName() << "\n");
    }
  }
}

// Return true if TTI->isAdvancedOptEnabled(AO) is true for all Functions
// in the LTO unit which have IR.
bool WholeProgramInfo::isAdvancedOptEnabled(
    TargetTransformInfo::AdvancedOptLevel AO) {
  return IsAdvancedOptEnabled[AO];
}

// This returns true if AssumeWholeProgram is true or WholeProgramSafe is true.
// WholeProgramSafe is set to true only if all symbols have been resolved
// and building executable (i.e not building shared library).
//
bool WholeProgramInfo::isWholeProgramSafe(void) {
  return WholeProgramSafe || AssumeWholeProgram;
}

// This function returns true if AssumeWholeProgram is turned on, or if
// WholeProgramSeen is true and all symbols are internal to the LTO unit.
bool WholeProgramInfo::isWholeProgramSeen(void) {
  return (WholeProgramSeen && isWholeProgramHidden()) || AssumeWholeProgram;
}

// Return true if all functions with IR are inside the LTO unit except
// main, runtime library calls and library functions (LibFuncs).
bool WholeProgramInfo::isWholeProgramHidden(void) {
  if (AssumeWholeProgramHidden)
    return true;

  return WholeProgramHidden || AssumeWholeProgram;
}

// Return true if the linker finds that all symbols were resolved or
// the assumption flag for whole program read was turned on.
bool WholeProgramInfo::isWholeProgramRead() {
  return WPUtils->getWholeProgramRead() || AssumeWholeProgramRead;
}

// Return true if the linker is generating an executable or the
// assumption flag for executable was turned on.
bool WholeProgramInfo::isLinkedAsExecutable() {
  return WPUtils->getLinkingExecutable() || AssumeWholeProgramExecutable;
}

// Returns 'true' if use of LibIRC is allowed for all functions with IR.
bool WholeProgramInfo::isLibIRCAllowedEverywhere() {
  return IsLibIRCAllowedEverywhere;
}

// Return the Function* that points to "main" or any of its forms,
// else return nullptr
Function *WholeProgramInfo::getMainFunction() {

  Function *Main = nullptr;

  for (const auto &Name : WPUtils->getMainNames()) {
    Main = M->getFunction(Name);

    if (Main)
      break;
  }

  return Main;
}

char WholeProgramAnalysis::PassID;

// Provide a definition for the static class member used to identify passes.
AnalysisKey WholeProgramAnalysis::Key;

WholeProgramInfo WholeProgramAnalysis::run(Module &M,
                                           AnalysisManager<Module> &AM) {
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  std::function<TargetTransformInfo &(Function &)> GTTI =
      [&FAM](Function &F) -> TargetTransformInfo & {
    return FAM.getResult<TargetIRAnalysis>(F);
  };

  auto GetTLI = [&FAM](Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(F);
  };

  WholeProgramInfo WPAResult(&M, GetTLI, GTTI, &WPUtils);
  WPAResult.analyzeModule();

  return WPAResult;
}
