//===-- IntelVPlanLoopExitCannonicalization.h -------------------*- C++ -*-===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines utilities to perform loops' exits canonicalization that is
/// needed for a further loop control flow unification (LoopCFU) transformation.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VPLAN_LOOP_EXIT_CANONICALIZATION_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VPLAN_LOOP_EXIT_CANONICALIZATION_H

#include "IntelVPlan.h"

namespace llvm {
namespace vpo {
void singleExitWhileLoopCanonicalization(VPLoop *VPL);
void mergeLoopExits(VPLoop *VPL, bool NeedsOuterLpEarlyExitHandling);
} // namespace vpo
} // namespace llvm

#endif
