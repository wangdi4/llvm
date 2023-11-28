#if INTEL_COLLAB
//===-- VPOParoptLoopCollapse.cpp - Paropt Loop Collapse Pass for OpenMP --===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Authors:
// Vyacheslav Zakharin (vyacheslav.p.zakharin@intel.com)
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This header file declares the loop nests collapsing pass for OpenMP loops.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_PAROPT_LOOP_COLLAPSE_H
#define LLVM_TRANSFORMS_VPO_PAROPT_LOOP_COLLAPSE_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"

namespace llvm {
class VPOParoptLoopCollapsePass
    : public PassInfoMixin<VPOParoptLoopCollapsePass> {
public:
  VPOParoptLoopCollapsePass() = default;

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
  static bool isRequired() { return true; }
};
} // end namespace llvm
#endif  // LLVM_TRANSFORMS_VPO_PAROPT_LOOP_COLLAPSE_H
#endif // INTEL_COLLAB
