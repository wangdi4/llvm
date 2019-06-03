//===-------------- OCLPrepareKernelForVecClone.h - Class definition -*-
// C++-*---------------===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file defines the OCLPrepareKernelForVecClone pass class.
// ===--------------------------------------------------------------------=== //
#ifndef BACKEND_VECTORIZER_OCLVECCLONE_PREPAREFORVECCLONE_H
#define BACKEND_VECTORIZER_OCLVECCLONE_PREPAREFORVECCLONE_H

#include "OCLPassSupport.h"
#include "VecConfig.h"
#include "WIAnalysis.h"

#include <string>

namespace intel {

class OCLPrepareKernelForVecClone {

private:
  // Configuration options
  const Intel::CPUId *CPUId = nullptr;

  // Kind of parameter in a function with 'declare simd' directive.
  enum ParamKindTy { LinearWithVarStride, Linear, Uniform, Vector };

  // Attribute set of the parameter.
  struct ParamAttrTy {
    ParamKindTy Kind = Uniform;
    unsigned StrideOrArg;
    unsigned Alignment;

    ParamAttrTy(ParamKindTy Kind, unsigned StrideOrArg = 0,
                unsigned Alignment = 0)
        : Kind(Kind), StrideOrArg(StrideOrArg), Alignment(Alignment) {}
  };

  enum MaskTy { MT_UndefinedMask = 0, MT_NonMask, MT_Mask };

  // \brief Adds vector-variant attributes to each kernel.
  void addVectorVariantAttrsToKernel(Function *F);

  // \brief Encodes vector-variants.
  void createEncodingForVectorVariants(Function *Fn, unsigned VlenVal,
                                       ArrayRef<ParamAttrTy> ParamAttrs,
                                       MaskTy State);

public:
  void run(Function *F);

  OCLPrepareKernelForVecClone(const Intel::CPUId *CPUId);

  OCLPrepareKernelForVecClone();

}; // end pass class

} // namespace intel

#endif // BACKEND_VECTORIZER_OCLVECCLONE_PREPAREFORVECCLONE_H

