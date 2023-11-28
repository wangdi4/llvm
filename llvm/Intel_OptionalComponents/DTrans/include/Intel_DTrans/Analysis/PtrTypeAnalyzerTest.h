//===----------------------PtrTypeAnalyzerTest.h--------------------------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

// This file defines passes used for testing the PtrTypeAnalyzer class.

#ifndef INTEL_DTRANS_ANALYSIS_PTRTYPEANALYZERTEST_H
#define INTEL_DTRANS_ANALYSIS_PTRTYPEANALYZERTEST_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

#if !INTEL_FEATURE_SW_DTRANS
#error PtrTypeAnalyzerTest.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

namespace llvm {
namespace dtransOP {

// This pass is strictly for testing the PtrTypeAnalyzer class. This pass will
// not be part of the compiler pipeline.
#if !INTEL_PRODUCT_RELEASE
class DTransPtrTypeAnalyzerTestPass
    : public PassInfoMixin<DTransPtrTypeAnalyzerTestPass> {

public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
};
#endif // !INTEL_PRODUCT_RELEASE

} // end namespace dtransOP

} // end namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_PTRTYPEANALYZERTEST_H
