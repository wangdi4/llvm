//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
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

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &OS);
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  ~VPlanCostModelProprietary() {}

private:
  static bool isUnitStrideLoadStore(const VPInstruction *VPinst);

  // FIXME: This is a temporary workaround until proper cost modeling is implemented.
  //
  // To bail out if too many i1 operations are inside the loop as that (most
  // probably) represents complicated CFG and we need to use Basic Block
  // Frequency info to correctly calculate the cost. Until it's done, just
  // report high vector cost for loops with too many i1 instructions.
  mutable unsigned NumberOfBoolComputations = 0;
};

} // namespace vpo

} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCOSTMODELPROPRIETARY_H
