/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2012).

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
#include "llvm/Support/StandardPasses.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/Analysis/Verifier.h"
#include "PrintIRPass.h"

extern "C" llvm::Pass *createVectorizerPass(const llvm::Module *runtimeModule, const intel::OptimizerConfig* pConfig);
extern "C" int getVectorizerWidths(llvm::Pass *V, llvm::SmallVectorImpl<int> &Widths);
extern "C" llvm::Pass *createBarrierMainPass(bool isDBG);
extern "C" unsigned int getBarrierStrideSize(llvm::Pass *pPass);

namespace Intel { namespace OpenCL { namespace DeviceBackend {

llvm::ModulePass *createRelaxedPass();
llvm::ModulePass *createBuiltInImportPass(llvm::Module* pRTModule);
llvm::ModulePass* createDebugInfoPass(llvm::LLVMContext* llvm_context, const llvm::Module* pRTModule);
llvm::ModulePass *createAddImplicitArgsPass(llvm::Pass* pVect, llvm::SmallVectorImpl<llvm::Function*> &vectFunctions);
llvm::ModulePass *createResolveWICallPass();
llvm::ModulePass *createUndifinedExternalFunctionsPass(std::vector<std::string> &undefinedExternalFunctions,
                                                       const std::vector<llvm::Module*>& runtimeModules );
llvm::ModulePass *createLocalBuffersPass();
llvm::ModulePass *createSvmlWrapperPass( llvm::LLVMContext *context);
llvm::ModulePass *createPrepareKernelArgsPass(llvm::SmallVectorImpl<llvm::Function*> &vectFunctions);

void getKernelInfoMap(llvm::ModulePass *pKUPath, std::map<const llvm::Function*, TLLVMKernelInfo>& infoMap);


Optimizer::Optimizer( llvm::Module* pModule,
                      llvm::Module* pRtlModule,
                      const intel::OptimizerConfig* pConfig):
    m_funcPasses(pModule),
    m_vectorizerPass(NULL),
    m_barrierPass(NULL),
    m_pModule(pModule),
    m_localBuffersPass(NULL)
{
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

  llvm::createStandardFunctionPasses(&m_funcPasses, uiOptLevel);

  bool has_bar = false;
  if (!pConfig->GetLibraryModule())
    has_bar = hasBarriers(pModule);

  // When running the standard optimization passes, do not change the loop-unswitch
  // pass on modules which contain barriers. This pass is illegal for barriers.
  llvm::createStandardModulePasses(
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
  
  if( pConfig->GetTransposeSize() != TRANSPOSE_SIZE_1 && !isDBG) {
    if(dumpIRBeforeConfig.ShouldPrintPass(DUMP_IR_VECTORIZER)){
        m_modulePasses.add(createPrintIRPass(DUMP_IR_VECTORIZER,
               OPTION_IR_DUMPTYPE_BEFORE, pConfig->GetDumpIRDir()));
    }
    if(pRtlModule != NULL) {
        m_vectorizerPass = createVectorizerPass(pRtlModule, pConfig);
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

  if ( isDBG ) {
    // DebugInfo pass must run before Barrier pass!
    m_modulePasses.add(createDebugInfoPass(&pModule->getContext(), pRtlModule));
  }

  if (!pConfig->GetLibraryModule()){
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
    m_modulePasses.add(createAddImplicitArgsPass(m_vectorizerPass, m_vectFunctions));
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
    // TODO : uncomment these passes when code generation bug CSSD100007274 will be fixed
    m_modulePasses.add(llvm::createFunctionInliningPass());           // Inline
    m_modulePasses.add(llvm::createDeadCodeEliminationPass());        // Delete dead instructions
    m_modulePasses.add(llvm::createCFGSimplificationPass());          // Simplify CFG
    m_modulePasses.add(llvm::createInstructionCombiningPass());       // Instruction combining
    m_modulePasses.add(llvm::createDeadStoreEliminationPass());       // Eliminated dead stores
    
#ifdef _DEBUG
    m_modulePasses.add(llvm::createVerifierPass());
#endif
  }
}

void Optimizer::Optimize()
{
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

size_t Optimizer::getPrivateMemorySize()
{
    return (size_t)getBarrierStrideSize(m_barrierPass);
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

    llvm::SmallVector<int, 16>       vectWidths;

    getVectorizerWidths(m_vectorizerPass, vectWidths);
    assert(m_vectFunctions.size() == vectWidths.size());

    for(unsigned int i=0; i < m_vectFunctions.size(); i++) {
        vector.push_back(FunctionWidthPair(m_vectFunctions[i], vectWidths[i]));
    }
}

void Optimizer::GetKernelsInfo(KernelsInfoMap& map)
{
    getKernelInfoMap(m_localBuffersPass, map);
}

}}}