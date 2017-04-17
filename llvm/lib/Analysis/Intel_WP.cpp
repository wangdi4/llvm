//===------- Intel_WP.cpp - Whole Program Analysis -*------===//
//
// Copyright (C) 2016-2017 Intel Corporation. All rights reserved.
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


#define DEBUG_TYPE  "wholeprogramanalysis"


INITIALIZE_PASS_BEGIN(WholeProgramWrapperPass, "wholeprogramanalysis",
                "Whole program analysis", false, false)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
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
//
void WholeProgramWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<TargetLibraryInfoWrapperPass>();
}

// This function returns true if all calls in "F" can be resolved using
// "TLI" info. Otherwise, it returns false and sets "unresolved_funcs_count"
// to number of unresolved calls in "F". 
//
bool WholeProgramInfo::resolveCallsInRoutine(
                 const TargetLibraryInfo &TLI, Function *F, 
                 int *unresolved_funcs_count) {

  bool resolved = true;

  for (inst_iterator II = inst_begin(&(*F)), E = inst_end(&(*F));
       II != E; ++II) {
    // Skip if it is not a call inst
    if (!isa<CallInst>(&*II) && !isa<InvokeInst>(&*II)) {
      continue;
    }

    CallSite CS = CallSite(&*II);
    Function *Callee = CS.getCalledFunction();
    if (Callee == nullptr) {
      // Indirect call:
      // It is possible that getCalledFunction can return nullptr even for
      // direct calls in some rare cases. It is possible to get name of
      // direct call by parsing getCalledValue() but go conservative for 
      // now.
      if (isa<ConstantExpr>(CS.getCalledValue())) {
          (*unresolved_funcs_count)++;
          resolved = false;
          if (WholeProgramTrace)
            errs() <<  "    Unhandled call:  " << &*II << "\n";
      }
    }
    else {
      if (Callee->isDeclaration() || Callee->isIntrinsic()) {
         LibFunc TheLibFunc;

         if (!(TLI.getLibFunc(Callee->getName(), TheLibFunc) &&
             TLI.has(TheLibFunc)) && !Callee->isIntrinsic()) {

          (*unresolved_funcs_count)++;
          resolved = false;
          if (WholeProgramTrace)
            errs() << Callee->getName()  << "    Not in intrinsic table \n";
       
        }
      }
      else if (!F->hasExactDefinition()) {
        // Treat it as unresolved. 
        // TODO: It can be improved by finding why it doesn't have
        // exact definition.
        (*unresolved_funcs_count)++;
        resolved = false;
        if (WholeProgramTrace)
          errs() << Callee->getName()  << "    No exact defintion \n";
      }
    }
  }
  return resolved;
}

bool WholeProgramInfo::resolveAllLibFunctions(Module &M,
                                       const TargetLibraryInfo &TLI) {
  bool all_resolved = true;
  bool main_def_seen_in_ir = false;
  int unresolved_funcs_count = 0;
  int unresolved_globals_count = 0;
  int unresolved_aliases_count = 0;

  // Walk through all functions to find unresolved calls.
  for (Function &F : M) {
    if (F.getName() == "main" && !F.isDeclaration()) {
      main_def_seen_in_ir = true;
    }
    all_resolved &= resolveCallsInRoutine(TLI, &F, &unresolved_funcs_count);
  }
   
  StringSet<> IgnoreKnownGlobals;
   
  IgnoreKnownGlobals.insert("stdin");
  IgnoreKnownGlobals.insert("stdout");
  IgnoreKnownGlobals.insert("stderr");
  IgnoreKnownGlobals.insert("environ");

  // Walk through all globals to find unresolved globals.
  for (auto &GV : M.globals()) {
    if (GV.hasLocalLinkage() || IgnoreKnownGlobals.count(GV.getName()))
      continue;
    if (WholeProgramTrace)
      errs() << GV.getName() << "  global is not local\n";
    all_resolved &= false;
    unresolved_globals_count++;
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
    errs() << "      " << unresolved_funcs_count << "  FUNCTIONS UNRESOLVED \n";
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

    // TODO: WholeProgramSafe is set only if we are building executable.
    // Need to another check here to make sure we are building
    // executable. Not yet found good solution to check this. It will be
    // fixed in the next check-in
    if (WholeProgramSeen) {
      WholeProgramSafe = true;
      if (WholeProgramTrace)
        errs() <<  "  WHOLE PROGRAM SAFE is determined\n";
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


