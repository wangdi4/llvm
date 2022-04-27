//===--- ExternalizeGlobalVariables.h ---------------------------*- C++ -*-===//
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

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_EXTERNALIZE_GLOBAL_VARIABLE_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_EXTERNALIZE_GLOBAL_VARIABLE_H

#include "llvm/IR/PassManager.h"

namespace llvm {
// This pass changes a linkage type of global variables from internal to
// external
class ExternalizeGlobalVariablesPass
    : public PassInfoMixin<ExternalizeGlobalVariablesPass> {
public:
  PreservedAnalyses run(Module &F, ModuleAnalysisManager &AM);

  // Glue for old PM.
  bool runImpl(Module &M);
};

} // namespace llvm
#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_EXTERNALIZE_GLOBAL_VARIABLE_H
