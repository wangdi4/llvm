//===- IntelVPlanTransformEarlyExitLoop.h ----------------------*- C++ -*-===//
//
//   Copyright (C) 2023 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===---------------------------------------------------------------------===//
///
/// This transform processes VPlans that have explicitly represented early-exit
/// loops (for example, search loops). It massages the control flow and sets up
/// mechanisms to identify when the loop body is executed and the vector lane
/// that takes the early-exit (if any).
//===---------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_TRANSFORM_EARLYEXIT_LOOP_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_TRANSFORM_EARLYEXIT_LOOP_H

#include "IntelVPlan.h"

namespace llvm {
namespace vpo {

class VPTransformEarlyExitLoop {
public:
  VPTransformEarlyExitLoop(const VPTransformEarlyExitLoop &) = delete;
  VPTransformEarlyExitLoop &
  operator=(const VPTransformEarlyExitLoop &) = delete;

  VPTransformEarlyExitLoop(VPlanVector &Plan) : Plan(Plan) {
    assert(Plan.isEarlyExitLoop() &&
           "VPlan representing early-exit loop is expected here.");
  }

  /// Perform the early-exit loop handling transformation.
  void transform();

private:
  VPlanVector &Plan;
};

} // namespace vpo
} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_TRANSFORM_EARLYEXIT_LOOP_H
