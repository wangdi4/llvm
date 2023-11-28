#if INTEL_FEATURE_SW_ADVANCED
//===------- Intel_PartialInline.h - Simple Partial Inline        -*------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass performs a simplified partial inlining for small functions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_INTEL_PI_H
#define LLVM_TRANSFORMS_IPO_INTEL_PI_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

/// Pass to perform partial inlining.
class IntelPartialInlinePass : public PassInfoMixin<IntelPartialInlinePass> {

public:
  IntelPartialInlinePass();
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &);
};

}
#endif // LLVM_TRANSFORMS_IPO_INTEL_PI_H

#endif // INTEL_FEATURE_SW_ADVANCED
