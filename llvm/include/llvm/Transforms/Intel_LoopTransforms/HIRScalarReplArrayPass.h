//===---- HIRScalarReplArray.h -------------------------------------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRSCALARREPLARRAY_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRSCALARREPLARRAY_H

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {

namespace loopopt {

class HIRScalarReplArrayPass : public HIRPassInfoMixin<HIRScalarReplArrayPass> {
public:
  static constexpr auto PassName = "hir-scalarrepl-array";
  PreservedAnalyses runImpl(Function &F, FunctionAnalysisManager &AM,
                            HIRFramework &HIRF);
};

} // namespace loopopt

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRSCALARREPLARRAY_H
