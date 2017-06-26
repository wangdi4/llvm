/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "Optimizer.h"
#include "VecConfig.h"
#include "CPUDetect.h"
#include "debuggingservicetype.h"
#include "CompilationUtils.h"
#include "MetaDataApi.h"
#include "OclTune.h"

#ifndef __APPLE__
#include "PrintIRPass.h"
#include "mic_dev_limits.h"
#endif //#ifndef __APPLE__
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/IPO/InferFunctionAttrs.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Support/Casting.h"

extern "C"{

void *createInstToFuncCallPass(bool);

llvm::Pass *createVectorizerPass(SmallVector<Module *, 2> builtinModules,
                                 const intel::OptimizerConfig *pConfig);
llvm::Pass *createBarrierMainPass(intel::DebuggingServiceType debugType);

llvm::ModulePass *createCLWGLoopCreatorPass();
llvm::ModulePass *createCLWGLoopBoundariesPass();
llvm::Pass *createCLBuiltinLICMPass();
llvm::Pass *createLoopStridedCodeMotionPass();
llvm::Pass *createCLStreamSamplerPass();
llvm::Pass *createPreventDivisionCrashesPass();
llvm::Pass *createOptimizeIDivPass();
llvm::Pass *createShiftZeroUpperBitsPass();
llvm::Pass *createBuiltinCallToInstPass();
llvm::Pass *createRelaxedPass();
llvm::Pass *createLinearIdResolverPass();
llvm::ModulePass *createSubGroupAdaptationPass();
llvm::ModulePass *createKernelAnalysisPass();
llvm::ModulePass *createBlockToFuncPtrPass();
llvm::ModulePass *createBuiltInImportPass(const char *CPUName);
llvm::ImmutablePass *createImplicitArgsAnalysisPass(llvm::LLVMContext *C);
llvm::ModulePass *createChannelPipeTransformationPass();
llvm::ModulePass *createPipeSupportPass();
llvm::ModulePass *createLocalBuffersPass(bool isNativeDebug);
llvm::ModulePass *createAddImplicitArgsPass();
llvm::ModulePass *createOclFunctionAttrsPass();
llvm::ModulePass *createOclSyncFunctionAttrsPass();
llvm::ModulePass *createModuleCleanupPass(bool SpareOnlyWrappers);
llvm::ModulePass *createGenericAddressStaticResolutionPass();
llvm::ModulePass *createGenericAddressDynamicResolutionPass();
llvm::ModulePass *createPrepareKernelArgsPass();
llvm::Pass *
createBuiltinLibInfoPass(SmallVector<Module *, 2> pRtlModuleList,
                         std::string type);
llvm::ModulePass *createUndifinedExternalFunctionsPass(
    std::vector<std::string> &undefinedExternalFunctions);
llvm::ModulePass *createKernelInfoWrapperPass();
llvm::ModulePass *createDuplicateCalledKernelsPass();
llvm::ModulePass *createPatchCallbackArgsPass();
llvm::ModulePass *createDeduceMaxWGDimPass();

llvm::ModulePass *createFMASplitterPass();
llvm::ModulePass *createSpirMaterializer();
void materializeSpirDataLayout(llvm::Module &);
llvm::FunctionPass *createPrefetchPassLevel(int level);
llvm::ModulePass *createRemovePrefetchPass();
llvm::ModulePass *createPrintIRPass(int option, int optionLocation,
                                    std::string dumpDir);
llvm::ModulePass *createDebugInfoPass();
llvm::ModulePass *createReduceAlignmentPass();
llvm::ModulePass *createProfilingInfoPass();
llvm::Pass *createSmartGVNPass(bool);

llvm::ModulePass *createSinCosFoldPass();
llvm::ModulePass *createResolveWICallPass();
llvm::ModulePass *createDetectFuncPtrCalls();
llvm::ModulePass *createDetectRecursionPass();
llvm::ModulePass *createCloneBlockInvokeFuncToKernelPass();
llvm::Pass *createResolveBlockToStaticCallPass();
llvm::ModulePass *createPreLegalizeBoolsPass();
llvm::ImmutablePass *createOCLAliasAnalysisPass();
llvm::ModulePass *createSPIR20BlocksToObjCBlocks();
llvm::ModulePass *createPrintfArgumentsPromotionPass();
}

using namespace intel;
namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

