//===- RuntimeService.h - Runtime service --------------------------*- C++-===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DPCPP_KERNEL_TRANSFORMS_UTILS_RUNTIME_SERVICES_H
#define INTEL_DPCPP_KERNEL_TRANSFORMS_UTILS_RUNTIME_SERVICES_H

#include "llvm/IR/Instructions.h"

namespace llvm {

/// These are services for runtime-specific information, e.g. detection of
/// Thread-ID creation instructions, and Scalar/Vector mapping of builtin
/// functions.
namespace RuntimeService {

Function *
findFunctionInBuiltinModules(const SmallVector<Module *, 2> &BuiltinModules,
                             StringRef Name);

/// Check if specified instruction is an ID generator with constant operator.
/// \returns a pair of whether the instruction is TID generator and whether
/// there is an error that its argument is not constant.
std::pair<bool, bool> isTIDGenerator(const CallInst *CI);

} // namespace RuntimeService
} // namespace llvm

#endif
