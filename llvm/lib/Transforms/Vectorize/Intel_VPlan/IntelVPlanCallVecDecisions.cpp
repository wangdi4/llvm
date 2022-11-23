//===-- IntelVPlanCallVecDecisions.cpp -------------------------*- C++ -*-===//
//
//   Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanCallVecDecisions.h"
#include "IntelVPlanUtils.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "VPlanCallVecDecisions"

using namespace llvm::vpo;

static bool isVPPopVF(const VPInstruction *I) {
  return I->getOpcode() == VPInstruction::PopVF;
}

void VPlanCallVecDecisions::runForVF(unsigned VF, const TargetLibraryInfo *TLI,
                                     const TargetTransformInfo *TTI) {
  LLVM_DEBUG(dbgs() << "Running CallVecDecisions for VF=" << VF << "\n");
  for (VPBasicBlock &VPBB : Plan) {
    for (VPInstruction &Inst : VPBB) {
      // Handle cases when incorrect interface is called for merged CFG.
      if (isa<VPPushVF>(&Inst) || isVPPopVF(&Inst)) {
        assert(false && "runForVF cannot be called for merged CFG. Use "
                        "runForMergedCFG instead.");
        // For release build, safely reset and call correct interface.
        reset();
        runForMergedCFG(TLI, TTI);
        return;
      }

      if (auto *VPCall = dyn_cast<VPCallInstruction>(&Inst))
        analyzeCall(VPCall, VF, TLI, TTI);
    }
  }
}

void VPlanCallVecDecisions::runForMergedCFG(const TargetLibraryInfo *TLI,
                                            const TargetTransformInfo *TTI) {
  LLVM_DEBUG(dbgs() << "Running CallVecDecisions for merged CFG\n");
  // Stack to track various VFs encountered in merged CFG.
  std::stack<unsigned> VFStack;
  // Current VF that must be used to analyze a call. It effecively tracks the
  // stack's top element.
  unsigned CurrentVF;
  ReversePostOrderTraversal<VPBasicBlock *> RPOT(&Plan.getEntryBlock());
  for (VPBasicBlock *VPBB : RPOT) {
    for (VPInstruction &Inst : *VPBB) {
      if (auto *PushVF = dyn_cast<VPPushVF>(&Inst))
        VFStack.push(PushVF->getVF());

      if (isVPPopVF(&Inst))
        VFStack.pop();

      CurrentVF = VFStack.empty() ? 0 : VFStack.top();

      if (auto *VPCall = dyn_cast<VPCallInstruction>(&Inst)) {
        assert(CurrentVF != 0 && "Valid VF was not identified for merged CFG.");
        analyzeCall(VPCall, CurrentVF, TLI, TTI);
      }
    }
  }

  assert(VFStack.empty() && "Expected empty VF stack.");
}

void VPlanCallVecDecisions::reset() {
  LLVM_DEBUG(dbgs() << "Resetting CallVecDecisions for Plan: " << Plan.getName()
                    << "\n");
  for (VPBasicBlock &VPBB : Plan) {
    for (VPInstruction &Inst : VPBB) {
      if (auto *VPCall = dyn_cast<VPCallInstruction>(&Inst))
        VPCall->resetVecScenario(0 /*NewVF*/);
    }
  }
}

