//===- RuntimeService.h - Runtime service --------------------------*- C++-===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_UTILS_RUNTIME_SERVICES_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_UTILS_RUNTIME_SERVICES_H

#include "llvm/IR/Instructions.h"

namespace llvm {

/// These are services for runtime-specific information, e.g. detection of
/// Thread-ID creation instructions, and Scalar/Vector mapping of builtin
/// functions.
class RuntimeService {
public:
  RuntimeService(ArrayRef<Module *> BuiltinModules) {
    this->BuiltinModules.assign(BuiltinModules.begin(), BuiltinModules.end());
  }

  /// Find a function in the runtime's built-in modules.
  /// \ FuncName Function name to look for.
  Function *findFunctionInBuiltinModules(StringRef FuncName) const;

  /// Check if \p CI is an TID generator with constant operator.
  /// \returns a tuple of
  ///   * true if \p CI is TID generator.
  ///   * true if there is an error that its argument is not constant.
  ///   * dimension of the TID generator.
  std::tuple<bool, bool, unsigned> isTIDGenerator(const CallInst *CI) const;

private:
  SmallVector<Module *, 2> BuiltinModules;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_UTILS_RUNTIME_SERVICES_H
