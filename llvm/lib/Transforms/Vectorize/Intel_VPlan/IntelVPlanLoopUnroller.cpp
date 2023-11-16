//===-- IntelVPlanLoopUnroller.cpp ------------------------------*- C++ -*-===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanLoopUnroller.h"
#include "IntelVPlanBuilder.h"
#include "IntelVPlanClone.h"
#include "IntelVPlanUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLInst.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "VPlanLoopUnroller"

static cl::opt<bool> EnablePartialSums(
    "vplan-enable-partial-sums", cl::Hidden, cl::init(true),
    cl::desc("Enable partial sum optimization in loop unroller"));

using namespace llvm::vpo;

static LoopVPlanDumpControl UnrollDumpControl("unroll", "VPlan loop unrolling");

// Given a PHI with two operands, return the operand values as a pair
// with the loop-defined operand first, or a pair of nullptrs if none
// are loop-defined.
static std::pair<VPInstruction *, VPInstruction *>
getHeaderPhiOperands(const VPLoop &VPL, const VPPHINode &Phi) {
  assert(Phi.getNumOperands() == 2 &&
         "All PHIs in loop header block should have only 2 value operands");
  VPInstruction *Op0 = dyn_cast<VPInstruction>(Phi.getOperand(0));
  VPInstruction *Op1 = dyn_cast<VPInstruction>(Phi.getOperand(1));
  if (Op0 && VPL.isLiveOut(Op0) && (!Op1 || !VPL.contains(Op1)))
    return std::make_pair(Op0, Op1);
  else if (Op1 && VPL.isLiveOut(Op1) && (!Op0 || !VPL.contains(Op0)))
    return std::make_pair(Op1, Op0);
  return std::make_pair((VPInstruction *)nullptr, (VPInstruction *)nullptr);
}

namespace {
/// Describes a partial sum candidate, i.e. a reduction which, after
/// unrolling, can be reassociated to use multiple parallel accumulators
/// in the unrolled loop, which are then reduced in the post-exit
/// to produce the final result.
/// We only handle cases where the reduction operation is a single
/// binary instruction, meaning the post-exit code can be generated as
/// (a_0 op (a_1 op .. )) for accumulators a_0 .. a_N.
struct PSumCandidate {
  /// Identifies a candidate reduction operation, given the loop-defined
  /// and initial values of a recurrent PHI.
  /// Returns the VPReductionFinal for the candidate if valid, or null o/w.
  static VPReductionFinal *getReductionFinal(const VPInstruction *LoopVal,
                                             const VPInstruction *InitVal);

  /// Create a candidate given the VPReductionInit and VPReductionFinal
  /// returned from getReductionFinal.
  PSumCandidate(VPReductionInit &RedInit, VPReductionFinal &RedFinal);
  PSumCandidate() = default;

  // The reduction opcode, used to generate the post-exit reductions.
  unsigned Opcode;
  // The VPReductionInit (with the identity element) to use
  // for parallel accumulators.
  VPReductionInit *Init;
  // The type of the recurrence.
  Type *ReductionType;
  // The fast-math flags on the recurrent instruction(s).
  FastMathFlags FMF;
  // The accumulator values, initially just the single reduction.
  SmallVector<VPValue *, 4> Accum;
};
} // namespace

