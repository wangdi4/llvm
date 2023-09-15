//===------- Intel_AggressiveSpeculation.h - Aggressive Speculation -------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SCALAR_AGGRESSIVESPECULATION_H
#define LLVM_TRANSFORMS_SCALAR_AGGRESSIVESPECULATION_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class AggressiveSpeculationPass
    : public PassInfoMixin<AggressiveSpeculationPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_AGGRESSIVESPECULATION_H
