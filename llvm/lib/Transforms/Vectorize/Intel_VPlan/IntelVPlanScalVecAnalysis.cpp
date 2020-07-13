//===- IntelVPlanScalVecAnalysis.cpp --------------------------------------===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
/// \file
///  This file provides implementation of VPlanScalVecAnalysis including
///  central compute method that can be invoked by external clients.
///
//===----------------------------------------------------------------------===//

#include "IntelVPlanScalVecAnalysis.h"
#include "IntelVPlan.h"
#include "IntelVPlanUtils.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include <numeric>

#define DEBUG_TYPE "vplan-scalvec-analysis"

using namespace llvm;
using namespace llvm::vpo;

static bool isLoopHeaderPHI(VPlan *Plan, const VPPHINode *Phi) {
  auto *PhiBlock = Phi->getParent();
  auto *Lp = Plan->getVPLoopInfo()->getLoopFor(PhiBlock);
  if (!Lp)
    return false;
  return Lp->getHeader() == PhiBlock;
}

static bool checkSVAForInstUseSites(
    const VPInstruction *Inst,
    std::function<bool(const VPInstruction *, unsigned)> Predicate) {
  bool AnyUserPassedPredicate =
      llvm::any_of(Inst->users(), [Inst, Predicate](VPUser *U) {
        auto *UserInst = dyn_cast<VPInstruction>(U);
        if (!UserInst)
          return false;

        return llvm::any_of(llvm::enumerate(UserInst->operands()),
                            [UserInst, Inst, Predicate](auto Operand) {
                              if (UserInst->getOperand(Operand.index()) == Inst)
                                return Predicate(UserInst, Operand.index());
                              return false;
                            });
      });

  return AnyUserPassedPredicate;
}

bool VPlanScalVecAnalysis::instNeedsBroadcast(const VPInstruction *Inst) const {
  if (instNeedsVectorCode(Inst))
    return false;

  auto NeedVectorCode = [this](const VPInstruction *I, unsigned OpIdx) {
    return operandNeedsVectorCode(I, OpIdx);
  };
  return checkSVAForInstUseSites(Inst, NeedVectorCode);
}

bool VPlanScalVecAnalysis::instNeedsExtractFromFirstActiveLane(
    const VPInstruction *Inst) const {
  if (instNeedsFirstScalarCode(Inst))
    return false;

  auto NeedFScalCode = [this](const VPInstruction *I, unsigned OpIdx) {
    return operandNeedsFirstScalarCode(I, OpIdx);
  };
  return checkSVAForInstUseSites(Inst, NeedFScalCode);
}

bool VPlanScalVecAnalysis::instNeedsExtractFromLastActiveLane(
    const VPInstruction *Inst) const {
  if (instNeedsLastScalarCode(Inst))
    return false;

  auto NeedLScalCode = [this](const VPInstruction *I, unsigned OpIdx) {
    return operandNeedsLastScalarCode(I, OpIdx);
  };
  return checkSVAForInstUseSites(Inst, NeedLScalCode);
}

bool VPlanScalVecAnalysis::computeSpecialInstruction(
    const VPInstruction *Inst, unsigned VF, const TargetLibraryInfo *TLI) {
  auto *DA = Plan->getVPlanDA();

  switch (Inst->getOpcode()) {
  case Instruction::PHI: {
    // Loop header PHIs need special back propagation since they introduce
    // backedges.
    if (isLoopHeaderPHI(Plan, cast<VPPHINode>(Inst))) {
      SVABits ExistingSVABits = getAllSetBitsFromUsers(Inst);
      // Set all bits from users to def and operands.
      setSVABitsForInst(Inst, ExistingSVABits);
      setSVABitsForAllOperands(Inst, ExistingSVABits);
      // Special back propagation needed for loop header PHIs.
      backPropagateSVABitsForRecurrentPHI(cast<VPPHINode>(Inst),
                                          ExistingSVABits, VF, TLI);
      return true;
    }

    // Non-loop header PHIs are not processed in a special manner.
    return false;
  }

  case Instruction::Load:
  case Instruction::Store: {
    // Non-simple loads/stores should be vectorized always.
    if (!Inst->isSimpleLoadStore()) {
      setSVAKindForInst(Inst, SVAKind::Vector);
      setSVAKindForAllOperands(Inst, SVAKind::Vector);
      return true;
    }

    // Loads/stores are processed uniquely since the nature of the instruction
    // is not propagated to its operands. Specialization is done for
    // unit-stride and uniform memory accesses.
    VPValue *Ptr = getLoadStorePointerOperand(Inst);
    unsigned PtrOpIdx = Inst->getOperandIndex(Ptr);
    if (DA->isUnitStridePtr(Ptr)) {
      // For a unit-stride access, pointer will be scalar in nature,
      // specifically requiring first lane value.
      setSVAKindForOperand(Inst, PtrOpIdx, SVAKind::FirstScalar);
      // The access itself is vector in nature.
      setSVAKindForInst(Inst, SVAKind::Vector);
      // For stores, the value operand should be vector.
      if (Inst->getOpcode() == Instruction::Store)
        setSVAKindForOperand(Inst, 0 /*Value OpIdx*/, SVAKind::Vector);
      return true;
    }
    if (!DA->isDivergent(*Ptr)) {
      // If the pointer operand is uniform, then the access itself is uniform
      // and hence scalar (for first lane) in nature.
      setSVAKindForOperand(Inst, PtrOpIdx, SVAKind::FirstScalar);
      if (Inst->getOpcode() == Instruction::Load)
        setSVAKindForInst(Inst, SVAKind::FirstScalar);
      // For uniform stores, the value operand maybe divergent. Set it as
      // last/first scalar based on uniformity, and accordingly decide the
      // nature of store itself.
      if (Inst->getOpcode() == Instruction::Store) {
        VPValue *ValueOp = Inst->getOperand(0);
        SVAKind StoreValOpKind = DA->isDivergent(*ValueOp)
                                     ? SVAKind::LastScalar
                                     : SVAKind::FirstScalar;
        setSVAKindForOperand(Inst, 0 /*Value OpIdx*/, StoreValOpKind);
        setSVAKindForInst(Inst, StoreValOpKind);
      }
      return true;
    }

    // All other cases should imply vector nature for the operands and the
    // instruction.
    setSVAKindForAllOperands(Inst, SVAKind::Vector);
    setSVAKindForInst(Inst, SVAKind::Vector);
    return true;
  }

  case Instruction::Call: {
    // Call instructions are processed uniquely since the nature of the
    // instruction and its operands is dependent on additional knowledge
    // availability. Specialization is done for different scenarios based on
    // results of CallVecDecisions analysis. If the call's vectorization
    // scenario is Undefined i.e. not analyzed, then we conservatively skip the
    // instruction from any further analysis and mark both instruction and its
    // operands as Vector.
    auto SetSVAKindForArgOperands = [this](const VPCallInstruction *VPCall,
                                           const SVAKind Kind) {
      for (unsigned ArgIdx = 0; ArgIdx < VPCall->arg_size(); ++ArgIdx)
        setSVAKindForOperand(VPCall, ArgIdx, Kind);
    };

    const VPCallInstruction *VPCall = cast<VPCallInstruction>(Inst);
    assert(VPCall->getVFForScenario() == VF && "No available scenario for VF.");
    switch (VPCall->getVectorizationScenario()) {
    case VPCallInstruction::CallVecScenariosTy::Undefined: {
      // No knowledge available for call, conservatively vectorize.
      setSVAKindForInst(Inst, SVAKind::Vector);
      SetSVAKindForArgOperands(VPCall, SVAKind::Vector);
      break;
    }
    case VPCallInstruction::CallVecScenariosTy::LibraryFunc: {
      // This is a vector library call, so all call arguments sould be vector in
      // nature.
      setSVAKindForInst(Inst, SVAKind::Vector);
      SetSVAKindForArgOperands(VPCall, SVAKind::Vector);
      break;
    }
    case VPCallInstruction::CallVecScenariosTy::Serialization: {
      // Serialization is tracked as emulated vectorization by SVA. Set
      // instruction and operands as vector.
      setSVAKindForInst(Inst, SVAKind::Vector);
      SetSVAKindForArgOperands(VPCall, SVAKind::Vector);
      break;
    }
    case VPCallInstruction::CallVecScenariosTy::DoNotWiden: {
      // Call will be emitted as single scalar copy in generated code.
      setSVAKindForInst(Inst, SVAKind::FirstScalar);
      SetSVAKindForArgOperands(VPCall, SVAKind::FirstScalar);
      break;
    }
    case VPCallInstruction::CallVecScenariosTy::VectorVariant: {
      // Call that will be vectorized using matching SIMD vector variant.
      // Argument nature is decided based on VectorVariant's properties.
      VectorVariant *VecVariant =
          const_cast<VectorVariant *>(VPCall->getVectorVariant());
      std::vector<VectorKind> Parms = VecVariant->getParameters();

      for (unsigned ArgIdx = 0; ArgIdx < VPCall->arg_size(); ++ArgIdx) {
        if (Parms[ArgIdx].isVector())
          setSVAKindForOperand(VPCall, ArgIdx, SVAKind::Vector);
        else
          setSVAKindForOperand(VPCall, ArgIdx, SVAKind::FirstScalar);
      }

      // Instruction itself is vector in nature.
      setSVAKindForInst(Inst, SVAKind::Vector);
      break;
    }
    case VPCallInstruction::CallVecScenariosTy::TrivialVectorIntrinsic: {
      // Call that is vectorized trivially using vector intrinsics. Argument
      // nature is decided based on intrinsic.
      Intrinsic::ID ID = VPCall->getVectorIntrinsic();
      for (unsigned ArgIdx = 0; ArgIdx < VPCall->arg_size(); ++ArgIdx) {
        if (hasVectorInstrinsicScalarOpd(ID, ArgIdx))
          setSVAKindForOperand(VPCall, ArgIdx, SVAKind::FirstScalar);
        else
          setSVAKindForOperand(VPCall, ArgIdx, SVAKind::Vector);
      }

      // Instruction itself is vector in nature.
      setSVAKindForInst(Inst, SVAKind::Vector);
      break;
    }
    default:
      llvm_unreachable("Unknown vectorization scenario.");
    }

    // Last operand of call is called value, which should be left as scalar.
    // TODO: Indirect calls.
    setSVAKindForOperand(Inst, VPCall->getNumOperands() - 1,
                         SVAKind::FirstScalar);
    return true;
  }

  // Specialization for loop entities related instructions.
  case VPInstruction::InductionInit: {
    // All operands of induction-init should be scalar strictly.
    setSVAKindForAllOperands(Inst, SVAKind::FirstScalar);
    // We don't set any specific bits for the instruction itself, it will be
    // decided only based on uses of the instruction. If there are no users, set
    // Vector conservatively.
    SVABits SetBits = getAllSetBitsFromUsers(Inst);
    setSVABitsForInst(Inst, SetBits);
    return true;
  }

  case VPInstruction::InductionInitStep: {
    // All operands of induction-init-step should be scalar strictly.
    setSVAKindForAllOperands(Inst, SVAKind::FirstScalar);
    // The instruction itself is always uniform. So we set it to be scalar only.
    setSVAKindForInst(Inst, SVAKind::FirstScalar);
    return true;
  }

  case VPInstruction::InductionFinal: {
    // The instruction itself emulates finalization with scalar instructions (or
    // implicit extract). It produces a scalar return value.
    setSVAKindForInst(Inst, SVAKind::FirstScalar);
    if (Inst->getNumOperands() == 2)
      // Close-form calculation, emulated with scalar operands.
      setSVAKindForAllOperands(Inst, SVAKind::FirstScalar);
    else
      // Non close-form calculation, need last lane value.
      setSVAKindForAllOperands(Inst, SVAKind::LastScalar);
    return true;
  }

  case VPInstruction::ReductionInit: {
    // All operands of reduction-init should be scalar strictly.
    setSVAKindForAllOperands(Inst, SVAKind::FirstScalar);
    // FIXME: Enable assert below when legality is fixed to bail out of
    // vectorization for loops where reduction PHI is stored to non-private
    // address. Check vplan_scalvec_analysis_header_phi_specialization.ll.
    // assert(!instNeedsFirstScalarCode(Inst) && !instNeedsLastScalarCode(Inst)
    // && "Reduction can never be done with scalar instructions.");
    setSVAKindForInst(Inst, SVAKind::Vector);
    return true;
  }

  case VPInstruction::ReductionFinal: {
    auto *RedFinal = cast<VPReductionFinal>(Inst);
    // Special processing for each operand of reduction-final.
    // 1. Reducing operand should be vector.
    unsigned ReducingOpIdx =
        RedFinal->getOperandIndex(RedFinal->getReducingOperand());
    setSVAKindForOperand(RedFinal, ReducingOpIdx, SVAKind::Vector);
    // 2. Start value, if any, should be scalar.
    if (auto *StartValOp = RedFinal->getStartValueOperand())
      setSVAKindForOperand(RedFinal, RedFinal->getOperandIndex(StartValOp),
                           SVAKind::FirstScalar);
    // 3. Parent exit value for index reduction should be vector.
    if (auto *ParentExitOp = RedFinal->getParentExitValOperand())
      setSVAKindForOperand(RedFinal, RedFinal->getOperandIndex(ParentExitOp),
                           SVAKind::Vector);
    // 4. Parent final value for index reduction should be scalar.
    if (auto *ParentFinalOp = RedFinal->getParentFinalValOperand())
      setSVAKindForOperand(RedFinal, RedFinal->getOperandIndex(ParentFinalOp),
                           SVAKind::FirstScalar);

    // The instruction itself is vectorized, although it produces a scalar
    // return value.
    setSVAKindForInst(RedFinal, SVAKind::Vector);
    setSVAKindForReturnValue(RedFinal, SVAKind::FirstScalar);
    return true;
  }

  case VPInstruction::AllocatePrivate: {
    // We don't set any specific bits for the allocate-private instruction, it
    // will decided only based on uses of the instruction. If there are no
    // users, set Vector conservatively.
    SVABits SetBits = getAllSetBitsFromUsers(Inst);
    setSVABitsForInst(Inst, SetBits);
    return true;
  }

  case VPInstruction::Pred: {
    // Block predicate is just used to track current BasicBlock's mask value, it
    // is not lowered into any instruction in CG. Set instruction as vector and
    // mask operand itself is vector.
    setSVAKindForInst(Inst, SVAKind::Vector);
    setSVAKindForAllOperands(Inst, SVAKind::Vector);
    return true;
  }

  case VPInstruction::AllZeroCheck: {
    // Instruction itself is lowered as vector bitcast operation, and currently
    // its only operand is expected to be vector in nature.
    setSVAKindForInst(Inst, SVAKind::Vector);
    setSVAKindForAllOperands(Inst, SVAKind::Vector);
    // Value produced by the instruction is scalar in nature.
    setSVAKindForReturnValue(Inst, SVAKind::FirstScalar);
    return true;
  }

  case VPInstruction::VectorTripCountCalculation: {
    // Instruction itself is unconditionally always scalar.
    setSVAKindForInst(Inst, SVAKind::FirstScalar);
    // All operands are always scalar too.
    setSVAKindForAllOperands(Inst, SVAKind::FirstScalar);
    return true;
  }

  case VPInstruction::OrigTripCountCalculation: {
    // Instruction itself is unconditionally always scalar.
    setSVAKindForInst(Inst, SVAKind::FirstScalar);
    // All operands are always scalar too.
    setSVAKindForAllOperands(Inst, SVAKind::FirstScalar);
    return true;
  }

  case VPInstruction::Not:
  case VPInstruction::SMax:
  case VPInstruction::UMax:
  case VPInstruction::FMax:
  case VPInstruction::SMin:
  case VPInstruction::UMin:
  case VPInstruction::FMin:
  case VPInstruction::Subscript:
  case VPInstruction::Blend:
  case VPInstruction::HIRCopy:
  case VPInstruction::Abs: {
    // VPlan-specific instructions that don't need special processing in SVA.
    return false;
  }

  default: {
    assert(Inst->getOpcode() <= Instruction::OtherOpsEnd &&
           "Unknown opcode seen in SVA.");
    assert(!isSVASpecialProcessedInst(Inst) &&
           "Specially processed instruction was not handled.");
    return false;
  }
  }
}

