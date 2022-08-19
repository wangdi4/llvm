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

static bool isLoopHeaderPHI(VPlanVector *Plan, const VPPHINode *Phi) {
  auto *PhiBlock = Phi->getParent();
  auto *Lp = Plan->getVPLoopInfo()->getLoopFor(PhiBlock);
  if (!Lp)
    return false;
  return Lp->getHeader() == PhiBlock;
}

// Helper to check if GEP is non-unit-strided with unit-strided pointer operand
// in SOA layout.
static bool isNonUnitStrSOAGEPWithUnitStrPtr(VPlanVector *Plan,
                                             const VPGEPInstruction *GEP) {
  auto *DA = Plan->getVPlanDA();
  return !DA->isSOAUnitStride(GEP) &&
         DA->isSOAUnitStride(GEP->getPointerOperand());
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
    const VPInstruction *Inst) {
  auto SetSVAKindForCallArgOperands = [this](const VPCallInstruction *VPCall,
                                             const SVAKind Kind) {
    for (unsigned ArgIdx = 0; ArgIdx < VPCall->getNumArgOperands(); ++ArgIdx)
      setSVAKindForOperand(VPCall, ArgIdx, Kind);
  };

  auto *DA = Plan->getVPlanDA();

  switch (Inst->getOpcode()) {
  case Instruction::PHI: {
    auto *Phi = cast<VPPHINode>(Inst);
    // Loop header PHIs need special back propagation since they introduce
    // backedges.
    if (isLoopHeaderPHI(Plan, Phi)) {
      SVABits ExistingSVABits = getAllSetBitsFromUsers(Phi);
      // Do not attempt to back propagate if header PHI has no processed users
      // yet. This implies that PHI is unused or used by another loop header
      // PHI. For example -
      //
      // header:
      //   %phi1 = phi i32 [0, %preheader], [%phi2, %exit]
      //   %phi2 = phi i32 [0, %preheader], [%val, %exit]  ---> Skipped
      //   ; No other use of %phi2
      //
      // In above example, we skip processing %phi2 here. Appropriate SVA bits
      // will be set during back propagation of %phi1.
      if (!ExistingSVABits.any()) {
        SkippedPHIs.insert(Phi);
      } else {
        // Set all bits from users to def and operands.
        setSVABitsForInst(Phi, ExistingSVABits);
        setSVABitsForAllOperands(Phi, ExistingSVABits);
        // Special back propagation needed for loop header PHIs.
        backPropagateSVABitsForRecurrentPHI(Phi, ExistingSVABits);
        // Remove skipped phi from the list after it's processed (triggered via
        // back propagation).
        if (SkippedPHIs.count(Phi))
          SkippedPHIs.erase(Phi);
      }
      return true;
    }

    // Non-loop header PHIs are not processed in a special manner.
    return false;
  }

  case Instruction::GetElementPtr: {
    auto *GEP = cast<VPGEPInstruction>(Inst);

    if (isNonUnitStrSOAGEPWithUnitStrPtr(Plan, GEP)) {
      // In case of SOA-unit stride pointer for non-unit strided GEP, we
      // retain the scalar-type pointer, typically <VF x Ty>*. Otherwise, we
      // get the vector version of the pointer, which is typically a vector of
      // pointers, i.e., <VF x Ty*>.
      setSVAKindForInst(GEP, SVAKind::Vector);
      setSVAKindForOperand(GEP, 0 /*Pointer operand*/, SVAKind::FirstScalar);
      for (auto *IdxOp : GEP->indices())
        setSVAKindForOperand(GEP, GEP->getOperandIndex(IdxOp), SVAKind::Vector);
      return true;
    }

    // GEP was not processed in a special manner.
    return false;
  }

  case Instruction::Load:
  case Instruction::Store: {
    // Loads/stores are processed uniquely since the nature of the instruction
    // is not propagated to its operands. Specialization is done for possible
    // unit-stride (including SOA) and uniform memory accesses.
    auto *LoadStore = cast<VPLoadStoreInst>(Inst);

    VPValue *Ptr = LoadStore->getPointerOperand();
    unsigned PtrOpIdx = LoadStore->getPointerOperandIndex();
    bool IsLoadOrUnmaskedStore = Inst->getOpcode() == Instruction::Load ||
                                 (Inst->getOpcode() == Instruction::Store &&
                                  Inst->getParent()->getPredicate() == nullptr);

    if (Inst->isSimpleLoadStore() && !DA->isDivergent(*Ptr) &&
        IsLoadOrUnmaskedStore) {
      // If the load/unmasked store is simple (non-atomic and non-volatile) and
      // the pointer operand is uniform, then the access itself is uniform and
      // hence scalar (for first/last lane) in nature.
      setSVAKindForOperand(Inst, PtrOpIdx, SVAKind::FirstScalar);
      if (Inst->getOpcode() == Instruction::Load)
        setSVAKindForInst(Inst, SVAKind::FirstScalar);
      // For uniform unmasked stores, the value operand maybe divergent. Set it
      // as last/first scalar based on uniformity, and accordingly decide the
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

    if (isVectorizableLoadStore(Inst) &&
        DA->isUnitStridePtr(Ptr, LoadStore->getValueType())) {
      // For a vectorizable unit-stride access, pointer will be scalar in
      // nature, specifically requiring first lane value.
      setSVAKindForOperand(Inst, PtrOpIdx, SVAKind::FirstScalar);
      // The access itself is vector in nature.
      setSVAKindForInst(Inst, SVAKind::Vector);
      // For stores, the value operand should be vector.
      if (Inst->getOpcode() == Instruction::Store)
        setSVAKindForOperand(Inst, 0 /*Value OpIdx*/, SVAKind::Vector);
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

    const VPCallInstruction *VPCall = cast<VPCallInstruction>(Inst);
    assert(VPCall->getVFForScenario() > 0 &&
           "CallVecScenario is not recorded for a valid VF.");
    switch (VPCall->getVectorizationScenario()) {
    case VPCallInstruction::CallVecScenariosTy::Undefined: {
      // No knowledge available for call, conservatively vectorize.
      setSVAKindForInst(Inst, SVAKind::Vector);
      SetSVAKindForCallArgOperands(VPCall, SVAKind::Vector);
      break;
    }
    case VPCallInstruction::CallVecScenariosTy::LibraryFunc: {
      // This is a vector library call, so all call arguments sould be vector in
      // nature.
      setSVAKindForInst(Inst, SVAKind::Vector);
      SetSVAKindForCallArgOperands(VPCall, SVAKind::Vector);
      break;
    }
    case VPCallInstruction::CallVecScenariosTy::Serialization: {
      // Serialization is tracked as emulated vectorization by SVA. Set
      // instruction and operands as vector.
      setSVAKindForInst(Inst, SVAKind::Vector);
      SetSVAKindForCallArgOperands(VPCall, SVAKind::Vector);
      break;
    }
    case VPCallInstruction::CallVecScenariosTy::DoNotWiden: {
      // Call will be emitted as single scalar copy in generated code.
      setSVAKindForInst(Inst, SVAKind::FirstScalar);
      SetSVAKindForCallArgOperands(VPCall, SVAKind::FirstScalar);
      break;
    }
    case VPCallInstruction::CallVecScenariosTy::UnmaskedWiden: {
      // Ther are no way to specify uniformity/linearity/etc. in the current
      // DPC++ spec.
      setSVAKindForInst(Inst, SVAKind::Vector);
      SetSVAKindForCallArgOperands(VPCall, SVAKind::Vector);
      break;
    }
    case VPCallInstruction::CallVecScenariosTy::VectorVariant: {
      // Call that will be vectorized using matching SIMD vector variant.
      // Argument nature is decided based on VectorVariant's properties.
      VectorVariant *VecVariant =
          const_cast<VectorVariant *>(VPCall->getVectorVariant());
      std::vector<VectorKind> Parms = VecVariant->getParameters();
      bool IsIntelIndirectCall = VPCall->isIntelIndirectCall();
      if (IsIntelIndirectCall) {
        // For __intel_indirect_calls, first argument represents pointer to call
        // which is not part of corresponding VectorVariant's parameters list.
        // Determine it's nature using DA.
        VPValue *FirstArg = VPCall->getArgOperand(0);
        SVAKind FirstArgKind = Plan->getVPlanDA()->isDivergent(*FirstArg)
                                   ? SVAKind::Vector
                                   : SVAKind::FirstScalar;
        setSVAKindForOperand(VPCall, 0 /*FirstArg*/, FirstArgKind);
      }

      for (unsigned ArgIdx = IsIntelIndirectCall ? 1 : 0, ParmIdx = 0;
           ArgIdx < VPCall->getNumArgOperands(); ++ArgIdx, ++ParmIdx) {
        assert(ParmIdx < Parms.size() && "Trying to access invalid parameter.");
        if (Parms[ParmIdx].isVector())
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
      for (unsigned ArgIdx = 0; ArgIdx < VPCall->getNumArgOperands();
           ++ArgIdx) {
        if (isVectorIntrinsicWithScalarOpAtArg(ID, ArgIdx))
          setSVAKindForOperand(VPCall, ArgIdx, SVAKind::FirstScalar);
        else
          setSVAKindForOperand(VPCall, ArgIdx, SVAKind::Vector);
      }

      // Instruction itself is vector in nature.
      setSVAKindForInst(Inst, SVAKind::Vector);
      break;
    }
    }

    // Last operand of call is called value, which should be left as scalar.
    // TODO: Indirect calls.
    setSVAKindForOperand(Inst, VPCall->getNumOperands() - 1,
                         SVAKind::FirstScalar);
    return true;
  }

  case VPInstruction::TransformLibraryCall: {
    const auto *VPCall = cast<VPTransformLibraryCall>(Inst);

    // Instruction itself is vector in nature.
    setSVAKindForInst(Inst, SVAKind::Vector);

    // This is a vector library call, so all call arguments should be vector in
    // nature.
    SetSVAKindForCallArgOperands(VPCall, SVAKind::Vector);

    // Last operand of call is called value, which should be left as scalar.
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
    const auto *Init = cast<VPReductionInit>(Inst);

    // All operands of reduction-init should be scalar strictly.
    setSVAKindForAllOperands(Inst, SVAKind::FirstScalar);

    // FIXME: Enable assert below when legality is fixed to bail out of
    // vectorization for loops where reduction PHI is stored to non-private
    // address. Check vplan_scalvec_analysis_header_phi_specialization.ll.
    // assert(!instNeedsFirstScalarCode(Inst) && !instNeedsLastScalarCode(Inst)
    // && "Reduction can never be done with scalar instructions.");

    if (Init->isScalar())
      setSVAKindForInst(Inst, SVAKind::FirstScalar);
    else
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

  case VPInstruction::ReductionFinalUdr: {
    // We make VF number of calls to Combiner to finalize UDRs, hence
    // instruction is vector in nature.
    setSVAKindForInst(Inst, SVAKind::Vector);
    setSVAKindForOperand(Inst, 0, SVAKind::Vector);
    setSVAKindForOperand(Inst, 1, SVAKind::FirstScalar);
    return true;
  }

  case VPInstruction::ReductionFinalInscan: {
    setSVAKindForAllOperands(Inst, SVAKind::Vector);
    // The instruction itself is vectorized, although it produces a scalar
    // return value.
    setSVAKindForInst(Inst, SVAKind::Vector);
    setSVAKindForReturnValue(Inst, SVAKind::FirstScalar);
    return true;
  }

  case VPInstruction::PrivateFinalMasked:
  case VPInstruction::PrivateFinalMaskedMem:
  case VPInstruction::PrivateFinalUncondMem:
  case VPInstruction::PrivateFinalUncond: {
    // The instruction is extract. It produces a scalar return value.
    setSVAKindForInst(Inst, SVAKind::Vector);
    setSVAKindForAllOperands(Inst, SVAKind::Vector);
    setSVAKindForReturnValue(Inst, SVAKind::FirstScalar);
    return true;
  }

  case VPInstruction::PrivateFinalCondMem:
  case VPInstruction::PrivateFinalCond: {
    // The instruction is extract. It produces a scalar return value.
    setSVAKindForInst(Inst, SVAKind::Vector);
    setSVAKindForOperand(Inst, 0, SVAKind::Vector);
    setSVAKindForOperand(Inst, 1, SVAKind::Vector);
    setSVAKindForOperand(Inst, 2, SVAKind::FirstScalar);
    setSVAKindForReturnValue(Inst, SVAKind::FirstScalar);
    return true;
  }

  case VPInstruction::PrivateFinalArray: {
    setSVAKindForInst(Inst, SVAKind::Vector);
    setSVAKindForOperand(Inst, 0, SVAKind::Vector);
    setSVAKindForOperand(Inst, 1, SVAKind::FirstScalar);
    return true;
  }

  case VPInstruction::PrivateFinalArrayMasked: {
    setSVAKindForInst(Inst, SVAKind::Vector);
    setSVAKindForOperand(Inst, 0, SVAKind::Vector);
    setSVAKindForOperand(Inst, 1, SVAKind::FirstScalar);
    setSVAKindForOperand(Inst, 2, SVAKind::Vector);
    return true;
  }

  case VPInstruction::PrivateLastValueNonPOD: {
    setSVAKindForInst(Inst, SVAKind::FirstScalar);
    setSVAKindForOperand(Inst, 0, SVAKind::LastScalar);
    setSVAKindForOperand(Inst, 1, SVAKind::FirstScalar);
    return true;
  }

  case VPInstruction::PrivateLastValueNonPODMasked: {
    setSVAKindForInst(Inst, SVAKind::Vector);
    setSVAKindForOperand(Inst, 0, SVAKind::Vector);
    setSVAKindForOperand(Inst, 1, SVAKind::FirstScalar);
    setSVAKindForOperand(Inst, 2, SVAKind::Vector);
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

  case VPInstruction::ActiveLane: {
    setSVAKindForInst(Inst, SVAKind::FirstScalar);
    setSVAKindForAllOperands(Inst, SVAKind::Vector);
    return true;
  }

  case VPInstruction::ActiveLaneExtract: {
    setSVAKindForInst(Inst, SVAKind::FirstScalar);
    setSVAKindForOperand(Inst, 0, SVAKind::Vector);
    setSVAKindForOperand(Inst, 1, SVAKind::FirstScalar);
    return true;
  }

  case VPInstruction::ScalarRemainder:
  case VPInstruction::ScalarPeel:
  case VPInstruction::ScalarPeelHIR:
  case VPInstruction::ScalarRemainderHIR: {
    // Instruction itself is unconditionally always scalar.
    setSVAKindForInst(Inst, SVAKind::FirstScalar);
    // All operands are always scalar too.
    setSVAKindForAllOperands(Inst, SVAKind::FirstScalar);
    return true;
  }

  case VPInstruction::PeelOrigLiveOut:
  case VPInstruction::RemOrigLiveOut:
  case VPInstruction::PeelOrigLiveOutHIR:
  case VPInstruction::RemOrigLiveOutHIR: {
    // Instruction itself is unconditionally always scalar.
    setSVAKindForInst(Inst, SVAKind::FirstScalar);
    // All operands are always scalar too.
    setSVAKindForAllOperands(Inst, SVAKind::FirstScalar);
    return true;
  }

  case VPInstruction::PushVF:
  case VPInstruction::PopVF:
    setSVAKindForInst(Inst, SVAKind::FirstScalar);
    return true;

  case VPInstruction::InvSCEVWrapper:
    // Instruction itself is unconditionally always scalar.
    setSVAKindForInst(Inst, SVAKind::FirstScalar);
    // All operands are always scalar too.
    setSVAKindForAllOperands(Inst, SVAKind::FirstScalar);
    return true;

  case VPInstruction::Not:
  case VPInstruction::SMax:
  case VPInstruction::UMax:
  case VPInstruction::FMax:
  case VPInstruction::SMin:
  case VPInstruction::UMin:
  case VPInstruction::UMinSeq:
  case VPInstruction::FMin:
  case VPInstruction::Subscript:
  case VPInstruction::Blend:
  case VPInstruction::HIRCopy:
  case VPInstruction::ConstStepVector:
  case VPInstruction::Abs: {
    // VPlan-specific instructions that don't need special processing in SVA.
    return false;
  }

  case VPInstruction::VLSLoad: {
    // This is a low-level instruction and we treat the resulting vector as
    // uniformly shared across all lanes, hence "FirstScalar".
    setSVAKindForReturnValue(Inst, SVAKind::FirstScalar);
    setSVAKindForInst(Inst, SVAKind::Vector);
    // This instruction currently has the semantics of a single wide continuous
    // load.
    setSVAKindForAllOperands(Inst, SVAKind::FirstScalar);
    return true;
  }

  case VPInstruction::VLSStore: {
    // Although resulting type is void (and it doesn't matter if it's "scalar"
    // or "vector"), be consistent with VLSLoad's properties.
    setSVAKindForReturnValue(Inst, SVAKind::FirstScalar);
    setSVAKindForInst(Inst, SVAKind::Vector);
    // This instruction currently has the semantics of a single wide continuous
    // store. If that is changed, we'd need to update the kind of the pointer
    // operand.
    setSVAKindForAllOperands(Inst, SVAKind::FirstScalar);
    return true;
  }

  case VPInstruction::VLSExtract: {
    // Each lanes has its own value.
    setSVAKindForInst(Inst, SVAKind::Vector);
    // Wide value is shared across all lanes.
    setSVAKindForAllOperands(Inst, SVAKind::FirstScalar);
    return true;
  }

  case VPInstruction::VLSInsert: {
    // Wide value is shared across all lanes.
    setSVAKindForReturnValue(Inst, SVAKind::FirstScalar);
    setSVAKindForInst(Inst, SVAKind::Vector);
    setSVAKindForOperand(Inst, 0, SVAKind::FirstScalar);
    // Each lane has its own value that we insert into a wide vector.
    setSVAKindForOperand(Inst, 1, SVAKind::Vector);
    return true;
  }

  case VPInstruction::GeneralMemOptConflict: {
    // Each lanes has its own value.
    setSVAKindForReturnValue(Inst, SVAKind::Vector);
    // Each lanes has its own value.
    setSVAKindForInst(Inst, SVAKind::Vector);
    // Each lanes has its own value.
    setSVAKindForAllOperands(Inst, SVAKind::Vector);
    return true;
  }

  case VPInstruction::TreeConflict: {
    // Each lanes has its own value.
    setSVAKindForReturnValue(Inst, SVAKind::Vector);
    // Each lanes has its own value.
    setSVAKindForInst(Inst, SVAKind::Vector);
    // Each lanes has its own value.
    setSVAKindForAllOperands(Inst, SVAKind::Vector);
    return true;
  }

  case VPInstruction::ConflictInsn: {
    // Wide value is shared across all lanes.
    setSVAKindForReturnValue(Inst, SVAKind::FirstScalar);
    // Instruction itself produces scalar value, but it is vector in nature.
    setSVAKindForInst(Inst, SVAKind::Vector);
    // Each lanes has its own value.
    setSVAKindForAllOperands(Inst, SVAKind::Vector);
    return true;
  }

  case VPInstruction::Permute: {
    // Each lanes has its own value.
    setSVAKindForReturnValue(Inst, SVAKind::Vector);
    // Each lanes has its own value.
    setSVAKindForInst(Inst, SVAKind::Vector);
    // Each lanes has its own value.
    setSVAKindForAllOperands(Inst, SVAKind::Vector);
    return true;
  }

  case VPInstruction::CvtMaskToInt: {
    // Wide value is shared across all lanes.
    setSVAKindForReturnValue(Inst, SVAKind::FirstScalar);
    // Instruction itself produces scalar value, but it is vector in nature.
    setSVAKindForInst(Inst, SVAKind::Vector);
    // Each lanes has its own value.
    setSVAKindForAllOperands(Inst, SVAKind::Vector);
    return true;
  }

  case VPInstruction::ExtractLastVectorLane: {
    setSVAKindForInst(Inst, SVAKind::FirstScalar);
    // Each lanes has its own value.
    setSVAKindForAllOperands(Inst, SVAKind::Vector);
    return true;
  }

  case VPInstruction::RunningInclusiveReduction:
  case VPInstruction::RunningExclusiveReduction: {
    // Instruction is vector.
    setSVAKindForInst(Inst, SVAKind::Vector);
    // First operand is vector, but the second and third are scalar.
    setSVAKindForOperand(Inst, 0 /*FirstArg*/, SVAKind::Vector);
    setSVAKindForOperand(Inst, 1 /*SecondArg*/, SVAKind::FirstScalar);
    setSVAKindForOperand(Inst, 2 /*ThirdArg*/, SVAKind::FirstScalar);
    return true;
  }

  case VPInstruction::CompressStore: {
    setSVAKindForInst(Inst, SVAKind::Vector);
    setSVAKindForOperand(Inst, 0, SVAKind::Vector);
    setSVAKindForOperand(Inst, 1, SVAKind::FirstScalar);
    return true;
  }
  case VPInstruction::CompressStoreNonu: {
    setSVAKindForInst(Inst, SVAKind::Vector);
    setSVAKindForOperand(Inst, 0, SVAKind::Vector);
    setSVAKindForOperand(Inst, 1, SVAKind::Vector);
    return true;
  }
  case VPInstruction::ExpandLoad: {
    setSVAKindForInst(Inst, SVAKind::Vector);
    setSVAKindForReturnValue(Inst, SVAKind::Vector);
    setSVAKindForOperand(Inst, 0, SVAKind::FirstScalar);
    return true;
  }
  case VPInstruction::ExpandLoadNonu: {
    setSVAKindForInst(Inst, SVAKind::Vector);
    setSVAKindForReturnValue(Inst, SVAKind::Vector);
    setSVAKindForOperand(Inst, 0, SVAKind::Vector);
    return true;
  }
  case VPInstruction::CompressExpandIndexInit: {
    setSVAKindForInst(Inst, SVAKind::Vector);
    setSVAKindForAllOperands(Inst, SVAKind::FirstScalar);
    return true;
  }
  case VPInstruction::CompressExpandIndexFinal: {
    setSVAKindForInst(Inst, SVAKind::FirstScalar);
    setSVAKindForAllOperands(Inst, SVAKind::FirstScalar);
    return true;
  }
  case VPInstruction::CompressExpandIndex: {
    setSVAKindForInst(Inst, SVAKind::Vector);
    setSVAKindForReturnValue(Inst, SVAKind::Vector);
    setSVAKindForOperand(Inst, 0, SVAKind::FirstScalar);
    return true;
  }
  case VPInstruction::CompressExpandIndexInc: {
    setSVAKindForInst(Inst, SVAKind::Vector);
    setSVAKindForReturnValue(Inst, SVAKind::Vector);
    setSVAKindForOperand(Inst, 0, SVAKind::Vector);
    return true;
  }

  case VPInstruction::SOAExtractValue: {
    // Instruction is scalar
    setSVAKindForInst(Inst, SVAKind::FirstScalar);

    // Return and aggregate argument are vector
    setSVAKindForReturnValue(Inst, SVAKind::Vector);
    setSVAKindForOperand(Inst, 0, SVAKind::Vector);

    // Index arguments are scalar
    for (unsigned I = 1; I < Inst->getNumOperands(); ++I) {
      setSVAKindForOperand(Inst, I, SVAKind::FirstScalar);
    }
    return true;
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

void VPlanScalVecAnalysis::compute(const VPInstruction *VPInst) {
  auto *DA = Plan->getVPlanDA();

  // Allocate expected number of elements for OperandBits. Default state is
  // all-zero for the bits so it would be invalid (assertion) to access them
  // anyways.
  if (VPlanSVAResults[VPInst].OperandBits.empty())
    VPlanSVAResults[VPInst].OperandBits.resize(VPInst->getNumOperands());

  // Case 1: Specially processed instruction in SVA.
  if (computeSpecialInstruction(VPInst)) {
    LLVM_DEBUG(dbgs() << "[SVA] Specially processed instruction ";
               VPInst->dump());
    assert(isSVASpecialProcessedInst(VPInst) &&
           "Specially processed instruction interface not updated");
    return;
  }

  assert(!isSVASpecialProcessedInst(VPInst) &&
         "Specially processed instruction cannot reach here.");

  // Check if instruction's nature was already analyzed.
  Optional<SVABits> InstBits = findSVABitsForInst(VPInst);
  // Combined nature from all use sites of instruction.
  SVABits CombinedUseBits = getAllSetBitsFromUsers(VPInst);

  // Case 2: This is a new VPInst not found in table yet since it has no
  // use-site bits. Determine its ScalVec nature using DA.
  if (CombinedUseBits.none()) {
    assert(InstBits == None && "Instruction is not expected to have SVABits.");
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
    return;
  }

  // Case 3: Decide instruction nature by combining SVABits from all its use
  // sites. We refine the results further if instruction's nature was already
  // known, for example during back propagation.
  if (!DA->isDivergent(*VPInst)) {
    // If the instruction has several uses but DA informs us that it is uniform,
    // then propagate scalar nature for operands and for the instruction.
    // Consider the example below -
    //
    // %a = add %uniform.op1, %uniform.op2
    // %b = mul %a, %divergent.op2
    //
    // The mul instruction will mark %a to be vector in nature in its operand
    // bits, but %uniform.op1 and %uniform.op2 need not be vector in nature for
    // this instruction pattern. They can be scalar nature (for first lane), and
    // %a can be updated to be scalar in nature only (drop other existing bits).
    // Its need of broadcast (for vector context) can be obtained by inspecting
    // the operand level bits at use site in %b.

    SVAKind NewSVAKind = SVAKind::FirstScalar;
    if (VPInst->mayHaveSideEffects()) {
      assert(false &&
             "Instruction with side effects is not safe to scalarize.");
      // For prod/release build vectorize the instruction conservatively.
      NewSVAKind = SVAKind::Vector;
    }

    setSVAKindForInst(VPInst, NewSVAKind);
    setSVAKindForAllOperands(VPInst, NewSVAKind);
    return;
  } else {
    // Propagate all set bits from users for instruction and its operands. If
    // the instruction already has some analyzed bits, then we refine it further
    // by or-ing its current bits with user bits.
    if (InstBits == None) {
      // Instruction has not been analyzed yet, initialize with empty bits.
      SVABits NullBits = 0;
      setSVABitsForInst(VPInst, NullBits);
      setSVABitsForAllOperands(VPInst, NullBits);
    }

    orSVABitsForInst(VPInst, CombinedUseBits);
    orSVABitsForAllOperands(VPInst, CombinedUseBits);
  }
}

void VPlanScalVecAnalysis::compute(VPlanVector *P) {
  Plan = P;

  // TODO: Is it okay to clear the table before a fresh compute cycle?
  clear();
  VPBasicBlock *CFGEntry = &Plan->getEntryBlock();
  for (auto *VPBB : post_order(CFGEntry)) {
    LLVM_DEBUG(dbgs() << "[SVA] Visiting BB " << VPBB->getName() << "\n");
    for (VPInstruction &Inst : reverse(*VPBB)) {
      // Compute SVA results for instruction during forward propagation.
      compute(&Inst);
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
      // Ignore user if the instruction is used by itself.
      if (UserInst == Inst)
        continue;

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
    const VPPHINode *Phi, SVABits &SetBits) {
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
        // Allow skipped loop header PHIs whose only users are other loop header
        // PHIs. Check PHI handling in computeSpecialInstruction for more
        // details.
        if (!isa<VPPHINode>(OpInst) ||
            !SkippedPHIs.count(cast<VPPHINode>(OpInst)))
          // Nothing to do, operand has not been visited yet. No need to process
          // in worklist.
          continue;
      }

      // All checks failed, add to worklist for reprocessing.
      Worklist.insert(OpInst);
    }
  };

  AddInstOperandsToWorklist(Phi);

  // Instructions in the worklist need a chain of propagation, this is triggered
  // because of a PHI node that introduces back-edge or recurrence property
  // (like reduction/induction).
  while (!Worklist.empty()) {
    const VPInstruction *Inst = Worklist.pop_back_val();
    SVABits OrigBits(0);
    if (auto FoundBits = findSVABitsForInst(Inst))
      OrigBits = FoundBits.getValue();
    assert((OrigBits.any() || SkippedPHIs.count(cast<VPPHINode>(Inst))) &&
           "Trying to back propagate to an unprocessed instruction.");
    // Compute SVA results for instruction during back propagation.
    compute(Inst);
    SVABits NewBits = getSVABitsForInst(Inst);
    // Continue back propagation into operands only if nature of instruction has
    // changed.
    if (OrigBits != NewBits)
      AddInstOperandsToWorklist(Inst);
  }
}

bool VPlanScalVecAnalysis::isSVASpecialProcessedInst(
    const VPInstruction *Inst) {
  switch (Inst->getOpcode()) {
  case Instruction::PHI:
    return isLoopHeaderPHI(Plan, cast<VPPHINode>(Inst));
  case Instruction::GetElementPtr:
    return isNonUnitStrSOAGEPWithUnitStrPtr(Plan, cast<VPGEPInstruction>(Inst));
  case Instruction::Load:
  case Instruction::Store:
  case Instruction::Call:
  case VPInstruction::TransformLibraryCall:
  case VPInstruction::InductionInit:
  case VPInstruction::InductionInitStep:
  case VPInstruction::InductionFinal:
  case VPInstruction::ReductionInit:
  case VPInstruction::ReductionFinal:
  case VPInstruction::ReductionFinalUdr:
  case VPInstruction::ReductionFinalInscan:
  case VPInstruction::Pred:
  case VPInstruction::AllocatePrivate:
  case VPInstruction::AllZeroCheck:
  case VPInstruction::VectorTripCountCalculation:
  case VPInstruction::OrigTripCountCalculation:
  case VPInstruction::ActiveLane:
  case VPInstruction::ActiveLaneExtract:
  case VPInstruction::ScalarRemainder:
  case VPInstruction::ScalarPeel:
  case VPInstruction::ScalarPeelHIR:
  case VPInstruction::ScalarRemainderHIR:
  case VPInstruction::PeelOrigLiveOut:
  case VPInstruction::RemOrigLiveOut:
  case VPInstruction::PeelOrigLiveOutHIR:
  case VPInstruction::RemOrigLiveOutHIR:
  case VPInstruction::PushVF:
  case VPInstruction::PopVF:
  case VPInstruction::PrivateFinalMasked:
  case VPInstruction::PrivateFinalMaskedMem:
  case VPInstruction::PrivateFinalUncondMem:
  case VPInstruction::PrivateFinalUncond:
  case VPInstruction::PrivateFinalCondMem:
  case VPInstruction::PrivateFinalCond:
  case VPInstruction::PrivateFinalArray:
  case VPInstruction::PrivateFinalArrayMasked:
  case VPInstruction::PrivateLastValueNonPOD:
  case VPInstruction::PrivateLastValueNonPODMasked:
  case VPInstruction::VLSLoad:
  case VPInstruction::VLSExtract:
  case VPInstruction::VLSInsert:
  case VPInstruction::VLSStore:
  case VPInstruction::InvSCEVWrapper:
  case VPInstruction::GeneralMemOptConflict:
  case VPInstruction::ConflictInsn:
  case VPInstruction::TreeConflict:
  case VPInstruction::Permute:
  case VPInstruction::CvtMaskToInt:
  case VPInstruction::ExtractLastVectorLane:
  case VPInstruction::RunningInclusiveReduction:
  case VPInstruction::RunningExclusiveReduction:
  case VPInstruction::CompressStore:
  case VPInstruction::CompressStoreNonu:
  case VPInstruction::ExpandLoad:
  case VPInstruction::ExpandLoadNonu:
  case VPInstruction::CompressExpandIndexInit:
  case VPInstruction::CompressExpandIndexFinal:
  case VPInstruction::CompressExpandIndex:
  case VPInstruction::CompressExpandIndexInc:
  case VPInstruction::SOAExtractValue:
    return true;
  default:
    return false;
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPlanScalVecAnalysisBase::print(raw_ostream &OS,
                                     const VPInstruction *VPI) {
  OS << "[";
  printSVAKindForInst(OS, VPI);
  OS << "] ";
  VPI->print(OS);
}

void VPlanScalVecAnalysisBase::print(raw_ostream &OS,
                                     const VPBasicBlock *VPBB) {
  for (auto &VPI : *VPBB)
    print(OS, &VPI);
}

void VPlanScalVecAnalysisBase::print(raw_ostream &OS) {
  OS << "\nPrinting ScalVec analysis results for " << Plan->getName() << "\n";
  ReversePostOrderTraversal<VPBasicBlock *> RPOT(&Plan->getEntryBlock());
  for (VPBasicBlock *VPBB : make_range(RPOT.begin(), RPOT.end())) {
    OS << "Basic Block: " << VPBB->getName() << "\n";
    print(OS, VPBB);
  }
}

void VPlanScalVecAnalysis::printSVAKindForInst(raw_ostream &OS,
                                               const VPInstruction *VPI) const {
  // Default if inst doesn't have valid SVABits is 0.
  SVABits InstBits(0);
  if (auto InstBitsOption = findSVABitsForInst(VPI))
    InstBits = InstBitsOption.getValue();
  // Default if inst doesn't have valid return value SVABits is 0.
  SVABits RetValBits(0);
  if (auto RetValBitsOption = findSVABitsForReturnValue(VPI))
    RetValBits = RetValBitsOption.getValue();
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
  // Default if operand doesn't have valid SVABits is 0.
  SVABits OpBits(0);
  if (auto OpBitsOption = findSVABitsForOperand(VPI, OpIdx))
    OpBits = OpBitsOption.getValue();
  if (OpBits.test(static_cast<unsigned>(SVAKind::FirstScalar)))
    OS << "F";
  if (OpBits.test(static_cast<unsigned>(SVAKind::LastScalar)))
    OS << "L";
  if (OpBits.test(static_cast<unsigned>(SVAKind::Vector)))
    OS << "V";
}

void VPlanScalVecAnalysisScalar::printSVAKindForInst(
  raw_ostream &OS, const VPInstruction *VPI) const {
  OS << "(F  )";
}

void VPlanScalVecAnalysisScalar::printSVAKindForOperand(
  raw_ostream &OS, const VPInstruction *VPI, unsigned OpIdx) const {
  OS << "F";
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP
