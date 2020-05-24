//===- IntelVPlanAlignmentAnalysis.cpp --------------------------*- C++ -*-===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanAlignmentAnalysis.h"

#include "IntelVPlan.h"
#include "IntelVPlanUtils.h"

#include <llvm/Analysis/VectorUtils.h>

#include <numeric>

#define DEBUG_TYPE "vplan-alignment-analysis"

using namespace llvm;
using namespace llvm::vpo;

/// Modular multiplicative inverse.
static int modularMultiplicativeInverse(int Val, int Mod) {
  assert(Val > 0 && Mod > Val && "Invalid Arguments");
  assert(greatestCommonDivisor(Val, Mod) == 1 && "Arguments are not coprime");

  int Prev = 1;
  int Curr = Val;

  // Given coprime Val and Mod (Mod > Val > 0), there exists n such that
  // Val^n ≡ 1 (mod Mod).
  while (Curr != 1) {
    Prev = Curr;
    Curr = (Curr * Val) % Mod;
  }

  // Return Val^{n - 1}.
  return Prev;
}

/// Compute -a^{-1} (mod b),
/// where a = InductionStep / RequiredAlignment
///       b = TargetAlignment / RequiredAlignment
static int computeMultiplierForDynamicPeeling(int Step, Align RequiredAlignment,
                                              Align TargetAlignment) {
  assert(TargetAlignment > RequiredAlignment &&
         "Peeling makes sense only if TargetAlignment > RequiredAlignment");
  assert(Step % RequiredAlignment.value() == 0 && "Bad RequiredAlignment");

  int ReqLog = Log2(RequiredAlignment);
  int TgtLog = Log2(TargetAlignment);
  int A = Step >> ReqLog;
  int B = 1 << (TgtLog - ReqLog);
  int InvA = modularMultiplicativeInverse(A % B, B);
  return B - InvA;
}

VPlanPeelingVariant::~VPlanPeelingVariant() {}

VPlanDynamicPeeling::VPlanDynamicPeeling(VPInstruction *Memref,
                                         VPConstStepInduction AccessAddress,
                                         Align TargetAlignment)
    : VPlanPeelingVariant(VPPK_DynamicPeeling), Memref(Memref),
      InvariantBase(AccessAddress.InvariantBase),
      RequiredAlignment(MinAlign(0, AccessAddress.Step)),
      TargetAlignment(TargetAlignment),
      Multiplier(computeMultiplierForDynamicPeeling(
          AccessAddress.Step, RequiredAlignment, TargetAlignment)) {}

int VPlanPeelingCostModelSimple::getCost(VPInstruction *Mrf, int VF,
                                         Align Alignment) {
  auto Size = DL->getTypeAllocSize(getLoadStoreType(Mrf));
  Align BestAlign(VF * MinAlign(0, Size));
  auto Opcode = Mrf->getOpcode();

  if (Alignment >= BestAlign)
    return 0;

  if (Opcode == Instruction::Load)
    return 2;

  if (Opcode == Instruction::Store)
    return 3;

  llvm_unreachable("Unexpected Opcode");
}

VPlanPeelingCandidate::VPlanPeelingCandidate(VPInstruction *Memref,
                                             VPConstStepInduction AccessAddress)
    : Memref(Memref), AccessAddress(AccessAddress) {
#ifndef NDEBUG
  auto *DL = Memref->getParent()->getParent()->getDataLayout();
  auto AccessSize = DL->getTypeAllocSize(getLoadStoreType(Memref));
  auto AccessStep = AccessAddress.Step;
  assert(AccessSize == TypeSize::Fixed(AccessStep) &&
         "Non-unit stride memory access");
#endif
}

void VPlanPeelingAnalysis::collectMemrefs(VPlan &Plan) {
  for (auto &VPBB : Plan)
    for (auto &VPInst : VPBB) {
      auto Opcode = VPInst.getOpcode();
      if (Opcode != Instruction::Load && Opcode != Instruction::Store)
        continue;

      auto *Pointer = getLoadStorePointerOperand(&VPInst);
      auto *Expr = VPSE->getVPlanSCEV(*Pointer);
      Optional<VPConstStepInduction> Ind = VPSE->asConstStepInduction(Expr);

      // Skip if access address is not an induction variable.
      if (!Ind)
        continue;

      // Skip accesses that are not unit-strided.
      Type *EltTy = cast<PointerType>(Pointer->getType())->getElementType();
      if (DL->getTypeAllocSize(EltTy) != TypeSize::Fixed(Ind->Step))
        continue;

      CandidateMemrefs.push_back({&VPInst, *Ind});
    }
}

std::unique_ptr<VPlanPeelingVariant>
VPlanPeelingAnalysis::selectBestPeelingVariant(int VF) {
  auto Static = selectBestStaticPeelingVariant(VF);
  auto DynamicOrNone = selectBestDynamicPeelingVariant(VF);
  if (DynamicOrNone && DynamicOrNone->second > Static.second)
    return std::make_unique<VPlanDynamicPeeling>(DynamicOrNone->first);
  return std::make_unique<VPlanStaticPeeling>(Static.first);
}

std::pair<VPlanStaticPeeling, int>
VPlanPeelingAnalysis::selectBestStaticPeelingVariant(int VF) {
  return {VPlanStaticPeeling(0), 0};
}

Optional<std::pair<VPlanDynamicPeeling, int>>
VPlanPeelingAnalysis::selectBestDynamicPeelingVariant(int VF) {
  if (CandidateMemrefs.empty())
    return None;

  // Map every collected memref to {Peeling, Profit} pair.
  auto Map = map_range(
      CandidateMemrefs,
      [this, VF](auto &Cand) -> std::pair<VPlanDynamicPeeling, int> {
        Align ReqAlign(MinAlign(0, Cand.accessAddress().Step));
        int Profit = CM->getCost(Cand.memref(), VF, ReqAlign) -
                     CM->getCost(Cand.memref(), VF, ReqAlign * VF);
        VPlanDynamicPeeling Peeling(Cand.memref(), Cand.accessAddress(),
                                    ReqAlign * VF);
        return {std::move(Peeling), Profit};
      });

  // Select the most profitable peeling variant in Map.
  auto Reduce = std::accumulate(Map.begin() + 1, Map.end(), *Map.begin(),
                                [](const auto &lhs, const auto &rhs) {
                                  return rhs.second > lhs.second ? rhs : lhs;
                                });

  return Reduce;
}
