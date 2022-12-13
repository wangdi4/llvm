//===- CleanupWrappedKernel.cpp - Delete wrapped kernel body --------------===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/CleanupWrappedKernel.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

using namespace llvm;

namespace {

bool runImpl(Module &M) {
  bool Changed = false;
  auto Kernels = CompilationUtils::getAllKernels(M);
  for (auto *Kernel : Kernels) {
    // If a kernel is wrapped - delete its body
    DPCPPKernelMetadataAPI::KernelInternalMetadataAPI KIMD(Kernel);
    if (KIMD.KernelWrapper.hasValue() && KIMD.KernelWrapper.get()) {
      Kernel->eraseMetadata(LLVMContext::MD_dbg);
      Kernel->eraseMetadata(LLVMContext::MD_prof);
      SmallVector<std::pair<unsigned, MDNode *>, 8> MDs;
      Kernel->getAllMetadata(MDs);
      Kernel->deleteBody();
      for (auto &MD : MDs)
        Kernel->setMetadata(MD.first, MD.second);

      Changed = true;
    }
  }

  return Changed;
}

} // namespace

PreservedAnalyses CleanupWrappedKernelPass::run(Module &M,
                                                ModuleAnalysisManager &AM) {
  if (!runImpl(M))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}
