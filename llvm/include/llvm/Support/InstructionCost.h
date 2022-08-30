//===- InstructionCost.h ----------------------------------------*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
/// This file defines an InstructionCost class that is used when calculating
/// the cost of an instruction, or a group of instructions. In addition to a
/// numeric value representing the cost the class also contains a state that
/// can be used to encode particular properties, such as a cost being invalid.
/// Operations on InstructionCost implement saturation arithmetic, so that
/// accumulating costs on large cost-values don't overflow.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_INSTRUCTIONCOST_H
#define LLVM_SUPPORT_INSTRUCTIONCOST_H

#include "llvm/ADT/APFixedPoint.h" // Intel
#include "llvm/ADT/APFloat.h"      // Intel
#include "llvm/ADT/Optional.h"
#include "llvm/Support/MathExtras.h"
#include <limits>

#if INTEL_CUSTOMIZATION
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
#include <sstream>
#include <iomanip>
#endif // !NDEBUG || LLVM_ENABLE_DUMP
#endif // INTEL_CUSTOMIZATION

namespace llvm {

class raw_ostream;

class InstructionCost {
public:
  using CostType = int64_t;

  /// CostState describes the state of a cost.
  enum CostState {
    Valid,  /// < The cost value represents a valid cost, even when the
            /// cost-value is large.
    Invalid /// < Invalid indicates there is no way to represent the cost as a
            /// numeric value. This state exists to represent a possible issue,
            /// e.g. if the cost-model knows the operation cannot be expanded
            /// into a valid code-sequence by the code-generator.  While some
            /// passes may assert that the calculated cost must be valid, it is
            /// up to individual passes how to interpret an Invalid cost. For
            /// example, a transformation pass could choose not to perform a
            /// transformation if the resulting cost would end up Invalid.
            /// Because some passes may assert a cost is Valid, it is not
            /// recommended to use Invalid costs to model 'Unknown'.
            /// Note that Invalid is semantically different from a (very) high,
            /// but valid cost, which intentionally indicates no issue, but
            /// rather a strong preference not to select a certain operation.
  };

private:
  CostType Value = 0;
  CostState State = Valid;

  void propagateState(const InstructionCost &RHS) {
    if (RHS.State == Invalid)
      State = Invalid;
  }

  static CostType getMaxValue() { return std::numeric_limits<CostType>::max(); }
  static CostType getMinValue() { return std::numeric_limits<CostType>::min(); }

public:
  // A default constructed InstructionCost is a valid zero cost
  InstructionCost() = default;

  InstructionCost(CostState) = delete;
  InstructionCost(CostType Val) : Value(Val), State(Valid) {}

  static InstructionCost getMax() { return getMaxValue(); }
  static InstructionCost getMin() { return getMinValue(); }
  static InstructionCost getInvalid(CostType Val = 0) {
    InstructionCost Tmp(Val);
    Tmp.setInvalid();
    return Tmp;
  }

  bool isValid() const { return State == Valid; }
  void setValid() { State = Valid; }
  void setInvalid() { State = Invalid; }
  CostState getState() const { return State; }

  /// This function is intended to be used as sparingly as possible, since the
  /// class provides the full range of operator support required for arithmetic
  /// and comparisons.
  Optional<CostType> getValue() const {
    if (isValid())
      return Value;
    return None;
  }

  /// For all of the arithmetic operators provided here any invalid state is
  /// perpetuated and cannot be removed. Once a cost becomes invalid it stays
  /// invalid, and it also inherits any invalid state from the RHS.
  /// Arithmetic work on the actual values is implemented with saturation,
  /// to avoid overflow when using more extreme cost values.

  InstructionCost &operator+=(const InstructionCost &RHS) {
    propagateState(RHS);

    // Saturating addition.
    InstructionCost::CostType Result;
    if (AddOverflow(Value, RHS.Value, Result))
      Result = RHS.Value > 0 ? getMaxValue() : getMinValue();

    Value = Result;
    return *this;
  }

