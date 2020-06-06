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
                                     const TargetLibraryInfo *TLI,
                                     const DataLayout *DL,
                                     VPlanVLSAnalysis *VLSA)
    : VPlanCostModel(Plan, VF, TTI, TLI, DL, VLSA) {
    VLSA->getOVLSMemrefs(Plan, VF);
  }

  virtual unsigned getCost(const VPInstruction *VPInst) final;
  virtual unsigned getCost(const VPBasicBlock *VPBB) final;
  virtual unsigned getCost() final;
  virtual unsigned getLoadStoreCost(const VPInstruction *VPInst) {
    return getLoadStoreCost(VPInst, false /* Don't use VLS cost by default */);
  }
  unsigned getLoadStoreCost(const VPInstruction *VPInst,
                            const bool UseVLSCost);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &OS, const std::string &Header);
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  ~VPlanCostModelProprietary() {}

private:
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printForVPInstruction(
    raw_ostream &OS, const VPInstruction *VPInst);
  void printForVPBasicBlock(
    raw_ostream &OS, const VPBasicBlock *VPBlock);
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  // Consolidates proprietary code that gets the cost of one operand or two
  // operands arithmetics instructions.
  virtual unsigned getArithmeticInstructionCost(const unsigned Opcode,
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

  // FIXME: This is a temporary workaround until proper cost modeling is
  // implemented.
  //
  // To bail out if too many i1 operations are inside the loop as that (most
  // probably) represents complicated CFG and we need to use Basic Block
  // Frequency info to correctly calculate the cost. Until it's done, just
  // report high vector cost for loops with too many i1 instructions.
  unsigned NumberOfBoolComputations = 0;

  /// \Returns True iff \p VPInst is Unit Strided load or store.
  virtual bool isUnitStrideLoadStore(const VPInstruction *VPInst) const final;
};

} // namespace vpo

} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCOSTMODELPROPRIETARY_H
