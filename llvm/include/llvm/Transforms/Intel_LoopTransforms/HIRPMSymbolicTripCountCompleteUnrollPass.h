//===---- HIRPMSymbolicTripCountCompleteUnroll.h -----------------*-
//C++-*---===//
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
// Implements HIR Loop Early Pattern Match Pass.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_HIRPMSYMBOLICTRIPCOUNTCOMPLETEUNROLLPASS_H
#define LLVM_HIRPMSYMBOLICTRIPCOUNTCOMPLETEUNROLLPASS_H

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {

namespace loopopt {

class HIRPMSymbolicTripCountCompleteUnrollPass
    : public HIRPassInfoMixin<HIRPMSymbolicTripCountCompleteUnrollPass> {
public:
  static constexpr auto PassName = "hir-pm-symbolic-tripcount-completeunroll";
  PreservedAnalyses runImpl(Function &F, FunctionAnalysisManager &AM,
                            HIRFramework &HIRF);
};

} // namespace loopopt

} // namespace llvm

#endif // LLVM_HIRPMSYMBOLICTRIPCOUNTCOMPLETEUNROLLPASS_H
