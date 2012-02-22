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
#include "Program.h"
#include "ProgramBuilder.h"
#include "CompilerConfig.h"
#include "CPUDetect.h"
#include "InstToFuncCall.h"
#include "llvm/Module.h"
#include "llvm/Pass.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/Analysis/Verifier.h"
#include "PrintIRPass.h"

extern "C" llvm::Pass *createVectorizerPass(const llvm::Module *runtimeModule,
                                            const intel::OptimizerConfig* pConfig,
                                            llvm::SmallVectorImpl<Function*> &optimizerFunctions,
                                            llvm::SmallVectorImpl<int> &optimizerWidths);
extern "C" llvm::Pass *createBarrierMainPass(bool isDBG);
extern "C" void getBarrierStrideSize(Pass *pPass, std::map<std::string, unsigned int>& bufferStrideMap);

extern "C" llvm::ModulePass* createCLWGLoopCreatorPass(llvm::SmallVectorImpl<Function*> *optimizerFunctions,
                                                       llvm::SmallVectorImpl<int> *optimizerWidths);
extern "C" llvm::ModulePass* createCLWGLoopBoundariesPass();

extern "C" void* createOpenclRuntimeSupport(const Module *runtimeModule);
extern "C" void* destroyOpenclRuntimeSupport();
extern "C" llvm::Pass *createPreventDivisionCrashesPass();
extern "C" llvm::Pass *createShiftZeroUpperBitsPass();
extern "C" llvm::ModulePass *createKernelAnalysisPass();

namespace Intel { namespace OpenCL { namespace DeviceBackend {

llvm::ModulePass *createRelaxedPass();
llvm::ModulePass *createBuiltInImportPass(llvm::Module* pRTModule);
llvm::ModulePass* createDebugInfoPass(llvm::LLVMContext* llvm_context, const llvm::Module* pRTModule);
llvm::ModulePass *createAddImplicitArgsPass(llvm::SmallVectorImpl<llvm::Function*> &vectFunctions);
llvm::ModulePass *createResolveWICallPass();
llvm::ModulePass *createUndifinedExternalFunctionsPass(std::vector<std::string> &undefinedExternalFunctions,
                                                       const std::vector<llvm::Module*>& runtimeModules );
llvm::ModulePass *createLocalBuffersPass();
llvm::ModulePass *createSvmlWrapperPass( llvm::LLVMContext *context);
llvm::ModulePass *createPrepareKernelArgsPass(llvm::SmallVectorImpl<llvm::Function*> &vectFunctions);

void getKernelInfoMap(llvm::ModulePass *pKUPath, std::map<const llvm::Function*, TLLVMKernelInfo>& infoMap);


