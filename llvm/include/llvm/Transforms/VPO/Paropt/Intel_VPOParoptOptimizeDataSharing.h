//===----------------- Intel_VPOParoptOptimizeDataSharing -----------------===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This header file declares optimization pass that transforms OpenMP clauses
/// to minimize shared data accesses across threads.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_PAROPT_OPTIMIZE_DATA_SHARING_H
#define LLVM_TRANSFORMS_VPO_PAROPT_OPTIMIZE_DATA_SHARING_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class VPOParoptOptimizeDataSharingPass
    : public PassInfoMixin<VPOParoptOptimizeDataSharingPass> {
public:
  VPOParoptOptimizeDataSharingPass() = default;

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
  static bool isRequired() { return true; }
};
} // end namespace llvm
#endif  // LLVM_TRANSFORMS_VPO_PAROPT_OPTIMIZE_DATA_SHARING_H
