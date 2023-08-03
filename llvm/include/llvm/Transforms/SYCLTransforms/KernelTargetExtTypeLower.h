//===- KernelTargetExtTypeLower.h -----------------------------------------===//
//
// Copyright (C) 2023 Intel Corporation
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

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_KERNELTARGETEXTTYPELOWER_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_KERNELTARGETEXTTYPELOWER_H

#include "llvm/IR/PassManager.h"

namespace llvm {

/// This pass lowers target extension type to its layout type and saves target
/// extension type of function parameter as metadata. The metadata is needed for
/// parsing kernel argument type in both PrepareKernelArgsPass and runtime.
class KernelTargetExtTypeLowerPass
    : public PassInfoMixin<KernelTargetExtTypeLowerPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  static bool isRequired() { return true; }
};

} // namespace llvm

#endif