void VPlanScalVecAnalysis::compute(VPlan *P, unsigned VF,
                                   const TargetLibraryInfo *TLI) {
  Plan = P;

  // TODO: Is it okay to clear the table before a fresh compute cycle?
  clear();
  VPBasicBlock *CFGEntry = Plan->getEntryBlock();
  auto *DA = Plan->getVPlanDA();
  for (auto *VPBB : post_order(CFGEntry)) {
    LLVM_DEBUG(dbgs() << "[SVA] Visiting BB " << VPBB->getName() << "\n");
    for (VPInstruction &Inst : reverse(*VPBB)) {
      auto *VPInst = &Inst;

      // Allocate expected number of elements for OperandBits. Default state is
      // all-zero for the bits so it would be invalid (assertion) to access them
      // anyways.
      VPlanSVAResults[VPInst].OperandBits.resize(VPInst->getNumOperands());

      // Case 1: Specially processed instruction in SVA.
      if (computeSpecialInstruction(VPInst, VF, TLI)) {
        LLVM_DEBUG(dbgs() << "[SVA] Specially processed instruction ";
                   VPInst->dump());
        continue;
      }

      assert(!isSVASpecialProcessedInst(VPInst) &&
             "Specially processed instruction cannot reach here.");

      // Case 2: Decide instruction nature by combining SVABits from all its use
      // sites.
      SVABits CombinedUseBits = getAllSetBitsFromUsers(VPInst);
      if (CombinedUseBits.any()) {
        if (!DA->isDivergent(*VPInst)) {
          // If the instruction has several uses but DA informs us that it is
          // uniform, then propagate scalar nature for operands and for the
          // instruction. Consider the example below -
          //
          // %a = add %uniform.op1, %uniform.op2
          // %b = mul %a, %divergent.op2
          //
          // The mul instruction will mark %a to be vector in nature in its
          // operand bits, but %uniform.op1 and %uniform.op2 need not be vector
          // in nature for this instruction pattern. They can be scalar nature
          // (for first lane), and %a can be updated to be scalar in nature only
          // (drop other existing bits). Its need of broadcast (for vector
          // context) can be obtained by inspecting the operand level bits at
          // use site in %b.

          SVAKind NewSVAKind = SVAKind::FirstScalar;
          if (VPInst->mayHaveSideEffects()) {
            assert(false &&
                   "Instruction with side effects is not safe to scalarize.");
            // For prod/release build vectorize the instruction conservatively.
            NewSVAKind = SVAKind::Vector;
          }

          setSVAKindForInst(VPInst, NewSVAKind);
          setSVAKindForAllOperands(VPInst, NewSVAKind);
        } else {
          // Propagate all set bits from users for instruction and its operands.
          setSVABitsForInst(VPInst, CombinedUseBits);
          setSVABitsForAllOperands(VPInst, CombinedUseBits);
        }
        continue;
      }

      // Case 3: This is a new VPInst not found in table yet. Determine its
      // ScalVec nature using DA.
      SVAKind NewSVAKind =
          DA->isDivergent(*VPInst) ? SVAKind::Vector : SVAKind::FirstScalar;

      if (VPInst->mayHaveSideEffects() && NewSVAKind == SVAKind::FirstScalar) {
        assert(false &&
               "Instruction with side effects is not safe to scalarize.");
        // For prod/release build vectorize the instruction conservatively.
        NewSVAKind = SVAKind::Vector;
      }

      setSVAKindForInst(VPInst, NewSVAKind);
      setSVAKindForAllOperands(VPInst, NewSVAKind);
    }
  }
}

