/*********************************************************************************************
 * TODO: add Copyright © 2011-2012, Intel Corporation
 *********************************************************************************************/
#include "debuggingservicetype.h"

#include "BarrierMain.h"
#include "BarrierUtils.h"

#include "llvm/PassManager.h"
#include "llvm/PassManagers.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Target/TargetData.h"

using namespace llvm;

extern "C" {
  FunctionPass* createPhiCanon();

  void* createRedundantPhiNodePass();
  void* createBarrierInFunctionPass();
  void* createRemoveDuplicationBarrierPass();
  void* createSplitBBonBarrierPass();
  void* createImplicitGIDPass();
  //void* createDataPerBarrierPass();
  //void* createWIRelatedValuePass();
  //void* createDataPerValuePass();
  void* createBarrierPass(bool isNativeDebug);

  void getBarrierPassStrideSize(Pass *pPass, std::map<std::string, unsigned int>& bufferStrideMap);
}


namespace intel {

  char intel::BarrierMain::ID = 0;

  BarrierMain::BarrierMain(DebuggingServiceType debugType) :
    ModulePass(ID), m_debugType(debugType) {}

  bool BarrierMain::runOnModule(Module &M) {
    PassManager barrierModulePM;

    //Register TargetData to the pass manager
    barrierModulePM.add(new llvm::TargetData(&M));

    if( m_debugType == None ) {
      //In DBG mode do not run extra llvm optimizations
      barrierModulePM.add(createPromoteMemoryToRegisterPass());
    }

    barrierModulePM.add(createPhiCanon());
    //Register barrier module passes
    barrierModulePM.add((FunctionPass*)createRedundantPhiNodePass());
    barrierModulePM.add((ModulePass*)createBarrierInFunctionPass());

    // Only run this when not debugging or when not in native (gdb) debugging
    if ( m_debugType != Native ) {
      // This optimization removes debug information from extraneous barrier 
      // calls by deleting them.
      barrierModulePM.add((ModulePass*)createRemoveDuplicationBarrierPass());
    }

    barrierModulePM.add((ModulePass*)createSplitBBonBarrierPass());
    if (m_debugType == Native) {
      barrierModulePM.add((ModulePass*)createImplicitGIDPass());
    }
    Pass *pBarrierPass = (ModulePass*)createBarrierPass(m_debugType == Native);
    barrierModulePM.add((ModulePass*)pBarrierPass);
#ifdef _DEBUG
    barrierModulePM.add(createVerifierPass());
#endif

    if( m_debugType == None ) {
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
  Pass* createBarrierMainPass(intel::DebuggingServiceType debugType) {
    return new intel::BarrierMain(debugType);
  }

  void getBarrierStrideSize(Pass *pPass, std::map<std::string, unsigned int>& bufferStrideMap) {
    ((intel::BarrierMain*)pPass)->getStrideMap(bufferStrideMap);
  }
}
