//===-------------- EliminateROFieldAccess.h -----------------------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares the DTrans read-only field access elimination pass.
//
//===----------------------------------------------------------------------===//

#if !INTEL_FEATURE_SW_DTRANS
#error EliminateROFieldAccess.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_TRANSFORMS_ELIMROFIELDACCESS_H
#define INTEL_DTRANS_TRANSFORMS_ELIMROFIELDACCESS_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class WholeProgramInfo;

namespace dtransOP {

class DTransSafetyInfo;

/// Pass to perform elimination.
class EliminateROFieldAccessPass
    : public PassInfoMixin<dtransOP::EliminateROFieldAccessPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  bool runImpl(Module &M, DTransSafetyInfo &DTInfo, WholeProgramInfo &WPInfo);
};

} // namespace dtransOP

} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_ELIMROFIELDACCESS_H
