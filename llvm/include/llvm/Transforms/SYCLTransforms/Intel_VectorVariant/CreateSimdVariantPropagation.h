//===--------------- CreateSimdVariantPropagation.h -*- C++ -*-------------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_VECTORVARIANT_CREATESIMDVARIANTPROPAGATION_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_VECTORVARIANT_CREATESIMDVARIANTPROPAGATION_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class CreateSimdVariantPropagation
    : public PassInfoMixin<CreateSimdVariantPropagation> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);

  bool runImpl(Module &M);

  static bool isRequired() { return true; }
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_VECTORVARIANT_CREATESIMDVARIANTPROPAGATION_H
