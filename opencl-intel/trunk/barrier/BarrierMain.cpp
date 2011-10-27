/*********************************************************************************************
 * TODO: add Copyright © 2011, Intel Corporation
 *********************************************************************************************/
#include "BarrierMain.h"
#include "BarrierUtils.h"

#include "llvm/PassManager.h"
#include "llvm/PassManagers.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Analysis/Verifier.h"

using namespace llvm;

extern "C" {
  FunctionPass* createPhiCanon();

  void* createRedundantPhiNodePass();
  void* createBarrierInFunctionPass();
  void* createRemoveDuplicationBarrierPass();
  void* createSplitBBonBarrierPass();
  //void* createDataPerBarrierPass();
  //void* createWIRelatedValuePass();
  //void* createDataPerValuePass();
  void* createBarrierPass();

  unsigned int getBarrierPassStrideSize(Pass *pPass);
}


namespace intel {

  char intel::BarrierMain::ID = 0;

  BarrierMain::BarrierMain(bool isDBG) :
    ModulePass(ID), m_isDBG(isDBG), m_strideSize(0) {}

  bool BarrierMain::runOnModule(Module &M) {
    PassManager barrierModulePM;

    if( !m_isDBG ) {
      //In DBG mode do not run extra llvm optimizations
      barrierModulePM.add(createPromoteMemoryToRegisterPass());
    }

    barrierModulePM.add(createPhiCanon());
    //Register barrier module passes
    barrierModulePM.add((FunctionPass*)createRedundantPhiNodePass());
    barrierModulePM.add((ModulePass*)createBarrierInFunctionPass());
    barrierModulePM.add((ModulePass*)createRemoveDuplicationBarrierPass());
    barrierModulePM.add((ModulePass*)createSplitBBonBarrierPass());
    Pass *pBarrierPass = (ModulePass*)createBarrierPass();
    barrierModulePM.add((ModulePass*)pBarrierPass);
#ifdef _DEBUG
    barrierModulePM.add(createVerifierPass());
#endif

    if( !m_isDBG ) {
      //In DBG mode do not run extra llvm optimizations
      barrierModulePM.add(createPromoteMemoryToRegisterPass());
      barrierModulePM.add(createLICMPass());
    }

    //Run module passes
    barrierModulePM.run(M);

    m_strideSize = getBarrierPassStrideSize(pBarrierPass);

    return true;
  }

  // Register this pass...
  //static RegisterPass<BarrierMain> BM("bmain",
  //  "Barrier Main pass",
  //  false, true);

} // namespace intel

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  Pass* createBarrierMainPass(bool isDBG) {
    return new intel::BarrierMain(isDBG);
  }

  unsigned int getBarrierStrideSize(Pass *pPass) {
    return ((intel::BarrierMain*)pPass)->getStrideSize();
  }
}