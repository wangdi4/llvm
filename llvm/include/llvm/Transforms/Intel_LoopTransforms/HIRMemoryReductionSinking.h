//===------------- HIRMemoryReductionSinking.h -----------------*- C++-*---===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// Implements HIR Memory Reduction Sinking Pass.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRMEMORYREDUCTIONSINKINGPASS_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRMEMORYREDUCTIONSINKINGPASS_H

#include "llvm/IR/PassManager.h"

namespace llvm {

namespace loopopt {

class HIRMemoryReductionSinkingPass
    : public PassInfoMixin<HIRMemoryReductionSinkingPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // namespace loopopt

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_MEMORYREDUCTIONSINKINGPASS_H
