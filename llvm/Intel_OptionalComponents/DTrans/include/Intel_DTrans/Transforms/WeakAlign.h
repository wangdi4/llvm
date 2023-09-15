//===---------------- WeakAlign.h - DTransWeakAlignPass -------------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares the DTrans Weak Align pass
//
//===----------------------------------------------------------------------===//

#if !INTEL_FEATURE_SW_DTRANS
#error WeakAlign.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_TRANSFORMS_WEAKALIGN_H
#define INTEL_DTRANS_TRANSFORMS_WEAKALIGN_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class TargetLibraryInfo;
class WholeProgramInfo;

namespace dtrans {

// This class implements the DTrans weak align pass to allow using qkmalloc with
// the weak memory alignment setting.
class WeakAlignPass : public PassInfoMixin<dtrans::WeakAlignPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  bool
  runImpl(Module &M,
          std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
          WholeProgramInfo &WPInfo);
};

} // namespace dtrans

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_WEAKALIGN_H