  /// createStandardModulePasses - Add the standard module passes.  This is
  /// expected to be run after the standard function passes.
  static inline void createStandardVolcanoModulePasses(PassManagerBase *PM,
                                                unsigned OptimizationLevel,
                                                bool OptimizeSize,
                                                bool UnitAtATime,
                                                bool UnrollLoops,
                                                bool SimplifyLibCalls,
                                                bool HaveExceptions,
                                                Pass *InliningPass) {
    if (OptimizationLevel == 0) {
      if (InliningPass)
        PM->add(InliningPass);
      return;
    }
    
    if (UnitAtATime) {
      PM->add(createGlobalOptimizerPass());     // Optimize out global vars
      
      PM->add(createIPSCCPPass());              // IP SCCP
      PM->add(createDeadArgEliminationPass());  // Dead argument elimination
    }
    PM->add(createInstructionSimplifierPass());
    PM->add(createInstructionCombiningPass());  // Clean up after IPCP & DAE
    PM->add(createCFGSimplificationPass());     // Clean up after IPCP & DAE
    
    // Start of CallGraph SCC passes.
    if (UnitAtATime && HaveExceptions)
      PM->add(createPruneEHPass());           // Remove dead EH info
    if (InliningPass)
      PM->add(InliningPass);
    if (UnitAtATime)
      PM->add(createFunctionAttrsPass());       // Set readonly/readnone attrs
    if (OptimizationLevel > 2)
      PM->add(createArgumentPromotionPass());   // Scalarize uninlined fn args
    
    // Start of function pass.
    PM->add(createScalarReplAggregatesPass(256));  // Break up aggregate allocas
    if (SimplifyLibCalls)
      PM->add(createSimplifyLibCallsPass());    // Library Call Optimizations
    PM->add(createEarlyCSEPass());              // Catch trivial redundancies
    PM->add(createInstructionSimplifierPass());
    PM->add(createInstructionCombiningPass());  // Cleanup for scalarrepl.
    PM->add(createJumpThreadingPass());         // Thread jumps.
    PM->add(createCorrelatedValuePropagationPass()); // Propagate conditionals
    PM->add(createCFGSimplificationPass());     // Merge & remove BBs
    PM->add(createInstructionCombiningPass());  // Combine silly seq's
    
    PM->add(createTailCallEliminationPass());   // Eliminate tail calls
    PM->add(createCFGSimplificationPass());     // Merge & remove BBs
    PM->add(createReassociatePass());           // Reassociate expressions
    PM->add(createLoopRotatePass());            // Rotate Loop
    PM->add(createLICMPass());                  // Hoist loop invariants
    PM->add(createLoopUnswitchPass(OptimizeSize || OptimizationLevel < 3));
    PM->add(createInstructionCombiningPass());  
    PM->add(createInstructionSimplifierPass());
    PM->add(createIndVarSimplifyPass());        // Canonicalize indvars
    PM->add(createLoopDeletionPass());          // Delete dead loops
    if (UnrollLoops)
      PM->add(createLoopUnrollPass());          // Unroll small loops
    PM->add(createInstructionCombiningPass());  // Clean up after the unroller
    PM->add(createInstructionSimplifierPass());
    if (OptimizationLevel > 1)
      PM->add(createGVNPass());                 // Remove redundancies
    PM->add(createMemCpyOptPass());             // Remove memcpy / form memset
    PM->add(createSCCPPass());                  // Constant prop with SCCP
  
    // Run instcombine after redundancy elimination to exploit opportunities
    // opened up by them.
    PM->add(createInstructionCombiningPass());
    PM->add(createJumpThreadingPass());         // Thread jumps
    PM->add(createCorrelatedValuePropagationPass());
    PM->add(createDeadStoreEliminationPass());  // Delete dead stores
    PM->add(createAggressiveDCEPass());         // Delete dead instructions
    PM->add(createCFGSimplificationPass());     // Merge & remove BBs
    PM->add(createInstructionCombiningPass());  // Clean up after everything.

    if (UnitAtATime) {
      PM->add(createStripDeadPrototypesPass()); // Get rid of dead prototypes

      // GlobalOpt already deletes dead functions and globals, at -O3 try a
      // late pass of GlobalDCE.  It is capable of deleting dead cycles.
      if (OptimizationLevel > 2)
        PM->add(createGlobalDCEPass());         // Remove dead fns and globals.
    
      if (OptimizationLevel > 1)
        PM->add(createConstantMergePass());       // Merge dup global constants
    }
  }

