//===-- DuplicateCalledKernelsPass.h - Duplicate called kernels -*- C++ -*-===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_DUPLICATE_CALLED_KERNELS_PASS_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_DUPLICATE_CALLED_KERNELS_PASS_H

#include "llvm/IR/PassManager.h"

namespace llvm {
class CallGraph;
class LocalBufferInfo;

/// \brief Duplicate Called Kernels pass, duplicate each kernel that is
/// called from other kernel/function. When duplicating a kernel, this pass
/// generates a new function that will be called instead of the original kernel.
///
/// Two kernels can't share the same local variable. If a function using local
/// variables is called by multiple kernels, the function and its associated
/// local variables are also cloned.
class DuplicateCalledKernelsPass
    : public PassInfoMixin<DuplicateCalledKernelsPass> {
public:
  bool runImpl(Module &M, CallGraph &CG, LocalBufferInfo &LBI);

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);

  static bool isRequired() { return true; }
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_DUPLICATE_CALLED_KERNELS_PASS_H
