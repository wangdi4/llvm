//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2018 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLAN_COST_MODEL_PROPRIETARY_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLAN_COST_MODEL_PROPRIETARY_H

#include "VPlanCostModel.h"

namespace llvm {

namespace vpo {

class VPlanCostModelProprietary : public VPlanCostModel {
public:
  explicit VPlanCostModelProprietary(const IntelVPlan *Plan, unsigned VF,
                                     const TargetTransformInfo *TTI)
      : VPlanCostModel(Plan, VF, TTI) {}

  virtual unsigned getCost(const VPInstruction *VPInst) const final;
  virtual unsigned getCost(const VPBasicBlock *VPBB) const final;
  virtual unsigned getCost() const final;

  void print(raw_ostream &OS);

  ~VPlanCostModelProprietary() {}

private:
  virtual unsigned getCost(const VPBlockBase *VPBlock) const final;
  unsigned getLoadStoreCost(const VPInstruction *VPInst) const;
  static bool isUnitStrideLoadStore(const VPInstruction *VPinst);
};

} // namespace vpo

} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLAN_COST_MODEL_PROPRIETARY_H
