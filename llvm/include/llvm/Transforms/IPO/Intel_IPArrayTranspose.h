//===----  Intel_IPArrayTranspose.h - Intel IPO Array Transpose   --------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// The pass tries to apply "Array Transpose" in IPO if it is profitable.

#ifndef LLVM_TRANSFORMS_IPO_INTEL_IPARRAYTRANSPOSE_H
#define LLVM_TRANSFORMS_IPO_INTEL_IPARRAYTRANSPOSE_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class ModulePass;

class IPArrayTransposePass : public PassInfoMixin<IPArrayTransposePass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};
} // End namespace llvm

#endif // LLVM_TRANSFORMS_IPO_INTEL_IPARRAYTRANSPOSE_H
