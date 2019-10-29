//===-- IntelLoopVectorizationPlanner.cpp ---------------------------------===//
//
//   Copyright (C) 2016-2019 Intel Corporation. All rights reserved.
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
#include "IntelLoopVectorizationLegality.h"
#include "IntelNewVPlanPredicator.h"
#include "IntelVPlanCostModel.h"
#include "IntelVPlanHCFGBuilder.h"
#include "IntelVPlanPredicator.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#if INTEL_CUSTOMIZATION
#include "IntelVPlanCostModelProprietary.h"
#include "IntelVPlanIdioms.h"
#include "IntelVPlanClone.h"
#include "VPlanHIR/IntelVPlanHCFGBuilderHIR.h"
#endif // INTEL_CUSTOMIZATION

#define DEBUG_TYPE "LoopVectorizationPlanner"

#if INTEL_CUSTOMIZATION
extern llvm::cl::opt<bool> VPlanConstrStressTest;
extern llvm::cl::opt<bool> EnableVPValueCodegen;

static cl::opt<unsigned>
VecThreshold("vec-threshold",
             cl::desc("sets a threshold for the vectorization on the probability"
                      "of profitable execution of the vectorized loop in parallel."),
             cl::init(100));

static cl::opt<bool>
    EnableNewVPlanPredicator("enable-new-vplan-predicator", cl::init(true),
                             cl::Hidden,
                             cl::desc("Enable New VPlan predicator."));
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

static cl::list<unsigned> VPlanCostModelPrintAnalysisForVF(
    "vplan-cost-model-print-analysis-for-vf", cl::Hidden, cl::CommaSeparated,
    cl::ZeroOrMore,
    cl::desc("Print detailed VPlan Cost Model Analysis report for the given "
             "VF. For testing/debug purposes only."));

using namespace llvm;
using namespace llvm::vpo;

static unsigned getForcedVF(const WRNVecLoopNode *WRLp) {
  if (VPlanForceVF)
    return VPlanForceVF;
  return WRLp && WRLp->getSimdlen() ? WRLp->getSimdlen() : 0;
}

#if INTEL_CUSTOMIZATION
static unsigned getSafelen(const WRNVecLoopNode *WRLp) {
  return WRLp && WRLp->getSafelen() ? WRLp->getSafelen() : UINT_MAX;
}
#endif // INTEL_CUSTOMIZATION

unsigned LoopVectorizationPlanner::buildInitialVPlans(LLVMContext *Context,
                                                      const DataLayout *DL) {
  collectDeadInstructions();

  unsigned MinVF, MaxVF;
  unsigned ForcedVF = getForcedVF(WRLp);

#if INTEL_CUSTOMIZATION
  unsigned Safelen = getSafelen(WRLp);

  LLVM_DEBUG(dbgs() << "LVP: ForcedVF: " << ForcedVF << "\n");
  LLVM_DEBUG(dbgs() << "LVP: Safelen: " << Safelen << "\n");

  // Early return from vectorizer if forced VF or safelen is 1
  if (ForcedVF == 1 || Safelen == 1) {
    LLVM_DEBUG(dbgs() << "LVP: The forced VF or safelen specified by user is "
                         "1, VPlans need not be constructed.\n");
    return 0;
  }
#endif // INTEL_CUSTOMIZATION

  if (ForcedVF) {
#if INTEL_CUSTOMIZATION
    if (ForcedVF > Safelen) {
      // We are bailing out of vectorization if ForcedVF > safelen
      assert(WRLp && WRLp->isOmpSIMDLoop() &&
             "safelen is set on a non-OMP SIMD loop.");
      LLVM_DEBUG(dbgs() << "VPlan: The forced VF is greater than safelen set "
                           "via `#pragma omp simd`\n");
      return 0;
    }
#endif // INTEL_CUSTOMIZATION
    MinVF = ForcedVF;
    MaxVF = ForcedVF;
#if INTEL_CUSTOMIZATION
  } else if (VPlanConstrStressTest) {
    // If we are only stress testing VPlan construction, force VPlan
    // construction for just VF 1. This avoids any divide by zero errors in the
    // min/max VF computation.
    MinVF = 1;
    MaxVF = 1;
#endif // INTEL_CUSTOMIZATION
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
#if INTEL_CUSTOMIZATION
    LLVM_DEBUG(dbgs() << "LVP: Orig MinVF: " << MinVF
                      << " Orig MaxVF: " << MaxVF << "\n");
    // Maximum allowed VF specified by user is Safelen
    MaxVF = std::min(MaxVF, Safelen);

    // If the minimum VF in the search space is greater than Safelen specified
    // by user, then we reduce the minimum VF to nearest power of 2 less than
    // or equal to Safelen
    MinVF = std::min(MinVF, (unsigned)PowerOf2Floor(Safelen));

    // FIXME: Potentially MinVF can be greater than MaxVF if TTI will start to
    // return 512, 1024 or higher values.
    assert(MinVF <= MaxVF && "Invalid range of VFs");
#else
    // FIXME: Potentially MinVF can be greater than MaxVF if TTI will start to
    // return 512, 1024 or higher values.
    assert(MinVF < MaxVF && "Invalid range of VFs");
#endif // INTEL_CUSTOMIZATION
  }

#if INTEL_CUSTOMIZATION
  LLVM_DEBUG(dbgs() << "LVP: MinVF: " << MinVF << " MaxVF: " << MaxVF << "\n");
#endif // INTEL_CUSTOMIZATION

  unsigned StartRangeVF = MinVF;
  unsigned EndRangeVF = MaxVF + 1;

  unsigned i = 0;
  for (; StartRangeVF < EndRangeVF; ++i) {
    // TODO: revisit when we build multiple VPlans.
    std::shared_ptr<VPlan> Plan =
        buildInitialVPlan(StartRangeVF, EndRangeVF, Context, DL);

    if (VPlanUseVPEntityInstructions) {
      VPLoop *MainLoop = *(Plan->getVPLoopInfo()->begin());
      // Loop entities may be not created in some cases.
      VPLoopEntityList *LE = Plan->getOrCreateLoopEntities(MainLoop);
      VPBuilder VPIRBuilder;
      LE->doEscapeAnalysis();
      LE->insertVPInstructions(VPIRBuilder);
      LLVM_DEBUG(Plan->setName("After insertion VPEntities instructions\n");
                 dbgs() << *Plan;);
    }

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
  const unsigned MaxVF = 32;
  VPlan *ScalarPlan = getVPlanForVF(1);
  assert(ScalarPlan && "There is no scalar VPlan!");
  // FIXME: Without peel and remainder vectorization, it's ok to get trip count
  // from the original loop. Has to be revisited after enabling of
  // peel/remainder vectorization.

  // Even if TripCount is more than 2^32 we can safely assume that it's equal
  // to 2^32, otherwise all logic below will have a problem with overflow.
  VPLoopInfo *VPLI = ScalarPlan->getVPLoopInfo();
  assert(std::distance(VPLI->begin(), VPLI->end()) == 1
         && "Expected single outermost loop!");
  VPLoop *OuterMostVPLoop = *VPLI->begin();
  uint64_t TripCount = std::min(OuterMostVPLoop->getTripCountInfo().TripCount,
                                (uint64_t)std::numeric_limits<unsigned>::max());
#if INTEL_CUSTOMIZATION
  CostModelTy ScalarCM(ScalarPlan, 1, TTI, DL, VLSA);
#else
  CostModelTy ScalarCM(ScalarPlan, 1, TTI, DL);
#endif // INTEL_CUSTOMIZATION
  unsigned ScalarIterationCost = ScalarCM.getCost();
  ScalarIterationCost =
      ScalarIterationCost == CostModelTy::UnknownCost ? 0 : ScalarIterationCost;
  // FIXME: that multiplication should be the part of CostModel - see below.
  uint64_t ScalarCost = ScalarIterationCost * TripCount;

  BestVF = 1;
  // FIXME: Currently, unroll is not in VPlan, so set it to 1.
  BestUF = 1;
  uint64_t BestCost = ScalarCost;
  LLVM_DEBUG(dbgs() << "Cost of Scalar VPlan: " << ScalarCost << '\n');

#if INTEL_CUSTOMIZATION
  // When a loop is marked with pragma vector always, the user wants the loop
  // to be vectorized. In order to force the loop to be vectorized, we set
  // BestCost to MAX value for such a case. WRLp can be null when stress testing
  // vector code generation.
  bool ShouldIgnoreProfitability =
      WRLp && (WRLp->getHasVectorAlways() || WRLp->isOmpSIMDLoop());

  bool IsVectorAlways = ShouldIgnoreProfitability || VecThreshold == 0;

  if (IsVectorAlways) {
    BestCost = std::numeric_limits<uint64_t>::max();
    LLVM_DEBUG(dbgs() << "'#pragma vector always'/ '#pragma omp simd' is used "
                         "for the given loop\n");
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
#if INTEL_CUSTOMIZATION
    CostModelTy VectorCM(Plan, VF, TTI, DL, VLSA);
#else
    CostModelTy VectorCM(Plan, VF, TTI, DL);
#endif // INTEL_CUSTOMIZATION
    const unsigned VectorIterationCost = VectorCM.getCost();
    if (VectorIterationCost == CostModelTy::UnknownCost) {
      LLVM_DEBUG(dbgs() << "Cost for VF = " << VF << " is unknown. Skip it.\n");
      continue;
    }

    const decltype(TripCount) VectorTripCount = TripCount / VF;
    const decltype(TripCount) RemainderTripCount = TripCount % VF;
    // TODO: Take into account overhead for some instructions until explicit
    // representation of peel/remainder not ready.
    uint64_t VectorCost = VectorIterationCost * VectorTripCount +
                          ScalarIterationCost * RemainderTripCount;
    if (0 < VecThreshold && VecThreshold < 100) {
      LLVM_DEBUG(dbgs() << "Applying threshold " << VecThreshold << " for VF "
                        << VF << ". Original cost = " << VectorCost << '\n');
      VectorCost = (uint64_t)(VectorCost * (100.0 - VecThreshold))/100.0f;
    }
    const char CmpChar =
        ScalarCost < VectorCost ? '<' : ScalarCost == VectorCost ? '=' : '>';
    (void) CmpChar;
    LLVM_DEBUG(dbgs() << "Scalar Cost = " << TripCount << " x "
                      << ScalarIterationCost << " = " << ScalarCost << ' '
                      << CmpChar << " VectorCost = " << VectorTripCount << "[x"
                      << VF << "] x " << VectorIterationCost << " + "
                      << ScalarIterationCost << " x " << RemainderTripCount
                      << " = " << VectorCost << '\n';);
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

    if (EnableNewVPlanPredicator) {
      NewVPlanPredicator VPP(*VPlan);
      VPP.predicate();
    } else {
      VPlanPredicator VPP(VPlan);
      VPP.predicate();
    }

    PredicatedVPlans.insert(VPlan);
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
template <typename CostModelTy>
void LoopVectorizationPlanner::printCostModelAnalysisIfRequested() {
  for (unsigned VFRequested : VPlanCostModelPrintAnalysisForVF) {
    if (!hasVPlanForVF(VFRequested)) {
      errs() << "VPlan for VF = " << VFRequested << " was not constructed\n";
      continue;
    }
    VPlan *Plan = getVPlanForVF(VFRequested);
#if INTEL_CUSTOMIZATION
    CostModelTy CM(Plan, VFRequested, TTI, DL, VLSA);
#else
    CostModelTy CM(Plan, VFRequested, TTI, DL);
#endif // INTEL_CUSTOMIZATION

    // If different stages in VPlanDriver were proper passes under pass manager
    // control it would have been opt's output stream (via "-o" switch). As it
    // is not so, just pass stdout so that we would not be required to redirect
    // stderr to Filecheck.
    CM.print(outs());
  }
}

// Explicit instantiations.
template void
LoopVectorizationPlanner::printCostModelAnalysisIfRequested<VPlanCostModel>();
template void LoopVectorizationPlanner::printCostModelAnalysisIfRequested<
    VPlanCostModelProprietary>();
#endif // !NDEBUG || LLVM_ENABLE_DUMP

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

std::shared_ptr<VPlan> LoopVectorizationPlanner::buildInitialVPlan(
    unsigned StartRangeVF, unsigned &EndRangeVF, LLVMContext *Context,
    const DataLayout *DL) {
  // Create new empty VPlan
  std::shared_ptr<VPlan> SharedPlan =
      std::make_shared<VPlan>(Context, DL);
  VPlan *Plan = SharedPlan.get();

  // Build hierarchical CFG
  VPlanHCFGBuilder HCFGBuilder(TheLoop, LI, SE, *DL, WRLp, Plan, Legal);
  HCFGBuilder.buildHierarchicalCFG();

  return SharedPlan;
}

// Feed explicit data, saved in WRNVecLoopNode to the CodeGen.
#if INTEL_CUSTOMIZATION
template <class Legality> constexpr IRKind getIRKindByLegality() {
  return std::is_same<Legality, VPOVectorizationLegality>::value
             ? IRKind::LLVMIR
             : IRKind::HIR;
}

template <class VPOVectorizationLegality>
#endif
void LoopVectorizationPlanner::EnterExplicitData(WRNVecLoopNode *WRLp,
                                                 VPOVectorizationLegality &LVL) {
#if INTEL_CUSTOMIZATION
  constexpr IRKind Kind = getIRKindByLegality<VPOVectorizationLegality>();
  if (Kind == IRKind::LLVMIR && EnableVPValueCodegen)
    VPlanUseVPEntityInstructions = true;
#endif
  // Collect any SIMD loop private information
  if (WRLp) {
    LastprivateClause &LastPrivateClause = WRLp->getLpriv();
    for (LastprivateItem *PrivItem : LastPrivateClause.items()) {
#if INTEL_CUSTOMIZATION
      auto PrivVal = PrivItem->getOrig<Kind>();
#else
      auto PrivVal = PrivItem->getOrig();
#endif
      LVL.addLoopPrivate(PrivVal, true, PrivItem->getIsConditional());
    }
    PrivateClause &PrivateClause = WRLp->getPriv();
    for (PrivateItem *PrivItem : PrivateClause.items()) {
#if INTEL_CUSTOMIZATION
      auto PrivVal = PrivItem->getOrig<Kind>();
#else
      auto PrivVal = PrivItem->getOrig();
#endif
      LVL.addLoopPrivate(PrivVal);
    }

    // Add information about loop linears to Legality
    LinearClause &LinearClause = WRLp->getLinear();
    for (LinearItem *LinItem : LinearClause.items()) {
#if INTEL_CUSTOMIZATION
      auto LinVal = LinItem->getOrig<Kind>();
      auto Step = LinItem->getStep<Kind>();
#else
      auto LinVal = LinItem->getOrig();
      auto Step = LinItem->getStep();
#endif
      LVL.addLinear(LinVal, Step);
    }

    ReductionClause &RedClause = WRLp->getRed();
    for (ReductionItem *RedItem : RedClause.items()) {
#if INTEL_CUSTOMIZATION
      auto V = RedItem->getOrig<Kind>();
#else
      auto V = RedItem->getOrig();
#endif
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
namespace llvm {
namespace vpo {
#if INTEL_CUSTOMIZATION
template void
LoopVectorizationPlanner::EnterExplicitData<HIRVectorizationLegality>(
    WRNVecLoopNode *WRLp, HIRVectorizationLegality &LVL);
template void
LoopVectorizationPlanner::EnterExplicitData<VPOVectorizationLegality>(
    WRNVecLoopNode *WRLp, VPOVectorizationLegality &LVL);
#endif
} // namespace llvm
} // namespace vpo

void LoopVectorizationPlanner::executeBestPlan(VPOCodeGen &LB) {
  assert(BestVF != 1 && "Non-vectorized loop should be handled elsewhere!");
  ILV = &LB;

  // Perform the actual loop widening (vectorization).
  // 1. Create a new empty loop. Unlink the old loop and connect the new one.
  ILV->createEmptyLoop();

  // 2. Widen each instruction in the old loop to a new one in the new loop.
  VPCallbackILV CallbackILV;

  VPlan *Plan = getVPlanForVF(BestVF);
  assert(Plan && "No VPlan found for BestVF.");
  VPTransformState State(BestVF, BestUF, LI, DT, ILV->getBuilder(), ILV,
                         CallbackILV, Legal, Plan->getVPLoopInfo());
  State.CFG.PrevBB = ILV->getLoopVectorPH();

#if INTEL_CUSTOMIZATION
  // Set ILV transform state
  ILV->setTransformState(&State);
#endif // INTEL_CUSTOMIZATION

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
