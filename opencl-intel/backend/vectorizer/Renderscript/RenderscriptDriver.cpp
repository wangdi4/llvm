/* ****************************************************************************** *\

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in accordance with the terms of that agreement
Copyright(c) 2011 - 2013 Intel Corporation. All Rights Reserved.

\* ****************************************************************************** */


#include "RenderscriptDriver.h"
#include "VectorizerCore.h"
#include "MetaDataApi.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"

#include "llvm/Pass.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/Module.h"

#ifdef __DEBUG
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#endif //__DEBUG

using namespace llvm;

char intel::RenderscriptVectorizer::ID = 0;

extern "C" Pass* createSpecialCaseBuiltinResolverPass();
extern "C" FunctionPass* createVectorizerCorePass(const intel::OptimizerConfig*);
extern "C" Pass* createBuiltinLibInfoPass(SmallVector<Module*, 2> pRtlModuleList, std::string type);
/// -- DBG Utilities
// These functions used for debugging
#ifdef __DEBUG
extern "C" void dumpDebugPoint(std::string tag, std::string title) {
  static unsigned int dbg_counter = 0;
  std::stringstream fileName;
  fileName << "/sdcard/RS_VEC_DBGP_" << tag << "_" << title << "." << dbg_counter;
  std::ofstream out(fileName.str().c_str());
  ++dbg_counter;
  out.flush();
}
extern "C" void dumpModule(std::string tag,
  std::string title,
  llvm::Module& module) {
  static unsigned int pre_counter = 0;
  std::string buffer;
  llvm::raw_string_ostream stream(buffer);
  std::stringstream fileName;
  fileName << "/sdcard/RS_VEC_" << tag << "_" << title << "_" << pre_counter << ".ll";
  std::ofstream out(fileName.str().c_str());
  stream << module;
  ++pre_counter;
  stream.flush();
  out << buffer;
  out.flush();
}
#endif // __DEBUG
/// -- DBG Utilities

