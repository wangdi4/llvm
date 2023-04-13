//==--- ReplaceScalarWithMask.h - ReplaceScalarWithMask pass - C++ -*-------==//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
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

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_REPLACE_SCALAR_WITH_MASK_PASS_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_REPLACE_SCALAR_WITH_MASK_PASS_H

#include "llvm/IR/PassManager.h"

namespace llvm {
/// @brief This pass is to replace a scalar kernel with a mask kernel.
class ReplaceScalarWithMaskPass
    : public PassInfoMixin<ReplaceScalarWithMaskPass> {
public:
  bool runImpl(Module &M);

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
};
} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_REPLACE_SCALAR_WITH_MASK_PASS_H