/// createStandardModulePasses - Add the standard module passes.  This is
/// expected to be run after the standard function passes.
static inline void createStandardLLVMPasses(llvm::legacy::PassManagerBase *PM,
                                            unsigned OptLevel, bool UnitAtATime,
                                            bool UnrollLoops,
                                            int rtLoopUnrollFactor,
                                            bool allowAllocaModificationOpt,
                                            bool isDBG, bool HasGatherScatter) {
  if (OptLevel == 0) {
    return;
  }

  if (UnitAtATime) {
    PM->add(llvm::createGlobalOptimizerPass());    // Optimize out global vars
    PM->add(llvm::createIPSCCPPass());             // IP SCCP
    PM->add(llvm::createDeadArgEliminationPass()); // Dead argument elimination
  }
  PM->add(llvm::createInstructionSimplifierPass());
  PM->add(llvm::createInstructionCombiningPass()); // Clean up after IPCP & DAE
  PM->add(llvm::createCFGSimplificationPass());    // Clean up after IPCP & DAE

  // Set readonly/readnone attrs
  if (UnitAtATime)
    PM->add(llvm::createInferFunctionAttrsLegacyPass());
  if (OptLevel > 2)
    PM->add(llvm::createArgumentPromotionPass()); // Scalarize uninlined fn args

  // Break up aggregate allocas
  PM->add(llvm::createSROAPass());
  PM->add(llvm::createEarlyCSEPass());           // Catch trivial redundancies
  PM->add(llvm::createInstructionSimplifierPass());
  PM->add(llvm::createInstructionCombiningPass()); // Cleanup for scalarrepl.
  PM->add(llvm::createJumpThreadingPass());        // Thread jumps.
  // Propagate conditionals
  PM->add(llvm::createCorrelatedValuePropagationPass());
  PM->add(llvm::createCFGSimplificationPass());      // Merge & remove BBs
  PM->add(llvm::createInstructionCombiningPass());   // Combine silly seq's

  PM->add(llvm::createTailCallEliminationPass()); // Eliminate tail calls
  PM->add(llvm::createCFGSimplificationPass());   // Merge & remove BBs
  PM->add(llvm::createReassociatePass());         // Reassociate expressions
  PM->add(llvm::createLoopRotatePass());          // Rotate Loop
  PM->add(llvm::createLICMPass());                // Hoist loop invariants
  PM->add(llvm::createLoopUnswitchPass(OptLevel < 3));
  PM->add(llvm::createInstructionCombiningPass());
  PM->add(llvm::createInstructionSimplifierPass());
  PM->add(llvm::createIndVarSimplifyPass()); // Canonicalize indvars
  PM->add(llvm::createLoopDeletionPass());   // Delete dead loops
  if (UnrollLoops) {
    PM->add(llvm::createLoopUnrollPass(512, 0, 0)); // Unroll small loops
    // unroll loops with non-constant trip count
    const int thresholdBase = 16;
    if (rtLoopUnrollFactor > 1) {
      const int threshold = thresholdBase * rtLoopUnrollFactor;
      PM->add(llvm::createLoopUnrollPass(threshold, rtLoopUnrollFactor, 0, 1));
    }
  }
  if (!isDBG) {
    PM->add(llvm::createFunctionInliningPass(4096)); // Inline (not only small)
                                                     // functions
  }
  // Break up aggregate allocas
  PM->add(llvm::createSROAPass());
  // Clean up after the unroller
  PM->add(llvm::createInstructionCombiningPass());
  PM->add(llvm::createInstructionSimplifierPass());
  if (allowAllocaModificationOpt) {
    if (OptLevel > 1)
      PM->add(llvm::createGVNPass());     // Remove redundancies
    PM->add(llvm::createMemCpyOptPass()); // Remove memcpy / form memset
  }
  PM->add(llvm::createSCCPPass()); // Constant prop with SCCP

  // Run instcombine after redundancy elimination to exploit opportunities
  // opened up by them.
  PM->add(llvm::createInstructionCombiningPass());
  PM->add(llvm::createJumpThreadingPass()); // Thread jumps
  PM->add(llvm::createCorrelatedValuePropagationPass());
  PM->add(llvm::createDeadStoreEliminationPass()); // Delete dead stores
  PM->add(llvm::createAggressiveDCEPass());        // Delete dead instructions
  PM->add(llvm::createCFGSimplificationPass());    // Merge & remove BBs
  PM->add(llvm::createInstructionCombiningPass()); // Clean up after everything.

  if (UnitAtATime) {
    // Get rid of dead prototypes
    PM->add(llvm::createStripDeadPrototypesPass());

    // GlobalOpt already deletes dead functions and globals, at -O3 try a
    // late pass of GlobalDCE.  It is capable of deleting dead cycles.
    if (OptLevel > 2)
      PM->add(llvm::createGlobalDCEPass()); // Remove dead fns and globals.

    if (OptLevel > 1)
      PM->add(llvm::createConstantMergePass()); // Merge dup global constants
  }
  PM->add(llvm::createUnifyFunctionExitNodesPass());
}

