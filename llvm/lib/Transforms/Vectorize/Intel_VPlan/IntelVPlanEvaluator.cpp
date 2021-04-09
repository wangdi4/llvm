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

namespace llvm {
namespace vpo {
bool EnableVectorizedPeel = false;
bool EnableNonMaskedVectorizedRemainder = false;
bool EnableMaskedVectorizedRemainder = false;
} // namespace vpo
} // namespace llvm

using namespace llvm;
using namespace llvm::vpo;

// Calculates the cost of a Plan. If there is not any Plan available, then
// this function returns UINT_MAX.
unsigned VPlanEvaluator::calculatePlanCost(unsigned VF, VPlanVector *Plan) {
  if (Plan) {
    VPlanCostModel CM(Plan, VF, TTI, TLI, DL, VLSA);
    // TODO: no peeling should be accounted here, update after interface
    // changes.
    return CM.getCost();
  }
  return UINT_MAX;
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

  if (!PeelingVariant) {
    PeelKind = PeelLoopKind::None;
    LoopCost = 0;
    PeelTC = 0;
    return PeelKind;
  }

  // Calculates the total cost of the masked vector peel loop.
  VPlanMasked *MaskedModePlan = Planner.getMaskedVPlanForVF(MainLoopVF);
  unsigned MaskedVectorCost = calculatePlanCost(MainLoopVF, MaskedModePlan);

  unsigned ScalarTC = getScalarPeelTripCount(MainLoopVF);
  if (ScalarIterCost * ScalarTC > MaskedVectorCost && EnableVectorizedPeelOpt) {
    PeelKind = PeelLoopKind::MaskedVector;
    PeelTC = ScalarTC;
    LoopCost = MaskedVectorCost;
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
  switch (getPeelLoopKind()) {
  case PeelLoopKind::None:
    OS << "There is no peel loop.\n";
    break;
  case PeelLoopKind::Scalar:
    OS << "The peel loop is scalar with trip count " << PeelTC << "."
       << " The scalar cost is " << LoopCost << "(" << PeelTC << " x "
       << ScalarIterCost << ").\n";
    break;
  case PeelLoopKind::MaskedVector:
    OS << "The peel loop has trip count " << PeelTC
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
  UnMaskedVectorCost = UINT_MAX;
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
    unsigned TempCost =
        calculatePlanCost(TempVF, RemPlan) * (MaxRemainderTC / TempVF);
    LLVM_DEBUG(dbgs() << "Remainder evaluator unmasked cost for VF=" << TempVF
                      << " :\n Pure vector cost=" << TempCost << " x "
                      << (MaxRemainderTC / TempVF)
                      << " iterations \n Scalar cost="
                      << (ScalarIterCost * (MaxRemainderTC % TempVF)) << " x "
                      << (MaxRemainderTC % TempVF) << " iterations\n");
    // Cost of the new scalar remainder loop.
    TempCost += ScalarIterCost * (MaxRemainderTC % TempVF);
    LLVM_DEBUG(dbgs() << " Final cost=" << TempCost << "\n");
    if (TempCost < UnMaskedVectorCost) {
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
  unsigned MaskedVectorCost =
      calculatePlanCost(MainLoopVF, MaskedModePlan) * MainLoopUF;

  // Calculate VF and the total cost for vector variant with possible scalar
  // remainder.
  calculateRemainderVFAndVectorCost();

  unsigned ScalarRemainderLoopCost = ScalarIterCost * RemainderTC;
  RemainderKind = RemainderLoopKind::Scalar;
  LoopCost = ScalarRemainderLoopCost;
  LLVM_DEBUG(dbgs() << "Remainder evaluator: scalar cost="
                    << ScalarRemainderLoopCost
                    << " masked cost=" << MaskedVectorCost
                    << " unmasked cost=" << UnMaskedVectorCost << "\n");

  if (LoopCost > MaskedVectorCost && EnableMaskedVectorizedRemainderOpt) {
    RemainderKind = RemainderLoopKind::MaskedVector;
    LoopCost = MaskedVectorCost;
  }
  if (LoopCost > UnMaskedVectorCost && EnableNonMaskedVectorizedRemainderOpt) {
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
  switch (getRemainderLoopKind()) {
  case RemainderLoopKind::None:
    OS << "There is no remainder loop.\n";
    break;
  case RemainderLoopKind::Scalar:
    OS << "The remainder loop is scalar with trip count " << RemainderTC << "."
       << " The scalar cost is " << LoopCost << "(" << RemainderTC << " x "
       << ScalarIterCost << ").\n";
    break;
  case RemainderLoopKind::VectorScalar:
    OS << "The remainder loop has trip count " << RemainderTC
       << " and it is vectorized with vector factor " << RemainderVF
       << ". The vector cost is " << LoopCost << ".\n";
    if (NewRemainderTC != 0)
      OS << "The remainder loop has a new remainder loop with trip count "
         << NewRemainderTC << ".\n";
    break;
  case RemainderLoopKind::MaskedVector:
    OS << "The remainder loop has trip count " << RemainderTC
       << " and it is vectorized with a mask. The vector cost is " << LoopCost
       << ".\n";
    break;
  }
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP
