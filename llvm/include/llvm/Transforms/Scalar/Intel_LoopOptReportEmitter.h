//=== Intel_LoopOptReportEmitter.h - Prints Loop Optimization reports ----===//
//
// Copyright (C) 2018-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPOPTREPORT_EMITTER_H
#define LLVM_TRANSFORMS_INTEL_LOOPOPTREPORT_EMITTER_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class LoopOptReportEmitterPass
    : public PassInfoMixin<LoopOptReportEmitterPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_LOOPOPTREPORT_EMITTER_H
