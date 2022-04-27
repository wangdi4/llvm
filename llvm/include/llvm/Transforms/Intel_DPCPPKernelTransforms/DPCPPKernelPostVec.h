//===- DPCPPKernelPostVec.h - Post vectorization pass -----------*- C++ -*-===//
//
// Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file defines the DPCPPKernelPostVec pass.
// ===--------------------------------------------------------------------=== //
#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_POST_VEC_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_POST_VEC_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class DPCPPKernelPostVecPass : public PassInfoMixin<DPCPPKernelPostVecPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &) {
    return runImpl(M) ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }

  // Glue for old PM.
  bool runImpl(Module &M);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_POST_VEC_H