VPlanScalVecAnalysis::SVABits
VPlanScalVecAnalysis::getAllSetBitsFromUsers(const VPInstruction *Inst,
                                             SVABits DefaultBits) {
  if (Inst->getNumUsers() == 0)
    return DefaultBits;

  SVABits CombinedUseBits;
  for (auto *User : Inst->users()) {
    if (auto *UserInst = dyn_cast<VPInstruction>(User)) {
      auto UserInstResults = VPlanSVAResults.find(UserInst);
      if (UserInstResults == VPlanSVAResults.end()) {
        // TODO: Only recurrent PHIs are allowed to be non-processed users. Fix
        // this assertion after def-use chain issue is resolved with VPlan
        // unroller.
        if (isa<VPPHINode>(UserInst)) {
          assert(isLoopHeaderPHI(Plan, cast<VPPHINode>(UserInst)) &&
                 "Recurrent header PHI can be the only non-processed user for "
                 "instructions inside the loop.");
        }
        continue;
      }

      // Record is known to be found in the table, retrieve it here for
      // accessing operand-level bits.
      OperandBitsTy &UserInstOperandBits = UserInstResults->second.OperandBits;

      for (unsigned OpIdx = 0; OpIdx < UserInst->getNumOperands(); ++OpIdx) {
        if (UserInst->getOperand(OpIdx) == Inst) {
          // User should already be processed, so try getting the set bits for
          // the use (asserts if not processed).
          assert(UserInstOperandBits[OpIdx].any() &&
                 "Use of an instruction was not processed.");
          CombinedUseBits |= UserInstOperandBits[OpIdx];
        }
      }
    }
  }

  return CombinedUseBits;
}

