#if INTEL_FEATURE_SW_ADVANCED
//===------ Intel_TightLoopEmitter.h - Prints tight loop info --------===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_TIGHTLOOP_EMITTER_H
#define LLVM_TRANSFORMS_INTEL_TIGHTLOOP_EMITTER_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class TightLoopEmitterPass : public PassInfoMixin<TightLoopEmitterPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_TIGHT_EMITTER_H
#endif // INTEL_FEATURE_SW_ADVANCED
