//===- SYCLEqualizer.h - SYCL kernel equalizer --------------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_EQUALIZER_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_EQUALIZER_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class BuiltinLibInfo;

/// Adjust the given module to be processed by the BE.
/// - replaces SPIR artifacts with Intel-implementation specific stuff.
/// - updates LLVM IR to version supported by back-end compiler
class SYCLEqualizerPass : public PassInfoMixin<SYCLEqualizerPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  static bool isRequired() { return true; }
};

} // namespace llvm

#endif
