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
#include "IntelVPlanIdioms.h"
#include "IntelVPlanVLSAnalysis.h"
#include "VPlanHIR/IntelVPlanVLSAnalysisHIR.h"
#include "VPlanHIR/IntelVPlanVLSClientHIR.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/TargetTransformInfo.h"

#define DEBUG_TYPE "vplan-cost-model-proprietary"

static cl::opt<bool> UseOVLSCM("vplan-cm-use-ovlscm", cl::init(false),
                               cl::desc("Consider cost returned by OVLSCostModel "
                                        "for optimized gathers and scatters."));

using namespace llvm::loopopt;

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
    // FIXME: It's not correct to getParentLoop() for outerloop
    // vectorization.
    assert(Inst->getParentLoop()->isInnermost() &&
           "Outerloop vectorization is not supported.");
    unsigned NestingLevel = Inst->getParentLoop()->getNestingLevel();

    return Opcode == Instruction::Load
               ? VPlanVLSAnalysisHIR::isUnitStride(Inst->getOperandDDRef(1),
                                                   NestingLevel)
               : VPlanVLSAnalysisHIR::isUnitStride(Inst->getLvalDDRef(),
                                                   NestingLevel);
  }
  return false;
}

unsigned
VPlanCostModelProprietary::getLoadStoreCost(const VPInstruction *VPInst,
                                            const bool UseVLSCost) const {
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
  unsigned Cost =
      IsUnit ? TTI->getMemoryOpCost(Opcode, getVectorizedType(OpTy, VF),
                                    Alignment, AddrSpace)
             : VPlanCostModel::getLoadStoreCost(VPInst);

  if (UseOVLSCM && VLSCM && UseVLSCost && VF > 1)
    if (OVLSGroup *Group = VLSA->getGroupsFor(Plan, VPInst))
      if (Group->size() > 1) {
        unsigned VLSCost = OptVLSInterface::getGroupCost(*Group, *VLSCM);
        if (VLSCost < Cost) {
          LLVM_DEBUG(dbgs() << "Reduced cost for "; VPInst->print(dbgs());
                     dbgs() << " from " << Cost << " to " << VLSCost << '\n');
          return VLSCost;
        }
        else
          LLVM_DEBUG(dbgs() << "Cost for "; VPInst->print(dbgs());
                     dbgs() << " was not reduced from " << Cost << " to " << VLSCost
                            << '\n');
      }
  return Cost;
}

unsigned VPlanCostModelProprietary::getCost(const VPInstruction *VPInst) const {
  unsigned Opcode = VPInst->getOpcode();
  switch (Opcode) {
  case Instruction::Load:
  case Instruction::Store:
    return getLoadStoreCost(VPInst, true);
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
  unsigned Cost = VPlanCostModel::getCost();

  switch (VPlanIdioms::isSearchLoop(Plan, VF, true)) {
  case VPlanIdioms::Unsafe:
    return UnknownCost;
  case VPlanIdioms::SearchLoopStrEq:
    // Without proper type information, cost model cannot properly compute the
    // cost, thus hard code VF.
    if (VF != 32)
      // Return some huge value, so that VectorCost still could be computed.
      return 10000000;
    break;
  default:
    // FIXME: Keep VF = 32 as unsupported right now due to huge perf
    // regressions.
    if (VF == 32)
      return UnknownCost;
  }

  return Cost;
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
