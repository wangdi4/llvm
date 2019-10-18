//===--------------- DynClone.h - DTransDynClonePass ------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares the DTrans Dynamic Cloning optimization pass.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DTRANS_TRANSFORMS_DYNCLONE_H
#define INTEL_DTRANS_TRANSFORMS_DYNCLONE_H

#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

namespace dtrans {

using LoopInfoFuncType = std::function<LoopInfo &(Function &)>;
using DynGetTLITy = std::function<const TargetLibraryInfo &(Function &)>;

/// Pass to perform Dynamic Cloning optimization.
class DynClonePass : public PassInfoMixin<DynClonePass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // This is used to share the core implementation with the legacy pass.
  bool runImpl(Module &M, DTransAnalysisInfo &Info, DynGetTLITy GetTLI,
               WholeProgramInfo &WPInfo, LoopInfoFuncType &GetLI);
};

} // namespace dtrans

ModulePass *createDTransDynCloneWrapperPass();

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_DYNCLONE_H
