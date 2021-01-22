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
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/Support/CommandLine.h"

#define DEBUG_TYPE "VPlanCallVecDecisions"

using namespace llvm::vpo;

static cl::opt<bool, true> VPlanVecNonReadonlyLibCallsOpt(
    "vplan-vectorize-non-readonly-libcalls", cl::Hidden,
    cl::location(VPlanVecNonReadonlyLibCalls),
    cl::desc(
        "Vectorize library calls even if they don't have readonly attribute."));

namespace llvm {
namespace vpo {
bool VPlanVecNonReadonlyLibCalls = true;
} // namespace vpo
} // namespace llvm

void VPlanCallVecDecisions::run(unsigned VF, const TargetLibraryInfo *TLI,
                                const TargetTransformInfo *TTI) {
  LLVM_DEBUG(dbgs() << "Running CallVecDecisions for VF=" << VF << "\n");
  for (VPBasicBlock &VPBB : Plan) {
    for (VPInstruction &Inst : VPBB) {
      if (auto *VPCall = dyn_cast<VPCallInstruction>(&Inst))
        analyzeCall(VPCall, VF, TLI, TTI);
    }
  }
}

std::unique_ptr<VectorVariant>
VPlanCallVecDecisions::getVectorVariantForCallParameters(
    const VPCallInstruction *VPCall,
    bool Masked,
    int VF) {
  auto *DA = Plan.getVPlanDA();
  std::vector<VectorKind> ParmKinds;
  for (unsigned I = VPCall->isIntelIndirectCall() ? 1 : 0;
       I < VPCall->getNumArgOperands(); ++I) {
    auto *CallArg = VPCall->getOperand(I);
    auto CallArgShape = DA->getVectorShape(CallArg);
    if (CallArgShape.isRandom() || CallArgShape.isUndefined())
      ParmKinds.push_back(VectorKind::vector());
    else if (CallArgShape.isAnyStrided() && CallArgShape.hasKnownStride())
      ParmKinds.push_back(VectorKind::linear(CallArgShape.getStrideVal()));
    else if (CallArgShape.isAnyStrided() && !CallArgShape.hasKnownStride())
      ParmKinds.push_back(VectorKind::variableStrided(I));
    else if (CallArgShape.isUniform())
      ParmKinds.push_back(VectorKind::uniform());
    else
      llvm_unreachable("Invalid parameter kind");
  }

  return std::make_unique<VectorVariant>(
             VectorVariant::ISAClass::OTHER, Masked, VF, ParmKinds,
             VPCall->getCalledFunction()->getName().str(),
             "" /* Alias not needed */);
}

llvm::Optional<std::pair<std::unique_ptr<VectorVariant>, unsigned>>
VPlanCallVecDecisions::matchVectorVariant(const VPCallInstruction *VPCall,
                                          bool Masked, unsigned VF,
                                          const TargetTransformInfo *TTI) {

  const CallInst *Call = VPCall->getUnderlyingCallInst();
  if (!Call->hasFnAttr("vector-variants"))
    return {};

  StringRef VecVariantStringValue =
      Call->getFnAttr("vector-variants").getValueAsString();

  assert(!VecVariantStringValue.empty() &&
         "VectorVariant string value shouldn't be empty!");

  LLVM_DEBUG(dbgs() << "Trying to find match for: " << VecVariantStringValue
                    << "\n");
  LLVM_DEBUG(dbgs() << "\nCall VF: " << VF << "\n");

  SmallVector<StringRef, 4> VariantAttributes;
  VecVariantStringValue.split(VariantAttributes, ",");
  SmallVector<VectorVariant, 4> Variants;

  for (unsigned I = 0; I < VariantAttributes.size(); ++I) {
    // Check if the called function name is contained in vector-variant string.
    // TODO: Do the check for all calls (not for indirect ones only) once
    // front-end problems are removed.
    auto *Fn = VPCall->getCalledFunction();
    assert(VPCall->isIntelIndirectCall() ||
           (Fn && VariantAttributes[I].contains(Fn->getName())));
    (void)Fn;
    Variants.push_back(VectorVariant(VariantAttributes[I]));
  }

  std::unique_ptr<VectorVariant> VariantForCall =
      getVectorVariantForCallParameters(VPCall, Masked, VF);

  int VariantIdx =
      TTI->getMatchingVectorVariant(*VariantForCall, Variants,
                                    Call->getModule());

  if (VariantIdx >= 0) {
    LLVM_DEBUG(dbgs() << "\nMatched call to: " << Variants[VariantIdx].encode()
                      << "\n\n");
    return std::make_pair(
        std::make_unique<VectorVariant>(Variants[VariantIdx]), (unsigned)VariantIdx);
  }

  return {};
}

Type *
VPlanCallVecDecisions::calcCharacteristicType(VPCallInstruction *VPCallInst,
                                              VectorVariant &Variant) {
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
  const CallInst *UnderlyingCI = VPCall->getUnderlyingCallInst();

  // Reset decisions that were taken for any previous VF as they will be
  // overwritten for currently analyzed VF.
  VPCall->resetVecScenario(VF);

  // Analysis would be trivial for VF=1 since all calls should just be scalar.
  if (VF == 1)
    return;

  // Ignored calls (do we need a new field in VecProperties for this?)
  if (isa<DbgInfoIntrinsic>(UnderlyingCI))
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
    VPCall->setUnmaskedVectorVariant(VecVariant->first, VecVariant->second);
    return;
  }

  // Indirect calls will be serialized, as of today.
  if (!F) {
    VPCall->setShouldBeSerialized();
    return;
  }

  StringRef CalledFuncName = F->getName();
  // Currently we assume CallVecDecisions analysis is run after predication. So
  // call is masked only if its parent VPBB has predicate.
  VPBasicBlock *VPBB = VPCall->getParent();
  bool IsMasked = VPBB->getPredicate() != nullptr;
  // Vectorizable library function like SVML calls. Set vector function name in
  // CallVecProperties. NOTE : Vector library calls can be used if call
  // is known to read memory only (non-default behavior).
  if (TLI->isFunctionVectorizable(CalledFuncName, VF, IsMasked) &&
      (VPlanVecNonReadonlyLibCalls || UnderlyingCI->onlyReadsMemory())) {
    VPCall->setVectorizeWithLibraryFn(
        TLI->getVectorizedFunction(CalledFuncName, VF, IsMasked));
    return;
  }

  // Function calls with available vector variants.
  if (auto VecVariant = matchVectorVariant(VPCall, IsMasked, VF, TTI)) {
    VPCall->setVectorizeWithVectorVariant(VecVariant.getValue().first,
                                          VecVariant.getValue().second);
    return;
  }

  // Use masked vector variant with all-zero mask for unmasked calls without
  // matching vector variant.
  // TODO: Same optimization can be done for calls with vectorizable library
  // function.
  if (!IsMasked) {
    auto MaskedVecVariant = matchVectorVariant(VPCall, true, VF, TTI);
    if (MaskedVecVariant) {
      VPCall->setVectorizeWithVectorVariant(MaskedVecVariant.getValue().first,
                                            MaskedVecVariant.getValue().second,
                                            true /*UseMaskedForUnmasked*/);
      return;
    }
  }

  // Vectorize by pumping the call for a lower VF. Since pumping is only done
  // for library calls today, ensure that call only reads memory. TODO: When
  // vector-variant pumping is implemented, restrict the check for library func
  // call.
  unsigned PumpFactor = getPumpFactor(CalledFuncName, IsMasked, VF, TLI);
  if (PumpFactor > 1 &&
      (VPlanVecNonReadonlyLibCalls || UnderlyingCI->onlyReadsMemory())) {
    unsigned LowerVF = VF / PumpFactor;
    assert(TLI->isFunctionVectorizable(CalledFuncName, LowerVF, IsMasked) &&
           "Library function cannot be vectorized with lower VF.");
    VPCall->setVectorizeWithLibraryFn(
        TLI->getVectorizedFunction(CalledFuncName, LowerVF, IsMasked),
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
      if (hasVectorInstrinsicScalarOpd(ID, ArgIdx) &&
          Plan.getVPlanDA()->isDivergent(*ArgOp)) {
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
      !VPCall->mayHaveSideEffects()) {
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

  // All other cases implies default properties i.e. call serialization.
  VPCall->setShouldBeSerialized();
  // TODO:
  // 1. OpenCLReadChannel/OpenCLWriteChannel calls?
}
