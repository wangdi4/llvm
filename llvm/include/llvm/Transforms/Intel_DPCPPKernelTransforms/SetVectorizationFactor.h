//==-- SetVectorizationFactor.h - Set vectorization factor ------- C++ -*---==//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_SET_VF_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_SET_VF_H

#include "llvm/Analysis/Intel_VectorVariant.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

/// Set "recommended_vector_length" metadata (A.K.A. Vectorization Factor) for
/// each function.
class SetVectorizationFactorPass
    : public PassInfoMixin<SetVectorizationFactorPass> {
public:
  explicit SetVectorizationFactorPass(
      VectorVariant::ISAClass ISA = VectorVariant::XMM);

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

  // Glue for old PM.
  bool runImpl(Function &F);

private:
  VectorVariant::ISAClass ISA;
};
} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_SET_VF_H
