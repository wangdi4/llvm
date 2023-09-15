//===---- HIRSSADeconstruction.h - Deconstructs SSA for HIR ----*- C++ --*-===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRSSADECONSTRUCTION_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRSSADECONSTRUCTION_H

#include "llvm/IR/PassManager.h"

namespace llvm {

namespace loopopt {

class HIRSSADeconstructionPass
    : public PassInfoMixin<HIRSSADeconstructionPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
  // Make the pass required so it always runs in opt-bisect mode.
  // Without this pass, assertion is sometimes triggered in HIRCodeGen
  // pass (another required pass) because we form incorrect HIR.
  // In the old PM, we used to discard all the formed regions as a
  // workaround but there isn't any way to do that in new PM.
  static bool isRequired() { return true; }
};

} // namespace loopopt
} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRSSADECONSTRUCTION_H
