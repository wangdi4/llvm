//===- Intel_LowerSubscriptIntrinsic.h - Lowering llvm.intel.subscript ----*- C++ -*-===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
/// \file
/// This file provides the interface for the lowering llvm.intel.subscript
/// intrinsic.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SCALAR_INTELLOWERSUBSCRIPTINTRINSIC_H
#define LLVM_TRANSFORMS_SCALAR_INTELLOWERSUBSCRIPTINTRINSIC_H

#include "llvm/IR/Function.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/PassManager.h"

namespace llvm {
/// Lowers llvm.intel.subscript into explicit address computations.
class LowerSubscriptIntrinsicPass
    : public PassInfoMixin<LowerSubscriptIntrinsicPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FM);
};
} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_INTELLOWERSUBSCRIPTINTRINSIC_H

