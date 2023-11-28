//===------------------------- Intel_ForceInline.h ------------------------===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass forces aggressive inlining by transforming all inlinehint
// attributes to alwaysinline. It's implemented as a module pass to ensure
// the consistency of the transformation and the resulting inlining.
//
//===----------------------------------------------------------------------===//
//

#ifndef LLVM_TRANSFORMS_IPO_INTEL_FORCEINLINE_H
#define LLVM_TRANSFORMS_IPO_INTEL_FORCEINLINE_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class InlineForceInlinePass : public PassInfoMixin<InlineForceInlinePass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
  static bool isRequired() { return true; }
};
} // namespace llvm

#endif // LLVM_TRANSFORMS_IPO_INTEL_FORCEINLINE_H
