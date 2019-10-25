//===-------- SOAToAOSPrepare.h - DTransSOAToAOSPreparePass  --------------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares the DTrans SOAToAOS prepare pass to perform
// transformations that enable SOATOSOA for more candidates.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DTRANS_TRANSFORMS_SOATOAOS_PREPARE_H
#define INTEL_DTRANS_TRANSFORMS_SOATOAOS_PREPARE_H

#if !INTEL_INCLUDE_DTRANS
#error SOAToAOSPrepare.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/Transforms/MemInitTrimDownInfoImpl.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {
namespace dtrans {

/// Pass to perform DTrans AOS to SOA Prepare transformations.
class SOAToAOSPreparePass : public PassInfoMixin<SOAToAOSPreparePass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // This is used to share the core implementation with the legacy pass.
  bool runImpl(Module &M, DTransAnalysisInfo &DTInfo, MemGetTLITy GetTLI,
               WholeProgramInfo &WPInfo,
               dtrans::MemInitDominatorTreeType &GetDT);
};

} // namespace dtrans

ModulePass *createDTransSOAToAOSPrepareWrapperPass();

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_SOATOAOS_PREPARE_H
