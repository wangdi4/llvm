//===---- Intel_ArgumentAlignment.h - Compute Memory Alignemnt        -*---===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass determines whether a formal argument is always aligned to a
// particular constant value, and uses that information to simplify alignment
// based tests inside the function.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_INTEL_ARGUMENTALIGNMENT_H
#define LLVM_TRANSFORMS_IPO_INTEL_ARGUMENTALIGNMENT_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

/// Pass to perform the alignment computation.
class IntelArgumentAlignmentPass :
    public PassInfoMixin<IntelArgumentAlignmentPass> {

public:
  IntelArgumentAlignmentPass();
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &);
}; // end IntelArgumentAlignmentPass

} // end llvm namespace
#endif // LLVM_TRANSFORMS_IPO_INTEL_ARGUMENTALIGNMENT_H
