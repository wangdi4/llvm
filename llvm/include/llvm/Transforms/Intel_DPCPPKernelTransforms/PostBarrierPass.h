//==--- PostBarrierPass.h - Resolve builtins created by Barrier - C++ -*----==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_POST_BARRIER_PASS_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_POST_BARRIER_PASS_H

#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

namespace llvm {

class PostBarrier : public ModulePass {
public:
  static char ID;

  PostBarrier();

  ~PostBarrier() {}

  llvm::StringRef getPassName() const override {
    return "Intel DPCPP Kernel Post Barrier";
  }

  bool runOnModule(Module &M) override;

private:
  /// Resolve __builtin_get_local_size calls created in Barrier.
  bool replaceGetLocalSizeCall(Module &M);

  /// Resolve get_special_buffer calls created in Barrier.
  bool resolveSpecialBufferCall(Module &M);
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_POST_BARRIER_PASS_H
