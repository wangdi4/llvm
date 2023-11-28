//===-- IntelVPlanCostModelHeuristics.cpp ---------------------------------===//
//
//   Copyright (C) 2021 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements Heuristics for VPlan cost modeling.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlan.h"
#include "IntelVPlanCostModel.h"
#include "IntelVPlanCostModelHeuristics.h"
#include "IntelVPlanIdioms.h"
#include "IntelVPlanLoopUnroller.h"
#include "IntelVPlanPatternMatch.h"
#include "IntelVPlanSLP.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"

#include <numeric>

#define DEBUG_TYPE "vplan-cost-model-heuristics"

#if INTEL_FEATURE_SW_ADVANCED

using namespace loopopt;
using namespace llvm::PatternMatch;

static cl::opt<unsigned> NumberOfSpillsPerExtraReg(
    "vplan-cost-model-number-of-spills-per-extra-reg", cl::init(2), cl::Hidden,
    cl::desc("The number of spills/fills generated on average for each HW "
             "register spilled and restored."));

// These are two experimentally set thresholds we use by default for desktop
// and HPC programs.
// TODO: There is no fundamental reason why those thresholds can not be the
// same. We might want to tune CM to match them.
// These threholds apply when estimating load/store port pressure, i.e. for
// hardware gather and scatter instructions.
// TODO: We might want to have separate code and threshold for estimating
// load and store ports pressure independently.
unsigned const CMGatherScatterDefaultThreshold = 50;
unsigned const CMGatherScatterDefaultThresholdZMM = 70;
// Separate setting for software gather/scatter intructions as we estimate
// shuffle port pressure for them.
unsigned const CMSWGatherScatterDefaultThreshold = 60;

static cl::opt<unsigned> CMGatherScatterThreshold(
  "vplan-cm-gather-scatter-threshold",
  cl::init(CMGatherScatterDefaultThreshold),
  cl::desc("If HW gather/scatter cost is more than CMGatherScatterThreshold "
           "percent of whole loop price the price of gather/scatter is "
           "doubled to make it harder to choose in favor of "
           "loop with gathers/scatters."));

static cl::opt<unsigned> CMSWGatherScatterThreshold(
  "vplan-cm-sw-gather-scatter-threshold",
  cl::init(CMSWGatherScatterDefaultThreshold),
  cl::desc("If SW gather/scatter cost is more than CMGatherScatterThreshold "
           "percent of whole loop price the price of gather/scatter is "
           "doubled to make it harder to choose in favor of "
           "loop with gathers/scatters."));

static cl::opt<unsigned> CMGatherScatterPenaltyFactor(
  "vplan-cm-gather-scatter-penalty-factor", cl::init(2), cl::Hidden,
  cl::desc("The factor which G/S cost multiplies by if G/S accumulated cost "
           "exceeds CMGatherScatterThreshold."));

static cl::opt<bool> UseOVLSCM(
  "vplan-cm-use-ovlscm", cl::init(true),
  cl::desc("Consider cost returned by OVLSCostModel "
           "for optimized gathers and scatters."));

static cl::opt<bool> VPlanPhiPumping(
    "vplan-phi-pumping", cl::init(true), cl::Hidden,
    cl::desc("Account for PHI pumping in counting PHI registers."));

// Control for all unrolling heuristics.
static cl::opt<bool> CMUseUnrollHeuristics(
    "vplan-cm-unroll", cl::init(true), cl::Hidden,
    cl::desc("Enable unrolling heuristic for VPlan cost model"));

// Limit the unrolling heuristic to only apply in the presence
// of partial sum reductions. This is the initial default to
// conservatively limit the scope of unrolling.
static cl::opt<bool> CMUnrollPartialSumsOnly(
    "vplan-cm-unroll-partial-sums-only", cl::init(true), cl::Hidden,
    cl::desc("Limit unroll heuristics to partial sum candidate loops only"));

// Override the VectorUnrollingPreferences PartialSumILPScore value
static cl::opt<float> CMUnrollILPScore(
    "vplan-cm-unroll-ilp-score", cl::init(1.f), cl::Hidden,
    cl::desc("Override the target ILP score for partial sum reductions"));

static cl::opt<bool> CMScalarSLPAnalysis(
    "vplan-cm-scalar-slp-analysis", cl::init(true), cl::Hidden,
    cl::desc(
        "Enables SLP analysis over scalar VPlan IR"));

static cl::opt<bool> CMUseSLPPatternHeuristics(
    "vplan-cm-use-slp-pattern-heuristics", cl::init(true), cl::Hidden,
    cl::desc("Allows CM to use SLP pattern detecting heuristics"));

static cl::opt<bool> CMUseSLPRedPatternHeuristics(
    "vplan-cm-use-slp-red-pattern-heuristics", cl::init(true), cl::Hidden,
    cl::desc("Allows CM to use SLP reduction pattern detecting heuristics"));

namespace llvm {

namespace vpo {

namespace VPlanCostModelHeuristics {

HeuristicBase::HeuristicBase(VPlanTTICostModel *CM, std::string Name) :
  CM(CM) {
  Plan = CM->Plan;
  VF = CM->VF;
  UF = CM->UF;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  this->Name = Name;
#else
  (void)Name;
#endif // !NDEBUG || LLVM_ENABLE_DUMP
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// VPlan scope format.
void HeuristicBase::printCostChange(
  const VPInstructionCost &RefCost, const VPInstructionCost &NewCost,
  const VPlan *, raw_ostream *OS) const {
  if (!OS || NewCost == RefCost)
    return;

  if (!NewCost.isValid())
    *OS << "Cost is set to " << NewCost << " by " << getName()
        << " heuristic\n";
  else if (NewCost > RefCost)
    *OS << "Extra cost due to " << getName() << " heuristic is "
        << NewCost - RefCost << '\n';
  else
    *OS << "Cost decrease due to " << getName() << " heuristic is "
        << RefCost - NewCost << '\n';
}

// VPInstruction scope format:
//  *name*(+num)
// OR:
//  *name*(-num)
void HeuristicBase::printCostChange(
  const VPInstructionCost &RefCost, const VPInstructionCost &NewCost,
  const VPInstruction *, raw_ostream *OS) const {
  if (!OS || NewCost == RefCost)
    return;

  *OS << " *" << getName() << "*(" <<
    (NewCost - RefCost).toString(true /* ForceSignPrint */) << ')';
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

const RegDDRef* HeuristicSLP::getHIRMemref(
  const VPInstruction *VPInst) {
  unsigned Opcode = VPInst->getOpcode();
  if (Opcode != Instruction::Load && Opcode != Instruction::Store)
    return nullptr;

  if (!VPInst->HIR().isMaster())
    return nullptr;
  auto *HInst = dyn_cast<HLInst>(VPInst->HIR().getUnderlyingNode());

  if (!HInst)
    return nullptr;

  return Opcode == Instruction::Load ? HInst->getOperandDDRef(1) :
                                       HInst->getLvalDDRef();
}

bool HeuristicSLP::findSLPHIRPattern(
  SmallVectorImpl<const RegDDRef*> &HIRMemrefs, unsigned PatternSize) {

  if (HIRMemrefs.size() < PatternSize)
    return false;

  const RegDDRef* baseDDref = HIRMemrefs.back();
  HIRMemrefs.pop_back();

  unsigned elSize = baseDDref->getDestTypeSizeInBytes();
  // maintain array of booleans each element of which is indicating
  // presence of memref with following offsets:
  // [ -3*elSize, -2*elSize, -1*elSize, 0, elSize, 2*elSize, 3*elSize ]
  // The middle element is always true, which corresponds to baseDDref.
  bool offsets[7] = {false, false, false, true, false, false, false};

  for(auto HIRMemref : HIRMemrefs) {
    int64_t distance = 0;
    if (HIRMemref->getDestTypeSizeInBytes() == elSize &&
        DDRefUtils::getConstByteDistance(baseDDref, HIRMemref, &distance) &&
        (distance % elSize) == 0) {
      int idx = distance / elSize + 3;
      if (idx >= 0 && static_cast<unsigned>(idx) < sizeof(offsets))
        offsets[idx] = true;
    }
  }

  // Check if any of PatternSize consecutive elements in offsets are true.
  unsigned cntTrue = 0;
  for (unsigned i = 0; i < sizeof(offsets); i++) {
    if (offsets[i]) {
      cntTrue++;
      if (cntTrue >= PatternSize)
        break;
    }
    else
      cntTrue = 0;
  }

  if (cntTrue >= PatternSize)
    return true;

  // Analyze remaining memrefs.
  return findSLPHIRPattern(HIRMemrefs, PatternSize);
}

bool HeuristicSLP::ProcessSLPHIRMemrefs(
  SmallVectorImpl<const RegDDRef*> const &HIRMemrefs, unsigned PatternSize) {

  unsigned WindowStartIndex = 0;
  bool PatternFound = false;
  do {
    // Prepare vector of VPlanSLPSearchWindowSize or less for further
    // processing.
    SmallVector<const RegDDRef*, VPlanSLPSearchWindowSize> HIRMemrefsTmp;
    for (unsigned i = WindowStartIndex;
         (i < (WindowStartIndex + VPlanSLPSearchWindowSize)) &&
           (i < HIRMemrefs.size()); i++)
      HIRMemrefsTmp.push_back(HIRMemrefs[i]);

    // Break out early if a pattern is found.
    if (findSLPHIRPattern(HIRMemrefsTmp, PatternSize)) {
      PatternFound = true;
      break;
    }
  } while ((VPlanSLPSearchWindowSize + WindowStartIndex++) <
           HIRMemrefs.size());
  return PatternFound;
}

bool HeuristicSLP::checkForSLPRedn(const VPReductionFinal *RednFinal,
                                   const VPBasicBlock *Header) const {
  const VPValue *RednVal = RednFinal->getOperand(0);
  if (!RednVal->getType()->isDoubleTy())
    return false;

  const VPInstruction *AddNonFmulInst = nullptr;
  const VPLoadStoreInst *MinusOneStrideLoadInst = nullptr,
                        *VLSLoadInst = nullptr;
  if (!match(RednVal, m_c_FAdd(m_Bind(AddNonFmulInst),
                               m_c_FMul(m_Bind(MinusOneStrideLoadInst),
                                        m_UIToFP(m_Bind(VLSLoadInst))))))
    return false;

  assert(AddNonFmulInst && MinusOneStrideLoadInst && VLSLoadInst &&
         "Unexpected null VPInstruction");
  if (AddNonFmulInst->getParent() != Header ||
      MinusOneStrideLoadInst->getParent() != Header ||
      VLSLoadInst->getParent() != Header)
    return false;

  if (!isa<VPPHINode>(AddNonFmulInst) ||
      MinusOneStrideLoadInst->getOpcode() != Instruction::Load ||
      VLSLoadInst->getOpcode() != Instruction::Load)
    return false;

  bool NegativeStride = false;
  if ((!CM->isUnitStrideLoadStore(MinusOneStrideLoadInst, NegativeStride)) ||
      !NegativeStride)
    return false;

  if (!CM->isOptimizedVLSGroupMember(VLSLoadInst))
    return false;

  LLVM_DEBUG(dbgs() << "SLP reduction candidate seen\n";
             dbgs() << "  Reduction: "; RednFinal->dump();
             dbgs() << "  Minus one stride load: ";
             MinusOneStrideLoadInst->dump(); dbgs() << "  VLS optimized load: ";
             VLSLoadInst->dump());

  return true;
}

void HeuristicSLP::apply(const VPInstructionCost &, VPInstructionCost &Cost,
                         VPInstructionCost &OvhCost, const VPlanVector *Plan,
                         raw_ostream *OS) const {

  if (!Cost.isValid() || !OvhCost.isValid())
    return;

  if (VF == 1 && CMScalarSLPAnalysis && CM->DDG) {
    // Apply cost reduction once SLP pattern is discovered in scalar Plan.
    // TODO: Eventually SLP cost modelling on scalar Plan should completely
    // replace the code below and we should be able to delete it.
    for (const VPBasicBlock *Block : depth_first(&Plan->getEntryBlock())) {
      VPlanSLP SLPDetector(CM, Block);
      Cost += SLPDetector.estimateSLPCostDifference();
    }
    return;
  }

  SmallVector<const RegDDRef*, VPlanSLPSearchWindowSize> HIRLoadMemrefs;
  SmallVector<const RegDDRef*, VPlanSLPSearchWindowSize> HIRStoreMemrefs;

  // Used to track if all the reductions seen in the loop are better optimized
  // using the SLP vectorizer. This is used to address performance regressions
  // until CM can generalize code to determine which loops are better candidates
  // for SLP vectorization.
  bool NonSLPRednSeen = false;
  unsigned NumSLPRednsSeen = 0;

  // Gather all Store and Load Memrefs since SLP starts pattern search on
  // stores and on our cases we have consequent loads as well.
  for (const VPBasicBlock *Block : depth_first(&Plan->getEntryBlock()))
    for (const VPInstruction &VPInst : *Block) {
      if (auto DDRef = getHIRMemref(&VPInst)) {
        if (VPInst.getOpcode() == Instruction::Store)
          HIRStoreMemrefs.push_back(DDRef);
        else if (VPInst.getOpcode() == Instruction::Load)
          HIRLoadMemrefs.push_back(DDRef);
      }

      if (auto *RednFinal = dyn_cast<VPReductionFinal>(&VPInst)) {
        const VPLoop *TopLoop = *(Plan->getVPLoopInfo()->begin());
        if (checkForSLPRedn(RednFinal, TopLoop->getHeader()))
          NumSLPRednsSeen++;
        else
          NonSLPRednSeen = true;
      }
    }

  if ((CMUseSLPPatternHeuristics &&
       ProcessSLPHIRMemrefs(HIRStoreMemrefs, VPlanSLPStorePatternSize) &&
       ProcessSLPHIRMemrefs(HIRLoadMemrefs, VPlanSLPLoadPatternSize)) ||
      (CMUseSLPRedPatternHeuristics && NumSLPRednsSeen == 4 && !NonSLPRednSeen))
    Cost *= VF;
}

VPInstructionCost HeuristicSpillFill::operator()(const VPBasicBlock *VPBlock,
                                                 LiveValuesTy &LiveValues,
                                                 bool SSERegsPressure) const {
  int NumberLiveValuesMax = 0;
  VPLoop *OuterMostVPLoop = *(Plan->getVPLoopInfo()->begin());
  StringRef TargetISA = CM->TTI.getISASetForIMLFunctions();
  bool TargetHasKReg = TargetISA.contains("avx512");

  // Skip values of such Type as if they don't make any register pressure when
  // value of such type isn't placed on register type which matches the request:
  // NeedsVecValue/NeedsScalValue and SSERegsPressure settings.
  auto SkipTy = [&](const Type *Ty,
                    bool NeedsVecValue,
                    bool NeedsScalValue) {
    bool BurnsVecReg =
      (NeedsVecValue || Ty->isVectorTy() ||
       (Ty->isFloatingPointTy() && !Ty->isX86_FP80Ty())) &&
      !(Ty->getScalarType()->isIntegerTy(1) && TargetHasKReg);
    bool BurnsGPR = NeedsScalValue && Ty->isIntegerTy();
    // Note that both predicates above can be false meaning that the value
    // in question presumably allocated elsewhere: X87 stack or k-register.
    return (SSERegsPressure && !BurnsVecReg) || (!SSERegsPressure && !BurnsGPR);
  };

  // Skip Inst result when it doesn't contribute into register pressure type
  // we model on this call (SSERegsPressure setting).
  auto SkipInstRes = [&](const VPInstruction *I) {
    // TODO:
    // VF checks should be removed eventually. It is a bug in the current
    // SVA implementation that we need them.
    //
    // TODO:
    // The implementation doesn't cover scalarization cases, when vector
    // instruction going to be scalarized contributing into scalar registers
    // pressure, not vector.
    //
    bool ResNeedsVec =
      (VF > 1) && Plan->getVPlanSVA()->instNeedsVectorCode(I);
    bool ResNeedsScal =
      (VF == 1) ||
      Plan->getVPlanSVA()->instNeedsFirstScalarCode(I) ||
      Plan->getVPlanSVA()->instNeedsLastScalarCode(I);

    return SkipTy(I->getType(), ResNeedsVec, ResNeedsScal);
  };

  // Skip Inst operand when it doesn't contribute into register pressure type
  // we model on this call (SSERegsPressure setting).
  auto SkipInstOperand = [&](const VPInstruction *I, unsigned OpndIdx) {
    // TODO:
    // VF checks should be removed eventually. It is a bug in the current
    // SVA implementation that we need them.
    //
    // TODO:
    // The implementation doesn't cover scalarization cases, when vector
    // instruction going to be scalarized contributing into scalar registers
    // pressure, not vector.
    //
    bool OpndNeedsVec =
      (VF > 1) && Plan->getVPlanSVA()->operandNeedsVectorCode(I, OpndIdx);
    bool OpndNeedsScal =
      (VF == 1) ||
      Plan->getVPlanSVA()->operandNeedsFirstScalarCode(I, OpndIdx) ||
      Plan->getVPlanSVA()->operandNeedsLastScalarCode(I, OpndIdx);

    return
      SkipTy(I->getOperand(OpndIdx)->getType(), OpndNeedsVec, OpndNeedsScal);
  };

  auto PHIs = (cast<VPBasicBlock>(OuterMostVPLoop->getHeader()))->getVPPhis();
  int FreeVecHWRegsNum = CM->TTI.getNumberOfRegisters(
      CM->TTI.getRegisterClassForType(SSERegsPressure));

  // TODO - When accounting for register pressure we only account for the
  // following cases
  // - Scalar VPValue use counting towards GPR register pressure
  // - Vector VPValue use counting towards vector register pressure
  // However, a scalar floating point type value uses a vector register. This
  // case is not modeled currently and is something that we need to look into
  // in future. However, scalar FP recurrences are expected to be rare(FP
  // induction is an example).
  auto &PSA = CM->getOrCreatePartialSumAnalysis();
  for (auto &Phi : PHIs) {
    if (SkipInstRes(&Phi))
      continue;

    // If we are not accounting for PHI pumping or computing non-sse register
    // pressure, assume PHI takes one register.
    if (!VPlanPhiPumping || !SSERegsPressure) {
      FreeVecHWRegsNum -= 1;
      continue;
    }

    // Estimated # of registers for the PHI value.
    unsigned PhiHWRegs = 1;

    Type *Ty = Phi.getType();
    if (isVectorizableTy(Ty->getScalarType()))
      PhiHWRegs = CM->TTI.getNumberOfParts(getWidenedType(Ty, VF));
    else
      // The type will be serialized. Model as VF registers.
      PhiHWRegs = VF;

    // When unrolling with partial sum reductions, model the
    // additional accumulator registers required.
    if (CMUseUnrollHeuristics && PSA.isCandidate(&Phi))
      PhiHWRegs *= UF;

    FreeVecHWRegsNum -= PhiHWRegs;
  }

  for (const VPInstruction &VPInst : reverse(*VPBlock)) {
    // Zero-cost and unknown-cost instructions are ignored.
    // That might be pseudo inst that don't induce real code on output.
    VPInstructionCost InstCost = CM->getTTICost(&VPInst);
    if (!InstCost.isValid() || InstCost == 0)
      continue;

    // Once definition is met the value is marked dead as the result of
    // instruction generally can occupy the same register as one of its
    // operand, unless all its operands are alive throughout the intruction.
    if (!SkipInstRes(&VPInst))
      LiveValues.erase(&VPInst);

    // Populate LiveValues for operands of given inst.

    // When legalization cost is greater than '1' the VP instruction is
    // eventually represented with two or more HW instructions which makes RP
    // spread across several HW instructions.  Math below adjust VPInst
    // granularity register pressure to HW instructions register pressure.
    //
    // The main idea here is that a VPInstructions that needs N registers to
    // hold its result is encoded into N machine instructions and the highest
    // register pressure is N / 2, not N, when N is big enough.
    //
    // N / 2 is too coarse for small N.  The approximation for small N is:
    // N = 1    ==>   RP = 1
    // N = 2    ==>   RP = 2
    // N = 4    ==>   RP = 4
    // N = 8    ==>   RP = 6
    // N = 16   ==>   RP = 11
    // N > 16   ==>   RP => N / 2
    auto TranslateVPInstRPToHWRP =
      [](unsigned VPInstRP) -> unsigned {
      if (VPInstRP <= 4)
        return VPInstRP;
      else if (VPInstRP <= 8)
        return (VPInstRP + 4) / 2;
      return (VPInstRP + 6) / 2;
    };

    for (unsigned Idx = 0; Idx < VPInst.getNumOperands(); Idx++) {
      auto *Op = VPInst.getOperand(Idx);
      // TODO:
      // Constants yet to be supported.
      if (!isa<VPInstruction>(Op))
        continue;

      if (SkipInstOperand(&VPInst, Idx))
        continue;

      const VPInstruction *OpInst = cast<VPInstruction>(Op);
      VPInstructionCost OpInstCost = CM->getTTICost(OpInst);
      if (!OpInstCost.isValid() || OpInstCost == 0)
        continue;

      Type *OpScalTy = OpInst->getType()->getScalarType();

      if (VectorType::isValidElementType(OpScalTy))
        LiveValues[OpInst] = TranslateVPInstRPToHWRP(
          CM->TTI.getNumberOfParts(getWidenedType(OpInst->getType(), VF)));
      else
        // RP for aggregate types are modelled as if they serialized with
        // VF instructions.
        LiveValues[OpInst] = TranslateVPInstRPToHWRP(VF);
    }

    auto LiveValuesNumbers = make_second_range(LiveValues);
    int NumberLiveValuesCur =
      std::accumulate(LiveValuesNumbers.begin(), LiveValuesNumbers.end(), 0);

    // Model that some intructions are lowered into a sequence of more basic
    // instructions requiring N-times more registers, where N is the number of
    // basic instructions.
    //
    // As we don't have information which operand is required within
    // N-sequence, we estimate that N-element sequence burns N / 2 registers.
    // The same heuristics as for N legalization cost is applied.
    //
    // The number of instructions VPInst is lowered into is its TTI Cost if it
    // is Load/Store. Applies to serialized Loads/Stores only.
    // Also Doesn't apply to other costly instructions such as Mul or Div.
    //
    // TODO: the model doesn't apply to Instructions that remain as a call to a
    // library (such as SVML).  For call cases we need estimation of how many
    // registers such call requires.
    //
    auto SerializableLoadStore =
      [&](const VPLoadStoreInst &VPInst) -> bool {
      // Don't need to serialize for Scalar VPlan but isLegalMaskedLoad/Store,
      // isLegalMaskedGather/Scatter return false for <1 x ...> vectors.
      // So special case VF = 1 here.
      if (VF == 1)
        return false;

      // Unvectorizable by VPlan types.
      if (!isVectorizableLoadStore(&VPInst))
        return true;

      bool IsLoad  =  (VPInst.getOpcode() == Instruction::Load);
      bool IsStore =  (VPInst.getOpcode() == Instruction::Store);
      bool IsMasked = (VPInst.getParent()->getPredicate() != nullptr);

      Align Alignment = Align(CM->getMemInstAlignment(&VPInst));
      Type *VTy = getWidenedType(VPInst.getValueType(), VF);
      bool NegativeStride = false;

      // Check for masked unit load/store presence in HW.
      if (CM->isUnitStrideLoadStore(&VPInst, NegativeStride)) {
        if ((IsMasked && IsLoad  &&
             !CM->TTI.isLegalMaskedLoad(VTy, Alignment)) ||
            (IsMasked && IsStore &&
             !CM->TTI.isLegalMaskedStore(VTy, Alignment)))
          return true;

        return false;
      }

      OVLSGroup *Group = CM->VLSA->getGroupsFor(Plan, &VPInst);
      if (Group && Group->size() > 1)
        // This is very imprecise and needs more refined checks. The idea is
        // that if we have 4 consequtive byte accesses, each of them on its own
        // might be illegal on HW (e.g. byte scatter), but legal if the whole
        // VLS group is accessed at once. See CMPLRLLVM-29061 for the benchmark
        // example.
        return false;

      // Check for unsupported gather/scatter instruction.
      // Note: any gather/scatter is considered as masked.
      if ((IsLoad && !CM->TTI.isLegalMaskedGather(VTy, Alignment)) ||
          (IsStore && !CM->TTI.isLegalMaskedScatter(VTy, Alignment)))
        return true;

      return false;
    };

    auto *LoadStore = dyn_cast<VPLoadStoreInst>(&VPInst);
    if (LoadStore && SerializableLoadStore(*LoadStore))
      // Estimate number of machine instructions used to serialize given
      // Load/Store basing on truncated to integer cost of given instruction.
      // int(InstCost) gives an estimation of the number of instructions the
      // serialized load/store is implemented with.
      //
      // Note:
      // The estimation applies to both register files: Vector registers and
      // Scalar registers. In order to do better estimation individually for
      // GPR and Vector registers we need to know what instructions the
      // serialized instruction decomposes into.
      NumberLiveValuesCur += TranslateVPInstRPToHWRP(InstCost.getInt64Value());

    LLVM_DEBUG(const auto &LVNs = make_second_range(LiveValues);
               dbgs() << "RP = " << NumberLiveValuesCur << ", LV# = " <<
               llvm::count_if(LVNs, [](int Elem) -> bool {
                   return Elem != 0;
                 }) << " for ";
               VPInst.printWithoutAnalyses(dbgs()); dbgs() << '\n';
               dbgs() << "Live vals:";
               for (const auto &LiveValue : LiveValues)
                 if (LiveValue.second) {
                   dbgs() << ' ';
                   LiveValue.first->printAsOperand(dbgs());
                 }
               dbgs() << '\n';);

    NumberLiveValuesMax = std::max(NumberLiveValuesCur, NumberLiveValuesMax);
  }

  LLVM_DEBUG(dbgs() << "Max RP " << NumberLiveValuesMax <<
             ", Num free regs: " << FreeVecHWRegsNum <<
             " for block " << VPBlock->getName() << " (VF = " << VF <<
             ", SSERegsPressure = " << SSERegsPressure << ")\n";);

  if (NumberLiveValuesMax <= FreeVecHWRegsNum)
    return 0;

  unsigned AS = CM->DL->getAllocaAddrSpace();
  unsigned RegBitWidth = CM->TTI.getLoadStoreVecRegBitWidth(AS);
  unsigned RegByteWidth = RegBitWidth / 8;
  Type *VecTy = getWidenedType(Type::getInt8Ty(*Plan->getLLVMContext()),
                               RegByteWidth);
  VPInstructionCost StoreCost = CM->TTI.getMemoryOpCost(
    Instruction::Store, VecTy, Align(RegByteWidth), AS);
  VPInstructionCost LoadCost = CM->TTI.getMemoryOpCost(
    Instruction::Load, VecTy, Align(RegByteWidth), AS);

  return NumberOfSpillsPerExtraReg *
    (NumberLiveValuesMax - FreeVecHWRegsNum) *
    (StoreCost + LoadCost);
}

void HeuristicSpillFill::apply(const VPInstructionCost &,
                               VPInstructionCost &Cost,
                               VPInstructionCost &OvhCost,
                               const VPlanVector *Plan, raw_ostream *OS) const {
  // Don't run register pressure heuristics on TTI models that do not support
  // scalar or vector registers.
  if (CM->TTI.getNumberOfRegisters(
        CM->TTI.getRegisterClassForType(false)) == 0 ||
      CM->TTI.getNumberOfRegisters(
        CM->TTI.getRegisterClassForType(true)) == 0)
    return;

  // LiveValues map contains the liveness of the given instruction multiplied
  // by its legalization factor.  The map is updated on each VPInstruction in
  // the loop below.
  //
  // Consider the following IR and the map content at each VPInst.  The
  // traversal is backward.
  //
  // 1: %val1 = def              ; LiveValues[%1] = 0,  LiveValues[%2] = 0
  // 2: %val2 = use %val1, %val1 ; LiveValues[%1] = L1, LiveValues[%2] = 0
  // 3: %val3 = use %val1, %val2 ; LiveValues[%1] = L1, LiveValues[%2] = L2
  //
  // where L1 - number of HW registers to hold value defined by %1,
  //       L2 - number of HW registers to hold value defined by %2.
  //
  // Register pressure in any given point the sum of all Ln values in the map.
  // The code below figures out what is maximum of register pressure in given
  // basic block.
  //
  // Keep track of vector and scalar live values in separate maps.
  LiveValuesTy VecLiveValues, ScalLiveValues;

  const VPLoop *L = Plan->getMainLoop(true /* StrictCheck*/);
  for (auto *Block : post_order(&Plan->getEntryBlock())) {
    // For simplicity we pass LiveOut from previous block as LiveIn to the next
    // block in walk like walking through a linear sequence of BBs.
    // TODO:
    // Eventually we need to fix the code to work correctly for non linear CFG:
    // possible inner loops and if-else conditions.
    // Currently CM is focused on inner loop vectorization and we can ignore
    // inner loops.  Non unifom conditions are turned into masked linear BB
    // sequence.  Uniform conditions are moved out of the loop by LoopOpt
    // normally and we don't see non linear CFG in VPlan in the most cases for
    // HIR pipeline.
    bool InLoop = L->contains(Block);
    (InLoop ? Cost : OvhCost) += (*this)(Block, ScalLiveValues, false);
    (InLoop ? Cost : OvhCost) += (*this)(Block, VecLiveValues, true);
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void HeuristicSpillFill::dump(raw_ostream &OS, const VPBasicBlock *VPBB) const {
  LiveValuesTy LiveValues;
  std::string ReturnStr;
  VPInstructionCost ScalSpillFillCost = (*this)(VPBB, LiveValues, false);
  if (ScalSpillFillCost > 0)
    OS << "Block Scalar spill/fill approximate cost (not included "
      "into base cost): " << ScalSpillFillCost << '\n';

  if (VF > 1) {
    LiveValues.clear();
    VPInstructionCost VecSpillFillCost = (*this)(VPBB, LiveValues, true);
    if (VecSpillFillCost > 0)
      OS << "Block Vector spill/fill approximate cost (not included into "
        "base cost): " << VecSpillFillCost << '\n';
  }
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

void HeuristicGatherScatter::apply(const VPInstructionCost &TTICost,
                                   VPInstructionCost &Cost,
                                   VPInstructionCost &OvhCost,
                                   const VPlanVector *Plan,
                                   raw_ostream *OS) const {
  if (!TTICost.isValid() || !Cost.isValid() || !OvhCost.isValid())
    return;

  VPlanCostPair GSCost = (*this)(Plan);
  if (GSCost.first == 0 && GSCost.second == 0)
    return;

  unsigned HWGSThreshold = CMGatherScatterThreshold;
  // TODO:
  // SWGSThreshold for VF = 2 is yet gets the old value default value to
  // minimize impact on benchmarks. This check has to be replaced with VF
  // sentisitive formulae or removed altogether eventually.
  // Same for Cost related check: it is there to cut off regressions observed
  // w/o this check. The regression have to be addressed and the check has to
  // be removed.
  unsigned SWGSThreshold = ((VF > 2) && (Cost < 100)) ?
    CMSWGatherScatterThreshold : CMGatherScatterThreshold;

  // If CMGatherScatterThreshold is not specified in the command line the
  // default value for heuristic is different in ZMM-enabled context.
  if (CMGatherScatterThreshold.getNumOccurrences() == 0 &&
      CM->TTI.getRegisterBitWidth(
        TargetTransformInfo::RGK_FixedWidthVector) >= 512)
    HWGSThreshold = CMGatherScatterDefaultThresholdZMM;

  // Increase GatherScatter cost contribution in case HW or SW Gathers/Scatters
  // (or both) take too much.
  if (TTICost * HWGSThreshold < GSCost.first * 100)
    Cost += CMGatherScatterPenaltyFactor.getValue() * GSCost.first;

  if (TTICost * SWGSThreshold < GSCost.second * 100)
    Cost += CMGatherScatterPenaltyFactor.getValue() * GSCost.second;
}

VPlanCostPair HeuristicGatherScatter::operator()(
  const VPlanVector *Plan) const {
  VPlanCostPair GSCost = {0, 0};
  if (VF == 1)
    return GSCost;

  for (auto *Block : depth_first(&Plan->getEntryBlock()))
    // FIXME: Use Block Frequency Info (or similar VPlan-specific analysis) to
    // correctly scale the cost of the basic block.
    GSCost = GSCost + (*this)(Block);

  return GSCost;
}

VPlanCostPair HeuristicGatherScatter::operator()(
  const VPBasicBlock *VPBlock) const {
  VPlanCostPair Cost = {0, 0};
  if (VF == 1)
    return Cost;

  for (const VPInstruction &VPInst : *VPBlock)
    Cost = Cost + (*this)(&VPInst);
  return Cost;
}

// CM local utility to determine if the input load/store instruction is
// inducing HW or SW gather or scatter intructions for given VF.
// The rotine returns VPLoadStoreInst if it is gather/scatter or nullptr
// otherwise. Iff the routne returns non null pointer the type of instruction
// (hardware or software) is indicated through IsHWGatherScatter argument.
static const VPLoadStoreInst *isGatherScatter(
    bool &IsHWGatherScatter,
    const VPInstruction *VPInst,
    unsigned VF,
    const VPlanTTICostModel *CM) {
  // The compiler doesn't generate HW or SW gather or scatter for VF = 1.
  if (VF == 1)
    return nullptr;

  auto *LoadStore = dyn_cast<VPLoadStoreInst>(VPInst);
  if (!LoadStore)
    return nullptr;

  bool NegativeStride;
  if (CM->isOptimizedVLSGroupMember(LoadStore) ||
      CM->isUniformLoadStore(LoadStore) ||
      CM->isUnitStrideLoadStore(LoadStore, NegativeStride))
    return nullptr;

  IsHWGatherScatter = false;
  // Unvectorizable by VPlan types will be serialized.
  if (!isVectorizableLoadStore(LoadStore))
    return LoadStore;

  // Check for unsupported gather/scatter instruction.
  // Note: any gather/scatter is considered as masked.
  Align Alignment = Align(CM->getMemInstAlignment(LoadStore));
  Type *VTy = getWidenedType(LoadStore->getValueType(), VF);

  if (VPInst->getOpcode() == Instruction::Load) {
    if (!CM->TTI.isLegalMaskedGather(VTy, Alignment) ||
        CM->TTI.forceScalarizeMaskedGather(cast<VectorType>(VTy), Alignment))
      return LoadStore;
  }
  else {
    if (!CM->TTI.isLegalMaskedScatter(VTy, Alignment) ||
        CM->TTI.forceScalarizeMaskedScatter(cast<VectorType>(VTy), Alignment))
      return LoadStore;
  }

  // Only HW gathers/scatters are expected to reach this code.
  IsHWGatherScatter = true;
  return LoadStore;
}

VPlanCostPair HeuristicGatherScatter::operator()(
  const VPInstruction *VPInst) const {
  bool IsHWGatherScatter = true;
  auto LoadStore = isGatherScatter(IsHWGatherScatter, VPInst, VF, CM);
  if (!LoadStore)
    return std::make_pair(0, 0);

  auto GSInstCost = CM->getLoadStoreCost(
    LoadStore, VF, true /* only need gather/scatter cost */);

  // TODO:
  // Currently we enable separate handling for i16 types only to reduce
  // other benchmarks impact. Eventually type checks either removed altogether
  // OR we may need to introduce separate tracking for SW gathers and scatters
  // typed differently if proven it is neccesary.
  Type *LSType = LoadStore->getValueType()->getScalarType();
  if (!IsHWGatherScatter && LSType->isIntegerTy() &&
      LSType->getIntegerBitWidth() == 16)
    return std::make_pair(0, GSInstCost);
  else
    return std::make_pair(GSInstCost, 0);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void HeuristicGatherScatter::dump(raw_ostream &OS,
                                  const VPlanVector *Plan) const {
  VPlanCostPair GatherScatterCost = (*this)(Plan);
  if (GatherScatterCost.first > 0)
    OS << "Plan total cost includes HW GS Cost: " <<
      GatherScatterCost.first << '\n';
  if (GatherScatterCost.second > 0)
    OS << "Plan total cost includes SW GS Cost: " <<
      GatherScatterCost.second << '\n';
}

void HeuristicGatherScatter::dump(raw_ostream &OS,
                                  const VPBasicBlock *VPBB) const {
  VPlanCostPair GatherScatterCost = (*this)(VPBB);
  if (GatherScatterCost.first > 0)
    OS << "Block total cost includes HW GS Cost: " <<
      GatherScatterCost.first << '\n';
  if (GatherScatterCost.second > 0)
    OS << "Block total cost includes SW GS Cost: " <<
      GatherScatterCost.second << '\n';
}

void HeuristicGatherScatter::dump(raw_ostream &OS,
                                  const VPInstruction *VPInst) const {
  VPlanCostPair GatherScatterCost = (*this)(VPInst);
  if (GatherScatterCost.first > 0)
    OS << " *HW GS*";
  if (GatherScatterCost.second > 0)
    OS << " *SW GS*";
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

// The utility below searches for patterns that form PSADBW instruction
// semantics.  The current implementation searches for the following pattern:
//
// EXPR1 can be one of:
// %Add    = ZExt(A) + ZExt(B) * (-1)
// OR:
// %Add    = ZExt(A) - ZExt(B)
//
// EXPR2 can be one of:
// %Abs    = Abs(%Add)
// OR:
// %Abs    = call %Add llvm.abs
//
// The pattern should be:
//
// EXPR1
// EXPR2
//
// Returns true if pattern is detected.
// Any outside users of instructions forming the pattern are ignored.
// The utility captures participating intructions into PatternInsts.
bool HeuristicPsadbw::checkPsadwbPattern(
  const VPInstruction *AbsInst,
  SmallPtrSetImpl<const VPInstruction*> &PatternInsts) const {
  const VPInstruction *Add = nullptr, *Mul = nullptr, *ZExtA = nullptr,
                      *ZExtB = nullptr;
  const VPValue *A = nullptr, *B = nullptr;

  auto MAdd =
    m_Bind(m_CombineOr(
      m_c_Add(m_Bind(m_ZExt(m_Bind(A)), ZExtA),
              m_Bind(m_c_Mul(m_Bind(m_ZExt(m_Bind(B)), ZExtB),
                             m_ConstantInt<-1, VPConstantInt>()), Mul)),
      m_Sub(m_Bind(m_ZExt(m_Bind(A)), ZExtA),
            m_Bind(m_ZExt(m_Bind(B)), ZExtB))), Add);
  if (!match(AbsInst, m_VPAbs(MAdd)) &&
      !match(AbsInst, m_Intrinsic<Intrinsic::abs>(MAdd)))
    return false;

  if (A->getType()->getScalarSizeInBits() != 8 ||
      B->getType()->getScalarSizeInBits() != 8)
    return false;

  // Store participating instructions into PatternInsts.
  PatternInsts.insert(AbsInst);
  PatternInsts.insert(Add);
  if (Mul)
    PatternInsts.insert(Mul);
  PatternInsts.insert(ZExtA);
  PatternInsts.insert(ZExtB);
  return true;
}

void HeuristicPsadbw::initForVPlan() {
  // Detect all psadbw patterns in the ctor and populate PsadbwPatternInsts
  // so dumping machinery can reveal participating intructions early.
  if (VF != 1)
    return;

  // Reset the map for the case of multiple calls to init().
  PsadbwPatternInsts.clear();

  const VPLoop *TopLoop = *(Plan->getVPLoopInfo()->begin());
  for (const VPLoop *VPL : post_order(TopLoop)) {
    if (VPL->getLoopDepth() != 1)
      continue;

    VPBasicBlock *Block = VPL->getHeader();
    VPBasicBlock *Latch = VPL->getLoopLatch();

    // Loop through PHI nodes.
    for (const VPPHINode &PhiNode : Block->getVPPhis()) {
      assert(PhiNode.getNumIncomingValues() == 2 &&
             "A loop header is expected to have two predecessors.");

      const auto *SumCarryOut =
        dyn_cast<VPInstruction>(PhiNode.getIncomingValue(Latch));
      if (!SumCarryOut || SumCarryOut->getOpcode() != Instruction::Add)
        continue;

      // Now go up through ADD's operands starting from SumCarryOut and
      // find the patterns.  We model unrolled at source level or by HIR
      // pattern such as:
      //
      // acc += abs(a - b);
      // acc += abs(c - d);
      //
      // We value those ADDs which have either another ADD in operands, or
      // select inst, or PhiNode.  Other ADDs or other Insts are not a part of
      // the pattern.

      // Keeps all instructions that are part of PSADBW pattern.
      SinglePatternInstsSet CurrPsadbwPatternInsts;
      std::stack<const VPInstruction *> AddsStack;

      AddsStack.push(SumCarryOut);

      while (!AddsStack.empty()) {
        const VPInstruction *AddInst = AddsStack.top();
        AddsStack.pop();

        const VPInstruction *LHS, *RHS;
        // In case of accumulator is 64 bits there are ZExt's in operands of
        // the ADD possible.  Go over them.
        if (!match(AddInst, m_Add(m_ZExtOrSelf(m_Bind(LHS)),
                                  m_ZExtOrSelf(m_Bind(RHS)))))
          continue;

        if (LHS->getOpcode() == Instruction::Add)
          AddsStack.push(LHS);
        if (RHS->getOpcode() == Instruction::Add)
          AddsStack.push(RHS);

        // Add PhiNode and AddInst into the list of pattern forming instruction
        // whenever pattern is found.
        //
        // Make sure to run checkPsadwbPattern for LHS and RHS as there could
        // be patterns on the both ways and we want checkPsadwbPattern() to
        // populate CurrPsadbwPatternInsts with corresponding instructions.
        bool PatternFound = checkPsadwbPattern(LHS, CurrPsadbwPatternInsts);
        if (checkPsadwbPattern(RHS, CurrPsadbwPatternInsts) || PatternFound) {
          CurrPsadbwPatternInsts.insert(&PhiNode);
          CurrPsadbwPatternInsts.insert(AddInst);
        }
      }

      if (CurrPsadbwPatternInsts.empty())
        continue;

      // There are some ADD instructions in VPlan which sum parts of PSADBW
      // pattern but they are left overboard (not in CurrPsadbwPatternInsts).
      // That may happen because of some ADD may have other ADD as their
      // operands but no Select to make 'PatternFound' trigger for them in the
      // loop above.
      //
      // Consider the following example.
      //
      // acc += abs(a[i + 0] - b[i + 0]
      // acc += abs(a[i + 1] - b[i + 1]
      // acc += abs(a[i + 2] - b[i + 2]
      // acc += abs(a[i + 3] - b[i + 3]
      //
      // Such case may end up with four pattern forming select instructions
      // that are summed:
      // %Sel1 = select ...
      // %Sel2 = select ...
      // %Sel3 = select ...
      // %Sel4 = select ...
      // %Add1 = add i32 %Sel1, %Sel2
      // %Add2 = add i32 %Sel3, %Sel4
      // %Add3 = add i32 %Add1, %Add2
      //
      // While %Add3's operands are processed in the main loop and added to
      // CurrPsadbwPatternInsts Add3 instruction itself is not added.
      //
      // Also there are possible ZExts that are not gathered in
      // CurrPsadbwPatternInsts in case ACC is I64.
      //
      // Make additial post pass throughout the current block to pick them up.
      for (const VPInstruction &VPInst : *Block) {
        const VPInstruction *LHS = nullptr, *RHS = nullptr;
        if (match(&VPInst, m_ZExt(m_Bind(LHS))) &&
            CurrPsadbwPatternInsts.count(LHS) > 0)
          CurrPsadbwPatternInsts.insert(&VPInst);
        else if (match(&VPInst, m_Add(m_Bind(LHS),
                                      m_Bind(RHS))) &&
                 CurrPsadbwPatternInsts.count(LHS) > 0 &&
                 CurrPsadbwPatternInsts.count(RHS) > 0)
          CurrPsadbwPatternInsts.insert(&VPInst);
      }

      // If loop has known number of iterations and it is 8/16 and SLP within
      // the loop iteration is not possible we skip such loop since vectorizing
      // by VPlan leads to complete unroll and ISel can handle vectorized by
      // VPlan code (i.e. recognize PSADBW).  Also if the loop has an outer
      // loop of known trip count it also can be completely unrolled causing
      // better performance comparing to PSADDBW in a loop. Otherwise if
      // vectorize in SLP, not in VPlan the unroller doesn't unroll
      // outer loop (the current pipeline is VPlan -> Unroll -> SLP).
      //
      // If there is 4 or more psadbw patterns contributing in the same
      // accumulator we expect SLP to trigger.
      TripCountInfo LoopTCI = VPL->getTripCountInfo();
      unsigned NumberOfPatterns = std::count_if(
        CurrPsadbwPatternInsts.begin(), CurrPsadbwPatternInsts.end(),
        [](const VPInstruction *I) {
          return I->getOpcode() == VPInstruction::Abs ||
                 match(I, m_Intrinsic<Intrinsic::abs>(m_VPValue())); });

      if (!LoopTCI.IsEstimated && NumberOfPatterns < 4 &&
          (LoopTCI.TripCount == 8 || LoopTCI.TripCount == 16))
        continue;

      // Keep all current pattern instructions in PsadbwPatternInsts.
      PsadbwPatternInsts[SumCarryOut].insert(CurrPsadbwPatternInsts.begin(),
                                             CurrPsadbwPatternInsts.end());
    }
  }
}

// Does all neccesary target checks and return corrected VPlan Cost.
void HeuristicPsadbw::apply(const VPInstructionCost &, VPInstructionCost &Cost,
                            VPInstructionCost &OvhCost, const VPlanVector *Plan,
                            raw_ostream *OS) const {

  VPInstructionCost PatternCost = 0;

  // PSADBW cost in terms of number of intructions.
  VPInstructionCost PsadbwCost = 1;

  for (const auto &PatternInstructionsEl : PsadbwPatternInsts) {
    const VPInstruction* SumCarryOut = PatternInstructionsEl.first;
    SinglePatternInstsSet PatternInstructions = PatternInstructionsEl.second;
    // Sum up costs of all instructions within PatternInstructions.
    // If there an instruction in the pattern has more than one use the whole
    // pattern cost is halved.  This way we model external to pattern uses
    // that keeps part of pattern instruction alive even if the whole pattern
    // is going to be replace with psadbw.  On average half of the whole
    // pattern is kept by one external use.  The more external uses the pattern
    // has the less the gain cost of idiom recognition.
    //
    // Note that SumCarryOut has two uses that are both within the pattern
    // (i.e. not external).
    assert(!PatternInstructions.empty() &&
           "The set is not expected to be empty.");

    VPInstructionCost CurrentPatternCost = 0;
    unsigned NumberOfExternalUses = 0;

    for (const VPInstruction *VPInst : PatternInstructions) {
      VPInstructionCost InstCost = CM->getTTICost(VPInst);
      if (InstCost.isUnknown())
        continue;

      CurrentPatternCost += InstCost;

      if ((VPInst == SumCarryOut && VPInst->getNumUsers() > 2) ||
          (VPInst != SumCarryOut && VPInst->getNumUsers() > 1))
        NumberOfExternalUses++;
    }

    // The following additional cost models performance impact due to code
    // size bloating if PSADBW opportunity is ignored.
    CurrentPatternCost *= 2;

    // Factor in NumberOfExternalUses:
    // Cost = Cost / (2 ^ NumberOfExternalUses)
    CurrentPatternCost /= (1 << NumberOfExternalUses);

    if (CurrentPatternCost.isValid() && CurrentPatternCost > PsadbwCost)
      PatternCost += CurrentPatternCost - PsadbwCost;
  }

  // Unsupported instruction within the pattern generally are not expected,
  // but in the heuristics we don't want to assert on that rather bail out.
  if (!PatternCost.isValid())
    return;

  if (PatternCost > Cost)
    // TODO:
    // Consider returning PsadbwCost here.
    Cost = 0;
  else
    Cost -= PatternCost;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void HeuristicPsadbw::dump(raw_ostream &OS, const VPInstruction *VPInst) const {
  for (const auto &PatternInstructionsEl : PsadbwPatternInsts) {
    const VPInstruction* SumCarryOut = PatternInstructionsEl.first;
    SinglePatternInstsSet PatternInstructions = PatternInstructionsEl.second;

    if (VPInst == SumCarryOut)
      OS << " *PSADBW* (CarryOut Def)";
    else if (PatternInstructions.count(VPInst) > 0) {
      OS << " *PSADBW*, CarryOut: ";
      SumCarryOut->printAsOperandNoType(OS);
    }
  }
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

void HeuristicSVMLIDivIRem::apply(const VPInstructionCost &TTICost,
                                  VPInstructionCost &Cost,
                                  VPInstructionCost &OvhCost,
                                  const VPInstruction *VPInst,
                                  raw_ostream *OS) const {

  // SVML implementation kicks in for VF > 2. Two scalar calls are used
  // to implement the operation for VF == 2.
  if (VF == 1 || VF == 2)
    return;

  unsigned Opcode = VPInst->getOpcode();

  if (Opcode != Instruction::UDiv && Opcode != Instruction::SDiv &&
      Opcode != Instruction::URem && Opcode != Instruction::SRem)
    return;

  // Special case for integer DIV/REM operation.
  //
  // Vector integer DIV/REM implemented through serialized scalar code
  // for VF = 2, yelding slightly worse performance comparing to vanilla
  // scalar code due to serialization overhead.
  //
  // For VF > 2 SVML functions are invoked: 8 elements function for int32 and
  // 4 elements function for int64.
  //
  // For int32 SVML yelds ~2x better performance vs scalar version for its
  // natural VF = 8 and for all VF greater than the natural VF.
  //
  // VF = 4 int32 implemented as masked VF = 8 case yelding 2x worse perfomance
  // vs VF = 8.
  //
  // int64 VF = 1 implementation holds RT check for input data that can be
  // divided using 32-bit DIV instruction giving 3.5x better performance.  For
  // 64-bit input data scalar version is ~30% slower of vector version.  We
  // make an assumption that in real applications half data fits 32-bit
  // representation making scalar version of 64-bit DIV/REM (3.5 / 2) ~ 2x
  // faster VS SVML version.
  //
  // Factor in those impirical data into multiplier of the scalar cost
  // of DIV operation.  This is not modelled well by TTI.
  //
  // Don't mess with with 8-/16-bit input data type if it ever possible to get
  // them here.
  //
  // TODO:
  // OpenCL CPU RT uses an alternative version of SVML and the heuristic
  // doesn't cover it.  OCL context can be checked with:
  // M->getNamedMetadata("opencl.ocl.version") != nullptr, where M is Module.
  // Currently CM doesn't have access to Module, neither we always have
  // UnderyingValue valid for Op1.  Thereby as of now OCL context check is
  // missed which is not an issue until VPlan becomes a part of OCL pipeline.
  //
  // TODO:
  // The code below is a temporal code to WA this problem.  Eventually
  // we want to have CG interface which would tells us what instructions
  // [U|S]Div/Rem or any other VPIntruction is implemented with.

  const Type *ScalarTy = VPInst->getType();
  unsigned ElemSize = ScalarTy->getPrimitiveSizeInBits();
  if (ElemSize != 32 && ElemSize != 64)
    return;

  VPInstructionCost ScalarCost = CM->getArithmeticInstructionCost(
    Opcode, VPInst->getOperand(0), VPInst->getOperand(1), ScalarTy, /*VF*/ 1);

  if (!ScalarCost.isValid())
    return;

  VPInstructionCost VectorCost;

  if (ElemSize == 64)
    VectorCost = ScalarCost * VF * 2;
  else if (VF == 2)
    VectorCost = ScalarCost * VF;
  else if (VF == 4)
    VectorCost = ScalarCost * 3;
  else
    VectorCost = ScalarCost * (VF / 2);

  // For operations with constant in argument basic cost model gives better
  // estimation, which we want to use instead of VectorCost.
  Cost = VPInstructionCost::Min(VectorCost, TTICost);
}

void HeuristicOVLSMember::apply(const VPInstructionCost &TTICost,
                                VPInstructionCost &Cost,
                                VPInstructionCost &OvhCost,
                                const VPInstruction *VPInst,
                                raw_ostream *OS) const {

  if (!UseOVLSCM || !CM->VLSA || VF == 1)
    return;

  OVLSGroup *Group = CM->VLSA->getGroupsFor(Plan, VPInst);
  if (!Group || Group->size() <= 1)
    return;

  auto *LoadStore = cast<VPLoadStoreInst>(VPInst);

  // FIXME: OVLSCostModel abstructions need to be revisitted as
  // VPlanVLSCostModel::getInstructionCost() implementation requires external
  // VF to form Vector Type of OVLSInstruction, whereas
  // OVLSTTICostModel::getInstructionCost() implementation fetches number of
  // elements using I->getType().
  //
  VPlanVLSCostModel VLSCM(VF, CM->TTI, LoadStore->getType()->getContext());
  VPInstructionCost VLSGroupCost = VPInstructionCost{
    OptVLSInterface::getGroupCost(*Group, VLSCM)};

  // If current load/store instruction is part of an optimized load/store group,
  // compare VLSGroupCost to TTI based cost for doing the interleaved access
  // and pick the smaller cost.
  // TODO - Codegen currently does not rely on OVLS to generate wide accesses
  // and shuffles. Consider using the TTI based cost always.
  if (CM->isOptimizedVLSGroupMember(LoadStore)) {
    Type *ValTy = LoadStore->getValueType();
    unsigned AddrSpace =
        cast<PointerType>(LoadStore->getPointerOperand()->getType())
            ->getAddressSpace();
    int InterleaveFactor =
        std::abs(computeInterleaveFactor(Group->getInsertPoint()));
    auto *WideVecTy = getWidenedType(ValTy, VF * InterleaveFactor);

    // Holds the indices of existing members in an interleaved load group.
    // Currently we only support accesses with no gaps and hence all indices
    // are pushed onto the vector.
    // NOTE: The code here mimics what is done in community LV to get the cost
    // of the interleaved access.
    SmallVector<unsigned, 4> Indices;
    for (int i = 0; i < InterleaveFactor; i++)
      Indices.push_back(i);

    // Calculate the cost of the whole interleaved group.
    VPInstructionCost TTIInterleaveCost = CM->TTI.getInterleavedMemoryOpCost(
        LoadStore->getOpcode(), WideVecTy, InterleaveFactor, Indices,
        cast<VPLoadStoreInst>(LoadStore)->getAlignment(), AddrSpace,
        TTI::TCK_RecipThroughput,
        LoadStore->getParent()->getPredicate() != nullptr /* UseMaskForCond */,
        false /* UseMaskForGaps */);
    if (!VLSGroupCost.isValid() || TTIInterleaveCost < VLSGroupCost)
      VLSGroupCost = TTIInterleaveCost;
  }

  // If OVLS group exists but its cost is Unknown or Invalid we skip such group.
  if (!VLSGroupCost.isValid())
    return;

  if (ProcessedOVLSGroups.count(Group) != 0) {
    if (!ProcessedOVLSGroups[Group]) {
      LLVM_DEBUG(dbgs() << "TTI cost of memrefs in the Group containing ";
                 LoadStore->printWithoutAnalyses(dbgs());
                 dbgs() << " is less than its OVLS Group cost.\n");
      return;
    }

    assert(is_contained(Group->getMemrefVec(), Group->getInsertPoint()) &&
           "OVLS Group's insertion point is not a member of the Group.");

    if (cast<VPVLSClientMemref>(Group->getInsertPoint())->getInstruction() ==
        LoadStore) {
      LLVM_DEBUG(dbgs() << "Whole OVLS Group cost is assigned on ";
               LoadStore->printWithoutAnalyses(dbgs());
               dbgs() << '\n');

      Cost = VLSGroupCost;
      return;
    }

    LLVM_DEBUG(dbgs() << "Group cost for ";
               LoadStore->printWithoutAnalyses(dbgs());
               dbgs() << " has already been taken into account.\n");

    Cost = 0;
    return;
  }

  VPInstructionCost TTIGroupCost = 0;
  for (OVLSMemref *OvlsMemref : Group->getMemrefVec())
    TTIGroupCost += CM->getLoadStoreCost(
      cast<VPVLSClientMemref>(OvlsMemref)->getInstruction(),
      Align(CM->getMemInstAlignment(LoadStore)), VF);

  if (VLSGroupCost >= TTIGroupCost) {
    LLVM_DEBUG(dbgs() << "Cost for "; LoadStore->printWithoutAnalyses(dbgs());
               dbgs() << " was not reduced from " << Cost << " (TTI group cost "
               << TTIGroupCost << ") to group cost " << VLSGroupCost << '\n');
    ProcessedOVLSGroups[Group] = false;
    return;
  }

  LLVM_DEBUG(dbgs() << "Reduced cost for ";
             LoadStore->printWithoutAnalyses(dbgs());
             dbgs() << " from " << Cost << " (TTI group cost " << TTIGroupCost
                    << " to group cost " << VLSGroupCost << ")\n");
  ProcessedOVLSGroups[Group] = true;

  // We are encountering an OVLS group for the first time. The group cost is
  // returned if we are dealing with the instruction corresponding to insertion
  // point of the group. We return 0 otherwise.
  if (cast<VPVLSClientMemref>(Group->getInsertPoint())->getInstruction() ==
      LoadStore) {
    LLVM_DEBUG(dbgs() << "Whole OVLS Group cost is assigned on ";
               LoadStore->printWithoutAnalyses(dbgs()); dbgs() << '\n');
    Cost = VLSGroupCost;
  } else
    Cost = 0;
}

void HeuristicUnroll::apply(const VPInstructionCost &TTICost,
                            VPInstructionCost &Cost, VPInstructionCost &OvhCost,
                            const VPlanVector *Plan, raw_ostream *OS) const {
  if (UF <= 1 || !CMUseUnrollHeuristics || Cost.isUnknown() || !Cost.isValid())
    return;
  assert(VF != 1 && "Expected VF > 1");

  auto &PSA = CM->getOrCreatePartialSumAnalysis();
  if (CMUnrollPartialSumsOnly && PSA.getCandidates().empty())
    return;

  // As we are usually reducing cost here, we collect the reduction
  // to be subtracted.
  VPInstructionCost CostReduction = 0;

  // The following scale is used for cost adjustments to effectively
  // cancel out later scaling by the UF, i.e if Cost = (C + RemCost)
  // for some C, then:
  //     UF * (C + RemCost - C * DescaleForUF)
  //  == UF * (C + RemCost - C + C / UF)
  //  == UF * RemCost + C
  VPInstructionCost DescaleForUF = (1. - 1. / (float)UF);

  // Target-specific coefficient indicating the relative profitability
  // of partial sums. This is set in the VPlan unroll preferences, but
  // may be overridden by hidden option.
  float BaseILPScore =
      CMUnrollILPScore.getNumOccurrences()
          ? (float)CMUnrollILPScore
          : CM->getVectorUnrollingPreferences().PartialSumILPScore;
  assert(BaseILPScore >= 0.f && "invalid ILP score");

  // Collect cost adjustments for all partial sum candidates.
  for (const auto &Iter : PSA.getCandidates()) {
    const auto &RI = Iter.second;

    // Compute a scaling factor in [0,CMUnrollILPScore] indicating the
    // relative benefit of parallelizing this reduction for UF.
    // The initial heuristic is just the ratio of the total reduction
    // cost to the loop cost, with the expectation that the ILP
    // benefits are best when there is little other independent
    // work in the loop.
    // The incoming loop cost may become lower than the cost
    // recorded on the partial sum candidate due to other heuristics,
    // so we clamp this to 1.
    auto ILPScore =
        std::min(BaseILPScore * RI.Cost / Cost, VPInstructionCost(1));

    // Assuming Cost = C' + R (where R = RI.RedCost), subtracting
    // R*(1-S) yields the following cost after scaling by UF:
    //   UF(C' + R - (1-S)*R) = UF*C' + UF*S*R
    // We subtract R * ILPScore * (1 - 1/uf) from the total
    // cost, so that a score of 0 gives S = 1 (no benefit), and a
    // score of 1 gives S = 1/UF.
    CostReduction += RI.RedCost * ILPScore * DescaleForUF;

    LLVM_DEBUG(dbgs() << "HeuristicUnroll: partial sum reduction seen\n";
               dbgs() << "  PHI node: "; Iter.first->dump();
               dbgs() << "  Reduction cost: " << RI.Cost << ", " << RI.RedCost
                      << "\n"
                      << "  ILPScore: " << ILPScore << " = " << BaseILPScore
                      << " * (" << RI.Cost << " / " << Cost << ")\n");
  }

  // Identify latch condition comparisons that have no other uses.
  // Only one instance of these will be live after unrolling.
  auto *Top = *(Plan->getVPLoopInfo()->begin());
  for (const VPLoop *VPL : post_order(Top)) {
    if (VPL->getLoopDepth() != 1)
      continue;
    auto *Latch = VPL->getLoopLatch();
    auto *BranchInst = dyn_cast<VPBranchInst>(Latch->getTerminator());
    if (BranchInst && BranchInst->isConditional())
      if (auto *BranchCond =
              dyn_cast<VPInstruction>(BranchInst->getCondition())) {
        auto BranchCost = CM->getTTICost(BranchCond);
        if (BranchCond->getNumUsers() == 1 && BranchCost.isValid() &&
            !BranchCost.isUnknown()) {
          CostReduction += BranchCost * DescaleForUF;

          LLVM_DEBUG(dbgs() << "HeuristicUnroll: latch condition seen\n";
                     dbgs() << "  Condition: "; BranchCond->dump();
                     dbgs() << "  Cost: " << BranchCost
                            << " will not be scaled by UF\n");
        }
      }
  }

  assert(Cost > CostReduction && "Cost reduction exceeds total cost");
  Cost = Cost - CostReduction;
  LLVM_DEBUG(dbgs() << "HeuristicUnoll: Cost reduction: " << CostReduction
                    << "\n");
}

void PartialSumAnalysis::analyze(VPlanTTICostModel *CM,
                                 const VPlanVector &Plan) {

  // Bail out if already analyzed or no consumer heuristics
  // are enabled.
  if (AnalyzedPlan == &Plan || !CMUseUnrollHeuristics)
    return;

  // Used to collect TTI costs, replacing unknown/invalid costs with
  // the specified default.
  auto getTTICost = [CM](const VPInstruction *Inst,
                         VPInstructionCost Default = VPInstructionCost(0)) {
    VPInstructionCost C = CM->getTTICost(Inst);
    return (!C.isValid() || C.isUnknown()) ? Default : C;
  };

  // Returns a pair (Cost, RedCost) for a  partial sum candidate,
  // given the PHI carrying the recurrence, where Cost is the total TTI costs
  // for instructions in the reduction proper or only used by the reduction,
  // and RedCost is the cost of the reduction chain instructions alone.
  auto getReductionCost = [&](const VPPHINode &Phi, const VPLoop *VPL) {
    VPInstructionCost RedCost, OpCost;
    // First do a depth-first traversal from the PHI to collect instructions
    // in the chain and their associated costs.
    df_iterator_default_set<VPUser *> Visited;
    SmallVector<VPInstruction *, 8> ReducChain;
    auto *U = const_cast<VPUser *>(cast<VPUser>(&Phi));
    for (auto It = df_ext_begin(U, Visited), End = df_ext_end(U, Visited);
         It != End;) {
      if (auto *Inst = dyn_cast<VPInstruction>(*It)) {
        if (!VPL->contains(Inst)) {
          Visited.completed(*It);
          It.skipChildren();
          continue;
        }
        ReducChain.push_back(Inst);
        RedCost += getTTICost(Inst);
      }
      ++It;
    }
    // Now walk back up the chain collecting operand costs. We traverse
    // up through instructions not in the chain and only used by the
    // reduction. We repurpose ReducChain as the stack for traversal.
    VPInstructionCost OperandCost = 0;
    while (!ReducChain.empty()) {
      auto *Inst = ReducChain.back();
      ReducChain.pop_back();
      for (unsigned I = 0; I < Inst->getNumOperands(); I++)
        if (auto *DefInst = dyn_cast<VPInstruction>(Inst->getOperand(I))) {
          if (!VPL->contains(DefInst) || Visited.count(DefInst))
            continue;
          auto It =
              llvm::find_if(DefInst->users(), [&Visited](const VPUser *U) {
                return isa<VPInstruction>(U) &&
                       !Visited.count(cast<VPInstruction>(U));
              });
          if (It == DefInst->user_end()) {
            OpCost += getTTICost(DefInst);
            Visited.insert(DefInst);
            ReducChain.push_back(DefInst);
          }
        }
    }

    LLVM_DEBUG(dbgs() << "PartialSumAnalysis: found candidate\n";
               dbgs() << "  PHI: "; Phi.dump();
               dbgs() << "  Recurrence cost = " << RedCost
                      << ", Operand cost = " << OpCost << "\n");
    return std::make_pair(OpCost + RedCost, RedCost);
  };

  AnalyzedPlan = &Plan;
  Candidates.clear();
  auto *Top = *(Plan.getVPLoopInfo()->begin());
  for (const VPLoop *VPL : post_order(Top)) {
    if (VPL->getLoopDepth() != 1)
      continue;

    // Loop through PHI nodes to classify and potentially compute
    // costs for recurrences.
    for (const VPPHINode &PhiNode : VPL->getHeader()->getVPPhis()) {
      assert(PhiNode.getNumIncomingValues() == 2 &&
             "A loop header is expected to have two predecessors.");
      Type *Ty = PhiNode.getType();
      if (!VectorType::isValidElementType(Ty->getScalarType()))
        continue;

      if (VPReductionFinal *VPRF =
              VPlanLoopUnroller::getPartialSumReducFinal(*VPL, PhiNode)) {
        auto Costs = getReductionCost(PhiNode, VPL);
        Candidates[&PhiNode] = {VPRF, Costs.first, Costs.second};
      }
    }
  }
}

} // namespace VPlanCostModelHeuristics

} // namespace vpo

} // namespace llvm

#endif // INTEL_FEATURE_SW_ADVANCED
