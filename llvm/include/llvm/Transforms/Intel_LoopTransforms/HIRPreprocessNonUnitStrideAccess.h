//===--------- HIRPreprocessNonUnitStrideAccess.h --------------*- C++-*---===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_HIRPREPROCESSNONUNITSTRIDEACCESS_H
#define LLVM_HIRPREPROCESSNONUNITSTRIDEACCESS_H

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {

namespace loopopt {

class HIRPreprocessNonUnitStrideAccess
    : public HIRPassInfoMixin<HIRPreprocessNonUnitStrideAccess> {

public:
  static constexpr auto PassName = "hir-preprocess-nonunit-stride-access";
  PreservedAnalyses runImpl(Function &F, FunctionAnalysisManager &AM,
                            HIRFramework &HIRF);
};

} // namespace loopopt

} // namespace llvm

#endif // LLVM_HIRPREPROCESSNONUNITSTRIDEACCESS_H