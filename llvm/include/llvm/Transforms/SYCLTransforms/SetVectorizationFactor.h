//==-- SetVectorizationFactor.h - Set vectorization factor ------- C++ -*---==//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_SET_VF_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_SET_VF_H

#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/SYCLTransforms/VFAnalysis.h"

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

  static bool isRequired() { return true; }

};
} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_SET_VF_H
