//===------ Intel_MultiVersion.h - Whole Function multi-versioning -*------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass performs whole function multi versioning.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SCALAR_INTEL_MULTIVERSIONING_H
#define LLVM_TRANSFORMS_SCALAR_INTEL_MULTIVERSIONING_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

/// Pass to perform function multi versioning.
class MultiVersioningPass : public PassInfoMixin<MultiVersioningPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

FunctionPass *createMultiVersioningWrapperPass();

} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR__INTEL_MULTIVERSIONING_H
