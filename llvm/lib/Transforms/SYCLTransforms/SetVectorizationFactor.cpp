//==-- SetVectorizationFactor.cpp - Set vectorization factor ------ C++ -*-==//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===-------------------------------------------------------------------=== //

#include "llvm/Transforms/SYCLTransforms/SetVectorizationFactor.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"

using namespace llvm;

#define DEBUG_TYPE "sycl-kernel-set-vf"

extern cl::opt<VFISAKind> IsaEncodingOverride;
SetVectorizationFactorPass::SetVectorizationFactorPass(
    VFISAKind ISA) {
  // Update IsaEncodingOverride. Used by VFAnalysis.
  if (!IsaEncodingOverride.getNumOccurrences())
    IsaEncodingOverride.setValue(ISA);
}

bool SetVectorizationFactorPass::runImpl(Module &M,
                                         const VFAnalysisInfo *VFInfo) {
  auto Kernels = SYCLKernelMetadataAPI::KernelList(M).getList();
  for (auto *Kernel : Kernels) {
    SYCLKernelMetadataAPI::KernelInternalMetadataAPI KIMD(Kernel);
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
