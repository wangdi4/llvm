//===- IVSplit.h - IV Split -------------------------------------*- C++ -*-===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass splits IV live ranges which span a lot of nesting inner loops.
// It can decrease register pressure of inner loops and void register allocator
// generating bad split/eviction/reload code.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IV_SPLIT_H
#define LLVM_IV_SPLIT_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class Function;

class IVSplitPass : public PassInfoMixin<IVSplitPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // end namespace llvm

#endif // LLVM_IV_SPLIT_H
