//===-- IntelVPlanCostModelProprietary.cpp --------------------------------===//
//
//   Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements VPlan cost modeling with Intel's IP.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanCostModelProprietary.h"
#include "IntelVPlan.h"
#include "IntelVPlanIdioms.h"
#include "IntelVPlanVLSAnalysis.h"
#include "VPlanHIR/IntelVPlanVLSAnalysisHIR.h"
#include "VPlanHIR/IntelVPlanVLSClientHIR.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Analysis/VectorUtils.h"
#include "IntelVPlanPatternMatch.h"
#include <numeric>
#include <stack>

#define DEBUG_TYPE "vplan-cost-model-proprietary"

static cl::opt<bool> UseOVLSCM(
  "vplan-cm-use-ovlscm", cl::init(true),
  cl::desc("Consider cost returned by OVLSCostModel "
           "for optimized gathers and scatters."));

static cl::opt<unsigned> BoolInstsBailOut(
  "vplan-cost-model-i1-bail-out-limit", cl::init(45), cl::Hidden,
  cl::desc("Don't vectorize if number of boolean computations in the VPlan "
           "is higher than the threshold."));

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

static cl::opt<unsigned> NumberOfSpillsPerExtraReg(
    "vplan-cost-model-number-of-spills-per-extra-reg", cl::init(2), cl::Hidden,
    cl::desc("The number of spills/fills generated on average for each HW "
             "register spilled and restored."));

using namespace llvm::loopopt;
using namespace llvm::PatternMatch;