static void populatePassesPreFailCheck(llvm::legacy::PassManagerBase &PM,
                                       llvm::Module *M,
                                       SmallVector<Module*, 2> & pRtlModuleList,
                                       unsigned OptLevel,
                                       const intel::OptimizerConfig *pConfig,
                                       bool isOcl20,
                                       bool isFpgaEmulator,
                                       bool UnrollLoops) {
  DebuggingServiceType debugType =
      getDebuggingServiceType(pConfig->GetDebugInfoFlag());
#ifndef __APPLE__
  PrintIRPass::DumpIRConfig dumpIRAfterConfig(pConfig->GetIRDumpOptionsAfter());
  PrintIRPass::DumpIRConfig dumpIRBeforeConfig(
      pConfig->GetIRDumpOptionsBefore());
#endif
  bool HasGatherScatter = pConfig->GetCpuId().HasGatherScatter();

  PM.add(createFMASplitterPass());
  PM.add(createOclSyncFunctionAttrsPass());
  PM.add(createPrintfArgumentsPromotionPass());
  if (isOcl20) {
    // Convert SPIR 2.0 blocks to Objective-C blocks
    PM.add(createSPIR20BlocksToObjCBlocks());
    // OCL2.0 resolve block to static call
    PM.add(createResolveBlockToStaticCallPass());
    // clone block_invoke functions to kernels
    PM.add(createCloneBlockInvokeFuncToKernelPass());
  }

  if (OptLevel > 0) {
    PM.add(llvm::createCFGSimplificationPass());
    if (OptLevel == 1)
      PM.add(llvm::createPromoteMemoryToRegisterPass());
    else {
      PM.add(llvm::createSROAPass());
    }
    PM.add(llvm::createInstructionCombiningPass());
    PM.add(llvm::createInstructionSimplifierPass());
  }

  if (isOcl20)
    PM.add(createSubGroupAdaptationPass());

  if (isOcl20) {
    // Flatten get_{local, global}_linear_id()
    PM.add(createLinearIdResolverPass());
  }

  PM.add(createBuiltinLibInfoPass(pRtlModuleList, ""));

  if (isFpgaEmulator) {
      PM.add(createChannelPipeTransformationPass());
      PM.add(createPipeSupportPass());
  }

  // Adding module passes.
#ifndef __APPLE__
  if (dumpIRBeforeConfig.ShouldPrintPass(DUMP_IR_TARGERT_DATA)) {
    PM.add(createPrintIRPass(DUMP_IR_TARGERT_DATA, OPTION_IR_DUMPTYPE_BEFORE,
                             pConfig->GetDumpIRDir()));
  }
#endif
  // OCL2.0 add Generic Address Static Resolution pass
  if (isOcl20) {
    // Static resolution of generic address space pointers
    if (OptLevel > 0) {
      PM.add(llvm::createPromoteMemoryToRegisterPass());
    }
    PM.add(createGenericAddressStaticResolutionPass());
  }

  PM.add(llvm::createBasicAAWrapperPass());
  PM.add(createOCLAliasAnalysisPass());
#ifndef __APPLE__
  if (dumpIRAfterConfig.ShouldPrintPass(DUMP_IR_TARGERT_DATA)) {
    PM.add(createPrintIRPass(DUMP_IR_TARGERT_DATA, OPTION_IR_DUMPTYPE_AFTER,
                             pConfig->GetDumpIRDir()));
  }
  if (!pConfig->GetLibraryModule() &&
      (getenv("DISMPF") != NULL || intel::Statistic::isEnabled()))
    PM.add(createRemovePrefetchPass());
#endif //#ifndef __APPLE__
  PM.add(createBuiltinCallToInstPass());

  bool allowAllocaModificationOpt = true;
  if (!pConfig->GetLibraryModule() && HasGatherScatter) {
    allowAllocaModificationOpt = false;
  }
  // When running the standard optimization passes, do not change the
  // loop-unswitch
  // pass on modules which contain barriers. This pass is illegal for
  // barriers.
  bool UnitAtATime = true;

  int rtLoopUnrollFactor = pConfig->GetRTLoopUnrollFactor();

  createStandardLLVMPasses(
      &PM, OptLevel,
      UnitAtATime, UnrollLoops, rtLoopUnrollFactor, allowAllocaModificationOpt,
      debugType != intel::None, HasGatherScatter);
  // check function pointers calls are gone after standard optimizations
  // if not compilation will fail
  PM.add(createDetectFuncPtrCalls());
  // check there is no recursion, if there is fail compilation
  PM.add(createDetectRecursionPass());
}

