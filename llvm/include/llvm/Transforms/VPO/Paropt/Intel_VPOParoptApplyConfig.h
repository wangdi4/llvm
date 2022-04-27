//===----------------- Intel_VPOParoptApplyConfig -----------------===//
//
//   Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This header files declares a pass that modifies/adds clauses requested
/// byt the user via VPOParoptConfig analysis.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_PAROPT_APPLY_CONFIG_H
#define LLVM_TRANSFORMS_VPO_PAROPT_APPLY_CONFIG_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class VPOParoptApplyConfigPass
    : public PassInfoMixin<VPOParoptApplyConfigPass> {
public:
  VPOParoptApplyConfigPass() = default;

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
  static bool isRequired() { return true; }
};
} // end namespace llvm
#endif  // LLVM_TRANSFORMS_VPO_PAROPT_APPLY_CONFIG_H
