//==DPCPPKernelWGLoopCreator.h - Create WG loops in DPCPP kernels- C++ -*---==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
#ifndef INTEL_DPCPP_KERNEL_TRANSFORMS_WGLOOPCREATOR_H
#define INTEL_DPCPP_KERNEL_TRANSFORMS_WGLOOPCREATOR_H

#include "llvm/IR/PassManager.h"

namespace llvm {

/// Create workgroup loop.
class DPCPPKernelWGLoopCreatorPass
    : public PassInfoMixin<DPCPPKernelWGLoopCreatorPass> {
public:
  DPCPPKernelWGLoopCreatorPass(bool UseTLSGlobals = false)
      : UseTLSGlobals(UseTLSGlobals) {}

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  static bool isRequired() { return true; }

private:
  bool UseTLSGlobals;
};

} // namespace llvm

#endif // INTEL_DPCPP_KERNEL_TRANSFORMS_WGLOOPCREATOR_H
