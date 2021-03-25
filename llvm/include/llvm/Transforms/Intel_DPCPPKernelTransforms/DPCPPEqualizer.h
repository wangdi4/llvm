//===- DPCPPEqualizer.h - DPC++ kernel equalizer --------------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_EQUALIZER_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_EQUALIZER_H

#include "llvm/IR/PassManager.h"

namespace llvm {

/// Adjust the given module to be processed by the BE.
/// - replaces SPIR artifacts with Intel-implementation specific stuff.
/// - updates LLVM IR to version supported by back-end compiler
class DPCPPEqualizerPass : public PassInfoMixin<DPCPPEqualizerPass> {
public:
  explicit DPCPPEqualizerPass() {}

  static StringRef name() { return "DPCPPEqualizerPass"; }

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // Glue for old PM.
  bool runImpl(Module &M);

private:
  /// Set block-literal-size attribute for enqueued kernels.
  void setBlockLiteralSizeMetadata(Function &F);

  /// Add sycl_kernel attribute and set C calling conventions for kernels.
  void formKernelsMetadata(Module &M);
};

} // namespace llvm

#endif
