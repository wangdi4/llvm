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

/// A pass that multi-versions functions for auto-arch, auto-cpu-dispatch and,
/// -vecabi=cmdtarget support.
class AutoCPUClonePass : public PassInfoMixin<AutoCPUClonePass> {
public:
  AutoCPUClonePass(bool GVV = false);
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &);

private:
  // When true, AutoCPUClonePass multi-versions functions per the targets
  // specified in llvm.vec.auto.cpu.dispatch metadata. Otherwise, pass
  // multi-versions functions per the targets listed in llvm.auto.arch and
  // llvm.auto.cpu.dispatch metadata.
  bool GenerateVectorVariants;
};
} // end namespace llvm

#endif // LLVM_TRANSFORMS_IPO_INTEL_AUTOCPUCLONE_H
