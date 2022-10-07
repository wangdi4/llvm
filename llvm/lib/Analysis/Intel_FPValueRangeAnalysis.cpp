//===--------------- Intel_FPValueRangeAnalysis.cpp -------------*- C++ -*-===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the FPValueRangeAnalysis class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_FPValueRangeAnalysis.h"
#include "llvm/Analysis/LazyValueInfo.h"
#include "llvm/IR/ConstantRange.h"
#include "llvm/IR/PatternMatch.h"

using namespace llvm;

FPValueRange FPValueRangeAnalysis::getOrInsertRange(Value *V) {
  auto I = ValueRangeMap.find(V);
  if (I != ValueRangeMap.end())
    return I->second;

  auto Insert = ValueRangeMap.insert(std::make_pair(
      V, FPValueRange::createEmpty(V->getType()->getFltSemantics())));
  (void)Insert; // INTEL
  assert(Insert.second);
  WidenCounter.insert(std::make_pair(V, 0));
  FPValueRange Ret = processEntry(V);

  auto NewI = ValueRangeMap.find(V);
  NewI->second = Ret;
  return Ret;
}

FPValueRange FPValueRangeAnalysis::widenRanges(Value *V, FPValueRange OldRange,
                                               FPValueRange NewRange) {
  FPValueRange Ret = FPValueRange::merge(OldRange, NewRange);

  if (OldRange.isConstantRange() && NewRange.isConstantRange()) {
    APFloat Lower = std::min(OldRange.getLower(), NewRange.getLower());
    APFloat Upper = std::max(OldRange.getUpper(), NewRange.getUpper());

    bool Extended = false;
    if (OldRange.getLower() > NewRange.getLower()) {
      Lower = APFloat::getInf(Lower.getSemantics(), /*IsNegative=*/true);
      Extended = true;
    }
    if (OldRange.getUpper() < NewRange.getUpper()) {
      Upper = APFloat::getInf(Lower.getSemantics(), /*IsNegative=*/false);
      Extended = true;
    }
    WidenCounter[V] += Extended;

    if (WidenCounter[V] > 3)
      Ret =
          FPValueRange::merge(Ret, FPValueRange::createConstantOrConstantRange(
                                       Lower, Upper, /*MaybeNaN=*/false,
                                       /*MaybeInfinity=*/true));
  }

  return Ret;
}

FPValueRange FPValueRangeAnalysis::processEntry(Value *V) {
  assert(V->getType()->isFloatingPointTy() &&
         "processEntry expects an FP value");

  const APFloat *C;
  if (match(V, PatternMatch::m_APFloat(C)))
    return FPValueRange::createConstant(*C, C->isNaN(), C->isInfinity());

  const fltSemantics &Semantics = V->getType()->getFltSemantics();

  if (isa<UndefValue>(V))
    return FPValueRange::createUndef(/*MaybeNaN=*/false,
                                     /*MaybeInfinity=*/false,
                                     Semantics);

  if (const auto *Op = dyn_cast<FPMathOperator>(V)) {
    FastMathFlags FMF = Op->getFastMathFlags();
    switch (Op->getOpcode()) {
    case Instruction::FMul: {
      Value *LHS = Op->getOperand(0);
      Value *RHS = Op->getOperand(1);
      FPValueRange LHSResult = getOrInsertRange(LHS).applyFastMathFlags(FMF);
      FPValueRange RHSResult = getOrInsertRange(RHS).applyFastMathFlags(FMF);

      return FPValueRange::multiply(LHSResult, RHSResult)
          .applyFastMathFlags(Op);
    }
    case Instruction::FRem: {
      Value *LHS = Op->getOperand(0);
      Value *RHS = Op->getOperand(1);
      FPValueRange LHSResult = getOrInsertRange(LHS).applyFastMathFlags(FMF);
      FPValueRange RHSResult = getOrInsertRange(RHS).applyFastMathFlags(FMF);

      return FPValueRange::mod(LHSResult, RHSResult).applyFastMathFlags(Op);
    }
    case Instruction::PHI: {
      const auto *P = cast<PHINode>(Op);
      auto Result = FPValueRange::createEmpty(Semantics);
      for (Value *V : P->incoming_values())
        Result = FPValueRange::merge(
            Result, getOrInsertRange(V).applyFastMathFlags(FMF));
      return Result.applyFastMathFlags(Op);
    }
    default:
      return FPValueRange::createUnknown(V);
    }
  } else if (isa<SIToFPInst>(V) || isa<UIToFPInst>(V)) {
    CastInst *CI = cast<CastInst>(V);
    unsigned BitWidth = CI->getSrcTy()->getIntegerBitWidth();
    ConstantRange IntRange = ConstantRange::getFull(BitWidth);

    // Try LazyValueInfo for sitofp/uitofp
    if (LVI)
      IntRange = LVI->getConstantRange(CI->getOperand(0), CI);

    APFloat Lower(Semantics,0), Upper(Semantics,0);
    bool Signed = isa<SIToFPInst>(V);
    Lower.convertFromAPInt(Signed ? IntRange.getSignedMin()
                                  : IntRange.getUnsignedMin(),
                           Signed, APFloat::rmNearestTiesToEven);
    Upper.convertFromAPInt(Signed ? IntRange.getSignedMax()
                                  : IntRange.getUnsignedMax(),
                           Signed, APFloat::rmNearestTiesToEven);
    return FPValueRange::createConstantOrConstantRange(Lower, Upper,
                                                       /*MaybeNaN=*/false,
                                                       /*MaybeInfinity=*/false);
  }

  return FPValueRange::createUnknown(V);
}

FPValueRange FPValueRangeAnalysis::computeRange(Value *V) {
  getOrInsertRange(V);

  bool Changed = true;
  for (unsigned I = 0; Changed && I < MaxIteration; ++I) {
    Changed = false;
    for (std::pair<Value *, FPValueRange> &Entry : ValueRangeMap) {
      FPValueRange OldRange = Entry.second;
      FPValueRange NewRange = processEntry(Entry.first);

      if (NewRange != OldRange) {
        // Use widening for PHIs
        NewRange = isa<PHINode>(Entry.first)
                       ? widenRanges(Entry.first, OldRange, NewRange)
                       : FPValueRange::merge(OldRange, NewRange);
        if (NewRange != OldRange) {
          Entry.second = NewRange;
          Changed = true;
        }
      }
    }
  }

  // Return unknown range if the iteration didn't converge
  if (Changed)
    return FPValueRange::createUnknown(V);
  else
    return getOrInsertRange(V);
}