namespace intel {

OCL_INITIALIZE_PASS_BEGIN(RenderscriptVectorizer, "rsVec", "render script vectorizer pass", false, false)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
OCL_INITIALIZE_PASS_END(RenderscriptVectorizer, "rsVec", "render script vectorizer pass", false, false)

RenderscriptVectorizer::RenderscriptVectorizer(const OptimizerConfig* pConfig,
  SmallVectorImpl<Function*> &optimizerFunctions,
  SmallVectorImpl<int> &optimizerWidths) :
  ModulePass(ID),
  m_numOfKernels(0),
  m_isModuleVectorized(false),
  m_pConfig(pConfig),
  m_pCPUId(NULL),
  m_optimizerFunctions(&optimizerFunctions),
  m_optimizerWidths(&optimizerWidths)
{
  // init debug prints
  initializeLoopInfoWrapperPassPass(*PassRegistry::getPassRegistry());
  V_INIT_PRINT;
}

RenderscriptVectorizer::RenderscriptVectorizer() :
  ModulePass(ID),
  m_numOfKernels(0),
  m_isModuleVectorized(false),
  m_pConfig(NULL),
  m_pCPUId(NULL),
  m_optimizerFunctions(NULL),
  m_optimizerWidths(NULL)
{
  // init debug prints
  initializeLoopInfoWrapperPassPass(*PassRegistry::getPassRegistry());
  m_pCPUId = new Intel::CPUId();
  m_pConfig = new OptimizerConfig(*m_pCPUId,
            0,
            std::vector<int>(),
            std::vector<int>(),
            "",
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            0,
            1);
  V_INIT_PRINT;
}

RenderscriptVectorizer::~RenderscriptVectorizer()
{
  if (m_pCPUId) {
    // Can reach here only if run through opt using default constructor.
    // In this case need to delete following (internally allocated) pointers.
    delete m_pCPUId;
    delete m_pConfig;
  }
  // Close the debug log elegantly
  V_DESTROY_PRINT;
}


bool RenderscriptVectorizer::runOnModule(Module &M)
{
  V_PRINT(wrapper, "\nEntered Vectorizer Wrapper!\n");
  // set isVectorized and proper number of kernels to zero, in case vectorization fails
  m_numOfKernels = 0;
  m_isModuleVectorized = true;

  // check for some common module errors, before actually diving in
  NamedMDNode *KernelsMD = M.getNamedMetadata("rs.indexed.kernels");
  if (!KernelsMD)
  {
    V_PRINT(wrapper, "Failed to find annotation. Aborting!\n");
    return false;
  }
  m_numOfKernels = KernelsMD->getNumOperands();

  if (m_numOfKernels == 0)
  {
    V_PRINT(wrapper, "Num of kernels is 0. Aborting!\n");
    return false;
  }
  m_runtimeModuleList = getAnalysis<BuiltinLibInfo>().getBuiltinModules();
  if (m_runtimeModuleList.size() == 0)
  {
    V_PRINT(wrapper, "Failed to find runtime module. Aborting!\n");
    return false;
  }


  for (int i = 0, e = m_numOfKernels; i < e; i++) {
    MDNode *FuncInfo = KernelsMD->getOperand(i);
    Function* F = llvm::mdconst::dyn_extract<llvm::Function>(FuncInfo->getOperand(0));
    assert(F && "runOnModule : F is NULL");
    F = dyn_cast<Function>(F->stripPointerCasts());
    m_scalarFuncsList.push_back(F);
  }


  // Create the vectorizer core pass that will do the vectotrization work.
  VectorizerCore *vectCore = (VectorizerCore *)createVectorizerCorePass(m_pConfig);
  legacy::FunctionPassManager vectPM(&M);
  vectPM.add(createBuiltinLibInfoPass(getAnalysis<BuiltinLibInfo>().getBuiltinModules(), "rs"));
  vectPM.add(vectCore);


  funcsVector::iterator fi = m_scalarFuncsList.begin();
  funcsVector::iterator fe = m_scalarFuncsList.end();
  for (; fi != fe; ++fi)
  {
    // default values for non vectorized kernels.
    Function *vectFunc = 0;
    int vectFuncWidth = 1;

    if (*fi) {
      // Clone the kernel
      ValueToValueMapTy vmap;
      Function *clone = CloneFunction(*fi, vmap, nullptr);
      clone->setName("__Vectorized_." + (*fi)->getName());
      vectPM.run(*clone);
      if (vectCore->isFunctionVectorized()) {
        // if the function is successfully vectorized update vectFunc and width.
        vectFunc = clone;
        vectFuncWidth = vectCore->getPacketWidth();
      } else {
        // We can't or choose not to vectorize the kernel, erase the clone from the module.
        clone->eraseFromParent();
      }
    }
    V_ASSERT(vectFuncWidth > 0 && "vect width for non vectoized kernels should be 1");
    if (m_optimizerFunctions) {
      m_optimizerFunctions->push_back(vectFunc);
    }
    if (m_optimizerWidths) {
      m_optimizerWidths->push_back(vectFuncWidth);
    }

  }


  {
      legacy::PassManager mpm;
    mpm.add(createBuiltinLibInfoPass(getAnalysis<BuiltinLibInfo>().getBuiltinModules(), "rs"));
    mpm.add(createSpecialCaseBuiltinResolverPass());
    mpm.run(M);
  }

  V_DUMP_MODULE((&M));
  //////////////////////////////////////////////
  //////////////////////////////////////////////
  V_PRINT(wrapper, "\nCompleted Vectorizer Wrapper!\n");

  return m_isModuleVectorized;
}

} // Namespace intel


///////////////////////////////////////////////////////////////////////////////////////////////////
// Interface functions for vectorizer
///////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" Pass *createRenderscriptVectorizerPass(
  const intel::OptimizerConfig* pConfig,
  SmallVectorImpl<Function*> &optimizerFunctions,
  SmallVectorImpl<int> &optimizerWidths)
{
  return new intel::RenderscriptVectorizer(pConfig,
    optimizerFunctions, optimizerWidths);
}

extern "C" intel::OptimizerConfig* createRenderscriptConfiguration(int width)
{
  std::string dumpIRDir = "";
  std::vector<int> dumpIROptionAfter;
  std::vector<int> dumpIROptionBefore;

  Intel::CPUId cpuId(Intel::CPU_COREI7, Intel::CFS_SSE42, false);

  return new intel::OptimizerConfig(cpuId,
            width,
            dumpIROptionAfter,
            dumpIROptionBefore,
            dumpIRDir,
            false,
            false,
            false,
            false,
            false,
            false,
            false,
            0,
            1);
}

extern "C" void deleteRenderscriptConfiguration(intel::OptimizerConfig*& pConfig)
{
  V_ASSERT(NULL != pConfig && "Trying to delete a null object!");
  delete pConfig;
  pConfig = NULL;
}

