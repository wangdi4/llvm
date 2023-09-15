//===---- HIRVecDirInsert.h - Implements HIRVecDirInsert transformation ---===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRVECDIRINSERT_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRVECDIRINSERT_H

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {

namespace loopopt {

class HIRVecDirInsertPass : public HIRPassInfoMixin<HIRVecDirInsertPass> {

  bool OuterVec;

public:
  HIRVecDirInsertPass(bool OuterVec = true) : OuterVec(OuterVec) {}

  static constexpr auto PassName = "hir-vec-dir-insert";
  PreservedAnalyses runImpl(Function &F, FunctionAnalysisManager &AM,
                            HIRFramework &HIRF);
};

} // namespace loopopt
} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRVECDIRINSERT_H
