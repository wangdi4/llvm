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

void VPlanCallVecDecisions::run(unsigned VF, const TargetLibraryInfo *TLI,
                                const TargetTransformInfo *TTI) {
  // Analysis would be trivial for VF=1 since all calls should just be scalar.
  if (VF == 1)
    return;

  LLVM_DEBUG(dbgs() << "Running CallVecDecisions for VF=" << VF << "\n");
  for (VPBasicBlock &VPBB : Plan) {
    for (VPInstruction &Inst : VPBB) {
      if (auto *VPCall = dyn_cast<VPCallInstruction>(&Inst))
        analyzeCall(VPCall, VF, TLI, TTI);
    }
  }
}

void VPlanCallVecDecisions::analyzeCall(VPCallInstruction *VPCall, unsigned VF,
                                        const TargetLibraryInfo *TLI,
                                        const TargetTransformInfo *TTI) {
  const CallInst *UnderlyingCI = VPCall->getUnderlyingCallInst();

  // 1. Ignored calls (do we need a new field in VecProperties for this?)
  if (isa<DbgInfoIntrinsic>(UnderlyingCI))
    return;

  Function *F = VPCall->getCalledFunction();

  // Reset decisions that were taken for any previous VF as they will be
  // overwritten for currently analyzed VF.
  VPCall->resetVecScenario(VF);

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
  // name in CallVecProperties.
  if (TLI->isFunctionVectorizable(CalledFuncName, VF, IsMasked)) {
    VPCall->setVectorizeWithLibraryFn(
        TLI->getVectorizedFunction(CalledFuncName, VF, IsMasked));
    return;
  }

  // 5. Function calls with available vector variants.
  if (auto VecVariant = matchVectorVariant(UnderlyingCI, IsMasked, VF, TTI)) {
    VPCall->setVectorizeWithVectorVariant(VecVariant);
    return;
  }

  // 6. Use masked vector variant with all-zero mask for unmasked calls
  // without matching vector variant.
  // TODO: Same optimization can be done for calls with vectorizable library
  // function.
  if (!IsMasked) {
    auto MaskedVecVariant = matchVectorVariant(UnderlyingCI, true, VF, TTI);
    if (MaskedVecVariant) {
      VPCall->setVectorizeWithVectorVariant(MaskedVecVariant,
                                            true /*UseMaskedForUnmasked*/);
      return;
    }
  }

  // 7. Vectorize by pumping the call for a lower VF.
  unsigned PumpFactor = getPumpFactor(CalledFuncName, IsMasked, VF, TLI);
  if (PumpFactor > 1) {
    unsigned LowerVF = VF / PumpFactor;
    assert(TLI->isFunctionVectorizable(CalledFuncName, LowerVF, IsMasked) &&
           "Library function cannot be vectorized with lower VF.");
    VPCall->setVectorizeWithLibraryFn(
        TLI->getVectorizedFunction(CalledFuncName, LowerVF, IsMasked),
        PumpFactor);
    return;
  }

  // 8. All other cases implies default properties i.e. call serialization.
  // TODO: Deterministic function calls with no side effects that operate on
  // uniform operands need to be marked as DoNotWiden.
  VPCall->setShouldBeSerialized();
  // TODO:
  // 1. OpenCLReadChannel/OpenCLWriteChannel calls?
}
