//===----  Intel_IPOPrefetch.h - Prefetch in IPO   --------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass handles non-loop-based prefetch work conducted in IPO.

#ifndef LLVM_TRANSFORMS_IPO_INTEL_IPOPREFETCH_H
#define LLVM_TRANSFORMS_IPO_INTEL_IPOPREFETCH_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class ModulePass;

class IntelIPOPrefetchPass
    : public PassInfoMixin<IntelIPOPrefetchPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};
} // End namespace llvm

#endif // LLVM_TRANSFORMS_IPO_INTEL_IPOPREFETCH_H
