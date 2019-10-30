//===------- Intel_WP.cpp - Whole Program Analysis -*------===//
//
// Copyright (C) 2016-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// This file does whole-program-analysis using TargetLibraryInfo.
//===----------------------------------------------------------------------===//

#include "llvm/ADT/StringSwitch.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/Intel_XmainOptLevelPass.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"

using namespace llvm;
using namespace llvm::llvm_intel_wp_analysis;

#define DEBUG_TYPE "whole-program-analysis"

namespace llvm {
namespace llvm_intel_wp_analysis {
// If it is true, compiler assumes that source files in the current
// compilation have entire program.
cl::opt<bool> AssumeWholeProgram("whole-program-assume",
                                 cl::init(false), cl::ReallyHidden);
} // llvm_intel_wp_analysis
} // llvm

// Flag to print the libfuncs found by whole program analysis.
static cl::opt<bool> WholeProgramTraceLibFuncs("whole-program-trace-libfuncs",
                                       cl::init(false), cl::ReallyHidden);

// Flag for printing the symbols that weren't internalized in the module
static cl::opt<bool> WholeProgramTraceVisibility(
    "whole-program-trace-visibility", cl::init(false), cl::ReallyHidden);

// Flag for printing the symbols resolution found by the linker
static cl::opt<bool> WholeProgramReadTrace(
    "whole-program-read-trace", cl::init(false), cl::ReallyHidden);

// Flag asserts that whole program will be detected.
static cl::opt<bool> WholeProgramAssert("whole-program-assert",
                                        cl::init(false), cl::ReallyHidden);

// If it is true, compiler assumes that whole program has been read (linker can
// reach all symbols).
static cl::opt<bool> AssumeWholeProgramRead("whole-program-assume-read",
                                            cl::init(false), cl::ReallyHidden);

// The compiler will assume that all symbols have hidden visibility if turned on
static cl::opt<bool> AssumeWholeProgramHidden("whole-program-assume-hidden",
                                            cl::init(false), cl::ReallyHidden);

// If it is true, compiler assumes that program is linked as executable.
static cl::opt<bool> AssumeWholeProgramExecutable(
    "whole-program-assume-executable", cl::init(false), cl::ReallyHidden);

// Flag to get whole program advanced optimization computation trace.
static cl::opt<bool>
    WholeProgramAdvanceOptTrace("whole-program-advanced-opt-trace",
                                cl::init(false), cl::ReallyHidden);

INITIALIZE_PASS_BEGIN(WholeProgramWrapperPass, "wholeprogramanalysis",
                "Whole program analysis", false, false)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(XmainOptLevelWrapperPass)
INITIALIZE_PASS_END(WholeProgramWrapperPass, "wholeprogramanalysis",
                "Whole program analysis", false, false)

char WholeProgramWrapperPass::ID = 0;

ModulePass *llvm::createWholeProgramWrapperPassPass() {
  return new WholeProgramWrapperPass();
}

WholeProgramWrapperPass::WholeProgramWrapperPass() : ModulePass(ID) {
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

  CallGraphWrapperPass *CGPass =
      getAnalysisIfAvailable<CallGraphWrapperPass>();
  CallGraph *CG = CGPass ? &CGPass->getCallGraph() : nullptr;

  // NOTE: The old pass manager uses two variables to represent the
  // optimization levels:
  //
  //   - OptLevel: stores the optimization level
  //               0 = -O0, 1 = -O1, 2 = -O2, 3 = -O3
  //
  //   - SizeLevel: stores if we are optimizing for size
  //               0 = no, 1 = Os, 2 = Oz
  //
  // The values of OptLevel can be 0, 1, 2 or 3.
  unsigned OptLevel = getAnalysis<XmainOptLevelWrapperPass>().getOptLevel();

  auto GetTLI = [this](Function &F) -> TargetLibraryInfo & {
    return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
  };

  Result.reset(new WholeProgramInfo(
               WholeProgramInfo::analyzeModule(M, GetTLI, GTTI,
               CG, OptLevel)));

  return false;
}

