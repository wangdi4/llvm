//===-------- Intel_InlineReportSetup.h - Inline Report Setup ------------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
//  This file implements an inlining report setup pass.
//
//===----------------------------------------------------------------------===//
//

#ifndef LLVM_TRANSFORMS_IPO_INTEL_INLINEREPORTSETUP_H
#define LLVM_TRANSFORMS_IPO_INTEL_INLINEREPORTSETUP_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/IPO/Intel_MDInlineReport.h"

namespace llvm {

class InlineReportSetupPass : public PassInfoMixin<InlineReportSetupPass> {
  InlineReportBuilder *MDIR;

public:
  static bool isRequired() { return true; }
  InlineReportSetupPass();
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
  InlineReportBuilder *getMDReport() { return MDIR; }
};
} // namespace llvm

#endif // LLVM_TRANSFORMS_IPO_INTEL_INLINEREPORTSETUP_H
