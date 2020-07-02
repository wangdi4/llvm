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
#include "IntelVPlanValueTracking.h"

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
static int computeMultiplierForPeeling(int Step, Align RequiredAlignment,
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
      Multiplier(computeMultiplierForPeeling(
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
                                             VPConstStepInduction AccessAddress,
                                             KnownBits InvariantBaseKnownBits)
    : Memref(Memref), AccessAddress(AccessAddress),
      InvariantBaseKnownBits(std::move(InvariantBaseKnownBits)) {
#ifndef NDEBUG
  auto *DL = Memref->getParent()->getParent()->getDataLayout();
  auto AccessSize = DL->getTypeAllocSize(getLoadStoreType(Memref));
  auto AccessStep = AccessAddress.Step;
  assert(AccessSize == TypeSize::Fixed(AccessStep) &&
         "Non-unit stride memory access");
  assert((InvariantBaseKnownBits.One & (MinAlign(0, AccessStep) - 1)) == 0 &&
         "Misaligned memory access");
#endif
}

void VPlanPeelingAnalysis::collectMemrefs(VPlan &Plan) {
  collectCandidateMemrefs(Plan);
  computeCongruentMemrefs();
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
  // We are going to compute profit for every possible static peel count and
  // select the most profitable one. The peel count can be any number in
  // [0...VF) interval. PeelCountProfit is a zero-initialized array of profits
  // for every possible peel count.
  std::vector<int> PeelCountProfit(VF);

  for (VPlanPeelingCandidate &Cand : CandidateMemrefs) {
    VPInstruction *Memref = Cand.memref();
    auto Step = Cand.accessAddress().Step;
    const KnownBits &KB = Cand.invariantBaseKnownBits();

    // How many lower bits are known.
    auto Known = (KB.Zero | KB.One).countTrailingOnes();

    // Required initial alignment.
    Align ReqAlign(MinAlign(0, Step));

    // Nothing can be done if there's not enough known bits.
    if (Known <= Log2(ReqAlign))
      continue;

    // Initial cost of the memory access.
    int CostBasis = CM->getCost(Memref, VF, ReqAlign);

    // How much we can improve alignment of the memref.
    int MaxExtraBits = std::min<int>(Known - Log2(ReqAlign), Log2_32(VF));

    // The best alignment that can be achieved is (VF * ReqAlign). However, the
    // algorithm below tries smaller alignments as well (starting from
    // 2 * ReqAlign). The smaller alignments may be beneficial as well and may
    // allow to align more memrefs simultaneously.
    //
    // The algorithm computes profit incrementally. It starts with 2*ReqAlign
    // (ExtraBits = 1), continues with 4*ReqAlign (ExtraBits = 2) and so on up
    // to VF*ReqAlign. At each step, the algorithm computes profit relative to
    // the previous step. As an example, let's assume that Memref is an access
    // to i32, that VF = 8, and that the access to <8 x i32> would be aligned
    // by 32 with peel count = 3. Then, PeelCountProfit[3] will be updated in
    // three steps:
    //
    //   ExtraBits = 1 (TgtAlign = 8):
    //     PeelCountProfit[3] += cost(align = 4) - cost(align = 8)
    //
    //   ExtraBits = 2 (TgtAlign = 16):
    //     PeelCountProfit[3] += cost(align = 8) - cost(align = 16)
    //
    //   ExtraBits = 3 (TgtAlign = 32):
    //     PeelCountProfit[3] += cost(align = 16) - cost(align = 32)
    //
    // Obviously, if we combine all the updates, the net result is as expected:
    //
    //   PeelCountProfit[3] += cost(align = 4) - cost(align = 32)
    //
    for (int ExtraBits = 1; ExtraBits <= MaxExtraBits; ++ExtraBits) {
      Align TgtAlign = ReqAlign * (1ULL << ExtraBits);
      int NewCost = CM->getCost(Memref, VF, TgtAlign);

      // Check if the new alignment is beneficial at all.
      if (NewCost == CostBasis)
        continue;

      // Compute the difference between the previous and the new costs. Update
      // the cost basis.
      auto Profit = CostBasis - NewCost;
      assert(Profit > 0 && "Broken cost model");
      CostBasis = NewCost;

      // Compute the peel count to make the memref aligned by TgtAlign.
      auto Multiplier = computeMultiplierForPeeling(Step, ReqAlign, TgtAlign);
      assert(KB.One.getLoBits(Log2(ReqAlign)) == 0 &&
             "Misaligned memory access in CandidateMemrefs");
      auto Quotient = KB.One.lshr(Log2(ReqAlign)).getZExtValue();
      auto MinPeelCount = (Quotient * Multiplier) & ((1 << ExtraBits) - 1);

      // MinPeelCount is the smallest peel count to achieve the new alignment,
      // but it may be not the only one in [0...VF) interval. For example,
      // for access to i32 with base_address ≡ 20 (mod 32) and VF = 8:
      //
      //   ExtraBits = 1 (TgtAlign = 8):
      //     MinPeelCount = 1;
      //     PeelCountProfit[1] += cost(align = 4) - cost(align = 8);
      //     PeelCountProfit[3] += cost(align = 4) - cost(align = 8);
      //     PeelCountProfit[5] += cost(align = 4) - cost(align = 8);
      //     PeelCountProfit[7] += cost(align = 4) - cost(align = 8);
      //
      //   ExtraBits = 2 (TgtAlign = 16):
      //     MinPeelCount = 3;
      //     PeelCountProfit[3] += cost(align = 8) - cost(align = 16);
      //     PeelCountProfit[7] += cost(align = 8) - cost(align = 16);
      //
      //   ExtraBits = 3 (TgtAlign = 32):
      //     MinPeelCount = 3;
      //     PeelCountProfit[3] += cost(align = 16) - cost(align = 32);
      //
      for (int PC = MinPeelCount; PC < VF; PC += (1 << ExtraBits))
        PeelCountProfit[PC] += Profit;
    }
  }

  auto Iter = std::max_element(PeelCountProfit.begin(), PeelCountProfit.end());
  int BestPeelCount = std::distance(PeelCountProfit.begin(), Iter);
  int MaxProfit = *Iter;
  return { VPlanStaticPeeling(BestPeelCount), MaxProfit };
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
        int CostBasis = 0, CostAlign = 0;
        CostBasis += CM->getCost(Cand.memref(), VF, ReqAlign);
        CostAlign += CM->getCost(Cand.memref(), VF, ReqAlign * VF);
        for (auto &Congr : CongruentMemrefs[Cand.memref()]) {
          CostBasis += CM->getCost(Congr.first, VF, ReqAlign);
          auto TgtAlign = std::min(ReqAlign * VF, Congr.second);
          CostAlign += CM->getCost(Congr.first, VF, TgtAlign);
        }
        int Profit = CostBasis - CostAlign;
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

void VPlanPeelingAnalysis::collectCandidateMemrefs(VPlan &Plan) {
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

      KnownBits KB = VPVT->getKnownBits(Ind->InvariantBase, &VPInst);

      // Skip the memref if the address is statically known to be misaligned.
      auto RequiredAlignment = MinAlign(0, Ind->Step);
      if ((KB.One & (RequiredAlignment - 1)) != 0)
        continue;

      CandidateMemrefs.push_back({&VPInst, *Ind, std::move(KB)});
    }

  sort(CandidateMemrefs, VPlanPeelingCandidate::ordByStep);
}

void VPlanPeelingAnalysis::computeCongruentMemrefs() {
  assert(is_sorted(CandidateMemrefs, VPlanPeelingCandidate::ordByStep) &&
         "CandidateMemrefs is not sorted");

  // Initialize CongruentMemrefs with empty lists.
  CongruentMemrefs.reserve(CandidateMemrefs.size());
  for (auto &Cand : CandidateMemrefs)
    CongruentMemrefs[Cand.memref()] = {};

  // Group memrefs together by access step, and process each group separately.
  auto StepBegin = CandidateMemrefs.cbegin();
  while (StepBegin != CandidateMemrefs.cend()) {
    auto Step = StepBegin->accessAddress().Step;
    auto StepEnd =
        std::upper_bound(StepBegin, CandidateMemrefs.cend(), *StepBegin,
                         VPlanPeelingCandidate::ordByStep);

    for (auto Cand = StepBegin; Cand != StepEnd; ++Cand) {
      for (auto Prev = StepBegin; Prev != Cand; ++Prev) {
        auto CandBase = Cand->accessAddress().InvariantBase;
        auto PrevBase = Prev->accessAddress().InvariantBase;
        auto Diff = VPSE->getMinusExpr(CandBase, PrevBase);
        auto KB = VPVT->getKnownBits(Diff, nullptr);

        // "Alignment" of the pointer difference determines how much alignment
        // can be transferred from one memref to another. That is, as long as
        // alignment of Mrf1 is not greater than A, alignment of Mrf2 is going
        // to be equal to alignment of Mrf1. Though, if alignment of Mrf1 is
        // greater than A, Mrf2 is going to be aligned by A only.
        Align A(1ULL << KB.Zero.countTrailingOnes());
        if (A > MinAlign(0, Step)) {
          CongruentMemrefs[Cand->memref()].emplace_back(Prev->memref(), A);
          CongruentMemrefs[Prev->memref()].emplace_back(Cand->memref(), A);
        }
      }
    }

    StepBegin = StepEnd;
  }
}