VFInfo VPlanCallVecDecisions::getVectorVariantForCallParameters(
    const VPCallInstruction *VPCall, bool Masked, int VF,
    SmallVectorImpl<bool> &ArgIsLinearPrivateMem) {

  auto VPAA = VPlanAlignmentAnalysis(*Plan.getVPSE(), *Plan.getVPVT(), VF);

  auto *DA = Plan.getVPlanDA();
  SmallVector<VFParameter, 8> Parameters;
  auto SkippedArgs = VPCall->isIntelIndirectCall() ? 1 : 0;
  for (unsigned I = SkippedArgs; I < VPCall->getNumArgOperands(); ++I) {
    auto *CallArg = VPCall->getOperand(I);
    auto CallArgShape = DA->getVectorShape(*CallArg);
    auto ParamPos = I - SkippedArgs;
    const VPValue* LinearPrivMem = nullptr;
    if (CallArgShape.isRandom() || CallArgShape.isUndefined()) {
      Parameters.push_back(VFParameter::vector(ParamPos));
    } else if (CallArgShape.isAnyStrided()) {
      LinearPrivMem = getVPValuePrivateMemoryPtr(CallArg);
      if (CallArgShape.hasKnownStride()) {
        Parameters.push_back(
            VFParameter::linear(ParamPos, CallArgShape.getStrideVal()));
      } else {
        // Note: this function builds a VFInfo (vector variant encoding) from
        // the caller side using DA so that later it can be used to find a
        // compatible match on the callee (function) side. Since there isn't a
        // way on the caller side to know where the variable stride argument is
        // located, the same parameter position is used for the stride argument
        // as the current linear argument. This is accounted for later during
        // the matching routine.
        Parameters.push_back(VFParameter::variableStrided(ParamPos, ParamPos));
      }
    } else if (CallArgShape.isUniform()) {
      Parameters.push_back(VFParameter::uniform(ParamPos));
    } else {
      llvm_unreachable("Invalid parameter kind");
    }

    // Parameter matching needs to know that this arg is private memory so that
    // it can distinguish between ref and val/uval cases for linear reference
    // arguments. As an example, if 'i' is the loop index on the caller side,
    // we need to distinguish between 'foo(a[i])' and 'foo(i)'. The latter case
    // will result in private memory being allocated for VF values of 'i'.
    ArgIsLinearPrivateMem.push_back(LinearPrivMem != nullptr);

    // If the call arg is a pointer, use alignment analysis to try to obtain a
    // known alignment for matching against variants with aligned params.
    if (CallArg->getType()->isPointerTy())
      Parameters.back().Alignment = VPAA.tryGetKnownAlignment(CallArg, VPCall);
  }

  Function *F = VPCall->getCalledFunction();
  assert(F && "Function is expected here.");
  return VFInfo::get(VFISAKind::Unknown, Masked, VF, Parameters,
                     F->getName().str());
}

llvm::Optional<std::pair<VFInfo, unsigned>>
VPlanCallVecDecisions::matchVectorVariant(const VPCallInstruction *VPCall,
                                          bool Masked, unsigned VF,
                                          const TargetTransformInfo *TTI) {

  const CallInst *Call = VPCall->getUnderlyingCallInst();
  // TODO: Add support to use called Function attributes when underlying Call is
  // not available.
  if (!Call || !Call->hasFnAttr("vector-variants"))
    return {};

  StringRef VecVariantStringValue =
      Call->getCallSiteOrFuncAttr("vector-variants").getValueAsString();

  assert(!VecVariantStringValue.empty() &&
         "VectorVariant string value shouldn't be empty!");

  LLVM_DEBUG(dbgs() << "Trying to find match for: " << VecVariantStringValue
                    << "\n");
  LLVM_DEBUG(dbgs() << "\nCall VF: " << VF << "\n");

  SmallVector<StringRef, 4> VariantAttributes;
  VecVariantStringValue.split(VariantAttributes, ",");

  SmallVector<VFInfo, 4> Variants(
      map_range(VariantAttributes, [VPCall](StringRef Attr) {
        // Check if the called function name is contained in vector-variant string.
        // TODO: Do the check for all calls (not for indirect ones only) once
        // front-end problems are removed.
        auto *Fn = VPCall->getCalledFunction();
        assert(VPCall->isIntelIndirectCall() ||
               (Fn && Attr.contains(Fn->getName())));
        (void)Fn;
        return VFABI::demangleForVFABI(Attr);
      }));

  if (VPCall->isIntelIndirectCall())
     Masked |= Plan.getVPlanDA()->isDivergent(*VPCall->getOperand(0));

  // Keep track of whether the linear argument to the vector function call is
  // from private memory. Only keep track of this for linears because matching
  // on linear reference parameters is determined by whether or not address
  // is taken on private memory or not. E.g., for linear reference parameters
  // using ref modifier, matching is done on non-private memory. For val/uval
  // modifiers, matching is done on private memory. See matchParameters() in
  // VectorUtils.cpp for more detail.
  SmallVector<bool, 8> ArgIsLinearPrivateMem;
  const VFInfo VariantForCall =
      getVectorVariantForCallParameters(VPCall, Masked, VF,
                                        ArgIsLinearPrivateMem);

  int VariantIdx =
      TTI->getMatchingVectorVariant(VariantForCall, Variants,
                                    Call->getModule(), ArgIsLinearPrivateMem);

  if (VariantIdx >= 0) {
    LLVM_DEBUG(dbgs() << "\nMatched call to: " << Variants[VariantIdx].VectorName
                      << "\n\n");
    return std::make_pair(Variants[VariantIdx], (unsigned)VariantIdx);
  }

  return {};
}

