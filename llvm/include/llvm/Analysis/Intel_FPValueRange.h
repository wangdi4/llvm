//===- Intel_FPValueRange.h - Represent a floating-point range --*- C++ -*-===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines FPValueRange class to represent the range of floating-point
// numbers a value in IR might be at runtime.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_FP_VALUE_RANGE_H
#define LLVM_ANALYSIS_INTEL_FP_VALUE_RANGE_H

#include "llvm/ADT/APFloat.h"
#include "llvm/IR/Operator.h"

namespace llvm {
class FPValueRange {
  enum ValueTag {
    // Empty set, used as an initial value to merge other ranges into.
    Empty,
    // Undef, can be assumed to be any value
    Undef,
    // A constant value or range
    Constant,
    // An unknown dynamic value
    Unknown
  };
  ValueTag Tag;
  // When Tag == Constant:
  // * Lower == Upper indicates a constant value.
  // * Lower < Upper indicates an inclusive range. But when Lower is -Infinity
  // or
  // * Upper is +Infinity and MaybeInfinity is false, the range doesn't cover
  // the Infinity value.
  // * Lower > Upper is not allowed.
  // * If one of them is NaN, the other one must be NaN too
  // Although the value carried by these fields is not used when Tag !=
  // Constant, they still stores fltSemantics of this range, and they must have
  // the same fltSemantics.
  APFloat Lower, Upper;
  // These flags indicates whether this range may contains NaN or Infinity. A
  // true value indicates the value this range represents is possible to be
  // NaN/Infinity (even if there is no NaN/Infinity in Lower/Upper). A false
  // value indicates the value will never be NaN/Infinity.
  // Combining the Constant tag and MaybeNaN/MaybeInfinity allows constructing
  // value sets like "from X to Y, plus NaN". This is useful in some
  // cases, e.g. for c = fmod(a, b), we have abs(c) <= b and can derive a
  // constant range from it (given b is also a constant range). But if a may
  // be Infinity, c can be NaN.
  bool MaybeNaN, MaybeInfinity;

  // Construct an empty range
  FPValueRange(const fltSemantics &semantics)
      : FPValueRange(Empty, false, false, semantics) {}

  // Constuct an empty, unknown or undef range
  FPValueRange(ValueTag Tag, bool MaybeNaN, bool MaybeInfinity,
               const fltSemantics &semantics)
      : Tag(Tag), Lower(semantics), Upper(semantics), MaybeNaN(MaybeNaN),
        MaybeInfinity(MaybeInfinity) {
    assert((Tag == Empty || Tag == Undef || Tag == Unknown) &&
           "Invalid ValueTag");
    assert((!(Tag == Empty && (MaybeNaN || MaybeInfinity))) &&
           "Empty range can't have NaN or Infinity");
  }

  // Construct a range with a constant range
  FPValueRange(APFloat Lower, APFloat Upper, bool MaybeNaN, bool MaybeInfinity)
      : Tag(Constant), Lower(Lower), Upper(Upper), MaybeNaN(MaybeNaN),
        MaybeInfinity(MaybeInfinity) {
    assert(&Lower.getSemantics() == &Upper.getSemantics() &&
           "Lower and upper bounds of a range must have the same fltSemantics");
    assert((!(Lower.isNaN() || Upper.isNaN())) &&
           "NaN is not allowed in bound of a range");
    assert(Lower < Upper && "Invalid range");
  }

  // Constant a range with a constant value
  FPValueRange(APFloat Value, bool MaybeNaN, bool MaybeInfinity)
      : Tag(Constant), Lower(Value), Upper(Value),
        MaybeNaN(MaybeNaN),
        MaybeInfinity(MaybeInfinity) {
    assert((!Value.isNaN() || MaybeNaN) &&
           "MaybeNaN must be true if Value is NaN");
    assert((!Value.isInfinity() || MaybeInfinity) &&
           "MaybeInfinity must be true if Value is Infinity");
  }

public:
  bool isEmpty() const { return Tag == Empty; }
  bool isUndef() const { return Tag == Undef; }
  bool isUnknown() const { return Tag == Unknown; }
  bool isConstantValue() const {
    return Tag == Constant && ((Lower == Upper) || Lower.isNaN());
  }
  bool isConstantRange() const {
    return Tag == Constant && Lower != Upper && (!Lower.isNaN());
  }

