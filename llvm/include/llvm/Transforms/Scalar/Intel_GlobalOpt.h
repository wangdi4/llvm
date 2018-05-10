//===- Intel_GlobalOpt.h - Optimize Global Variables ------------*- C++ -*-===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass performs register promotion for simple global
// variables that are not escaped.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SCALAR_INTEL_GLOBALOPT_H
#define LLVM_TRANSFORMS_SCALAR_INTEL_GLOBALOPT_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class Function;

class NonLTOGlobalOptPass : public PassInfoMixin<NonLTOGlobalOptPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_INTEL_GLOBALOPT_H
