//===------ Intel_FPValueRange.cpp - Represent a floating point range -----===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the FPValueRange class.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_FPValueRange.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/APSInt.h"
#include "llvm/IR/ConstantRange.h"
#include "llvm/Support/Debug.h"
#include <numeric>

using namespace llvm;

// Return the largest APFloat in [begin, end). Ignore all NaNs. If all elements
// are NaN, returns NaN.
template <typename IterT> static APFloat minnumRange(IterT begin, IterT end) {
  assert(begin != end);
  return std::accumulate(begin, end, *begin, minnum);
}

// Return the smallest APFloat in [begin, end). Ignore all NaNs. If all elements
// are NaN, returns NaN.
template <typename IterT> static APFloat maxnumRange(IterT begin, IterT end) {
  assert(begin != end);
  return std::accumulate(begin, end, *begin, maxnum);
}

bool FPValueRange::contains(const FPValueRange& Other) const {
  // Handle NaN and Infinity separately so we don't have to deal with them
  // later.
  if (!getMaybeNaN() && Other.getMaybeNaN())
    return false;
  if (!getMaybeInfinity() && Other.getMaybeInfinity())
    return false;
  FPValueRange ThisWithoutNaNAndInfinity =
      setMaybeNaN(false).setMaybeInfinity(false);
  FPValueRange OtherWithoutNaNAndInfinity =
      Other.setMaybeNaN(false).setMaybeInfinity(false);

  if (OtherWithoutNaNAndInfinity.isEmpty())
    return true;
  if (ThisWithoutNaNAndInfinity.isEmpty())
    return false;

  if (ThisWithoutNaNAndInfinity.isUnknown())
    return true;

  if (ThisWithoutNaNAndInfinity.isUndef() ||
      OtherWithoutNaNAndInfinity.isUndef())
    return true;

  if (ThisWithoutNaNAndInfinity.isConstantValue() &&
      OtherWithoutNaNAndInfinity.isConstantValue() &&
      ThisWithoutNaNAndInfinity.getConstantValue() ==
          OtherWithoutNaNAndInfinity.getConstantValue())
    return true;

  if (ThisWithoutNaNAndInfinity.isConstantRange()) {
    if (OtherWithoutNaNAndInfinity.isConstantValue()) {
      APFloat OtherValue = OtherWithoutNaNAndInfinity.getConstantValue();
      return OtherValue >= ThisWithoutNaNAndInfinity.getLower() &&
             OtherValue <= ThisWithoutNaNAndInfinity.getUpper();
    }
    if (OtherWithoutNaNAndInfinity.isConstantRange())
      return OtherWithoutNaNAndInfinity.getLower() >=
                 ThisWithoutNaNAndInfinity.getLower() &&
             OtherWithoutNaNAndInfinity.getUpper() <=
                 ThisWithoutNaNAndInfinity.getUpper();
  }

  return false;
}

