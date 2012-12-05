/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  Optimizer.cpp

\*****************************************************************************/

#include "Optimizer.h"
#include "VecConfig.h"
#include "exceptions.h"
#include "cl_device_api.h"
#include "cl_types.h"
#include "CPUDetect.h"
//#include "InstToFuncCall.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Pass.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Assembly/PrintModulePass.h"
#include "PrintIRPass.h"
#include "debuggingservicetype.h"

extern "C" void* createInstToFuncCallPass(bool);
extern "C" llvm::ModulePass *createPrintIRPass(int option, int optionLocation, std::string dumpDir);

extern "C" llvm::Pass *createVectorizerPass(const llvm::Module *runtimeModule,
                                            const intel::OptimizerConfig* pConfig,
                                            llvm::SmallVectorImpl<llvm::Function*> &optimizerFunctions,
                                            llvm::SmallVectorImpl<int> &optimizerWidths);
extern "C" llvm::Pass *createBarrierMainPass(intel::DebuggingServiceType debugType);
extern "C" void getBarrierStrideSize(llvm::Pass *pPass, std::map<std::string, unsigned int>& bufferStrideMap);

extern "C" llvm::ModulePass* createCLWGLoopCreatorPass(llvm::SmallVectorImpl<llvm::Function*> *optimizerFunctions,
                                                       llvm::SmallVectorImpl<int> *optimizerWidths);
extern "C" llvm::ModulePass* createCLWGLoopBoundariesPass();
extern "C" llvm::Pass* createCLBuiltinLICMPass();
extern "C" llvm::Pass* createLoopStridedCodeMotionPass();

extern "C" void* createOpenclRuntimeSupport(const llvm::Module *runtimeModule);
extern "C" void* destroyOpenclRuntimeSupport();
extern "C" llvm::Pass *createPreventDivisionCrashesPass();
extern "C" llvm::Pass *createShiftZeroUpperBitsPass();
extern "C" llvm::Pass *createShuffleCallToInstPass();
extern "C" llvm::Pass *createRelaxedPass();
extern "C" llvm::ModulePass *createKernelAnalysisPass();
extern "C" llvm::ModulePass *createBuiltInImportPass(llvm::Module* pRTModule);

extern "C" llvm::FunctionPass *createPrefetchPass();
 
namespace Intel { namespace OpenCL { namespace DeviceBackend {

llvm::ModulePass* createDebugInfoPass(llvm::LLVMContext* llvm_context, const llvm::Module* pRTModule);
llvm::ModulePass* createImplicitGlobalIdPass(llvm::LLVMContext* llvm_context, const llvm::Module* pRTModule);
llvm::ModulePass* createProfilingInfoPass();
llvm::ModulePass *createAddImplicitArgsPass(llvm::SmallVectorImpl<llvm::Function*> &vectFunctions);
llvm::ModulePass *createResolveWICallPass();
llvm::ModulePass *createUndifinedExternalFunctionsPass(std::vector<std::string> &undefinedExternalFunctions,
                                                       const std::vector<llvm::Module*>& runtimeModules );
llvm::ModulePass *createLocalBuffersPass(bool isNativeDebug);
llvm::ModulePass *createPrepareKernelArgsPass(llvm::SmallVectorImpl<llvm::Function*> &vectFunctions);
llvm::ModulePass *createModuleCleanupPass(llvm::SmallVectorImpl<llvm::Function*> &vectFunctions);
llvm::ModulePass *createKernelInfoWrapperPass();

void getKernelInfoMap(llvm::ModulePass *pKUPath, std::map<const llvm::Function*, TLLVMKernelInfo>& infoMap);
void getKernelLocalBufferInfoMap(llvm::ModulePass *pKUPath, std::map<const llvm::Function*, TLLVMKernelInfo>& infoMap);
 
void getKernelInfoMap(llvm::ModulePass *pKUPath, std::map<std::string, TKernelInfo>& infoMap);



