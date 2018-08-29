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

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCOSTMODELPROPRIETARY_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCOSTMODELPROPRIETARY_H

#include "IntelVPlanCostModel.h"
#include "IntelVPlanVLSAnalysis.h"

namespace llvm {

namespace vpo {

class VPlanCostModelProprietary : public VPlanCostModel {
public:
  explicit VPlanCostModelProprietary(const VPlan *Plan, unsigned VF,
                                     const TargetTransformInfo *TTI,
                                     const DataLayout *DL,
                                     VPlanVLSAnalysis *VLSA)
      : VPlanCostModel(Plan, VF, TTI, DL, VLSA) {
    VLSA->getOVLSMemrefs(Plan, VF);
  }

  virtual unsigned getCost(const VPInstruction *VPInst) const final;
  virtual unsigned getCost(const VPBasicBlock *VPBB) const final;
  virtual unsigned getCost() const final;
  virtual unsigned getLoadStoreCost(const VPInstruction *VPInst) const {
    return getLoadStoreCost(VPInst, false /* Don't use VLS cost by default */);
  }
  unsigned getLoadStoreCost(const VPInstruction *VPInst,
                            const bool UseVLSCost) const;

  void print(raw_ostream &OS);

  ~VPlanCostModelProprietary() {}

private:
  virtual unsigned getCost(const VPBlockBase *VPBlock) const final;
  static bool isUnitStrideLoadStore(const VPInstruction *VPinst);
};

} // namespace vpo

} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCOSTMODELPROPRIETARY_H
