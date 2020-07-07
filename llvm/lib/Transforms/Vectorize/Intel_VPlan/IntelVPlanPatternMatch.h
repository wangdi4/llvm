//===- IntelVPlanPatternMatch.h ---------------------------------*- C++ -*-===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VPLAN_PATTERN_MATCH_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VPLAN_PATTERN_MATCH_H

#include "IntelVPlan.h"
#include "llvm/IR/PatternMatch.h"

namespace llvm {
namespace vpo {

// Matches abs VPInstruction.
template <typename OP_t>
struct m_VPAbs_matcher {
  OP_t X;

  m_VPAbs_matcher(const OP_t &X) : X(X) {}
  bool match(const VPValue *V) {
    if (auto *I = dyn_cast<VPInstruction>(V))
      if (I->getOpcode() == VPInstruction::Abs)
        return X.match(I->getOperand(0));
    return false;
  }
};

template <typename OP_t>
inline decltype(auto) m_VPAbs(OP_t &V) {
  return m_VPAbs_matcher<OP_t>(V);
}
} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VPLAN_PATTERN_MATCH_H
