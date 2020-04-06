//===------------ DPCPPKernelPostVec.h - Class definition -*- C++ ---------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file defines the DPCPPKernelPostVec pass.
// ===--------------------------------------------------------------------=== //
#ifndef BACKEND_VECTORIZER_DPCPPVECCLONE_DPCPPPOSTVECT_H
#define BACKEND_VECTORIZER_DPCPPVECCLONE_DPCPPPOSTVECT_H

#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

namespace llvm {
class DPCPPKernelPostVec : public llvm::ModulePass {

private:
  bool runOnModule(llvm::Module &M) override;

  /// Checks if there are openmp directives in the kernel. If not, then the
  /// kernel was vectorized.
  bool isKernelVectorized(llvm::Function *F);

public:
  static char ID;

  DPCPPKernelPostVec();

  /// Returns the name of the pass
  llvm::StringRef getPassName() const override {
    return "VPlan post vectorization pass for DPCPP kernels";
  }
};
} // namespace llvm

#endif // BACKEND_VECTORIZER_DPCPPVECCLONE_DPCPPPOSTVECT_H
