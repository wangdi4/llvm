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
  explicit VPlanCostModelProprietary(const VPlanVector *Plan, unsigned VF,
                                     const TargetTransformInfo *TTI,
                                     const TargetLibraryInfo *TLI,
                                     const DataLayout *DL,
                                     VPlanVLSAnalysis *VLSA = nullptr)
    : VPlanCostModel(Plan, VF, TTI, TLI, DL, VLSA),
      HeuristicsPipelinePlan(this), HeuristicsPipelineInst(this) {}

  using VPlanCostModel::getCost;

  ~VPlanCostModelProprietary() {}

  // Temporal virtual method to invoke Heuristics initialization.
  void initHeuristicsForVPlan() final {
    HeuristicsPipelinePlan.initForVPlan();
  }

  // Temporal virtual methods to invoke apply facilities on HeuristicsPipeline.
  void applyHeuristicsPipeline(
    unsigned TTICost, unsigned &Cost,
    const VPlanVector *Plan, raw_ostream *OS = nullptr) const final {
    HeuristicsPipelinePlan.apply(TTICost, Cost, Plan, OS);
  }

  void applyHeuristicsPipeline(
    unsigned TTICost, unsigned &Cost,
    const VPInstruction *VPInst, raw_ostream *OS = nullptr) const final {
    HeuristicsPipelineInst.apply(TTICost, Cost, VPInst, OS);
  }
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Temporal virtual methods to invoke dump facilities on HeuristicsPipeline.
  void dumpHeuristicsPipeline(raw_ostream &OS,
                              const VPlanVector *Plan) const final {
    HeuristicsPipelinePlan.dump(OS, Plan);
    HeuristicsPipelineInst.dump(OS, Plan);
  }
  void dumpHeuristicsPipeline(raw_ostream &OS,
                              const VPBasicBlock *VPBB) const final {
    HeuristicsPipelinePlan.dump(OS, VPBB);
    HeuristicsPipelineInst.dump(OS, VPBB);
  }
  void dumpHeuristicsPipeline(raw_ostream &OS,
                              const VPInstruction *VPInst) const final {
    HeuristicsPipelinePlan.dump(OS, VPInst);
    HeuristicsPipelineInst.dump(OS, VPInst);
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
private:
  unsigned getCost(const VPInstruction *VPInst) final;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  std::string getHeaderPrefix() const final {
    // Proprietary Cost Model prepends the Header in dumps with "HIR " string
    // to ease distinguishing HIR CM dumps VS Base CM dumps.  Please see
    // VPlanCostModel::print() for details.
    return "HIR ";
  };
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  // Heuristics list type specific to proprietary cost model.
  HeuristicsList<
    const VPlanVector,
    VPlanCostModelHeuristics::HeuristicSearchLoop,
    VPlanCostModelHeuristics::HeuristicSLP,
    VPlanCostModelHeuristics::HeuristicGatherScatter,
    VPlanCostModelHeuristics::HeuristicSpillFill,
    VPlanCostModelHeuristics::HeuristicPsadbw> HeuristicsPipelinePlan;

  HeuristicsList<
    const VPInstruction,
    VPlanCostModelHeuristics::HeuristicOVLSMember,
    VPlanCostModelHeuristics::HeuristicSVMLIDivIRem> HeuristicsPipelineInst;
};

} // namespace vpo

} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCOSTMODELPROPRIETARY_H
