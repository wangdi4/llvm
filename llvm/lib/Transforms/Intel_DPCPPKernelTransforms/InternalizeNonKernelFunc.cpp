//=== InternalizeNonKernelFuncPass.cpp - Internalize nonkernel func * C++ *===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/InternalizeNonKernelFunc.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

using namespace llvm;

#define DEBUG_TYPE "dpcpp-kernel-internalize-func"

namespace {

/// Legacy InternalizeNonKernelFunc pass.
class InternalizeNonKernelFuncLegacy : public ModulePass {
  InternalizeNonKernelFuncPass Impl;

public:
  static char ID;

  InternalizeNonKernelFuncLegacy() : ModulePass(ID) {
    initializeInternalizeNonKernelFuncLegacyPass(
        *PassRegistry::getPassRegistry());
  }

  ~InternalizeNonKernelFuncLegacy() {}

  StringRef getPassName() const override {
    return "InternalizeNonKernelFuncLegacy";
  }

  bool runOnModule(Module &M) override { return Impl.runImpl(M); }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addPreserved<CallGraphWrapperPass>();
    AU.setPreservesCFG();
  }
};

} // namespace

char InternalizeNonKernelFuncLegacy::ID = 0;

INITIALIZE_PASS(InternalizeNonKernelFuncLegacy, DEBUG_TYPE,
                "Internalize global variables and non-kernel functions", false,
                false)

ModulePass *llvm::createInternalizeNonKernelFuncLegacyPass() {
  return new InternalizeNonKernelFuncLegacy();
}

bool InternalizeNonKernelFuncPass::runImpl(Module &M) {
  using namespace DPCPPKernelMetadataAPI;
  bool Changed = false;
  auto Kernels = KernelList(&M).getList();

  for (auto &Func : M) {
    if (Func.hasOptNone() || Func.isDeclaration())
      continue;

    if (Func.hasFnAttribute("referenced-indirectly"))
      continue;

    // We shall not internalize kernels
    if (std::find(std::begin(Kernels), std::end(Kernels), &Func) !=
        std::end(Kernels))
      continue;

    Func.setLinkage(GlobalValue::InternalLinkage);
    Changed = true;
  }
  return Changed;
}

PreservedAnalyses
InternalizeNonKernelFuncPass::run(Module &M, ModuleAnalysisManager & /*AM*/) {
  (void)runImpl(M);
  PreservedAnalyses PA;
  PA.preserve<CallGraphAnalysis>();
  PA.preserveSet<CFGAnalyses>();
  return PA;
}
