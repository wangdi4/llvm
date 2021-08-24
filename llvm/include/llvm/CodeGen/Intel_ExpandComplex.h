//===---- Intel_ExpandComplex.h - Expand experimental complex intrinsics --===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CODEGEN_EXPANDCOMPLEX_H
#define LLVM_CODEGEN_EXPANDCOMPLEX_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class ExpandComplexPass
    : public PassInfoMixin<ExpandComplexPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};
} // end namespace llvm

#endif // LLVM_CODEGEN_EXPANDCOMPLEX_H
