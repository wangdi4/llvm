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
#include "IntelVPlanCostModel.h"
#include "IntelVPlanUtils.h"
#include "IntelVPlanValueTracking.h"

#include <llvm/Analysis/VectorUtils.h>

#include <numeric>

#define DEBUG_TYPE "vplan-alignment-analysis"

static cl::opt<bool>
    ForceDynAlignment("vplan-force-dyn-alignment", cl::init(false), cl::Hidden,
                      cl::desc("Force dynamic peeling for alignment."));

using namespace llvm;
using namespace llvm::vpo;

/// Modular multiplicative inverse.
static int modularMultiplicativeInverse(int Val, int Mod) {
  assert(Val > 0 && Mod > Val && "Invalid Arguments");
  assert(std::gcd(Val, Mod) == 1 && "Arguments are not coprime");

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

VPlanStaticPeeling VPlanStaticPeeling::NoPeelLoop{0};

VPlanDynamicPeeling::VPlanDynamicPeeling(VPLoadStoreInst *Memref,
                                         VPConstStepInduction AccessAddress,
                                         Align TargetAlignment)
    : VPlanPeelingVariant(VPPK_DynamicPeeling), Memref(Memref),
      InvariantBase(AccessAddress.InvariantBase),
      RequiredAlignment(MinAlign(0, AccessAddress.Step)),
      TargetAlignment(TargetAlignment),
      Multiplier(computeMultiplierForPeeling(
          AccessAddress.Step, RequiredAlignment, TargetAlignment)) {}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPlanDynamicPeeling::print(raw_ostream &OS) const {
  OS << "VPlanDynamicPeeling: \n"
     << '\t' << "Memref: ";
  if (Memref)
    Memref->dump();
  else
    OS << "nullptr\n";
  OS << '\t' << "InvariantBase: ";
  // VPlanSCEV is an opaque pointer. Just print the hex value.
  // During the debugging pointer can be casted and explored further.
  (OS << "0x").write_hex(reinterpret_cast<size_t>(InvariantBase)) << '\n';
  OS << '\t' << "RequiredAlignment: " << RequiredAlignment.value() << '\n'
     << '\t' << "TargetAlignment: " << TargetAlignment.value() << '\n'
     << '\t' << "Multiplier: " << Multiplier << '\n';
}
#endif

VPInstructionCost VPlanPeelingCostModelSimple::getCost(
  VPLoadStoreInst *Mrf, int VF, Align Alignment) {
  auto Size = DL->getTypeAllocSize(Mrf->getValueType());
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

VPInstructionCost VPlanPeelingCostModelGeneral::getCost(
  VPLoadStoreInst *Mrf, int VF, Align Alignment) {
  return CM->getLoadStoreCost(Mrf, Alignment, VF);
}

VPlanPeelingCandidate::VPlanPeelingCandidate(VPLoadStoreInst *Memref,
                                             VPConstStepInduction AccessAddress,
                                             KnownBits InvariantBaseKnownBits)
    : Memref(Memref), AccessAddress(AccessAddress),
      InvariantBaseKnownBits(std::move(InvariantBaseKnownBits)) {
#ifndef NDEBUG
  auto *DL = Memref->getParent()->getParent()->getDataLayout();
  auto AccessSize = DL->getTypeAllocSize(Memref->getValueType());
  auto AccessStep = AccessAddress.Step;
  assert(AccessSize == TypeSize::Fixed(AccessStep) &&
         "Non-unit stride memory access");
  assert(
    (this->InvariantBaseKnownBits.One & (MinAlign(0, AccessStep) - 1)) == 0 &&
    "Misaligned memory access");
#endif
}

void VPlanPeelingAnalysis::collectMemrefs(VPlanVector &Plan) {
  collectCandidateMemrefs(Plan);
  computeCongruentMemrefs();
  LLVM_DEBUG(dump());
}

std::unique_ptr<VPlanPeelingVariant>
VPlanPeelingAnalysis::selectBestPeelingVariant(int VF,
                                               VPlanPeelingCostModel &CM,
                                               bool EnableDynamic) {
  auto Static = selectBestStaticPeelingVariant(VF, CM);
  if (EnableDynamic) {
    auto DynamicOrNone = selectBestDynamicPeelingVariant(VF, CM);
    if (DynamicOrNone &&
        (ForceDynAlignment || DynamicOrNone->second > Static.second))
      return std::make_unique<VPlanDynamicPeeling>(DynamicOrNone->first);
  }
  return std::make_unique<VPlanStaticPeeling>(Static.first);
}

std::pair<VPlanStaticPeeling, VPInstructionCost>
VPlanPeelingAnalysis::selectBestStaticPeelingVariant(
    int VF, VPlanPeelingCostModel &CM) {
  // We are going to compute profit for every possible static peel count and
  // select the most profitable one. The peel count can be any number in
  // [0...VF) interval. PeelCountProfit is a zero-initialized array of profits
  // for every possible peel count.
  std::vector<VPInstructionCost> PeelCountProfit(VF);

  for (VPlanPeelingCandidate &Cand : CandidateMemrefs) {
    VPLoadStoreInst *Memref = Cand.memref();
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
    VPInstructionCost CostBasis = CM.getCost(Memref, VF, ReqAlign);

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
      VPInstructionCost NewCost = CM.getCost(Memref, VF, TgtAlign);

      // Check if the new alignment is beneficial at all.
      if (NewCost == CostBasis)
        continue;

      // Compute the difference between the previous and the new costs. Update
      // the cost basis.
      auto Profit = CostBasis - NewCost;
      assert(Profit.isValid() && Profit > 0 && "Broken cost model");
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

  LLVM_DEBUG(dbgs() << "Static peeling selection (" << VF
                    << " items), profit by offsets: \n";
             for (size_t P = 0; P < PeelCountProfit.size(); P++)
               dbgs() << " " << P << "->" << PeelCountProfit[P];
             dbgs() << "\n";);

  auto Iter = std::max_element(PeelCountProfit.begin(), PeelCountProfit.end());
  int BestPeelCount = std::distance(PeelCountProfit.begin(), Iter);
  auto MaxProfit = *Iter;
  return { VPlanStaticPeeling(BestPeelCount), MaxProfit };
}

Optional<std::pair<VPlanDynamicPeeling, VPInstructionCost>>
VPlanPeelingAnalysis::selectBestDynamicPeelingVariant(
    int VF, VPlanPeelingCostModel &CM) {
  if (CandidateMemrefs.empty())
    return None;

  // Map every collected memref to {Peeling, Profit} pair.
  auto Map = map_range(
      CandidateMemrefs,
      [this, VF, &CM](auto &Cand) ->
      std::pair<VPlanDynamicPeeling, VPInstructionCost> {
        Align ReqAlign(MinAlign(0, Cand.accessAddress().Step));
        VPInstructionCost CostBasis = 0, CostAlign = 0;
        CostBasis += CM.getCost(Cand.memref(), VF, ReqAlign);
        CostAlign += CM.getCost(Cand.memref(), VF, ReqAlign * VF);
        for (auto &Congr : CongruentMemrefs[Cand.memref()]) {
          CostBasis += CM.getCost(Congr.first, VF, ReqAlign);
          auto TgtAlign = std::min(ReqAlign * VF, Congr.second);
          CostAlign += CM.getCost(Congr.first, VF, TgtAlign);
        }
        VPInstructionCost Profit = CostBasis - CostAlign;
        VPlanDynamicPeeling Peeling(Cand.memref(), Cand.accessAddress(),
                                    ReqAlign * VF);
        return {std::move(Peeling), Profit};
      });

  LLVM_DEBUG(dbgs() << "Dynamic peeling selection ("
                    << std::distance(Map.begin(), Map.end()) << " items)\n";
             for (auto P = Map.begin(); P != Map.end(); P++) {
               (*P).first.memref()->printWithoutAnalyses(dbgs());
               dbgs() << " profit=" << (*P).second << "\n";
             });

  // Select the most profitable peeling variant in Map.
  auto Reduce = std::accumulate(Map.begin() + 1, Map.end(), *Map.begin(),
                                [](const auto &lhs, const auto &rhs) {
                                  return rhs.second > lhs.second ? rhs : lhs;
                                });

  return Reduce;
}

void VPlanPeelingAnalysis::collectCandidateMemrefs(VPlanVector &Plan) {
  for (auto &VPBB : Plan)
    for (auto &VPInst : VPBB) {
      auto *LS = dyn_cast<VPLoadStoreInst>(&VPInst);
      if (!LS)
        continue;

      auto *Expr = LS->getAddressSCEV();
      Optional<VPConstStepInduction> Ind = VPSE->asConstStepInduction(Expr);

      // Skip if access address is not an induction variable.
      if (!Ind)
        continue;

      // Skip accesses that are not unit-strided.
      Type *EltTy = LS->getValueType();
      if (DL->getTypeAllocSize(EltTy) != TypeSize::Fixed(Ind->Step))
        continue;

      KnownBits KB = VPVT->getKnownBits(Ind->InvariantBase, LS);

      // Skip the memref if the address is statically known to be misaligned.
      auto RequiredAlignment = MinAlign(0, Ind->Step);
      if ((KB.One & (RequiredAlignment - 1)) != 0)
        continue;

      CandidateMemrefs.push_back({LS, *Ind, std::move(KB)});
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
        // If bases are the same, then no need to record this in
        // CongruentMemrefs. Also, attempting to propagate alignment will
        // cause an assert in Align() constructor because Diff KnownBits
        // Zero will be all ones and the shift will result in Align(0).
       if (CandBase == PrevBase)
         continue;
        auto Diff = VPSE->getMinusExpr(CandBase, PrevBase);
        if (!Diff)
          continue;
        auto KB = VPVT->getKnownBits(Diff, nullptr);

        // We can have a case especially in HIR, where the base canon expressions
        // coming from different memory references are different but are
        // equivalent. For example the base CE from the load and store below:
        //   a[i] = a[i] + 1
        // Treat this case the same as CandBase == PrevBase.
        if (KB.isZero())
          continue;

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

void VPlanPeelingAnalysis::dump() {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  dbgs() << "\nVPlanPeelingAnalysis Candidate Memrefs:\n";

  for (auto &Mrf : CandidateMemrefs) {
    // Print the memref.
    dbgs() << '[' << Mrf.memref() << "] ";
    Mrf.memref()->printWithoutAnalyses(dbgs());

    // Print 8 low (known) bits.
    dbgs() << " | KB = 0b";
    const KnownBits &KB = Mrf.invariantBaseKnownBits();
    for (int i = 7; i >= 0; --i) {
      if (KB.One[i])
        dbgs() << '1';
      else if (KB.Zero[i])
        dbgs() << '0';
      else
        dbgs() << '?';
    }
    dbgs() << '\n';

    // Print congruent memrefs.
    auto &CongruentList = CongruentMemrefs[Mrf.memref()];
    for (auto &Pair : CongruentList) {
      dbgs() << "  -> [" << Pair.first << "] ";
      Pair.first->printWithoutAnalyses(dbgs());
      dbgs() << " (A" << Pair.second.value() << ")\n";
    }
    if (CongruentList.empty())
      dbgs() << "  (none)\n";
  }
#endif
}

Align VPlanAlignmentAnalysis::getAlignmentUnitStride(
    const VPLoadStoreInst &Memref, VPlanPeelingVariant *Peeling) const {
  if (!Peeling || VF == 1)
    return Memref.getAlignment();
  if (auto *SP = dyn_cast<VPlanStaticPeeling>(Peeling))
    return getAlignmentUnitStrideImpl(Memref, *SP);
  if (auto *DP = dyn_cast<VPlanDynamicPeeling>(Peeling))
    return getAlignmentUnitStrideImpl(Memref, *DP);
  llvm_unreachable("Unsupported peeling variant");
}

Align VPlanAlignmentAnalysis::getAlignmentUnitStrideImpl(
    const VPLoadStoreInst &Memref, VPlanStaticPeeling &SP) const {
  Align AlignFromIR = Memref.getAlignment();

  auto Ind = VPSE->asConstStepInduction(Memref.getAddressSCEV());
  if (!Ind)
    return AlignFromIR;

  // FIXME: In case of reverse iteration, we should compute alignment of
  //        the pointer in the last lane.
  if (Ind->Step <= 0)
    return AlignFromIR;

  auto BaseKB = VPVT->getKnownBits(Ind->InvariantBase, &Memref);
  auto NumKnownLowBits = (BaseKB.One | BaseKB.Zero).countTrailingOnes();
  uint64_t Mask = ~0ULL << NumKnownLowBits;

  auto Offset = SP.peelCount() * Ind->Step;
  auto AdjustedBase = (BaseKB.One + Offset) | Mask;
  Align AlignFromVPVT{1ULL << AdjustedBase.countTrailingZeros()};

  // Alignment of access cannot be larger than alignment of the step, which
  // equals to (VF * Step) for widened memrefs.
  Align AlignFromStep{MinAlign(0, VF * Ind->Step)};

  // Generally, the resulting alignment is equal to AlignFromVPVT. However, it
  // cannot be lower than AlignFromIR and higher than AlignFromStep.
  return std::min(AlignFromStep, std::max(AlignFromIR, AlignFromVPVT));
}

Align VPlanAlignmentAnalysis::getAlignmentUnitStrideImpl(
    const VPLoadStoreInst &Memref, VPlanDynamicPeeling &DP) const {
  Align AlignFromIR = Memref.getAlignment();

  VPlanSCEV *DstScev = Memref.getAddressSCEV();
  auto DstInd = VPSE->asConstStepInduction(DstScev);
  if (!DstInd)
    return AlignFromIR;

  VPlanSCEV *SrcScev = DP.memref()->getAddressSCEV();
  auto SrcInd = VPSE->asConstStepInduction(SrcScev);
  assert(SrcInd && "Dynamic peeling on a non-inductive address?");

  auto Step = DstInd->Step;
  // Dynamic peeling won't help if memrefs have different steps.
  if (SrcInd->Step != Step)
    return AlignFromIR;

  VPlanSCEV *Diff = VPSE->getMinusExpr(DstScev, SrcScev);
  // If we are unable to compute the diff, there is not much that we can
  // do. Use alignment from IR.
  if (!Diff)
    return AlignFromIR;

  auto KB = VPVT->getKnownBits(Diff, &Memref);
  if (KB.isZero()) {
    // The memref is congruently equal to the peeling base. Return targeted
    // peeling alignment. Note: the steps equality is checked above.
    return DP.targetAlignment();
  }

  Align AlignFromDiff{1ULL << KB.countMinTrailingZeros()};

  // Alignment of access cannot be larger than alignment of the step, which
  // equals to (VF * Step) for widened memrefs.
  Align AlignFromStep{MinAlign(0, VF * Step)};

  // Generally, the resulting alignment is equal to AlignFromDiff capped by
  // peeling target alignment. However, it cannot be lower than AlignFromIR and
  // higher than AlignFromStep.
  Align AlignFromDiffCapped = std::min(AlignFromDiff, DP.targetAlignment());
  return std::min(AlignFromStep, std::max(AlignFromIR, AlignFromDiffCapped));
}
