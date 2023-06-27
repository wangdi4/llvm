//===------- Intel_DeadDopeVectorElimination.h ----------------------------===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// Dead Dope Vector Elimination
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_INTEL_DEADDOPEVECTORELIMINATION_H
#define LLVM_TRANSFORMS_IPO_INTEL_DEADDOPEVECTORELIMINATION_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

///
/// Pass to perform dead dope vector elimination.
///
/// The goal is to determine for formal parameters (Fortran dummy arguments)
/// that are pointers to dope vectors, which lower bounds, strides, and
/// and extents stored in those dope vectors are constant, and then replace
/// accesses of these lower bounds, strides, and extents with those constant
/// values.
///
class DeadDopeVectorEliminationPass
    : public PassInfoMixin<DeadDopeVectorEliminationPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // end namespace llvm
#endif // LLVM_TRANSFORMS_IPO_INTEL_DEADDOPEVECTORELIMINATION_H
