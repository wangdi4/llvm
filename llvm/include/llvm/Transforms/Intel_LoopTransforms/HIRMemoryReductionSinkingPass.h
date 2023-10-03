//===------------- HIRMemoryReductionSinking.h -----------------*- C++-*---===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
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

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {

namespace loopopt {

class HIRMemoryReductionSinkingPass
    : public HIRPassInfoMixin<HIRMemoryReductionSinkingPass> {
private:
  bool AllowConditionalReductionSinking;

public:
  static constexpr auto PassName = "hir-memory-reduction-sinking";
  HIRMemoryReductionSinkingPass(bool AllowConditionalReductionSinking = true)
      : AllowConditionalReductionSinking(AllowConditionalReductionSinking) {}
  PreservedAnalyses runImpl(Function &F, FunctionAnalysisManager &AM,
                            HIRFramework &HIRF);
};

} // namespace loopopt

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_MEMORYREDUCTIONSINKINGPASS_H
