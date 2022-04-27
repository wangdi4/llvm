//===----- DTransForceInline.h - Force inline/noinline for DTrans --------===//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DTRANS_TRANSFORMS_FORCEINLINE_H
#define INTEL_DTRANS_TRANSFORMS_FORCEINLINE_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {
namespace dtrans {

// This pass will force inline or noinline on Functions to support
// DTrans transformations.
class DTransForceInlinePass
    : public PassInfoMixin<DTransForceInlinePass> {

public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);

  bool runImpl(Module &M);
};

} // end namespace dtrans

ModulePass *createDTransForceInlineWrapperPass();

} // end namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_FORCEINLINE_H