FPValueRange FPValueRange::merge(const FPValueRange &LHS,
                                 const FPValueRange &RHS) {
  // It's possible for the merged value to be NaN/Infinity, if one of the
  // incoming ranges covers this values.
  bool MayReturnNaN = LHS.getMaybeNaN() || RHS.getMaybeNaN();
  bool MayReturnInfinity = LHS.getMaybeInfinity() || RHS.getMaybeInfinity();

  // Pick the non-empty one if one of them is empty.
  if (LHS.isEmpty() || RHS.isEmpty())
    return LHS.isEmpty() ? RHS : LHS;

  // Merging unknown range with any other range is still unknown.
  if (LHS.isUnknown() || RHS.isUnknown())
    return createUnknown(MayReturnNaN, MayReturnInfinity, LHS.getSemantics());

  // Undef is handled similarly with Empty
  if (LHS.isUndef() || RHS.isUndef())
    return (LHS.isUndef() ? RHS : LHS)
        .setMaybeNaN(MayReturnNaN)
        .setMaybeInfinity(MayReturnInfinity);

  // If both of the input ranges are constant, put the bounds from the inputs
  // together into a list, find the min and max value to create a new range.
  // minnum/maxnum is used in case there is a ConstantValue range with NaN as
  // value.
  if (LHS.isConstantValue() && RHS.isConstantValue()) {
    std::initializer_list<APFloat> Values = {LHS.getConstantValue(),
                                             RHS.getConstantValue()};
    return createConstantOrConstantRange(
        minnumRange(Values.begin(), Values.end()),
        maxnumRange(Values.begin(), Values.end()), MayReturnNaN,
        MayReturnInfinity);
  }

  if (LHS.isConstantValue() && RHS.isConstantRange()) {
    std::initializer_list<APFloat> Values = {LHS.getConstantValue(),
                                             RHS.getLower(), RHS.getUpper()};
    return createConstantOrConstantRange(
        minnumRange(Values.begin(), Values.end()),
        maxnumRange(Values.begin(), Values.end()), MayReturnNaN,
        MayReturnInfinity);
  }

  if (RHS.isConstantValue() && LHS.isConstantRange())
    return merge(RHS, LHS);

  if (LHS.isConstantRange() && RHS.isConstantRange()) {
    std::initializer_list<APFloat> Values = {LHS.getLower(), LHS.getUpper(),
                                             RHS.getLower(), RHS.getUpper()};
    return createConstantOrConstantRange(
        minnumRange(Values.begin(), Values.end()),
        maxnumRange(Values.begin(), Values.end()), MayReturnNaN,
        MayReturnInfinity);
  }

  llvm_unreachable("Missing branch in merge");
}

FPValueRange FPValueRange::multiply(const FPValueRange &LHS,
                                    const FPValueRange &RHS) {
  const fltSemantics &semantics = LHS.getSemantics();
  if (LHS.isEmpty() || RHS.isEmpty())
    return createEmpty(semantics);

  // For any x, x * NaN = NaN
  bool MayReturnNaN = LHS.getMaybeNaN() || RHS.getMaybeNaN();
  // 0 * Infinity = NaN
  MayReturnNaN |= (LHS.getMaybeInfinity() && RHS.getMaybeZero()) ||
                  (RHS.getMaybeInfinity() && LHS.getMaybeZero());

  // Infinity * x = Infinity, if x is not 0 or NaN
  FPValueRange LHSWithoutNaN = LHS.setMaybeNaN(false);
  FPValueRange RHSWithoutNaN = RHS.setMaybeNaN(false);
  bool MayReturnInfinity =
      (!LHSWithoutNaN.isZero() && !RHSWithoutNaN.isZero()) &&
      (LHSWithoutNaN.getMaybeInfinity() || LHSWithoutNaN.getMaybeInfinity()) &&
      (!LHSWithoutNaN.isEmpty() && !RHSWithoutNaN.isEmpty());

  if (LHS.isUndef() || RHS.isUndef())
    return createUndef(MayReturnNaN, MayReturnInfinity, semantics);

  if (LHS.isUnknown() || RHS.isUnknown())
    return createUnknown(MayReturnNaN, false, semantics);

  // If both of the input ranges are constant, calculate the products between
  // lower and upper bounds of the inputs, find the min and max value to
  // create a new range.
  if (LHS.isConstantValue() && RHS.isConstantValue()) {
    APFloat Value = LHS.getConstantValue() * RHS.getConstantValue();
    return createConstant(Value, Value.isNaN() || MayReturnNaN,
                          Value.isInfinity());
  }

  if (LHS.isConstantValue() && RHS.isConstantRange()) {
    std::initializer_list<APFloat> Values = {
        LHS.getConstantValue() * RHS.getLower(),
        LHS.getConstantValue() * RHS.getUpper()};
    APFloat Min = minnumRange(Values.begin(), Values.end()),
            Max = maxnumRange(Values.begin(), Values.end());
    return createConstantOrConstantRange(Min, Max, MayReturnNaN,
                                         Min.isInfinity() || Max.isInfinity());
  } else if (RHS.isConstantValue() && LHS.isConstantRange()) {
    return multiply(RHS, LHS);
  }

  if (LHS.isConstantRange() && RHS.isConstantRange()) {
    std::initializer_list<APFloat> Values = {
        LHS.getLower() * RHS.getLower(), LHS.getLower() * RHS.getUpper(),
        LHS.getUpper() * RHS.getLower(), LHS.getUpper() * RHS.getUpper()};
    APFloat Min = minnumRange(Values.begin(), Values.end()),
            Max = maxnumRange(Values.begin(), Values.end());
    return createConstantOrConstantRange(Min, Max, MayReturnNaN,
                                         Min.isInfinity() || Max.isInfinity());
  }

  llvm_unreachable("Missing branch in multiply");
}

