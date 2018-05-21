//===------- Intel_WP.cpp - Whole Program Analysis -*------===//
//
// Copyright (C) 2016-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// This file does whole-program-analysis using TargetLibraryInfo.
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"

using namespace llvm;

// If it is true, compiler assumes that source files in the current
// compilation have entire program.
static cl::opt<bool> AssumeWholeProgram("whole-program-assume",
                                        cl::init(false), cl::ReallyHidden);

// Flag to get whole program analysis trace.
static cl::opt<bool> WholeProgramTrace("whole-program-trace",
                                       cl::init(false), cl::ReallyHidden);

// Flag asserts that whole program will be detected.
static cl::opt<bool> WholeProgramAssert("whole-program-assert",
                                        cl::init(false), cl::ReallyHidden);

// If it is true, compiler assumes that whole program has been read (linker can
// reach all symbols).
static cl::opt<bool> AssumeWholeProgramRead("whole-program-assume-read",
                                            cl::init(false), cl::ReallyHidden);

// If it is true, compiler assumes that program is linked as executable.
static cl::opt<bool> AssumeWholeProgramExecutable(
    "whole-program-assume-executable", cl::init(false), cl::ReallyHidden);

// This flag will be set to true after the whole program has been read i.e. the
// linker was able to resolve each function symbol to either one of linked
// modules or dynamic libraries.
static bool WholeProgramRead = false;

// This flag will be set to true by linker plugin when the linker is linking an
// exectubale.
static bool LinkingExecutable = false;

#define DEBUG_TYPE  "wholeprogramanalysis"


INITIALIZE_PASS_BEGIN(WholeProgramWrapperPass, "wholeprogramanalysis",
                "Whole program analysis", false, false)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(WholeProgramWrapperPass, "wholeprogramanalysis",
                "Whole program analysis", false, false)

void llvm::SetWholeProgramRead(bool ProgramRead) {
  WholeProgramRead = ProgramRead;
}

void llvm::SetLinkingExecutable(bool LinkingExe) {
  LinkingExecutable = LinkingExe;
}

static bool IsWholeProgramRead() {
  return WholeProgramRead || AssumeWholeProgramRead;
}

static bool IsLinkedAsExecutable() {
  return LinkingExecutable || AssumeWholeProgramExecutable;
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
  CallGraphWrapperPass *CGPass =
      getAnalysisIfAvailable<CallGraphWrapperPass>();
  CallGraph *CG = CGPass ? &CGPass->getCallGraph() : nullptr;
  Result.reset(new WholeProgramInfo(
                WholeProgramInfo::analyzeModule(
      M, getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(), CG)));
  return false;
}

WholeProgramInfo::WholeProgramInfo() {
  WholeProgramSafe = false;
  WholeProgramSeen = false;
}

WholeProgramInfo::~WholeProgramInfo() {}


// This routine sets linkage of "GV" to Internal except when GV is in
// "AlwaysPreserved".
//
bool WholeProgramInfo::makeInternalize(GlobalValue &GV,
                                       const StringSet<> &AlwaysPreserved) {

  if (GV.hasLocalLinkage())
    return false;

  // Not checking for DLLExportStorage, hasAvailableExternallyLinkage,
  // user supplied preserved symbols, a member of an externally
  // visible comdat etc since user is asserting that it is WholeProgramAssume.
  if (GV.isDeclaration() || AlwaysPreserved.count(GV.getName()))
    return false;

  GV.setVisibility(GlobalValue::DefaultVisibility);
  GV.setLinkage(GlobalValue::InternalLinkage);
  return true;
}

// This routine is called when WholeProgramAssume is true. It sets
// linkage of all globals to Internal if possible.
//
void WholeProgramInfo::makeAllLocalToCompilationUnit(Module &M,
                                                         CallGraph *CG) {

  // Set of symbols private to the compiler that this pass should not touch.
  StringSet<> AlwaysPreserved;

  AlwaysPreserved.insert("main");

  // Never internalize the llvm.used symbol.  It is used to implement
  // attribute((used)).
  AlwaysPreserved.insert("llvm.used");
  AlwaysPreserved.insert("llvm.compiler.used");

  // Never internalize anchors used by the machine module info
  AlwaysPreserved.insert("llvm.global_ctors");
  AlwaysPreserved.insert("llvm.global_dtors");
  AlwaysPreserved.insert("llvm.global.annotations");

  // Never internalize symbols code-gen inserts.
  AlwaysPreserved.insert("__stack_chk_fail");
  AlwaysPreserved.insert("__stack_chk_guard");

  CallGraphNode *ExternalNode = CG ? CG->getExternalCallingNode() : nullptr;

  for (Function &I : M) {
    if (!makeInternalize(I, AlwaysPreserved))
      continue;

    if (ExternalNode)
      ExternalNode->removeOneAbstractEdgeTo((*CG)[&I]);

    if (WholeProgramTrace)
      errs() << "    Internalized func " << I.getName() << "\n";
  }
  for (auto &GV : M.globals()) {
    if (!makeInternalize(GV, AlwaysPreserved))
      continue;

    if (WholeProgramTrace)
      errs() << "    Internalized gvar " << GV.getName() << "\n";
  }

  for (auto &GA : M.aliases()) {
    if (!makeInternalize(GA, AlwaysPreserved))
      continue;

    if (WholeProgramTrace)
      errs() << "    Internalized alias " << GA.getName() << "\n";
  }
}

WholeProgramInfo
WholeProgramInfo::analyzeModule(Module &M, const TargetLibraryInfo &TLI,
                                CallGraph *CG) {

  WholeProgramInfo Result;
  Result.wholeProgramAllExternsAreIntrins(M, TLI);

  if (AssumeWholeProgram) {
    if (WholeProgramTrace)
      errs() << "whole-progran-assume is enabled ... \n";

    Result.WholeProgramSeen = true;
    Result.makeAllLocalToCompilationUnit(M, CG);
  }

  return Result;
}

// This analysis depends on TargetLibraryInfo. Analysis info is not
// modified by any other pass.
void WholeProgramWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<TargetLibraryInfoWrapperPass>();
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
    if (Callee->isIntrinsic() || !Callee->isDeclaration())
      return true;

    LibFunc TheLibFunc;
    if (!TLI.getLibFunc(Callee->getName(), TheLibFunc) ||
        !TLI.has(TheLibFunc)) {
      if (WholeProgramTrace) {
        errs() << Callee->getName() << "    Is not a recognized LibFunc.\n";
      }
      ++UnresolvedCallsCount;
      return false;
    }
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
  for (inst_iterator II = inst_begin(&(*F)), E = inst_end(&(*F));
       II != E; ++II) {
    // Skip if it is not a call inst
    if (!isa<CallInst>(&*II) && !isa<InvokeInst>(&*II)) {
      continue;
    }

    CallSite CS = CallSite(&*II);
    Resolved &= resolveCalledValue(TLI, CS.getCalledValue(), F);
    if (!Resolved && !WholeProgramTrace)
      return false;
  }
  return Resolved;
}

bool WholeProgramInfo::resolveAllLibFunctions(Module &M,
                                       const TargetLibraryInfo &TLI) {
  bool all_resolved = true;
  bool main_def_seen_in_ir = false;
  int unresolved_globals_count = 0;
  int unresolved_aliases_count = 0;

  UnresolvedCallsCount = 0;

  // Walk through all functions to find unresolved calls.
  for (Function &F : M) {
    if (F.getName() == "main" && !F.isDeclaration()) {
      main_def_seen_in_ir = true;
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
      errs() << "      " << "  Main def seen \n";
    else
      errs() << "      " << "  Main def not seen \n";
    errs() << "      " << UnresolvedCallsCount << "  FUNCTIONS UNRESOLVED \n";
    errs() << "      " << unresolved_globals_count << "  GLOBALS UNRESOLVED \n";
    errs() << "      " << unresolved_aliases_count << "  ALIASES UNRESOLVED \n";
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

    WholeProgramSafe = WholeProgramSeen && IsWholeProgramRead() && IsLinkedAsExecutable();
    if (WholeProgramTrace) {
      if (WholeProgramSafe) {
        errs() <<  "  WHOLE PROGRAM SAFE is determined\n";
      } else {
        errs() <<  "  WHOLE PROGRAM SAFE is *NOT* determined:\n";
        if (!WholeProgramSeen)
          errs() <<  "    whole program not seen;\n";
        if (!IsWholeProgramRead())
          errs() <<  "    whole program not read;\n";
        if (!IsLinkedAsExecutable())
          errs() <<  "    not linking an executable;\n";
      }
    }
}

// This returns true if AssumeWholeProgram is true or WholeProgramSafe is true.
// WholeProgramSafe is set to true only if all symbols have been resolved
// and building executable (i.e not building shared library).
//
bool WholeProgramInfo::isWholeProgramSafe(void) {
  return WholeProgramSafe || AssumeWholeProgram;
}

// This returns true if AssumeWholeProgram or WholeProgramSeen is true.
// WholeProgramSeen is set to true if all symbols have been resolved.
bool WholeProgramInfo::isWholeProgramSeen(void) {
  return WholeProgramSeen || AssumeWholeProgram;
}

char WholeProgramAnalysis::PassID;

// Provide a definition for the static class member used to identify passes.
AnalysisKey WholeProgramAnalysis::Key;

WholeProgramInfo WholeProgramAnalysis::run(Module &M,
                                AnalysisManager<Module> &AM) {
  return WholeProgramInfo::analyzeModule(M,
                                      AM.getResult<TargetLibraryAnalysis>(M),
                                  AM.getCachedResult<CallGraphAnalysis>(M));
}


