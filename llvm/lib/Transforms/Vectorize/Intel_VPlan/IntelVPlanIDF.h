//===-- IntelVPlanIDF -----------------------------------------------------===//
//
//   Copyright (C) 2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Specialization of IDF classes for VPlan CFG hierarchy.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANIDF_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANIDF_H

#include "IntelVPlan.h"

#include "llvm/Support/GenericIteratedDominanceFrontier.h"

namespace llvm {
namespace vpo {
class VPlanForwardIDFCalculator final
    : public IDFCalculatorBase<VPBlockBase, false> {
public:
  using IDFCalculatorBase =
      typename llvm::IDFCalculatorBase<VPBlockBase, false>;
  VPlanForwardIDFCalculator(VPDominatorTree &DT) : IDFCalculatorBase(DT) {}
};
} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANIDF_H
