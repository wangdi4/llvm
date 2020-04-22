//===---- HIRPMSymbolicTripCountCompleteUnroll.h -----------------*-
//C++-*---===//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
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

#include "llvm/IR/PassManager.h"

namespace llvm {

namespace loopopt {

class HIRPMSymbolicTripCountCompleteUnrollPass
    : public PassInfoMixin<HIRPMSymbolicTripCountCompleteUnrollPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // namespace loopopt

} // namespace llvm

#endif // LLVM_HIRPMSYMBOLICTRIPCOUNTCOMPLETEUNROLLPASS_H
