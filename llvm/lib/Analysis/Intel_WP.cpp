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
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"

using namespace llvm;
using namespace llvm::llvm_intel_wp_analysis;

namespace llvm {
namespace llvm_intel_wp_analysis {
// If it is true, compiler assumes that source files in the current
// compilation have entire program.
cl::opt<bool> AssumeWholeProgram("whole-program-assume",
                                 cl::init(false), cl::ReallyHidden);
}
}

// Flag to get whole program analysis trace.
static cl::opt<bool> WholeProgramTrace("whole-program-trace",
                                       cl::init(false), cl::ReallyHidden);

// Flag to print the libfuncs found by whole program analysis.
static cl::opt<bool> WholeProgramTraceLibFuncs("whole-program-trace-libfuncs",
                                       cl::init(false), cl::ReallyHidden);

// Flag for printing the unresolved symbols
static cl::opt<bool> WholeProgramTraceSymbols("whole-program-trace-symbols",
                                       cl::init(false), cl::ReallyHidden);

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

// True if the LTO process finds that all symbols are inside the LTO unit.
// The only symbols that will be treated as externals are main and those
// that are in the RuntimeLibcalls table.
static bool HiddenVisibility = false;

// SetVector for storing the symbols that are visible to regular objects.
// These symbols might have IR in the summary section, but the LTO
// visibility analysis found that there might be a none-LTO unit that
// are accessing them.
static SetVector<StringRef> VisibleSymbolsVector;

#define DEBUG_TYPE  "wholeprogramanalysis"

INITIALIZE_PASS_BEGIN(WholeProgramWrapperPass, "wholeprogramanalysis",
                "Whole program analysis", false, false)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_END(WholeProgramWrapperPass, "wholeprogramanalysis",
                "Whole program analysis", false, false)

// Store if all symbols have hidden visibility. Called during LTO
// symbols resolution.
void llvm::setVisibilityHidden(bool AllSymbolsHidden) {
  HiddenVisibility = AllSymbolsHidden;
}

// Store the input symbol name in the VisibleSymbolsVector. These
// symbols will be printed during the whole program analysis trace.
void llvm::storeVisibleSymbols(StringRef SymbolName) {
  if (WholeProgramTrace)
    VisibleSymbolsVector.insert(SymbolName);
}

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
  Result.reset(new WholeProgramInfo(
                WholeProgramInfo::analyzeModule(
      M, getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(), GTTI, CG)));
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

// Traverse through the IR and replace the calls to the intrinsic
// llvm.intel.wholeprogramsafe with true if whole program safe was
// detected. Else, replace the calls with a false. The intrinsic
// llvm.intel.wholeprogramsafe should be removed completely after
// this process since it won't be lowered. See the language reference
// manual for more information.
void WholeProgramInfo::foldIntrinsicWholeProgramSafe(Module &M) {

  Function *WhPrIntrin = M.getFunction("llvm.intel.wholeprogramsafe");

  if (!WhPrIntrin)
    return;

  LLVMContext &Context = M.getContext();

  ConstantInt *InitVal = (isWholeProgramSafe()?
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

WholeProgramInfo
WholeProgramInfo::analyzeModule(Module &M, const TargetLibraryInfo &TLI,
                                function_ref<TargetTransformInfo
                                    &(Function &)> GTTI, CallGraph *CG) {

  WholeProgramInfo Result;

  Result.wholeProgramAllExternsAreIntrins(M, TLI);

  if (AssumeWholeProgram) {
    if (WholeProgramTrace)
      errs() << "whole-program-assume is enabled ... \n";

    Result.WholeProgramSeen = true;
  }

  // Remove the uses of the intrinsic
  // llvm.intel.wholeprogramsafe
  Result.foldIntrinsicWholeProgramSafe(M);
  Result.computeIsAdvancedOptEnabled(M, GTTI);

  return Result;
}

// This analysis depends on TargetLibraryInfo and TargetTransformInfo.
// Analysis info is not modified by any other pass.
void WholeProgramWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.addRequired<TargetTransformInfoWrapperPass>();
}

// This function takes Value that is an operand to call or invoke instruction
// and checks if it can be resolved as a known function.
bool WholeProgramInfo::resolveCalledValue(
    const TargetLibraryInfo &TLI, const Value *Arg, const Function *Caller) {
  assert(Arg && "Whole-Program-Analysis: invalid call argument");
  // Remove any potential cast before doing anything with the value.
  Arg = Arg->stripPointerCasts();

  // If the value is a function try to resolve it.
  if (const Function *Callee = dyn_cast<Function>(Arg)) {
    if (Callee->isIntrinsic() || !Callee->isDeclaration()) {
      if (WholeProgramTraceLibFuncs && Callee->isIntrinsic())
        LibFuncsFound.insert(Callee);
      return true;
    }

    LibFunc TheLibFunc;
    if (!TLI.getLibFunc(Callee->getName(), TheLibFunc) ||
        !TLI.has(TheLibFunc)) {

      if (WholeProgramTrace || WholeProgramTraceLibFuncs)
        LibFuncsNotFound.insert(Callee);

      ++UnresolvedCallsCount;
      return false;
    }
    // Libfunc found, return true
    if (WholeProgramTraceLibFuncs)
      LibFuncsFound.insert(Callee);
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
bool WholeProgramInfo::resolveCallsInRoutine(const TargetLibraryInfo &TLI,
                                             Function *F) {
  bool Resolved = true;
  for (auto &II : instructions(F)) {
    // Skip if it is not a call inst
    if (!isa<CallInst>(&II) && !isa<InvokeInst>(&II)) {
      continue;
    }

    CallBase *CS = dyn_cast<CallBase>(&II);
    Resolved &= resolveCalledValue(TLI, CS->getCalledValue(), F);
    if (!Resolved && !WholeProgramTrace)
      return false;
  }
  return Resolved;
}

// Return true if the input StringRef represents any form of
// main. Else return false.
bool WholeProgramInfo::isMainEntryPoint(llvm::StringRef GlobName) {

  return llvm::StringSwitch<bool>(GlobName)
      .Cases("main",
             "MAIN__",
             "wmain",
             "WinMain",
             "wWinMain",
             "DllMain",
             true)
      .Default(false);

}

bool WholeProgramInfo::resolveAllLibFunctions(Module &M,
                                       const TargetLibraryInfo &TLI) {
  bool all_resolved = true;
  bool main_def_seen_in_ir = false;
  int unresolved_globals_count = 0;
  int unresolved_aliases_count = 0;

  UnresolvedCallsCount = 0;
  uint32_t ExternalFunctions = 0;

  // Walk through all functions to find unresolved calls.
  LibFunc TheLibFunc;
  for (Function &F : M) {
    if (!F.isDeclaration() && isMainEntryPoint(F.getName())) {
      main_def_seen_in_ir = true;
    }

    // First check if the current function has local linkage (IR),
    // it is a LibFunc or an intrinsic
    if (!AssumeWholeProgram && F.isDeclaration() && !F.hasLocalLinkage()) {

      // Check if the current function is a LibFunc or an intrinsic
      if (F.isIntrinsic() ||
          (TLI.getLibFunc(F.getName(), TheLibFunc) &&
          TLI.has(TheLibFunc))) {
        if (WholeProgramTraceLibFuncs)
          LibFuncsFound.insert(&F);

      } else {
        // If the external isn't an intrinsic or a LibFunc, then we have
        // an symbol outside the compilation unit or LTO module

        // TODO: For now, we insert the Function into ExternalSymbols
        // rather than LibFuncsNotFound. The difference is that at this
        // point we can't prove if the missing symbol is a LibFunc or not.
        if (WholeProgramTraceSymbols)
          ExternalSymbols.insert(&F);

        ExternalFunctions++;
        all_resolved &= false;
      }
      // We can skip because there is no IR
      continue;
    }

    all_resolved &= resolveCallsInRoutine(TLI, &F);
  }

  // Walk through all aliases to find unresolved aliases.
  for (auto &GA : M.aliases()) {
    if (GA.hasLocalLinkage())
      continue;
    if (WholeProgramTrace)
      errs() << GA.getName() << "  alias is not local\n";
    all_resolved &= false;
    unresolved_aliases_count++;
  }

  if (WholeProgramTrace) {
    if (main_def_seen_in_ir)
      errs() << "  Main definition seen \n";
    else
      errs() << "  Main definition not seen \n";
    errs() << "  UNRESOLVED CALLSITES: " << UnresolvedCallsCount << "\n";
    errs() << "  GLOBALS UNRESOLVED: " << unresolved_globals_count << "\n";
    errs() << "  ALIASES UNRESOLVED: " << unresolved_aliases_count << "\n";

    if (WholeProgramTraceLibFuncs) {
      errs() << "  TOTAL LIBFUNCS: "
             << LibFuncsFound.size() + LibFuncsNotFound.size() << "\n";
      errs() << "  LIBFUNCS FOUND: " << LibFuncsFound.size() << "\n";
      for (const Function *F : LibFuncsFound)
        errs() << "      " << F->getName() << "\n";
    }

    errs() << "  LIBFUNCS NOT FOUND: " << LibFuncsNotFound.size() << "\n";
    for (const Function *F : LibFuncsNotFound)
      errs() << "      " << F->getName() << "\n";

    errs() << "  EXTERNAL FUNCTIONS: " << ExternalFunctions << "\n";
    if (WholeProgramTraceSymbols) {
      for (const Function *F : ExternalSymbols)
        errs() << "      " << F->getName() << "\n";
    }

    // Print those symbols that are visible outside the LTO unit
    errs() << "  VISIBLE OUTSIDE LTO: " << VisibleSymbolsVector.size() << "\n";
    for (StringRef SymbolName : VisibleSymbolsVector) {
      errs() << "      " << SymbolName << "\n";
    }
  }

  // Print only the libfuncs
  else if (WholeProgramTraceLibFuncs) {
    errs() << "WHOLE-PROGRAM-ANALYSIS: LIBFUNCS TRACE\n\n";
    errs() << "  TOTAL LIBFUNCS: "
           << LibFuncsFound.size() + LibFuncsNotFound.size() << "\n";
    errs() << "  LIBFUNCS FOUND: " << LibFuncsFound.size() << "\n";
    for (const Function *F : LibFuncsFound)
      errs() << "      " << F->getName() << "\n";
    errs() << "  LIBFUNCS NOT FOUND: " << LibFuncsNotFound.size() << "\n";
    for (const Function *F : LibFuncsNotFound)
      errs() << "      " << F->getName() << "\n";
  }

  // Check for all_resolved if WholeProgramAssert is true.
  if (WholeProgramAssert)
    assert(all_resolved &&
           "Whole-Program-Analysis: Did not detect whole program");

  all_resolved &= main_def_seen_in_ir;

  return all_resolved;
}

// Detect whole program using intrinsic table.
//
void WholeProgramInfo::wholeProgramAllExternsAreIntrins(Module &M,
                                            const TargetLibraryInfo &TLI) {
    if (WholeProgramTrace)
      errs() << "\nWHOLE-PROGRAM-ANALYSIS: SIMPLE ANALYSIS\n\n";

    bool resolved = resolveAllLibFunctions(M, TLI);

    if (resolved) {
      WholeProgramSeen = true;
      if (WholeProgramTrace)
        errs() <<  "  WHOLE PROGRAM DETECTED \n";
    }
    else {
      if (WholeProgramTrace)
        errs() <<  "  WHOLE PROGRAM NOT DETECTED \n";
    }

    WholeProgramSafe = isWholeProgramSeen() && isWholeProgramRead()
                       && isLinkedAsExecutable();
    if (WholeProgramTrace) {
      if (WholeProgramSafe) {
        errs() <<  "  WHOLE PROGRAM SAFE is determined\n";
      } else {
        errs() <<  "  WHOLE PROGRAM SAFE is *NOT* determined:\n";
        if (!isWholeProgramSeen())
          errs() <<  "    whole program not seen;\n";
        if (!isWholeProgramRead())
          errs() <<  "    whole program not read;\n";
        if (!isLinkedAsExecutable())
          errs() <<  "    not linking an executable;\n";
      }
    }

    if (WholeProgramTraceSymbols && !WholeProgramTrace) {
      errs() << "\nWHOLE-PROGRAM-ANALYSIS: EXTERNAL FUNCTIONS TRACE\n\n";
      errs() << "  EXTERNAL FUNCTIONS: " << ExternalSymbols.size() << "\n";
      for (const Function *F : ExternalSymbols)
        errs() << "      " << F->getName() << "\n";
    }
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

  return HiddenVisibility;
}

// Return true if the linker finds that all symbols were resolved or
// the assumption flag for whole program read was turned on.
bool WholeProgramInfo::isWholeProgramRead() {
  return getWholeProgramRead() || AssumeWholeProgramRead;
}

// Return true if the linker is generating an executable or the
// assumption flag for executable was turned on.
bool WholeProgramInfo::isLinkedAsExecutable() {
  return getLinkingExecutable() || AssumeWholeProgramExecutable;
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
  return WholeProgramInfo::analyzeModule(M,
                                  AM.getResult<TargetLibraryAnalysis>(M), GTTI,
                                  AM.getCachedResult<CallGraphAnalysis>(M));
}


