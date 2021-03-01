//===-- IntelVPlanLoopUnroller.h --------------------------------*- C++ -*-===//
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
/// This file defines VPlanLoopUnroller class. VPlanLoopUnroller implements
/// loop unrolling VPlan-to-VPlan transformation.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANLOOPUNROLLER_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANLOOPUNROLLER_H

#include "IntelVPlan.h"
#include "llvm/ADT/DenseMap.h"

namespace llvm {
namespace vpo {

class VPlanLoopUnroller {

public:
  VPlanLoopUnroller(VPlanVector &Plan, unsigned UF) : Plan(Plan), UF(UF) {}

  // Store here unrolled parts numbers for all cloned VPInstructions.
  using VPInstUnrollPartTy = DenseMap<const VPInstruction *, unsigned>;

  /// Perform unrolling of the VPlan passed in constructor, filling in
  /// the optional (\p VPInstUnrollPart) map of newly generated instructions
  /// to the unrolling parts.
  void run(VPInstUnrollPartTy *VPInstUnrollPart = nullptr);

private:
  VPlanVector &Plan;
  unsigned UF;
};

} // end namespace vpo
} // end namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANLOOPUNROLLER_H
