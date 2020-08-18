//===-- IntelVPlanLoopCFU.h -------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements inner loop control flow uniformity transformation.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_LOOPCFU_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_LOOPCFU_H

#include "IntelVPlan.h"

namespace llvm {
namespace vpo {

class VPlanLoopCFU {
  VPlan &Plan;
  void run(VPLoop *VPL);

  /// Rematerialize \p LiveOut instruction of a loop \p VPL in terms of other
  /// live-outs in the \p VPL's exit, if possible.
  ///
  /// \Returns the newly created rematerialized instruction or nullptr.
  ///
  /// Example:
  ///
  ///   latch:
  ///     %iv.next = <def>
  ///     %cond = icmp eq %iv.next, 42
  ///
  ///   exit:
  ///     %iv.next.lcssa = phi [ %iv.next ]
  ///     %cond.lcssa = phi [ %cond ]
  ///
  /// We will replace %cond.lcssa with a
  ///
  ///    %rematerialized = icmp eq %iv.next.lcssa, 42
  ///
  /// That is needed because all live-outs from divergent loop require creation
  /// of the recurrence phi to record the last unmasked execution for the lane.
  /// By this rematerialization we avoid the need to create one.
  VPInstruction *tryRematerializeLiveOut(VPLoop *VPL, VPInstruction *LiveOut);

  /// Rematerialize all rematerializable live-outs of a loop \p VPL.
  ///
  /// See tryRematerializeLiveOut for details.
  void rematerializeLiveOuts(VPLoop *VPL);

public:
  VPlanLoopCFU(VPlan &Plan) : Plan(Plan) {}
  void run();
};
} // namespace vpo
} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_LOOPCFU_H
