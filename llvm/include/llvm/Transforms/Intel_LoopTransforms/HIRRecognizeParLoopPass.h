//===----- HIRRecognizeParLoop.h - Recognizes Parallel loops --------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
// This pass recognizes parallel loops using OpenMP directives and
// HIRParVecAnalysis.
//===----------------------------------------------------------------------===//
#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIR_RECOGNIZE_PAR_LOOP_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIR_RECOGNIZE_PAR_LOOP_H

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {
namespace loopopt {

class HIRRecognizeParLoopPass
    : public HIRPassInfoMixin<HIRRecognizeParLoopPass> {
public:
  static constexpr auto PassName = "hir-recognize-par-loop";
  PreservedAnalyses runImpl(Function &F, FunctionAnalysisManager &AM,
                            HIRFramework &HIRF);
};
}
}

#endif // LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIR_RECOGNIZE_PAR_LOOP_H
