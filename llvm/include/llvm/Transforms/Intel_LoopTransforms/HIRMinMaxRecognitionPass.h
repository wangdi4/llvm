//===--- HIRMinMaxRecognition.h ------------------------------*- C++ -*---===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRMINMAXRECOGNITION_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRMINMAXRECOGNITION_H

#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {
class Function;

namespace loopopt {

class HIRMinMaxRecognitionPass
    : public HIRPassInfoMixin<HIRMinMaxRecognitionPass> {
public:
  static constexpr auto PassName = "hir-minmax-recognition";
  PreservedAnalyses runImpl(Function &F, FunctionAnalysisManager &AM,
                            HIRFramework &HIRF);
};

} // namespace loopopt
} // namespace llvm

#endif
