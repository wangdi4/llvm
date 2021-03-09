//===----------- HIRCrossLoopArrayContraction.h ----------------*- C++-*---===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_HIRCROSSLOOPARRAYCONTRACTION_H
#define LLVM_HIRCROSSLOOPARRAYCONTRACTION_H

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {

namespace loopopt {

class HIRCrossLoopArrayContractionPass
    : public HIRPassInfoMixin<HIRCrossLoopArrayContractionPass> {

public:
  static constexpr auto PassName = "hir-cross-loop-array-contraction";
  PreservedAnalyses runImpl(Function &F, FunctionAnalysisManager &AM,
                            HIRFramework &HIRF);
};

} // namespace loopopt

} // namespace llvm

#endif //LLVM_HIRCROSSLOOPARRAYCONTRACTION_H