void VPlanLoopUnroller::run() {
  assert(UF > 1 && "Can't unroll with unroll factor less than 2");

  VPBuilder VPBldr;
  VPLoopInfo *VPLI = Plan.getVPLoopInfo();
  VPLoop *VPL = Plan.getMainLoop(true);
  assert(VPL->getSubLoops().empty() &&
         "Unrolling of loops with subloops is not supported");

  // Collect loop live-out users to process them after the unroll.
  // Two containers, the first one is map for cases when we can have only one
  // use, and the second one is vector, for cases when several users can have
  // same operand. The first case corresponds to VPExternalUses, the second - to
  // VPInstructions in loop exit. For example, the last value calculation
  // for minmax+index. Some values calculated inside the loop can be used
  // at least twice, e.g. the main reduction in minmax+index idiom is used
  // to calculate last value of that main reduction itself and also to calculate
  // the value of index part of the idiom. Below is the code snipped with
  // example of VPInstruction for minmax+index with two indexes. Here, %VP0 and
  // %VP2 calculated inside the loop are used twice in the loop exit.
  // i32 %RED_FINAL = reduction-final{u_smax} i32 %VP0
  // i32 %RED_FINAL_1 = reduction-final{s_smin} i32 %VP2 i32 %VP0 i32 %RED_FINAL
  // i32 %RED_FINAL_2 = reduction-final{s_smin} i32 %VP4 i32 %VP2 i32
  // %RED_FINAL_1
  DenseMap<VPUser *, VPInstruction *> LiveOutExtUsers;
  using UseOpPair = std::pair<VPInstruction *, VPInstruction *>;
  SmallVector<UseOpPair, 4> LiveOutInstUsers;

  for (VPBasicBlock *Block : VPL->blocks())
    for (VPInstruction &Inst : *Block)
      for (VPUser *User : Inst.users())
        if (isa<VPExternalUse>(User)) {
          assert(LiveOutExtUsers.find(User) == LiveOutExtUsers.end() &&
                 "Only one instruction for a live-out user is supported");
          LiveOutExtUsers[User] = &Inst;
        } else if (auto UseInst = dyn_cast<VPInstruction>(User))
          if (!VPL->contains(UseInst)) {
            LiveOutInstUsers.push_back(std::make_pair(UseInst, &Inst));
          }

  VPBasicBlock *Header = VPL->getHeader();
  assert(Header && "Expected single header block");
  VPBasicBlock *Exit = VPL->getUniqueExitBlock();
  assert(Exit && "expected unique exit");
  VPBasicBlock *Latch = VPL->getLoopLatch();
  assert(Latch && "Expected single latch block");
  VPBasicBlock *CurrentLatch = Latch;

  // TODO: Support explicit uniform IV in the vectorized loop for HIR pipeline.
  if (auto *Cmp = dyn_cast<VPInstruction>(CurrentLatch->getCondBit()))
    if (auto *VTC = dyn_cast<VPVectorTripCountCalculation>(Cmp->getOperand(1)))
      VTC->setUF(VTC->getUF() * UF);

  SmallVector<VPCloneUtils::Value2ValueMapTy, 8> Clones(UF - 1);
  for (unsigned Part = 0; Part < UF - 1; Part++) {
    VPCloneUtils::cloneBlocksRange(Header, Latch, Clones[Part],
                                   Plan.getVPlanDA());
  }

  // Hold the current last update instruction for each header PHI node.
  DenseMap<VPInstruction *, VPValue *> PHILastUpdate;

  // Hold the reduction operations for each header PHI node for
  // which we will perform the partial sum optimization.
  SmallMapVector<VPPHINode *, PSumCandidate, 8> PSumCandidates;

  // Identify partial sum candidates by their VPReductions, and keep
  // a list of values to be reduced in the exit block.
  if (EnablePartialSums)
    for (VPPHINode &Phi : Header->getVPPhis())
      if (VPReductionFinal *ReducFinal = getPartialSumReducFinal(*VPL, Phi)) {
        auto PhiOpnds = getHeaderPhiOperands(*VPL, Phi);
        // If the original VPReductionInit uses a start value, we'll need
        // to generate one with just the identity.
        VPReductionInit *ReducInit = cast<VPReductionInit>(PhiOpnds.second);
        if (ReducInit->usesStartValue()) {
          VPBldr.setInsertPoint(ReducInit);
          auto IdInit = VPBldr.createReductionInit(
              ReducInit->getIdentityOperand(),
              /*Start=*/nullptr, /*UseStart=*/false, ReducInit->isScalar(),
              ReducInit->isComplex());
          ReducInit = cast<VPReductionInit>(IdInit);
          Plan.getVPlanDA()->markDivergent(*ReducInit);
        }
        // Record the candidate and the original loop-defined result.
        auto It = PSumCandidates.insert(
            std::make_pair(&Phi, PSumCandidate(*ReducInit, *ReducFinal)));
        assert(It.second && "expected unique PHI nodes");
        It.first->second.Accum.push_back(PhiOpnds.first);
      }

  // Main loop. Repeats the loop N times (where N = UF - 1).
  //   For example:
  //     Header <---+
  //     [ phis ]   |
  //     |          |
  //     Body       |
  //     |          |
  //     Latch -----+
  //
  //   Will be expanded into (with UF = 3, N = 2):
  //     Header <----------+
  //     [ phis ]          |
  //     |                 |
  //     Body              |
  //     |                 |
  //     Latch             |
  //     |                 |
  //     Header Clone #1   |
  //     |                 |
  //     Body Clone #1     |
  //     |                 |
  //     Latch Clone #1    |
  //     |                 |
  //     Header Clone #2   |
  //     |                 |
  //     Body Clone #2     |
  //     |                 |
  //     Latch Clone #2 ---+
  for (unsigned Part = 0; Part < UF - 1; Part++) {
    VPCloneUtils::Value2ValueMapTy &ValueMap = Clones[Part];
    VPCloneUtils::Value2ValueMapTy ReverseMap;
    for (const auto &It : ValueMap)
      ReverseMap[It.second] = It.first;
    assert(ReverseMap.size() == ValueMap.size() &&
           "Expecting unique values only in ValueMap");

    auto *ClonedHeader = cast<VPBasicBlock>(ValueMap[Header]);
    auto *ClonedLatch = cast<VPBasicBlock>(ValueMap[Latch]);

    // Process induction/reduction PHI nodes.
    std::set<VPInstruction *> InstToRemove;
    for (VPPHINode &ClonedInst : ClonedHeader->getVPPhis()) {
      auto It = ReverseMap.find(&ClonedInst);
      assert(It != ReverseMap.end() &&
             "ReverseMap should contain all the cloned instructions");

      VPPHINode *OrigInst = dyn_cast<VPPHINode>(It->second);
      assert(OrigInst &&
             "The clone of VPPHINode expected to be a VPPHINode too");

      // Try to deduce induction/reduction PHI node.
      VPInstruction *LastUpdate = getHeaderPhiOperands(*VPL, *OrigInst).first;
      assert(LastUpdate &&
             "Expecting to have only induction/reduction PHIs here");

      if (PHILastUpdate.find(OrigInst) == PHILastUpdate.end())
        PHILastUpdate[OrigInst] = LastUpdate;

      // Update the unrolled loop header PHI's incoming block
      // to cloned loop latch.
      OrigInst->setIncomingBlock(OrigInst->getBlockIndex(CurrentLatch),
                                 ClonedLatch);

      // For partial sum candidates, generate a new PHI in the header
      // and stash the accumulator value for reduction in the exit.
      auto PSIt = PSumCandidates.find(OrigInst);
      if (PSIt != PSumCandidates.end()) {
        VPInstruction *ClonedReduc = cast<VPInstruction>(ValueMap[LastUpdate]);

        // Generate the new PHI in the header to use the candidate's
        // initializer and the current iteration's result.
        VPBldr.setInsertPoint(OrigInst);
        VPPHINode *Phi =
            cast<VPPHINode>(VPBldr.createPhiInstruction(ClonedInst.getType()));
        Phi->addIncoming(PSIt->second.Init, PSIt->second.Init->getParent());
        Phi->addIncoming(ClonedReduc,
                         cast<VPBasicBlock>(Clones[UF - 2][Latch]));
        Plan.getVPlanDA()->markDivergent(*Phi);

        // Add the new PHI to the value map.
        ValueMap[OrigInst] = Phi;

        // Record the accumulator result for post-loop reduction.
        PSIt->second.Accum.push_back(ClonedReduc);
      } else {
        // For the current iteration replace a clone of the original PHI with
        // the current last update instruction.
        ValueMap[OrigInst] = PHILastUpdate[OrigInst];

        // Replace the last update instruction with the cloned one
        // related to the current iteration.
        PHILastUpdate[OrigInst] = ValueMap[LastUpdate];

        // Not actually a live-out, but this will help to replace phi's operand
        // to the final last update instruction
        LiveOutExtUsers[OrigInst] = LastUpdate;
      }

      InstToRemove.insert(&ClonedInst);
    }

    for (auto It : InstToRemove)
      ClonedHeader->eraseInstruction(It);

    // Remap operands.
    VPValueMapper Mapper(ValueMap);
    for (VPBasicBlock *Block : VPL->blocks())
      Mapper.remapOperands(Block);

    // Move forward latch's condition.
    VPValue *CondBit = CurrentLatch->getCondBit();
    assert(CondBit && "The loop latch is expected to have CondBit");

    // Insert cloned blocks into the loop.
    ClonedLatch->setTerminator(CurrentLatch->getSuccessor(0),
                               CurrentLatch->getSuccessor(1), CondBit);
    CurrentLatch->setTerminator(ClonedHeader);

    CurrentLatch = ClonedLatch;
  }

  // TODO: Implement as part of some earlier traversal to save compile time.
  // Add all cloned blocks into the loop.
  for (auto &ValueMap : Clones)
    for (const auto &Pair : ValueMap)
      if (isa<VPBasicBlock>(Pair.first))
        VPL->addBasicBlockToLoop(cast<VPBasicBlock>(Pair.second), *VPLI);

  // Get the last iteration's value map for live-out fixup.
  VPCloneUtils::Value2ValueMapTy &ValueMap = Clones[UF - 2];

  // Reduce the partial sum values for each candidate to a single
  // value to replace the original reduction's live-outs.
  for (auto &It : PSumCandidates) {
    PSumCandidate &PS = It.second;
    VPValue *Orig = PS.Accum[0];
    VPBldr.setInsertPointFirstNonPhi(Exit);
    unsigned NumAccums = PS.Accum.size();

    while (NumAccums > 1) {
      unsigned CurSize = 0, Index;

      for (Index = 0; Index < NumAccums - 1; Index += 2) {
        VPInstruction *VPI;
        if (isMinMaxOpcode(PS.Opcode)) {
          Function *Fn = Intrinsic::getDeclaration(
              Header->getParent()->getModule(),
              getIntrinsicForMinMaxOpcode(PS.Opcode), {PS.ReductionType});
          VPI = VPBldr.createCall(Fn, {PS.Accum[Index], PS.Accum[Index + 1]});
        } else
          VPI = VPBldr.createNaryOp(PS.Opcode, PS.ReductionType,
                                    {PS.Accum[Index], PS.Accum[Index + 1]});
        if (VPI->hasFastMathFlags())
          VPI->setFastMathFlags(PS.FMF);
        PS.Accum[CurSize++] = VPI;
        Plan.getVPlanDA()->markDivergent(*VPI);
      }
      // If last accumulator is leftover, add it to be processed
      if (Index < NumAccums)
        PS.Accum[CurSize++] = PS.Accum[NumAccums - 1];

      NumAccums = CurSize;
    }
    // Replace the last-iteration result with the reduced value.
    ValueMap[Orig] = PS.Accum[0];
  }

  // Replace uses of live-outs with the last unrolling part clone of them.
  for (const auto &It : LiveOutExtUsers)
    It.first->replaceUsesOfWith(It.second, ValueMap[It.second]);
  for (const auto &It : LiveOutInstUsers)
    It.first->replaceUsesOfWith(It.second, ValueMap[It.second]);

  CurrentLatch->setCondBit(ValueMap[CurrentLatch->getCondBit()]);

  // VPlan has been modified, thus we have to reset SVA results, if any.
  Plan.invalidateAnalyses({VPAnalysisID::SVA});

  VPLAN_DUMP(UnrollDumpControl, Plan);
}

