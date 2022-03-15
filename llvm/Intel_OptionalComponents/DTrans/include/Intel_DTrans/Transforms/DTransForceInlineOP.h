//===----- DTransForceInlineOP.h - Force inline/noinline for DTrans ------===//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DTRANS_TRANSFORMS_FORCEINLINEOP_H
#define INTEL_DTRANS_TRANSFORMS_FORCEINLINEOP_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {
namespace dtransOP {

// This pass will force inline or noinline on Functions to support
// DTrans transformations. This is the opaque pointer friendly version.
class DTransForceInlineOPPass
    : public PassInfoMixin<DTransForceInlineOPPass> {

public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);

  bool runImpl(Module &M);
};

} // end namespace dtransOP

ModulePass *createDTransForceInlineOPWrapperPass();

} // end namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_FORCEINLINEOP_H
