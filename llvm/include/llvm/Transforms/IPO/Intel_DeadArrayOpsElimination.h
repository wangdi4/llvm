//===------- Intel_DeadArrayOpsElimination.h ------------------------------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// Elimination of dead array element operations
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_INTEL_DEADARRAYOPSELIMINATION_H
#define LLVM_TRANSFORMS_IPO_INTEL_DEADARRAYOPSELIMINATION_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

///
/// Pass to perform dead array element operations elimination.
///
/// The goal is to eliminate dead array element operations.
///
class DeadArrayOpsEliminationPass
    : public PassInfoMixin<DeadArrayOpsEliminationPass> {
public:
  DeadArrayOpsEliminationPass(void);
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // end namespace llvm
#endif // LLVM_TRANSFORMS_IPO_INTEL_DEADARRAYOPSELIMINATION_H