VPReductionFinal *
VPlanLoopUnroller::getPartialSumReducFinal(const VPLoop &VPL,
                                           const VPPHINode &Phi) {
  if (!EnablePartialSums)
    return nullptr;
  VPInstruction *LoopVal, *InitVal;
  std::tie(LoopVal, InitVal) = getHeaderPhiOperands(VPL, Phi);
  if (!InitVal || !LoopVal)
    return nullptr;
  // Ensure the initial value is a reduction init, and
  // filter out the in-scan reductions.
  auto RedInit = dyn_cast<VPReductionInit>(InitVal);
  if (!RedInit || RedInit->isScalar())
    return nullptr;
  // Locate the VPReductionFinal and filter out unsupported cases
  // based on the opcode/FMF.
  auto Iter = llvm::find_if(LoopVal->users(), [](const VPUser *U) {
    return isa<VPReductionFinal>(U);
  });
  assert(Iter != LoopVal->user_end() && "Expected non-null ReductionFinal");
  VPReductionFinal *RedFinal = cast<VPReductionFinal>(*Iter);
  switch (RedFinal->getBinOpcode()) {
  // Floating point reduction operators, which require that the fast math
  // flags permit reassociation.
  case Instruction::FAdd:
  case Instruction::FSub:
  case Instruction::FMul:
    if (LoopVal->hasFastMathFlags() &&
        LoopVal->getFastMathFlags().allowReassoc())
      return RedFinal;
    break;
  // Integer reduction operators are only limited to single-instruction cases.
  case Instruction::Add:
  case Instruction::Sub:
  case Instruction::Mul:
  case Instruction::Or:
  case Instruction::Xor:
  case Instruction::And:
    return RedFinal;
  // Min/max reductions are only supported when not part of a
  // min/max + index idiom.
  // TODO: remove this restriction by adding support for
  // parallelizing the min/max + index idiom in the unroller.
  case VPInstruction::SMax:
  case VPInstruction::UMax:
  case VPInstruction::FMax:
  case VPInstruction::SMin:
  case VPInstruction::UMin:
  case VPInstruction::FMin:
    if (all_of(RedFinal->users(),
               [](const VPUser *U) { return !isa<VPInstruction>(U); }) &&
        !RedFinal->isMinMaxIndex())
      return RedFinal;
    break;
  default:
    break;
  }
  return nullptr;
}

PSumCandidate::PSumCandidate(VPReductionInit &RedInit,
                             VPReductionFinal &RedFinal)
    : Opcode(RedFinal.getBinOpcode()), Init(&RedInit),
      ReductionType(RedFinal.getType()) {
  if (RedFinal.hasFastMathFlags())
    FMF = RedFinal.getFastMathFlags();
}