static void
populatePassesPostFailCheck(llvm::legacy::PassManagerBase &PM, llvm::Module *M,
                            SmallVector<Module *, 2> &pRtlModuleList,
                            unsigned OptLevel,
                            const intel::OptimizerConfig *pConfig,
                            std::vector<std::string> &UndefinedExternals,
                            bool isOcl20, bool UnrollLoops) {
  bool isProfiling = pConfig->GetProfilingFlag();
  bool HasGatherScatter = pConfig->GetCpuId().HasGatherScatter();
  // Tune the maximum size of the basic block for memory dependency analysis
  // utilized by GVN.
  DebuggingServiceType debugType =
      getDebuggingServiceType(pConfig->GetDebugInfoFlag());
#ifndef __APPLE__
  PrintIRPass::DumpIRConfig dumpIRAfterConfig(pConfig->GetIRDumpOptionsAfter());
  PrintIRPass::DumpIRConfig dumpIRBeforeConfig(
      pConfig->GetIRDumpOptionsBefore());
#endif
  PM.add(createBuiltinLibInfoPass(pRtlModuleList, ""));
  PM.add(createImplicitArgsAnalysisPass(&M->getContext()));

  if (isOcl20) {
    // Repeat static resolution of generic address space pointers after
    // LLVM IR was optimized
    PM.add(createGenericAddressStaticResolutionPass());
    // No need to run function inlining pass here, because if there are still
    // non-inlined functions left - then we don't have to inline new ones.
  }
  // Run the OclFunctionAttrs pass after GenericAddressStaticResolution
  PM.add(createOclFunctionAttrsPass());

  PM.add(llvm::createUnifyFunctionExitNodesPass());


  PM.add(llvm::createBasicAAWrapperPass());
  PM.add(createOCLAliasAnalysisPass());

  // Should be called before vectorizer!
  PM.add((llvm::Pass*)createInstToFuncCallPass(HasGatherScatter));

  PM.add(createDuplicateCalledKernelsPass());

  if (debugType == intel::None && !pConfig->GetLibraryModule()) {
    PM.add(createKernelAnalysisPass());
    PM.add(createCLWGLoopBoundariesPass());
    PM.add(llvm::createDeadCodeEliminationPass());
    PM.add(llvm::createCFGSimplificationPass());
  }

  // In Apple build TRANSPOSE_SIZE_1 is not declared
  if (pConfig->GetTransposeSize() != 1 /*TRANSPOSE_SIZE_1*/
      && debugType == intel::None && OptLevel != 0) {
#ifndef __APPLE__
    // In profiling mode remove llvm.dbg.value calls before vectorizer.
    if (isProfiling) {
      PM.add(createProfilingInfoPass());
    }

    if (dumpIRBeforeConfig.ShouldPrintPass(DUMP_IR_VECTORIZER)) {
      PM.add(createPrintIRPass(DUMP_IR_VECTORIZER, OPTION_IR_DUMPTYPE_BEFORE,
                               pConfig->GetDumpIRDir()));
    }
    PM.add(createSinCosFoldPass());
#endif //#ifndef __APPLE__
    if (!pRtlModuleList.empty()) {
      PM.add(createVectorizerPass(pRtlModuleList, pConfig));
    }
#ifndef __APPLE__
    if (dumpIRAfterConfig.ShouldPrintPass(DUMP_IR_VECTORIZER)) {
      PM.add(createPrintIRPass(DUMP_IR_VECTORIZER, OPTION_IR_DUMPTYPE_AFTER,
                               pConfig->GetDumpIRDir()));
    }
    if (!HasGatherScatter) {
      PM.add(createReduceAlignmentPass());
      // no point to run for older CPU archs
      if (pConfig->GetCpuId().HasSSE41()) {
        // Workaround boolean vectors legalization issue.
        PM.add(createPreLegalizeBoolsPass());
      }
    }
#endif //#ifndef __APPLE__
  }
#ifdef _DEBUG
  PM.add(llvm::createVerifierPass());
#endif

  // Unroll small loops with unknown trip count.
  PM.add(llvm::createLoopUnrollPass(16, 0, 0, 1));
  // The ShiftZeroUpperBits pass should be added after the vectorizer because
  // the vectorizer may transform scalar shifts into vector shifts, and we want
  // this pass to fix all vector shift in this module.
  PM.add(createShiftZeroUpperBitsPass());
  PM.add(createOptimizeIDivPass());
  PM.add(createPreventDivisionCrashesPass());
  // We need InstructionCombining and GVN passes after ShiftZeroUpperBits,
  // PreventDivisionCrashes passes to optimize redundancy introduced by those
  // passes
  if (debugType == intel::None) {
    PM.add(llvm::createInstructionCombiningPass());
    PM.add(createSmartGVNPass(false));
  }
#ifndef __APPLE__
  // The debugType enum and isProfiling flag are mutually exclusive, with
  // precedence given to debugType.
  if (debugType == Simulator) {
    // DebugInfo pass must run before Barrier pass when debugging with simulator
    PM.add(createDebugInfoPass());
  } else if (isProfiling) {
    PM.add(createProfilingInfoPass());
  }
#endif
  if (isOcl20) {
    // Resolve (dynamically) generic address space pointers which are relevant
    // for correct execution
    PM.add(createGenericAddressDynamicResolutionPass());
    // No need to run function inlining pass here, because if there are still
    // non-inlined functions left - then we don't have to inline new ones.
  }

  // Get Some info about the kernel should be called before BarrierPass and
  // createPrepareKernelArgsPass
  if (!pRtlModuleList.empty()) {
    PM.add(createKernelInfoWrapperPass());
  }

  // Adding WG loops
  if (!pConfig->GetLibraryModule()) {
    if (debugType == intel::None) {
      PM.add(createDeduceMaxWGDimPass());
      PM.add(createCLWGLoopCreatorPass());
    }
    // This is a good time to remove internal functions which are not called
    // TODO: Once we set the linkage of internal functions correctly, we won't
    // to run this pass because the LLVM Inliner, for example, will delete
    // uncalled functions it inlines.
    PM.add(createModuleCleanupPass(false));
    PM.add(createBarrierMainPass(debugType));

    // After adding loops run loop optimizations.
    if (debugType == intel::None) {
      PM.add(createCLBuiltinLICMPass());
      PM.add(llvm::createLICMPass());
#ifdef __APPLE__
      // Workaround due to a bug in LICM, need to break the Loop passes flow
      // after LICM.
      // TODO: remove it after fixing the bug in LICM.
      PM.add(llvm::createVerifierPass());
#endif //#ifdef __APPLE__
      PM.add(createLoopStridedCodeMotionPass());
      PM.add(createCLStreamSamplerPass());
    }
  }

  if (pConfig->GetRelaxedMath()) {
    PM.add(createRelaxedPass());
  }

  // The following three passes (AddImplicitArgs/ResolveWICall/LocalBuffer)
  // must run before createBuiltInImportPass!
  if (!pConfig->GetLibraryModule()) {
    PM.add(createAddImplicitArgsPass());
    PM.add(createResolveWICallPass());
    PM.add(createLocalBuffersPass(debugType == Native));
    // clang converts OCL's local to global.
    // createLocalBuffersPass changes the local allocation from global to a
    // kernel argument.
    // The next pass createGlobalOptimizerPass cleans the unused global
    // allocation in order to make sure we will not allocate redundant space on
    // the jit
    if (debugType != Native)
      PM.add(llvm::createGlobalOptimizerPass());
  }

#ifdef _DEBUG
  PM.add(llvm::createVerifierPass());
#endif

  // This pass checks if the module uses an undefined function or not
  // assumption: should run after WI function resolving
  PM.add(createUndifinedExternalFunctionsPass(UndefinedExternals));

  if (!pRtlModuleList.empty()) {
    // Replace pointers to %opencl.block opaque type to function pointer casts
    // The right place to run this pass in or after SPIR20BlockToObjCBlock Pass,
    // but it is impossible in current design since functions in user and RTL modules
    // doesn't match by names there (need to run passes like GAS resolution before).
    PM.add(createBlockToFuncPtrPass());
    // Inline BI function
    PM.add(createBuiltInImportPass(pConfig->GetCpuId().GetCPUPrefix()));
    // Need to convert shuffle calls to shuffle IR before running inline pass
    // on built-ins
    PM.add(createBuiltinCallToInstPass());
  }

// funcPassMgr->add(new intel::SelectLower());

#ifdef _DEBUG
  PM.add(llvm::createVerifierPass());
#endif

  if (debugType == intel::None) {
    if (HasGatherScatter)
      PM.add(llvm::createFunctionInliningPass(4096)); // Inline (not only small) functions.
    else
      PM.add(llvm::createFunctionInliningPass());     // Inline small functions
  } else {
    // Functions with the alwaysinline attribute need to be inlined for
    // functional purposes
    PM.add(llvm::createAlwaysInlinerLegacyPass());
  }
  // Some built-in functions contain calls to external functions which take
  // arguments that are retrieved from the function's implicit arguments.
  // Currently only applies to OpenCL 2.x
  if (isOcl20)
    PM.add(createPatchCallbackArgsPass());

  if (debugType == intel::None) {
    PM.add(llvm::createArgumentPromotionPass()); // Scalarize uninlined fn args
    PM.add(llvm::createInstructionCombiningPass()); // Cleanup for scalarrepl.
    PM.add(llvm::createDeadStoreEliminationPass()); // Delete dead stores
    PM.add(llvm::createAggressiveDCEPass());        // Delete dead instructions
    PM.add(llvm::createCFGSimplificationPass());    // Merge & remove BBs
    PM.add(llvm::createInstructionCombiningPass()); // Cleanup for scalarrepl.
#ifdef __APPLE__
    // Due to none default ABI, some built-ins are creating an alloca in middle
    // of function. Need to run scalar aggregation to get rid of these alloca
    // (after built-in import). mem2reg pass is not enough! as it only handles
    // alloca in first basic block.
    PM.add(llvm::createScalarReplAggregatesPass());
#else
    PM.add(llvm::createPromoteMemoryToRegisterPass());
#endif
  }

  // PrepareKernelArgsPass must run in debugging mode as well
  if (!pConfig->GetLibraryModule())
    PM.add(createPrepareKernelArgsPass());

  if ( debugType == intel::None ) {
    // These passes come after PrepareKernelArgs pass to eliminate the
    // redundancy reducced by it
    PM.add(llvm::createFunctionInliningPass());    // Inline
    PM.add(llvm::createDeadCodeEliminationPass()); // Delete dead instructions
    PM.add(llvm::createCFGSimplificationPass());   // Simplify CFG
    PM.add(llvm::createInstructionCombiningPass()); // Instruction combining
    PM.add(llvm::createDeadStoreEliminationPass()); // Eliminated dead stores
    PM.add(llvm::createEarlyCSEPass());
    PM.add(createSmartGVNPass(true)); // GVN with "no load" heuristic

#ifdef _DEBUG
    PM.add(llvm::createVerifierPass());
#endif
  }

  // Remove unneeded functions from the module.
  // *** keep this optimization last, or at least after function inlining! ***
  if (!pConfig->GetLibraryModule())
    PM.add(createModuleCleanupPass(true));

#ifndef __APPLE__
  // Add prefetches if useful for micro-architecture, if not in debug mode,
  // and don't change libraries
  if (debugType == intel::None && !pConfig->GetLibraryModule() &&
      HasGatherScatter) {
    int APFLevel = pConfig->GetAPFLevel();
    // do APF and following cleaning passes only if APF is not disabled
    if (APFLevel != APFLEVEL_0_DISAPF) {
      if (pConfig->GetCpuId().RequirePrefetch())
        PM.add(createPrefetchPassLevel(pConfig->GetAPFLevel()));

      PM.add(llvm::createDeadCodeEliminationPass()); // Delete dead instructions
      PM.add(llvm::createInstructionCombiningPass()); // Instruction combining
      PM.add(createSmartGVNPass(false));
#ifdef _DEBUG
      PM.add(llvm::createVerifierPass());
#endif
    }
  }
  if (UnrollLoops && debugType == intel::None) {
    PM.add(llvm::createLoopUnrollPass(4, 0, 0)); // Unroll small loops
  }
#endif
}

