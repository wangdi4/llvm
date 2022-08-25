//===--------------------- SGSizeCollector.h -*- C++ -*--------------------===//
//
// Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_VECTORIZER_VECTORVARIANT_SGSIZECOLLECTOR_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_VECTORIZER_VECTORVARIANT_SGSIZECOLLECTOR_H

#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class SGSizeCollectorPass : public PassInfoMixin<SGSizeCollectorPass> {
public:
  SGSizeCollectorPass(VFISAKind ISA = VFISAKind::SSE)
      : ISA(ISA) {}

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);

  bool runImpl(Module &M);

private:
  VFISAKind ISA;
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_VECTORIZER_VECTORVARIANT_SGSIZECOLLECTOR_H
