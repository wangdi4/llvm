//===---------------- Intel_InlineReportEmitter.h - Inline Report  ----------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements an inlining report emitter pass.
//
//===----------------------------------------------------------------------===//
//

#ifndef LLVM_TRANSFORMS_IPO_INTEL_INLINEREPORTEMITTER_H
#define LLVM_TRANSFORMS_IPO_INTEL_INLINEREPORTEMITTER_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class InlineReportEmitterPass : public PassInfoMixin<InlineReportEmitterPass> {
  unsigned OptLevel;
  unsigned SizeLevel;
  bool PrepareForLTO = false;

public:
  static bool isRequired() { return true; }
  InlineReportEmitterPass(unsigned OL = 0, unsigned SL = 0,
                          bool PrepForLTO = false)
      : OptLevel(OL), SizeLevel(SL), PrepareForLTO(PrepForLTO) {}
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};
} // namespace llvm

#endif // LLVM_TRANSFORMS_IPO_INTEL_INLINEREPORTEMITTER_H
