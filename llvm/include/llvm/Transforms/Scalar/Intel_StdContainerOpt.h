// StdContainerOpt.h -- Std Container Optimization Pass --
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SCALAR_STDCONTAINEROPT_H
#define LLVM_TRANSFORMS_SCALAR_STDCONTAINEROPT_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class StdContainerOptPass : public PassInfoMixin<StdContainerOptPass> {
public:
  StdContainerOptPass(){};
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_STDCONTAINEROPT_H
