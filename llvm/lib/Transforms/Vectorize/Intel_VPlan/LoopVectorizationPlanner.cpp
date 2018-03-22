//===-- LoopVectorizationPlanner.cpp --------------------------------------===//
//
//   Copyright (C) 2016-2018 Intel Corporation. All rights reserved.
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
#include "VPlanCostModel.h"
#include "VPlanHCFGBuilder.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"

#define DEBUG_TYPE "LoopVectorizationPlanner"


using namespace llvm;
using namespace llvm::vpo;

unsigned LoopVectorizationPlannerBase::buildInitialVPlans(unsigned MinVF,
                                                          unsigned MaxVF) {
  collectDeadInstructions();

  // TODO: Don't build Scalar VPlan if it's not needed.
  unsigned StartRangeVF = 1;
  unsigned EndRangeVF = MaxVF + 1;

  unsigned i = 0;
  for (; StartRangeVF < EndRangeVF; ++i) {
    std::shared_ptr<IntelVPlan> Plan =
        buildInitialVPlan(StartRangeVF, EndRangeVF);

    for (unsigned TmpVF = StartRangeVF; TmpVF < EndRangeVF; TmpVF *= 2)
      VPlans[TmpVF] = Plan;

    // If previously built VPlan was scalar (VF==1) and its end range is smaller
    // than MinVF we don't have to build the plans in range [EndRangeVF, MinVF).
    StartRangeVF = StartRangeVF == 1 ? std::max(EndRangeVF, MinVF) : EndRangeVF;
    EndRangeVF = MaxVF + 1;
  }

  return i;
}

unsigned LoopVectorizationPlannerBase::selectVF(unsigned VF, bool Forced) {
  if (Forced) {
    setBestPlan(VF, 1);
    return VF;
  }

  // For now, only choose between VF == 1 and VF == VPlanDefaultVF, but do so
  // only if the VF is not explicitly specified by the simd pragma.
  IntelVPlan *VectorPlan = getVPlanForVF(VF);
  assert(VectorPlan && "There is no VPlan for the requested VF!");
  IntelVPlan *ScalarPlan = getVPlanForVF(1);
  assert(ScalarPlan && "There is no scalar VPlan!");

  unsigned VectorCost = VPlanCostModel::getVPlanCost(VectorPlan, VF, TTI);
  unsigned ScalarCost = VPlanCostModel::getVPlanCost(ScalarPlan, 1, TTI);

  unsigned ChosenVF = VectorCost < VF * ScalarCost ? VF : 1;
  setBestPlan(ChosenVF, 1);
  return ChosenVF;
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

std::shared_ptr<IntelVPlan>
LoopVectorizationPlanner::buildInitialVPlan(unsigned StartRangeVF,
                                            unsigned &EndRangeVF) {
  // Create new empty VPlan
  std::shared_ptr<IntelVPlan> SharedPlan = std::make_shared<IntelVPlan>();
  IntelVPlan *Plan = SharedPlan.get();

  // Build hierarchical CFG
  VPlanHCFGBuilder HCFGBuilder(WRLp, TheLoop, Plan, LI, SE, Legal);
  HCFGBuilder.buildHierarchicalCFG();

  return SharedPlan;
}

// Feed explicit data, saved in WRNVecLoopNode to the CodeGen.
void LoopVectorizationPlanner::EnterExplicitData(
    WRNVecLoopNode *WRLp, VPOVectorizationLegality &LVL) {
  // Collect any SIMD loop private information
  if (WRLp) {
    LastprivateClause &LastPrivateClause = WRLp->getLpriv();
    for (LastprivateItem *PrivItem : LastPrivateClause.items()) {
      auto PrivVal = PrivItem->getOrig();
      if (isa<AllocaInst>(PrivVal))
        LVL.addLoopPrivate(PrivVal, true, PrivItem->getIsConditional());
    }
    PrivateClause &PrivateClause = WRLp->getPriv();
    for (PrivateItem *PrivItem : PrivateClause.items()) {
      auto PrivVal = PrivItem->getOrig();
      if (isa<AllocaInst>(PrivVal))
        LVL.addLoopPrivate(PrivVal);
    }

    // Add information about loop linears to Legality
    LinearClause &LinearClause = WRLp->getLinear();
    for (LinearItem *LinItem : LinearClause.items()) {
      auto LinVal = LinItem->getOrig();

      // Currently front-end does not yet support globals - restrict to allocas
      // for now.
      if (isa<AllocaInst>(LinVal))
        LVL.addLinear(LinVal, LinItem->getStep());
    }

    ReductionClause &RedClause = WRLp->getRed();
    for (ReductionItem *RedItem : RedClause.items()) {
      Value *V = RedItem->getOrig();
      ReductionItem::WRNReductionKind Type = RedItem->getType();
      switch (Type) {
      case ReductionItem::WRNReductionMin:
        LVL.addReductionMin(V, !RedItem->getIsUnsigned());
        break;
      case ReductionItem::WRNReductionMax:
        LVL.addReductionMax(V, !RedItem->getIsUnsigned());
        break;
      case ReductionItem::WRNReductionAdd:
      case ReductionItem::WRNReductionSub:
        LVL.addReductionAdd(V);
        break;
      case ReductionItem::WRNReductionMult:
        LVL.addReductionMult(V);
        break;
      case ReductionItem::WRNReductionBor:
        LVL.addReductionOr(V);
        break;
      case ReductionItem::WRNReductionBxor:
        LVL.addReductionXor(V);
        break;
      case ReductionItem::WRNReductionBand:
        LVL.addReductionAnd(V);
        break;
      default:
        break;
      }
    }
  }
}

void LoopVectorizationPlanner::executeBestPlan(VPOCodeGen &LB) {
  assert(BestVF != 1 && "Non-vectorized loop should be handled elsewhere!");
  ILV = &LB;

  // Perform the actual loop widening (vectorization).
  // 1. Create a new empty loop. Unlink the old loop and connect the new one.
  ILV->createEmptyLoop();

  // 2. Widen each instruction in the old loop to a new one in the new loop.
  VPCallbackILV CallbackILV;
  /*TODO: Necessary in VPO?*/
  VectorizerValueMap ValMap(BestVF, 1 /*UF*/);

  VPTransformState State(BestVF, BestUF, LI, DT, ILV->getBuilder(), ValMap, ILV,
                         CallbackILV, Legal);
  State.CFG.PrevBB = ILV->getLoopVectorPH();

  VPlan *Plan = getVPlanForVF(BestVF);
  // TODO: This should be removed once we get proper divergence analysis
  State.UniformCBVs = &Plan->UniformCBVs;

  ILV->collectUniformsAndScalars(BestVF);

  Plan->execute(&State);

  // 3. Take care of phi's to fix: reduction, 1st-order-recurrence, loop-closed.
  ILV->finalizeLoop();
}

void LoopVectorizationPlanner::collectDeadInstructions() {
  VPOCodeGen::collectTriviallyDeadInstructions(TheLoop, Legal,
                                               DeadInstructions);
}

