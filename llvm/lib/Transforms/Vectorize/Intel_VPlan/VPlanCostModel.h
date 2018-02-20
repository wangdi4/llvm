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
///
/// \file
/// This file defines the VPlanCostModel class that is used for all cost
/// estimations performed in the VPlan-based vectorizer. The class provides both
/// the interfaces to calculate different costs (e.g., a single VPInstruction or
/// the whole VPlan) and the dedicated printing methods used exclusively for
/// testing purposes.
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
  const unsigned VF;
  const TargetTransformInfo *TTI;

  static constexpr unsigned UnknownCost = static_cast<unsigned>(-1);

public:
  VPlanCostModel(const vpo::IntelVPlan *Plan, unsigned VF,
                 const TargetTransformInfo *TTI)
      : Plan(Plan), VF(VF), TTI(TTI) {}

  /// Calculate the cost of the given \p VPInst.
  ///
  /// Although the cost is calculated for a single instruction only, it is still
  /// possible for this method to take other instruction into consideration.
  unsigned getCost(const vpo::VPInstruction *VPInst) const;
  /// Calculate the total cost of the instructions for a given \p VPBB.
  unsigned getCost(const vpo::VPBasicBlock *VPBB) const;
  /// Calculate the cost of the whole VPlan.
  unsigned getCost() const;

  /// Print fully detailed report for the costs inside the underlying VPlan.
  ///
  /// To be used for testing purposes only (similar to print method in analysis
  /// passes).
  void print(raw_ostream &OS) const;

private:
  // TODO: These two methods below should probably go away once we start using
  // the traversal used in VPlan dumps.
  /// Helper function to recursively travers the VPlan during detailed cost
  /// printing.
  void printForVPBlockBase(raw_ostream &OS,
                           const vpo::VPBlockBase *VPBlock) const;
  /// Helper function to recursively travers the VPlan and accumulate the cost.
  unsigned getCost(const vpo::VPBlockBase *VPBlock) const;

  // These utilities are private for the class instead of being defined as
  // static functions because they need access to underlying Inst/HIRData in
  // VPInstruction via the friends relation between VPlanCostModel and
  // VPInstruction.
  //
  // Also, they won't be necessary if we had VPType for each VPValue.
  static Type *getMemInstValueType(const vpo::VPInstruction *VPInst);
  static unsigned getMemInstAlignment(const vpo::VPInstruction *VPInst);
  static unsigned getMemInstAddressSpace(const vpo::VPInstruction *VPInst);
};
} // namespace llvm
#endif
