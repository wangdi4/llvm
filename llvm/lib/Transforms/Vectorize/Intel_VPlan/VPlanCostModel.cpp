//===-- VPlanCostModel.cpp ------------------------------------------------===//
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
// This file implements VPlan cost modeling.
//
//===----------------------------------------------------------------------===//

#include "VPlanCostModel.h"
#include "VPlan.h"
#include <llvm/Analysis/TargetTransformInfo.h>

#define DEBUG_TYPE "vplan-cost-model"

using namespace llvm;
using namespace vpo;

unsigned VPlanCostModel::getCost(const VPInstruction *VPInst) {
  unsigned Opcode = VPInst->getOpcode();
  switch (Opcode) {
  default:
    return UnknownCost;
  }
}

unsigned VPlanCostModel::getCost(const VPBasicBlock *VPBB) {
  unsigned Cost = 0;
  for (const VPRecipeBase &Recipe : *VPBB) {
    const VPInstruction *VPInst = dyn_cast<VPInstruction>(&Recipe);
    // FIXME: cost of other recipes?
    if (!VPInst)
      continue;

    unsigned InstCost = getCost(VPInst);
    if (InstCost == UnknownCost)
      return UnknownCost;
    Cost += InstCost;
  }

  return Cost;
}

void VPlanCostModel::printForVPBlockBase(raw_ostream &OS, const VPBlockBase *VPBlock) {
  // TODO: match print order with "vector execution order".
  if (auto Region = dyn_cast<VPRegionBlock>(VPBlock)) {
    for (const VPBlockBase *Block : depth_first(Region->getEntry()))
      printForVPBlockBase(OS, Block);
    return;
  }

  const VPBasicBlock *VPBB = cast<VPBasicBlock>(VPBlock);
  OS << "Analyzing VPBasicBlock " << VPBB->getName() << ", total cost: ";
  unsigned VPBBCost = getCost(VPBB);
  if (VPBBCost == UnknownCost)
    OS << "Unknown\n";
  else
    OS << VPBBCost << '\n';

  for (const VPRecipeBase &Recipe : *VPBB) {
    const VPInstruction *VPInst = dyn_cast<VPInstruction>(&Recipe);
    // FIXME: cost of other recipes?
    if (!VPInst)
      continue;

    unsigned Cost = getCost(VPInst);
    if (Cost == UnknownCost)
      OS << "  Unknown cost for ";
    else
      OS << "  Cost " << Cost << " for ";
    VPInst->print(OS);
    OS << '\n';
  }
}

void VPlanCostModel::print(raw_ostream &OS) {
  OS << "Cost Model for VPlan: " << Plan->getName() << '\n';
  DEBUG(dbgs() << *Plan;);

  // TODO: match print order with "vector execution order".
  for (const VPBlockBase *Block : depth_first(Plan->getEntry()))
    printForVPBlockBase(OS, Block);
}
