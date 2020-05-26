//===-- IntelVPlanCostModelProprietary.cpp --------------------------------===//
//
//   Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
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
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "vplan-cost-model-proprietary"

static cl::opt<bool> UseOVLSCM("vplan-cm-use-ovlscm", cl::init(true),
                               cl::desc("Consider cost returned by OVLSCostModel "
                                        "for optimized gathers and scatters."));
static cl::opt<unsigned> BoolInstsBailOut(
    "vplan-cost-model-i1-bail-out-limit", cl::init(45), cl::Hidden,
    cl::desc("Don't vectorize if number of boolean computations in the VPlan "
             "is higher than the threshold."));

using namespace llvm::loopopt;

namespace llvm {

namespace vpo {

bool VPlanCostModelProprietary::isUnitStrideLoadStore(
  const VPInstruction *VPInst) const {
  unsigned Opcode = VPInst->getOpcode();
  assert((Opcode == Instruction::Load || Opcode == Instruction::Store) &&
         "Is not load or store instruction.");

  if (VPlanCostModel::isUnitStrideLoadStore(VPInst))
    return true;

  if (!VPInst->HIR.isMaster())
    return false;

  if (auto Inst = dyn_cast<HLInst>(VPInst->HIR.getUnderlyingNode())) {
    // FIXME: It's not correct to getParentLoop() for outerloop
    // vectorization.
    if (!Inst->getParentLoop()) {
      return false;
    }
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

unsigned VPlanCostModelProprietary::getLoadStoreCost(
  const VPInstruction *VPInst, const bool UseVLSCost) {
  unsigned Cost = VPlanCostModel::getLoadStoreCost(VPInst);

  if (!UseOVLSCM || !VLSCM || !UseVLSCost || VF == 1)
    return Cost;

  OVLSGroup *Group = VLSA->getGroupsFor(Plan, VPInst);
  if (!Group || Group->size() <= 1)
    return Cost;

  if (ProcessedOVLSGroups.count(Group) != 0) {
    // Group cost has already been assigned.
    LLVM_DEBUG(dbgs() << "Group cost for "; VPInst->print(dbgs());
               dbgs() << " has already been taken into account.\n");
    return ProcessedOVLSGroups[Group] ? 0 : Cost;
  }

  unsigned VLSGroupCost = OptVLSInterface::getGroupCost(*Group, *VLSCM);
  unsigned TTIGroupCost = 0;
  for (OVLSMemref *OvlsMemref : Group->getMemrefVec())
    TTIGroupCost += VPlanCostModel::getLoadStoreCost(
      cast<VPVLSClientMemref>(OvlsMemref)->getInstruction());

  if (VLSGroupCost >= TTIGroupCost) {
    LLVM_DEBUG(dbgs() << "Cost for "; VPInst->print(dbgs());
               dbgs() << " was not reduced from " << Cost <<
               " (TTI group cost " << TTIGroupCost <<
               ") to group cost " << VLSGroupCost << '\n');
    ProcessedOVLSGroups[Group] = false;
    return Cost;
  }

  LLVM_DEBUG(dbgs() << "Reduced cost for "; VPInst->print(dbgs());
             dbgs() << " from " << Cost << " (TTI group cost " <<
             TTIGroupCost << " to group cost " << VLSGroupCost << '\n');
  ProcessedOVLSGroups[Group] = true;
  return VLSGroupCost;
}

unsigned VPlanCostModelProprietary::getCost(const VPInstruction *VPInst) {
  if (VPInst->getType()->isIntegerTy(1))
    ++NumberOfBoolComputations;

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
unsigned VPlanCostModelProprietary::getCost(const VPBasicBlock *VPBB) {
  return VPlanCostModel::getCost(VPBB);
}

unsigned VPlanCostModelProprietary::getCost() {
  NumberOfBoolComputations = 0;
  ProcessedOVLSGroups.clear();
  unsigned Cost = VPlanCostModel::getCost();

  // Array ref which needs to be aligned via loop peeling, if any.
  RegDDRef *PeelArrayRef = nullptr;
  switch (VPlanIdioms::isSearchLoop(Plan, VF, true, PeelArrayRef)) {
  case VPlanIdioms::Unsafe:
    return UnknownCost;
  case VPlanIdioms::SearchLoopStrEq:
    // Without proper type information, cost model cannot properly compute the
    // cost, thus hard code VF.
    if (VF == 1)
      return 1000;
    if (VF != 32)
      // Return some huge value, so that VectorCost still could be computed.
      return UnknownCost;
    break;
  case VPlanIdioms::SearchLoopStructPtrEq:
    // Without proper type information, cost model cannot properly compute the
    // cost, thus hard code VF.
    if (VF == 1)
      return 1000;
    if (VF != 4)
      // Return some huge value, so that VectorCost still could be computed.
      return UnknownCost;
    break;
  default:
    // FIXME: Keep VF = 32 as unsupported right now due to huge perf
    // regressions.
    if (VF == 32)
      return UnknownCost;
  }

  LLVM_DEBUG(dbgs() << "Number of i1 calculations: " << NumberOfBoolComputations
                    << "\n");
  if (VF != 1 && NumberOfBoolComputations >= BoolInstsBailOut) {
    LLVM_DEBUG(
        dbgs() << "Returning UnknownCost due to too many i1 calculations.\n");
    return UnknownCost;
  }

  return Cost;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPlanCostModelProprietary::printForVPInstruction(
  raw_ostream &OS, const VPInstruction *VPInst) {
  OS << "  Cost " << getCostNumberString(getCost(VPInst)) << " for ";
  VPInst->print(OS);
  // TODO: Attributes yet to be populated.
  std::string Attributes = "";

  if (OVLSGroup *Group = VLSA->getGroupsFor(Plan, VPInst))
    if (ProcessedOVLSGroups.count(Group) != 0)
      Attributes += "OVLS ";

  if (!Attributes.empty())
    OS << " ( " << Attributes << ")";
  OS << '\n';
}

void VPlanCostModelProprietary::printForVPBasicBlock(
  raw_ostream &OS, const VPBasicBlock *VPBB) {
  OS << "Analyzing VPBasicBlock " << VPBB->getName() << ", total cost: " <<
    getCostNumberString(getCost(VPBB)) << '\n';

  // Clearing ProcessedOVLSGroups is valid while VLS works within a basic block.
  // TODO: The code should be revisited once the assumption is changed.
  // Clearing before Instruction traversal is required to allow Instruction
  // dumping function to print out correct Costs and not required for CM to
  // work properly.
  ProcessedOVLSGroups.clear();

  for (const VPInstruction &VPInst : *VPBB)
    printForVPInstruction(OS, &VPInst);
}

void VPlanCostModelProprietary::print(
  raw_ostream &OS, const std::string &Header) {
  OS << "HIR Cost Model for VPlan " << Header << " with VF = " << VF << ":\n";
  OS << "Total Cost: " << getCost() << '\n';
  LLVM_DEBUG(dbgs() << *Plan;);

  // TODO: match print order with "vector execution order".
  for (const VPBasicBlock *Block : depth_first(Plan->getEntryBlock()))
    printForVPBasicBlock(OS, Block);
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

} // namespace vpo

} // namespace llvm
