//===--- VPOParoptPrepare.h --- Paropt Prepare Class Support -*- C++ --*---===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file provides the interface for the legacy thread private transformation.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_PAROPT_TPV_H
#define LLVM_TRANSFORMS_VPO_PAROPT_TPV_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

namespace vpo {

/// Legacy thread private transformation pass.
class VPOParoptTpvLegacyPass : public PassInfoMixin<VPOParoptTpvLegacyPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // end namespace vpo
} // end namespace llvm


#endif // LLVM_TRANSFORMS_VPO_PAROPT_TPV_H
