//===- RuntimeService.cpp - Runtime service ------------------------*- C++-===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/RuntimeService.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"

using namespace llvm;
using namespace DPCPPKernelCompilationUtils;

Function *
RuntimeService::findFunctionInBuiltinModules(StringRef FuncName) const {
  for (Module *M : BuiltinModules) {
    Function *RetFunction = M->getFunction(FuncName);
    if (RetFunction)
      return RetFunction;
  }
  return nullptr;
}

std::tuple<bool, bool, unsigned>
RuntimeService::isTIDGenerator(const CallInst *CI) const {
  if (!CI || !CI->getCalledFunction())
    return {false, false, 0};

  StringRef FName = CI->getCalledFunction()->getName();
  if (!isGetGlobalId(FName) && !isGetLocalId(FName) &&
      !isGetSubGroupLocalId(FName))
    return {false, false, 0}; // not a get_***_id function.

  // Early exit for subgroup TIDs that do not take any operands.
  // Dummy Dim 0 as subgroup does not have a clear dimension.
  if (isGetSubGroupLocalId(FName))
    return {true, false, 0};

  // Go on checking the first argument for other TIDS.
  Value *Op = CI->getArgOperand(0);

  // Check if the argument is constant - if not, we cannot determine if
  // the call will generate different IDs per different vectorization lanes.
  if (!isa<ConstantInt>(Op))
    return {false, true, 0};

  // Report the dimension of the request.
  auto Dim =
      static_cast<unsigned>(cast<ConstantInt>(Op)->getValue().getZExtValue());

  // This is indeed a TID generator.
  return {true, false, Dim};
}
