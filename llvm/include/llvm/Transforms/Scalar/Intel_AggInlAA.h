//===------- Intel_AggInlAA.h - Aggressive Inline AA  -*------===//
//
// Copyright (C) 2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// Aggressive Inline AA
//===----------------------------------------------------------------------===//
//
#ifndef LLVM_ANALYSIS_INTELAGGINLAA_H
#define LLVM_ANALYSIS_INTELAGGINLAA_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"


namespace llvm {

struct AggInlAAPass : PassInfoMixin<AggInlAAPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &);
};

}

#endif
