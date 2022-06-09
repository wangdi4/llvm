//===-- DuplicateCalledKernelsPass.h - Duplicate called kernels -*- C++ -*-===//
//
// Copyright (C) 2020-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_DUPLICATE_CALLED_KERNELS_PASS_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_DUPLICATE_CALLED_KERNELS_PASS_H

#include "llvm/IR/PassManager.h"

namespace llvm {

/// \brief Duplicate Called Kernels pass, simply duplicate each kernel  that is
/// called from other kernel/function. When duplicating a kernel, this pass
/// generate a new function that will be called instead of the original kernel.
//  P.S. It assumes that CloneFunction handles llvm debug info right.
class DuplicateCalledKernelsPass
    : public PassInfoMixin<DuplicateCalledKernelsPass> {
public:
  bool runImpl(Module &M);

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
};

class DuplicateCalledKernelsLegacy : public ModulePass {
  DuplicateCalledKernelsPass Impl;

public:
  static char ID;

  DuplicateCalledKernelsLegacy();

  virtual llvm::StringRef getPassName() const override {
    return "DuplicateCalledKernels";
  }

  virtual bool runOnModule(Module &M) override;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_DUPLICATE_CALLED_KERNELS_PASS_H
