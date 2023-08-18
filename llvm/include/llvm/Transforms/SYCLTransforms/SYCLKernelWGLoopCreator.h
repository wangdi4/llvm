//==SYCLKernelWGLoopCreator.h - Create WG loops in SYCL kernels- C++ -*---==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_WGLOOPCREATOR_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_WGLOOPCREATOR_H

#include "llvm/IR/PassManager.h"

namespace llvm {

/// Create workgroup loop.
class SYCLKernelWGLoopCreatorPass
    : public PassInfoMixin<SYCLKernelWGLoopCreatorPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  static bool isRequired() { return true; }
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_WGLOOPCREATOR_H
