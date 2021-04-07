//===- CleanupWrappedKernel.cpp - Delete wrapped kernel body --------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/CleanupWrappedKernel.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Passes.h"

using namespace llvm;

namespace {

class CleanupWrappedKernelLegacy : public ModulePass {
public:
  static char ID;

  CleanupWrappedKernelLegacy() : ModulePass(ID) {}

  StringRef getPassName() const override {
    return "CleanupWrappedKernelLegacy";
  }

  bool runOnModule(Module &M) override;
};

bool runImpl(Module &M) {
  bool Changed = false;
  auto Kernels = DPCPPKernelCompilationUtils::getAllKernels(M);
  for (auto *Kernel : Kernels) {
    // If a kernel is wrapped - delete its body
    if (Kernel->hasFnAttribute("kernel_wrapper")) {
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

INITIALIZE_PASS(CleanupWrappedKernelLegacy, "dpcpp-kernel-cleanup-wrapped",
                "Delete bodies of wrapped kernels", false, false)

char CleanupWrappedKernelLegacy::ID = 0;

bool CleanupWrappedKernelLegacy::runOnModule(Module &M) { return runImpl(M); }

ModulePass *llvm::createCleanupWrappedKernelLegacyPass() {
  return new CleanupWrappedKernelLegacy();
}

PreservedAnalyses CleanupWrappedKernelPass::run(Module &M,
                                                ModuleAnalysisManager &AM) {
  if (!runImpl(M))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}