  const APFloat &getConstantValue() const {
    assert(isConstantValue() && "Cannot get the constant of a non-constant!");
    assert((Lower == Upper || Lower.isNaN()) &&
           "Lower and Upper must be the same value for constant");
    return Lower;
  }

  const APFloat &getLower() const {
    assert(isConstantRange() &&
           "Cannot get the lower bound of a non-constant-range!");
    return Lower;
  }

  const APFloat &getUpper() const {
    assert(isConstantRange() &&
           "Cannot get the upper bound of a non-constant-range!");
    return Upper;
  }

  bool getMaybeNaN() const { return MaybeNaN; }
  bool getMaybeInfinity() const { return MaybeInfinity; }
  bool getMaybeZero() const {
    return isUnknown() || (isConstantValue() && getConstantValue().isZero()) ||
           (isConstantRange() &&
            (getLower().isZero() || getUpper().isZero() ||
             (getLower().isNegative() && (!getUpper().isNegative()))));
  }

  bool isNaN() const {
    return isConstantValue() && getConstantValue().isNaN() &&
           (!getMaybeInfinity());
  }
  bool isInfinity() const {
    return isConstantValue() && getConstantValue().isInfinity() &&
           (!getMaybeNaN());
  }
  bool isZero() const {
    return isConstantValue() && getConstantValue().isZero() &&
           (!getMaybeNaN()) && (!getMaybeInfinity());
  }

  const fltSemantics &getSemantics() const {
    assert(&Lower.getSemantics() == &Upper.getSemantics() &&
           "Inconsistent fltSemantics");
    return Lower.getSemantics();
  }

  FPValueRange setMaybeNaN(bool Value) const {
    if (!Value && isConstantValue() && getConstantValue().isNaN())
      return createEmptyOrSpecialConstant(false, getMaybeInfinity(),
                                          getSemantics());
    if (Value && isEmpty())
      return createEmptyOrSpecialConstant(true, false, getSemantics());
    FPValueRange Ret(*this);
    Ret.MaybeNaN = Value;
    return Ret;
  }

  FPValueRange setMaybeInfinity(bool Value) const {
    if (!Value && isConstantValue() && getConstantValue().isInfinity())
      return createEmptyOrSpecialConstant(getMaybeNaN(), false, getSemantics());
    if (Value && isEmpty())
      return createEmptyOrSpecialConstant(false, true, getSemantics());
    FPValueRange Ret(*this);
    Ret.MaybeInfinity = Value;
    return Ret;
  }

  // Return a new range with NaN or Infinity values disabled if nonan/noinf flag
  // is set in FMF.
  FPValueRange applyFastMathFlags(FastMathFlags FMF) const {
    FPValueRange Ret(*this);

    if (FMF.noNaNs() || FMF.noInfs())
      if (Ret.isConstantValue() && (Ret.getConstantValue().isNaN() ||
                                    Ret.getConstantValue().isInfinity())) {
        if (Ret.getMaybeNaN() && FMF.noNaNs())
          Ret = createEmptyOrSpecialConstant(false, Ret.getMaybeInfinity(),
                                             getSemantics());
        if (Ret.getMaybeInfinity() && FMF.noInfs())
          Ret = createEmptyOrSpecialConstant(Ret.getMaybeNaN(), false,
                                             getSemantics());
        if (Ret.isEmpty())
          Ret = createUndef(false, false, getSemantics());
      }

    if (FMF.noNaNs())
      Ret = Ret.setMaybeNaN(false);
    if (FMF.noInfs())
      Ret = Ret.setMaybeInfinity(false);
    return Ret;
  }

  FPValueRange applyFastMathFlags(const FPMathOperator *Op) const {
    return applyFastMathFlags(Op->getFastMathFlags());
  }

  // Returns true if the other range is a subset of this range.
  bool contains(const FPValueRange& Other) const;

  /// Returns true if this range is guaranteed to be in the given bit range,
  /// false if it's guaranteed not to be in the range. Returns None if the range
  /// can't be determined.
  std::optional<bool> isInBitRange(unsigned BitRange) const;

  bool operator==(const FPValueRange &Other) const {
    if (&getSemantics() != &Other.getSemantics())
      return false;

    // contains() may treat undef as anything, good for optimization but not
    // helpful for comparison, therefore an explicit test for undef is required.
    if (isUndef() || Other.isUndef())
      return (isUndef() && Other.isUndef()) &&
             (getMaybeNaN() == Other.getMaybeNaN()) &&
             (getMaybeInfinity() == Other.getMaybeInfinity());

    return Other.contains(*this) && this->contains(Other);
  }

  bool operator!=(const FPValueRange &Other) const {
    return !(*this == Other);
  }

  static FPValueRange createEmpty(const fltSemantics &semantics) {
    return FPValueRange(semantics);
  }

  // Create an FPValueRange representing an empty set or special FP constants.
  // Set MaybeNaN or MaybeInfinity to true for special constants. If both are
  // false, an empty set is returned.
  static FPValueRange
  createEmptyOrSpecialConstant(bool MaybeNaN, bool MaybeInfinity,
                               const fltSemantics &semantics) {
    if (MaybeNaN)
      return createConstant(APFloat::getNaN(semantics), true, MaybeInfinity);
    else if (MaybeInfinity)
      return createConstant(APFloat::getInf(semantics), false, true);

    return FPValueRange(Empty, false, false, semantics);
  }

  static FPValueRange createUndef(bool MaybeNaN, bool MaybeInfinity,
                                  const fltSemantics &semantics) {
    return FPValueRange(Undef, MaybeNaN, MaybeInfinity, semantics);
  }

  static FPValueRange createUnknown(bool MaybeNaN, bool MaybeInfinity,
                                    const fltSemantics &semantics) {
    return FPValueRange(Unknown, MaybeNaN, MaybeInfinity, semantics);
  }

  static FPValueRange createUnknown(FastMathFlags FMF,
                                    const fltSemantics &semantics) {
    return FPValueRange(Unknown, !FMF.noNaNs(), !FMF.noInfs(), semantics);
  }

  static FPValueRange createUnknown(const Value *V) {
    assert(V->getType()->isFloatingPointTy() &&
           "Creating an FPValueRange requires a floating-point instruction");
    if (auto FPOp = dyn_cast<FPMathOperator>(V))
      return createUnknown(FPOp->getFastMathFlags(),
                           V->getType()->getFltSemantics());
    return createUnknown(false, false, V->getType()->getFltSemantics());
  }

  static FPValueRange createConstant(const APFloat &ConstVal, bool MaybeNaN,
                                     bool MaybeInfinity) {
    return FPValueRange(ConstVal, MaybeNaN, MaybeInfinity);
  }

  // If Lower == Upper, returns a constant, otherwise returns a constant range.
  static FPValueRange createConstantOrConstantRange(const APFloat &Lower,
                                                    const APFloat &Upper,
                                                    bool MaybeNaN,
                                                    bool MaybeInfinity) {
    if (Lower == Upper || (Lower.isNaN() && Upper.isNaN()))
      return createConstant(Lower, MaybeNaN, MaybeInfinity);
    if (Lower.isInfinity() && Lower.isNegative() && Upper.isInfinity() &&
        !Upper.isNegative())
      return createUnknown(MaybeNaN, MaybeInfinity, Lower.getSemantics());
    return FPValueRange(Lower, Upper, MaybeNaN, MaybeInfinity);
  }

  // Returns a new range that is guaranteed to contain all potential values in
  // both LHS and RHS. Used for PHI instructions.
  static FPValueRange merge(const FPValueRange &LHS, const FPValueRange &RHS);

  // Returns a new range that is guaranteed to contain the product of all
  // value combinations in LHS and RHS. If NoInfs is true, we'll assume that
  // none of the operands and product is Infinity.
  static FPValueRange multiply(const FPValueRange &LHS, const FPValueRange &RHS);

  // Returns a new range that is guaranteed to contain the floating-point
  // remainder of all value combinations in LHS and RHS.
  static FPValueRange mod(const FPValueRange& LHS, const FPValueRange& RHS);
};

} // namespace llvm
#endif // LLVM_ANALYSIS_INTEL_FP_VALUE_RANGE_H
