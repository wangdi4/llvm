// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "Main.h"
#include "VectorizerCore.h"
#include "MetadataAPI.h"
#include "OclTune.h"

#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Pass.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"

using namespace llvm;
using namespace Intel::MetadataAPI;

char intel::Vectorizer::ID = 0;

extern "C" Pass* createSpecialCaseBuiltinResolverPass();
extern "C" FunctionPass* createVectorizerCorePass(const intel::OptimizerConfig*);
extern "C" Pass* createBuiltinLibInfoPass(
  llvm::SmallVector<llvm::Module*, 2> pRtlModuleList, std::string type);

namespace intel {

  Vectorizer::Vectorizer(llvm::SmallVector<llvm::Module*, 2> rtList, const OptimizerConfig* pConfig) :
  ModulePass(ID),
  m_pConfig(pConfig),
  m_optimizerFunctions(nullptr),
  m_optimizerWidths(nullptr)
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

  // holds all the "original" (scalar) functions
  SmallVector<Function*, ESTIMATED_NUM_OF_FUNCTIONS> scalarFuncsList;

  // set isModuleVectorized and proper number of kernels to zero, in case vectorization fails
  // number of kernels in current module
  unsigned numOfKernels = 0;
  // was current module vectorized
  bool isModuleVectorized = true;

  auto Kernels = KernelList(&M).getList();

  // check for some common module errors, before actually diving in
  if (Kernels.empty())
  {
    V_PRINT(wrapper, "Failed to find annotation. Aborting!\n");
    return false;
  }
  numOfKernels = Kernels.size();
  if (numOfKernels == 0)
  {
    V_PRINT(wrapper, "Num of kernels is 0. Aborting!\n");
    return false;
  }
  if (m_runtimeModuleList.size() == 0)
  {
    V_PRINT(wrapper, "Failed to find runtime module. Aborting!\n");
    return false;
  }


  for (auto* pFunc : Kernels) {
    auto kimd = KernelMetadataAPI(pFunc);
    auto VecTypeHint = kimd.VecTypeHint;
    auto VecLenHint = kimd.VecLenHint;
    bool disableVect = false;

    // look for vector type hint metadata
    if (!VecLenHint.hasValue() && VecTypeHint.hasValue()) {
      Type* VTHTy = VecTypeHint.getType();
      if (!VTHTy->isFloatTy()     &&
          !VTHTy->isDoubleTy()    &&
          !VTHTy->isIntegerTy(8)  &&
          !VTHTy->isIntegerTy(16) &&
          !VTHTy->isIntegerTy(32) &&
          !VTHTy->isIntegerTy(64)) {
            disableVect = true;
      }
    }

    // Only add kernels to list, if they:
    // 1. have scalar vec-type hint
    // 2. don't have vec-type hint
    // 3. have vec-len hint
    if (!disableVect)
      scalarFuncsList.push_back(pFunc);
  }


  // Create the vectorizer core pass that will do the vectotrization work.
  VectorizerCore *vectCore = (VectorizerCore *)createVectorizerCorePass(m_pConfig);

  TargetMachine* targetMachine = m_pConfig->GetTargetMachine();
  V_ASSERT(targetMachine && "Uninitialized TargetMachine!");

  legacy::FunctionPassManager vectPM(&M);
  vectPM.add(createTargetTransformInfoWrapperPass(
                 targetMachine->getTargetIRAnalysis()));
  TargetLibraryInfoImpl TLII(Triple(M.getTargetTriple()));
  vectPM.add(new TargetLibraryInfoWrapperPass(TLII));
  vectPM.add(createBuiltinLibInfoPass(getAnalysis<BuiltinLibInfo>().getBuiltinModules(), ""));
  vectPM.add(vectCore);

  for (auto *F : scalarFuncsList)
  {
    // default values for non vectorized kernels.
    Function *vectFunc = 0;
    int vectFuncWidth = 1;
    unsigned int vectDim = 0;
    bool canUniteWorkgroups = false;

    assert(F && "scalarFuncsList should not contain nullptrs!");

    // Clone the kernel
    ValueToValueMapTy vmap;
    Function *clone = CloneFunction(F, vmap, nullptr);
    clone->setName("__Vectorized_." + F->getName());

    auto vkimd = KernelInternalMetadataAPI(clone);
    vkimd.VectorizedKernel.set(nullptr);
    vkimd.ScalarizedKernel.set(F);

    vectPM.run(*clone);
    if (vectCore->isFunctionVectorized()) {
      // if the function is successfully vectorized update vectFunc and width.
      vectFunc = clone;
      vectFuncWidth = vectCore->getPacketWidth();
      vectDim = vectCore->getVectorizationDim();
      canUniteWorkgroups = vectCore->getCanUniteWorkgroups();
      // copy stats from the original function to the new one
      intel::Statistic::copyFunctionStats(*F, *clone);
    } else {
      // We can't or choose not to vectorize the kernel, erase the clone from the module.
      // but first copy the vectorizer stats back to the original function
      intel::Statistic::copyFunctionStats(*clone, *F);
      intel::Statistic::removeFunctionStats(*clone);
      clone->eraseFromParent();
    }
    V_ASSERT(vectFuncWidth > 0 && "vect width for non vectoized kernels should be 1");
    //Initialize scalar kernel information, which contains:
    // * pointer to vectorized kernel
    // * vectorized width of 1 (as it is the scalar version)
    // * NULL as pointer to scalar version (as there is no scalar version for scalar kernel)
    auto skimd = KernelInternalMetadataAPI(F);
    skimd.VectorizedKernel.set(vectFunc);
    skimd.VectorizedWidth.set(1);
    skimd.ScalarizedKernel.set(nullptr);
    if (vectFunc) {
      //Initialize vector kernel information
      // * NULL pointer to vectorized kernel (as there is no vectorized version for vectroized kernel)
      // * vectorized width
      // * pointer to scalar version
      auto vkimd = KernelInternalMetadataAPI(vectFunc);
      vkimd.VectorizedKernel.set(nullptr);
      vkimd.VectorizedWidth.set(vectFuncWidth);
      vkimd.ScalarizedKernel.set(F);
      vkimd.VectorizationDimension.set(vectDim);
      vkimd.CanUniteWorkgroups.set(canUniteWorkgroups);
    }
  }

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

  return isModuleVectorized;
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

