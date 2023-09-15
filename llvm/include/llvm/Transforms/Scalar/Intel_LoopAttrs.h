//===------       Intel_LoopAttrs.h - Compute loop attributes       -*-----===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass goes through each loop in a function and checks which attributes
// or metadata can be added for the loop. If an attribute affects the function,
// then this pass will update the function attributes too.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SCALAR_INTEL_LOOPATTRS_H
#define LLVM_TRANSFORMS_SCALAR_INTEL_LOOPATTRS_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class IntelLoopAttrsPass : public PassInfoMixin<IntelLoopAttrsPass> {
  const bool EnableDTrans;

public:
  IntelLoopAttrsPass() : EnableDTrans(false) { }
  IntelLoopAttrsPass(bool EnableDTrans) : EnableDTrans(EnableDTrans) { }
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

FunctionPass *createIntelLoopAttrsWrapperPass(bool EnableDTrans = false);

} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_INTEL_LOOPATTRS_H