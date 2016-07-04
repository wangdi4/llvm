//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   VPOSLEV.h -- Defines the SIMD Lane Evolution data type.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_SLEV_H
#define LLVM_ANALYSIS_VPO_SLEV_H

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/APFloat.h"

namespace llvm { // LLVM Namespace

namespace vpo {  // VPO Vectorizer Namespace

enum SLEVKind {
  BOTTOM,   /// No SLEV information
  CONSTANT, /// Compile-time constant
  UNIFORM,  /// All elements in vector are identical
  STRIDED,  /// Elements are in a compile-time constant stride
  RANDOM,   /// TOP, no evolution between SIMD lanes.
};

/// \brief This class holds a SIMD lane evolution state.
class SLEV {

public:

  /// A class used by SLEVs for representing and operating on the numeric
  /// values SLEV manipulates (representing constant values and strides).
  class Number {
  private:
    bool Defined;
    bool IsInteger;
    APSInt IntN;
    APFloat FloatN;
  public:
    Number() :
        Defined(false), IsInteger(false), IntN(APSInt::getAllOnesValue(32)),
        FloatN(APFloat::getAllOnesValue(32)) {}
    Number(const APSInt& N) : Defined(true),
                              IsInteger(true),
                              IntN(N),
                              FloatN(APFloat::getAllOnesValue(32)) {}
    Number(const APFloat& N) : Defined(true),
                               IsInteger(true),
                               IntN(APSInt::getAllOnesValue(32)),
                               FloatN(N) {}
    Number(const Number& O) : Defined(O.Defined),
                              IsInteger(O.IsInteger), 
                              IntN(O.IntN),
                              FloatN(O.FloatN) {}

    void print(raw_ostream& OS) const {
      if (!Defined) {
        OS << "<?>";
        return;
      }
      if (IsInteger)
        OS << IntN;
      else {
        SmallVector<char, 10> StrFloat;
        FloatN.toString(StrFloat);
        OS << StrFloat;
      }
    }

    bool operator==(const Number& O) const {
      if (!Defined)
        return !O.Defined;
      if (!O.Defined)
        return false;
      if (IsInteger != O.IsInteger)
        return false;
      if (IsInteger)
        return APInt::isSameValue(IntN, O.IntN);
      return FloatN.compare(O.FloatN) == APFloat::cmpEqual;
    }

    bool operator!=(const Number &O) const { return !(*this == O); }

    Number operator+(const Number& RHS) const {
      assert(Defined && RHS.Defined && "Operating on undefined numbers");
      if (IsInteger && RHS.IsInteger) {
        return Number(IntN + RHS.IntN);
      }
      if (IsInteger) {
        APFloat AsFloat(RHS.FloatN.getSemantics());
        APFloat::roundingMode rm = APFloat::rmNearestTiesToEven; // TODO
        AsFloat.convertFromAPInt(IntN, IntN.isSigned(), rm);
        return Number(AsFloat + RHS.FloatN);
      }
      if (RHS.IsInteger) {
        APFloat AsFloat(FloatN.getSemantics());
        APFloat::roundingMode rm = APFloat::rmNearestTiesToEven; // TODO
        AsFloat.convertFromAPInt(RHS.IntN, RHS.IntN.isSigned(), rm);
        return Number(FloatN + AsFloat);
      }
      return Number(FloatN + RHS.FloatN);
    }

    Number operator-(const Number& RHS) const {
      assert(Defined && RHS.Defined && "Operating on undefined numbers");
      if (IsInteger && RHS.IsInteger) {
        return Number(IntN - RHS.IntN);
      }
      if (IsInteger) {
        APFloat AsFloat(RHS.FloatN.getSemantics());
        APFloat::roundingMode rm = APFloat::rmNearestTiesToEven; // TODO
        AsFloat.convertFromAPInt(IntN, IntN.isSigned(), rm);
        return Number(AsFloat - RHS.FloatN);
      }
      if (RHS.IsInteger) {
        APFloat AsFloat(FloatN.getSemantics());
        APFloat::roundingMode rm = APFloat::rmNearestTiesToEven; // TODO
        AsFloat.convertFromAPInt(RHS.IntN, RHS.IntN.isSigned(), rm);
        return Number(FloatN - AsFloat);
      }
      return Number(FloatN - RHS.FloatN);
    }

