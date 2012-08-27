/*********************************************************************************************
* Copyright © 2010, Intel Corporation
* Subject to the terms and conditions of the Master Development License
* Agreement between Intel and Apple dated August 26, 2005; under the Intel
* CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
*********************************************************************************************/

#include <iomanip>

#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/IPO/InlinerPass.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Pass.h"
#include "llvm/Linker.h"
#include "llvm/PassManager.h"
#include "Main.h"
#include "InstCounter.h"
#include "RuntimeServices.h"
#include "X86Lower.h"
#include "Packetizer.h"
#include "Resolver.h"
#include "MICResolver.h"
#include "WIAnalysis.h"
#include "VecConfig.h"
#include "IRPrinter.h"
#include "TargetArch.h"
#include "VectorizerCore.h"

// Placeholders for debug log files
FILE * prtFile;
FILE * moduleDmp;

using namespace llvm;

char intel::Vectorizer::ID = 0;

extern "C" Pass *createSpecialCaseBuiltinResolverPass();
extern "C" FunctionPass *createVectorizerCorePass(const intel::OptimizerConfig*, bool);

namespace intel {

Vectorizer::Vectorizer(const Module * rt, const OptimizerConfig* pConfig,
  SmallVectorImpl<Function*> &optimizerFunctions,
  SmallVectorImpl<int> &optimizerWidths) : 
  ModulePass(ID),
  m_runtimeModule(rt),
  m_numOfKernels(0),
  m_isModuleVectorized(false),
  m_pConfig(pConfig),
  m_optimizerFunctions(&optimizerFunctions),
  m_optimizerWidths(&optimizerWidths)
{
  // init debug prints
  initializeLoopInfoPass(*PassRegistry::getPassRegistry());
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

  // check for some common module errors, before actually diving in
  NamedMDNode *KernelsMD = M.getNamedMetadata("opencl.kernels");
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
  if (!m_runtimeModule)
  {
    V_PRINT(wrapper, "Failed to find runtime module. Aborting!\n");
    return false;
  }


  for (int i = 0, e = KernelsMD->getNumOperands(); i < e; i++) {
    MDNode *FuncInfo = KernelsMD->getOperand(i);
    Value *field0 = FuncInfo->getOperand(0)->stripPointerCasts();
    Function *F = dyn_cast<Function>(field0);
    bool disableVect = false;

    //look for vector type hint metadata
    for (int i = 1, e = FuncInfo->getNumOperands(); i < e; i++) {
      MDNode *MDVTH = dyn_cast<MDNode>(FuncInfo->getOperand(i));
      assert(MDVTH && "Malformed metadata!");

      MDString *tag = dyn_cast<MDString>(MDVTH->getOperand(0));
      assert(tag && "Malformed metadata!");

      if (tag && tag->getString() == "vec_type_hint") {
        // extract type
        Type *VTHTy = MDVTH->getOperand(1)->getType();

        if (!VTHTy->isFloatTy()     &&
          !VTHTy->isDoubleTy()    &&
          !VTHTy->isIntegerTy(8)  &&
          !VTHTy->isIntegerTy(16) &&
          !VTHTy->isIntegerTy(32) &&
          !VTHTy->isIntegerTy(64)) {
            disableVect = true;
        }
        break;
      }
    }

    // Only add kernels to list, if they have scalar vec-type hint (or none)
    if (!disableVect)
      m_scalarFuncsList.push_back(F);
    else
      m_scalarFuncsList.push_back(NULL);
  }


  // Create the vectorizer core pass that will do the vectotrization work.
  VectorizerCore *vectCore = (VectorizerCore *)createVectorizerCorePass(m_pConfig, false);
  FunctionPassManager vectPM(&M);
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
      Function *clone = CloneFunction(*fi);
      clone->setName("__Vectorized_." + (*fi)->getName());
      M.getFunctionList().push_back(clone);
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
    m_optimizerFunctions->push_back(vectFunc);
    m_optimizerWidths->push_back(vectFuncWidth);
    
  }

  
  {
    PassManager mpm;
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
  Pass *createVectorizerPass(const Module *runtimeModule, const intel::OptimizerConfig* pConfig, 
  SmallVectorImpl<Function*> &optimizerFunctions,
  SmallVectorImpl<int> &optimizerWidths)
{    
  return new intel::Vectorizer(runtimeModule, pConfig,
    optimizerFunctions, optimizerWidths);
}

