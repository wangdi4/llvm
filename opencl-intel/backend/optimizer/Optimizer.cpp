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

#ifndef __APPLE__
#include "PrintIRPass.h"
#include "mic_dev_limits.h"
#endif //#ifndef __APPLE__
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Pass.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Version.h"
#if LLVM_VERSION == 3425
#include "llvm/Target/TargetData.h"
#else
#include "llvm/DataLayout.h"
#endif
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Assembly/PrintModulePass.h"
#include "llvm/Metadata.h"
#include "llvm/Support/Casting.h"

extern "C"{

void* createInstToFuncCallPass(bool);

llvm::Pass *createVectorizerPass(const llvm::Module *runtimeModule,
                                 const intel::OptimizerConfig* pConfig);
llvm::Pass *createBarrierMainPass(intel::DebuggingServiceType debugType);

llvm::ModulePass* createCLWGLoopCreatorPass();
llvm::ModulePass* createCLWGLoopBoundariesPass();
llvm::Pass* createCLBuiltinLICMPass();
llvm::Pass* createLoopStridedCodeMotionPass();
llvm::Pass* createCLStreamSamplerPass();
llvm::Pass *createPreventDivisionCrashesPass();
llvm::Pass *createShiftZeroUpperBitsPass();
llvm::Pass *createShuffleCallToInstPass();
llvm::Pass *createRelaxedPass();
llvm::Pass *createLinearIdResolverPass();
llvm::ModulePass *createKernelAnalysisPass();
llvm::ModulePass *createBuiltInImportPass();
llvm::ImmutablePass * createImplicitArgsAnalysisPass(llvm::LLVMContext *C);
llvm::ModulePass *createLocalBuffersPass(bool isNativeDebug);
llvm::ModulePass *createAddImplicitArgsPass();
llvm::ModulePass *createOclFunctionAttrsPass();
llvm::ModulePass *createModuleCleanupPass();
llvm::ModulePass *createGenericAddressStaticResolutionPass();
llvm::ModulePass *createGenericAddressDynamicResolutionPass();
llvm::ModulePass *createPrepareKernelArgsPass();
llvm::Pass *createBuiltinLibInfoPass(llvm::Module* pRTModule, std::string type);
llvm::ModulePass *createUndifinedExternalFunctionsPass(std::vector<std::string> &undefinedExternalFunctions);
llvm::ModulePass *createKernelInfoWrapperPass();
llvm::ModulePass *createDuplicateCalledKernelsPass();

#ifdef __APPLE__
llvm::Pass *createClangCompatFixerPass();
#else
llvm::ModulePass *createSpirMaterializer();
void materializeSpirDataLayout(llvm::Module&);
llvm::FunctionPass *createPrefetchPassLevel(int level);
llvm::ModulePass * createRemovePrefetchPass();
llvm::ModulePass *createPrintIRPass(int option, int optionLocation, std::string dumpDir);
llvm::ModulePass* createDebugInfoPass();
llvm::ModulePass *createReduceAlignmentPass();
llvm::ModulePass* createProfilingInfoPass();
#endif
llvm::ModulePass *createResolveWICallPass();
llvm::ModulePass *createDetectFuncPtrCalls();
llvm::ModulePass *createDetectRecursionPass();
llvm::ModulePass *createCloneBlockInvokeFuncToKernelPass();
llvm::Pass *createResolveBlockToStaticCallPass();
}

