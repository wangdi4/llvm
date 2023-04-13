//===- RuntimeService.h - Runtime service --------------------------*- C++-===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_RUNTIME_SERVICES_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_RUNTIME_SERVICES_H

#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/SYCLTransforms/DevLimits.h"

namespace llvm {

/// These are services for runtime-specific information, e.g. detection of
/// Thread-ID creation instructions, and Scalar/Vector mapping of builtin
/// functions.
class RuntimeService {
public:
  RuntimeService(ArrayRef<Module *> BuiltinModules) {
    setBuiltinModules(BuiltinModules);
  }

  void setBuiltinModules(ArrayRef<Module *> BuiltinModules) {
    this->BuiltinModules.assign(BuiltinModules.begin(), BuiltinModules.end());
  }

  /// Find a function in the runtime's built-in modules.
  /// \ FuncName Function name to look for.
  Function *findFunctionInBuiltinModules(StringRef FuncName) const;

  unsigned getNumJitDimensions() const { return MAX_WORK_DIM; }

  /// Return true if the function has no side effects. This means it can be
  /// safely vectorized regardless if it is being masked.
  /// @param FuncName Function name to check.
  bool hasNoSideEffect(StringRef FuncName) const;

  /// Returns true if the function is atomic built-in.
  bool isAtomicBuiltin(StringRef FuncName) const;

  /// Return true if the function is a descriptor of image built-in.
  /// \param FuncName Function name to check.
  bool isImageDescBuiltin(StringRef FuncName) const;

  /// Return true if the function is a safe llvm initrinsic.
  /// \param FuncName Function name to check.
  bool isSafeLLVMIntrinsic(StringRef FuncName) const;

  /// Checks if \pFuncName is mangled scalar min or max name.
  /// \param FuncName input mangled name.
  /// \param IsMin will be true if it is min builtin.
  /// \param IsSigned will be true if it is signd min/max builtin.
  /// \returns true iff funcName is scalar min/max builtin.
  bool isScalarMinMaxBuiltin(StringRef FuncName, bool &IsMin,
                             bool &IsSigned) const;

  /// Return true if the function is synchronization function with no side
  /// effects.
  /// \param FuncName Function name to check.
  bool isSyncWithNoSideEffect(StringRef FuncName) const;

  /// Return true if the function is a work-item builtin.
  /// \param FuncName Function name to check.
  bool isWorkItemBuiltin(StringRef FuncName) const;

  /// Return true if the function needs 'VPlan' style masking, meaning it has
  /// i32 mask as the last argument.
  /// \param FuncName Function name to check.
  bool needsVPlanStyleMask(StringRef FuncName) const;

  /// Return true if func_name is safe to speculative execute, and hence
  ///        can be hoisted even if it is under control flow.
  /// \param FuncName Function name to check.
  bool isSafeToSpeculativeExecute(StringRef FuncName);

private:
  SmallVector<Module *, 2> BuiltinModules;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_RUNTIME_SERVICES_H