WholeProgramInfo::WholeProgramInfo() {
  WholeProgramSafe = false;
  WholeProgramSeen = false;
  auto EE = TargetTransformInfo::AdvancedOptLevel::AO_TargetNumLevels;
  unsigned E = static_cast<unsigned>(EE);
  for (unsigned I = 0; I < E; ++I)
    IsAdvancedOptEnabled[I] = true;
}

WholeProgramInfo::~WholeProgramInfo() {}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
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
      dbgs() << "  WHOLE PROGRAM" << (isWholeProgramSeen() ? " " : " NOT ")
             << "DETECTED \n";
      dbgs() << "  WHOLE PROGRAM SAFE is"
             << (WholeProgramSafe ? " " : " *NOT* ")
             << "determined:\n";

      if (AssumeWholeProgram) {
        dbgs() << "whole-program-assume is enabled ... \n";
      }
      else {
        if (!isWholeProgramSeen())
          dbgs() <<  "    whole program not seen;\n";
        if (!isWholeProgramRead())
          dbgs() <<  "    whole program not read;\n";
        if (!isLinkedAsExecutable())
          dbgs() <<  "    not linking an executable;\n";
      }
    };

    bool Simple = !WholeProgramTraceLibFuncs &&
                  !WholeProgramTraceVisibility &&
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
      dbgs() << "WHOLE PROGRAM READ\n";

    dbgs() << "\n\n";

    if (Simple || Full) {
      dbgs() << "  Main definition" << (MainDefSeen ? " " : " not ")
             << "seen \n";
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
      WPUtils.PrintSymbolsResolution();

    PrintWPResult();
}
#endif // NDEBUG || LLVM_ENABLE_DUMP

// Traverse through the IR and replace the calls to the intrinsic
// llvm.intel.wholeprogramsafe with true if whole program safe was
// detected. Else, replace the calls with a false. The intrinsic
// llvm.intel.wholeprogramsafe should be removed completely after
// this process since it won't be lowered. See the language reference
// manual for more information.
void WholeProgramInfo::foldIntrinsicWholeProgramSafe(Module &M,
                                                     unsigned OptLevel) {

  Function *WhPrIntrin = M.getFunction("llvm.intel.wholeprogramsafe");

  if (!WhPrIntrin)
    return;

  LLVMContext &Context = M.getContext();

  // If the optimization level is 0 then we are going to take the path
  // when whole program is not safe. This means that any optimization
  // wrapped in the intrinsic llvm.intel.wholeprogramsafe won't be
  // applied (e.g. devirtualization).
  ConstantInt *InitVal = ((isWholeProgramSafe() && OptLevel > 0) ?
                          ConstantInt::getTrue(Context) :
                          ConstantInt::getFalse(Context));

  while (!WhPrIntrin->use_empty()){
    // The intrinsic llvm.intel.wholeprogramsafe is only supported for
    // CallInst instructions. The only intrinsics that are allowed in
    // InvokeInst are: donothing, patchpoint, statepoint, coro_resume
    // and coro_destroy.
    CallInst *IntrinCall = cast<CallInst>(WhPrIntrin->user_back());
    IntrinCall->replaceAllUsesWith(InitVal);
    IntrinCall->eraseFromParent();
  }

  assert(WhPrIntrin->use_empty() && "Whole Program Analysis: intrinsic"
          " llvm.intel.wholeprogramsafe wasn't removed correctly.");

  WhPrIntrin->eraseFromParent();

  assert(!M.getFunction("llvm.intel.wholeprogramsafe") &&
          "Whole Program Analysis: intrinsic llvm.intel.wholeprogramsafe"
          " wasn't removed correctly.");
}

