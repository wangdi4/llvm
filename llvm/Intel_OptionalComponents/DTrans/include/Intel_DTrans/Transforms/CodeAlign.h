//===---------------- CodeAlign.h - DTransCodeAlignPass -------------------===//
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares the DTrans Code Align pass
//
//===----------------------------------------------------------------------===//

#if !INTEL_FEATURE_SW_DTRANS
#error CodeAlign.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_TRANSFORMS_CODEALIGN_H
#define INTEL_DTRANS_TRANSFORMS_CODEALIGN_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class WholeProgramInfo;

namespace dtrans {

// This class implements the DTrans code align pass.
class CodeAlignPass : public PassInfoMixin<dtrans::CodeAlignPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // This is used to share the core implementation with the legacy pass.
  bool runImpl(Module &M, WholeProgramInfo &WPInfo);
};

} // namespace dtrans

ModulePass *createDTransCodeAlignWrapperPass();

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_CODEALIGN_H
