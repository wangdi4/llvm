//===--------------- AOSToSOA.h - DTransAOSToSOAPass  ---------------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares the DTrans Array of Structures to Structure of Arrays
// data layout optimization pass.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_DTRANS_AOSTOSOA_H
#define LLVM_TRANSFORMS_INTEL_DTRANS_AOSTOSOA_H

#include "llvm/Analysis/Intel_DTrans/DTransAnalysis.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

namespace dtrans {

/// Pass to perform DTrans AOS to SOA optimizations.
class AOSToSOAPass : public PassInfoMixin<dtrans::AOSToSOAPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // This is used to share the core implementation with the legacy pass.
  bool runImpl(Module &M, DTransAnalysisInfo &DTInfo,
               const TargetLibraryInfo &TLI);
};

} // namespace dtrans

ModulePass *createDTransAOSToSOAWrapperPass();

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DTRANS_AOSTOSOA_H
