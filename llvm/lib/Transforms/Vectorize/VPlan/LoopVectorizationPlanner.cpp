//===-- LoopVectorizationPlanner.cpp --------------------------------------===//
//
//   Copyright (C) 2015-2017 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements LoopVectorizationPlannerBase and
/// LoopVectorizationPlanner.
///
//===----------------------------------------------------------------------===//

#include "LoopVectorizationPlanner.h"
#include "LoopVectorizationCodeGen.h"
#include "VPlanHCFGBuilder.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"

#define DEBUG_TYPE "LoopVectorizationPlanner"

using namespace llvm;

unsigned LoopVectorizationPlannerBase::buildInitialVPlans(unsigned MinVF,
                                                          unsigned MaxVF) {
  collectDeadInstructions();

  unsigned StartRangeVF = MinVF;
  unsigned EndRangeVF = MaxVF + 1;

  unsigned i = 0;
  for (; StartRangeVF < EndRangeVF; ++i) {
    std::shared_ptr<IntelVPlan> Plan =
        buildInitialVPlan(StartRangeVF, EndRangeVF);

    for (unsigned TmpVF = StartRangeVF; TmpVF < EndRangeVF; TmpVF *= 2)
      VPlans[TmpVF] = Plan;

    StartRangeVF = EndRangeVF;
    EndRangeVF = MaxVF + 1;
  }

  return i;
}

void LoopVectorizationPlannerBase::setBestPlan(unsigned VF, unsigned UF) {
  DEBUG(dbgs() << "Setting best plan to VF=" << VF << ", UF=" << UF << '\n');
  BestVF = VF;
  BestUF = UF;

  assert(VPlans.count(VF) && "Best VF does not have a VPlan.");
  // Delete all other VPlans.
  for (auto &Entry : VPlans) {
    if (Entry.first != VF)
      VPlans.erase(Entry.first);
  }
}

void LoopVectorizationPlannerBase::printCurrentPlans(const std::string &Title,
                                                     raw_ostream &O) {
  auto printPlan = [&](IntelVPlan *Plan, const SmallVectorImpl<unsigned> &VFs,
                       const std::string &Prefix) {
    std::string Title;
    raw_string_ostream RSO(Title);
    RSO << Prefix << " for VF=";
    if (VFs.size() == 1)
      RSO << VFs[0];
    else {
      RSO << "{";
      bool First = true;
      for (unsigned VF : VFs) {
        if (!First)
          RSO << ",";
        RSO << VF;
        First = false;
      }
      RSO << "}";
    }
    VPlanPrinter PlanPrinter(O, *Plan);
    PlanPrinter.dump(RSO.str());
  };

  if (VPlans.empty())
    return;

  IntelVPlan *Current = VPlans.begin()->second.get();

  SmallVector<unsigned, 4> VFs;
  for (auto &Entry : VPlans) {
    IntelVPlan *Plan = Entry.second.get();
    if (Plan != Current) {
      // Hit another VPlan. Print the current VPlan for the VFs it served thus
      // far and move on to the VPlan we just encountered.
      printPlan(Current, VFs, Title);
      Current = Plan;
      VFs.clear();
    }
    // Add VF to the list of VFs served by current VPlan.
    VFs.push_back(Entry.first);
  }
  // Print the current VPlan.
  printPlan(Current, VFs, Title);
}

std::shared_ptr<IntelVPlan>
LoopVectorizationPlanner::buildInitialVPlan(unsigned StartRangeVF,
                                            unsigned &EndRangeVF) {
  // TODO: StartRangeVF and EndRangeVF are not being used by now

  // Create new empty VPlan
  std::shared_ptr<IntelVPlan> SharedPlan = std::make_shared<IntelVPlan>();
  IntelVPlan *Plan = SharedPlan.get();
  IntelVPlanUtils PlanUtils(Plan);

  // Build hierarchical CFG
  VPlanHCFGBuilder HCFGBuilder(LI, SE, Legal);
  HCFGBuilder.buildHierarchicalCFG(TheLoop, WRLoop, Plan);

  return SharedPlan;
}

void LoopVectorizationPlanner::executeBestPlan(VPOCodeGen &LB) {
  // Collect any SIMD loop private information
  if (WRLoop) {
    LastprivateClause &LastPrivateClause = WRLoop->getLpriv();
    for (LastprivateItem *PrivItem : LastPrivateClause.items()) {
      auto PrivVal = PrivItem->getOrig();
      if (isa<AllocaInst>(PrivVal))
        LB.addLoopPrivate(PrivVal, true, PrivItem->getIsConditional());
    }
    PrivateClause &PrivateClause = WRLoop->getPriv();
    for (PrivateItem *PrivItem : PrivateClause.items()) {
      auto PrivVal = PrivItem->getOrig();
      if (isa<AllocaInst>(PrivVal))
        LB.addLoopPrivate(PrivVal);
    }

    // Add information about loop linears to Legality
    LinearClause &LinearClause = WRLoop->getLinear();
    for (LinearItem *LinItem : LinearClause.items()) {
      auto LinVal = LinItem->getOrig();

      // Currently front-end does not yet support globals - restrict to allocas
      // for now.
      if (isa<AllocaInst>(LinVal))
        LB.addLinear(LinVal, LinItem->getStep());
    }
  }
  
  ILV = &LB;

  // Perform the actual loop widening (vectorization).
  // 1. Create a new empty loop. Unlink the old loop and connect the new one.
  ILV->createEmptyLoop();

  // 2. Widen each instruction in the old loop to a new one in the new loop.

  VPTransformState State(BestVF, BestUF, LI, DT, ILV->getBuilder(), ILV, &Legal);
  State.CFG.PrevBB = ILV->getLoopVectorPH();

  VPlan *Plan = getVPlanForVF(BestVF);

  ILV->collectUniformsAndScalars(BestVF);

  Plan->vectorize(&State);

  // 3. Take care of phi's to fix: reduction, 1st-order-recurrence, loop-closed.
  ILV->finalizeLoop();
}

void LoopVectorizationPlanner::collectDeadInstructions() {
  VPOCodeGen::collectTriviallyDeadInstructions(TheLoop, &Legal,
                                               DeadInstructions);
}
