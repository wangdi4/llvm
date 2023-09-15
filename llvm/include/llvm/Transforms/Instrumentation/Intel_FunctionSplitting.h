//===----------------- Intel_FunctionSplitting.h ------------------------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===--------------------------------------------------------------------===//
// Function splitting transformation:
//  - Extract cold basic blocks to new function to improve code locality.
//===--------------------------------------------------------------------===//
//

#ifndef LLVM_TRANSFORMS_INSTRUMENTATION_INTEL_FUNCTIONSPLITTING_H
#define LLVM_TRANSFORMS_INSTRUMENTATION_INTEL_FUNCTIONSPLITTING_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

namespace llvm {

class FunctionSplittingPass : public PassInfoMixin<FunctionSplittingPass> {
public:
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

};

ModulePass *createFunctionSplittingWrapperPass();

} // namespace llvm


#endif // LLVM_TRANSFORMS_INSTRUMENTATION_INTEL_FUNCTIONSPLITTING_H
