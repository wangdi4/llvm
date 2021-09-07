//===- RuntimeService.cpp - Runtime service ------------------------*- C++-===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "RuntimeService.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"

using namespace llvm::DPCPPKernelCompilationUtils;

namespace llvm {
namespace RuntimeService {

Function *
findFunctionInBuiltinModules(const SmallVector<Module *, 2> &BuiltinModules,
                             StringRef Name) {
  for (Module *M : BuiltinModules) {
    Function *RetFunction = M->getFunction(Name);
    if (RetFunction)
      return RetFunction;
  }
  return nullptr;
}

std::pair<bool, bool> isTIDGenerator(const CallInst *CI) {
  if (!CI || !CI->getCalledFunction())
    return {false, false};

  StringRef FName = CI->getCalledFunction()->getName();
  if (!isGetGlobalId(FName) && !isGetLocalId(FName) &&
      !isGetSubGroupLocalId(FName))
    return {false, false}; // not a get_***_id function.

  // Early exit for subgroup TIDs that do not take any operands.
  if (isGetSubGroupLocalId(FName))
    return {true, false};

  // Go on checking the first argument for other TIDS.
  Value *Dim = CI->getArgOperand(0);

  // Check if the argument is constant - if not, we cannot determine if
  // the call will generate different IDs per different vectorization lanes.
  if (!isa<ConstantInt>(Dim))
    return {false, true};

  // This is indeed a TID generator.
  return {true, false};
}

} // namespace RuntimeService
} // namespace llvm
