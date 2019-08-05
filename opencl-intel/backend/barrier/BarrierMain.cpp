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

#include "BarrierMain.h"
#include "BarrierUtils.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/IR/Verifier.h"

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
  void *createBarrierPass(bool isNativeDebug, bool useTLSGlobals);
  Pass* createBuiltinLibInfoPass(SmallVector<Module*, 2> builtinsList, std::string type);

  void getBarrierPassStrideSize(Pass *pPass, std::map<std::string, unsigned int>& bufferStrideMap);
}


namespace intel {

  char intel::BarrierMain::ID = 0;

  BarrierMain::BarrierMain(DebuggingServiceType debugType, bool useTLSGlobals)
      : ModulePass(ID), m_debugType(debugType), m_useTLSGlobals(useTLSGlobals) {
  }

  bool BarrierMain::runOnModule(Module &M) {
    legacy::PassManager barrierModulePM;

    barrierModulePM.add(createBuiltinLibInfoPass(getAnalysis<BuiltinLibInfo>().getBuiltinModules(), ""));

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
    Pass *pBarrierPass =
        (ModulePass *)createBarrierPass(m_debugType == Native, m_useTLSGlobals);
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
Pass *createBarrierMainPass(intel::DebuggingServiceType debugType,
                            bool useTLSGlobals) {
  return new intel::BarrierMain(debugType, useTLSGlobals);
  }

  void getBarrierStrideSize(Pass *pPass, std::map<std::string, unsigned int>& bufferStrideMap) {
    ((intel::BarrierMain*)pPass)->getStrideMap(bufferStrideMap);
  }
}
