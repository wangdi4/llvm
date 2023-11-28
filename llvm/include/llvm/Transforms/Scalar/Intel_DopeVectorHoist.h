//===------ Intel_DopeVectorHoist.h -  -*------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// Header file to implement hosting base address loads of Dope Vector
// arguments.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SCALAR_INTEL_DOPEVECTORHOIST_H
#define LLVM_TRANSFORMS_SCALAR_INTEL_DOPEVECTORHOIST_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

/// Pass to perform function DV Hoist.
class DopeVectorHoistPass : public PassInfoMixin<DopeVectorHoistPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

FunctionPass *createDopeVectorHoistWrapperPass();

} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_INTEL_DOPEVECTORHOIST_H
