//==--------- SinCosFold.h - Fold sin(), cos() into sincos() ----- C++ -*---==//
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
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_SIN_COS_FOLD_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_SIN_COS_FOLD_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class RuntimeService;

/// SinCosFold pass replaces a pair of sin() and cos() call of the same angle
/// argument with a single sincos() call.
/// The replacement strategy is conservative -- the sin() and cos() calls need
/// to live in the same basic block.
class SinCosFoldPass : public PassInfoMixin<SinCosFoldPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);

  bool runImpl(Function &F, RuntimeService *RTS);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_SIN_COS_FOLD_H
