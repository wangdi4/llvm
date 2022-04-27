//===-- Intel_LowerSubscriptIntrinsic.h - llvm.intel.subscript --*- C++ -*-===//
//
// Copyright (C) 2018-2020 Intel Corporation. All rights reserved.
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
  static bool isRequired() { return true; }
};

/// Lowers getelementptr to llvm.intel.subscript for testing.
class ConvertGEPToSubscriptIntrinsicPass
    : public PassInfoMixin<ConvertGEPToSubscriptIntrinsicPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FM);

  /// Replaces GetElementPtrInst with llvm.intel.subscripts.
  /// New instructions are inserted before \p GEP.
  /// If \p Unlink is false, then GEP is not removed. All Uses are replaced
  /// though.
  ///
  /// Returns true if replacement happened.
  /// There is no replacement if no array type are involved and/or
  /// array indexes are all 0.
  static bool convertGEPToSubscriptIntrinsic(const DataLayout &DL,
                                             GetElementPtrInst *GEP,
                                             bool Unlink = true);
  // Replace single use, suitable for replacement of constant expression.
  static bool convertGEPToSubscriptIntrinsic(const DataLayout &DL,
                                             Instruction *Inst, Use *GEP);
};
} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_INTELLOWERSUBSCRIPTINTRINSIC_H