void VPlanScalVecAnalysis::setSVAKindForOperand(const VPInstruction *Inst,
                                                unsigned OpIdx,
                                                const SVAKind Kind) {
  assert(OpIdx < Inst->getNumOperands() &&
         "Invalid operand index for instruction.");
  VPlanSVAResults[Inst].OperandBits[OpIdx].set(static_cast<unsigned>(Kind));
}

void VPlanScalVecAnalysis::setSVABitsForOperand(const VPInstruction *Inst,
                                                unsigned OpIdx,
                                                SVABits &SetBits) {
  assert(OpIdx < Inst->getNumOperands() &&
         "Invalid operand index for instruction.");
  VPlanSVAResults[Inst].OperandBits[OpIdx] = SetBits;
}

void VPlanScalVecAnalysis::setSVAKindForAllOperands(const VPInstruction *Inst,
                                                    const SVAKind Kind) {
  for (unsigned I = 0, E = Inst->getNumOperands(); I < E; ++I)
    setSVAKindForOperand(Inst, I, Kind);
}

void VPlanScalVecAnalysis::setSVABitsForAllOperands(const VPInstruction *Inst,
                                                    SVABits &SetBits) {
  for (unsigned I = 0, E = Inst->getNumOperands(); I < E; ++I)
    setSVABitsForOperand(Inst, I, SetBits);
}

