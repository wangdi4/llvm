//===-- CoerceWin64Types.h - Coerce types to ensure win64 ABI compliance --===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_COERCE_WIN64_TYPES_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_COERCE_WIN64_TYPES_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class CoerceWin64TypesPass : public PassInfoMixin<CoerceWin64TypesPass> {
public:
  CoerceWin64TypesPass();

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // Glue for old PM.
  bool runImpl(Module &M);

private:
  bool runOnFunction(Function *F);

  DenseMap<Function *, Function *> FunctionMap;
};
} // namespace llvm

#endif
