//===- VectorKernelElimination.h - Vector kernel elimination ----*- C++ -*-===//
//
// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===----------------------------------------------------------------------===//
//
// VectorKernelEliminationPass uses heuristic from WeightedInstCountAnalysis to
// estimate costs of scalar and vector kernels. Vector kernel is eliminated if
// its cost is too high.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_VECTORKERNELELIMINATION_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_VECTORKERNELELIMINATION_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class VectorKernelEliminationPass
    : public PassInfoMixin<VectorKernelEliminationPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // namespace llvm

#endif