  InstructionCost &operator+=(const CostType RHS) {
    InstructionCost RHS2(RHS);
    *this += RHS2;
    return *this;
  }

  InstructionCost &operator-=(const InstructionCost &RHS) {
    propagateState(RHS);

    // Saturating subtract.
    InstructionCost::CostType Result;
    if (SubOverflow(Value, RHS.Value, Result))
      Result = RHS.Value > 0 ? getMinValue() : getMaxValue();
    Value = Result;
    return *this;
  }

  InstructionCost &operator-=(const CostType RHS) {
    InstructionCost RHS2(RHS);
    *this -= RHS2;
    return *this;
  }

  InstructionCost &operator*=(const InstructionCost &RHS) {
    propagateState(RHS);

    // Saturating multiply.
    InstructionCost::CostType Result;
    if (MulOverflow(Value, RHS.Value, Result)) {
      if ((Value > 0 && RHS.Value > 0) || (Value < 0 && RHS.Value < 0))
        Result = getMaxValue();
      else
        Result = getMinValue();
    }

    Value = Result;
    return *this;
  }

  InstructionCost &operator*=(const CostType RHS) {
    InstructionCost RHS2(RHS);
    *this *= RHS2;
    return *this;
  }

  InstructionCost &operator/=(const InstructionCost &RHS) {
    propagateState(RHS);
    Value /= RHS.Value;
    return *this;
  }

  InstructionCost &operator/=(const CostType RHS) {
    InstructionCost RHS2(RHS);
    *this /= RHS2;
    return *this;
  }

  InstructionCost &operator++() {
    *this += 1;
    return *this;
  }

  InstructionCost operator++(int) {
    InstructionCost Copy = *this;
    ++*this;
    return Copy;
  }

  InstructionCost &operator--() {
    *this -= 1;
    return *this;
  }

  InstructionCost operator--(int) {
    InstructionCost Copy = *this;
    --*this;
    return Copy;
  }

  /// For the comparison operators we have chosen to use lexicographical
  /// ordering where valid costs are always considered to be less than invalid
  /// costs. This avoids having to add asserts to the comparison operators that
  /// the states are valid and users can test for validity of the cost
  /// explicitly.
  bool operator<(const InstructionCost &RHS) const {
    if (State != RHS.State)
      return State < RHS.State;
    return Value < RHS.Value;
  }

  // Implement in terms of operator< to ensure that the two comparisons stay in
  // sync
  bool operator==(const InstructionCost &RHS) const {
    return !(*this < RHS) && !(RHS < *this);
  }

  bool operator!=(const InstructionCost &RHS) const { return !(*this == RHS); }

  bool operator==(const CostType RHS) const {
    InstructionCost RHS2(RHS);
    return *this == RHS2;
  }

  bool operator!=(const CostType RHS) const { return !(*this == RHS); }

  bool operator>(const InstructionCost &RHS) const { return RHS < *this; }

  bool operator<=(const InstructionCost &RHS) const { return !(RHS < *this); }

  bool operator>=(const InstructionCost &RHS) const { return !(*this < RHS); }

  bool operator<(const CostType RHS) const {
    InstructionCost RHS2(RHS);
    return *this < RHS2;
  }

  bool operator>(const CostType RHS) const {
    InstructionCost RHS2(RHS);
    return *this > RHS2;
  }

  bool operator<=(const CostType RHS) const {
    InstructionCost RHS2(RHS);
    return *this <= RHS2;
  }

  bool operator>=(const CostType RHS) const {
    InstructionCost RHS2(RHS);
    return *this >= RHS2;
  }

  void print(raw_ostream &OS) const;

