//===------ SOAToAOSOPPrepare.h - DTransSOAToAOSOPPreparePass  ------------===//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
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

#ifndef INTEL_DTRANS_TRANSFORMS_SOATOAOSOP_PREPARE_H
#define INTEL_DTRANS_TRANSFORMS_SOATOAOSOP_PREPARE_H

#if !INTEL_FEATURE_SW_DTRANS
#error SOAToAOSOPPrepare.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"
#include "Intel_DTrans/Transforms/StructOfArraysOPInfoImpl.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {
namespace dtransOP {

/// Pass to perform DTrans AOS to SOA Prepare transformations.
class SOAToAOSOPPreparePass : public PassInfoMixin<SOAToAOSOPPreparePass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // This is used to share the core implementation with the legacy pass.
  bool runImpl(Module &M, DTransSafetyInfo &DTInfo, SOAGetTLITy GetTLI,
               WholeProgramInfo &WPInfo, SOADominatorTreeType &GetDT,
               function_ref<BlockFrequencyInfo &(Function &)> GetBFI);
};

} // namespace dtransOP

ModulePass *createDTransSOAToAOSOPPrepareWrapperPass();

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_SOATOAOSOP_PREPARE_H
