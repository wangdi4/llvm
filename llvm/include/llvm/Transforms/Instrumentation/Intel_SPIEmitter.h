//===- Transforms/Instrumentation/Intel_SPIEmitter.h -----------*- C++ -*-===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines the pass that can emit a static profile information (SPI)
// file. A SPI file is an alternative method of supplying the code coverage
// mapping data structures to the llvm-cov tool. This method allows for easily
// generating the code coverage reports on a project consisting of one or more
// binaries or shared objects without the executable images being available when
// running llvm-cov, because information about all the files of the project can
// be contained within the single SPI file.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INSTRUMENTATION_INTEL_SPIEMITTER_H
#define LLVM_TRANSFORMS_INSTRUMENTATION_INTEL_SPIEMITTER_H

#include "llvm/IR/PassManager.h"

namespace llvm {

// This pass takes the global variables that were created by the front-end and
// InstrProfiling pass that hold information about mapping profile data to code
// coverage regions, if they are present, and stores them into a standalone file
// that can be used by the llvm-cov tool.
class SPIEmitter : public PassInfoMixin<SPIEmitter> {
public:
  SPIEmitter() {}

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_INSTRUMENTATION_INTEL_SPIEMITTER_H
