//===----  Intel_AdvancedFastCall.h - Intel Advanced Fast Call   --------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// The pass extends the cases that the "fastcc" calling convention can be
// applied to.

#ifndef LLVM_TRANSFORMS_IPO_INTEL_ADVANCEDFASTCALL_H
#define LLVM_TRANSFORMS_IPO_INTEL_ADVANCEDFASTCALL_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class ModulePass;

class IntelAdvancedFastCallPass
    : public PassInfoMixin<IntelAdvancedFastCallPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};
} // End namespace llvm

#endif // LLVM_TRANSFORMS_IPO_INTEL_ADVANCEDFASTCALL_H
