//===------------ Intel_VPOParoptSharedPrivatization.h ----------*- C++ -*-===//
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
/// Declaration of the paropt optimization pass that privatizes shared items in
/// work regions where it is safe to do so.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_PAROPT_SHARED_PRIVATIZATION_H
#define LLVM_TRANSFORMS_VPO_PAROPT_SHARED_PRIVATIZATION_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class VPOParoptSharedPrivatizationPass
    : public PassInfoMixin<VPOParoptSharedPrivatizationPass> {
public:
  /// VPOParoptSharedPrivatizationPass constructor. Supported values for \p Mode
  /// so far include OmpOffload only.
  explicit VPOParoptSharedPrivatizationPass(unsigned Mode = 0u) : Mode(Mode) {}
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

private:
  unsigned Mode;
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_VPO_PAROPT_SHARED_PRIVATIZATION_H
