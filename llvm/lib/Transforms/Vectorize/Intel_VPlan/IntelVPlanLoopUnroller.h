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

  /// Perform unrolling of the VPlan passed in constructor.
  void run();

  /// Given a loop and header PHI node, check if the PHI represents
  /// a partial-sum candidate reduction: the PHI inputs must be
  /// a reduction-init from the preheader and a latch value which is
  /// used by a (single) reduction-final in the unique exit.
  /// If all conditions are met, returns the VPReductionFinal, or
  /// nullptr otherwise.
  /// NB: this returns nullptr if the partial sum optimization is
  /// not enabled.
  static VPReductionFinal *getPartialSumReducFinal(const VPLoop &VPL,
                                                   const VPPHINode &Phi);

private:
  VPlanVector &Plan;
  unsigned UF;
};

} // end namespace vpo
} // end namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANLOOPUNROLLER_H