Optimizer::~Optimizer() {}

Optimizer::Optimizer(llvm::Module *pModule,
                     llvm::SmallVector<llvm::Module *, 2> pRtlModuleList,
                     const intel::OptimizerConfig *pConfig)
    : m_pModule(pModule), m_pRtlModuleList(pRtlModuleList) {

  DebuggingServiceType debugType =
      getDebuggingServiceType(pConfig->GetDebugInfoFlag());

#ifndef __APPLE__
  // Materializing the spir datalayout according to the triple.
  materializeSpirDataLayout(*pModule);
#endif //#ifndef __APPLE__

  unsigned int OptLevel = 3;
  if (pConfig->GetDisableOpt() || debugType != intel::None)
    OptLevel = 0;
  const bool isFpgaEmulator = pConfig->isFpgaEmulator();

  // Detect OCL2.0 compilation mode
  const bool isOcl20 = CompilationUtils::getCLVersionFromModuleOrDefault(
                           *pModule) >= OclVersion::CL_VER_2_0;
  bool UnrollLoops = true;
  // Add passes which will run unconditionally
  populatePassesPreFailCheck(m_PreFailCheckPM, pModule, m_pRtlModuleList,
                             OptLevel, pConfig, isOcl20,
                             isFpgaEmulator, UnrollLoops);

  // Add passes which will be run only if hasFunctionPtrCalls() and
  // hasRecursion() will return false
  populatePassesPostFailCheck(
      m_PostFailCheckPM, pModule, m_pRtlModuleList, OptLevel,
      pConfig, m_undefinedExternalFunctions, isOcl20, UnrollLoops);
}

