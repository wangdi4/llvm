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
#include <cmath>

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
bool EnableNonMaskedVectorizedRemainder = true;
bool EnableMaskedVectorizedRemainder = true;
int MaskedGainThreshold = 50;
int UnmaskedGainThreshold = 50;
} // namespace vpo
} // namespace llvm

static cl::opt<int, true> MaskedGainThresholdOpt(
    "vplan-masked-remainder-gain-threshold", cl::Hidden,
    cl::location(llvm::vpo::MaskedGainThreshold),
    cl::desc("Minimum value of masked remainder gain, in percent of scalar "
             "remainder cost. Used only for unknown TC. The bigger value "
             "the bigger gain required for masked remainder to be choosen."));

static cl::opt<int, true> UnmaskedGainThresholdOpt(
    "vplan-unmasked-remainder-gain-threshold", cl::Hidden,
    cl::location(llvm::vpo::UnmaskedGainThreshold),
    cl::desc("Minimum value of unmasked remainder gain, in percent of scalar "
             "remainder cost. Used only for unknown TC. The bigger value "
             "the bigger gain required for unmasked remainder to be choosen."));

using namespace llvm;
using namespace llvm::vpo;

// Calculates the cost of \p Plan for given \p VF. If \p Plan is not available
// (nullptr) then this function returns an invalid VPInstructionCost.
VPlanCostPair VPlanEvaluator::calculatePlanCost(unsigned VF, VPlanVector *Plan,
                                                bool AddZTT, bool ForPeel) {
  if (Plan) {
    raw_ostream *OS = nullptr;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    OS = EnableEvaluatorsCostModelDumps ? &outs() : nullptr;
#endif // !NDEBUG || LLVM_ENABLE_DUMP
    auto CostModel = Planner.createNoSLPCostModel(Plan, VF);
    VPInstructionCost IterCost, Overhead;
    std::tie(IterCost, Overhead) =
        CostModel->getCost(ForPeel, nullptr /* PeelingVariant */, OS);
    if (AddZTT) {
      VPInstructionCost ZttCost = CostModel->getZTTCost(
          Type::getInt64Ty(*Plan->getExternals().getLLVMContext()));
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
      if (OS && EnableEvaluatorsCostModelDumps)
        *OS << "Adding ZTT cost: " << ZttCost << "\n";
#endif // !NDEBUG || LLVM_ENABLE_DUMP
      Overhead += ZttCost;
    } else if (OS)
      *OS << "Not adding Ztt\n";
    return std::make_pair(IterCost, Overhead);
  }
  return std::make_pair(VPInstructionCost::getInvalid(),
                        VPInstructionCost::getInvalid());
}

// Peel loop's trip count might be available at compile-time or it might be
// calculated at run-time. In the worst case, the trip count of peel loop is
// equal to MainLoopVF-1. If the trip count is not available at compile-time,
// then we set the trip count to MainLoopVF-1.
unsigned VPlanPeelEvaluator::getScalarPeelTripCount(unsigned MainLoopVF) const {
  if (PeelingVariant) {
    if (PeelingVariant->getKind() == VPPK_NoPeeling ||
        PeelingVariant->getKind() == VPPK_NoPeelingAligned ||
        PeelingVariant->getKind() == VPPK_NoPeelingUnaligned)
      return 0;
    return PeelingVariant->getKind() == VPPK_StaticPeeling
               ? cast<VPlanStaticPeeling>(PeelingVariant)->peelCount()
               : MainLoopVF - 1;
  }
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
  std::tie(MaskedVectorIterCost, MaskedVectorOverhead) = calculatePlanCost(
      MainLoopVF, MaskedModePlan,
      PeelingVariant->getKind() == VPPK_DynamicPeeling, true /* ForPeel */);

  const unsigned ScalarTC = getScalarPeelTripCount(MainLoopVF);
  const auto ScalarLoopCost = ScalarIterCost * ScalarTC;
  const auto MaskedLoopCost = MaskedVectorIterCost + MaskedVectorOverhead;
  LLVM_DEBUG(dbgs() << "Peel evaluator for VF=" << MainLoopVF << ": "
                    << "scalar cost=" << ScalarLoopCost << " "
                    << "masked cost=" << MaskedLoopCost << " "
                    << "masked gain=" << ScalarLoopCost - MaskedLoopCost
                    << "\n");

  if (EnableVectorizedPeel) {
    if (MaskedLoopCost.isValid() && ScalarLoopCost > MaskedLoopCost) {
      LLVM_DEBUG(
          dbgs() << "Choosing masked peel (masked peel is more profitable)\n");
      PeelKind = PeelLoopKind::MaskedVector;
      LoopCost = MaskedLoopCost;
      PeelTC = ScalarTC;
    } else {
      LLVM_DEBUG(
          dbgs() << "Choosing scalar peel (scalar peel is more profitable)\n");
      PeelKind = PeelLoopKind::Scalar;
      LoopCost = ScalarLoopCost;
      PeelTC = ScalarTC;
    }
  } else {
    LLVM_DEBUG(dbgs() << "Choosing scalar peel (masked peel disabled)\n");
    PeelKind = PeelLoopKind::Scalar;
    LoopCost = ScalarLoopCost;
    PeelTC = ScalarTC;
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
    if (isa<VPlanNoPeeling>(PeelingVariant)) {
      PeelLoopModeStr = "(disabled)";
      TripCountStr = "known";
    }
    if (isa<VPlanNoPeelingAligned>(PeelingVariant)) {
      PeelLoopModeStr = "(disabled, aligned)";
      TripCountStr = "known";
    }
    if (isa<VPlanNoPeelingUnaligned>(PeelingVariant)) {
      PeelLoopModeStr = "(disabled, unaligned)";
      TripCountStr = "known";
    }
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
  UnmaskedVectorCost = VPInstructionCost::getInvalid();

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
    VPLoop *L = RemPlan->getMainLoop(true /*strict_check*/);
    if (!L->hasNormalizedInduction()) {
      LLVM_DEBUG(dbgs() << "Remainder evaluator: unmasked VPlan for VF="
                        << TempVF << " is not normalized\n");
      continue;
    }
    if (TempVF > RemainderTC) {
      LLVM_DEBUG(dbgs() << "Remainder evaluator skips VF=" << TempVF
                        << " as it's bigger than remainder TC\n");
      continue;
    }
    VPInstructionCost TempIterCost, TempOverhead;
    std::tie(TempIterCost, TempOverhead) =
        calculatePlanCost(TempVF, RemPlan, TCIsUnknown);
    if (!TempIterCost.isValid() || !TempOverhead.isValid()) {
      LLVM_DEBUG(dbgs() << "Remainder evaluator skips VF=" << TempVF
                        << " as corresponding VPlan cost is not valid\n");
      continue;
    }
    VPInstructionCost TempCost =
        TempOverhead + TempIterCost * (RemainderTC / TempVF);
    LLVM_DEBUG(dbgs() << "Remainder evaluator unmasked cost for VF=" << TempVF
                      << " :\n Vector cost=" << TempCost << "= " << TempIterCost
                      << " x " << (RemainderTC / TempVF) << " iterations + "
                      << TempOverhead << "\n Scalar cost="
                      << (ScalarIterCost * (RemainderTC % TempVF)) << "= "
                      << ScalarIterCost << " x " << (RemainderTC % TempVF)
                      << " iterations\n");
    // Cost of the new scalar remainder loop.
    TempCost += ScalarIterCost * (RemainderTC % TempVF);
    if (!TempCost.isValid()) {
      LLVM_DEBUG(dbgs() << "Remainder evaluator skips VF=" << TempVF
                        << " as corresponding VPlan final cost is " << TempCost
                        << "\n");
      continue;
    }
    LLVM_DEBUG(dbgs() << " Final cost=" << TempCost << "\n");
    if (!UnmaskedVectorCost.isValid() || TempCost < UnmaskedVectorCost) {
      UnmaskedVectorCost = TempCost;
      RemainderVF = TempVF;
    }
  }
}

VPInstructionCost
VPlanRemainderEvaluator::calculatePumpingOverhead(VPlanMasked *MaskedModePlan) {

  VPInstructionCost Ret = 0;
  if (!MaskedModePlan)
    return Ret;

  unsigned MaxRegWidth =
      TTI->getRegisterBitWidth(TargetTransformInfo::RGK_FixedWidthVector)
          .getFixedValue();

  for (auto &VPInstR : vpinstructions(MaskedModePlan)) {
    VPInstruction *VPInst = &VPInstR;
    if (VPInst->getParent()->getPredicate() == nullptr)
      continue;
    if (isa<VPGEPInstruction>(VPInst) || isa<VPSubscriptInst>(VPInst) ||
        isa<VPPHINode>(VPInst) || isa<VPBlendInst>(VPInst))
      continue;

    Type *Ty;
    if (auto LdSt = dyn_cast<VPLoadStoreInst>(VPInst))
      Ty = LdSt->getValueType();
    else
      Ty = VPInst->getType();

    if (Ty->isVoidTy() || !isVectorizableTy(Ty))
      continue;

    unsigned CurrWidth = Ty->getPrimitiveSizeInBits() * MainLoopVF;
    // As we introduce masking artificially, we consider every masked instruction
    // on additional registers as an overhead. Add #registers.
    if (CurrWidth > MaxRegWidth)
      Ret += CurrWidth / MaxRegWidth;
  }
  return Ret;
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
  // Calculate masked vplan cost w/o adding ZTT cost: it's added for scalar
  // remainder as well so should not affect comparison with scalar remainder.
  std::tie(MaskedVectorIterCost, MaskedOverhead) =
      calculatePlanCost(MainLoopVF, MaskedModePlan, false);
  VPInstructionCost MaskedVectorCost = MaskedOverhead +
    MaskedVectorIterCost * MainLoopUF;

  if (MaskedVectorCost.isValid() && ScalarIterCost.isValid() &&
      ScalarIterCost > 0)
    MinProfitableMaskedRemTC =
        std::ceil((MaskedVectorCost / ScalarIterCost).getFloatValue());

  VPInstructionCost PumpingOvh =
      calculatePumpingOverhead(MaskedModePlan) * MainLoopUF;
  MaskedVectorCost += PumpingOvh;

  LLVM_DEBUG(dbgs() << "Remainder evaluator masked cost for VF=" << MainLoopVF
                    << " : " << MaskedVectorIterCost << " x " << MainLoopUF
                    << " + " << MaskedOverhead << " (ovh) + " << PumpingOvh
                    << " (pump ovh) = " << MaskedVectorCost
                    << "; ProfTC=" << MinProfitableMaskedRemTC << "\n";);

  // Calculate VF and the total cost for vector variant with possible scalar
  // remainder.
  calculateRemainderVFAndVectorCost();

  VPInstructionCost ScalarRemainderLoopCost = ScalarIterCost * RemainderTC;
  RemainderKind = RemainderLoopKind::Scalar;
  LoopCost = ScalarRemainderLoopCost;
  LLVM_DEBUG(dbgs() << "Remainder evaluator: scalar cost="
                    << ScalarRemainderLoopCost
                    << " masked cost=" << MaskedVectorCost
                    << " unmasked cost=" << UnmaskedVectorCost << "\n");

  if (!LoopCost.isValid()) {
    LLVM_DEBUG(
        dbgs() << "No vector remainder due to invalid scalar cost");
    return RemainderKind;
  }

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

  if (!MaskedVectorCost.isValid() && !UnmaskedVectorCost.isValid()) {
    LLVM_DEBUG(dbgs() << "Both vector costs are invalid.");
    return RemainderKind;
  }

  if (Planner.isVecRemainderEnforced()) {
    // If vector remainder is enforced choose by cost model ignoring enabling
    // switches.
    if (MaskedVectorCost.isValid() && (UnmaskedVectorCost.isInvalid() ||
                                       MaskedVectorCost < UnmaskedVectorCost)) {
      RemainderKind = RemainderLoopKind::MaskedVector;
      LoopCost = MaskedVectorCost;
    }
    else if (UnmaskedVectorCost.isValid()) {
      RemainderKind = RemainderLoopKind::VectorScalar;
      LoopCost = UnmaskedVectorCost;
      // Trip count of the second, scalar, remainder
      NewRemainderTC = RemainderTC % RemainderVF;
      // Trip count of vectorized remainder
      RemainderTC = RemainderTC / RemainderVF;
    }
    return RemainderKind;
  }
  VPInstructionCost MaskedGain = VPInstructionCost::getInvalid();
  VPInstructionCost UnmaskedGain = VPInstructionCost::getInvalid();

  auto CalculateAdjustedGain = [this](VPInstructionCost ScalarLoopCost,
                                  VPInstructionCost VecCost, int Threshold,
                                  StringRef Name) {
    VPInstructionCost AdjustedGain = Threshold;
    AdjustedGain = ScalarLoopCost * (AdjustedGain / 100.);
    VPInstructionCost Gain = ScalarLoopCost - VecCost;
    LLVM_DEBUG(dbgs() << "Adjusted " << Name << " gain=" << AdjustedGain
                      << "\n";);
    // For unknown TC, due to we assume max TC, if the gain is less than
    // adjusted one make it invalid.
    if (Gain.isValid() && TCIsUnknown && Gain < AdjustedGain) {
      LLVM_DEBUG(dbgs() << "Remainder " << Name << " gain=" << Gain
                        << "is less than thresholded (" << AdjustedGain
                        << "), discarded\n";);
      Gain = VPInstructionCost::getInvalid();
    }
    return Gain;
  };

  if (MaskedVectorCost.isValid() && EnableMaskedVectorizedRemainder) {
    MaskedGain =
        CalculateAdjustedGain(ScalarRemainderLoopCost, MaskedVectorCost,
                              MaskedGainThreshold, "masked");
  }
  if (UnmaskedVectorCost.isValid() && EnableNonMaskedVectorizedRemainder) {
    UnmaskedGain =
        CalculateAdjustedGain(ScalarRemainderLoopCost, UnmaskedVectorCost,
                              UnmaskedGainThreshold, "unmasked");
  }
  LLVM_DEBUG(dbgs() << "Remainder evaluator: masked gain=" << MaskedGain
                    << " unmasked gain=" << UnmaskedGain << "\n");

  if (MaskedGain.isValid() && MaskedGain > 0) {
    RemainderKind = RemainderLoopKind::MaskedVector;
    LoopCost = MaskedVectorCost;
  }
  if (UnmaskedGain.isValid() && UnmaskedGain > 0)
    if (!MaskedGain.isValid() || UnmaskedGain > MaskedGain) {
      RemainderKind = RemainderLoopKind::VectorScalar;
      LoopCost = UnmaskedVectorCost;
      // Trip count of the second, scalar, remainder
      NewRemainderTC = RemainderTC % RemainderVF;
      // Trip count of vectorized remainder
      RemainderTC = RemainderTC / RemainderVF;
    }
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
