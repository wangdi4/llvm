//===- ArgNoAliasProp.h ------ Add noalias to function args -----*- C++ -*-===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_ARG_NOALIAS_PROP_H
#define LLVM_TRANSFORMS_IPO_ARG_NOALIAS_PROP_H

#include "llvm/IR/PassManager.h"

namespace llvm {

struct ArgNoAliasPropPass : public PassInfoMixin<ArgNoAliasPropPass> {
  ArgNoAliasPropPass() {}
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_IPO_ARG_NOALIAS_PROP_H
