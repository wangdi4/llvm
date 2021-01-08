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

bool VPlanCallVecDecisions::matchAndScoreVariantParameters(
    const VPCallInstruction *VPCall,
    VectorVariant &Variant,
    int &Score,
    int &MaxArg) {
  LLVM_DEBUG(dbgs() << "\nAttempting to match variant parameters for: " <<
             Variant.encode() << "\n");
  // Ensure caller arguments and simd function parameters are compatible.
  // Use DA to determine argument shapes from the caller side. This analysis
  // is needed to do proper matching for both explicit and auto vectorization.
  auto *DA = Plan.getVPlanDA();
  std::vector<VectorKind> Parms;
  Parms = Variant.getParameters();

  std::vector<int> ArgScores;
  for (unsigned I = VPCall->isIntelIndirectCall() ? 1 : 0, J = 0;
       I < VPCall->getNumArgOperands(); ++I, ++J) {
    int ArgScore;
    auto *CallArg = VPCall->getOperand(I);
    auto CallArgShape = DA->getVectorShape(CallArg);

    LLVM_DEBUG(dbgs() << "Call Arg: "; CallArgShape.print(dbgs());
               dbgs() << ' ' << *CallArg << "\n");

    // Linear and uniform arguments can always safely be put into vectors, but
    // reduce score in those cases because scalar is optimal.
    if (Parms[J].isVector()) {
      if (CallArgShape.isRandom())
        ArgScore = Vector2VectorScore;
      else
        ArgScore = Scalar2VectorScore; // uniform/linear -> vector
      ArgScores.push_back(ArgScore);
      Score += ArgScore;
      continue;
    }

    if (Parms[J].isLinear() && CallArgShape.isAnyStrided() &&
        CallArgShape.hasKnownStride() &&
        Parms[J].getStride() == CallArgShape.getStrideVal()) {
      ArgScore = Linear2LinearScore;
      ArgScores.push_back(ArgScore);
      Score += ArgScore;
      continue;
    }

    if (Parms[J].isUniform() && CallArgShape.isUniform()) {
      // Uniform ptr arguments are more beneficial for performance, so weight
      // them accordingly.
      if (isa<PointerType>(CallArg->getType()))
        ArgScore = UniformPtr2UniformPtrScore;
      else
        ArgScore = Uniform2UniformScore;
      ArgScores.push_back(ArgScore);
      Score += ArgScore;
      continue;
    }

    LLVM_DEBUG(dbgs() << "Arg did not match variant parameter!\n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "Args matched variant parameters\n");
  // If two args have the same max score, the 1st is selected.
  MaxArg =
      std::max_element(ArgScores.begin(), ArgScores.end()) - ArgScores.begin();
  LLVM_DEBUG(dbgs() << "MaxArg: " << MaxArg << "\n");
  return true;
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
  unsigned TargetMaxRegWidth = TTI->getRegisterBitWidth(true);
  LLVM_DEBUG(dbgs() << "Target Max Register Width: " << TargetMaxRegWidth
                    << "\n");

  VectorVariant::ISAClass TargetIsaClass;
  switch (TargetMaxRegWidth) {
  case 128:
    TargetIsaClass = VectorVariant::ISAClass::XMM;
    break;
  case 256:
    // Important Note: there is no way to inspect CPU or FeatureBitset from
    // the LLVM compiler middle end (i.e., lib/Analysis, lib/Transforms). This
    // can only be done from the front-end or from lib/Target. Thus, we select
    // avx2 by default for 256-bit vector register targets. Plus, I don't
    // think we currently have anything baked in to TTI to differentiate avx
    // vs. avx2. Namely, whether or not for 256-bit register targets there is
    // 256-bit integer support. TODO: add has256BitIntegerSupport() to TTI.
    TargetIsaClass = VectorVariant::ISAClass::YMM2;
    break;
  case 512:
    TargetIsaClass = VectorVariant::ISAClass::ZMM;
    break;
  default:
    llvm_unreachable("Invalid target vector register width");
  }
  LLVM_DEBUG(dbgs() << "Target ISA Class: "
                    << VectorVariant::ISAClassToString(TargetIsaClass)
                    << "\n\n");

  SmallVector<StringRef, 4> Variants;
  VecVariantStringValue.split(Variants, ",");
  SmallVector<std::pair<VectorVariant, unsigned>, 4> CandidateFunctions;
  for (unsigned I = 0; I < Variants.size(); ++I) {

    // Check if the called function name is contained in vector-variant string.
    // TODO: Do the check for all calls (not for indirect ones only) once
    // front-end problems are removed.
    auto *Fn = VPCall->getCalledFunction();
    assert(VPCall->isIntelIndirectCall() ||
           (Fn && Variants[I].contains(Fn->getName())));
    (void)Fn;

    VectorVariant Variant(Variants[I]);
    VectorVariant::ISAClass VariantIsaClass = Variant.getISA();
    LLVM_DEBUG(dbgs() << "Variant ISA Class: "
                      << VectorVariant::ISAClassToString(VariantIsaClass)
                      << "\n");
    unsigned IsaClassMaxRegWidth =
        VectorVariant::ISAClassMaxRegisterWidth(VariantIsaClass);
    LLVM_DEBUG(dbgs() << "Isa Class Max Vector Register Width: "
                      << IsaClassMaxRegWidth << "\n");
    (void)IsaClassMaxRegWidth;
    unsigned FuncVF = Variant.getVlen();
    LLVM_DEBUG(dbgs() << "Func VF: " << FuncVF << "\n\n");

    // Filter candidate functions by VF, ISA, and mask. Candidates will then
    // be scored by matching parameters and the highest scored function gets
    // selected. Note: strict VF matching for now, but later we can account
    // for multiple pumping scenarios in the scoring once codegen supports
    // this.
    if (FuncVF == VF && VariantIsaClass <= TargetIsaClass &&
        Variant.isMasked() == Masked)
      CandidateFunctions.push_back(std::make_pair(Variant, I));
  }

  int VariantIdx = -1;
  int BestScore = -1;
  // Keep track of parameter position containing the largest score. Can be
  // used as a tiebreaker when selecting the best variant.
  int BestArg = -1;
  VectorVariant::ISAClass BestIsa = VectorVariant::ISAClass::XMM;
  for (unsigned I = 0; I < CandidateFunctions.size(); ++I) {
    VectorVariant CandidateVariant = CandidateFunctions[I].first;
    int Score = 0;
    int MaxArg = 0;
    bool Match =
        matchAndScoreVariantParameters(VPCall, CandidateVariant, Score, MaxArg);
    if (Match) {
      LLVM_DEBUG(dbgs() << "Matched function: " << CandidateVariant.encode()
                        << "\n");
      LLVM_DEBUG(dbgs() << "Score: " << Score << "\n");
    }
    // Matched function will be the one with the best score. For tiebreaker
    // when scores match, pick highest ISA.
    VectorVariant::ISAClass VariantIsa = CandidateVariant.getISA();
    if (Match &&
        ((Score > BestScore) || (Score == BestScore && VariantIsa > BestIsa) ||
         (Score == BestScore && VariantIsa == BestIsa && MaxArg > BestArg))) {
      BestScore = Score;
      BestArg = MaxArg;
      BestIsa = VariantIsa;
      VariantIdx = I;
    }
  }

  if (VariantIdx >= 0) {
    LLVM_DEBUG(dbgs() << "\nMatched call to: "
                      << CandidateFunctions[VariantIdx].first.encode()
                      << "\n\n");
    return std::make_pair(
        std::make_unique<VectorVariant>(CandidateFunctions[VariantIdx].first),
        CandidateFunctions[VariantIdx].second);
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

  // 1. Ignored calls (do we need a new field in VecProperties for this?)
  if (isa<DbgInfoIntrinsic>(UnderlyingCI))
    return;

  Function *F = VPCall->getCalledFunction();

  // 2. Call was already marked to be strictly not widended (for example, kernel
  // convergent uniform calls).
  if (VPCall->getVectorizationScenario() ==
      VPCallInstruction::CallVecScenariosTy::DoNotWiden) {
    return;
  }

  // 3. Indirect calls will be serialized, as of today.
  if (!F) {
    VPCall->setShouldBeSerialized();
    return;
  }

  StringRef CalledFuncName = F->getName();
  // Currently we assume CallVecDecisions analysis is run after predication. So
  // call is masked only if its parent VPBB has predicate.
  VPBasicBlock *VPBB = VPCall->getParent();
  bool IsMasked = VPBB->getPredicate() != nullptr;
  // 4. Vectorizable library function like SVML calls. Set vector function
  // name in CallVecProperties. NOTE : Vector library calls can be used if call
  // is known to read memory only (non-default behavior).
  if (TLI->isFunctionVectorizable(CalledFuncName, VF, IsMasked) &&
      (VPlanVecNonReadonlyLibCalls || UnderlyingCI->onlyReadsMemory())) {
    VPCall->setVectorizeWithLibraryFn(
        TLI->getVectorizedFunction(CalledFuncName, VF, IsMasked));
    return;
  }

  // 5. Function calls with available vector variants.
  if (auto VecVariant = matchVectorVariant(VPCall, IsMasked, VF, TTI)) {
    VPCall->setVectorizeWithVectorVariant(VecVariant.getValue().first,
                                          VecVariant.getValue().second);
    return;
  }

  // 6. Use masked vector variant with all-zero mask for unmasked calls
  // without matching vector variant.
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

  // 7. Vectorize by pumping the call for a lower VF. Since pumping is only done
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

  // 8. Trivially vectorizable call using vector intrinsics.
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

  // 9.  Deterministic function calls with no side effects that operate on
  // uniform operands need to be marked as DoNotWiden.
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

  // 10. All other cases implies default properties i.e. call serialization.
  VPCall->setShouldBeSerialized();
  // TODO:
  // 1. OpenCLReadChannel/OpenCLWriteChannel calls?
}
