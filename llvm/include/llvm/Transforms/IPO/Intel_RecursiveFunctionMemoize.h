//===- Intel_RecursiveFunctionMemoize.h ----------------------------------===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_INTEL_RECURSIVEFUNCTIONMEMOIZE_H
#define LLVM_TRANSFORMS_IPO_INTEL_RECURSIVEFUNCTIONMEMOIZE_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class RecursiveFunctionMemoizePass
    : public PassInfoMixin<RecursiveFunctionMemoizePass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_IPO_INTEL_RECURSIVEFUNCTIONMEMOIZE_H