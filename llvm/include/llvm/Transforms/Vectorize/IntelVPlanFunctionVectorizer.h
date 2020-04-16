//===-- IntelVPlanFunctionVectorizer.h ------------------------*- c/c++ -*-===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines VPlan vectorizer driver pass. For now it is only expected
// to be used in internal VPlan LIT tests for passes that are generally
// applicable for both loop/region vectorization. That simplifies both the input
// IR/CHECK lines and makes it easier to understand the purpose of a given LIT
// test.
//
//
// The following assumptions are being made:
//
// 1) Whole function vectorization
// 2) Single dedicate exit with "ret void" terminator
// 3) All function operands are uniform
// 4) @llvm.vplan.laneid function call is the only source of linearity and is
//    handled specifically during the CFG construction.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VPLAN_FUNCTION_VECTORIZER_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VPLAN_FUNCTION_VECTORIZER_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

namespace llvm {
namespace vpo {
class VPlanFunctionVectorizerPass
    : public PassInfoMixin<VPlanFunctionVectorizerPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

class VPlanFunctionVectorizerLegacyPass : public FunctionPass {
protected:
  void getAnalysisUsage(AnalysisUsage &AU) const override;

public:
  static char ID; // Pass identification, replacement for typeid
  VPlanFunctionVectorizerLegacyPass();

  bool runOnFunction(Function &Fn) override;
};

} // namespace vpo
} // namespace llvm

#endif