void VPlanScalVecAnalysis::orSVABitsForOperand(const VPInstruction *Inst,
                                               unsigned OpIdx,
                                               SVABits &OrBits) {
  assert(OpIdx < Inst->getNumOperands() &&
         "Invalid operand index for instruction.");
  VPlanSVAResults[Inst].OperandBits[OpIdx] |= OrBits;
}

void VPlanScalVecAnalysis::orSVABitsForAllOperands(const VPInstruction *Inst,
                                                   SVABits &OrBits) {
  for (unsigned I = 0, E = Inst->getNumOperands(); I < E; ++I)
    orSVABitsForOperand(Inst, I, OrBits);
}

void VPlanScalVecAnalysis::backPropagateSVABitsForRecurrentPHI(
    const VPPHINode *Phi, SVABits &SetBits, unsigned VF,
    const TargetLibraryInfo *TLI) {
  SetVector<const VPInstruction *> Worklist;
  auto AddInstOperandsToWorklist =
      [&Worklist, this, &SetBits](const VPInstruction *VPI) -> void {
    for (auto *Op : VPI->operands()) {
      // Cases where only one-level propagation of the SVA kind for operands is
      // needed.
      if (!isa<VPInstruction>(Op)) {
        // Non-VPInstruction
        orSVABitsForOperand(VPI, VPI->getOperandIndex(Op), SetBits);
        continue;
      }
      auto *OpInst = cast<VPInstruction>(Op);
      Optional<SVABits> CurrentOpSVABits = findSVABitsForInst(OpInst);

      if (CurrentOpSVABits && CurrentOpSVABits.getValue() == SetBits) {
        // Nothing to do, operand already has same state as Inst.
        continue;
      }

      if (CurrentOpSVABits == None) {
        // Nothing to do, operand has not been visited yet. No need to process
        // in worklist.
        continue;
      }

      // All checks failed, add to worklist for reprocessing.
      Worklist.insert(OpInst);
    }
  };

  AddInstOperandsToWorklist(Phi);

  // Instructions in the worklist need a chain of propagation, This is triggered
  // because of a PHI node that introduces back-edge or recurrence property
  // (like reduction/induction).
  while (!Worklist.empty()) {
    const VPInstruction *Inst = Worklist.pop_back_val();

    if (isSVASpecialProcessedInst(Inst)) {
      // Special processed instructions should be handled differently, their
      // nature may depend on operand-site bits which were updated during this
      // chain of back propagation.
      // TODO: Operands of special instruction may need to be added back to
      // worklist here. Update based on use-case in future.
      computeSpecialInstruction(Inst, VF, TLI);
      continue;
    }

    assert(!isSVASpecialProcessedInst(Inst) &&
           "Specially processed SVA instruction cannot reach here.");
    SVABits InstBits = getSVABitsForInst(Inst);
    // Get the set of bits that are not set in instruction's SVABits. InstBits
    // should be a subset of SetBits, so we use XOR here to find difference.
    SVABits CommonBits = InstBits & SetBits;
    (void)CommonBits;
    assert(CommonBits == InstBits && "InstBits is not a subset of SetBits.");
    SVABits DiffBits = InstBits ^ SetBits;
    if (DiffBits.any()) {
      orSVABitsForInst(Inst, DiffBits);
      orSVABitsForAllOperands(Inst, DiffBits);
    }
    AddInstOperandsToWorklist(Inst);
  }
}

