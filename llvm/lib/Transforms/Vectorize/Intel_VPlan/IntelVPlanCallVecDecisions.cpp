//===-- IntelVPlanCallVecDecisions.cpp -------------------------*- C++ -*-===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
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
#include <algorithm>
#include <iterator>
#include <optional>

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

static void getCartesianProduct(std::vector<std::vector<VFParameter>> &Scratch,
                                std::vector<VFParameter> &CurrentEncodings,
                                std::vector<std::vector<VFParameter>> &Result) {
  // Compute the cartesian product of the previous result with the current
  // vector of parameter encodings. Scratch is used to compute temporary results
  // and then stored back to result, and then the caller passes in the next vector
  // of parameter encodings and we do it again.  E.g.,
  //
  // Result = { u }
  // CurrentEncodings = { l, R, U, L }
  // Scratch = { ul, uR, uU, uL }
  for (auto &PreviousEncodings : Result) {
    for (auto &ParamEncoding : CurrentEncodings) {
      Scratch.push_back(PreviousEncodings);
      Scratch.back().push_back(ParamEncoding);
    }
  }
}

static VPVectorShape getShapeFromTrunc(VPlanVector &Plan,
                                       VPInstruction *Trunc) {
  auto *TruncOp = Trunc->getOperand(0);
  // KnownBits can only be computed for integral types
  auto *VPVT = Plan.getVPVT();
  auto *DA = Plan.getVPlanDA();
  if (VPVT && TruncOp->getType()->isIntegerTy()) {
    auto OpKB = VPVT->getKnownBits(TruncOp, Trunc);
    if (!OpKB.isUnknown()) {
      // Does value fit into signed range bits?
      auto MinSignedVal = OpKB.getSignedMinValue();
      auto MaxSignedVal = OpKB.getSignedMaxValue();
      Type *ToTy = Trunc->getType();
      unsigned ToSize = ToTy->getScalarSizeInBits();
      if (ToSize == 32 &&
          (MinSignedVal.getSExtValue() >= INT_MIN &&
           MaxSignedVal.getSExtValue() <= INT_MAX))
        return DA->getVectorShape(*TruncOp);
    }
  }
  return DA->getVectorShape(*Trunc);
}

void VPlanCallVecDecisions::getVectorVariantsForCallParameters(
    const VPCallInstruction *VPCall, bool Masked, int VF,
    SmallVectorImpl<bool> &ArgIsLinearPrivateMem,
    SmallVectorImpl<VFInfo> &VFInfos, const TargetTransformInfo *TTI) {

  std::vector<std::vector<VFParameter>> Encodings;
  auto VPAA =
      VPlanAlignmentAnalysis(*Plan.getVPSE(), *Plan.getVPVT(), *TTI, VF);

  auto *DA = Plan.getVPlanDA();
  auto SkippedArgs = VPCall->isIntelIndirectCall() ? 1 : 0;
  for (unsigned I = SkippedArgs; I < VPCall->getNumArgOperands(); ++I) {
    auto *CallArg = VPCall->getOperand(I);
    auto CallArgShape = DA->getVectorShape(*CallArg);
    // TODO: this is a workaround for CMPLRLLVM-43350 until the more robust
    // DA solution is ready.
    if (auto *Inst = dyn_cast<VPInstruction>(CallArg)) {
      if (Inst->getOpcode() == Instruction::Trunc)
        CallArgShape = getShapeFromTrunc(Plan, Inst);
    }
    auto ParamPos = I - SkippedArgs;
    const VPValue* LinearPrivMem = nullptr;
    std::vector<VFParameter> ParamEncodings;
    MaybeAlign ArgAlign = std::nullopt;
    bool IsPointerArg = CallArg->getType()->isPointerTy();
    if (IsPointerArg)
      ArgAlign = VPAA.tryGetKnownAlignment(CallArg, VPCall);

    if (CallArgShape.isRandom() || CallArgShape.isUndefined()) {
      ParamEncodings.push_back(VFParameter::vector(ParamPos, ArgAlign));
    } else if (CallArgShape.isAnyStrided() && !IsPointerArg) {
      // Handle linear integer args
      if (CallArgShape.hasKnownStride()) {
        int64_t Stride = CallArgShape.getStrideVal();
        ParamEncodings.push_back(
            VFParameter::linear(ParamPos, Stride, ArgAlign));
      } else {
        ParamEncodings.push_back(
            VFParameter::linearPos(ParamPos, ParamPos, ArgAlign));
      }
    } else if (CallArgShape.isAnyStrided() && IsPointerArg) {
      if (CallArgShape.hasKnownStride()) {
        int64_t Stride = CallArgShape.getStrideVal();
        // Stride is known for the pointer, so possible variants include
        // 'l' and 'R' (ref modifier) encodings since the pointer could
        // be a reference arg.
        ParamEncodings.push_back(
            VFParameter::linear(ParamPos, Stride, ArgAlign));
        ParamEncodings.push_back(
            VFParameter::linearRef(ParamPos, Stride, ArgAlign));
        LinearPrivMem = getVPValuePrivateMemoryPtr(CallArg);
        if (LinearPrivMem) {
          // Adjust stride value for private memory to enable matching for uval
          // and val modifiers. The stride for the pointer to the private memory
          // is unit stride because it will be widened by VF. The stride needed
          // here to match the callee is adjusted to be the stride of the value
          // pointed to, if in fact it is a constant stride. If the adjusted
          // stride is unknown, then a variable stride encoding is used.
          std::vector<int64_t> AdjustedStrides;
          for (auto *User : LinearPrivMem->users()) {
            auto *VPInst = dyn_cast<VPInstruction>(User);
            if (VPInst && VPInst->getOpcode() == Instruction::Store) {
              auto StoredValShape = DA->getVectorShape(*VPInst->getOperand(0));
              if (auto *Op0Inst =
                      dyn_cast<VPInstruction>(VPInst->getOperand(0))) {
                if (Op0Inst->getOpcode() == Instruction::Trunc)
                  StoredValShape = getShapeFromTrunc(Plan, Op0Inst);
              }
              if (!StoredValShape.hasKnownStride() ||
                  StoredValShape.isUniform()) {
                AdjustedStrides.clear();
                break;
              }
              int64_t AdjustedStride = StoredValShape.getStrideVal();
              AdjustedStrides.push_back(AdjustedStride);
            }
          }

          // If we have multiple stores of a value to private memory, make sure
          // all strides match.
          if (AdjustedStrides.size() > 0 &&
              all_of(AdjustedStrides.begin()+1, AdjustedStrides.end(),
                  [&](int64_t S) { return S == AdjustedStrides[0];})) {
            // Since we can prove that the values stored to the private memory
            // are linear, encode the val/uval variants.
            ParamEncodings.push_back(
                VFParameter::linearUVal(ParamPos, AdjustedStrides[0],
                                        ArgAlign));
            ParamEncodings.push_back(
                VFParameter::linearVal(ParamPos, AdjustedStrides[0], ArgAlign));
          } else {
            // Stride was unknown or inconsistent, so we can try to match
            // variable stride cases.
            ParamEncodings.push_back(
                VFParameter::linearUValPos(ParamPos, ParamPos, ArgAlign));
            ParamEncodings.push_back(
                VFParameter::linearValPos(ParamPos, ParamPos, ArgAlign));
          }
        }
      } else {
        // Stride of pointer is unknown, so possible variants include 'ls'
        // and 'Rs'. 'U' and 'L' variants, uval and val, respectively are
        // not a possibility because we can't prove the linearity of the
        // values stored to the pointer.
        ParamEncodings.push_back(
            VFParameter::linearPos(ParamPos, ParamPos, ArgAlign));
        ParamEncodings.push_back(
            VFParameter::linearRefPos(ParamPos, ParamPos, ArgAlign));
      }
    } else if (CallArgShape.isUniform()) {
      ParamEncodings.push_back(VFParameter::uniform(ParamPos, ArgAlign));
    } else {
      llvm_unreachable("Invalid parameter kind");
    }

    // TODO: ArgIsLinearPrivateMem is no longer needed for matching since we
    // now build multiple VFInfo objects and let the matching logic decide which
    // is the best one.
    ArgIsLinearPrivateMem.push_back(LinearPrivMem != nullptr);

    Encodings.push_back(ParamEncodings);
  }

  Function *F = VPCall->getCalledFunction();
  assert(F && "Function is expected here.");

  // Get all combinations of parameter encodings and create a VFInfo for each
  // one.
  std::vector<std::vector<VFParameter>> Scratch;
  std::vector<std::vector<VFParameter>> Result = {{}};
  for (auto &ParamEncoding : Encodings) {
    getCartesianProduct(Scratch, ParamEncoding, Result);
    Result = std::move(Scratch);
  }

  for (unsigned long i=0; i<Result.size(); ++i) {
    SmallVector<VFParameter, 8> Parameters;
    for (unsigned long j=0; j<Result[i].size(); ++j) {
      Parameters.push_back(Result[i][j]);
    }
    VFInfo CallVariant =
        VFInfo::get(VFISAKind::Unknown, Masked, VF, Parameters,
                    F->getName().str());
    LLVM_DEBUG(dbgs() << "Call Variant: " << CallVariant.VectorName << "\n");
    VFInfos.push_back(CallVariant);
  }
}

std::optional<std::pair<VFInfo, unsigned>>
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

  // Currently uval args are not supported when unoptimized IR is presented to
  // VecClone. Filter those variants out of the list before doing the matching
  // in TTI.
  const Function *F = Call->getParent()->getParent();
  bool OptNoneFunc = F->hasFnAttribute(Attribute::OptimizeNone);
  SmallVector<VFInfo, 4> FilteredVariants;
  for (auto &Variant : Variants) {
    ArrayRef<VFParameter> Parms = Variant.getParameters();
    if (OptNoneFunc && any_of(Parms, [](VFParameter Parm) {
                return Parm.isLinearUVal();
              }))
      continue;
    FilteredVariants.push_back(Variant);
  }

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
  SmallVector<VFInfo, 8> VFInfos;
  getVectorVariantsForCallParameters(VPCall, Masked, VF, ArgIsLinearPrivateMem,
                                     VFInfos, TTI);

  int VariantIdx =
      TTI->getMatchingVectorVariant(VFInfos, FilteredVariants,
                                    Call->getModule(), ArgIsLinearPrivateMem);

  if (VariantIdx >= 0) {
    LLVM_DEBUG(dbgs() << "\nMatched call to: "
                      << FilteredVariants[VariantIdx].VectorName << "\n\n");
    return std::make_pair(FilteredVariants[VariantIdx], (unsigned)VariantIdx);
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

  // Call to builtin_prefetch should not be vectorized(widened/scalarized).
  // This is especially problematic for cases where the pointer being prefetched
  // is unit strided where we unnecessarily generate VF prefetches from
  // {ptr, ptr + 1, ..., ptr + VF -1}. This can lead to big performance
  // regressions especially in hot loops due to the extracts needed to generate
  // the scalarized calls. Classic compiler handles prefetches the same way.
  if (VPCall->isIntrinsicFromList({Intrinsic::prefetch})) {
    VPCall->setShouldNotBeWidened();
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

  // llvm.intel.directive elementsize intrinsics should not be widened.
  if (VPCall->isElementsizeIntrinsic()) {
    VPCall->setShouldNotBeWidened();
    return;
  }

  // llvm.stacksave and llvm.stackrestore intrinsics should not be widened.
  if (VPCall->isIntrinsicFromList(
          {Intrinsic::stacksave, Intrinsic::stackrestore})) {
    VPCall->setShouldNotBeWidened();
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

  // Calls with kernel-call-once cannot be serialized or pumped
  if (VPCall->isKernelCallOnce()) {
    VPCall->setShouldNotBeWidened();
    return;
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
                                  IsMasked, TTI)) {
    VPCall->setVectorizeWithLibraryFn(TLI->getVectorizedFunction(
        CalledFuncName, ElementCount::getFixed(VF), IsMasked, TTI));
    return;
  }

  // Vectorize by pumping the call for a lower VF.
  unsigned PumpFactor = getPumpFactor(*UnderlyingCI, IsMasked, VF, TLI, TTI);
  if (PumpFactor > 1) {
    unsigned LowerVF = VF / PumpFactor;
    assert(TLI->isFunctionVectorizable(
               *UnderlyingCI, ElementCount::getFixed(LowerVF), IsMasked, TTI) &&
           "Library function cannot be vectorized with lower VF.");
    VPCall->setVectorizeWithLibraryFn(
        TLI->getVectorizedFunction(
            CalledFuncName, ElementCount::getFixed(LowerVF), IsMasked, TTI),
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

  // All other cases implies default properties i.e. call serialization.
  VPCall->setShouldBeSerialized();
  VPCall->setSerializationReason
      (VPCallInstruction::SerializationReasonTy::NO_VECTOR_VARIANT);
  // TODO:
  // 1. OpenCLReadChannel/OpenCLWriteChannel calls?
}
