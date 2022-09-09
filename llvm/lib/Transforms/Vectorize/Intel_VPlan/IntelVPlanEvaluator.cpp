//===-- IntelVPlanEvaluator.cpp -------------------------------------------===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanEvaluator.h"
#include "IntelLoopVectorizationPlanner.h"
#include "IntelVPlan.h"
#include "IntelVPlanCostModel.h"
#include "IntelVPlanVLSAnalysis.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"

#define DEBUG_TYPE "VPlanEvaluator"

static cl::opt<bool, true>
    EnableVectorizedPeelOpt("vplan-enable-vectorized-peel",
                            cl::location(llvm::vpo::EnableVectorizedPeel),
                            cl::desc("Enable vectorized peel."));

static cl::opt<bool, true> EnableNonMaskedVectorizedRemainderOpt(
    "vplan-enable-non-masked-vectorized-remainder",
    cl::location(llvm::vpo::EnableNonMaskedVectorizedRemainder),
    cl::desc("Enable non-masked vectorized remainder."));

static cl::opt<bool, true> EnableMaskedVectorizedRemainderOpt(
    "vplan-enable-masked-vectorized-remainder",
    cl::location(llvm::vpo::EnableMaskedVectorizedRemainder),
    cl::desc("Enable masked vectorized remainder."));

static cl::opt<bool> EnableEvaluatorsCostModelDumps(
    "vplan-enable-evaluators-cost-model-dumps",
    cl::init(false), cl::Hidden,
    cl::desc("Enable Cost Model dumps for loop peel & reminder for "
             "every VF tried."));

namespace llvm {
namespace vpo {
bool EnableVectorizedPeel = false;
bool EnableNonMaskedVectorizedRemainder = false;
bool EnableMaskedVectorizedRemainder = false;
} // namespace vpo
} // namespace llvm

using namespace llvm;
using namespace llvm::vpo;

// Calculates the cost of \p Plan for given \p VF. If \p Plan is not available
// (nullptr) then this function returns an invalid VPInstructionCost.
VPlanCostPair VPlanEvaluator::calculatePlanCost(unsigned VF,
                                                VPlanVector *Plan) {
  if (Plan) {
    raw_ostream *OS = nullptr;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    OS = EnableEvaluatorsCostModelDumps ? &outs() : nullptr;
#endif // !NDEBUG || LLVM_ENABLE_DUMP
    return Planner.createCostModel(Plan, VF)->getCost(
      nullptr /* PeelingVariant */, OS);
  }
  return std::make_pair(VPInstructionCost::getInvalid(),
                        VPInstructionCost::getInvalid());
}

// Peel loop's trip count might be available at compile-time or it might be
// calculated at run-time. In the worst case, the trip count of peel loop is
// equal to MainLoopVF-1. If the trip count is not available at compile-time, then we
// set the trip count to MainLoopVF-1.
unsigned VPlanPeelEvaluator::getScalarPeelTripCount(unsigned MainLoopVF) const {
  if (PeelingVariant)
    return PeelingVariant->getKind() == VPPK_StaticPeeling
               ? cast<VPlanStaticPeeling>(PeelingVariant)->peelCount()
               : MainLoopVF - 1;
  return 0;
}

