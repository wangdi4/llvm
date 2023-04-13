//===- AddFastMath.h - AddFastMath pass C++ -*----------------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_ADD_FAST_MATH_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_ADD_FAST_MATH_H

#include "llvm/IR/PassManager.h"

namespace llvm {
class AddFastMathPass : public PassInfoMixin<AddFastMathPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

  // Glue for old PM.
  bool runImpl(Function &F);
};

} // namespace llvm
#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_ADD_FAST_MATH_H
