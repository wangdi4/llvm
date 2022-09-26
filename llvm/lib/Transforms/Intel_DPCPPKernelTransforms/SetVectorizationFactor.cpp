//==-- SetVectorizationFactor.cpp - Set vectorization factor ------ C++ -*-==//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===-------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SetVectorizationFactor.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/InitializePasses.h"
#include "llvm/PassRegistry.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

using namespace llvm;

#define DEBUG_TYPE "dpcpp-kernel-set-vf"

namespace {

/// Legacy SetVectorizationFactor pass.
class SetVectorizationFactorLegacy : public ModulePass {
  SetVectorizationFactorPass Impl;

public:
  static char ID;

  SetVectorizationFactorLegacy(VFISAKind ISA = VFISAKind::SSE)
      : ModulePass(ID), Impl(ISA) {
    initializeSetVectorizationFactorLegacyPass(
        *PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return "SetVectorizationFactorLegacy";
  }

  bool runOnModule(Module &M) override {
    return Impl.runImpl(M, &getAnalysis<VFAnalysisLegacy>().getResult());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<VFAnalysisLegacy>();
    AU.setPreservesAll();
  }
};
} // namespace

char SetVectorizationFactorLegacy::ID = 0;

INITIALIZE_PASS_BEGIN(SetVectorizationFactorLegacy, DEBUG_TYPE,
                      "Set VF metadata for kernels", false, false)
INITIALIZE_PASS_DEPENDENCY(VFAnalysisLegacy)
INITIALIZE_PASS_END(SetVectorizationFactorLegacy, DEBUG_TYPE,
                    "Set VF metadata for kernels", false, false)

ModulePass *
llvm::createSetVectorizationFactorLegacyPass(VFISAKind ISA) {
  return new SetVectorizationFactorLegacy(ISA);
}

extern cl::opt<VFISAKind> IsaEncodingOverride;
SetVectorizationFactorPass::SetVectorizationFactorPass(
    VFISAKind ISA) {
  // Update IsaEncodingOverride. Used by VFAnalysis.
  if (!IsaEncodingOverride.getNumOccurrences())
    IsaEncodingOverride.setValue(ISA);
}

bool SetVectorizationFactorPass::runImpl(Module &M,
                                         const VFAnalysisInfo *VFInfo) {
  auto Kernels = DPCPPKernelMetadataAPI::KernelList(M).getList();
  for (auto *Kernel : Kernels) {
    DPCPPKernelMetadataAPI::KernelInternalMetadataAPI KIMD(Kernel);
    unsigned VF = VFInfo->getVF(Kernel);
    KIMD.RecommendedVL.set(VF);
    LLVM_DEBUG(dbgs() << "Set VF=" << VF << " for kernel " << Kernel->getName()
                      << '\n');
    unsigned SGEmuSize = VFInfo->getSubgroupEmulationSize(Kernel);
    if (SGEmuSize == 0)
      continue;
    KIMD.SubgroupEmuSize.set(SGEmuSize);
    // The kernel needs to go through Barrier passes when it needs to be
    // emulated.
    KIMD.NoBarrierPath.set(false);
    LLVM_DEBUG(dbgs() << "Set SGEmuSize=" << SGEmuSize << " for kernel "
                      << Kernel->getName() << '\n');
  }
  return !Kernels.empty();
}

PreservedAnalyses SetVectorizationFactorPass::run(Module &M,
                                                  ModuleAnalysisManager &AM) {
  VFAnalysisInfo *VFInfo = &AM.getResult<VFAnalysis>(M);
  (void)runImpl(M, VFInfo);
  // This pass won't invalidate any analysis results.
  return PreservedAnalyses::all();
}
