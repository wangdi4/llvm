#if INTEL_COLLAB // -*- C++ -*-
//===-------- CFGSimplify.h - Simplifies CFG after Paropt -*- C++ -*-------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// This file declares the VPOCFGSimplifyPass pass.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_UTILS_CFGSIMPLIFY_H
#define LLVM_TRANSFORMS_VPO_UTILS_CFGSIMPLIFY_H

#include "llvm/IR/PassManager.h"

namespace llvm {
class VPOCFGSimplifyPass : public PassInfoMixin<VPOCFGSimplifyPass> {
public:
  VPOCFGSimplifyPass() = default;

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
  static bool isRequired() { return true; }
};
} // end namespace llvm
#endif // LLVM_TRANSFORMS_VPO_UTILS_CFGSIMPLIFY_H
#endif // INTEL_COLLAB