using namespace intel;
namespace Intel { namespace OpenCL { namespace DeviceBackend {
static bool hasBarriers(llvm::Module *pModule) {
  for (llvm::Module::iterator it = pModule->begin(), e = pModule->end();
       it != e; ++it) {
    llvm::Function *f = it;
    // If name of function contain the word 'barrier', assume that
    // the module calls a 'barrier' function.
    if (f->getName().find("barrier") != std::string::npos) {
      return true;
    }
  }
  return false;
}

/// createStandardModulePasses - Add the standard module passes.  This is
/// expected to be run after the standard function passes.
static inline void
createStandardLLVMPasses(llvm::PassManagerBase *PM,
                         unsigned OptLevel,
                         bool OptimizeSize,
                         bool UnitAtATime,
                         bool UnrollLoops,
                         bool SimplifyLibCalls,
                         bool allowAllocaModificationOpt,
                         bool isDBG,
                         bool HasGatherScatter) {
  if (OptLevel == 0) {
    return;
  }

  if (UnitAtATime) {
    PM->add(llvm::createGlobalOptimizerPass()); // Optimize out global vars
    PM->add(llvm::createIPSCCPPass());             // IP SCCP
    PM->add(llvm::createDeadArgEliminationPass()); // Dead argument elimination
  }
  PM->add(llvm::createInstructionSimplifierPass());
  PM->add(llvm::createInstructionCombiningPass()); // Clean up after IPCP & DAE
  PM->add(llvm::createCFGSimplificationPass());    // Clean up after IPCP & DAE

  if (UnitAtATime)
    PM->add(llvm::createFunctionAttrsPass()); // Set readonly/readnone attrs
  if (OptLevel > 2)
    PM->add(llvm::createArgumentPromotionPass()); // Scalarize uninlined fn args

  // A workaround to fix regression in sgemm on CPU and not causing new
  // regression on Machine with Gather Scatter
  int sroaArrSize = 16;
  if (HasGatherScatter)
    sroaArrSize = -1;
  // Break up aggregate allocas
  PM->add(llvm::createScalarReplAggregatesPass(256, true, -1, sroaArrSize, 64));
  if (SimplifyLibCalls)
    PM->add(llvm::createSimplifyLibCallsPass()); // Library Call Optimizations
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
  PM->add(llvm::createLoopUnswitchPass(OptimizeSize || OptLevel < 3));
  PM->add(llvm::createInstructionCombiningPass());
  PM->add(llvm::createInstructionSimplifierPass());
  PM->add(llvm::createIndVarSimplifyPass()); // Canonicalize indvars
  PM->add(llvm::createLoopDeletionPass());   // Delete dead loops
  if (UnrollLoops) {
    PM->add(llvm::createLoopUnrollPass(512, 0, 0)); // Unroll small loops
  }
  if (!isDBG) {
    PM->add(llvm::createFunctionInliningPass(4096)); // Inline (not only small)
                                                     // functions
  }
  // A workaround to fix regression in sgemm on CPU and not causing new
  // regression on Machine with Gather Scatter
  // Break up aggregate allocas
  PM->add(llvm::createScalarReplAggregatesPass(256, true, -1, sroaArrSize, 64));
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

Pass *createDataLayout(Module *M) {
#if LLVM_VERSION == 3425
  return new llvm::TargetData(M);
#else
  return new llvm::DataLayout(M);
#endif
}

static void populatePassesPreFailCheck(llvm::PassManagerBase &PM,
                                       llvm::Module *M,
                                       unsigned OptLevel,
                                       const intel::OptimizerConfig *pConfig,
                                       bool isOcl20,
                                       bool UnrollLoops) {
  DebuggingServiceType debugType =
      getDebuggingServiceType(pConfig->GetDebugInfoFlag());
  PrintIRPass::DumpIRConfig dumpIRAfterConfig(pConfig->GetIRDumpOptionsAfter());
  PrintIRPass::DumpIRConfig dumpIRBeforeConfig(
      pConfig->GetIRDumpOptionsBefore());
  bool HasGatherScatter = pConfig->GetCpuId().HasGatherScatter();

  PM.add(createDataLayout(M));
  if (isOcl20) {
    // OCL2.0 resolve block to static call
    PM.add(createResolveBlockToStaticCallPass());
  }
  if (OptLevel > 0) {
    PM.add(llvm::createCFGSimplificationPass());
    if (OptLevel == 1)
      PM.add(llvm::createPromoteMemoryToRegisterPass());
    else {
      // A workaround to fix regression in sgemm on CPU and not causing new
      // regression on Machine with Gather Scatter
      int sroaArrSize = 16;
      if (HasGatherScatter)
        sroaArrSize = -1;
      PM.add(
          llvm::createScalarReplAggregatesPass(256, true, -1, sroaArrSize, 64));
    }
    PM.add(llvm::createInstructionCombiningPass());
    PM.add(llvm::createInstructionSimplifierPass());
  }
  if (isOcl20) {
    // Flatten get_{local, global}_linear_id()
    PM.add(createLinearIdResolverPass());
  }

  // Adding module passes.
  if (dumpIRBeforeConfig.ShouldPrintPass(DUMP_IR_TARGERT_DATA)) {
    PM.add(createPrintIRPass(DUMP_IR_TARGERT_DATA, OPTION_IR_DUMPTYPE_BEFORE,
                             pConfig->GetDumpIRDir()));
  PM.add(createImplicitArgsAnalysisPass(M->getContext()));
  }
#ifdef __APPLE__
  PM.add(createClangCompatFixerPass());
#endif
  // OCL2.0 add Generic Address Static Resolution pass
  if (isOcl20) {
    // Static resolution of generic address space pointers
    if (OptLevel > 0) {
      PM.add(llvm::createPromoteMemoryToRegisterPass());
    }
    PM.add(createGenericAddressStaticResolutionPass());
  }
  PM.add(llvm::createBasicAliasAnalysisPass());
#ifndef __APPLE__
  if (dumpIRAfterConfig.ShouldPrintPass(DUMP_IR_TARGERT_DATA)) {
    PM.add(createPrintIRPass(DUMP_IR_TARGERT_DATA, OPTION_IR_DUMPTYPE_AFTER,
                             pConfig->GetDumpIRDir()));
  }
  if (!pConfig->GetLibraryModule() && getenv("DISMPF") != NULL)
    PM.add(createRemovePrefetchPass());
#endif //#ifndef __APPLE__
  PM.add(createShuffleCallToInstPass());
  bool has_bar = false;
  if (!pConfig->GetLibraryModule())
    has_bar = hasBarriers(M);
  bool allowAllocaModificationOpt = true;
  if (!pConfig->GetLibraryModule() && HasGatherScatter) {
    allowAllocaModificationOpt = false;
  }
  // When running the standard optimization passes, do not change the
  // loop-unswitch
  // pass on modules which contain barriers. This pass is illegal for
  // barriers.
  bool UnitAtATime = true;
  createStandardLLVMPasses(
      &PM, OptLevel, has_bar, // This parameter controls the unswitch pass
      UnitAtATime, UnrollLoops, false, allowAllocaModificationOpt,
      debugType != intel::None, HasGatherScatter);
  // check function pointers calls are gone after standard optimizations
  // if not compilation will fail
  PM.add(createDetectFuncPtrCalls());
  // check there is no recursion, if there is fail compilation
  PM.add(createDetectRecursionPass());
}

static void populatePassesPostFailCheck(llvm::PassManagerBase &PM,
                                        llvm::Module *M,
                                        llvm::Module *pRtlModule,
                                        unsigned OptLevel,
                                        const intel::OptimizerConfig *pConfig,
                                        std::vector<std::string> &UndefinedExternals,
                                        bool isOcl20,
                                        bool UnrollLoops) {
  bool isProfiling = pConfig->GetProfilingFlag();
  bool DisableSimplifyLibCalls = true;
  bool HasGatherScatter = pConfig->GetCpuId().HasGatherScatter();
  DebuggingServiceType debugType = getDebuggingServiceType(pConfig->GetDebugInfoFlag());
  PrintIRPass::DumpIRConfig dumpIRAfterConfig(pConfig->GetIRDumpOptionsAfter());
  PrintIRPass::DumpIRConfig dumpIRBeforeConfig(pConfig->GetIRDumpOptionsBefore());
  PM.add(createDataLayout(M));
  PM.add(createBuiltinLibInfoPass(pRtlModule, ""));

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
  

  PM.add(llvm::createBasicAliasAnalysisPass());

  if (isOcl20) {
    // OCL2.0 Extexecution.
    // clone block_invoke functions to kernels
    PM.add(createCloneBlockInvokeFuncToKernelPass());
  }
  
  // Should be called before vectorizer!
  PM.add((llvm::Pass*)createInstToFuncCallPass(HasGatherScatter));

  PM.add(createDuplicateCalledKernelsPass());

  if ( debugType == intel::None && !pConfig->GetLibraryModule() ) {
    PM.add(createKernelAnalysisPass());
    PM.add(createCLWGLoopBoundariesPass());
    PM.add(llvm::createDeadCodeEliminationPass());
    PM.add(llvm::createCFGSimplificationPass());
  }

  // In Apple build TRANSPOSE_SIZE_1 is not declared
  if( pConfig->GetTransposeSize() != 1 /*TRANSPOSE_SIZE_1*/
    && debugType == intel::None
    && OptLevel != 0)
  {
#ifndef __APPLE__
    // In profiling mode remove llvm.dbg.value calls
    // before vectorizer.
    if (isProfiling) {
      PM.add(createProfilingInfoPass());
    }

    if(dumpIRBeforeConfig.ShouldPrintPass(DUMP_IR_VECTORIZER)){
        PM.add(createPrintIRPass(DUMP_IR_VECTORIZER,
               OPTION_IR_DUMPTYPE_BEFORE, pConfig->GetDumpIRDir()));
    }
#endif //#ifndef __APPLE__
    if(pRtlModule != NULL) {
        PM.add(createVectorizerPass(pRtlModule, pConfig));
    }
#ifndef __APPLE__
    if (dumpIRAfterConfig.ShouldPrintPass(DUMP_IR_VECTORIZER)) {
      PM.add(createPrintIRPass(DUMP_IR_VECTORIZER, OPTION_IR_DUMPTYPE_AFTER,
                               pConfig->GetDumpIRDir()));
    }
    if (!HasGatherScatter) {
      PM.add(createReduceAlignmentPass());
    }
#endif //#ifndef __APPLE__
  }
#ifdef _DEBUG
  PM.add(llvm::createVerifierPass());
#endif

  // The ShiftZeroUpperBits pass should be added after the vectorizer because the vectorizer
  // may transform scalar shifts into vector shifts, and we want this pass to fix all vector
  // shift in this module.
  PM.add(createShiftZeroUpperBitsPass());
  PM.add(createPreventDivisionCrashesPass());
  // We need InstructionCombining and GVN passes after ShiftZeroUpperBits, PreventDivisionCrashes passes
  // to optimize redundancy introduced by those passes
  if ( debugType == intel::None ) {
    PM.add(llvm::createInstructionCombiningPass());
    PM.add(llvm::createGVNPass());
  }
#ifndef __APPLE__
  // The debugType enum and isProfiling flag are mutually exclusive, with precedence
  // given to debugType.
  //
  if (debugType == Simulator) {
    // DebugInfo pass must run before Barrier pass when debugging with simulator
    PM.add(createDebugInfoPass());
  } else if (isProfiling) {
    PM.add(createProfilingInfoPass());
  }
#endif
   // Get Some info about the kernel
   // should be called before BarrierPass and createPrepareKernelArgsPass
   if(pRtlModule != NULL) {
     PM.add(createKernelInfoWrapperPass());
   }

  if (isOcl20) {
    // Resolve (dynamically) generic address space pointers which are relevant for
    // correct execution
    PM.add(createGenericAddressDynamicResolutionPass());
    // No need to run function inlining pass here, because if there are still
    // non-inlined functions left - then we don't have to inline new ones.
  }

  // Adding WG loops
  if (!pConfig->GetLibraryModule()){
    if ( debugType == intel::None ) {
      PM.add(createCLWGLoopCreatorPass());
    }
    PM.add(createBarrierMainPass(debugType));

    // After adding loops run loop optimizations.
    if( debugType == intel::None ) {
      PM.add(createCLBuiltinLICMPass());
      PM.add(llvm::createLICMPass());
#ifdef __APPLE__
      // Workaround due to a bug in LICM, need to break the Loop passes flow after LICM.
      // TODO: remove it after fixing the bug in LICM.
      PM.add(llvm::createVerifierPass());
#endif //#ifdef __APPLE__
      PM.add(createLoopStridedCodeMotionPass());
      PM.add(createCLStreamSamplerPass());
    }
  }

  if( pConfig->GetRelaxedMath() ) {
    PM.add(createRelaxedPass());
  }

  // The following three passes (AddImplicitArgs/ResolveWICall/LocalBuffer)
  // must run before createBuiltInImportPass!
  if(!pConfig->GetLibraryModule())
  {
    PM.add(createAddImplicitArgsPass());
    PM.add(createResolveWICallPass());
    PM.add(createLocalBuffersPass(debugType == Native));
    // clang converts OCL's local to global.
    // createLocalBuffersPass changes the local allocation from global to a kernel argument.
    // The next pass createGlobalOptimizerPass cleans the unused global allocation in order to make sure
    // we will not allocate redundant space on the jit
    if (debugType != Native)
      PM.add(llvm::createGlobalOptimizerPass());
  }

#ifdef _DEBUG
  PM.add(llvm::createVerifierPass());
#endif

  // This pass checks if the module uses an undefined function or not
  // assumption: should run after WI function resolving
  PM.add(createUndifinedExternalFunctionsPass(UndefinedExternals));

  if(pRtlModule != NULL) {
    PM.add(createBuiltInImportPass()); // Inline BI function
    //Need to convert shuffle calls to shuffle IR before running inline pass on built-ins
    PM.add(createShuffleCallToInstPass());
  }

  //funcPassMgr->add(new intel::SelectLower());

#ifdef _DEBUG
  PM.add(llvm::createVerifierPass());
#endif

  if (debugType == intel::None) {
    if (HasGatherScatter)
      PM.add(llvm::createFunctionInliningPass(4096)); // Inline (not only small) functions.
    else
      PM.add(llvm::createFunctionInliningPass());     // Inline small functions
  }

  if (debugType == intel::None) {
    PM.add(llvm::createArgumentPromotionPass());    // Scalarize uninlined fn args

    if (!DisableSimplifyLibCalls) {
      PM.add(llvm::createSimplifyLibCallsPass());   // Library Call Optimizations
    }
    PM.add(llvm::createInstructionCombiningPass()); // Cleanup for scalarrepl.
    PM.add(llvm::createDeadStoreEliminationPass()); // Delete dead stores
    PM.add(llvm::createAggressiveDCEPass());        // Delete dead instructions
    PM.add(llvm::createCFGSimplificationPass());    // Merge & remove BBs
    PM.add(llvm::createInstructionCombiningPass()); // Cleanup for scalarrepl.
#ifdef __APPLE__
    //Due to none default ABI, some built-ins are creating an allaca in middle of function.
    //Need to run scalar aggregation to get red of these alloca (after built-in import).
    //mem2reg pass is not enough! as it only handles alloca in first basic block.
    PM.add(llvm::createScalarReplAggregatesPass());
#else
    PM.add(llvm::createPromoteMemoryToRegisterPass());
#endif
  }

  // PrepareKernelArgsPass must run in debugging mode as well
  if (!pConfig->GetLibraryModule())
    PM.add(createPrepareKernelArgsPass());

  if ( debugType == intel::None ) {
    // These passes come after PrepareKernelArgs pass to eliminate the redundancy reducced by it
    PM.add(llvm::createFunctionInliningPass());           // Inline
    PM.add(llvm::createDeadCodeEliminationPass());        // Delete dead instructions
    PM.add(llvm::createCFGSimplificationPass());          // Simplify CFG
    PM.add(llvm::createInstructionCombiningPass());       // Instruction combining
    PM.add(llvm::createDeadStoreEliminationPass());       // Eliminated dead stores
    PM.add(llvm::createEarlyCSEPass());
    PM.add(llvm::createGVNPass());

#ifdef _DEBUG
    PM.add(llvm::createVerifierPass());
#endif
  }

  // Remove unneeded functions from the module.
  // *** keep this optimization last, or at least after function inlining! ***
  if (!pConfig->GetLibraryModule())
    PM.add(createModuleCleanupPass());

#ifndef __APPLE__
  // Add prefetches if useful for micro-architecture, if not in debug mode,
  // and don't change libraries
  if (debugType == None && !pConfig->GetLibraryModule() && HasGatherScatter) {
    int APFLevel = pConfig->GetAPFLevel();
    // do APF and following cleaning passes only if APF is not disabled
    if (APFLevel != APFLEVEL_0_DISAPF) {
      if (pConfig->GetCpuId().RequirePrefetch())
        PM.add(createPrefetchPassLevel(pConfig->GetAPFLevel()));

      PM.add(llvm::createDeadCodeEliminationPass());        // Delete dead instructions
      PM.add(llvm::createInstructionCombiningPass());       // Instruction combining
      PM.add(llvm::createGVNPass());
#ifdef _DEBUG
      PM.add(llvm::createVerifierPass());
#endif
    }
  }
  if (UnrollLoops && debugType == None) {
    PM.add(llvm::createLoopUnrollPass(4, 0, 0));          // Unroll small loops
  }
#endif
}

Optimizer::~Optimizer()
{
}

Optimizer::Optimizer( llvm::Module* pModule,
                      llvm::Module* pRtlModule,
                      const intel::OptimizerConfig* pConfig):

    m_pModule(pModule)
{

  DebuggingServiceType debugType =
      getDebuggingServiceType(pConfig->GetDebugInfoFlag());
#ifndef __APPLE__

  // Materializing the spir datalayout according to the triple.
  materializeSpirDataLayout(*pModule);
#endif //#ifndef __APPLE__

  unsigned int OptLevel = 3;
  if (pConfig->GetDisableOpt() || debugType != intel::None)
    OptLevel = 0;

  // Detect OCL2.0 compilation mode
  const bool isOcl20 = CompilationUtils::getCLVersionFromModuleOrDefault(
                           *pModule) >= OclVersion::CL_VER_2_0;
  bool UnrollLoops = true;
  // Add passes which will run unconditionally
  populatePassesPreFailCheck(m_PreFailCheckPM, pModule, OptLevel, pConfig,
                             isOcl20, UnrollLoops);

  // Add passes which will be run only if hasFunctionPtrCalls() and hasRecursion()
  // will return false
  populatePassesPostFailCheck(m_PostFailCheckPM, pModule, pRtlModule, OptLevel,
                              pConfig, m_undefinedExternalFunctions, isOcl20,
                              UnrollLoops);
}

void Optimizer::Optimize()
{
#ifndef __APPLE__
    std::auto_ptr<llvm::ModulePass> materializer(createSpirMaterializer());
    materializer->runOnModule(*m_pModule);
#endif
    m_PreFailCheckPM.run(*m_pModule);
    
    // if there are still unresolved functon pointer calls
    // after standard LLVM optimizations applied
    // it means blocks variable call cannot be resolved to static call
    // Interrupt optimizations and return.
    // Compilation will report failure
    if(hasFunctionPtrCalls()){
      return;
    }

    // if there are still recursive calls  after standard
    // LLVM optimizations applied  Compilation will report failure
    if(hasRecursion()){
      return;
    }

    m_PostFailCheckPM.run(*m_pModule);
}


bool Optimizer::hasUndefinedExternals() const
{
    return !m_undefinedExternalFunctions.empty();
}

const std::vector<std::string>& Optimizer::GetUndefinedExternals() const
{
    return m_undefinedExternalFunctions;
}

bool Optimizer::hasFunctionPtrCalls()
{
    return !GetFuncNames(true).empty();
}

bool Optimizer::hasRecursion()
{
    return !GetFuncNames(false).empty();
}

std::vector<std::string> Optimizer::GetFuncNames(bool funcsWithFuncPtrCalls)
{
    bool returnFunc = false;
    assert(m_pModule && "Module is NULL");
    std::vector<std::string> res;
    MetaDataUtils mdUtils(m_pModule);

    // check FunctionInfo exists
    if(mdUtils.empty_FunctionsInfo()){
      return std::vector<std::string>();
    }

    MetaDataUtils::FunctionsInfoMap::iterator i = mdUtils.begin_FunctionsInfo();
    MetaDataUtils::FunctionsInfoMap::iterator e = mdUtils.end_FunctionsInfo();
    for(; i != e; ++i ){
        llvm::Function * pFunc = i->first;
        Intel::FunctionInfoMetaDataHandle kimd = i->second;
        // If additional else-ifs are needed in order to examine other function properties,
        // better change the "bool callingFunc" to an enum and use switch-case.
        if(funcsWithFuncPtrCalls == true){  // if calling func = hasFunctionPtrCall
          returnFunc = kimd->isFuncPtrCallHasValue() && kimd->getFuncPtrCall();
        }
        else{ // if calling func = hasRecursion
          returnFunc = kimd->isHasRecursionHasValue() && kimd->getHasRecursion();
        }
        if(returnFunc){
          res.push_back(pFunc->getName());
          returnFunc = false;
        }
    }

    return res;
}

}}}
