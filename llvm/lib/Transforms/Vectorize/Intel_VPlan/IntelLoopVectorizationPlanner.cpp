//===-- IntelLoopVectorizationPlanner.cpp ---------------------------------===//
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

#include "IntelLoopVectorizationPlanner.h"
#include "IntelLoopVectorizationCodeGen.h"
#include "IntelVPlanCostModel.h"
#include "IntelVPlanHCFGBuilder.h"
#include "IntelVPlanPredicator.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"
#if INTEL_CUSTOMIZATION
#include "IntelVPlanCostModelProprietary.h"
#include "IntelVPlanIdioms.h"
#endif // INTEL_CUSTOMIZATION

#define DEBUG_TYPE "LoopVectorizationPlanner"

#if INTEL_CUSTOMIZATION
cl::opt<uint64_t>
    VPlanDefaultEstTrip("vplan-default-est-trip", cl::init(300),
                        cl::desc("Default estimated trip count"));
static cl::opt<unsigned>
VecThreshold("vec-threshold",
             cl::desc("sets a threshold for the vectorization on the probability"
                      "of profitable execution of the vectorized loop in parallel."),
             cl::init(100));
#else
cl::opt<unsigned>
    VPlanDefaultEstTrip("vplan-default-est-trip", cl::init(300),
                        cl::desc("Default estimated trip count"));
#endif // INTEL_CUSTOMIZATION
static cl::opt<unsigned> VPlanForceVF(
    "vplan-force-vf", cl::init(0),
    cl::desc("Force VPlan to use given VF"));

static cl::opt<bool>
    DisableVPlanPredicator("disable-vplan-predicator", cl::init(false),
                           cl::Hidden, cl::desc("Disable VPlan predicator."));

using namespace llvm;
using namespace llvm::vpo;

static unsigned getForcedVF(const WRNVecLoopNode *WRLp) {
  if (VPlanForceVF)
    return VPlanForceVF;
  return WRLp && WRLp->getSimdlen() ? WRLp->getSimdlen() : 0;
}

// Return trip count for a given VPlan for a first loop during DFS.
// Assume, that VPlan has only 1 loop without peel and/or remainder(s).
// FIXME: This function is incorrect if peel, main and remainder loop will be
// explicitly represented in VPlan. Also it's incorrect for multi level loop
// vectorization.
static uint64_t getTripCountForFirstLoopInDfs(const VPlan *VPlan) {

  std::function<const VPLoopRegion *(const VPBlockBase *)> FindLoop =
      [&](const VPBlockBase *VPBlock) -> const VPLoopRegion * {
    if (const auto Loop = dyn_cast<const VPLoopRegion>(VPBlock))
      return Loop;

    if (const auto Region = dyn_cast<const VPRegionBlock>(VPBlock))
      for (const VPBlockBase *Block : depth_first(Region->getEntry()))
        if (const VPLoopRegion *Loop = FindLoop(Block))
          return Loop;

    return nullptr;
  };

  const auto Loop = FindLoop(VPlan->getEntry());

  return VPlan->getVPLoopAnalysis()->getTripCountFor(Loop);
}

unsigned LoopVectorizationPlanner::buildInitialVPlans() {
  collectDeadInstructions();

  unsigned MinVF, MaxVF;
  unsigned ForcedVF = getForcedVF(WRLp);
  if (ForcedVF) {
    MinVF = ForcedVF;
    MaxVF = ForcedVF;
  } else {
    unsigned MinWidthInBits, MaxWidthInBits;
    std::tie(MinWidthInBits, MaxWidthInBits) = getTypesWidthRangeInBits();
    const unsigned MinVectorWidth = TTI->getMinVectorRegisterBitWidth();
    // FIXME: Cannot simply call TTI->getRegisterBitWidth(true), because by
    // default it returns 32 regardless to Vector argument.
    // Hardcode register size to 512.
    const unsigned MaxVectorWidth =
        std::max(512u, TTI->getRegisterBitWidth(true) /* Vector */);
    // FIXME: Currently limits MaxVF by 32.
    MaxVF = std::min(MaxVectorWidth / MinWidthInBits, 32u);
    MinVF = std::max(MinVectorWidth / MaxWidthInBits, 1u);
    // FIXME: Potentially MinVF can be greater than MaxVF if TTI will start to
    // return 512, 1024 or higher values.
    assert(MinVF < MaxVF && "Invalid range of VFs");
  }

  unsigned StartRangeVF = MinVF;
  unsigned EndRangeVF = MaxVF + 1;

  unsigned i = 0;
  for (; StartRangeVF < EndRangeVF; ++i) {
    // TODO: revisit when we build multiple VPlans.
    std::shared_ptr<VPlan> Plan =
        buildInitialVPlan(StartRangeVF, EndRangeVF);

    for (unsigned TmpVF = StartRangeVF; TmpVF < EndRangeVF; TmpVF *= 2)
      VPlans[TmpVF] = Plan;

    StartRangeVF = EndRangeVF;
    EndRangeVF = MaxVF + 1;
  }

  // Scalar VPlan is not necessary when VF was forced.
  // TODO: Need it later for optreport to print potential speedup.
  if (!ForcedVF)
    VPlans[1] = VPlans[MinVF];

  return i;
}

/// Evaluate cost model for available VPlans and find the best one.
/// \Returns VF which corresponds to the best VPlan (could be VF = 1).
template <typename CostModelTy>
unsigned LoopVectorizationPlanner::selectBestPlan() {
  if (VPlans.size() == 1) {
    unsigned ForcedVF = getForcedVF(WRLp);
    assert(ForcedVF &&
           "Only one VPlan was constructed with non-forced vectorization.");
    BestVF = ForcedVF;
    // FIXME: this code should be revisited later to select best UF
    // even with forced VF.
    BestUF = 1;
    LLVM_DEBUG(dbgs() << "There is only VPlan with VF=" << BestVF
                      << ", selecting it.\n");
    return ForcedVF;
  }

  // FIXME: This value of MaxVF has to be aligned with value of MaxVF in
  // buildInitialVPlan.
  // TODO: Add options to set MinVF and MaxVF.
  const unsigned MaxVF = 16;
  VPlan *ScalarPlan = getVPlanForVF(1);
  assert(ScalarPlan && "There is no scalar VPlan!");
  // FIXME: Without peel and remainder vectorization, it's ok to get trip count
  // from the original loop. Has to be revisited after enabling of
  // peel/remainder vectorization.
  unsigned TripCount = ::getTripCountForFirstLoopInDfs(ScalarPlan);
  CostModelTy ScalarCM(ScalarPlan, 1, TTI, DL);
  unsigned ScalarIterationCost = ScalarCM.getCost();
  // FIXME: that multiplication should be the part of CostModel - see below.
  unsigned ScalarCost = ScalarIterationCost * TripCount;

  BestVF = 1;
  // FIXME: Currently, unroll is not in VPlan, so set it to 1.
  BestUF = 1;
  unsigned BestCost = ScalarCost;
  LLVM_DEBUG(dbgs() << "Cost of Scalar VPlan: " << ScalarCost << '\n');

#if INTEL_CUSTOMIZATION
  // When a loop is marked with pragma vector always, the user wants the loop
  // to be vectorized. In order to force the loop to be vectorized, we set
  // BestCost to MAX value for such a case. WRLp can be null when stress testing
  // vector code generation.
  bool IsVectorAlways =
      (WRLp && WRLp->getIgnoreProfitability()) || VecThreshold == 0;
  if (IsVectorAlways) {
    BestCost = std::numeric_limits<unsigned>::max();
    LLVM_DEBUG(dbgs() << "vector always is used for the given loop\n");
  }
#endif // INTEL_CUSTOMIZATION

  // FIXME: Currently limit this to VF = 16. Has to be fixed with more accurate
  // cost model.
  for (unsigned VF = 2; VF <= MaxVF; VF *= 2) {
    if (!hasVPlanForVF(VF))
      continue;

    if (TripCount < VF)
      continue; // FIXME: Consider masked low trip later.

    VPlan *Plan = getVPlanForVF(VF);

    // FIXME: The remainder loop should be an explicit part of VPlan and the
    // cost model should just do the right thing calulating the cost of the
    // plan. However this is not the case yet so do some simple heuristic.
    CostModelTy VectorCM(Plan, VF, TTI, DL);
    unsigned VectorIterationCost = VectorCM.getCost();
    // TODO: Take into account overhead for some instructions until explicit
    // representation of peel/remainder not ready.
    unsigned VectorCost = VectorIterationCost * (TripCount / VF) +
                          ScalarIterationCost * (TripCount % VF);
    if (0 < VecThreshold && VecThreshold < 100) {
      LLVM_DEBUG(dbgs() << "Applying threshold " << VecThreshold << " for VF "
                        << VF << ". Original cost = " << VectorCost << "\n");
      VectorCost = (unsigned)(VectorCost * (100.0 - VecThreshold))/100.0f;
    }
    LLVM_DEBUG(dbgs() << "Cost of VPlan for VF=" << VF << ": " << VectorCost
                      << '\n');
    if (VectorCost < BestCost) {
      BestCost = VectorCost;
      BestVF = VF;
    }
  }
#if INTEL_CUSTOMIZATION
  // Corner case: all available VPlans have UMAX cost.
  // With 'vector always' we have to vectorize with some VF, so select first
  // available VF.
  if (BestVF == 1 && IsVectorAlways)
    for (BestVF = 2; BestVF <= MaxVF; BestVF *= 2)
      if (hasVPlanForVF(BestVF))
        break;
#endif // INTEL_CUSTOMIZATION

  // Delete all other VPlans.
  for (auto &It : VPlans) {
    if (It.first != BestVF)
      VPlans.erase(It.first);
  }
  LLVM_DEBUG(dbgs() << "Selecting VPlan with VF=" << BestVF << '\n');
  return BestVF;
}

template unsigned
LoopVectorizationPlanner::selectBestPlan<VPlanCostModel>(void);
#if INTEL_CUSTOMIZATION
template unsigned
LoopVectorizationPlanner::selectBestPlan<VPlanCostModelProprietary>(void);
#endif // INTEL_CUSTOMIZATION

void LoopVectorizationPlanner::predicate() {
  if (DisableVPlanPredicator)
    return;

  DenseSet<VPlan*> PredicatedVPlans;
  for (auto It : VPlans) {
    if (It.first == 1)
      continue; // Ignore Scalar VPlan;
    VPlan *VPlan = It.second.get();
    if (PredicatedVPlans.count(VPlan))
      continue; // Already predicated.

    VPlanPredicator VPP(VPlan);
    VPP.predicate();
    PredicatedVPlans.insert(VPlan);
  }
}

// TODO: Current implementation is too aggressive and may lead to increase of
// compile time. Also similar changeset in community led to performance drops.
// See https://reviews.llvm.org/D44523 for a similar discussion about LV.
std::pair<unsigned, unsigned>
LoopVectorizationPlanner::getTypesWidthRangeInBits() const {
  unsigned MinWidth = static_cast<unsigned>(-1);
  unsigned MaxWidth = 0;

  for (const BasicBlock *BB : TheLoop->blocks()) {
    for (const Instruction &Inst : *BB) {
      if (auto Ty = Inst.getType()) {
        unsigned Width = Ty->getPrimitiveSizeInBits();

        // Ignore the conditions (i1) - they're unlikely to be widened.
        if (Width < 8)
          continue;

        MinWidth = std::min(MinWidth, Width);
        MaxWidth = std::max(MaxWidth, Width);
      }
    }
  }
  return {MinWidth, MaxWidth};
}

std::shared_ptr<VPlan>
LoopVectorizationPlanner::buildInitialVPlan(unsigned StartRangeVF,
                                            unsigned &EndRangeVF) {
  // Create new empty VPlan
  std::shared_ptr<VPlan> SharedPlan = std::make_shared<VPlan>(VPLA);
  VPlan *Plan = SharedPlan.get();

  // Build hierarchical CFG
  VPlanHCFGBuilder HCFGBuilder(TheLoop, LI, SE, WRLp, Plan, Legal);
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
