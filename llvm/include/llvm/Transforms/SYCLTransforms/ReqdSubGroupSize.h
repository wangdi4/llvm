//===----- ReqdSubGroupSize.h - ReqdSubGroupSize pass - C++ -*-------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
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

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_REQD_SUBGROUP_SIZE_PASS_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_REQD_SUBGROUP_SIZE_PASS_H

#include "llvm/IR/PassManager.h"

namespace llvm {
class ReqdSubGroupSizePass : public PassInfoMixin<ReqdSubGroupSizePass> {
public:
  bool runImpl(Module &M);
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
  static bool isRequired() { return true; }
};
} // namespace llvm
#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_REQD_SUBGROUP_SIZE_PASS_H