namespace llvm {

namespace vpo {

bool VPlanCostModelProprietary::isUnitStrideLoadStore(
  const VPInstruction *VPInst,
  bool &NegativeStride) const {
  unsigned Opcode = VPInst->getOpcode();
  assert((Opcode == Instruction::Load || Opcode == Instruction::Store) &&
         "Is not load or store instruction.");

  if (VPlanCostModel::isUnitStrideLoadStore(VPInst, NegativeStride))
    return true;

  if (!VPInst->HIR.isMaster())
    return false;

  if (auto Inst = dyn_cast<HLInst>(VPInst->HIR.getUnderlyingNode())) {
    // FIXME: It's not correct to getParentLoop() for outerloop
    // vectorization.
    if (!Inst->getParentLoop()) {
      return false;
    }
    assert(Inst->getParentLoop()->isInnermost() &&
           "Outerloop vectorization is not supported.");
    unsigned NestingLevel = Inst->getParentLoop()->getNestingLevel();

    return Opcode == Instruction::Load ?
      Inst->getOperandDDRef(1)->isUnitStride(NestingLevel, NegativeStride) :
      Inst->getLvalDDRef()->isUnitStride(NestingLevel, NegativeStride);
  }
  return false;
}

unsigned VPlanCostModelProprietary::getArithmeticInstructionCost(
  const unsigned Opcode,
  const VPValue *Op1,
  const VPValue *Op2,
  const Type *ScalarTy,
  const unsigned VF) {
  unsigned BaseCMCost = VPlanCostModel::getArithmeticInstructionCost(
    Opcode, Op1, Op2, ScalarTy, VF);

  if (BaseCMCost == UnknownCost)
    return BaseCMCost;

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
  if (VF > 1 &&
      (Opcode == Instruction::UDiv || Opcode == Instruction::SDiv ||
       Opcode == Instruction::URem || Opcode == Instruction::SRem) &&
      TLI->isSVMLEnabled()) {
    unsigned ElemSize = ScalarTy->getPrimitiveSizeInBits();
    if (ElemSize != 32 && ElemSize != 64)
      return BaseCMCost;

    unsigned ScalarCost = VPlanCostModel::getArithmeticInstructionCost(
      Opcode, Op1, Op2, ScalarTy, 1);

    if (ScalarCost == UnknownCost)
      return ScalarCost;

    unsigned VectorCost;

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
    return std::min(VectorCost, BaseCMCost);
  }

  return BaseCMCost;
}

const RegDDRef* VPlanCostModelProprietary::getHIRMemref(
  const VPInstruction *VPInst) {
  unsigned Opcode = VPInst->getOpcode();
  if (Opcode != Instruction::Load && Opcode != Instruction::Store)
    return nullptr;

  if (!VPInst->HIR.isMaster())
    return nullptr;
  auto *HInst = dyn_cast<HLInst>(VPInst->HIR.getUnderlyingNode());

  if (!HInst)
    return nullptr;

  return Opcode == Instruction::Load ? HInst->getOperandDDRef(1) :
                                       HInst->getLvalDDRef();
}

unsigned VPlanCostModelProprietary::getLoadStoreCost(
  const VPInstruction *VPInst, const bool UseVLSCost) {
  unsigned Cost = VPlanCostModel::getLoadStoreCost(VPInst);

  if (!UseOVLSCM || !VLSCM || !UseVLSCost || VF == 1)
    return Cost;

  OVLSGroup *Group = VLSA->getGroupsFor(Plan, VPInst);
  if (!Group || Group->size() <= 1)
    return Cost;

  if (ProcessedOVLSGroups.count(Group) != 0) {
    // Group cost has already been assigned.
    LLVM_DEBUG(dbgs() << "Group cost for "; VPInst->print(dbgs());
               dbgs() << " has already been taken into account.\n");
    return ProcessedOVLSGroups[Group] ? 0 : Cost;
  }

  unsigned VLSGroupCost = OptVLSInterface::getGroupCost(*Group, *VLSCM);
  unsigned TTIGroupCost = 0;
  for (OVLSMemref *OvlsMemref : Group->getMemrefVec())
    TTIGroupCost += VPlanCostModel::getLoadStoreCost(
      cast<VPVLSClientMemref>(OvlsMemref)->getInstruction());

  if (VLSGroupCost >= TTIGroupCost) {
    LLVM_DEBUG(dbgs() << "Cost for "; VPInst->print(dbgs());
               dbgs() << " was not reduced from " << Cost <<
               " (TTI group cost " << TTIGroupCost <<
               ") to group cost " << VLSGroupCost << '\n');
    ProcessedOVLSGroups[Group] = false;
    return Cost;
  }

  LLVM_DEBUG(dbgs() << "Reduced cost for "; VPInst->print(dbgs());
             dbgs() << " from " << Cost << " (TTI group cost " <<
             TTIGroupCost << " to group cost " << VLSGroupCost << '\n');
  ProcessedOVLSGroups[Group] = true;
  return VLSGroupCost;
}

unsigned VPlanCostModelProprietary::getCost(const VPInstruction *VPInst) {
  if (VPInst->getType()->isIntegerTy(1))
    ++NumberOfBoolComputations;

  unsigned Opcode = VPInst->getOpcode();
  switch (Opcode) {
  case Instruction::Load:
  case Instruction::Store:
    return getLoadStoreCost(VPInst, true);
  // TODO: So far there's no explicit representation for reduction
  // initializations and finalizations. Need to account overhead for such
  // instructions, until VPlan is ready to have explicit representation for
  // that.
  default:
    return VPlanCostModel::getCost(VPInst);
  }
}

// The utility below searches for patterns that form PSADBW instruction
// semantics.  The current implementation searches for the following pattern:
//
// %Add    = ZExt(A) + ZExt(B) * (-1)
// %Abs    = Abs(%Add)
//
// Returns true if pattern is detected.
// Any outside users of instructions forming the pattern are ignored.
// The utility captures participating intructions into PatternInsts.
bool VPlanCostModelProprietary::checkPsadwbPattern(
   const VPInstruction *AbsInst,
   SmallPtrSetImpl<const VPInstruction*> &PatternInsts) {
  const VPInstruction *Add, *Mul, *ZExtA, *ZExtB;
  const VPValue *A, *B;

  auto MAdd =
    m_Bind(m_c_Add(m_Bind(m_ZExt(m_Bind(A)), ZExtA),
                   m_Bind(m_c_Mul(m_Bind(m_ZExt(m_Bind(B)), ZExtB),
                                  m_ConstantInt<-1, VPConstantInt>()),
                          Mul)), Add);
  if (!match(AbsInst, m_VPAbs(MAdd)))
    return false;

  if (A->getType()->getScalarSizeInBits() != 8 ||
      B->getType()->getScalarSizeInBits() != 8)
    return false;

  // Store participating instructions into PatternInsts.
  PatternInsts.insert(AbsInst);
  PatternInsts.insert(Add);
  PatternInsts.insert(Mul);
  PatternInsts.insert(ZExtA);
  PatternInsts.insert(ZExtB);
  return true;
}

// Does all neccesary target checks and return correction to VPlan Cost for
// possible PSADWB patterns.
unsigned VPlanCostModelProprietary::getPsadwbPatternCost() {

  // Don't apply bonus on VF != 1 plan as we exactly want to keep scalar
  // VPlan in case psadbw pattern is found.
  if (VF != 1)
    return 0;

  unsigned Cost = 0;

  // PSADBW cost in terms of number of intructions.
  const unsigned PsadbwCost = 1;
  NumberOfBoolComputations = 0;

  const VPLoop *TopLoop = *(Plan->getVPLoopInfo()->begin());
  for (const VPLoop *VPL : post_order(TopLoop)) {
    assert(VPL->getLoopDepth() == 1 && "Innermost loop only is expected.");
    if (VPL->getLoopDepth() != 1)
      continue;

    // If loop has known number of iterations and it is 8/16 we skip it here
    // since vectorizing by VPlan leads to complete unroll and ISel can handle
    // vectorized by VPlan code (i.e. recognize PSADBW).  Also if the loop has
    // an outer loop of known trip count it also can be completely unrolled
    // causing better performance comparing to PSADDBW in a loop.
    TripCountInfo LoopTCI = VPL->getTripCountInfo();
    if (!LoopTCI.IsEstimated &&
        (LoopTCI.TripCount == 8 || LoopTCI.TripCount == 16))
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
        const VPInstruction *LHS, *RHS;
        if (match(&VPInst, m_ZExt(m_Bind(LHS))) &&
            CurrPsadbwPatternInsts.count(LHS) > 0)
          CurrPsadbwPatternInsts.insert(&VPInst);
        else if (match(&VPInst, m_Add(m_Bind(LHS),
                                      m_Bind(RHS))) &&
                 CurrPsadbwPatternInsts.count(LHS) > 0 &&
                 CurrPsadbwPatternInsts.count(RHS) > 0)
          CurrPsadbwPatternInsts.insert(&VPInst);
      }

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
          unsigned InstCost = getCost(VPInst);
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
          Cost += CurrentPatternCost - PsadbwCost;
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

  return Cost;
}

unsigned VPlanCostModelProprietary::getGatherScatterCost() {
  if (VF == 1)
    return 0;

  unsigned Cost = 0;
  for (auto *Block : depth_first(Plan->getEntryBlock()))
    // FIXME: Use Block Frequency Info (or similar VPlan-specific analysis) to
    // correctly scale the cost of the basic block.
    Cost += getGatherScatterCost(Block);
  return Cost;
}

unsigned VPlanCostModelProprietary::getGatherScatterCost(
  const VPBasicBlock *VPBlock) {
  if (VF == 1)
    return 0;

  unsigned Cost = 0;
  for (const VPInstruction &VPInst : *VPBlock)
    Cost += getGatherScatterCost(&VPInst);
  return Cost;
}

unsigned VPlanCostModelProprietary::getGatherScatterCost(
  const VPInstruction *VPInst) {
  if (VF == 1)
    return 0;

  bool NegativeStride;
  if ((VPInst->getOpcode() == Instruction::Load ||
       VPInst->getOpcode() == Instruction::Store) &&
      !isUnitStrideLoadStore(VPInst, NegativeStride))
    // NOTE:
    // VLS groups are ignored here.  We may want to take into consideration
    // that some memrefs are parts of VLS groups eventually.
    return VPlanCostModel::getLoadStoreCost(VPInst);
  return 0;
}

// Right now it calls for VPlanCostModel::getCost(VPBB), but later we may want
// to have more precise cost estimation for VPBB.
unsigned VPlanCostModelProprietary::getCost(const VPBasicBlock *VPBB) {
  return VPlanCostModel::getCost(VPBB);
}

unsigned VPlanCostModelProprietary::getSpillFillCost(
  const VPBasicBlock *VPBlock,
  DenseMap<const VPInstruction*, int>& LiveValues) {
  int NumberLiveValuesMax = 0;
  VPLoop *OuterMostVPLoop = *(Plan->getVPLoopInfo()->begin());
  auto PHIs = (cast<VPBasicBlock>(OuterMostVPLoop->getHeader()))->getVPPhis();
  int NumberPHIs = std::distance(PHIs.begin(), PHIs.end());
  int FreeVecHWRegsNum =
    TTI->getNumberOfRegisters(TTI->getRegisterClassForType(true)) - NumberPHIs;

  // TODO:
  // SVA should be utilized to calculate GPR pressure separately (i.e. detect
  // instructions that we don't vectorize and that go to GPR instead of
  // vector registers).
  for (const VPInstruction &VPInst : reverse(*VPBlock)) {
    // Zero-cost and unknown-cost instructions are ignored.  That might be
    // pseudo inst that don't induce real code on output.
    //
    // Use TTI Cost model as Proprietary cost can be 0 for loads/stores that
    // are part of OVLS group.
    unsigned InstCost = VPlanCostModel::getCost(&VPInst);
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
    // N = 16   ==>   RP = 12
    // N > 16   ==>   RP = N / 2
    auto TranslateVPInstRPToHWRP =
      [](unsigned VPInstRP) -> unsigned {
      switch (VPInstRP) {
        case 1:
        case 2:
        case 4:
          return VPInstRP;
        case 8:
          return 6;
        case 16:
          return 12;
        default:
          return VPInstRP / 2;
      }
    };

    for (auto *Op : VPInst.operands()) {
      // TODO:
      // Constants yet to be supported.
      if (!isa<VPInstruction>(Op))
        continue;

      const VPInstruction *OpInst = cast<VPInstruction>(Op);
      InstCost = VPlanCostModel::getCost(OpInst);
      if (InstCost == UnknownCost || InstCost == 0)
        continue;

      Type *OpScalTy = OpInst->getType()->getScalarType();

      if (VectorType::isValidElementType(OpScalTy))
        LiveValues[OpInst] = TranslateVPInstRPToHWRP(
          TTI->getNumberOfParts(getWidenedType(OpInst->getType(), VF)));
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
    // is Load/Store.  Doesn't apply to other costly instructions such as Mul
    // or Div.
    //
    // TODO: the model doesn't apply to Instructions that remain as a call to a
    // library (such as SVML).  For call cases we need estimation of how many
    // registers such call requires.
    if ((VPInst.getOpcode() == Instruction::Load ||
         VPInst.getOpcode() == Instruction::Store) &&
        (InstCost = VPlanCostModel::getCost(&VPInst)) > 1)
      NumberLiveValuesCur = TranslateVPInstRPToHWRP(
        InstCost * NumberLiveValuesCur);

    LLVM_DEBUG(auto LVNs = make_second_range(LiveValues);
               dbgs() << "RP = " << NumberLiveValuesCur << ", LV# = " <<
               llvm::count_if(LVNs, [](int Elem) -> bool {
                   return Elem != 0;
                 }) << " for ";
               VPInst.print(dbgs()); dbgs() << '\n';
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

  unsigned AS = DL->getAllocaAddrSpace();
  unsigned RegBitWidth = TTI->getLoadStoreVecRegBitWidth(AS);
  unsigned RegByteWidth = RegBitWidth / 8;
  Type *VecTy = getWidenedType(Type::getInt8Ty(*Plan->getLLVMContext()),
                               RegByteWidth);
  unsigned StoreCost = TTI->getMemoryOpCost(
    Instruction::Store, VecTy, Align(RegByteWidth), AS);
  unsigned LoadCost = TTI->getMemoryOpCost(
    Instruction::Load, VecTy, Align(RegByteWidth), AS);

  return NumberOfSpillsPerExtraReg *
    (NumberLiveValuesMax - FreeVecHWRegsNum) *
    (StoreCost + LoadCost);
}

unsigned VPlanCostModelProprietary::getSpillFillCost() {
  NumberOfBoolComputations = 0;
  unsigned Cost = 0;
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
  DenseMap<const VPInstruction*, int> LiveValues;

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
    Cost += getSpillFillCost(Block, LiveValues);
  }

  return Cost;
}

unsigned VPlanCostModelProprietary::getCost() {
  NumberOfBoolComputations = 0;
  ProcessedOVLSGroups.clear();
  unsigned Cost = VPlanCostModel::getCost();

  // Array ref which needs to be aligned via loop peeling, if any.
  RegDDRef *PeelArrayRef = nullptr;
  switch (VPlanIdioms::isSearchLoop(Plan, VF, true, PeelArrayRef)) {
  case VPlanIdioms::Unsafe:
    return UnknownCost;
  case VPlanIdioms::SearchLoopStrEq:
    // Without proper type information, cost model cannot properly compute the
    // cost, thus hard code VF.
    if (VF == 1)
      return 1000;
    if (VF != 32)
      // Return some huge value, so that VectorCost still could be computed.
      return UnknownCost;
    break;
  case VPlanIdioms::SearchLoopStructPtrEq:
    // Without proper type information, cost model cannot properly compute the
    // cost, thus hard code VF.
    if (VF == 1)
      return 1000;
    if (VF != 4)
      // Return some huge value, so that VectorCost still could be computed.
      return UnknownCost;
    break;
  default:
    // FIXME: Keep VF = 32 as unsupported right now due to huge perf
    // regressions.
    if (VF == 32)
      return UnknownCost;
  }

  LLVM_DEBUG(dbgs() << "Number of i1 calculations: " << NumberOfBoolComputations
                    << "\n");
  if (VF != 1 && NumberOfBoolComputations >= BoolInstsBailOut) {
    LLVM_DEBUG(
        dbgs() << "Returning UnknownCost due to too many i1 calculations.\n");
    return UnknownCost;
  }

  unsigned TTICost = Cost;
  unsigned GatherScatterCost = getGatherScatterCost();
  // Double GatherScatter cost contribution in case Gathers/Scatters take too
  // much to make it harder to choose this VF.
  if (TTICost * CMGatherScatterThreshold < GatherScatterCost * 100)
    Cost += CMGatherScatterPenaltyFactor * GatherScatterCost;

  Cost += getSpillFillCost();

  // Go though all instructions again to find obvious SLP patterns.
  if (CheckForSLPExtraCost())
    Cost += (VF - 1) * TTICost;

  // getPsadwbPatternCost() can return more than we have in Cost, so check for
  // underflow.
  unsigned PsadwbPatternCost = getPsadwbPatternCost();
  if (Cost > PsadwbPatternCost)
    Cost -= PsadwbPatternCost;
  else
    Cost = 0;

  return Cost;
}

bool VPlanCostModelProprietary::findSLPHIRPattern(
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

bool VPlanCostModelProprietary::ProcessSLPHIRMemrefs(
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

bool VPlanCostModelProprietary::CheckForSLPExtraCost() const {
  if (VF == 1)
    return false;

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
    return true;

  return false;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPlanCostModelProprietary::printForVPInstruction(
  raw_ostream &OS, const VPInstruction *VPInst) {
  OS << "  Cost " << getCostNumberString(getCost(VPInst)) << " for ";
  VPInst->print(OS);

  std::string Attributes = "";

  if (getGatherScatterCost(VPInst) > 0)
    Attributes += "GS ";

  if (OVLSGroup *Group = VLSA->getGroupsFor(Plan, VPInst))
    if (ProcessedOVLSGroups.count(Group) != 0)
      Attributes += "OVLS ";

  if (PsadbwPatternInsts.count(VPInst) > 0)
    Attributes += "PSADBW ";

  if (!Attributes.empty())
    OS << " ( " << Attributes << ")";
  OS << '\n';
}

void VPlanCostModelProprietary::printForVPBasicBlock(
  raw_ostream &OS, const VPBasicBlock *VPBB) {
  OS << "Analyzing VPBasicBlock " << VPBB->getName() << ", total cost: " <<
    getCostNumberString(getCost(VPBB)) << '\n';

  unsigned GatherScatterCost = getGatherScatterCost(VPBB);
  if (GatherScatterCost > 0)
    OS << "total cost includes GS Cost: " << GatherScatterCost << '\n';

  DenseMap<const VPInstruction*, int> LiveValues;
  unsigned SpillFillCost = getSpillFillCost(VPBB, LiveValues);
  if (SpillFillCost > 0)
    OS << "Block spill/fill approximate cost (not included into total cost): "
       << SpillFillCost << '\n';

  // Clearing ProcessedOVLSGroups is valid while VLS works within a basic block.
  // TODO: The code should be revisited once the assumption is changed.
  // Clearing before Instruction traversal is required to allow Instruction
  // dumping function to print out correct Costs and not required for CM to
  // work properly.
  ProcessedOVLSGroups.clear();

  for (const VPInstruction &VPInst : *VPBB)
    if (PrintTerminatorInst || !isa<VPBranchInst>(VPInst))
      printForVPInstruction(OS, &VPInst);
}

void VPlanCostModelProprietary::print(
  raw_ostream &OS, const std::string &Header) {
  unsigned GatherScatterCost = getGatherScatterCost();
  unsigned SpillFillCost = getSpillFillCost();
  unsigned TTICost = VPlanCostModel::getCost();
  OS << "HIR Cost Model for VPlan " << Header << " with VF = " << VF << ":\n";
  OS << "Total VPlan Cost: " << getCost() << '\n';
  OS << "VPlan Base Cost before adjustments: " << TTICost << '\n';

  if (GatherScatterCost > 0)
    OS << "VPlan Base Cost includes Total VPlan GS Cost: " <<
      GatherScatterCost << '\n';
  if (TTICost * CMGatherScatterThreshold < GatherScatterCost * 100)
    OS << "Total VPlan GS Cost is bumped: +" <<
      CMGatherScatterPenaltyFactor * GatherScatterCost << '\n';
  if (CheckForSLPExtraCost())
    OS << "SLP breaking penalty cost: +" << (VF - 1) * TTICost << '\n';
  if (SpillFillCost)
    OS << "Total VPlan spill/fill cost: +" << SpillFillCost << '\n';
  unsigned PsadbwCostAdj = getPsadwbPatternCost();
  if (PsadbwCostAdj > 0)
    OS << "PSADBW pattern adjustment: -"  << PsadbwCostAdj << '\n';

  LLVM_DEBUG(dbgs() << *Plan;);

  // TODO: match print order with "vector execution order".
  for (const VPBasicBlock *Block : depth_first(Plan->getEntryBlock()))
    printForVPBasicBlock(OS, Block);
  OS << '\n';
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

} // namespace vpo

} // namespace llvm
