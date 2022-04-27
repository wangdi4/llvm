//===------------- ReorderFieldsOP.h - DTransReorderFieldsPass ------------===//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares the DTrans reorder fields optimization pass for opaque
// pointers.
//
//===----------------------------------------------------------------------===//

#if !INTEL_FEATURE_SW_DTRANS
#error ReorderFieldsOP.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_TRANSFORMS_REORDERFIELDSOP_H
#define INTEL_DTRANS_TRANSFORMS_REORDERFIELDSOP_H

#include "llvm/IR/PassManager.h"

namespace llvm {
class Function;
class Module;
class TargetLibraryInfo;
class WholeProgramInfo;

namespace dtransOP {
class DTransSafetyInfo;

class ReorderFieldsOPPass
    : public PassInfoMixin<dtransOP::ReorderFieldsOPPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // This is used to share the core implementation with the legacy pass.
  bool
  runImpl(Module &M, DTransSafetyInfo *DTInfo,
          std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
          WholeProgramInfo &WPInfo);
};

} // namespace dtransOP

ModulePass *createDTransReorderFieldsOPWrapperPass();

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_REORDERFIELDSOP_H
