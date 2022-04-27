//===--------------- CodeAlignOP.h - DTransCodeAlignPass ------------------===//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares the DTrans Code Align pass for opaque pointers
//
//===----------------------------------------------------------------------===//

#if !INTEL_FEATURE_SW_DTRANS
#error CodeAlignOP.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_TRANSFORMS_CODEALIGNOP_H
#define INTEL_DTRANS_TRANSFORMS_CODEALIGNOP_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class WholeProgramInfo;

namespace dtransOP {
class DTransSafetyInfo;

// This class implements the DTrans code align pass.
class CodeAlignPass : public PassInfoMixin<dtransOP::CodeAlignPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // This is used to share the core implementation with the legacy pass.
  bool runImpl(Module &M, WholeProgramInfo &WPInfo, DTransSafetyInfo *DTInfo);
};

} // namespace dtransOP

ModulePass *createDTransCodeAlignOPWrapperPass();

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_CODEALIGNOP_H
