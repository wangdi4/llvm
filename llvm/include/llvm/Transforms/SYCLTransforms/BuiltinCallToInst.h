//===- BuiltinCallToInst.h - Resolve supported builtin calls ----*- C++ -*-===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_BUILTIN_CALL_TO_INST_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_BUILTIN_CALL_TO_INST_H

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

/// Resolve supported builtin calls, e.g. relational, shuffle with const mask.

class BuiltinCallToInstPass : public PassInfoMixin<BuiltinCallToInstPass> {
public:
  explicit BuiltinCallToInstPass() {}

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

  // Glue for old PM.
  bool runImpl(Function &F);

  static bool isRequired() { return true; }

private:
  /// Built-in function type.
  enum BuiltinType {
    BI_SHUFFLE1,
    BI_SHUFFLE1_HELPER,
    BI_SHUFFLE2,
    BI_SHUFFLE2_HELPER,
    BI_REL_IS_LESS,
    BI_REL_IS_LESS_EQUAL,
    BI_REL_IS_LESS_GREATER,
    BI_REL_IS_GREATER,
    BI_REL_IS_GREATER_EQUAL,
    BI_REL_IS_EQUAL,
    BI_REL_IS_NOT_EQUAL,
    BI_REL_IS_ORDERED,
    BI_REL_IS_UNORDERED,
    BI_REL_IS_NAN,
    BI_REL_IS_FINITE,
    BI_REL_IS_INF,
    BI_REL_IS_NORMAL,
    BI_REL_SIGNBIT,
    BI_UDIV,
    BI_SDIV,
    BI_UREM,
    BI_SREM,
    BI_NOT_SUPPORTED
  };

  /// Check if given called function is a supported built-in.
  /// \return BuiltinType.
  BuiltinType isSupportedBuiltin(CallInst *CI);

  /// Find all built-in calls in current function to handle.
  void findBuiltinCallsToHandle(Function &F);

  /// Handle all supported built-in calls in current function.
  /// \return true if function is changed.
  bool handleSupportedBuiltinCalls();

  /// Handle all shuffle calls in current function.
  /// \param ShuffleCall call instruction to shuffle built-in.
  /// \param ShuffleTy type of the called shuffle built-in.
  void handleShuffleCalls(CallInst *ShuffleCall, BuiltinType ShuffleTy);

  /// Create a fcmp instruction for relational built-in with 2 operands.
  Value *createFCmp(IRBuilder<> &Builder, BuiltinType RelationalTy, Value *Op1,
                    Value *Op2);

  /// Create a llvm.is.fpclass intrinsic for relational built-in with single
  /// operand.
  Value *createIsFPClass(IRBuilder<> &Builder, BuiltinType RelationalTy,
                         Value *Op);

  /// Handle all relational calls in current function.
  /// \param RelationalCall call instruction to relational built-in.
  /// \param RelationalTy type of the called relational built-in.
  void handleRelationalCalls(CallInst *RelationalCall,
                             BuiltinType RelationalTy);

  /// Handle scalar integer div/rem calls.
  void handleDivRemCalls(CallInst *CI, BuiltinType DivRemTy);

  /// A vector holding all supported built-in calls with their type.
  std::vector<std::pair<CallInst *, BuiltinType>> BuiltinCalls;
};

} // namespace llvm

#endif