  /// createStandardModulePasses - Add the standard module passes.  This is
  /// expected to be run after the standard function passes.
  static inline void createStandardVolcanoModulePasses(llvm::PassManagerBase *PM,
                                                unsigned OptimizationLevel,
                                                bool OptimizeSize,
                                                bool UnitAtATime,
                                                bool UnrollLoops,
                                                bool SimplifyLibCalls,
                                                bool allowAllocaModificationOpt,
                                                bool isDBG) {
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
    PM->add(llvm::createScalarReplAggregatesPass(256));  // Break up aggregate allocas
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
      PM->add(llvm::createLoopUnrollPass(512, 0, 0));          // Unroll small loops
    }
    if (!isDBG)
      PM->add(llvm::createFunctionInliningPass(4096)); //Inline (not only small) functions
    PM->add(llvm::createScalarReplAggregatesPass(256));  // Break up aggregate allocas
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
                                                  unsigned OptimizationLevel) {
    if (OptimizationLevel > 0) {
      PM->add(llvm::createCFGSimplificationPass());
      if (OptimizationLevel == 1)
        PM->add(llvm::createPromoteMemoryToRegisterPass());
      else
        PM->add(llvm::createScalarReplAggregatesPass(256));
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
    m_funcPasses(pModule),
    m_vectorizerPass(NULL),
    m_barrierPass(NULL),
    m_pModule(pModule),
    m_localBuffersPass(NULL)
{
  using namespace intel;

  createOpenclRuntimeSupport(pRtlModule);
  bool UnitAtATime LLVM_BACKEND_UNUSED = true;
  bool DisableSimplifyLibCalls = true;
  DebuggingServiceType debugType = getDebuggingServiceType(pConfig->GetDebugInfoFlag());
  bool isProfiling = pConfig->GetProfilingFlag();
  PrintIRPass::DumpIRConfig dumpIRAfterConfig(pConfig->GetIRDumpOptionsAfter());
  PrintIRPass::DumpIRConfig dumpIRBeforeConfig(pConfig->GetIRDumpOptionsBefore());

  if(dumpIRBeforeConfig.ShouldPrintPass(DUMP_IR_TARGERT_DATA)){
    m_modulePasses.add(createPrintIRPass(DUMP_IR_TARGERT_DATA,
               OPTION_IR_DUMPTYPE_BEFORE, pConfig->GetDumpIRDir()));
  }
  
  // Add an appropriate TargetData instance for this module...
  m_modulePasses.add(new llvm::TargetData(pModule));
  m_modulePasses.add(llvm::createBasicAliasAnalysisPass());
  m_funcPasses.add(new llvm::TargetData(pModule));

  if(dumpIRAfterConfig.ShouldPrintPass(DUMP_IR_TARGERT_DATA)){
    m_modulePasses.add(createPrintIRPass(DUMP_IR_TARGERT_DATA,
               OPTION_IR_DUMPTYPE_AFTER, pConfig->GetDumpIRDir()));
  }

  m_modulePasses.add(createShuffleCallToInstPass());

  unsigned int uiOptLevel;
  if (pConfig->GetDisableOpt() || debugType != None) {
    uiOptLevel = 0;
  } else {
    uiOptLevel = 3;
  }

  createStandardVolcanoFunctionPasses(&m_funcPasses, uiOptLevel);

  bool has_bar = false;
  if (!pConfig->GetLibraryModule())
    has_bar = hasBarriers(pModule);

  bool allowAllocaModificationOpt = true;
  if (!pConfig->GetLibraryModule() && pConfig->GetCpuId().IsMIC()) {
    allowAllocaModificationOpt = false;
  }
  // When running the standard optimization passes, do not change the loop-unswitch
  // pass on modules which contain barriers. This pass is illegal for barriers.
  createStandardVolcanoModulePasses(
      &m_modulePasses,
      uiOptLevel,
      has_bar, // This parameter controls the unswitch pass
      true,
      true,
      false,
      allowAllocaModificationOpt,
      debugType != None);

  m_modulePasses.add(llvm::createUnifyFunctionExitNodesPass());
  
  // Should be called before vectorizer!
  m_modulePasses.add((llvm::Pass*)createInstToFuncCallPass(pConfig->GetCpuId().IsMIC()));

  if ( debugType == None && !pConfig->GetLibraryModule()) {
    m_modulePasses.add(createKernelAnalysisPass());
    m_modulePasses.add(createCLWGLoopBoundariesPass());
    m_modulePasses.add(llvm::createDeadCodeEliminationPass());
    m_modulePasses.add(llvm::createCFGSimplificationPass());

  }

  if( pConfig->GetTransposeSize() != TRANSPOSE_SIZE_1
    && debugType == None
    && uiOptLevel != 0)
  {
    // In profiling mode remove llvm.dbg.value calls 
    // before vectorizer.
    if (isProfiling) {
      m_modulePasses.add(createProfilingInfoPass());
    }

    if(dumpIRBeforeConfig.ShouldPrintPass(DUMP_IR_VECTORIZER)){
        m_modulePasses.add(createPrintIRPass(DUMP_IR_VECTORIZER,
               OPTION_IR_DUMPTYPE_BEFORE, pConfig->GetDumpIRDir()));
    }
    if(pRtlModule != NULL) {
        m_vectorizerPass = createVectorizerPass(pRtlModule, pConfig, 
                                                m_vectFunctions, m_vectWidths);
        m_modulePasses.add(m_vectorizerPass);
    }
    if(dumpIRAfterConfig.ShouldPrintPass(DUMP_IR_VECTORIZER)){
        m_modulePasses.add(createPrintIRPass(DUMP_IR_VECTORIZER,
               OPTION_IR_DUMPTYPE_AFTER, pConfig->GetDumpIRDir()));
    }
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
  // to optimize redundancy entroduced by those passes
  if ( debugType == None ) {
    m_modulePasses.add(llvm::createInstructionCombiningPass());
    m_modulePasses.add(llvm::createGVNPass());
  }
 
  // The debugType enum and isProfiling flag are mutually exclusive, with precedence
  // given to debugType.
  //
  if (debugType == Simulator) {
    // DebugInfo pass must run before Barrier pass when debugging with simulator
    m_modulePasses.add(createDebugInfoPass(&pModule->getContext(), pRtlModule));
  } else if (isProfiling) {
    m_modulePasses.add(createProfilingInfoPass());
  }
   
   // Get Some info about the kernel
   // should be called before BarrierPass and createPrepareKernelArgsPass
   if(pRtlModule != NULL) {
     m_kernelInfoPass = createKernelInfoWrapperPass();
     m_modulePasses.add(m_kernelInfoPass);
   }

  // Adding WG loops
  if (!pConfig->GetLibraryModule()){
    m_modulePasses.add(createCLWGLoopCreatorPass(&m_vectFunctions, &m_vectWidths));
    m_barrierPass = createBarrierMainPass(debugType);
    m_modulePasses.add(m_barrierPass);
    
    // After adding loops run loop optimizations.
    if( debugType == None ) {
      m_modulePasses.add(createCLBuiltinLICMPass());
      m_modulePasses.add(llvm::createLICMPass());
      m_modulePasses.add(createLoopStridedCodeMotionPass());
    }
  }

  if( pConfig->GetRelaxedMath() ) {
    m_modulePasses.add(createRelaxedPass());
  }

  // The following three passes (AddImplicitArgs/ResolveWICall/LocalBuffer)
  // must run before createBuiltInImportPass!
  if(!pConfig->GetLibraryModule())
  {
    m_modulePasses.add(createAddImplicitArgsPass(m_vectFunctions));
    m_modulePasses.add(createResolveWICallPass());
    m_localBuffersPass = createLocalBuffersPass(debugType == Native);
    m_modulePasses.add(m_localBuffersPass);
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

  if(pRtlModule != NULL)
    m_modulePasses.add((llvm::ModulePass*)createBuiltInImportPass(pRtlModule)); // Inline BI function

  //funcPassMgr->add(new intel::SelectLower());

#ifdef _DEBUG
  m_modulePasses.add(llvm::createVerifierPass());
#endif

  if ( debugType == None ) {
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
    m_modulePasses.add(llvm::createPromoteMemoryToRegisterPass());
  }

  // PrepareKernelArgsPass must run in debugging mode as well
  if (!pConfig->GetLibraryModule())
    m_modulePasses.add(createPrepareKernelArgsPass(m_vectFunctions));

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
      m_modulePasses.add(createModuleCleanupPass(m_vectFunctions));

    // Add prefetches only for MIC, if not in debug mode, and don't change the
    // library
    if (debugType == None && !pConfig->GetLibraryModule() &&
        pConfig->GetCpuId().GetCPU() == Intel::MIC_KNC) {
      m_modulePasses.add(createPrefetchPass());

      m_modulePasses.add(llvm::createDeadCodeEliminationPass());        // Delete dead instructions
      m_modulePasses.add(llvm::createInstructionCombiningPass());       // Instruction combining
      m_modulePasses.add(llvm::createGVNPass());
#ifdef _DEBUG
      m_modulePasses.add(llvm::createVerifierPass());
#endif
    }
}

void Optimizer::Optimize()
{
    m_vectFunctions.clear();
    m_vectWidths.clear();
    for (llvm::Module::iterator i = m_pModule->begin(), e = m_pModule->end(); i != e; ++i) {
        m_funcPasses.run(*i);
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

void Optimizer::getPrivateMemorySize(std::map<std::string, unsigned int>& bufferStrideMap)
{
    getBarrierStrideSize(m_barrierPass, bufferStrideMap);
}

bool Optimizer::hasUndefinedExternals() const 
{ 
    return !m_undefinedExternalFunctions.empty(); 
}

const std::vector<std::string>& Optimizer::GetUndefinedExternals() const 
{ 
    return m_undefinedExternalFunctions; 
}

void Optimizer::GetVectorizedFunctions(FunctionWidthVector& vector)
{
    vector.clear();

    if( NULL == m_vectorizerPass )
        return;

    for(unsigned int i=0; i < m_vectFunctions.size(); i++) {
        vector.push_back(FunctionWidthPair(m_vectFunctions[i], m_vectWidths[i]));
    }
}

void Optimizer::GetKernelsInfo(KernelsInfoMap& map)
{
    getKernelInfoMap(m_kernelInfoPass, map);
}

void Optimizer::GetKernelsLocalBufferInfo(KernelsLocalBufferInfoMap& map)
{
    getKernelLocalBufferInfoMap(m_localBuffersPass, map);
}

}}}
