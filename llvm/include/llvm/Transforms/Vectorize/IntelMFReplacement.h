//===-------------------IntelMFReplacement.cpp ----------------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
// OpenCL: Replace arithmetic instructions like udiv, idiv, urem and srem
// with OpenCL vector functions.

#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTELMFREPLACEMENT_H
#define LLVM_TRANSFORMS_VECTORIZE_INTELMFREPLACEMENT_H

namespace llvm {

class MathLibraryFunctionsReplacementPass
    : public PassInfoMixin<MathLibraryFunctionsReplacementPass> {
private:
  bool isOCL;

public:
  MathLibraryFunctionsReplacementPass(bool isOCL = true) : isOCL(isOCL) {}
  // Run the pass over the function.
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTELMFREPLACEMENT_H
