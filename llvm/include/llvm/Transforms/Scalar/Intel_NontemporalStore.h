//===- Intel_NonTemporalStore.h - Unaligned nontemporal store ---*- C++ -*-===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SCALAR_NONTEMPORALSTORE_H
#define LLVM_TRANSFORMS_SCALAR_NONTEMPORALSTORE_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class NontemporalStorePass : public PassInfoMixin<NontemporalStorePass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &);
};

}

#endif //LLVM_TRANSFORMS_SCALAR_NONTEMPORALSTORE_H
