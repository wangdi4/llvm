//===- IntelVPlanValueTracking.cpp ------------------------------*- C++ -*-===//
//
//   Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanValueTracking.h"

#include "IntelVPlan.h"
#include "IntelVPlanScalarEvolution.h"

#include <llvm/Analysis/ScalarEvolutionExpressions.h>
#include <llvm/Analysis/ValueTracking.h>

#define DEBUG_TYPE "vplan-value-tracking"

using namespace llvm;
using namespace llvm::vpo;

const Instruction *
VPlanValueTrackingLLVM::tryToGetUnderlyingInst(const VPInstruction *VPInst) {
  // FIXME: CtxI == NULL should probably mean the entry to the main loop.
  return (VPInst && VPInst->isUnderlyingIRValid())
             ? cast<Instruction>(VPInst->getUnderlyingValue())
             : nullptr;
}

KnownBits VPlanValueTrackingLLVM::getKnownBits(VPlanSCEV *Expr,
                                               const VPInstruction *CtxI) {
  return getKnownBitsImpl(VPSE->toSCEV(Expr), tryToGetUnderlyingInst(CtxI));
}

KnownBits VPlanValueTrackingLLVM::getKnownBits(const VPValue *Val,
                                               const VPInstruction *CtxI) {
  unsigned BitWidth = DL->getTypeSizeInBits(Val->getType());
  Value *Underlying = Val->getUnderlyingValue();
  if (!Underlying) {
    return KnownBits(BitWidth);
  }
  return getKnownBitsImpl(Underlying, tryToGetUnderlyingInst(CtxI));
}

KnownBits VPlanValueTrackingLLVM::getKnownBitsImpl(const SCEV *Scev,
                                                   const Instruction *CtxI) {
  auto BitWidth = DL->getTypeSizeInBits(Scev->getType());

  if (auto *ScevConst = dyn_cast<SCEVConstant>(Scev)) {
    KnownBits KB(BitWidth);
    KB.One = ScevConst->getAPInt();
    KB.Zero = ~KB.One;
    return KB;
  }

  if (auto *ScevUnknown = dyn_cast<SCEVUnknown>(Scev)) {
    Value *V = ScevUnknown->getValue();
    return llvm::computeKnownBits(V, *DL, 0, VPAC->getLLVMCache(), CtxI, DT);
  }

  if (auto *ScevAdd = dyn_cast<SCEVAddExpr>(Scev)) {
    bool NSW = ScevAdd->hasNoSignedWrap();
    KnownBits KB(BitWidth);
    KB.setAllZero();

    for (auto *Op : ScevAdd->operands()) {
      KnownBits OpKB = getKnownBitsImpl(Op, CtxI);
      KB = KnownBits::computeForAddSub(true, NSW, KB, OpKB);
    }

    return KB;
  }

  if (auto *ScevMul = dyn_cast<SCEVMulExpr>(Scev)) {
    // There's no public KnownBits API to compute known bits for the result of
    // multiplication. So, the only thing we do ourselves here is just computing
    // the number of low zero bits.
    int NZero = 0;
    for (auto *Op : ScevMul->operands()) {
      KnownBits OpKB = getKnownBitsImpl(Op, CtxI);
      NZero += OpKB.Zero.countTrailingOnes();
    }

    KnownBits KB(BitWidth);
    KB.Zero.setBits(0, NZero);
    return KB;
  }

  if (auto *ScevPtrToInt = dyn_cast<SCEVPtrToIntExpr>(Scev)) {
    // SCEVPtrToIntExpr should not change the known bits number.
    return getKnownBitsImpl(ScevPtrToInt->getOperand(), CtxI);
  }

  return KnownBits(BitWidth);
}

KnownBits VPlanValueTrackingLLVM::getKnownBitsImpl(const Value *Val,
                                                   const Instruction *CtxI) {
  return llvm::computeKnownBits(Val, *DL, 0, VPAC->getLLVMCache(), CtxI, DT);
}
