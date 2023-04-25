//===-------- SOAToAOSPrepare.h - DTransSOAToAOSPreparePass  --------------===//
//
// Copyright (C) 2019-2023 Intel Corporation. All rights reserved.
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

#if !INTEL_FEATURE_SW_DTRANS
#error SOAToAOSPrepare.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/Transforms/StructOfArraysInfoImpl.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {
namespace dtrans {

/// Pass to perform DTrans AOS to SOA Prepare transformations.
class SOAToAOSPreparePass : public PassInfoMixin<SOAToAOSPreparePass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  bool runImpl(Module &M, DTransAnalysisInfo &DTInfo, SOAGetTLITy GetTLI,
               WholeProgramInfo &WPInfo,
               dtrans::SOADominatorTreeType &GetDT);
};

} // namespace dtrans

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_SOATOAOS_PREPARE_H
