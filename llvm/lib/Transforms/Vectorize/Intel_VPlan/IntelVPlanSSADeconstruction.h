//===-- IntelVPlanSSADeconstruction.h ---------------------------*- C++ -*-===//
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
/// This file defines VPlanSSADeconstruction class which implements a pseudo
/// SSA deconstruction algorithm via a VPlan-to-VPlan transformation.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANSSADECONSTRUCTION_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANSSADECONSTRUCTION_H

#include "IntelVPlan.h"

namespace llvm {
namespace vpo {

extern bool PrintAfterSSADeconstruction;

class VPlanSSADeconstruction {

public:
  VPlanSSADeconstruction(VPlanVector &Plan) : Plan(Plan) {}

  /// A VPlan-to-VPlan transformation that breaks down PHIs to copy instructions
  /// inserted in appropriate incoming blocks. This transformation is done as
  /// part of CG preparation phase. The implementation is equivalent to a
  /// "pseudo" SSA deconstruction, only difference being the PHI node still
  /// remains in VPlan CFG but is not explicitly widened during CG.
  /// NOTE: Current technique would not work for live-out header PHI nodes since
  /// previous iteration value is needed. However such PHIs cannot occur in
  /// VPlan since the PHIs are introduced explicitly by us during decomposition
  /// of HIR nodes inside the target loop being vectorized.
  void run();

private:
  VPlanVector &Plan;
};

} // end namespace vpo
} // end namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANSSADECONSTRUCTION_H
