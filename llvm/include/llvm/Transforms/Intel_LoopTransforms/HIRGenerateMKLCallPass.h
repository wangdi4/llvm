//===------ HIRGenerateMKLCall.h - Implements Generate MKL Call class -----===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRGENERATEMKLCALL_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRGENERATEMKLCALL_H

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {

namespace loopopt {

class HIRGenerateMKLCallPass : public HIRPassInfoMixin<HIRGenerateMKLCallPass> {
public:
  static constexpr auto PassName = "hir-generate-mkl-call";
  PreservedAnalyses runImpl(Function &F, FunctionAnalysisManager &AM,
                            HIRFramework &HIRF);
};

} // namespace loopopt

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRGENERATEMKLCALL_H
