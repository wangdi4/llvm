//===------ DPCPPPrepareKernelForVecClone.h - Class definition -*- C++-*---===//
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
/// This file defines the DPCPPPrepareKernelForVecClone pass class.
// ===--------------------------------------------------------------------=== //
#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_PREPARE_FOR_VEC_CLONE_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_PREPARE_FOR_VEC_CLONE_H

#include "llvm/Analysis/TargetTransformInfo.h"

namespace llvm {

class Function;

class DPCPPPrepareKernelForVecClone {

private:
  TargetTransformInfo &TTI;
  Function *F;

  /// Kind of parameter in a function with 'declare simd' directive.
  enum ParamKindTy { LinearWithVarStride, Linear, Uniform, Vector };

  /// Attribute set of the parameter.
  struct ParamAttrTy {
    ParamKindTy Kind = Uniform;
    unsigned StrideOrArg;
    unsigned Alignment;

    ParamAttrTy(ParamKindTy Kind, unsigned StrideOrArg = 0,
                unsigned Alignment = 0)
        : Kind(Kind), StrideOrArg(StrideOrArg), Alignment(Alignment) {}
  };

  enum MaskTy { MT_UndefinedMask = 0, MT_NonMask, MT_Mask };

  /// Adds vector-variant attributes to each kernel.
  void addVectorVariantAttrsToKernel();

  /// Encodes vector-variants.
  void createEncodingForVectorVariants(Function *Fn, unsigned VlenVal,
                                       ArrayRef<ParamAttrTy> ParamAttrs,
                                       MaskTy State);

public:
  DPCPPPrepareKernelForVecClone(Function *F, TargetTransformInfo &TTI);
  void run();

}; // end pass class

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_PREPARE_FOR_VEC_CLONE_H
