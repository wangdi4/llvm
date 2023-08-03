//===- Intel_GVBasedMultiVersioning.h -------------------------------------===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SCALAR_INTEL_GVBASEDMULTIVERSIONING_H
#define LLVM_TRANSFORMS_SCALAR_INTEL_GVBASEDMULTIVERSIONING_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"

namespace llvm {
class GVBasedMultiVersioningPass
    : public PassInfoMixin<GVBasedMultiVersioningPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};
} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_INTEL_GVBASEDMULTIVERSIONING_H