WholeProgramInfo WholeProgramInfo::analyzeModule(
    Module &M,
    std::function<const TargetLibraryInfo &(Function &)> GetTLI,
    function_ref<TargetTransformInfo &(Function &)> GTTI, CallGraph *CG,
    unsigned OptLevel) {

  WholeProgramInfo Result;

  Result.wholeProgramAllExternsAreIntrins(M, GetTLI);

  // Remove the uses of the intrinsic
  // llvm.intel.wholeprogramsafe
  Result.foldIntrinsicWholeProgramSafe(M, OptLevel);
  Result.computeIsAdvancedOptEnabled(M, GTTI);

  return Result;
}

// This analysis depends on TargetLibraryInfo and TargetTransformInfo.
// Analysis info is not modified by any other pass.
void WholeProgramWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.addRequired<TargetTransformInfoWrapperPass>();
  AU.addRequired<XmainOptLevelWrapperPass>();
}

// This function takes Value that is an operand to call or invoke instruction
// and checks if it can be resolved as a known function.
bool WholeProgramInfo::resolveCalledValue(
    std::function<const TargetLibraryInfo &(Function &)> GetTLI,
    const Value *Arg, const Function *Caller) {

  assert(Arg && "Whole-Program-Analysis: invalid call argument");
  // Remove any potential cast before doing anything with the value.
  Arg = Arg->stripPointerCasts();

  // If the value is a function try to resolve it.
  if (const Function *Callee = dyn_cast<Function>(Arg)) {
    if (Callee->isIntrinsic() || !Callee->isDeclaration()) {
      LLVM_DEBUG({
        if (WholeProgramTraceLibFuncs && Callee->isIntrinsic())
          LibFuncsFound.insert(Callee);
      });
      return true;
    }

    const TargetLibraryInfo &TLI = GetTLI(*const_cast<Function*>(Callee));

    LibFunc TheLibFunc;
    if (!TLI.getLibFunc(Callee->getName(), TheLibFunc) ||
        !TLI.has(TheLibFunc)) {

      LLVM_DEBUG({
        LibFuncsNotFound.insert(Callee);
        ++UnresolvedCallsCount;
      });
      return false;
    }

    LLVM_DEBUG({
      if (WholeProgramTraceLibFuncs)
        LibFuncsFound.insert(Callee);
    });
    return true;
  }

  // Callees which are result of instructions like load cannot be resolved at
  // compile time, ignore them. If the whole program is read we can assume that
  // indirect calls will invoke one of the analyzed functions.
  return true;
}

// This function returns true if all calls in "F" can be resolved using
// "TLI" info. Otherwise, it returns false and sets "unresolved_funcs_count"
// to number of unresolved calls in "F".
bool WholeProgramInfo::resolveCallsInRoutine(
    std::function<const TargetLibraryInfo &(Function &)> GetTLI,
    Function *F) {

  bool Resolved = true;
  for (auto &II : instructions(F)) {
    // Skip if it is not a call inst
    if (!isa<CallInst>(&II) && !isa<InvokeInst>(&II)) {
      continue;
    }

    CallBase *CS = dyn_cast<CallBase>(&II);
    Resolved &= resolveCalledValue(GetTLI, CS->getCalledValue(), F);

    LLVM_DEBUG({
      // If we are printing the trace for whole program analysis then
      // we need to traverse the whole module.
      continue;
    });

    if (!Resolved)
      return false;
  }
  return Resolved;
}

bool WholeProgramInfo::resolveAllLibFunctions(
    Module &M,
    std::function<const TargetLibraryInfo &(Function &)> GetTLI) {

  bool Resolved = true;
  MainDefSeen = false;

  LLVM_DEBUG({
    UnresolvedCallsCount = 0;
  });

  // Walk through all functions to find unresolved calls.
  LibFunc TheLibFunc;
  for (Function &F : M) {
    if (!F.isDeclaration() && WPUtils.isMainEntryPoint(F.getName()))
      MainDefSeen = true;

    // First check if the current function has local linkage (IR),
    // it is a LibFunc or an intrinsic
    if (!AssumeWholeProgram && F.isDeclaration() && !F.hasLocalLinkage()) {

      const TargetLibraryInfo &TLI = GetTLI(F);

      bool LibFuncFound = F.isIntrinsic() ||
                          (TLI.getLibFunc(F.getName(), TheLibFunc) &&
                           TLI.has(TheLibFunc));
      // Check if the current function is a LibFunc or an intrinsic
      if (!LibFuncFound) {
        LLVM_DEBUG({
          LibFuncsNotFound.insert(&F);
        });
        Resolved &= false;
      }
      // We can skip because there is no IR
      continue;
    }

    Resolved &= resolveCallsInRoutine(GetTLI, &F);
  }

  // Walk through all aliases to find unresolved aliases.
  for (auto &GA : M.aliases()) {
    if (GA.hasLocalLinkage())
      continue;
    LLVM_DEBUG({
      AliasesNotFound.insert(&GA);
    });
    Resolved &= false;
  }

  // Check for Resolved if WholeProgramAssert is true.
  // NOTE: This is not an actual c assert, this is just an early exit.
  // The assert function calls abort and produces some messages that could
  // be misleading. Instead, we are going to use report_fatal_error.
  if (WholeProgramAssert && !Resolved)
    report_fatal_error("Whole-Program-Analysis: Did not detect whole program");

  Resolved &= MainDefSeen;

  return Resolved;
}

// Compute if all functions in the module M that have at least one User are
// internal with the exception of libfuncs, main and functions added by the
// linker. The visibility is used to secure that all the functions are inside
// the module.
//
void WholeProgramInfo::computeFunctionsVisibility(
    Module &M,
    std::function<const TargetLibraryInfo &(Function &)> GetTLI){
  LibFunc TheLibFunc;

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
  auto IsBranchFunnelFunc = [](Function &F) {

    // If there is no IR then there is nothing to check
    if (F.isDeclaration())
      return false;

    // Return type will always be void
    if (!F.getReturnType()->isVoidTy())
      return false;

    // Only one basic block
    if (F.size() != 1)
      return false;

    BasicBlock &BB = F.front();

    // Branch funnels contains only 2 instructions:
    //   call to intrinsic
    //   return void
    if (BB.size() != 2)
      return false;

    CallBase *FunnelCall = dyn_cast<CallBase>(&(BB.front()));
    ReturnInst *Ret = dyn_cast<ReturnInst>(&(BB.back()));

    if (!FunnelCall || !Ret)
      return false;

    // Check that the return type is void
    if (!Ret->getType()->isVoidTy())
      return false;

    Function *Func = FunnelCall->getCalledFunction();

    if (!Func || !Func->isIntrinsic())
      return false;

    // Calls the intrinsic to branch funnel
    if (Func->getIntrinsicID() == llvm::Intrinsic::icall_branch_funnel)
      return true;

    return false;
  };

  for (Function &F : M) {

    if (!F.hasLocalLinkage()) {

      // If there isn't any user then it means that the function is
      // not needed.
      if (F.user_empty())
        continue;

      StringRef SymbolName = F.getName();

      bool IsLinkerAddedSymbol = WPUtils.isLinkerAddedSymbol(SymbolName);

      bool IsMain = WPUtils.isMainEntryPoint(SymbolName);

      const TargetLibraryInfo &TLI = GetTLI(F);

      bool IsLibFunc = F.isIntrinsic() ||
                       (TLI.getLibFunc(F.getName(), TheLibFunc) &&
                        TLI.has(TheLibFunc)) || IsBranchFunnelFunc(F);

      // A whole program hidden symbol will be one or more of the following:
      //   * is main (or one of it's form)
      //   * is not external (internalized symbol and is not main)
      //   * is a library function
      //   * is a linker added symbol (will be treated as libfunc)
      bool IsWPHiddenSymbol = IsLinkerAddedSymbol || IsLibFunc || IsMain;

      // The only symbols that should be external are main, library functions
      // (LibFuncs) or special symbols added by the linker.
      if (!IsWPHiddenSymbol)
        VisibleFunctions.insert(&F);
    }
  }
}

// Detect whole program using intrinsic table.
//
void WholeProgramInfo::wholeProgramAllExternsAreIntrins(
    Module &M,
    std::function<const TargetLibraryInfo &(Function &)> GetTLI) {

  // Compute if all functions are internal
  computeFunctionsVisibility(M, GetTLI);
  bool AllResolved = resolveAllLibFunctions(M, GetTLI);

  if (AllResolved)
    WholeProgramSeen = true;

  WholeProgramSafe = isWholeProgramSeen() && isWholeProgramRead()
                     && isLinkedAsExecutable();

  LLVM_DEBUG( printWholeProgramTrace(); );
}

// Compute and store the value of isAdvancedOptEnabled(AO) for all
// possible values of the argument AO.
void WholeProgramInfo::computeIsAdvancedOptEnabled(Module &M,
                       function_ref<TargetTransformInfo &(Function &)> GTTI) {
  for (Function &F : M) {
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
      if (Enabled[TargetTransformInfo::AdvancedOptLevel::AO_TargetHasSSE42])
        dbgs() << "Target has SSE42\n";
      if (Enabled[TargetTransformInfo::AdvancedOptLevel::AO_TargetHasAVX])
        dbgs() << "Target has AVX\n";
      if (Enabled[TargetTransformInfo::AdvancedOptLevel::AO_TargetHasAVX2])
        dbgs() << "Target has AVX2\n";
      if (Enabled[TargetTransformInfo::AdvancedOptLevel::AO_TargetHasAVX512])
        dbgs() << "Target has AVX512\n";
    }
  });
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

// Return true if all symbols are inside the LTO unit except
// main, runtime library calls and library functions (LibFuncs).
bool WholeProgramInfo::isWholeProgramHidden(void) {
  if (AssumeWholeProgramHidden)
    return true;

  return VisibleFunctions.empty() || AssumeWholeProgram;
}

// Return true if the linker finds that all symbols were resolved or
// the assumption flag for whole program read was turned on.
bool WholeProgramInfo::isWholeProgramRead() {
  return WPUtils.getWholeProgramRead() || AssumeWholeProgramRead;
}

// Return true if the linker is generating an executable or the
// assumption flag for executable was turned on.
bool WholeProgramInfo::isLinkedAsExecutable() {
  return WPUtils.getLinkingExecutable() || AssumeWholeProgramExecutable;
}

// Return the Function* that points to "main" or any of its forms,
// else return nullptr
Function* WholeProgramInfo::getMainFunction(Module &M) {

  Function *Main = nullptr;

  for (auto Name : WPUtils.getMainNames()) {
    Main = M.getFunction(Name);

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

  // NOTE: The new pass manager uses an enum to represent the
  // optimization levels:
  //
  //   - PassBuilder::OptimizationLevel
  //
  //     PassBuilder::OptimizationLevel::O0 = -O0
  //     PassBuilder::OptimizationLevel::O1 = -O1
  //     PassBuilder::OptimizationLevel::O2 = -O2
  //     PassBuilder::OptimizationLevel::O3 = -O3
  //     PassBuilder::OptimizationLevel::Os = -Os
  //     PassBuilder::OptimizationLevel::Oz = -Oz
  //
  // The values of OptLevel can be 0, 1, 2, 3, 4 or 5. The options -Os and
  // -Oz are optimization level but for size. The compiler will treat these
  // two levels as -O2 but without increasing the size of the code. The main
  // difference between -Os and -Oz is that the second one does more aggressive
  // optimizations related to size.
  unsigned OptLevel = AM.getResult<XmainOptLevelAnalysis>(M).getOptLevel();

  auto GetTLI = [&FAM](Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(F);
  };

  return WholeProgramInfo::analyzeModule(M, GetTLI, GTTI,
                                  AM.getCachedResult<CallGraphAnalysis>(M),
                                  OptLevel);
}
