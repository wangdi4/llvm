//===--------------------- UpdateCallAttrs.h -*- C++ -*--------------------===//
//
// Copyright (C) 2020-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef DPCPP_KERNEL_TRANSFORMS_UPDATE_CALL_ATTRS_H
#define DPCPP_KERNEL_TRANSFORMS_UPDATE_CALL_ATTRS_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class UpdateCallAttrs : public PassInfoMixin<UpdateCallAttrs> {
public:
  static StringRef name() { return "UpdateCallAttrs"; }

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);

  bool runImpl(Module &M);
};

} // namespace llvm

#endif // DPCPP_KERNEL_TRANSFORMS_UPDATE_CALL_ATTRS_H
