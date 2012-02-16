/*********************************************************************************************
 * TODO: add Copyright © 2011-2012, Intel Corporation
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

  void getBarrierPassStrideSize(Pass *pPass, std::map<std::string, unsigned int>& bufferStrideMap);
}


namespace intel {

  char intel::BarrierMain::ID = 0;

  BarrierMain::BarrierMain(bool isDBG) :
    ModulePass(ID), m_isDBG(isDBG) {}

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

    getBarrierPassStrideSize(pBarrierPass, m_bufferStrideMap);

    return true;
  }

  // Register this pass...
  //static RegisterPass<BarrierMain> BM("B-Main",
  //  "Barrier Pass - Main pass",
  //  false, true);

} // namespace intel

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  Pass* createBarrierMainPass(bool isDBG) {
    return new intel::BarrierMain(isDBG);
  }

  void getBarrierStrideSize(Pass *pPass, std::map<std::string, unsigned int>& bufferStrideMap) {
    ((intel::BarrierMain*)pPass)->getStrideMap(bufferStrideMap);
  }
}