//===------------------- VectorVariantFillIn.h -*- C++ -*------------------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef DPCPP_KERNEL_TRANSFORMS_VECTORVARIANT_VECTORVARIANTFILLIN_H
#define DPCPP_KERNEL_TRANSFORMS_VECTORVARIANT_VECTORVARIANTFILLIN_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class VectorVariantFillIn : public PassInfoMixin<VectorVariantFillIn> {
public:
  static StringRef name() { return "VectorVariantFillIn"; }

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);

  bool runImpl(Module &M);
};

} // namespace llvm

#endif // DPCPP_KERNEL_TRANSFORMS_VECTORVARIANT_VECTORVARIANTFILLIN_H