FPValueRange FPValueRange::mod(const FPValueRange &LHS,
                               const FPValueRange &RHS) {
  if (LHS.isEmpty() || RHS.isEmpty())
    return createEmpty(LHS.getSemantics());

  // For any x, fmod(Infinity, x) = NaN
  bool MayReturnNaN = LHS.getMaybeInfinity();
  // If either of x and y is NaN, fmod(x, y) = NaN
  MayReturnNaN |= LHS.getMaybeNaN() || RHS.getMaybeNaN();
  // For any x, fmod(x, 0) = NaN
  MayReturnNaN |= RHS.getMaybeZero();

  if (RHS.isUnknown())
    return createUnknown(MayReturnNaN, false, LHS.getSemantics());

  if (RHS.isUndef())
    return createUndef(MayReturnNaN, false, LHS.getSemantics());

  // If the divisor can be proved to be in a constant range [a, b], the range of
  // the remainder is just [-max(abs(a), abs(b)), max(abs(a), abs(b)].
  // TODO: Only divisor's range (RHS) is examined now, we can also consider the
  // dividend's range (LHS).
  if (RHS.isConstantValue()) {
    APFloat AbsValue = abs(RHS.getConstantValue());
    return createConstantOrConstantRange(-AbsValue, AbsValue, MayReturnNaN,
                                         false);
  }

  if (RHS.isConstantRange()) {
    APFloat LowerAbsValue = abs(RHS.getLower());
    APFloat UpperAbsValue = abs(RHS.getUpper());
    APFloat MaxAbsValue = maxnum(LowerAbsValue, UpperAbsValue);
    return createConstantOrConstantRange(-MaxAbsValue, MaxAbsValue,
                                         MayReturnNaN, false);
  }

  llvm_unreachable("Missing branch in mod");
}

std::optional<bool> FPValueRange::isInBitRange(unsigned BitRange) const {
  ConstantRange IntRange = ConstantRange::getFull(BitRange);

  if (isUndef() || isEmpty())
    return true;

  if (isUnknown())
    return std::nullopt;

  if (isConstantValue()) {
    APFloat C = getConstantValue();
    APSInt CFloor(BitRange, false);
    APSInt CCeil(BitRange, false);
    bool IsExact;
    // Overflow indicates the bit range given is too small
    if (C.convertToInteger(CFloor, RoundingMode::TowardNegative, &IsExact) &
        APFloat::opInvalidOp)
      return false;
    if (C.convertToInteger(CCeil, RoundingMode::TowardPositive, &IsExact) &
        APFloat::opInvalidOp)
      return false;
    if (C.isInteger()) {
      if (CCeil.isMaxValue())
        return false;
      else
        ++CCeil;
    }
    return IntRange.contains(ConstantRange(CFloor, CCeil));
  }

  if (isConstantRange()) {
    APSInt Lower(BitRange, false);
    APSInt Upper(BitRange, false);
    bool IsExact;
    if (getLower().convertToInteger(Lower, RoundingMode::TowardNegative,
                                    &IsExact) &
        APFloat::opInvalidOp)
      return false;
    if (getUpper().convertToInteger(Upper, RoundingMode::TowardPositive,
                                    &IsExact) &
        APFloat::opInvalidOp)
      return false;
    if (getUpper().isInteger()) {
      if (Upper.isMaxValue())
        return false;
      else
        ++Upper;
    }
    return IntRange.contains(ConstantRange(Lower, Upper));
  }

  llvm_unreachable("Missing branch in isInBitRange");
}
