//===----------------------- HIRStoreResultIntoTempArray.h -----------------------------===//
//
// Copyright (C) 2020-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRSTORERESULTINTOTEMPARRAY_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRSTORERESULTINTOTEMPARRAY_H

#include "llvm/IR/PassManager.h"

namespace llvm {

namespace loopopt {

class HIRStoreResultIntoTempArrayPass
    : public PassInfoMixin<HIRStoreResultIntoTempArrayPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // namespace loopopt

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRSTORERESULTINTOTEMPARRAY_H

