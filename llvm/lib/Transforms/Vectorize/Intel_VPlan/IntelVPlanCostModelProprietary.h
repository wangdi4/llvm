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
  using VPlanCostModel::getLoadStoreCost;
public:
  explicit VPlanCostModelProprietary(const VPlan *Plan, unsigned VF,
                                     const TargetTransformInfo *TTI,
                                     const TargetLibraryInfo *TLI,
                                     const DataLayout *DL,
                                     VPlanVLSAnalysis *VLSA = nullptr)
    : VPlanCostModel(Plan, VF, TTI, TLI, DL, VLSA) {
    if (VLSA)
      VLSA->getOVLSMemrefs(Plan, VF);

    // Clear out HeuristicsPipeline from Base Cost Model Heuristic and fill it
    // up with Proprietary Cost Model heuristics set in the order they should
    // be applied.
    HeuristicsPipeline.clear();
    HeuristicsPipeline.push_back(
      std::make_unique<VPlanCostModelHeuristics::HeuristicSearchLoop>(this));
    if (VF != 1) {
      HeuristicsPipeline.push_back(
        std::make_unique<VPlanCostModelHeuristics::HeuristicSLP>(this));
      HeuristicsPipeline.push_back(
        std::make_unique<VPlanCostModelHeuristics::HeuristicGatherScatter>(
          this));
    }
    HeuristicsPipeline.push_back(
      std::make_unique<VPlanCostModelHeuristics::HeuristicSpillFill>(this));
    if (VF == 1)
      // Don't apply bonus on VF != 1 plan as we exactly want to keep scalar
      // VPlan in case psadbw pattern is found.
      HeuristicsPipeline.push_back(
        std::make_unique<VPlanCostModelHeuristics::HeuristicPsadbw>(this));
  }

  using VPlanCostModel::getCost;
  unsigned getLoadStoreCost(
    const VPInstruction *VPInst, Align Alignment, unsigned VF) final {
    return getLoadStoreCost(VPInst, Alignment, VF,
                            false /* Don't use VLS cost by default */);
  }

  ~VPlanCostModelProprietary() {}

private:
  unsigned getCost(const VPInstruction *VPInst) final;
  unsigned getLoadStoreCost(const VPInstruction *VPInst,
                            Align Alignment, unsigned VF,
                            const bool UseVLSCost);
  unsigned getLoadStoreCost(const VPInstruction *VPInst, unsigned VF,
                            const bool UseVLSCost) {
    unsigned Alignment = VPlanCostModel::getMemInstAlignment(VPInst);
    return getLoadStoreCost(VPInst, Align(Alignment), VF, UseVLSCost);
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  std::string getAttrString(const VPInstruction *VPInst) const final;
  std::string getHeaderPrefix() const final {
    // Proprietary Cost Model prepends the Header in dumps with "HIR " string
    // to ease distinguishing HIR CM dumps VS Base CM dumps.  Please see
    // VPlanCostModel::print() for details.
    return "HIR ";
  };
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  // Consolidates proprietary code that gets the cost of one operand or two
  // operands arithmetics instructions.
  unsigned getArithmeticInstructionCost(const unsigned Opcode,
                                        const VPValue *Op1,
                                        const VPValue *Op2,
                                        const Type *ScalarTy,
                                        const unsigned VF) final;

  // ProcessedOVLSGroups holds the groups which Cost has already been taken into
  // account while traversing through VPlan during getCost().  This way we avoid
  // taking the same group price multiple times.
  // If Cost of OVLS group is better in terms of performance comparing to TTI
  // costs of intruction OVLS group would replace, then ProcessedOVLSGroups map
  // holds 'true' for this group.  Otherwise 'false' is stored in the map.
  using OVLSGroupMap = DenseMap<const OVLSGroup *, bool>;
  OVLSGroupMap ProcessedOVLSGroups;
};

} // namespace vpo

} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCOSTMODELPROPRIETARY_H
