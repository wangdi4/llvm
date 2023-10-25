//===- Transforms/Instrumentation/Intel_SPIEmitter.cpp ---------*- C++ -*-===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Instrumentation/Intel_SPIEmitter.h"

#include "llvm/Pass.h"

using namespace llvm;

PreservedAnalyses SPIEmitter::run(Module &M, ModuleAnalysisManager &AM) {
  // TODO: Implement pass to write code coverage mapping data to file.

  return PreservedAnalyses::all();
}
