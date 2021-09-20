//===--------------- Intel_VPOParoptTargetInline.h --------------*- C++ -*-===//
//
//   Copyright (C) 2021 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file declares a module pass which adds alwaysinline attribute to all
/// functions called from OpenMP target regions.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_PAROPT_TARGET_INLINE_H
#define LLVM_TRANSFORMS_VPO_PAROPT_TARGET_INLINE_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class VPOParoptTargetInlinePass
    : public PassInfoMixin<VPOParoptTargetInlinePass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_VPO_PAROPT_TARGET_INLINE_H
