//===--Intel_FoldWPIntrinsic.h - Intrinsic wholeprogramsafe Lowering -*-===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// This pass traverses through the IR and replaces the calls to the intrinsic
// llvm.intel.wholeprogramsafe with true if whole program safe was detected.
// Else, replace the calls with false. The intrinsic
// llvm.intel.wholeprogramsafe should be removed completely after this process
// since it won't be lowered. See the language reference manual for more
// information.

#ifndef LLVM_TRANSFORMS_IPO_INTEL_FOLDWPINTRINSIC_H
#define LLVM_TRANSFORMS_IPO_INTEL_FOLDWPINTRINSIC_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

/// Pass to fold the llvm.intel.wholeprogramsafe intrinsic.
class IntelFoldWPIntrinsicPass :
    public PassInfoMixin<IntelFoldWPIntrinsicPass> {

public:
  IntelFoldWPIntrinsicPass();
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &);
}; // end IntelFoldWPIntrinPass

} // end llvm namespace
#endif // LLVM_TRANSFORMS_IPO_INTEL_FOLDWPINTRINSIC_H
