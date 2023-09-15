//===------- HIRCodeGen.h - Implements HIRCodeGen class -------*- C++ -*---===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRCODEGEN_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRCODEGEN_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

namespace loopopt {

class HIRCodeGenPass : public PassInfoMixin<HIRCodeGenPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &);
  static bool isRequired() { return true; }
};

}

}

#endif // LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRCODEGEN_H
