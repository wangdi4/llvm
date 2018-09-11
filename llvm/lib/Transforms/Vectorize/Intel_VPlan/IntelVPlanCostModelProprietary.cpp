//===-- IntelVPlanCostModelProprietary.cpp --------------------------------===//
//
//   Copyright (C) 2018 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements VPlan cost modeling with Intel's IP.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanCostModelProprietary.h"
#include "IntelVPlan.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/TargetTransformInfo.h"

#define DEBUG_TYPE "vplan-cost-model-proprietary"

using namespace llvm::loopopt;

// TODO: Replace this function with a call to divergence analysis when it is
// ready.
static bool isUnitStride(const RegDDRef *Ref, unsigned NestingLevel) {

  if (!Ref->isMemRef())
    return false;

  bool IsNegStride;
  if (!Ref->isUnitStride(NestingLevel, IsNegStride) || IsNegStride)
    return false;

  return true;
}

namespace llvm {

namespace vpo {

// TODO: Ideally this function must not be a member function of
// VPlanCostModelProprietary, however now it accesses
// VPInstruction::getHIRData() which is protected.
bool VPlanCostModelProprietary::isUnitStrideLoadStore(
    const VPInstruction *VPInst) {
  unsigned Opcode = VPInst->getOpcode();
  assert((Opcode == Instruction::Load || Opcode == Instruction::Store) &&
         "Is not load or store instruction.");
  if (!VPInst->HIR.isMaster())
    return false; // CHECKME: Is that correct?

  if (auto Inst = dyn_cast<HLInst>(VPInst->HIR.getUnderlyingNode())) {
    unsigned NestingLevel = Inst->getParentLoop()->getNestingLevel();

    return Opcode == Instruction::Load
               ? isUnitStride(Inst->getOperandDDRef(1), NestingLevel)
               : isUnitStride(Inst->getLvalDDRef(), NestingLevel);
  }
  return false;
}

unsigned
VPlanCostModelProprietary::getLoadStoreCost(const VPInstruction *VPInst) const {
  Type *OpTy = getMemInstValueType(VPInst);

  // FIXME: That should be removed later.
  if (!OpTy)
    return UnknownCost;

  assert(OpTy && "Can't get type of the load/store instruction!");
  unsigned Opcode = VPInst->getOpcode();

  unsigned Alignment = getMemInstAlignment(VPInst);
  unsigned AddrSpace = getMemInstAddressSpace(VPInst);

  bool IsUnit = isUnitStrideLoadStore(VPInst);

  // TODO: On archs w/o native masked operations support next estimation is
  // still not accurate. Masked loads or stores will be emulated with
  // if-then-else statements, thus additional cost of movemask and cmps
  // must be added.
  return IsUnit ? TTI->getMemoryOpCost(Opcode, getVectorizedType(OpTy, VF),
                                       Alignment, AddrSpace)
                : VPlanCostModel::getCost(VPInst);
}

unsigned VPlanCostModelProprietary::getCost(const VPInstruction *VPInst) const {
  unsigned Opcode = VPInst->getOpcode();
  switch (Opcode) {
  case Instruction::Load:
  case Instruction::Store:
    return getLoadStoreCost(VPInst);
  // TODO: So far there's no explicit representation for reduction
  // initializations and finalizations. Need to account overhead for such
  // instructions, until VPlan is ready to have explicit representation for
  // that.
  default:
    return VPlanCostModel::getCost(VPInst);
  }
}

// Right now it calls for VPlanCostModel::getCost(VPBB), but later we may want
// to have more precise cost estimation for VPBB.
unsigned VPlanCostModelProprietary::getCost(const VPBasicBlock *VPBB) const {
  return VPlanCostModel::getCost(VPBB);
}

// Right now it calls for VPlanCostModel::getCost(VPBB), but later we may want
// to have more precise cost estimation for VPBB.
unsigned VPlanCostModelProprietary::getCost(const VPBlockBase *VPBlock) const {
  return VPlanCostModel::getCost(VPBlock);
}

unsigned VPlanCostModelProprietary::getCost() const {
  return VPlanCostModel::getCost();
}

void VPlanCostModelProprietary::print(raw_ostream &OS) {
  OS << "HIR Cost Model for VPlan " << Plan->getName() << " with VF = " << VF
     << ":\n";
  OS << "Total Cost: " << getCost() << '\n';
  LLVM_DEBUG(dbgs() << *Plan;);

  // TODO: match print order with "vector execution order".
  for (const VPBlockBase *Block : depth_first(Plan->getEntry()))
    printForVPBlockBase(OS, Block);
}

} // namespace vpo

} // namespace llvm
