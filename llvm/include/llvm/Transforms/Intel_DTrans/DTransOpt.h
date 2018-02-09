//===---------------- DTransOpt.h - DTransOpt placeholder  -*--------------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass is a placeholder for future DTrans optimization passes.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_DTRANS_DTRANSOPT_H
#define LLVM_TRANSFORMS_INTEL_DTRANS_DTRANSOPT_H

#include "llvm/Analysis/Intel_DTrans/DTransAnalysis.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

/// Pass to perform DTrans optimizations.
class DTransOptPass : public PassInfoMixin<DTransOptPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &);

  PreservedAnalyses runImpl(Module &M, DTransAnalysisInfo &DTInfo);
};

ModulePass *createDTransOptWrapperPass();

} // namespace llvm
#endif // LLVM_TRANSFORMS_INTEL_DTRANS_DTRANSOPT_H
