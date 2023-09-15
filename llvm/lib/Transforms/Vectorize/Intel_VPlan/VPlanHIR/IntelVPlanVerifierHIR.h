//===-- IntelVPlanVerifierHIR.h ---------------------------------*- C++ -*-===//
//
//   Copyright (C) 2017 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file declares VPlanVerifierHIR class which specializes VPlan
/// verification algorithm with HIR-specific checks.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANVERIFIER_HIR_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANVERIFIER_HIR_H

#include "../IntelVPlanVerifier.h"

namespace llvm {
namespace loopopt {
class HLLoop;
} // namespace loopopt
namespace vpo {

/// Specialization of VPlanVerifier for HIR. It uses HIR specific information
/// such as HLLoop.
class VPlanVerifierHIR : public VPlanVerifier {

private:
  // Outermost HIR loop to be vectorized.
  const loopopt::HLLoop *TheLoop;

  unsigned countLoopsInUnderlyingIR() const override;

public:
  VPlanVerifierHIR(const loopopt::HLLoop *HLLp);
};

} // namespace vpo
} // namespace llvm

#endif //LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLANHIR_INTELVPLANVERIFIER_HIR_H
