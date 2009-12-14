//===--- backend_consumer.cpp - Interface to LLVM backend technologies -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "stdafx.h"

#include "backend_consumer.h"

using namespace clang;
using namespace llvm;

#define __FULL_OPTIMIZATION__

FunctionPassManager *BackendConsumer::getCodeGenPasses() const {
  if (!CodeGenPasses) {
    CodeGenPasses = new FunctionPassManager(ModuleProvider);
    CodeGenPasses->add(new TargetData(*TheTargetData));
  }

  return CodeGenPasses;
}

PassManager *BackendConsumer::getPerModulePasses() const {
  if (!PerModulePasses) {
    PerModulePasses = new PassManager();
    PerModulePasses->add(new TargetData(*TheTargetData));
  }

  return PerModulePasses;
}

FunctionPassManager *BackendConsumer::getPerFunctionPasses() const {
  if (!PerFunctionPasses) {
    PerFunctionPasses = new FunctionPassManager(ModuleProvider);
    PerFunctionPasses->add(new TargetData(*TheTargetData));
  }

  return PerFunctionPasses;
}

void BackendConsumer::CreatePasses()
{
  // In -O0 if checking is disabled, we don't even have per-function passes.

#ifdef __FULL_OPTIMIZATION__
  if (CompileOpts.VerifyModule)
    getPerFunctionPasses()->add(createVerifierPass());

  if (CompileOpts.OptimizationLevel > 0) {
    FunctionPassManager *PM = getPerFunctionPasses();
    PM->add(createCFGSimplificationPass());
    if (CompileOpts.OptimizationLevel == 1)
      PM->add(createPromoteMemoryToRegisterPass());
    else
      PM->add(createScalarReplAggregatesPass());
    PM->add(createInstructionCombiningPass());
  }

  // For now we always create per module passes.
  PassManager *PM = getPerModulePasses();
  if (CompileOpts.OptimizationLevel > 0) {
    if (CompileOpts.UnitAtATime)
      PM->add(createRaiseAllocationsPass());      // call %malloc -> malloc inst
    PM->add(createCFGSimplificationPass());       // Clean up disgusting code
    PM->add(createPromoteMemoryToRegisterPass()); // Kill useless allocas
    if (CompileOpts.UnitAtATime) {
      PM->add(createGlobalOptimizerPass());       // Optimize out global vars
      PM->add(createGlobalDCEPass());             // Remove unused fns and globs
      PM->add(createIPConstantPropagationPass()); // IP Constant Propagation
      PM->add(createDeadArgEliminationPass());    // Dead argument elimination
    }
    PM->add(createInstructionCombiningPass());    // Clean up after IPCP & DAE
    PM->add(createCFGSimplificationPass());       // Clean up after IPCP & DAE
    if (CompileOpts.UnitAtATime) {
      PM->add(createPruneEHPass());               // Remove dead EH info
      PM->add(createFunctionAttrsPass());         // Set readonly/readnone attrs
    }
    if (CompileOpts.InlineFunctions)
      PM->add(createFunctionInliningPass());      // Inline small functions
    else 
      PM->add(createAlwaysInlinerPass());         // Respect always_inline
    if (CompileOpts.OptimizationLevel > 2)
      PM->add(createArgumentPromotionPass());     // Scalarize uninlined fn args
    if (CompileOpts.SimplifyLibCalls)
      PM->add(createSimplifyLibCallsPass());      // Library Call Optimizations
    PM->add(createInstructionCombiningPass());    // Cleanup for scalarrepl.
    PM->add(createJumpThreadingPass());           // Thread jumps.
    PM->add(createCFGSimplificationPass());       // Merge & remove BBs
    PM->add(createScalarReplAggregatesPass());    // Break up aggregate allocas
    PM->add(createInstructionCombiningPass());    // Combine silly seq's
    PM->add(createCondPropagationPass());         // Propagate conditionals
    PM->add(createTailCallEliminationPass());     // Eliminate tail calls
    PM->add(createCFGSimplificationPass());       // Merge & remove BBs
    PM->add(createReassociatePass());             // Reassociate expressions
    PM->add(createLoopRotatePass());              // Rotate Loop
    PM->add(createLICMPass());                    // Hoist loop invariants
    PM->add(createLoopUnswitchPass(CompileOpts.OptimizeSize ? true : false));
//    PM->add(createLoopIndexSplitPass());          // Split loop index
    PM->add(createInstructionCombiningPass());  
    PM->add(createIndVarSimplifyPass());          // Canonicalize indvars
    PM->add(createLoopDeletionPass());            // Delete dead loops
    if (CompileOpts.UnrollLoops)
      PM->add(createLoopUnrollPass());            // Unroll small loops
    PM->add(createInstructionCombiningPass());    // Clean up after the unroller
    PM->add(createGVNPass());                     // Remove redundancies
    PM->add(createMemCpyOptPass());               // Remove memcpy / form memset
    PM->add(createSCCPPass());                    // Constant prop with SCCP
    
    // Run instcombine after redundancy elimination to exploit opportunities
    // opened up by them.
    PM->add(createInstructionCombiningPass());
    PM->add(createCondPropagationPass());         // Propagate conditionals
    PM->add(createDeadStoreEliminationPass());    // Delete dead stores
    PM->add(createAggressiveDCEPass());           // Delete dead instructions
    PM->add(createCFGSimplificationPass());       // Merge & remove BBs

    if (CompileOpts.UnitAtATime) {
      PM->add(createStripDeadPrototypesPass());   // Get rid of dead prototypes
      PM->add(createDeadTypeEliminationPass());   // Eliminate dead types
    }

    if (CompileOpts.OptimizationLevel > 1 && CompileOpts.UnitAtATime)
      PM->add(createConstantMergePass());         // Merge dup global constants 
  } else {
    PM->add(createAlwaysInlinerPass());  
  }
#else
	// Function level optimizations
    FunctionPassManager *FPM = getPerFunctionPasses();
    FPM->add(createCFGSimplificationPass());
    FPM->add(createScalarReplAggregatesPass());
    FPM->add(createInstructionCombiningPass());

	// Module level optimizations
	PassManager *MPM = getPerModulePasses();
    MPM->add(createGlobalOptimizerPass());       // Optimize out global vars
    MPM->add(createGlobalDCEPass());             // Remove unused fns and globs
    MPM->add(createInstructionCombiningPass());    // Clean up after IPCP & DAE

	MPM->add(createScalarReplAggregatesPass());    // Break up aggregate allocas
    MPM->add(createInstructionCombiningPass());    // Combine silly seq's

    MPM->add(createGVNPass());                     // Remove redundancies
    MPM->add(createInstructionCombiningPass());

#endif
}

/// EmitAssembly - Handle interaction with LLVM backend to generate
/// actual machine code. 
void BackendConsumer::EmitAssembly()
{
	// Silently ignore if we weren't initialized for some reason.
	if (!TheModule || !TheTargetData)
		return;
  
  
	// Make sure IR generation is happy with the module. This is
	// released by the module provider.
	Module *M = Gen->ReleaseModule();
	if (!M)
	{
		// The module has been released by IR gen on failures, do not
		// double free.
		ModuleProvider->releaseModule();
		TheModule = 0;
		return;
	}

	assert(TheModule == M && "Unexpected module change during IR generation");

	CreatePasses();

	std::string Error;

    AsmOutStream = new raw_svector_ostream(*OStream);
	getPerModulePasses()->add(createBitcodeWriterPass(*AsmOutStream));

	// Run passes. For now we do all passes at once, but eventually we
	// would like to have the option of streaming code generation.

	if (PerFunctionPasses)
	{
		PrettyStackTraceString CrashInfo("Per-function optimization");

		PerFunctionPasses->doInitialization();
		for (Module::iterator I = M->begin(), E = M->end(); I != E; ++I)
		{
		  if (!I->isDeclaration())
			PerFunctionPasses->run(*I);
		}
		PerFunctionPasses->doFinalization();
	}

	if (PerModulePasses)
	{
		PrettyStackTraceString CrashInfo("Per-module optimization passes");
		PerModulePasses->run(*M);
	}

	if (CodeGenPasses)
	{
		PrettyStackTraceString CrashInfo("Code generation");
		CodeGenPasses->doInitialization();
		for (Module::iterator I = M->begin(), E = M->end(); I != E; ++I)
		{
		  if (!I->isDeclaration())
			CodeGenPasses->run(*I);
		}
		CodeGenPasses->doFinalization();
	}
}
