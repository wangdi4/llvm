//===---- HIRPropagateCastedIV.h -----------------*- C++-*---===//
//
// Copyright (C) 2015-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// Implements HIR Propagate Casted IV Pass.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRPROPAGATECASTEDIVPASS_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRPROPAGATECASTEDIVPASS_H

#include "llvm/IR/PassManager.h"

namespace llvm {

namespace loopopt {

class HIRPropagateCastedIVPass
    : public PassInfoMixin<HIRPropagateCastedIVPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // namespace loopopt

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRPROPAGATECASTEDIVPASS_H
