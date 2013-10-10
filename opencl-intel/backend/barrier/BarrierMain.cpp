/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "debuggingservicetype.h"

#include "BarrierMain.h"
#include "BarrierUtils.h"

#include "llvm/PassManager.h"
#include "llvm/PassManagers.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Version.h"
#if LLVM_VERSION == 3425
#include "llvm/Target/TargetData.h"
#else
#include "llvm/DataLayout.h"
#endif

using namespace llvm;

extern "C" {
  FunctionPass* createPhiCanon();

  void* createRedundantPhiNodePass();
  void* createGroupBuiltinPass();
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

    //Register DataLayout to the pass manager
#if LLVM_VERSION == 3425
    barrierModulePM.add(new llvm::TargetData(&M));
#else
    barrierModulePM.add(new llvm::DataLayout(&M));
#endif

    if( m_debugType == None ) {
      //In DBG mode do not run extra llvm optimizations
      barrierModulePM.add(createPromoteMemoryToRegisterPass());
    }

    barrierModulePM.add(createPhiCanon());
    //Register barrier module passes
    barrierModulePM.add((FunctionPass*)createRedundantPhiNodePass());
    barrierModulePM.add((ModulePass*)createGroupBuiltinPass());
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
