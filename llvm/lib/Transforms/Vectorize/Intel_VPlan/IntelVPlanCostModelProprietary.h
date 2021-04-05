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
  explicit VPlanCostModelProprietary(const VPlanVector *Plan, unsigned VF,
                                     const TargetTransformInfo *TTI,
                                     const TargetLibraryInfo *TLI,
                                     const DataLayout *DL,
                                     VPlanVLSAnalysis *VLSA = nullptr)
    : VPlanCostModel(Plan, VF, TTI, TLI, DL, VLSA),
      HeuristicsPipeline(this) {
    if (VLSA)
      VLSA->getOVLSMemrefs(Plan, VF);
  }

  using VPlanCostModel::getCost;
  unsigned getLoadStoreCost(
    const VPInstruction *VPInst, Align Alignment, unsigned VF) final {
    return getLoadStoreCost(VPInst, Alignment, VF,
                            false /* Don't use VLS cost by default */);
  }

  ~VPlanCostModelProprietary() {}

  // Temporal virtual methods to invoke apply facilities on HeuristicsPipeline.
  void applyHeuristicsPipeline(
    unsigned TTICost, unsigned &Cost,
    const VPlanVector *Plan, raw_ostream *OS = nullptr) const final {
    HeuristicsPipeline.apply(TTICost, Cost, Plan, OS);
  }
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Temporal virtual methods to invoke dump facilities on HeuristicsPipeline.
  void dumpHeuristicsPipeline(raw_ostream &OS,
                              const VPlanVector *Plan) const final {
    HeuristicsPipeline.dump(OS, Plan);
  }
  void dumpHeuristicsPipeline(raw_ostream &OS,
                              const VPBasicBlock *VPBB) const final {
    HeuristicsPipeline.dump(OS, VPBB);
  }
  void dumpHeuristicsPipeline(raw_ostream &OS,
                              const VPInstruction *VPInst) const final {
    HeuristicsPipeline.dump(OS, VPInst);
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
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

  // Heuristics list type specific to proprietary cost model.
  HeuristicsList<
    const VPlanVector,
    VPlanCostModelHeuristics::HeuristicSearchLoop,
    VPlanCostModelHeuristics::HeuristicSLP,
    VPlanCostModelHeuristics::HeuristicGatherScatter,
    VPlanCostModelHeuristics::HeuristicSpillFill,
    VPlanCostModelHeuristics::HeuristicPsadbw> HeuristicsPipeline;
};

} // namespace vpo

} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCOSTMODELPROPRIETARY_H
