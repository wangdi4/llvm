//===--------------- MemInitTrimDown.h - DTransMemInitTrimDownPass --------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares the Initial Memory Allocation Trim Down optimization
// pass.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DTRANS_TRANSFORMS_MEMINITTRIMDOWN_H
#define INTEL_DTRANS_TRANSFORMS_MEMINITTRIMDOWN_H

#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

#include "Intel_DTrans/Transforms/MemInitTrimDownInfoImpl.h"

namespace llvm {

namespace dtrans {

/// Pass to perform Initial Memory Allocation Trim Down optimization.
class MemInitTrimDownPass : public PassInfoMixin<MemInitTrimDownPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // This is used to share the core implementation with the legacy pass.
  bool runImpl(Module &M, DTransAnalysisInfo &Info, MemGetTLITy GetTLI,
               WholeProgramInfo &WPInfo, MemInitDominatorTreeType &GetDT);
};

} // namespace dtrans

ModulePass *createDTransMemInitTrimDownWrapperPass();

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_MEMINITTRIMDOWN_H
