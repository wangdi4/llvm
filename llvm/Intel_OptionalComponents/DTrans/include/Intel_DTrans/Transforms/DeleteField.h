//===--------------- DeleteField.h - DTransDeleteFieldPass  ---------------===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
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

#if !INTEL_INCLUDE_DTRANS
#error DeleteField.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#ifndef INTEL_DTRANS_TRANSFORMS_DELETEFIELD_H
#define INTEL_DTRANS_TRANSFORMS_DELETEFIELD_H

#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

namespace dtrans {

/// Pass to perform DTrans optimizations.
class DeleteFieldPass : public PassInfoMixin<dtrans::DeleteFieldPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // This is used to share the core implementation with the legacy pass.
  bool
  runImpl(Module &M, DTransAnalysisInfo &Info,
          std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
          WholeProgramInfo &WPInfo);
};

} // namespace dtrans

ModulePass *createDTransDeleteFieldWrapperPass();

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_DELETEFIELD_H
