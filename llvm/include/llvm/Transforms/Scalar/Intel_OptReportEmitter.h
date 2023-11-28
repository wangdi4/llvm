//===------ Intel_OptReportEmitter.h - Prints Optimization reports --------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_OPTREPORT_EMITTER_H
#define LLVM_TRANSFORMS_INTEL_OPTREPORT_EMITTER_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class OptReportEmitterPass : public PassInfoMixin<OptReportEmitterPass> {
public:
  static bool isRequired() { return true; }
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_OPTREPORT_EMITTER_H