  template <class Function>
  auto map(const Function &F) const -> InstructionCost {
    if (isValid())
      return F(Value);
    return getInvalid();
  }
};

inline InstructionCost operator+(const InstructionCost &LHS,
                                 const InstructionCost &RHS) {
  InstructionCost LHS2(LHS);
  LHS2 += RHS;
  return LHS2;
}

inline InstructionCost operator-(const InstructionCost &LHS,
                                 const InstructionCost &RHS) {
  InstructionCost LHS2(LHS);
  LHS2 -= RHS;
  return LHS2;
}

inline InstructionCost operator*(const InstructionCost &LHS,
                                 const InstructionCost &RHS) {
  InstructionCost LHS2(LHS);
  LHS2 *= RHS;
  return LHS2;
}

inline InstructionCost operator/(const InstructionCost &LHS,
                                 const InstructionCost &RHS) {
  InstructionCost LHS2(LHS);
  LHS2 /= RHS;
  return LHS2;
}

inline raw_ostream &operator<<(raw_ostream &OS, const InstructionCost &V) {
  V.print(OS);
  return OS;
}

#if INTEL_CUSTOMIZATION
namespace vpo {
  // It's not clear how exactly UnknownCost/InifiniteCost will be distinguished
  // in the llvm::InstructionCost. Original RFC stated that it would be handled
  // somehow, but it doesn't seem to be the case yet.
  //
  // While that is unclear, we use VPInstructionCost type with intention to
  // merge its implemention into llvm::InstructionCost eventually.

class VPInstructionCost {
public:
  using CostType = APFixedPoint;

  /// CostState describes the state of a cost.
  enum CostState {
    Valid,    // The cost represents a valid cost.
    Unknown,  // The cost is unknown: the instruction(s) is(are) not modelled.
    Invalid   // The cost is invalid: overflow (or another irreversible
              // condition) occurred.
              //
              // Both Unknown and Invalid costs are poisonous and result Unknown
              // or Invalid cost in arithmetic operations. Some relationship
              // operations such as == and != are allowed for them but other
              // relation operations asserts that input values are Valid.
  };

private:
  CostType Value;
  CostState State;

  // Helper method to set the Value from int64_t input Val. If Val doesn't fit
  // the current Fixed Point Semantics it results Invalid cost.
  void setFromInt64Value(int64_t Val) {
    bool Overflow = false;
    APSInt APSIntValue = APSInt::get(Val);
    Value = CostType::getFromIntValue(
      APSIntValue, getVPInstructionCostSema(), &Overflow);
    if (Overflow)
      setInvalid();
  }

  // Helper method to set the Value from float input Val. If Val doesn't fit
  // the current Fixed Point Semantics it results Invalid cost.
  void setFromFloatValue(float Val) {
    bool Overflow = false;
    APFloat APFloatValue(Val);
    Value = CostType::getFromFloatValue(
      APFloatValue, getVPInstructionCostSema(), &Overflow);
    if (Overflow)
      setInvalid();
  }

  // The helper method to return FixedPoint semantics that is used to create
  // every VPInstructionCost object.
  static FixedPointSemantics getVPInstructionCostSema() {
    // We reserve 6 positions for fractional bits, which corresponds to 1/64
    // in precision. That should be enough for Alignment Analysis which may
    // have 1/64 probability of cache line boundary crossing.
    //
    // We may keep overall size to be 64 leaving 58 bits for integer part of
    // the number which corresponds to level 7 of loop nestness with default
    // trip count value (2^58 ~ 300^7). Which means that we model up to
    // 7 nested loops with TC capped at default TC.
    //
    // We may want to correct these fixed numbers once they do not provide the
    // required precision or dynamic range.
    return FixedPointSemantics {
      64 /* Width */, 6 /* Scale */, true /* IsSigned */,
        false /* IsSaturated */, false /* HasUnsignedPadding */};
  }

  // This method defines the rules how poisonous states (Invalid, Unknown)
  // propagate from RHS to the result of an operation.
  void propagateState(const VPInstructionCost &RHS) {
    if (RHS.isInvalid())
      setInvalid();
    else if (RHS.isUnknown() && !isInvalid())
      setUnknown();
  }

public:
  // A default constructed InstructionCost is a valid zero cost.
  VPInstructionCost() : Value(0, getVPInstructionCostSema()), State(Valid) {}
  VPInstructionCost(CostType Val) : VPInstructionCost() { Value = Val; }
  /// Treat all integral initializers the same way: upcast them to int64_t.
  template <typename ValTy,
            typename = std::enable_if_t<std::is_integral<ValTy>::value>>
  VPInstructionCost(ValTy Val) : VPInstructionCost() { setFromInt64Value(Val); }
  VPInstructionCost(float Val) : VPInstructionCost() { setFromFloatValue(Val); }
  VPInstructionCost(const InstructionCost& TTICost) : VPInstructionCost() {
    if (auto MaybeCost = TTICost.getValue())
      setFromInt64Value(*MaybeCost);
    else
      setInvalid();
  }

  bool isValid()   const { return State == Valid; }
  bool isUnknown() const { return State == Unknown; }
  bool isInvalid() const { return State == Invalid; }

  void setInvalid() { State = Invalid; }
  void setUnknown() { State = Unknown; }

  CostState getState() const { return State; }

  static VPInstructionCost getMax() {
    return CostType::getMax(getVPInstructionCostSema());
  }

  static VPInstructionCost getInvalid() {
    VPInstructionCost Tmp;
    Tmp.setInvalid();
    return Tmp;
  }

  static VPInstructionCost getUnknown() {
    VPInstructionCost Tmp;
    Tmp.setUnknown();
    return Tmp;
  }

  static VPInstructionCost Min(const VPInstructionCost &LHS,
                               const VPInstructionCost &RHS) {
    if (RHS < LHS)
      return RHS;
    return LHS;
  }

  /// Only a Valid Cost value can be extracted out of VPInstructionCost.
  CostType getValue() const {
    assert(isValid() && "Attempt to extract Invalid/Unknown cost.");
    return Value;
  }

  // The interface to convert VPInstructionCost returned by VPlan
  // CostModel into 64 bit integer dropping fractional bits.
  int64_t getInt64Value() const {
    return getValue().getIntPart().getExtValue();
  }

  // The interface to convert VPInstructionCost returned by VPlan
  // CostModel into 32 bit float value.
  float getFloatValue() const {
    return getValue().convertToFloat(APFloatBase::IEEEsingle()).
      convertToFloat();
  }

  /// For all of the arithmetic operators provided here any Invalid or Unknown
  /// state is perpetuated and cannot be removed. Once a cost becomes Invalid
  /// or Unknown it stays Invalid/Unknown, and it also inherits any Invalid/
  /// Unknown state from the RHS.
  ///
  /// If arithmetic operation on the actual values overflows the result has
  /// Invalid state.
  VPInstructionCost &operator+=(const VPInstructionCost &RHS) {
    propagateState(RHS);
    if (!isValid())
      return *this;

    bool Overflow = false;
    Value = getValue().add(RHS.getValue(), &Overflow);
    if (Overflow)
      setInvalid();
    return *this;
  }

  VPInstructionCost &operator*=(const VPInstructionCost &RHS) {
    propagateState(RHS);
    if (!isValid())
      return *this;

    bool Overflow = false;
    Value = getValue().mul(RHS.getValue(), &Overflow);
    if (Overflow)
      setInvalid();
    return *this;
  }

  VPInstructionCost &operator-=(const VPInstructionCost &RHS) {
    propagateState(RHS);
    if (!isValid())
      return *this;

    bool Overflow = false;
    Value = getValue().sub(RHS.getValue(), &Overflow);
    if (Overflow)
      setInvalid();
    return *this;
  }

  VPInstructionCost &operator/=(const VPInstructionCost &RHS) {
    propagateState(RHS);
    if (!isValid())
      return *this;

    bool Overflow = false;
    Value = getValue().div(RHS.getValue(), &Overflow);
    if (Overflow)
      setInvalid();
    return *this;
  }

  // Relation operators allow comparison of Invalid or Unknown costs with the
  // certain limits.
  // You can't compare with '<' or '>' the Unknown/Invalid costs, but you can
  // check for '==' or '!=' relationship the Unknown/Invalid costs.
  //
  bool operator< (const VPInstructionCost &RHS) const {
    return getValue() < RHS.getValue();
  }

  bool operator> (const VPInstructionCost &RHS) const { return RHS < *this; }
  bool operator>=(const VPInstructionCost &RHS) const { return !(*this < RHS); }
  bool operator<=(const VPInstructionCost &RHS) const { return !(*this > RHS); }
  bool operator==(const VPInstructionCost &RHS) const {
    if (State != RHS.State)
      return false;
    if (State == Unknown || State == Invalid)
      return true;
    return !(*this < RHS) && !(RHS < *this);
  }
  bool operator!=(const VPInstructionCost &RHS) const {
    return !(*this == RHS);
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  std::string toString(bool ForceSignPrint = false) const {
    std::ostringstream outString;
    if (isInvalid())
      outString << "Invalid";
    else if (isUnknown())
      outString << "Unknown";
    else {
      if (ForceSignPrint && *this > VPInstructionCost())
        outString << '+';
      // We want to print out big integers as '1000000' rather than '1e+6',
      // which is std::fixed format of float printing. On other hand we
      // don't want to see trailing zeros, which is std::defaultfloat format.
      // In order to satisfy these both requirements we output the value of
      // integer type whenever fractional bits are zeros.
      //
      // Figure out the precision required for printing from the current Fixed
      // Point Semantics.
      FixedPointSemantics Sema = getVPInstructionCostSema();

      // The number takes Sema.getWidth() bits to represent it.
      // 2^10 ~ 10^3 ==> 2^Sema.getWidth() = (2^10)^K = (10^3)^K
      // K <= (Sema.getWidth() / 10) + 1 and Precision = 3 * K.
      unsigned Precision = 3 * (Sema.getWidth() / 10 + 1);
      if (getInt64Value() == getFloatValue())
        outString << std::setprecision(Precision) << getInt64Value();
      else
        outString << std::setprecision(Precision) << getFloatValue();
    }
    return outString.str();
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
inline raw_ostream &operator<<(raw_ostream &OS, const VPInstructionCost &IC) {
  return OS << IC.toString();
}

#endif // !NDEBUG || LLVM_ENABLE_DUMP

inline VPInstructionCost operator+(const VPInstructionCost &LHS,
                                   const VPInstructionCost &RHS) {
  VPInstructionCost LHS2{LHS};
  LHS2 += RHS;
  return LHS2;
}

inline VPInstructionCost operator-(const VPInstructionCost &LHS,
                                   const VPInstructionCost &RHS) {
  VPInstructionCost LHS2{LHS};
  LHS2 -= RHS;
  return LHS2;
}

inline VPInstructionCost operator*(const VPInstructionCost &LHS,
                                   const VPInstructionCost &RHS) {
  VPInstructionCost LHS2{LHS};
  LHS2 *= RHS;
  return LHS2;
}

inline VPInstructionCost operator/(const VPInstructionCost &LHS,
                                   const VPInstructionCost &RHS) {
  VPInstructionCost LHS2{LHS};
  LHS2 /= RHS;
  return LHS2;
}

// The getCost() for VPlan returns a pair of costs, first is a loop iteration
// cost and the second is cost of preheader/postexit blocks where the vector
// initiliazations/finalizations reside.
using VPlanCostPair = std::pair<VPInstructionCost, VPInstructionCost>;

} // namespace vpo
#endif // INTEL_CUSTOMIZATION

} // namespace llvm

#endif
