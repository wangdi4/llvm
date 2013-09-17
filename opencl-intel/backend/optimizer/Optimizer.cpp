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
#if LLVM_VERSION == 3200
#include "llvm/DataLayout.h"
#else
#include "llvm/Target/TargetData.h"
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
llvm::ModulePass *createBuiltInImportPass(llvm::Module* pRTModule);
llvm::ModulePass *createLocalBuffersPass(bool isNativeDebug);
llvm::ModulePass *createAddImplicitArgsPass();
llvm::ModulePass *createModuleCleanupPass();
llvm::ModulePass *createGenericAddressStaticResolutionPass();
llvm::ModulePass *createGenericAddressDynamicResolutionPass();
llvm::ModulePass *createPrepareKernelArgsPass();

void* destroyOpenclRuntimeSupport();
#ifdef __APPLE__
void* createAppleOpenclRuntimeSupport(const llvm::Module *runtimeModule);
llvm::Pass *createClangCompatFixerPass();
#else
llvm::ModulePass *createSpirMaterializer();
void materializeSpirDataLayout(llvm::Module&);
void* createVolcanoOpenclRuntimeSupport(const llvm::Module *runtimeModule);
llvm::FunctionPass *createPrefetchPassLevel(int level);
llvm::ModulePass * createRemovePrefetchPass();
llvm::ModulePass *createPrintIRPass(int option, int optionLocation, std::string dumpDir);
llvm::ModulePass* createDebugInfoPass(llvm::LLVMContext* llvm_context, const llvm::Module* pRTModule);
#endif
llvm::ModulePass *createResolveWICallPass();
llvm::ModulePass *createDetectFuncPtrCalls();
llvm::ModulePass *createCloneBlockInvokeFuncToKernelPass();
llvm::Pass *createResolveBlockToStaticCallPass();
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {
#ifndef __APPLE__
llvm::ModulePass* createProfilingInfoPass();
#endif //#ifndef __APPLE__

llvm::ModulePass *createUndifinedExternalFunctionsPass(std::vector<std::string> &undefinedExternalFunctions,
                                                       const std::vector<llvm::Module*>& runtimeModules );

llvm::ModulePass *createKernelInfoWrapperPass();

  /// createStandardModulePasses - Add the standard module passes.  This is
  /// expected to be run after the standard function passes.
  static inline void createStandardVolcanoModulePasses(llvm::PassManagerBase *PM,
                                                unsigned OptimizationLevel,
                                                bool OptimizeSize,
                                                bool UnitAtATime,
                                                bool UnrollLoops,
                                                bool SimplifyLibCalls,
                                                bool allowAllocaModificationOpt,
                                                bool isDBG,
                                                const intel::OptimizerConfig* pConfig) {
    if (OptimizationLevel == 0) {
      return;
    }

    if (UnitAtATime) {
      PM->add(llvm::createGlobalOptimizerPass());     // Optimize out global vars

      PM->add(llvm::createIPSCCPPass());              // IP SCCP
      PM->add(llvm::createDeadArgEliminationPass());  // Dead argument elimination
    }
    PM->add(llvm::createInstructionSimplifierPass());
    PM->add(llvm::createInstructionCombiningPass());  // Clean up after IPCP & DAE
    PM->add(llvm::createCFGSimplificationPass());     // Clean up after IPCP & DAE

    if (UnitAtATime)
      PM->add(llvm::createFunctionAttrsPass());       // Set readonly/readnone attrs
    if (OptimizationLevel > 2)
      PM->add(llvm::createArgumentPromotionPass());   // Scalarize uninlined fn args

    // Start of function pass.
    // A workaround to fix regression in sgemm on CPU and not causing new regression on MIC
    int sroaArrSize = -1;
    if (! pConfig->GetCpuId().HasGatherScatter())
      sroaArrSize = 16;
    PM->add(llvm::createScalarReplAggregatesPass(256, true, -1, sroaArrSize, 64));  // Break up aggregate allocas
    if (SimplifyLibCalls)
      PM->add(llvm::createSimplifyLibCallsPass());    // Library Call Optimizations
    PM->add(llvm::createEarlyCSEPass());              // Catch trivial redundancies
    PM->add(llvm::createInstructionSimplifierPass());
    PM->add(llvm::createInstructionCombiningPass());  // Cleanup for scalarrepl.
    PM->add(llvm::createJumpThreadingPass());         // Thread jumps.
    PM->add(llvm::createCorrelatedValuePropagationPass()); // Propagate conditionals
    PM->add(llvm::createCFGSimplificationPass());     // Merge & remove BBs
    PM->add(llvm::createInstructionCombiningPass());  // Combine silly seq's

    PM->add(llvm::createTailCallEliminationPass());   // Eliminate tail calls
    PM->add(llvm::createCFGSimplificationPass());     // Merge & remove BBs
    PM->add(llvm::createReassociatePass());           // Reassociate expressions
    PM->add(llvm::createLoopRotatePass());            // Rotate Loop
    PM->add(llvm::createLICMPass());                  // Hoist loop invariants
    PM->add(llvm::createLoopUnswitchPass(OptimizeSize || OptimizationLevel < 3));
    PM->add(llvm::createInstructionCombiningPass());
    PM->add(llvm::createInstructionSimplifierPass());
    PM->add(llvm::createIndVarSimplifyPass());        // Canonicalize indvars
    PM->add(llvm::createLoopDeletionPass());          // Delete dead loops
    if (UnrollLoops) {
      PM->add(llvm::createLoopUnrollPass(512, 0, 0)); // Unroll small loops
    }
    if (!isDBG) {
      PM->add(llvm::createFunctionInliningPass(4096)); //Inline (not only small) functions
    }
    // A workaround to fix regression in sgemm on CPU and not causing new regression on MIC
    PM->add(llvm::createScalarReplAggregatesPass(256, true, -1, sroaArrSize, 64));  // Break up aggregate allocas
    PM->add(llvm::createInstructionCombiningPass());  // Clean up after the unroller
    PM->add(llvm::createInstructionSimplifierPass());
    if (allowAllocaModificationOpt){
      if (OptimizationLevel > 1)
      PM->add(llvm::createGVNPass());                 // Remove redundancies
    PM->add(llvm::createMemCpyOptPass());             // Remove memcpy / form memset
    }
    PM->add(llvm::createSCCPPass());                  // Constant prop with SCCP

    // Run instcombine after redundancy elimination to exploit opportunities
    // opened up by them.
    PM->add(llvm::createInstructionCombiningPass());
    PM->add(llvm::createJumpThreadingPass());         // Thread jumps
    PM->add(llvm::createCorrelatedValuePropagationPass());
    PM->add(llvm::createDeadStoreEliminationPass());  // Delete dead stores
    PM->add(llvm::createAggressiveDCEPass());         // Delete dead instructions
    PM->add(llvm::createCFGSimplificationPass());     // Merge & remove BBs
    PM->add(llvm::createInstructionCombiningPass());  // Clean up after everything.

    if (UnitAtATime) {
      PM->add(llvm::createStripDeadPrototypesPass()); // Get rid of dead prototypes

      // GlobalOpt already deletes dead functions and globals, at -O3 try a
      // late pass of GlobalDCE.  It is capable of deleting dead cycles.
      if (OptimizationLevel > 2)
        PM->add(llvm::createGlobalDCEPass());         // Remove dead fns and globals.

      if (OptimizationLevel > 1)
        PM->add(llvm::createConstantMergePass());       // Merge dup global constants
    }
  }

  static inline void createStandardVolcanoFunctionPasses(llvm::PassManagerBase *PM,
                                                  unsigned OptimizationLevel,
                                                  const intel::OptimizerConfig* pConfig) {
    if (OptimizationLevel > 0) {
      PM->add(llvm::createCFGSimplificationPass());
      if (OptimizationLevel == 1)
        PM->add(llvm::createPromoteMemoryToRegisterPass());
      else {
      // A workaround to fix regression in sgemm on CPU and not causing new regression on MIC
        int sroaArrSize = -1;
        if (! pConfig->GetCpuId().HasGatherScatter())
          sroaArrSize = 16;
        PM->add(llvm::createScalarReplAggregatesPass(256, true, -1, sroaArrSize, 64));
      }
      PM->add(llvm::createInstructionCombiningPass());
      PM->add(llvm::createInstructionSimplifierPass());
    }
  }

Optimizer::~Optimizer()
{
  destroyOpenclRuntimeSupport();
}

Optimizer::Optimizer( llvm::Module* pModule,
                      llvm::Module* pRtlModule,
                      const intel::OptimizerConfig* pConfig):

    m_funcStandardLLVMPasses(pModule),
    m_pModule(pModule)
{
  using namespace intel;

#ifndef __APPLE__
  createVolcanoOpenclRuntimeSupport(pRtlModule);
#else
  createAppleOpenclRuntimeSupport(pRtlModule);
#endif
  bool UnitAtATime = true;
  bool DisableSimplifyLibCalls = true;
  DebuggingServiceType debugType = getDebuggingServiceType(pConfig->GetDebugInfoFlag());
#ifndef __APPLE__
  bool isProfiling = pConfig->GetProfilingFlag();

  PrintIRPass::DumpIRConfig dumpIRAfterConfig(pConfig->GetIRDumpOptionsAfter());
  PrintIRPass::DumpIRConfig dumpIRBeforeConfig(pConfig->GetIRDumpOptionsBefore());

  if(dumpIRBeforeConfig.ShouldPrintPass(DUMP_IR_TARGERT_DATA)){
    m_moduleStandardLLVMPasses.add(createPrintIRPass(DUMP_IR_TARGERT_DATA,
               OPTION_IR_DUMPTYPE_BEFORE, pConfig->GetDumpIRDir()));
  }

  // Materializing the spir datalayout according to the triple.
  materializeSpirDataLayout(*pModule);
#endif //#ifndef __APPLE__

  unsigned int uiOptLevel;
  if (pConfig->GetDisableOpt() || debugType != None) {
    uiOptLevel = 0;
  } else {
    uiOptLevel = 3;
  }

// Adding function passes.
#if LLVM_VERSION == 3200
  m_funcStandardLLVMPasses.add(new llvm::DataLayout(pModule));
#else
  m_funcStandardLLVMPasses.add(new llvm::TargetData(pModule));
#endif
  // Detect OCL2.0 compilation mode
  const bool isOcl20 =
    (CompilationUtils::getCLVersionFromModule(*pModule) >= CompilationUtils::CL_VER_2_0);
  
  // OCL2.0 resolve block to static call
  if(isOcl20)
    m_funcStandardLLVMPasses.add(createResolveBlockToStaticCallPass());

  createStandardVolcanoFunctionPasses(&m_funcStandardLLVMPasses, uiOptLevel, pConfig);

// Adding module passes.
// Add an appropriate DataLayout instance for this module...
#if LLVM_VERSION == 3200
  m_moduleStandardLLVMPasses.add(new llvm::DataLayout(pModule));
#else
  m_moduleStandardLLVMPasses.add(new llvm::TargetData(pModule));
#endif
#ifdef __APPLE__
  m_moduleStandardLLVMPasses.add(createClangCompatFixerPass());
#endif
   
  // OCL2.0 add Generic Address Static Resolution pass
  if (isOcl20) {
    // Static resolution of generic address space pointers
    if (uiOptLevel > 0) {
      m_moduleStandardLLVMPasses.add(llvm::createPromoteMemoryToRegisterPass());
    }
    m_moduleStandardLLVMPasses.add(createGenericAddressStaticResolutionPass());
    // Flatten get_{local, global}_linear_id()
    m_funcStandardLLVMPasses.add(createLinearIdResolverPass());
  }
  m_moduleStandardLLVMPasses.add(llvm::createBasicAliasAnalysisPass());
#ifndef __APPLE__
  if(dumpIRAfterConfig.ShouldPrintPass(DUMP_IR_TARGERT_DATA)){
    m_moduleStandardLLVMPasses.add(createPrintIRPass(DUMP_IR_TARGERT_DATA,
               OPTION_IR_DUMPTYPE_AFTER, pConfig->GetDumpIRDir()));
  }
  if (!pConfig->GetLibraryModule() && getenv("DISMPF") != NULL)
    m_moduleStandardLLVMPasses.add(createRemovePrefetchPass());
#endif //#ifndef __APPLE__

  m_moduleStandardLLVMPasses.add(createShuffleCallToInstPass());

  bool has_bar = false;
  if (!pConfig->GetLibraryModule())
    has_bar = hasBarriers(pModule);

  bool allowAllocaModificationOpt = true;
  if (!pConfig->GetLibraryModule() && pConfig->GetCpuId().HasGatherScatter()) {
    allowAllocaModificationOpt = false;
  }
  const bool unrollLoops = true;
  // When running the standard optimization passes, do not change the loop-unswitch
  // pass on modules which contain barriers. This pass is illegal for barriers.
  createStandardVolcanoModulePasses(
      &m_moduleStandardLLVMPasses,
      uiOptLevel,
      has_bar, // This parameter controls the unswitch pass
      UnitAtATime,
      unrollLoops,
      false,
      allowAllocaModificationOpt,
      debugType != None,
      pConfig);

  if (isOcl20) {
    // Repeat static resolution of generic address space pointers after 
    // LLVM IR was optimized
    m_modulePasses.add(createGenericAddressStaticResolutionPass());
    // No need to run function inlining pass here, because if there are still
    // non-inlined functions left - then we don't have to inline new ones.
  }

  m_modulePasses.add(llvm::createUnifyFunctionExitNodesPass());
  m_moduleStandardLLVMPasses.add(llvm::createUnifyFunctionExitNodesPass());
  
  // check function pointers calls are gone after standard optimizations
  // if not fail compilation
  m_moduleStandardLLVMPasses.add(createDetectFuncPtrCalls());

  // add DataLayout analysis to m_modulePasses
  // we need to add it once again since standardVolcanoModulePasses
  // has its own datalayout and basic analysis
  // !!! Volcano m_modulePasses should have its own datalayout analysis pass
  // since some LLVM optimizations in m_modulePasses (instcombine) are using their info
#if LLVM_VERSION == 3200
  m_modulePasses.add(new llvm::DataLayout(pModule));
#else
  m_modulePasses.add(new llvm::TargetData(pModule));
#endif
  m_modulePasses.add(llvm::createBasicAliasAnalysisPass());

  if (isOcl20) {
    // OCL2.0 Extexecution.
    // clone block_invoke functions to kernels
    m_modulePasses.add(createCloneBlockInvokeFuncToKernelPass());
  }
  
  // Should be called before vectorizer!
  m_modulePasses.add((llvm::Pass*)createInstToFuncCallPass(pConfig->GetCpuId().HasGatherScatter()));

  if ( debugType == None && !pConfig->GetLibraryModule() ) {
    m_modulePasses.add(createKernelAnalysisPass());
    m_modulePasses.add(createCLWGLoopBoundariesPass());
    m_modulePasses.add(llvm::createDeadCodeEliminationPass());
    m_modulePasses.add(llvm::createCFGSimplificationPass());

  }

  // In Apple build TRANSPOSE_SIZE_1 is not declared
  if( pConfig->GetTransposeSize() != 1 /*TRANSPOSE_SIZE_1*/
    && debugType == None
    && uiOptLevel != 0)
  {
#ifndef __APPLE__
    // In profiling mode remove llvm.dbg.value calls
    // before vectorizer.
    if (isProfiling) {
      m_modulePasses.add(createProfilingInfoPass());
    }

    if(dumpIRBeforeConfig.ShouldPrintPass(DUMP_IR_VECTORIZER)){
        m_modulePasses.add(createPrintIRPass(DUMP_IR_VECTORIZER,
               OPTION_IR_DUMPTYPE_BEFORE, pConfig->GetDumpIRDir()));
    }
#endif //#ifndef __APPLE__
    if(pRtlModule != NULL) {
        m_modulePasses.add(createVectorizerPass(pRtlModule, pConfig));
    }
#ifndef __APPLE__
    if(dumpIRAfterConfig.ShouldPrintPass(DUMP_IR_VECTORIZER)){
        m_modulePasses.add(createPrintIRPass(DUMP_IR_VECTORIZER,
               OPTION_IR_DUMPTYPE_AFTER, pConfig->GetDumpIRDir()));
    }
#endif //#ifndef __APPLE__
  }
#ifdef _DEBUG
  m_modulePasses.add(llvm::createVerifierPass());
#endif

  // The ShiftZeroUpperBits pass should be added after the vectorizer because the vectorizer
  // may transform scalar shifts into vector shifts, and we want this pass to fix all vector
  // shift in this module.
  m_modulePasses.add(createShiftZeroUpperBitsPass());
  m_modulePasses.add(createPreventDivisionCrashesPass());
  // We need InstructionCombining and GVN passes after ShiftZeroUpperBits, PreventDivisionCrashes passes
  // to optimize redundancy introduced by those passes
  if ( debugType == None ) {
    m_modulePasses.add(llvm::createInstructionCombiningPass());
    m_modulePasses.add(llvm::createGVNPass());
  }
#ifndef __APPLE__
  // The debugType enum and isProfiling flag are mutually exclusive, with precedence
  // given to debugType.
  //
  if (debugType == Simulator) {
    // DebugInfo pass must run before Barrier pass when debugging with simulator
    m_modulePasses.add(createDebugInfoPass(&pModule->getContext(), pRtlModule));
  } else if (isProfiling) {
    m_modulePasses.add(createProfilingInfoPass());
  }
#endif
   // Get Some info about the kernel
   // should be called before BarrierPass and createPrepareKernelArgsPass
   if(pRtlModule != NULL) {
     m_modulePasses.add(createKernelInfoWrapperPass());
   }

  if (isOcl20) {
    // Resolve (dynamically) generic address space pointers which are relevant for
    // correct execution
    m_modulePasses.add(createGenericAddressDynamicResolutionPass());
    // No need to run function inlining pass here, because if there are still
    // non-inlined functions left - then we don't have to inline new ones.
  }

  // Adding WG loops
  if (!pConfig->GetLibraryModule()){
    if ( debugType == None ) {
      m_modulePasses.add(createCLWGLoopCreatorPass());
    }
    m_modulePasses.add(createBarrierMainPass(debugType));

    // After adding loops run loop optimizations.
    if( debugType == None ) {
      m_modulePasses.add(createCLBuiltinLICMPass());
      m_modulePasses.add(llvm::createLICMPass());
#ifdef __APPLE__
      // Workaround due to a bug in LICM, need to break the Loop passes flow after LICM.
      // TODO: remove it after fixing the bug in LICM.
      m_modulePasses.add(llvm::createVerifierPass());
#endif //#ifdef __APPLE__
      m_modulePasses.add(createLoopStridedCodeMotionPass());
      m_modulePasses.add(createCLStreamSamplerPass());
    }
  }

  if( pConfig->GetRelaxedMath() ) {
    m_modulePasses.add(createRelaxedPass());
  }

  // The following three passes (AddImplicitArgs/ResolveWICall/LocalBuffer)
  // must run before createBuiltInImportPass!
  if(!pConfig->GetLibraryModule())
  {
    m_modulePasses.add(createAddImplicitArgsPass());
    m_modulePasses.add(createResolveWICallPass());
    m_modulePasses.add(createLocalBuffersPass(debugType == Native));
    // clang converts OCL's local to global.
    // createLocalBuffersPass changes the local allocation from global to a kernel argument.
    // The next pass createGlobalOptimizerPass cleans the unused global allocation in order to make sure
    // we will not allocate redundant space on the jit
    if (debugType != Native)
      m_modulePasses.add(llvm::createGlobalOptimizerPass());
  }

#ifdef _DEBUG
  m_modulePasses.add(llvm::createVerifierPass());
#endif

  // This pass checks if the module uses an undefined function or not
  // TODO : need to add the image library also
  // assumption: should run after WI function inlining
  std::vector<llvm::Module*> runtimeModules;
  if(pRtlModule != NULL) runtimeModules.push_back(pRtlModule);
  m_modulePasses.add(createUndifinedExternalFunctionsPass(m_undefinedExternalFunctions, runtimeModules));

  if(pRtlModule != NULL) {
    m_modulePasses.add((llvm::ModulePass*)createBuiltInImportPass(pRtlModule)); // Inline BI function
    //Need to convert shuffle calls to shuffle IR before running inline pass on built-ins
    m_modulePasses.add(createShuffleCallToInstPass());
  }

  //funcPassMgr->add(new intel::SelectLower());

#ifdef _DEBUG
  m_modulePasses.add(llvm::createVerifierPass());
#endif

  if ( debugType == None ) {
    if (pConfig->GetCpuId().HasGatherScatter())
      m_modulePasses.add(llvm::createFunctionInliningPass(4096));     // Inline (not only small) functions.
    else
      m_modulePasses.add(llvm::createFunctionInliningPass());     // Inline small functions
  }

  if ( debugType == None ) {
    m_modulePasses.add(llvm::createArgumentPromotionPass());        // Scalarize uninlined fn args

    if ( !DisableSimplifyLibCalls ) {
      m_modulePasses.add(llvm::createSimplifyLibCallsPass());   // Library Call Optimizations
    }
    m_modulePasses.add(llvm::createInstructionCombiningPass()); // Cleanup for scalarrepl.
    m_modulePasses.add(llvm::createDeadStoreEliminationPass());   // Delete dead stores
    m_modulePasses.add(llvm::createAggressiveDCEPass());          // Delete dead instructions
    m_modulePasses.add(llvm::createCFGSimplificationPass());      // Merge & remove BBs
    m_modulePasses.add(llvm::createInstructionCombiningPass()); // Cleanup for scalarrepl.
#ifdef __APPLE__
    //Due to none default ABI, some built-ins are creating an allaca in middle of function.
    //Need to run scalar aggregation to get red of these alloca (after built-in import).
    //mem2reg pass is not enough! as it only handles alloca in first basic block.
    m_modulePasses.add(llvm::createScalarReplAggregatesPass());
#else
    m_modulePasses.add(llvm::createPromoteMemoryToRegisterPass());
#endif
  }

  // PrepareKernelArgsPass must run in debugging mode as well
  if (!pConfig->GetLibraryModule())
    m_modulePasses.add(createPrepareKernelArgsPass());

  if ( debugType == None ) {
    // These passes come after PrepareKernelArgs pass to eliminate the redundancy reducced by it
    m_modulePasses.add(llvm::createFunctionInliningPass());           // Inline
    m_modulePasses.add(llvm::createDeadCodeEliminationPass());        // Delete dead instructions
    m_modulePasses.add(llvm::createCFGSimplificationPass());          // Simplify CFG
    m_modulePasses.add(llvm::createInstructionCombiningPass());       // Instruction combining
    m_modulePasses.add(llvm::createDeadStoreEliminationPass());       // Eliminated dead stores
    m_modulePasses.add(llvm::createEarlyCSEPass());
    m_modulePasses.add(llvm::createGVNPass());

#ifdef _DEBUG
    m_modulePasses.add(llvm::createVerifierPass());
#endif
  }

  // Remove unneeded functions from the module.
  // *** keep this optimization last, or at least after function inlining! ***
  if (!pConfig->GetLibraryModule())
    m_modulePasses.add(createModuleCleanupPass());

#ifndef __APPLE__
  // Add prefetches if useful for micro-architecture, if not in debug mode,
  // and don't change libraries
  if (debugType == None && !pConfig->GetLibraryModule() &&
      pConfig->GetCpuId().HasGatherScatter()) {
    int APFLevel = pConfig->GetAPFLevel();
    // do APF and following cleaning passes only if APF is not disabled
    if (APFLevel != APFLEVEL_0_DISAPF) {
      m_modulePasses.add(createPrefetchPassLevel(pConfig->GetAPFLevel()));

      m_modulePasses.add(llvm::createDeadCodeEliminationPass());        // Delete dead instructions
      m_modulePasses.add(llvm::createInstructionCombiningPass());       // Instruction combining
      m_modulePasses.add(llvm::createGVNPass());
#ifdef _DEBUG
      m_modulePasses.add(llvm::createVerifierPass());
#endif
    }
  }
  if (unrollLoops && debugType == None) {
    m_modulePasses.add(llvm::createLoopUnrollPass(4, 0, 0));          // Unroll small loops
  }
#endif
}

void Optimizer::Optimize()
{
#ifndef __APPLE__
    std::auto_ptr<llvm::ModulePass> materializer(createSpirMaterializer());
    materializer->runOnModule(*m_pModule);
#endif
    for (llvm::Module::iterator i = m_pModule->begin(), e = m_pModule->end(); i != e; ++i) {
        m_funcStandardLLVMPasses.run(*i);
    }
    m_moduleStandardLLVMPasses.run(*m_pModule);
    
    // if there are still unresolved functon pointer calls
    // after standard LLVM optimizations applied
    // it means blocks variable call cannot be resolved to static call
    // Interrupt optimizations and return.
    // Compilation will report failure
    if(hasFunctionPtrCalls()){
      return;
    }

    m_modulePasses.run(*m_pModule);
}

bool Optimizer::hasBarriers(llvm::Module *pModule)
{
    for (llvm::Module::iterator it = pModule->begin(),e=pModule->end();it!=e;++it) {
        llvm::Function* f = it;
        // If name of function contain the word 'barrier', assume that
        // the module calls a 'barrier' function.
        if (f->getName().find("barrier") != std::string::npos) {
            return true;
        }
    }
    return false;
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
  return !GetFunctionPtrCallNames().empty();
}

std::vector<std::string> Optimizer::GetFunctionPtrCallNames()
{
    assert(m_pModule && "Module is NULL");
    std::vector<std::string> res;
    MetaDataUtils mdUtils(m_pModule);
  
    // check FunctionInfo exists
    if(mdUtils.empty_FunctionsInfo())
        return std::vector<std::string>();

    MetaDataUtils::FunctionsInfoMap::iterator i = mdUtils.begin_FunctionsInfo();
    MetaDataUtils::FunctionsInfoMap::iterator e = mdUtils.end_FunctionsInfo();
    for(; i != e; ++i )
    {
        llvm::Function * pFunc = i->first;
        Intel::FunctionInfoMetaDataHandle kimd = i->second;
        if(kimd->isFuncPtrCallHasValue() && kimd->getFuncPtrCall() == true)
            res.push_back(pFunc->getName());
    }

    return res;
}

}}}
