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
#include "IntelVPlanUtils.h"
#include "IntelVPlanPatternMatch.h"

#include <numeric>

#define DEBUG_TYPE "vplan-cost-model-heuristics"

using namespace loopopt;
using namespace llvm::PatternMatch;

static cl::opt<unsigned> NumberOfSpillsPerExtraReg(
    "vplan-cost-model-number-of-spills-per-extra-reg", cl::init(2), cl::Hidden,
    cl::desc("The number of spills/fills generated on average for each HW "
             "register spilled and restored."));

static cl::opt<unsigned> CMGatherScatterThreshold(
  "vplan-cm-gather-scatter-threshold", cl::init(50),
  cl::desc("If gather/scatter cost is more than CMGatherScatterThreshold "
           "percent of whole loop price the price of gather/scatter is "
           "doubled to make it harder to choose in favor of "
           "loop with gathers/scatters."));

static cl::opt<unsigned> CMGatherScatterPenaltyFactor(
  "vplan-cm-gather-scatter-penalty-factor", cl::init(2), cl::Hidden,
  cl::desc("The factor which G/S cost multiplies by if G/S accumulated cost "
           "exceeds CMGatherScatterThreshold."));

namespace llvm {

namespace vpo {

namespace VPlanCostModelHeuristics {

HeuristicBase::HeuristicBase(VPlanTTICostModel *CM, std::string Name) :
  CM(CM) {
  Plan = CM->Plan;
  VF = CM->VF;
  UnknownCost = CM->UnknownCost;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  this->Name = Name;
#else
  (void)Name;
#endif // !NDEBUG || LLVM_ENABLE_DUMP
}

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

void HeuristicSLP::apply(
  unsigned TTICost, unsigned &Cost, const VPlanVector *Plan) const {
  (void)TTICost;
  SmallVector<const RegDDRef*, VPlanSLPSearchWindowSize> HIRLoadMemrefs;
  SmallVector<const RegDDRef*, VPlanSLPSearchWindowSize> HIRStoreMemrefs;
  // Gather all Store and Load Memrefs since SLP starts pattern search on
  // stores and on our cases we have consequent loads as well.
  for (const VPBasicBlock *Block : depth_first(Plan->getEntryBlock()))
    for (const VPInstruction &VPInst : *Block)
      if (auto DDRef = getHIRMemref(&VPInst)) {
        if (VPInst.getOpcode() == Instruction::Store)
          HIRStoreMemrefs.push_back(DDRef);
        else if (VPInst.getOpcode() == Instruction::Load)
          HIRLoadMemrefs.push_back(DDRef);
      }

  if (ProcessSLPHIRMemrefs(HIRStoreMemrefs, VPlanSLPStorePatternSize) &&
      ProcessSLPHIRMemrefs(HIRLoadMemrefs,  VPlanSLPLoadPatternSize))
    Cost *= VF;
}

void HeuristicSearchLoop::apply(
  unsigned TTICost, unsigned &Cost, const VPlanVector *Plan) const {
  (void)TTICost;
  // Array ref which needs to be aligned via loop peeling, if any.
  RegDDRef *PeelArrayRef = nullptr;
  switch (VPlanIdioms::isSearchLoop(Plan, VF, true, PeelArrayRef)) {
  case VPlanIdioms::Unsafe:
    Cost = UnknownCost;
    break;
  case VPlanIdioms::SearchLoopStrEq:
    // Without proper type information, cost model cannot properly compute the
    // cost, thus hard code VF.
    if (VF == 1)
      Cost = VPlanTTIWrapper::Multiplier * 1000;
    else if (VF != 32)
      // Return some huge value, so that VectorCost still could be computed.
      Cost = UnknownCost;
    break;
  case VPlanIdioms::SearchLoopStructPtrEq:
    // Without proper type information, cost model cannot properly compute the
    // cost, thus hard code VF.
    if (VF == 1)
      Cost = VPlanTTIWrapper::Multiplier * 1000;
    else if (VF != 4)
      // Return some huge value, so that VectorCost still could be computed.
      Cost = UnknownCost;
    break;
  default:
    // FIXME: Keep VF = 32 as unsupported right now due to huge perf
    // regressions.
    if (VF == 32)
      Cost = UnknownCost;
  }
}

unsigned HeuristicSpillFill::operator()(
  const VPBasicBlock *VPBlock,
  LiveValuesTy &LiveValues,
  bool VectorRegsPressure) const {
  int NumberLiveValuesMax = 0;
  VPLoop *OuterMostVPLoop = *(Plan->getVPLoopInfo()->begin());
  auto SkipInst = [&](const VPInstruction* I) -> bool {
    // Ignore instructions that do not induce vector code if VectorRegsPressure
    // is true or ignore 'vector only' instructions for VectorRegsPressure
    // false.
    //
    // TODO:
    // VF checks should be removed eventually. It is a bug in the current
    // SVA implementation that we need them.
    //
    // TODO:
    // The implementation doesn't cover scalarization cases, when vector
    // instruction going to be scalarized contributing into scalar registers
    // pressure, not vector.
    //
    // TODO:
    // SVA marks input vector types that are not re-vectorized as
    // NeedsFirst/LastScalarCode while they still contribute into vector
    // register pressure, not scalar.
    //
    bool NeedsVecInst =
      (VF > 1) && Plan->getVPlanSVA()->instNeedsVectorCode(I);
    bool NeedsScalInst =
      (VF == 1) || Plan->getVPlanSVA()->instNeedsFirstScalarCode(I);
    return (VectorRegsPressure && !NeedsVecInst) ||
           (!VectorRegsPressure && !NeedsScalInst);
  };
  auto PHIs = (cast<VPBasicBlock>(OuterMostVPLoop->getHeader()))->getVPPhis();
  int NumberPHIs = llvm::count_if(PHIs, [&](auto& PHI) {
    return !SkipInst(&PHI);});
  int FreeVecHWRegsNum = CM->VPTTI.getNumberOfRegisters(
    CM->VPTTI.getRegisterClassForType(VectorRegsPressure)) - NumberPHIs;

  for (const VPInstruction &VPInst : reverse(*VPBlock)) {
    if (SkipInst(&VPInst))
      continue;

    // Zero-cost and unknown-cost instructions are ignored.  That might be
    // pseudo inst that don't induce real code on output.
    //
    // Use TTI Cost model as Proprietary cost can be 0 for loads/stores that
    // are part of OVLS group.
    unsigned InstCost = CM->getTTICost(&VPInst);
    if (InstCost == UnknownCost || InstCost == 0)
      continue;

    // Once definition is met the value is marked dead as the result of
   // instruction generally can occupy the same register as one of its
    // operand, unless all its operands are alive throughout the intruction.
    LiveValues[&VPInst] = 0;

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

    for (auto *Op : VPInst.operands()) {
      // TODO:
      // Constants yet to be supported.
      if (!isa<VPInstruction>(Op))
        continue;

      const VPInstruction *OpInst = cast<VPInstruction>(Op);
      unsigned OpInstCost = CM->getTTICost(OpInst);
      if (OpInstCost == UnknownCost || OpInstCost == 0)
        continue;

      Type *OpScalTy = OpInst->getType()->getScalarType();

      if (VectorType::isValidElementType(OpScalTy))
        LiveValues[OpInst] = TranslateVPInstRPToHWRP(
          CM->VPTTI.getNumberOfParts(getWidenedType(OpInst->getType(), VF)));
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
      [&](const VPInstruction &VPInst) -> bool {
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
      Type *VTy = getWidenedType(getLoadStoreType(&VPInst), VF);
      bool NegativeStride = false;

      // Check for masked unit load/store presence in HW.
      if (CM->isUnitStrideLoadStore(&VPInst, NegativeStride)) {
        if ((IsMasked && IsLoad  &&
             !CM->VPTTI.isLegalMaskedLoad(VTy, Alignment)) ||
            (IsMasked && IsStore &&
             !CM->VPTTI.isLegalMaskedStore(VTy, Alignment)))
          return true;
      }
      // Check for unsupported gather/scatter instruction.
      // Note: any gather/scatter is considered as masked.
      else if ((IsLoad  && !CM->VPTTI.isLegalMaskedGather(VTy, Alignment)) ||
               (IsStore && !CM->VPTTI.isLegalMaskedScatter(VTy, Alignment)))
        return true;

      return false;
    };

    if ((VPInst.getOpcode() == Instruction::Load ||
         VPInst.getOpcode() == Instruction::Store) &&
        SerializableLoadStore(VPInst))
      NumberLiveValuesCur += TranslateVPInstRPToHWRP(
        // VPlanTTIWrapper::estimateNumberOfInstructions(InstCost) gives
        // an estimation of the number of instructions the serialized
        // load/store is implemented with.
        VPlanTTIWrapper::estimateNumberOfInstructions(InstCost));

    LLVM_DEBUG(auto LVNs = make_second_range(LiveValues);
               dbgs() << "RP = " << NumberLiveValuesCur << ", LV# = " <<
               llvm::count_if(LVNs, [](int Elem) -> bool {
                   return Elem != 0;
                 }) << " for ";
               VPInst.printWithoutAnalyses(dbgs()); dbgs() << '\n';
               dbgs() << "Live vals:";
               for (auto LiveValue : LiveValues)
                 if (LiveValue.second) {
                   dbgs() << ' ';
                   LiveValue.first->printAsOperand(dbgs());
                 }
               dbgs() << '\n';);

    NumberLiveValuesMax = std::max(NumberLiveValuesCur, NumberLiveValuesMax);
  }

  LLVM_DEBUG(dbgs() << "Max RP " << NumberLiveValuesMax <<
             ", Num free regs: " << FreeVecHWRegsNum <<
             " for block " << VPBlock->getName() << " (VF = " << VF << ")\n";);

  if (NumberLiveValuesMax <= FreeVecHWRegsNum)
    return 0;

  unsigned AS = CM->DL->getAllocaAddrSpace();
  unsigned RegBitWidth = CM->VPTTI.getLoadStoreVecRegBitWidth(AS);
  unsigned RegByteWidth = RegBitWidth / 8;
  Type *VecTy = getWidenedType(Type::getInt8Ty(*Plan->getLLVMContext()),
                               RegByteWidth);
  unsigned StoreCost = CM->VPTTI.getMemoryOpCost(
    Instruction::Store, VecTy, Align(RegByteWidth), AS);
  unsigned LoadCost = CM->VPTTI.getMemoryOpCost(
    Instruction::Load, VecTy, Align(RegByteWidth), AS);

  return NumberOfSpillsPerExtraReg *
    (NumberLiveValuesMax - FreeVecHWRegsNum) *
    (StoreCost + LoadCost);
}

void HeuristicSpillFill::apply(
  unsigned TTICost, unsigned &Cost, const VPlanVector *Plan) const {
  (void)TTICost;
  // Don't run register pressure heuristics on TTI models that do not support
  // scalar or vector registers.
  if (CM->VPTTI.getNumberOfRegisters(
        CM->VPTTI.getRegisterClassForType(false)) == 0 ||
      CM->VPTTI.getNumberOfRegisters(
        CM->VPTTI.getRegisterClassForType(true)) == 0)
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

  for (auto *Block : post_order(Plan->getEntryBlock())) {
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
    Cost += (*this)(Block, ScalLiveValues, false);

    if (VF > 1)
      Cost += (*this)(Block, VecLiveValues, true);
  }
}

void HeuristicGatherScatter::apply(
  unsigned TTICost, unsigned &Cost, const VPlanVector *Plan) const {
  unsigned GSCost = 0;
  for (auto *Block : depth_first(Plan->getEntryBlock()))
    // FIXME: Use Block Frequency Info (or similar VPlan-specific analysis) to
    // correctly scale the cost of the basic block.
    GSCost += (*this)(Block);

  // Double GatherScatter cost contribution in case Gathers/Scatters take too
  // much to make it harder to choose this VF.
  if (TTICost * CMGatherScatterThreshold < GSCost * 100)
    Cost += CMGatherScatterPenaltyFactor * GSCost;
}

unsigned HeuristicGatherScatter::operator()(
  const VPBasicBlock *VPBlock) const {
  if (VF == 1)
    return 0;

  unsigned Cost = 0;
  for (const VPInstruction &VPInst : *VPBlock)
    Cost += (*this)(&VPInst);
  return Cost;
}

unsigned HeuristicGatherScatter::operator()(
  const VPInstruction *VPInst) const {
  if (VF == 1)
    return 0;

  bool NegativeStride;
  if ((VPInst->getOpcode() == Instruction::Load ||
       VPInst->getOpcode() == Instruction::Store) &&
      !CM->isOptimizedVLSGroupMember(VPInst) &&
      !CM->isUnitStrideLoadStore(VPInst, NegativeStride))
    return CM->getLoadStoreCost(VPInst, VF);
  return 0;
}

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

// Does all neccesary target checks and return corrected VPlan Cost.
void HeuristicPsadbw::apply(
  unsigned TTICost, unsigned &Cost, const VPlanVector *Plan) const {
  (void)TTICost;
  unsigned PatternCost = 0;

  // Scaled PSADBW cost in terms of number of intructions.
  const unsigned PsadbwCost = 1 * VPlanTTIWrapper::Multiplier;

  const VPLoop *TopLoop = *(Plan->getVPLoopInfo()->begin());
  for (const VPLoop *VPL : post_order(TopLoop)) {
    assert(VPL->getLoopDepth() == 1 && "Innermost loop only is expected.");
    if (VPL->getLoopDepth() != 1)
      continue;

    VPBasicBlock *Block = VPL->getHeader();
    VPBasicBlock *PH = VPL->getLoopPreheader();
    VPBasicBlock *Latch = VPL->getLoopLatch();

    // Loop through PHI nodes.
    for (const VPPHINode &PhiNode : Block->getVPPhis()) {
      assert(PhiNode.getNumIncomingValues() == 2 &&
             "A loop header is expected to have two predecessors.");

      const auto *RedInitInst =
        dyn_cast<VPReductionInit>(PhiNode.getIncomingValue(PH));
      if (!RedInitInst)
        continue;

      const auto *SumCarryOut =
        dyn_cast<VPInstruction>(PhiNode.getIncomingValue(Latch));
      if (!SumCarryOut || SumCarryOut->getOpcode() != Instruction::Add)
        continue;

      // Check that there is VPReductionFinal among users of SumCarryOut.
      const VPReductionFinal *ReductionFinalInst = nullptr;
      auto It = llvm::find_if(SumCarryOut->users(),
                              [](const auto* SumVPUser) {
                                return (isa<VPReductionFinal>(SumVPUser));
                              });
      if (It == SumCarryOut->users().end())
        continue;

      ReductionFinalInst = cast<VPReductionFinal>(*It);

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
      SmallPtrSet<const VPInstruction*, 32> CurrPsadbwPatternInsts;
      std::stack<const VPInstruction *> AddsStack;

      AddsStack.push(SumCarryOut);

      while (!AddsStack.empty()) {
        const VPInstruction *AddInst = AddsStack.top();
        AddsStack.pop();

        const VPInstruction *LHS = nullptr, *RHS = nullptr;
        // In case of accumulator is 64 bits there are ZExt's in operands of
        // the ADD possible.  Go over them.
        if (!match(AddInst, m_Add(m_ZExtOrSelf(m_Bind(LHS)),
                                  m_ZExtOrSelf(m_Bind(RHS)))))
          continue;

        if (LHS->getOpcode() == Instruction::Add)
          AddsStack.push(LHS);
        if (RHS->getOpcode() == Instruction::Add)
          AddsStack.push(RHS);

        // Add PhiNode, RedInitInst, ReductionFinalInst and AddInst into
        // the list of pattern forming instruction whenever pattern is found
        // along LHS operand or RHS operand of the add.
        //
        // Make sure to run checkPsadwbPattern for LHS and RHS as there could
        // be patterns on the both ways and we want checkPsadwbPattern() to
        // populate CurrPsadbwPatternInsts with corresponding instructions.
        bool PatternFound = checkPsadwbPattern(LHS, CurrPsadbwPatternInsts);
        if (checkPsadwbPattern(RHS, CurrPsadbwPatternInsts) || PatternFound) {
          CurrPsadbwPatternInsts.insert(&PhiNode);
          CurrPsadbwPatternInsts.insert(RedInitInst);
          CurrPsadbwPatternInsts.insert(AddInst);
          CurrPsadbwPatternInsts.insert(ReductionFinalInst);
        }
      }

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

      // Sum up costs of all instructions in CurrPsadbwPatternInsts.
      // If there an instruction in the pattern has more than one use the whole
      // pattern cost is halved.  This way we model external to pattern uses
      // that keeps part of pattern instruction alive even if the whole pattern
      // is going to be replace with psadbw.  On average half of the whole
      // pattern is kept by one external use.  The more external uses the pattern
      // has the less the gain cost of idiom recognition.
      //
      // Note that SumCarryOut has two uses that are both within the pattern
      // (i.e. not external).
      if (!CurrPsadbwPatternInsts.empty()) {
        unsigned CurrentPatternCost = 0;
        unsigned NumberOfExternalUses = 0;

        for (const VPInstruction *VPInst : CurrPsadbwPatternInsts) {
          unsigned InstCost = CM->getTTICost(VPInst);
          if (InstCost != UnknownCost)
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

        if (CurrentPatternCost > PsadbwCost) {
          PatternCost += CurrentPatternCost - PsadbwCost;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
          // When the pattern accepted update PsadbwPatternInsts.
          // Currently PsadbwPatternInsts is used for debug output only.
          PsadbwPatternInsts.insert(CurrPsadbwPatternInsts.begin(),
                                    CurrPsadbwPatternInsts.end());
#endif // !NDEBUG || LLVM_ENABLE_DUMP
        }
      }
    }
  }

  if (PatternCost > Cost)
    // TODO:
    // Consider returning PsadbwCost here.
    Cost = 0;
  else
    Cost -= PatternCost;
}

} // namespace VPlanCostModelHeuristics

} // namespace vpo

} // namespace llvm