    Number operator*(const Number& RHS) const {
      assert(Defined && RHS.Defined && "Operating on undefined numbers");
      if (IsInteger && RHS.IsInteger) {
        return Number(IntN * RHS.IntN);
      }
      if (IsInteger) {
        APFloat AsFloat(RHS.FloatN.getSemantics());
        APFloat::roundingMode rm = APFloat::rmNearestTiesToEven; // TODO
        AsFloat.convertFromAPInt(IntN, IntN.isSigned(), rm);
        return Number(AsFloat * RHS.FloatN);
      }
      if (RHS.IsInteger) {
        APFloat AsFloat(FloatN.getSemantics());
        APFloat::roundingMode rm = APFloat::rmNearestTiesToEven; // TODO
        AsFloat.convertFromAPInt(RHS.IntN, RHS.IntN.isSigned(), rm);
        return Number(FloatN * AsFloat);
      }
      return Number(FloatN * RHS.FloatN);
    }

    Number operator/(const Number& RHS) const {
      assert(Defined && RHS.Defined && "Operating on undefined numbers");
      if (IsInteger && RHS.IsInteger) {
        return Number(IntN / RHS.IntN);
      }
      if (IsInteger) {
        APFloat AsFloat(RHS.FloatN.getSemantics());
        APFloat::roundingMode rm = APFloat::rmNearestTiesToEven; // TODO
        AsFloat.convertFromAPInt(IntN, IntN.isSigned(), rm);
        return Number(AsFloat / RHS.FloatN);
      }
      if (RHS.IsInteger) {
        APFloat AsFloat(FloatN.getSemantics());
        APFloat::roundingMode rm = APFloat::rmNearestTiesToEven; // TODO
        AsFloat.convertFromAPInt(RHS.IntN, RHS.IntN.isSigned(), rm);
        return Number(FloatN / AsFloat);
      }
      return Number(FloatN / RHS.FloatN);
    }

    APSInt getInteger() const {
      assert(Defined && IsInteger && "This is not an integer number");
      return IntN;
    }
    APFloat getFloat() const {
      assert(Defined && !IsInteger && "This is not a floating-point number");
      return FloatN;
    }
  };

  static const Number Zero;
  static const Number One;

private:

  /// The kind of this SLEV.
  SLEVKind Kind;

  /// The (possible) compile-time numeric value related to this SLEV.
  Number NumericValue;

public:

  SLEV() {
    setBOTTOM();
  }

  /// \brief Constructor for SLEV kinds that do not take a numeric value.
  SLEV(SLEVKind SK) {
    switch(SK) {
    case BOTTOM:
      setBOTTOM();
      break;
    case UNIFORM:
      setUNIFORM();
      break;
    case RANDOM:
      setRANDOM();
      break;
    default:
      llvm_unreachable("SLEV kind requires a numeric value");
    }
  }

  /// \brief Cconstructor for SLEV kinds that take a numeric value.
  SLEV(SLEVKind SK, const Number& N) {
    switch(SK) {
    case CONSTANT:
      setCONSTANT(N);
      break;
    case STRIDED:
      setSTRIDED(N);
      break;
    default:
      llvm_unreachable("SLEV kind does not require a numeric value");
    }
  }

  SLEV(const SLEV& Other) {
    copyValue(Other);
  }

  virtual ~SLEV() {}

  void copyValue(const SLEV& Other) {
    Kind = Other.Kind;
    NumericValue = Other.NumericValue;
  }

  void setBOTTOM() {
    Kind = SLEVKind::BOTTOM;
    NumericValue = Number();
  }

  void setCONSTANT(Number C) {
    Kind = SLEVKind::CONSTANT;
    NumericValue = C;
  }

  void setUNIFORM() {
    Kind = SLEVKind::UNIFORM;
    NumericValue = Number();
  }

  void setSTRIDED(Number C) {
    Kind = SLEVKind::STRIDED;
    NumericValue = C;
  }

  void setRANDOM() {
    Kind = SLEVKind::RANDOM;
    NumericValue = Number();
  }

  bool operator==(const SLEV& O) {
    return Kind == O.Kind && NumericValue == O.NumericValue;
  }

  bool operator!=(const SLEV& O) { return !(*this == O); }

  SLEVKind getKind() const { return Kind; }

  const Number& getConstant() const {
    static Number Undef;
    if (isBOTTOM())
      return Undef;
    assert(isConstant() && "Cannot get constant for non-constant");
    return NumericValue;
  }

  const Number& getStride() const {
    static Number Undef;
    if (isBOTTOM())
      return Undef;
    assert(isStrided() && "Cannot get stride for non-strided");
    if (isUniform())
      return Zero; // The stride of any Uniform is zero.
    return NumericValue;
  }

  /// \brief This is the SLEV lattice least upper bound operation.
  static SLEV join(const SLEV &A, const SLEV &B) {
    const SLEV *Low = A.Kind <= B.Kind ? &A : &B;
    const SLEV *High = A.Kind <= B.Kind ? &B : &A;
    switch (Low->Kind) {
    case BOTTOM:
      return *High;
    case CONSTANT:
      switch (High->Kind) {
      case CONSTANT:
        if (Low->NumericValue != High->NumericValue)
          return SLEV(UNIFORM); // Two different CONSTANTs.
        // Fallthrough
      case UNIFORM:
        return *High; // Two equal CONSTANTS, or CONSTANT V UNIFORM.
      default:
        break;
      }
      break;
    case UNIFORM:
      if (High->Kind == UNIFORM)
        return *High;
      break;
    case STRIDED:
      if (High->Kind == STRIDED && Low->NumericValue == High->NumericValue)
        return *High; // Two equal STRIDED.
    default:
      break;
    }

    // Couldn't find a lesser upper bound than RANDOM.
    return SLEV(RANDOM);
  }

  bool isBOTTOM() const {
    return Kind == SLEVKind::BOTTOM;
  }

  bool isConstant() const {
    return Kind == SLEVKind::CONSTANT;
  }

  bool isUniform() const {
    switch (Kind) {
    case SLEVKind::CONSTANT:
    case SLEVKind::UNIFORM:
      return true;
    default:
      return false;
    }
  }

  bool isSTRIDED() const {
    return Kind == SLEVKind::STRIDED;
  }

  bool isRANDOM() const {
    return Kind == SLEVKind::RANDOM;
  }

  /// \brief Convenience method for determining the special case of stride 1.
  bool isConsecutive() const {
    return Kind == SLEVKind::STRIDED && NumericValue == One;
  }

  /// \brief Convenience method for determining if the lanes have a constant
  /// stride, including a stride of zero.
  /// If you're interested if finding if the SLEV's Kind is STRIDED use
  /// isSTRIDED().
  bool isStrided() const {
    switch (Kind) {
    case SLEVKind::CONSTANT:
    case SLEVKind::UNIFORM:
    case SLEVKind::STRIDED:
      return true;
    default:
      return false;
    }
  }

  void printValue(raw_ostream& OS) const {
    switch (Kind) {
    case BOTTOM:
      OS << "BOTTOM";
      break;
    case CONSTANT:
      OS << "CONSTANT<";
      NumericValue.print(OS);
      OS << ">";
      break;
    case UNIFORM:
      OS << "UNIFORM";
      break;
    case STRIDED:
      OS << "STRIDED<";
      NumericValue.print(OS);
      OS << ">";
      break;
    case RANDOM:
      OS << "RANDOM";
      break;
    }
  }
};

} // End VPO Vectorizer Namespace

} // End LLVM Namespace 

#endif // LLVM_ANALYSIS_VPO_SLEV_H
