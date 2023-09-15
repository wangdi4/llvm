//===--- Intel_SYCLPrepareKernelForVecClone.h - Class definition -*- C++-*===//
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
/// This file defines the SYCLPrepareKernelForVecClone pass class.
// ===--------------------------------------------------------------------=== //
#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_PREPARE_FOR_VEC_CLONE_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_PREPARE_FOR_VEC_CLONE_H

#include "llvm/Analysis/VectorUtils.h"

namespace llvm {

class SYCLPrepareKernelForVecClone {

private:
  VFISAKind ISA;

  /// Adds vector-variant attributes to each kernel.
  void addVectorVariantAttrsToKernel(Function &F);

  /// Encodes vector-variants.
  void createEncodingForVectorVariants(Function &F, unsigned VF,
                                       ArrayRef<VFParamKind> ParamKinds,
                                       bool NeedMaskedVariant);

public:
  explicit SYCLPrepareKernelForVecClone(VFISAKind ISA);
  void run(Function &F);

}; // end pass class

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_PREPARE_FOR_VEC_CLONE_H
