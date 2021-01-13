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
                                     VPlanVLSAnalysis *VLSA)
    : VPlanCostModel(Plan, VF, TTI, TLI, DL, VLSA) {
    VLSA->getOVLSMemrefs(Plan, VF);
  }

  unsigned getCost() final;
  unsigned getLoadStoreCost(
    const VPInstruction *VPInst, Align Alignment, unsigned VF) final {
    return getLoadStoreCost(VPInst, Alignment, VF,
                            false /* Don't use VLS cost by default */);
  }
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void print(raw_ostream &OS, const std::string &Header);
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  ~VPlanCostModelProprietary() {}

private:
  unsigned getCost(const VPInstruction *VPInst) final;
  unsigned getCost(const VPBasicBlock *VPBB) final;
  unsigned getLoadStoreCost(const VPInstruction *VPInst,
                            Align Alignment, unsigned VF,
                            const bool UseVLSCost);
  unsigned getLoadStoreCost(const VPInstruction *VPInst, unsigned VF,
                            const bool UseVLSCost) {
    unsigned Alignment = VPlanCostModel::getMemInstAlignment(VPInst);
    return getLoadStoreCost(VPInst, Align(Alignment), VF, UseVLSCost);
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printForVPInstruction(raw_ostream &OS, const VPInstruction *VPInst);
  void printForVPBasicBlock(raw_ostream &OS, const VPBasicBlock *VPBlock);
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  // Implements basic register pressure calculation pass.
  // Calculates the pressure of Vector Instructions only if VectorInsts is
  // true. Calculates scalar instructions pressure otherwise.
  // LiveValues map contains the liveness of the given instruction multiplied
  // by its legalization factor.  The map contains LiveOut values for the block
  // on input of getSpillFillCost(Block, LiveValues) and the map is updated
  // by getSpillFillCost() and contains LiveIn values after the call.
  unsigned getSpillFillCost(
    const VPBasicBlock *VPBlock,
    DenseMap<const VPInstruction*, int /* legalization factor */> &LiveValues,
    bool VectorInsts);
  unsigned getSpillFillCost(void);

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

  // PsadbwPatternInsts holds all instructions that are part of any PSADBW
  // pattern.  Used by dumping facilities only.
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // PsadbwPatternInsts holds all instructions that are part of any PSADBW
  // pattern.  Used by dumping facilities only.
  SmallPtrSet<const VPInstruction*, 32> PsadbwPatternInsts;
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  // Checks for PSADWB pattern starting SelInst and updates
  // CurrPsadbwPatternInsts argument with instructions forming PSADWB pattern
  // based on SelInst.
  //
  // Returns true if pattern is found, false otherwise.
  bool checkPsadwbPattern(
    const VPInstruction *SelInst,
    SmallPtrSetImpl<const VPInstruction*> &CurrPsadbwPatternInsts);

  // Return the sum of costs of all PSADWB patterns in VPlan.
  // Also populates PsadbwPatternInsts with pattern instructions.
  unsigned getPsadwbPatternCost();

  // getGatherScatterCost() interfaces calculate the sum of TTI costs of
  // load and store instructions that are not unit load or store (i.e.
  // most likely are implemented with gather or scatter HW instructions
  // or just serialized).
  //
  // Note that g/s cost is collected with a separate pass though VPlan, not
  // inside getCost(VPInstruction) routine in a dedicated accumulator, because
  // getCost(VPInstruction) can be called multiple times for the same
  // VPInstruction from various heuristics and we don't want getCost() to have
  // a side effect of updating acc.
  //
  // As a cost of such approach we need to keep details of walk through VPlan
  // in sync with walk though VPlan in main getCost() routines.  Such, the same
  // weights should be applied on blocks when block frequency info is deployed.
  unsigned getGatherScatterCost(const VPInstruction *VPInst);
  unsigned getGatherScatterCost(const VPBasicBlock *VPBlock);
  unsigned getGatherScatterCost();
};

} // namespace vpo

} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANCOSTMODELPROPRIETARY_H
