//===----  Intel_AutoCPUClone.h - Intel Automatic CPU Dispatch -----------===//
//
// Copyright (C) Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_INTEL_AUTOCPUCLONE_H
#define LLVM_TRANSFORMS_IPO_INTEL_AUTOCPUCLONE_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class Module;

/// A pass that multiversions functions(auto-cpu-dispatch).
class AutoCPUClonePass : public PassInfoMixin<AutoCPUClonePass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &);
};
} // end namespace llvm

#endif // LLVM_TRANSFORMS_IPO_INTEL_AUTOCPUCLONE_H
