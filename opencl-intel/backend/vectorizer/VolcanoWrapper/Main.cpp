/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "Main.h"
#include "VectorizerCore.h"
#include "MetaDataApi.h"
#include "OclTune.h"

#include "llvm/Pass.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"

// Placeholders for debug log files
FILE * prtFile;
FILE * moduleDmp;

using namespace llvm;

char intel::Vectorizer::ID = 0;

extern "C" Pass* createSpecialCaseBuiltinResolverPass();
extern "C" FunctionPass* createVectorizerCorePass(const intel::OptimizerConfig*);
extern "C" Pass* createBuiltinLibInfoPass(llvm::SmallVector<llvm::Module*, 2> pRtlModuleList, std::string type);

namespace intel {

  Vectorizer::Vectorizer(llvm::SmallVector<llvm::Module*, 2> rtList, const OptimizerConfig* pConfig) :
  ModulePass(ID),
  m_numOfKernels(0),
  m_isModuleVectorized(false),
  m_pConfig(pConfig)
{
  m_runtimeModuleList = rtList;
  // init debug prints
  initializeLoopInfoWrapperPassPass(*PassRegistry::getPassRegistry());
  V_INIT_PRINT;
}

Vectorizer::~Vectorizer()
{
  // Close the debug log elegantly
  V_DESTROY_PRINT;
}


bool Vectorizer::runOnModule(Module &M)
{
  V_PRINT(wrapper, "\nEntered Vectorizer Wrapper!\n");
  // set isVectorized and proper number of kernels to zero, in case vectorization fails
  m_numOfKernels = 0;
  m_isModuleVectorized = true;

  Intel::MetaDataUtils mdUtils(&M);

  // check for some common module errors, before actually diving in
  if (mdUtils.empty_Kernels())
  {
    V_PRINT(wrapper, "Failed to find annotation. Aborting!\n");
    return false;
  }
  m_numOfKernels = mdUtils.size_Kernels();
  if (m_numOfKernels == 0)
  {
    V_PRINT(wrapper, "Num of kernels is 0. Aborting!\n");
    return false;
  }
  if (m_runtimeModuleList.size() == 0)
  {
    V_PRINT(wrapper, "Failed to find runtime module. Aborting!\n");
    return false;
  }


  for (Intel::MetaDataUtils::KernelsList::const_iterator i = mdUtils.begin_Kernels(), e = mdUtils.end_Kernels(); i != e; ++i) {
    Intel::KernelMetaDataHandle kmd = (*i);
    Function *F = kmd->getFunction();
    bool disableVect = false;

    //look for vector type hint metadata
    if (kmd->isVecTypeHintHasValue()) {
      Type* VTHTy = kmd->getVecTypeHint()->getType();
      if (!VTHTy->isFloatTy()     &&
          !VTHTy->isDoubleTy()    &&
          !VTHTy->isIntegerTy(8)  &&
          !VTHTy->isIntegerTy(16) &&
          !VTHTy->isIntegerTy(32) &&
          !VTHTy->isIntegerTy(64)) {
            disableVect = true;
      }
    }

    // Only add kernels to list, if they have scalar vec-type hint (or none)
    if (!disableVect)
      m_scalarFuncsList.push_back(F);
    else
      m_scalarFuncsList.push_back(NULL);
  }


  // Create the vectorizer core pass that will do the vectotrization work.
  VectorizerCore *vectCore = (VectorizerCore *)createVectorizerCorePass(m_pConfig);
  legacy::FunctionPassManager vectPM(&M);
  vectPM.add(createBuiltinLibInfoPass(getAnalysis<BuiltinLibInfo>().getBuiltinModules(), ""));
  vectPM.add(vectCore);


  funcsVector::iterator fi = m_scalarFuncsList.begin();
  funcsVector::iterator fe = m_scalarFuncsList.end();
  for (; fi != fe; ++fi)
  {
    // default values for non vectorized kernels.
    Function *vectFunc = 0;
    int vectFuncWidth = 1;
    unsigned int vectDim = 0;
    bool canUniteWorkgroups = false;

    if (*fi) {
      // Clone the kernel
      ValueToValueMapTy vmap;
      Function *clone = CloneFunction(*fi, vmap, nullptr);
      // [LLVM 3.6 UPGRADE] Set the vectorized function name manually at least until fix isn't done in LLVM
      // or solved in another way.
      clone->setName("__Vectorized_." + (*fi)->getName());
      M.getFunctionList().push_back(clone);

      // Todo: due to a bug in the metadata we can't save changes more than once
      // (even if we reinstantiate the metadata object after saving).
      // Until this is fixed, we send the scalar function directly to the vectorizer core.
      //Intel::KernelInfoMetaDataHandle vkimd = mdUtils.getOrInsertKernelsInfoItem(clone);
      //vkimd->setVectorizedKernel(NULL);
      //vkimd->setScalarizedKernel(*fi);
      //Save Metadata to the module
      //mdUtils.save(M.getContext());

      vectCore->setScalarFunc(*fi);

      vectPM.run(*clone);
      if (vectCore->isFunctionVectorized()) {
        // if the function is successfully vectorized update vectFunc and width.
        vectFunc = clone;
        vectFuncWidth = vectCore->getPacketWidth();
        vectDim = vectCore->getVectorizationDim();
        canUniteWorkgroups = vectCore->getCanUniteWorkgroups();
        // copy stats from the original function to the new one
        intel::Statistic::copyFunctionStats(**fi, *clone);
      } else {
        // We can't or choose not to vectorize the kernel, erase the clone from the module.
        // but first copy the vectorizer stats back to the original function
        intel::Statistic::copyFunctionStats(*clone, **fi);
        intel::Statistic::removeFunctionStats(*clone);
        clone->eraseFromParent();
      }
      V_ASSERT(vectFuncWidth > 0 && "vect width for non vectoized kernels should be 1");
      //Initialize scalar kernel information, which contains:
      // * pointer to vectorized kernel
      // * vectorized width of 1 (as it is the scalar version)
      // * NULL as pointer to scalar version (as there is no scalar version for scalar kernel)
      Intel::KernelInfoMetaDataHandle skimd = mdUtils.getOrInsertKernelsInfoItem(*fi);
      skimd->setVectorizedKernel(vectFunc);
      skimd->setVectorizedWidth(1);
      skimd->setScalarizedKernel(NULL);
      if (vectFunc) {
        //Initialize vector kernel information
        // * NULL pointer to vectorized kernel (as there is no vectorized version for vectroized kernel)
        // * vectorized width
        // * pointer to scalar version
        Intel::KernelInfoMetaDataHandle vkimd = mdUtils.getOrInsertKernelsInfoItem(vectFunc);
        vkimd->setVectorizedKernel(NULL);
        vkimd->setVectorizedWidth(vectFuncWidth);
        vkimd->setScalarizedKernel(*fi);
        vkimd->setVectorizationDimension(vectDim);
        vkimd->setCanUniteWorkgroups(canUniteWorkgroups);
      }
    }
  }

  //Save Metadata to the module
  mdUtils.save(M.getContext());

  {
    legacy::PassManager mpm;
    mpm.add(createBuiltinLibInfoPass(getAnalysis<BuiltinLibInfo>().getBuiltinModules(), ""));
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
extern "C"
  Pass *createVectorizerPass(llvm::SmallVector<llvm::Module*, 2> runtimeModuleList, const intel::OptimizerConfig* pConfig)
{
  return new intel::Vectorizer(runtimeModuleList, pConfig);
}