void Optimizer::Optimize() {
#ifndef __APPLE__
  legacy::PassManager materializerPM;
  materializerPM.add(createBuiltinLibInfoPass(m_pRtlModuleList, ""));
  materializerPM.add(createSpirMaterializer());
  materializerPM.run(*m_pModule);
#endif
  m_PreFailCheckPM.run(*m_pModule);

  // if there are still unresolved functon pointer calls
  // after standard LLVM optimizations applied
  // it means blocks variable call cannot be resolved to static call
  // Interrupt optimizations and return.
  // Compilation will report failure
  if (hasFunctionPtrCalls()) {
    return;
  }

  // if there are still recursive calls  after standard
  // LLVM optimizations applied  Compilation will report failure
  if (hasRecursion()) {
    return;
  }

  m_PostFailCheckPM.run(*m_pModule);
}

bool Optimizer::hasUndefinedExternals() const {
  return !m_undefinedExternalFunctions.empty();
}

const std::vector<std::string> &Optimizer::GetUndefinedExternals() const {
  return m_undefinedExternalFunctions;
}

bool Optimizer::hasFunctionPtrCalls() { return !GetFuncNames(true).empty(); }

bool Optimizer::hasRecursion() { return !GetFuncNames(false).empty(); }

std::vector<std::string> Optimizer::GetFuncNames(bool funcsWithFuncPtrCalls) {
  bool returnFunc = false;
  assert(m_pModule && "Module is NULL");
  std::vector<std::string> res;
  MetaDataUtils mdUtils(m_pModule);

  // check FunctionInfo exists
  if (mdUtils.empty_FunctionsInfo()) {
    return std::vector<std::string>();
  }

  MetaDataUtils::FunctionsInfoMap::iterator i = mdUtils.begin_FunctionsInfo();
  MetaDataUtils::FunctionsInfoMap::iterator e = mdUtils.end_FunctionsInfo();
  for (; i != e; ++i) {
    llvm::Function *pFunc = i->first;
    Intel::FunctionInfoMetaDataHandle kimd = i->second;
    // If additional else-ifs are needed in order to examine other function
    // properties, better change the "bool callingFunc" to an enum and use
    // switch-case.
    if (funcsWithFuncPtrCalls == true) { // if calling func = hasFunctionPtrCall
      returnFunc = kimd->isFuncPtrCallHasValue() && kimd->getFuncPtrCall();
    } else { // if calling func = hasRecursion
      returnFunc = kimd->isHasRecursionHasValue() && kimd->getHasRecursion();
    }
    if (returnFunc) {
      res.push_back(pFunc->getName());
      returnFunc = false;
    }
  }

  return res;
}

}}}
