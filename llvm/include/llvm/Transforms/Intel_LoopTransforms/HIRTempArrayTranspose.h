//===----------- HIRTempArrayTranspose.h ----------------*- C++-*---===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_HIRTEMPARRAYTRANSPOSE_H
#define LLVM_HIRTEMPARRAYTRANSPOSE_H

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {

namespace loopopt {

class HIRTempArrayTransposePass
    : public HIRPassInfoMixin<HIRTempArrayTransposePass> {

public:
  static constexpr auto PassName = "hir-temp-array-transpose";
  PreservedAnalyses runImpl(Function &F, FunctionAnalysisManager &AM,
                            HIRFramework &HIRF);
};

} // namespace loopopt

} // namespace llvm

#endif // LLVM_HIRTEMPARRAYTRANSPOSE_H
