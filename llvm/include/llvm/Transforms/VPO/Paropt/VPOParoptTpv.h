#if INTEL_COLLAB // -*- C++ -*-
//===--- VPOParoptPrepare.h --- Paropt Threadprivate Support -*- C++ --*---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file provides the interface for the legacy threadprivate
/// transformation.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_PAROPT_TPV_H
#define LLVM_TRANSFORMS_VPO_PAROPT_TPV_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

/// Legacy thread private transformation pass.
class VPOParoptTpvLegacyPass : public PassInfoMixin<VPOParoptTpvLegacyPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // end namespace llvm


#endif // LLVM_TRANSFORMS_VPO_PAROPT_TPV_H
#endif // INTEL_COLLAB