bool VPlanScalVecAnalysis::isSVASpecialProcessedInst(
    const VPInstruction *Inst) {
  switch (Inst->getOpcode()) {
  case Instruction::PHI:
    return isLoopHeaderPHI(Plan, cast<VPPHINode>(Inst));
  case Instruction::Load:
  case Instruction::Store:
  case Instruction::Call:
  case VPInstruction::InductionInit:
  case VPInstruction::InductionInitStep:
  case VPInstruction::InductionFinal:
  case VPInstruction::ReductionInit:
  case VPInstruction::ReductionFinal:
  case VPInstruction::Pred:
  case VPInstruction::AllZeroCheck:
  case VPInstruction::VectorTripCountCalculation:
  case VPInstruction::OrigTripCountCalculation:
    return true;
  default:
    return false;
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPlanScalVecAnalysis::print(raw_ostream &OS, const VPInstruction *VPI) {
  OS << "[";
  printSVAKindForInst(OS, VPI);
  OS << "] ";
  VPI->dump(OS);
}

void VPlanScalVecAnalysis::print(raw_ostream &OS, const VPBasicBlock *VPBB) {
  for (auto &VPI : *VPBB)
    print(OS, &VPI);
}

void VPlanScalVecAnalysis::print(raw_ostream &OS) {
  OS << "\nPrinting ScalVec analysis results for " << Plan->getName() << "\n";
  ReversePostOrderTraversal<VPBasicBlock *> RPOT(Plan->getEntryBlock());
  for (VPBasicBlock *VPBB : make_range(RPOT.begin(), RPOT.end())) {
    OS << "Basic Block: " << VPBB->getName() << "\n";
    print(OS, VPBB);
  }
}

void VPlanScalVecAnalysis::printSVAKindForInst(raw_ostream &OS,
                                               const VPInstruction *VPI) const {
  SVABits InstBits = getSVABitsForInst(VPI);
  SVABits RetValBits = getSVABitsForReturnValue(VPI);
  if (InstBits != RetValBits) {
    OS << "RetVal:(";
    if (RetValBits.test(static_cast<unsigned>(SVAKind::FirstScalar)))
      OS << "F";
    else
      OS << " ";
    if (RetValBits.test(static_cast<unsigned>(SVAKind::Vector)))
      OS << "V";
    else
      OS << " ";
    if (RetValBits.test(static_cast<unsigned>(SVAKind::LastScalar)))
      OS << "L";
    else
      OS << " ";
    OS << "), Inst:";
  }
  OS << "(";
  if (InstBits.test(static_cast<unsigned>(SVAKind::FirstScalar)))
    OS << "F";
  else
    OS << " ";
  if (InstBits.test(static_cast<unsigned>(SVAKind::Vector)))
    OS << "V";
  else
    OS << " ";
  if (InstBits.test(static_cast<unsigned>(SVAKind::LastScalar)))
    OS << "L";
  else
    OS << " ";
  OS << ")";
}

void VPlanScalVecAnalysis::printSVAKindForOperand(raw_ostream &OS,
                                                  const VPInstruction *VPI,
                                                  unsigned OpIdx) const {
  SVABits OpBits = getSVABitsForOperand(VPI, OpIdx);
  if (OpBits.test(static_cast<unsigned>(SVAKind::FirstScalar)))
    OS << "F";
  if (OpBits.test(static_cast<unsigned>(SVAKind::LastScalar)))
    OS << "L";
  if (OpBits.test(static_cast<unsigned>(SVAKind::Vector)))
    OS << "V";
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP
