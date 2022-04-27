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

#include "llvm/IR/Verifier.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils.h"

using namespace llvm;

extern "C" {
  void *createRemoveDuplicationBarrierPass(bool IsNativeDebug);

  void *createReplaceScalarWithMaskPass();
}

namespace intel {

void addBarrierMainPasses(llvm::legacy::PassManagerBase &PM,
                          SmallVector<Module *, 2> &RtlModuleList,
                          unsigned OptLevel,
                          intel::DebuggingServiceType DebugType,
                          bool UseTLSGlobals, ArrayRef<VectItem> VectInfos) {
  if (OptLevel > 0) {
    // Currently, vectorizer is enabled only when OptLevel > 0.
    PM.add((ModulePass *)createReplaceScalarWithMaskPass());
    // Reslove sub_group call introduced by ReplaceScalarWithMask pass.
    PM.add(createResolveSubGroupWICallLegacyPass(RtlModuleList,
                                                 /*ResolveSGBarrier*/ false));

    PM.add(createDeadCodeEliminationPass());
    PM.add(createCFGSimplificationPass());

    PM.add(createPromoteMemoryToRegisterPass());
  }

  PM.add(createPhiCanonicalizationLegacyPass());
  // Register barrier module passes
  PM.add(createRedundantPhiNodeLegacyPass());
  PM.add(createGroupBuiltinLegacyPass());
  PM.add(createBarrierInFunctionLegacyPass());

  // Only run this when not debugging or when not in native (gdb) debugging
  if (DebugType != Native) {
    // This optimization removes debug information from extraneous barrier
    // calls by deleting them.
    PM.add(
        (ModulePass *)createRemoveDuplicationBarrierPass(DebugType == Native));
  }

  // Begin sub-group emulation
  PM.add(createSGBuiltinLegacyPass(VectInfos));
  PM.add(createSGBarrierPropagateLegacyPass());
  PM.add(createSGBarrierSimplifyLegacyPass());
  // Insert ImplicitGIDPass in the middle of subgroup emulation
  // to track GIDs in emulation loops
  if (DebugType == Native)
    PM.add(createImplicitGIDLegacyPass(/*HandleBarrier*/ true));

  // Resume sub-group emulation
  PM.add(createSGValueWidenLegacyPass());
  PM.add(createSGLoopConstructLegacyPass());
#ifdef _DEBUG
  PM.add(createVerifierPass());
#endif
  // End sub-group emulation

  // Since previous passes didn't resolve sub-group barriers, we need to
  // resolve them here.
  PM.add(createResolveSubGroupWICallLegacyPass(RtlModuleList,
                                               /*ResolveSGBarrier*/ true));

  PM.add(createSplitBBonBarrierLegacyPass());

  if (OptLevel > 0) {
    PM.add(createReduceCrossBarrierValuesLegacyPass());
#ifdef _DEBUG
    PM.add(createVerifierPass());
#endif
  }
  Pass *pBarrierPass = (ModulePass *)createKernelBarrierLegacyPass(
      DebugType == Native, UseTLSGlobals);
  PM.add((ModulePass *)pBarrierPass);
#ifdef _DEBUG
  PM.add(createVerifierPass());
#endif

  if (OptLevel > 0) {
    PM.add(createPromoteMemoryToRegisterPass());
  }
}

} // namespace intel
