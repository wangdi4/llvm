#if INTEL_COLLAB // -*- C++ -*-
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2022 Intel Corporation
//
//===-- VPOParoptLoopTransform.h - OpenMP Loop Transform Pass --===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This header file declares the loop transform pass for OpenMP loops.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_PAROPT_LOOP_TRANSFORM_H
#define LLVM_TRANSFORMS_VPO_PAROPT_LOOP_TRANSFORM_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"

namespace llvm {
class VPOParoptLoopTransformPass
    : public PassInfoMixin<VPOParoptLoopTransformPass> {
public:
  VPOParoptLoopTransformPass() = default;

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
  static bool isRequired() { return true; }
};
} // end namespace llvm
#endif // LLVM_TRANSFORMS_VPO_PAROPT_LOOP_TRANSFORM_H
#endif // INTEL_COLLAB