Type *
VPlanCallVecDecisions::calcCharacteristicType(VPCallInstruction *VPCallInst,
                                              const VFInfo &Variant) {
  assert(VPCallInst && "VPCallInst should not be nullptr");
  auto Args  = VPCallInst->arg_operands();
  if (VPCallInst->isIntelIndirectCall())
    Args = llvm::drop_begin(Args, 1);
  return llvm::calcCharacteristicType(
      VPCallInst->getType(),
      map_range(Args, [](VPValue* V)->VPValue& {return *V;}),
      Variant, *VPCallInst->getParent()->getParent()->getDataLayout());
}

void VPlanCallVecDecisions::analyzeCall(VPCallInstruction *VPCall, unsigned VF,
                                        const TargetLibraryInfo *TLI,
                                        const TargetTransformInfo *TTI) {
  assert(
      !isa<VPTransformLibraryCall>(VPCall) &&
      "VPTransformLibraryCalls should only be run *after* call vec decisions.");

  const CallInst *UnderlyingCI = VPCall->getUnderlyingCallInst();

  // Reset decisions that were taken for any previous VF as they will be
  // overwritten for currently analyzed VF.
  VPCall->resetVecScenario(VF);

  // Analysis would be trivial for VF=1 since all calls should just be scalar.
  if (VF == 1)
    return;

  // Ignored calls (do we need a new field in VecProperties for this?)
  if (isa_and_nonnull<DbgInfoIntrinsic>(UnderlyingCI))
    return;

  Function *F = VPCall->getCalledFunction();

  // Call was already marked to be strictly not widended (for example, kernel
  // convergent uniform calls).
  if (VPCall->getVectorizationScenario() ==
      VPCallInstruction::CallVecScenariosTy::DoNotWiden) {
    return;
  }

  // DPC++'s unmasked functions. The implementation is vector-variant based but
  // no pumping allowed and the mask needs to be ignore when making the call
  // (the whole purpose of the feature), hence separate scenario.
  if (VPCall->getVectorizationScenario() ==
      VPCallInstruction::CallVecScenariosTy::UnmaskedWiden) {
    auto VecVariant = matchVectorVariant(VPCall, false, VF, TTI);
    VPCall->setUnmaskedVectorVariant(std::move(VecVariant->first),
                                     VecVariant->second);
    return;
  }

  // Indirect calls will be serialized, as of today.
  if (!F) {
    VPCall->setSerializationReason(VPCallInstruction::
        SerializationReasonTy::INDIRECT_CALL);
    VPCall->setShouldBeSerialized();
    return;
  }

  StringRef CalledFuncName = F->getName();
  // Currently we assume CallVecDecisions analysis is run after predication. So
  // call is masked only if its parent VPBB has predicate.
  VPBasicBlock *VPBB = VPCall->getParent();
  bool IsMasked = VPBB->getPredicate() != nullptr;

  // Function calls with available vector variants.
  if (auto VecVariant = matchVectorVariant(VPCall, IsMasked, VF, TTI)) {
    VPCall->setVectorizeWithVectorVariant(std::move(VecVariant->first),
                                          VecVariant->second);
    return;
  }

  // Use masked vector variant with all-zero mask for unmasked calls without
  // matching vector variant.
  // TODO: Same optimization can be done for calls with vectorizable library
  // function.
  if (!IsMasked) {
    if (auto MaskedVecVariant = matchVectorVariant(VPCall, true, VF, TTI)) {
      VPCall->setVectorizeWithVectorVariant(std::move(MaskedVecVariant->first),
                                            MaskedVecVariant->second,
                                            true /*UseMaskedForUnmasked*/);
      return;
    }
  }

  // If underlying CallInst is not available for further analysis, serialize
  // conservatively if current decision is undefined.
  if (!UnderlyingCI) {
    if (VPCall->getVectorizationScenario() ==
        VPCallInstruction::CallVecScenariosTy::Undefined) {
      VPCall->setShouldBeSerialized();
    } else {
      assert(VPCall->getVFForScenario() == VF &&
             "No known scenario for call without underlying CI.");
    }
    VPCall->setSerializationReason(VPCallInstruction::
        SerializationReasonTy::CURRENT_CONTEXT);
    return;
  }

  // For all other scenarios below, underlying CI is a hard-requirement.
  // TODO: Explore options to relax the hard-requirement for future.
  assert(UnderlyingCI && "UnderlyingCI is nullptr.");

  // For OpenCL sincos, we can vectorize with SVML version only if cos pointer
  // operands is a private pointer in SOA layout. Serialize in other cases to
  // avoid stability issues, because the vector version accepts pointer to a
  // vector and not vector of pointers.
  if (isOpenCLSinCos(CalledFuncName)) {
    VPValue *CosPtr = VPCall->getOperand(1);
    auto *PvtCosPtr = getVPValuePrivateMemoryPtr(CosPtr);
    if (!PvtCosPtr || !cast<VPAllocatePrivate>(PvtCosPtr)->isSOALayout()) {
      VPCall->setShouldBeSerialized();
      VPCall->setSerializationReason(
          VPCallInstruction::SerializationReasonTy::CURRENT_CONTEXT);
      return;
    }
  }

  // Vectorizable library function like SVML calls. Set vector function name in
  // CallVecProperties.
  if (TLI->isFunctionVectorizable(*UnderlyingCI, ElementCount::getFixed(VF),
                                  IsMasked)) {
    VPCall->setVectorizeWithLibraryFn(TLI->getVectorizedFunction(
        CalledFuncName, ElementCount::getFixed(VF), IsMasked));
    return;
  }

  // Vectorize by pumping the call for a lower VF.
  unsigned PumpFactor = getPumpFactor(*UnderlyingCI, IsMasked, VF, TLI);
  if (PumpFactor > 1) {
    unsigned LowerVF = VF / PumpFactor;
    assert(TLI->isFunctionVectorizable(
               *UnderlyingCI, ElementCount::getFixed(LowerVF), IsMasked) &&
           "Library function cannot be vectorized with lower VF.");
    VPCall->setVectorizeWithLibraryFn(
        TLI->getVectorizedFunction(CalledFuncName,
                                   ElementCount::getFixed(LowerVF), IsMasked),
        PumpFactor);
    return;
  }

  // Trivially vectorizable call using vector intrinsics.
  Intrinsic::ID ID = getVectorIntrinsicIDForCall(UnderlyingCI, TLI);
  bool TrivialVectorIntrinsic =
      (ID != Intrinsic::not_intrinsic) && isTriviallyVectorizable(ID);
  if (TrivialVectorIntrinsic) {
    // Check if the vectorizable intrinsic call has any always scalar operand
    // (for example, cttz, ctlz, powi) that is loop variant/divergent. Such
    // intrinsic calls cannot be vectorized, they should be strictly serialized.
    for (auto *ArgOp : VPCall->arg_operands()) {
      unsigned ArgIdx = VPCall->getOperandIndex(ArgOp);
      if (isVectorIntrinsicWithScalarOpAtArg(ID, ArgIdx) &&
          Plan.getVPlanDA()->isDivergent(*ArgOp)) {
        VPCall->setSerializationReason(VPCallInstruction::
            SerializationReasonTy::SCALAR_OPERANDS);
        VPCall->setShouldBeSerialized();
        return;
      }
    }

    VPCall->setVectorizeWithIntrinsic(ID);
    return;
  }

  // Deterministic function calls with no side effects that operate on uniform
  // operands need to be marked as DoNotWiden.
  if (!Plan.getVPlanDA()->isDivergent(*VPCall) &&
      (!VPCall->mayHaveSideEffects() ||
       VPCall->isIntrinsicFromList({Intrinsic::assume}))) {
    // NOTE: If assert is triggered, it implies that DA has been updated to
    // allow non-uniform operands for uniform call. This would require some
    // additional updates to handling of different cases in this analysis
    // (potentially having to introduce a new VecScenario).
    assert(llvm::all_of(VPCall->arg_operands(),
                        [this](VPValue *V) {
                          return !Plan.getVPlanDA()->isDivergent(*V);
                        }) &&
           "Operands of call are not uniform.");
    VPCall->setShouldNotBeWidened();
    return;
  }

  // llvm.experimental.noalias.scope.decl intrinsic calls should not be widened.
  if (VPCall->isIntrinsicFromList(
          {Intrinsic::experimental_noalias_scope_decl})) {
    VPCall->setShouldNotBeWidened();
    return;
  }

  // lifetime_start/end intrinsics operating on private memory optimized for
  // SOA-layout are not widened.
  if (VPCall->isLifetimeStartOrEndIntrinsic()) {
    auto *PrivPtr = dyn_cast_or_null<VPAllocatePrivate>(
        getVPValuePrivateMemoryPtr(VPCall->getOperand(1)));
    if (PrivPtr && PrivPtr->isSOALayout()) {
      VPCall->setShouldNotBeWidened();
      return;
    }
  }

  // All other cases implies default properties i.e. call serialization.
  VPCall->setShouldBeSerialized();
  VPCall->setSerializationReason
      (VPCallInstruction::SerializationReasonTy::NO_VECTOR_VARIANT);
  // TODO:
  // 1. OpenCLReadChannel/OpenCLWriteChannel calls?
}
