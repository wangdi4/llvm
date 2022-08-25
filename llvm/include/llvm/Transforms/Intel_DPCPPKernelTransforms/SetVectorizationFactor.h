//==-- SetVectorizationFactor.h - Set vectorization factor ------- C++ -*---==//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_SET_VF_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_SET_VF_H

#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/VFAnalysis.h"

namespace llvm {

/// Set "recommended_vector_length" metadata (A.K.A. Vectorization Factor) for
/// kernel functions.
class SetVectorizationFactorPass
    : public PassInfoMixin<SetVectorizationFactorPass> {
public:
  explicit SetVectorizationFactorPass(
      VFISAKind ISA = VFISAKind::SSE);

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // Glue for old PM.
  bool runImpl(Module &M, const VFAnalysisInfo *VFInfo);
};
} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_SET_VF_H
