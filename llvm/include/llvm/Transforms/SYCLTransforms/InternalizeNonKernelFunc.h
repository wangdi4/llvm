//===- InternalizeNonKernelFunc.h - Internalize nonkernel func --*- C++ -*-===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_INTERNALIZE_NONKERNEL_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_INTERNALIZE_NONKERNEL_H

#include "llvm/IR/PassManager.h"

namespace llvm {

/// Internalize non-kernel functions.
class InternalizeNonKernelFuncPass
    : public PassInfoMixin<InternalizeNonKernelFuncPass> {
public:
  explicit InternalizeNonKernelFuncPass() {}

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // Glue for old PM.
  bool runImpl(Module &M);
};

} // namespace llvm

#endif
