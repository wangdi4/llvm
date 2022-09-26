//===--VPODirectiveCleanup.h-----------------------------------------------===//
//
//   Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements VPO directive cleanup pass.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_VPO_VECOPT_VPO_DIRECTIVE_CLEANUP_H
#define LLVM_TRANSFORMS_INTEL_VPO_VECOPT_VPO_DIRECTIVE_CLEANUP_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

namespace llvm {

class VPODirectiveCleanupPass : public PassInfoMixin<VPODirectiveCleanupPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

  bool runImpl(Function &F);
  static bool isRequired() { return true; }

private:
  bool removeScanFence(Function &F);
};

namespace vpo {

class VPODirectiveCleanup : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid

  VPODirectiveCleanup() : FunctionPass(ID) {
    initializeVPODirectiveCleanupPass(*PassRegistry::getPassRegistry());
  }
  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
private:
  VPODirectiveCleanupPass Impl;
};

} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_VPO_VECOPT_VPO_DIRECTIVE_CLEANUP_H
