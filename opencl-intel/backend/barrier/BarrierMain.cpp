// Copyright 2012-2021 Intel Corporation.
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

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BarrierPass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"

#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"

using namespace llvm;

extern bool EnableSubGroupEmulation;

static cl::opt<bool> EnableReducingCrossBarrierValues(
    "enable-reducing-cross-barrier-values",
    cl::init(true), cl::Hidden);

extern "C" {
  void *createRemoveDuplicationBarrierPass(bool IsNativeDebug);

  Pass *createImplicitGIDPass(bool HandleBarrier);
  void* createReplaceScalarWithMaskPass();
  Pass* createBuiltinLibInfoPass(SmallVector<Module*, 2> builtinsList, std::string type);
  FunctionPass *createReduceCrossBarrierValuesPass();

  // subgroup emulation passes
  Pass *createSGBarrierPropagatePass();
  Pass *createSGBarrierSimplifyPass();
  Pass *createSGLoopConstructPass();
  Pass *createSGValueWidenPass(bool EnableDebug);
  Pass *createSubGroupBuiltinPass();

  /// Support for static linking of modules for Windows
  /// This pass is called by a modified Opt.exe
  Pass *createBarrierMainPass(unsigned optLevel,
                              intel::DebuggingServiceType debugType,
                              bool useTLSGlobals) {
    return new intel::BarrierMain(optLevel, debugType, useTLSGlobals);
  }
}

namespace intel {

  char intel::BarrierMain::ID = 0;

  BarrierMain::BarrierMain(unsigned optLevel, DebuggingServiceType debugType, bool useTLSGlobals)
      : ModulePass(ID), m_optLevel(optLevel), m_debugType(debugType), m_useTLSGlobals(useTLSGlobals) {
  }

  bool BarrierMain::runOnModule(Module &M) {
    legacy::PassManager barrierModulePM;

    barrierModulePM.add(createBuiltinLibInfoPass(getAnalysis<BuiltinLibInfo>().getBuiltinModules(), ""));
    if( m_optLevel > 0 ) {
      // Currently, vectorizer is enabled only when m_optLevel > 0.
      barrierModulePM.add((ModulePass*)createReplaceScalarWithMaskPass());
      // Reslove sub_group call introduced by ReplaceScalarWithMask pass.
      barrierModulePM.add(createResolveSubGroupWICallLegacyPass(
          getAnalysis<BuiltinLibInfo>().getBuiltinModules(),
          /*ResolveSGBarrier*/ false));

      barrierModulePM.add(createDeadCodeEliminationPass());
      barrierModulePM.add(createCFGSimplificationPass());

      barrierModulePM.add(createPromoteMemoryToRegisterPass());
    }

    barrierModulePM.add(createPhiCanonicalizationLegacyPass());
    //Register barrier module passes
    barrierModulePM.add(createRedundantPhiNodeLegacyPass());
    barrierModulePM.add(createGroupBuiltinLegacyPass(
        getAnalysis<BuiltinLibInfo>().getBuiltinModules()));
    barrierModulePM.add(createBarrierInFunctionLegacyPass());

    // Only run this when not debugging or when not in native (gdb) debugging
    if ( m_debugType != Native ) {
      // This optimization removes debug information from extraneous barrier
      // calls by deleting them.
      barrierModulePM.add((ModulePass *)createRemoveDuplicationBarrierPass(
          m_debugType == Native));
    }

    // Begin sub-group emulation
    if (EnableSubGroupEmulation) {
      barrierModulePM.add(createSubGroupBuiltinPass());
      barrierModulePM.add(createSGBarrierPropagatePass());
      barrierModulePM.add(createSGBarrierSimplifyPass());
    }
    // Insert ImplicitGIDPass in the middle of subgroup emulation
    // to track GIDs in emulation loops
    if (m_debugType == Native)
      barrierModulePM.add(
          (ModulePass *)createImplicitGIDPass(/*HandleBarrier*/ true));

    // Resume sub-group emulation
    if (EnableSubGroupEmulation) {
      barrierModulePM.add(createSGValueWidenPass(m_debugType == Native));
      barrierModulePM.add(createSGLoopConstructPass());
#ifdef _DEBUG
      barrierModulePM.add(createVerifierPass());
#endif
    }
    // End sub-group emulation

    // Since previous passes didn't resolve sub-group barriers, we need to
    // resolve them here.
    barrierModulePM.add(createResolveSubGroupWICallLegacyPass(
        getAnalysis<BuiltinLibInfo>().getBuiltinModules(),
        /*ResolveSGBarrier*/ true));

    barrierModulePM.add(createSplitBBonBarrierLegacyPass());

    if (m_optLevel > 0 && EnableReducingCrossBarrierValues) {
      barrierModulePM.add(createReduceCrossBarrierValuesPass());
#ifdef _DEBUG
      barrierModulePM.add(createVerifierPass());
#endif
    }
    Pass *pBarrierPass = (ModulePass *)createKernelBarrierLegacyPass(
        m_debugType == Native, m_useTLSGlobals);
    barrierModulePM.add((ModulePass*)pBarrierPass);
#ifdef _DEBUG
    barrierModulePM.add(createVerifierPass());
#endif

    if( m_optLevel > 0 ) {
      barrierModulePM.add(createPromoteMemoryToRegisterPass());
    }

    //Run module passes
    barrierModulePM.run(M);

    return true;
  }

  // Register this pass...
  //static RegisterPass<BarrierMain> BM("B-Main",
  //  "Barrier Pass - Main pass",
  //  false, true);

} // namespace intel
