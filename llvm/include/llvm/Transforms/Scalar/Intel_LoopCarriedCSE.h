//===--- LoopCarriedCSE.h- Implements Loop Carried CSE ----------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SCALAR_LOOPCARRIEDCSE_H
#define LLVM_TRANSFORMS_SCALAR_LOOPCARRIEDCSE_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class Function;

class LoopCarriedCSEPass : public PassInfoMixin<LoopCarriedCSEPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_LOOPCARRIEDCSE_H
