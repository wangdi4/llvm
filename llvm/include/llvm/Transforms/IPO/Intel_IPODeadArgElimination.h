//=--- Intel_IPODeadArgElimination.h - Simplified dead arg elimination  -*--=//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass implements a simplified dead argument elimination with IPO
// analysis. The goal is to eliminate those arguments that initializes data but
// the actual value is not used across multiple functions.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_INTEL_IPODEADARGELIMINATION_H
#define LLVM_TRANSFORMS_IPO_INTEL_IPODEADARGELIMINATION_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class IntelIPODeadArgEliminationPass
    : public PassInfoMixin<IntelIPODeadArgEliminationPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_IPO_INTEL_IPODEADARGELIMINATION_H