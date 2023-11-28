//===-- VPOParoptLoopFuseCollapse.h - OpenMP Loop FuseAndCollapse Pass --===//
//
//   Copyright (C) 2023 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This header file declares the loop FuseAndCollapse pass for OpenMP loops.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_PAROPT_LOOP_FUSE_COLLAPSE_H
#define LLVM_TRANSFORMS_VPO_PAROPT_LOOP_FUSE_COLLAPSE_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"

namespace llvm {
class VPOParoptLoopFuseCollapsePass
    : public PassInfoMixin<VPOParoptLoopFuseCollapsePass> {
public:
  VPOParoptLoopFuseCollapsePass() = default;

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
  static bool isRequired() { return true; }
};
} // end namespace llvm
#endif // LLVM_TRANSFORMS_VPO_PAROPT_LOOP_FUSE_COLLAPSE_H