  static inline void createStandardVolcanoFunctionPasses(PassManagerBase *PM,
                                                  unsigned OptimizationLevel) {
    if (OptimizationLevel > 0) {
      PM->add(createCFGSimplificationPass());
      if (OptimizationLevel == 1)
        PM->add(createPromoteMemoryToRegisterPass());
      else
        PM->add(createScalarReplAggregatesPass(256));
      PM->add(createInstructionCombiningPass());
      PM->add(createInstructionSimplifierPass());
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
  createOpenclRuntimeSupport(pRtlModule);
  bool UnitAtATime LLVM_BACKEND_UNUSED = true;
  bool DisableSimplifyLibCalls = true;
  bool isDBG = pConfig->GetDebugInfoFlag();
  PrintIRPass::DumpIRConfig dumpIRAfterConfig(pConfig->GetIRDumpOptionsAfter());
  PrintIRPass::DumpIRConfig dumpIRBeforeConfig(pConfig->GetIRDumpOptionsBefore());

  if(dumpIRBeforeConfig.ShouldPrintPass(DUMP_IR_TARGERT_DATA)){
    m_modulePasses.add(createPrintIRPass(DUMP_IR_TARGERT_DATA,
               OPTION_IR_DUMPTYPE_BEFORE, pConfig->GetDumpIRDir()));
  }
  
  // Add an appropriate TargetData instance for this module...
  m_modulePasses.add(new llvm::TargetData(pModule));
  m_funcPasses.add(new llvm::TargetData(pModule));

  if(dumpIRAfterConfig.ShouldPrintPass(DUMP_IR_TARGERT_DATA)){
    m_modulePasses.add(createPrintIRPass(DUMP_IR_TARGERT_DATA,
               OPTION_IR_DUMPTYPE_AFTER, pConfig->GetDumpIRDir()));
  }

  unsigned int uiOptLevel;
  if (pConfig->GetDisableOpt() || isDBG) {
    uiOptLevel = 0;
  } else {
    uiOptLevel = 3;
  }

  createStandardVolcanoFunctionPasses(&m_funcPasses, uiOptLevel);

  bool has_bar = false;
  if (!pConfig->GetLibraryModule())
    has_bar = hasBarriers(pModule);

  // When running the standard optimization passes, do not change the loop-unswitch
  // pass on modules which contain barriers. This pass is illegal for barriers.
  createStandardVolcanoModulePasses(
      &m_modulePasses,
      uiOptLevel,
      has_bar, // This parameter controls the unswitch pass
      true,
      true,
      false,
      false,
      NULL
      );

  m_modulePasses.add(llvm::createUnifyFunctionExitNodesPass());
  
  if ( !isDBG && !pConfig->GetLibraryModule()) {
    m_modulePasses.add(createKernelAnalysisPass());
    m_modulePasses.add(createCLWGLoopBoundariesPass());
    m_modulePasses.add(llvm::createDeadCodeEliminationPass());
    m_modulePasses.add(llvm::createCFGSimplificationPass());

  }
  
  if( pConfig->GetTransposeSize() != TRANSPOSE_SIZE_1 && !isDBG) {
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
  if (! isDBG ) {
    m_modulePasses.add(llvm::createInstructionCombiningPass());
    m_modulePasses.add(llvm::createGVNPass());
  }
  
  if ( isDBG ) {
    // DebugInfo pass must run before Barrier pass!
    m_modulePasses.add(createDebugInfoPass(&pModule->getContext(), pRtlModule));
  }

  if (!pConfig->GetLibraryModule()){
    m_modulePasses.add(createCLWGLoopCreatorPass(&m_vectFunctions, &m_vectWidths));
    m_barrierPass = createBarrierMainPass(isDBG);
    m_modulePasses.add(m_barrierPass);
  }

  if( pConfig->GetRelaxedMath() ) {
    m_modulePasses.add(createRelaxedPass());
  }

  m_modulePasses.add(createInstToFuncCallPass());

  // The following three passes (AddImplicitArgs/ResolveWICall/LocalBuffer)
  // must run before createBuiltInImportPass!
  if(!pConfig->GetLibraryModule())
  {
    m_modulePasses.add(createAddImplicitArgsPass(m_vectFunctions));
    m_modulePasses.add(createResolveWICallPass());
    m_localBuffersPass = createLocalBuffersPass();
    m_modulePasses.add(m_localBuffersPass);
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
    m_modulePasses.add(createBuiltInImportPass(pRtlModule)); // Inline BI function

  //funcPassMgr->add(new intel::SelectLower());

#ifdef _DEBUG
  m_modulePasses.add(llvm::createVerifierPass());
#endif

  if ( !isDBG ) {
    m_modulePasses.add(llvm::createFunctionInliningPass());     // Inline small functions
  }

#ifdef _M_X64
  // TODO: remove the pass below once SVML will be aligned with Win64 ABI
  m_modulePasses.add(createSvmlWrapperPass( &m_pModule->getContext()));
#endif

  if ( !isDBG ) {
    m_modulePasses.add(llvm::createArgumentPromotionPass());        // Scalarize uninlined fn args
  
    if ( !DisableSimplifyLibCalls ) {
      m_modulePasses.add(llvm::createSimplifyLibCallsPass());   // Library Call Optimizations
    }
    m_modulePasses.add(llvm::createInstructionCombiningPass()); // Cleanup for scalarrepl.
    m_modulePasses.add(llvm::createDeadStoreEliminationPass());   // Delete dead stores
    m_modulePasses.add(llvm::createAggressiveDCEPass());          // Delete dead instructions
    m_modulePasses.add(llvm::createCFGSimplificationPass());      // Merge & remove BBs
    m_modulePasses.add(llvm::createInstructionCombiningPass()); // Cleanup for scalarrepl.
  }

  // PrepareKernelArgsPass must run in debugging mode as well
  if (!pConfig->GetLibraryModule())
    m_modulePasses.add(createPrepareKernelArgsPass(m_vectFunctions));

  if ( !isDBG ) {
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
        if (f->getNameStr().find("barrier") != std::string::npos) {
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
    getKernelInfoMap(m_localBuffersPass, map);
}

}}}