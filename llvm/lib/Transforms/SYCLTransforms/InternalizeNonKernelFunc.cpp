//=== InternalizeNonKernelFuncPass.cpp - Internalize nonkernel func * C++ *===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/InternalizeNonKernelFunc.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"

using namespace llvm;

#define DEBUG_TYPE "sycl-kernel-internalize-func"

bool InternalizeNonKernelFuncPass::runImpl(Module &M) {
  using namespace SYCLKernelMetadataAPI;
  bool Changed = false;
  auto Kernels = KernelList(&M).getList();

  for (auto &Func : M) {
    if (Func.isDeclaration())
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
