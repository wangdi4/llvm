//===- DeduceMaxWGDim.h - Deduce max WG dimemsion executed ------*- C++ -*-===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_DEDUCEMAXWGDIM_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_DEDUCEMAXWGDIM_H

#include "llvm/IR/PassManager.h"

namespace llvm {

/// Deduce the maximum WG dimemsion that needs to be executed and save result
/// to max_wg_dimensions metadata.
class DeduceMaxWGDimPass : public PassInfoMixin<DeduceMaxWGDimPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_DEDUCEMAXWGDIM_H
