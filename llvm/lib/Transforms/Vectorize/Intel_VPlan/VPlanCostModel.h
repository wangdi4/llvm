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

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLAN_COST_MODEL_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPLAN_COST_MODEL_H

namespace llvm {
class TargetTransformInfo;
class Type;
class raw_ostream;

namespace vpo {
class IntelVPlan;
class VPBasicBlock;
class VPBlockBase;
class VPInstruction;
} // namespace vpo

class VPlanCostModel {
  const vpo::IntelVPlan *Plan;
  unsigned VF;
  const TargetTransformInfo *TTI;

  static constexpr unsigned UnknownCost = static_cast<unsigned>(-1);

public:
  VPlanCostModel(const vpo::IntelVPlan *Plan, unsigned VF,
                 const TargetTransformInfo *TTI)
      : Plan(Plan), VF(VF), TTI(TTI) {}

  unsigned getCost(const vpo::VPInstruction *VPInst);
  unsigned getCost(const vpo::VPBasicBlock *VPBB);
  void print(raw_ostream &OS);

private:
  void printForVPBlockBase(raw_ostream &OS, const vpo::VPBlockBase *VPBlock);
};
} // namespace llvm
#endif