// Selects the best peeling variant (none, scalar, masked vector).
VPlanPeelEvaluator::PeelLoopKind VPlanPeelEvaluator::calculateBestVariant() {

  if (!PeelingVariant || !ScalarIterCost.isValid() ||
      getScalarPeelTripCount(MainLoopVF) == 0) {
    PeelKind = PeelLoopKind::None;
    LoopCost = 0;
    PeelTC = 0;
    return PeelKind;
  }

  // Calculates the total cost of the masked vector peel loop.
  VPlanMasked *MaskedModePlan = Planner.getMaskedVPlanForVF(MainLoopVF);
  VPInstructionCost MaskedVectorIterCost, MaskedVectorOverhead;
  std::tie(MaskedVectorIterCost, MaskedVectorOverhead) =
      calculatePlanCost(MainLoopVF, MaskedModePlan);

  unsigned ScalarTC = getScalarPeelTripCount(MainLoopVF);
  if (MaskedVectorIterCost.isValid() && MaskedVectorOverhead.isValid() &&
      (ScalarIterCost * ScalarTC >
       (MaskedVectorIterCost + MaskedVectorOverhead)) &&
      EnableVectorizedPeelOpt) {
    PeelKind = PeelLoopKind::MaskedVector;
    PeelTC = ScalarTC;
    LoopCost = MaskedVectorIterCost + MaskedVectorOverhead;
  } else {
    PeelKind = PeelLoopKind::Scalar;
    PeelTC = ScalarTC;
    LoopCost = ScalarIterCost * ScalarTC;
  }
  // TODO: calculate the cost of the runtime checks when the interface is
  // available.
  // Overhead of peel checks is identical for both variants.
  // LoopCost += calculatePeelChecksCost(MainLoopVF);
  return PeelKind;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
const char *VPlanPeelEvaluator::getPeelLoopKindStr() const {
  switch (getPeelLoopKind()) {
  case PeelLoopKind::None:
    return "no peel loop";
  case PeelLoopKind::Scalar:
    return "scalar peel loop";
  case PeelLoopKind::MaskedVector:
    return "masked vector peel loop";
  }
  llvm_unreachable("bad peel loop kind");
  return "error";
}

void VPlanPeelEvaluator::dump(raw_ostream &OS) const {
  StringRef PeelLoopModeStr = "";
  StringRef TripCountStr = "";
  if (PeelingVariant) {
    if (isa<VPlanStaticPeeling>(PeelingVariant)) {
      PeelLoopModeStr = "(static)";
      TripCountStr = "known";
    }
    if (isa<VPlanDynamicPeeling>(PeelingVariant)) {
      PeelLoopModeStr = "(dynamic)";
      TripCountStr = "estimated";
    }
  }
  switch (getPeelLoopKind()) {
  case PeelLoopKind::None:
    OS << "There is no peel loop.\n";
    break;
  case PeelLoopKind::Scalar:
    OS << "The peel loop is scalar " << PeelLoopModeStr << " with "
       << TripCountStr << " trip count " << PeelTC << "."
       << " The scalar cost is " << LoopCost << "(" << PeelTC
       << " x " << ScalarIterCost << ").\n";
    break;
  case PeelLoopKind::MaskedVector:
    OS << "The peel loop " << PeelLoopModeStr << " has " << TripCountStr
       << " trip count " << PeelTC
       << " and it is vectorized with a mask. The vector cost is " << LoopCost
       << ".\n";
    break;
  }
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

// Finds the best VF to vectorize the remainder loop and calculates the total
// cost of vectorizing the remainder. After vectorizing the remainder loop, we
// might have a new remainder loop for the remaining iterations of the
// vectorized remainder loop.
void VPlanRemainderEvaluator::calculateRemainderVFAndVectorCost() {
  unsigned MaxRemainderTC = MainLoopVF * MainLoopUF - 1;
  UnMaskedVectorCost = VPInstructionCost::getInvalid();
  // The remainder loop cannot be vectorized with VF bigger than the one of the
  // main loop.
  for (unsigned TempVF = MainLoopVF / 2; TempVF > 1; TempVF /= 2) {
    // Cost of the vectorized remainder loop.
    VPlanVector *RemPlan = Planner.getVPlanForVF(TempVF);
    if (!RemPlan) {
      LLVM_DEBUG(dbgs() << "Remainder evaluator: no unmasked VPlan for VF="
                        << TempVF << "\n");
      continue;
    }
    VPInstructionCost TempIterCost, TempOverhead;
    std::tie(TempIterCost, TempOverhead) = calculatePlanCost(TempVF, RemPlan);
    if (!TempIterCost.isValid() || !TempOverhead.isValid()) {
      LLVM_DEBUG(dbgs() << "Remainder evaluator skips VF=" << TempVF
                        << " as corresponding VPlan cost is not valid\n");
      continue;
    }
    VPInstructionCost TempCost =
        TempOverhead + TempIterCost * (MaxRemainderTC / TempVF);
    LLVM_DEBUG(dbgs() << "Remainder evaluator unmasked cost for VF=" << TempVF
                      << " :\n Vector cost=" << TempCost << "= " << TempIterCost <<" x "
                      << (MaxRemainderTC / TempVF)
                      << " iterations + " << TempOverhead << "\n Scalar cost="
                      << (ScalarIterCost * (MaxRemainderTC % TempVF)) << " x "
                      << (MaxRemainderTC % TempVF) << " iterations\n");
    // Cost of the new scalar remainder loop.
    TempCost += ScalarIterCost * (MaxRemainderTC % TempVF);
    if (!TempCost.isValid()) {
      LLVM_DEBUG(dbgs() << "Remainder evaluator skips VF=" << TempVF
                        << " as corresponding VPlan final cost is " << TempCost
                        << "\n");
      continue;
    }
    LLVM_DEBUG(dbgs() << " Final cost=" << TempCost << "\n");
    if (!UnMaskedVectorCost.isValid() || TempCost < UnMaskedVectorCost) {
      UnMaskedVectorCost = TempCost;
      RemainderVF = TempVF;
    }
  }
}

// Selects the best peeling variant (none, scalar, vector/scalar, masked
// vector).
VPlanRemainderEvaluator::RemainderLoopKind
VPlanRemainderEvaluator::calculateBestVariant() {
  if (RemainderTC == 0) {
    RemainderKind = RemainderLoopKind::None;
    LoopCost = 0;
    return RemainderKind;
  }

  // Calculates the total cost for masked mode loop if it is available.
  VPlanMasked *MaskedModePlan = Planner.getMaskedVPlanForVF(MainLoopVF);
  VPInstructionCost MaskedVectorIterCost, MaskedOverhead;
  std::tie(MaskedVectorIterCost, MaskedOverhead) =
      calculatePlanCost(MainLoopVF, MaskedModePlan);
  VPInstructionCost MaskedVectorCost = MaskedOverhead +
    MaskedVectorIterCost * MainLoopUF;

  // Calculate VF and the total cost for vector variant with possible scalar
  // remainder.
  calculateRemainderVFAndVectorCost();

  VPInstructionCost ScalarRemainderLoopCost = ScalarIterCost * RemainderTC;
  RemainderKind = RemainderLoopKind::Scalar;
  LoopCost = ScalarRemainderLoopCost;
  LLVM_DEBUG(dbgs() << "Remainder evaluator: scalar cost="
                    << ScalarRemainderLoopCost
                    << " masked cost=" << MaskedVectorCost
                    << " unmasked cost=" << UnMaskedVectorCost << "\n");

  // Don't try vector version of remainder if it's disabled by pragma or
  // (not enabled by switch and not enforced by pragma). I.e. pragma overrides
  // the switch value always.
  if (Planner.isVecRemainderDisabled() ||
      (!Planner.isVecRemainderEnforced() && !EnableMaskedVectorizedRemainder &&
       !EnableNonMaskedVectorizedRemainder)) {
    LLVM_DEBUG(dbgs() << "No vector remainder enabled");
    LLVM_DEBUG(dbgs() << "Pragma: " << Planner.isVecRemainderDisabled()
                      << "opts: " << EnableMaskedVectorizedRemainder << ", "
                      << EnableNonMaskedVectorizedRemainder);
    return RemainderKind;
  }
  if (!MaskedVectorCost.isValid() && !UnMaskedVectorCost.isValid()) {
    LLVM_DEBUG(dbgs() << "Both vector costs are invalid.");
    return RemainderKind;
  }

  // if vectorization of remainder is enforced disregard scalar cost
  if (Planner.isVecRemainderEnforced())
    LoopCost = VPInstructionCost::getMax();

  if (MaskedVectorCost.isValid() && LoopCost.isValid() &&
      LoopCost > MaskedVectorCost &&
      (Planner.isVecRemainderEnforced() || EnableMaskedVectorizedRemainder)) {
    RemainderKind = RemainderLoopKind::MaskedVector;
    LoopCost = MaskedVectorCost;
  }
  if (UnMaskedVectorCost.isValid() && LoopCost.isValid() &&
      LoopCost > UnMaskedVectorCost &&
      (Planner.isVecRemainderEnforced() ||
       EnableNonMaskedVectorizedRemainder)) {
    RemainderKind = RemainderLoopKind::VectorScalar;
    LoopCost = UnMaskedVectorCost;
    RemainderTC = (MainLoopUF * MainLoopVF - 1) / RemainderVF;
    NewRemainderTC = (MainLoopUF * MainLoopVF - 1) % RemainderVF;
  }
  // TODO: calcualte the cost of run-time checks
  // LoopCost += calculateRTChecksCost(MainLoopVF);
  return RemainderKind;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
const char *VPlanRemainderEvaluator::getRemainderLoopKindStr() const {
  switch (getRemainderLoopKind()) {
  case RemainderLoopKind::None:
    return "no remainder loop";
  case RemainderLoopKind::Scalar:
    return "scalar remainder loop";
  case RemainderLoopKind::VectorScalar:
    return "vector remainder loop";
  case RemainderLoopKind::MaskedVector:
    return "masked vector remainder loop";
  }
  llvm_unreachable("bad remainder loop kind");
  return "error";
}

void VPlanRemainderEvaluator::dump(raw_ostream &OS) const {
  StringRef TripCountStr = PeelIsDynamic ? "estimated" : "known";
  switch (getRemainderLoopKind()) {
  case RemainderLoopKind::None:
    OS << "There is no remainder loop.\n";
    break;
  case RemainderLoopKind::Scalar:
    OS << "The remainder loop is scalar with " << TripCountStr << " trip count "
       << RemainderTC << "." << " The scalar cost is " << LoopCost << "("
       << RemainderTC << " x " << ScalarIterCost << ").\n";
    break;
  case RemainderLoopKind::VectorScalar:
    OS << "The remainder loop has " << TripCountStr << " trip count "
       << RemainderTC << " and it is vectorized with vector factor "
       << RemainderVF << ". The vector cost is " << LoopCost << ".\n";
    if (NewRemainderTC != 0)
      OS << "The remainder loop has a new remainder loop with "
         << TripCountStr << " trip count " << NewRemainderTC << ".\n";
    break;
  case RemainderLoopKind::MaskedVector:
    OS << "The remainder loop has " << TripCountStr << " trip count "
       << RemainderTC
       << " and it is vectorized with a mask. The vector cost is " << LoopCost
       << ".\n";
    break;
  }
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP
