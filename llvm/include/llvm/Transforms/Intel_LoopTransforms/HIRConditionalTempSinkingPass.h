//===------------- HIRConditionalTempSinking.h -----------------*- C++-*---===//
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
// Implements HIR Conditional Temp Sinking Pass.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRCONDITIONALTEMPSINKINGPASS_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRCONDITIONALTEMPSINKINGPASS_H

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {

namespace loopopt {

class HIRConditionalTempSinkingPass
    : public HIRPassInfoMixin<HIRConditionalTempSinkingPass> {
public:
  static constexpr auto PassName = "hir-conditional-temp-sinking";
  PreservedAnalyses runImpl(Function &F, FunctionAnalysisManager &AM,
                            HIRFramework &HIRF);
};

} // namespace loopopt

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_CONDITIONALTEMPSINKINGPASS_H
