//===- AddTLSGlobals.h - AddTLSGlobals pass C++ -*-------------------------===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_ADD_TLS_GLOBALS_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_ADD_TLS_GLOBALS_H

#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/ImplicitArgsAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LocalBufferAnalysis.h"

namespace llvm {
class AddTLSGlobalsPass : public PassInfoMixin<AddTLSGlobalsPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // Glue for old PM.
  bool runImpl(Module &M, ImplicitArgsInfo *IAInfo);

  static bool isRequired() { return true; }

private:
  /// @brief The llvm module this pass needs to update
  Module *M;

  LLVMContext *Ctx;
  /// @brief Local memory pointer TLS global
  GlobalVariable *LocalMemBase;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_ADD_TLS_GLOBALS_H
