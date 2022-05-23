//===- OptimizeIDivAndIRem.h - OptimizeIDivAndIRem pass C++ -*-------------===//
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
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_OPTIMIZE_IDIV_AND_IREM_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_OPTIMIZE_IDIV_AND_IREM_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class RuntimeService;

class OptimizeIDivAndIRemPass : public PassInfoMixin<OptimizeIDivAndIRemPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

  // Glue for old PM.
  bool runImpl(Function &F, RuntimeService *RTS);
};

} // namespace llvm
#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_OPTIMIZE_IDIV_AND_IREM_H
