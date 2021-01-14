//==--- Intel_HandlePragmaVectorAligned.h ----------------------*- C++ -*---==//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_SCALAR_INTEL_HANDLE_PRAGMA_VECTOR_ALIGNED_H
#define LLVM_TRANSFORMS_SCALAR_INTEL_HANDLE_PRAGMA_VECTOR_ALIGNED_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class Function;

class HandlePragmaVectorAlignedPass
    : public PassInfoMixin<HandlePragmaVectorAlignedPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_INTEL_HANDLE_PRAGMA_VECTOR_ALIGNED_H
