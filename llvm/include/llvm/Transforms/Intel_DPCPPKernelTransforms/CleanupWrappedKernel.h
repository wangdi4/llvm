//===- CleanupWrappedKernel.h - Delete wrapped kernel body ----------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_CLEANUP_WRAPPED_KERNEL_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_CLEANUP_WRAPPED_KERNEL_H

#include "llvm/IR/PassManager.h"

namespace llvm {
/// Deletes a body for wrapped kernels while preserving Metadata.
class CleanupWrappedKernelPass
    : public PassInfoMixin<CleanupWrappedKernelPass> {
public:
  static StringRef name() { return "CleanupWrappedKernelPass"; }

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // namespace llvm

#endif
