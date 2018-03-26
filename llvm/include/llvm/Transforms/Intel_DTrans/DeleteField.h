//===--------------- DeleteField.h - DTransDeleteFieldPass  ---------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares the DTrans delete field optimization pass.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_DTRANS_DELETEFIELD_H
#define LLVM_TRANSFORMS_INTEL_DTRANS_DELETEFIELD_H

#include "llvm/Analysis/Intel_DTrans/DTransAnalysis.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

namespace dtrans {

/// Pass to perform DTrans optimizations.
class DeleteFieldPass : public PassInfoMixin<dtrans::DeleteFieldPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &);

  // This is used to share the core implementation with the legacy pass.
  PreservedAnalyses runImpl(Module &M, DTransAnalysisInfo &);
};

} // namespace dtrans

ModulePass *createDTransDeleteFieldWrapperPass();

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DTRANS_DELETEFIELD_H
