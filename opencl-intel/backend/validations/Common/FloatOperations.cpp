// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "FloatOperations.h"
#include "imathLibd.h"
#include "reference_math.h"
#include <algorithm>

namespace Validation {
namespace Utils {
template <> IntStorage<float> FloatParts<float>::GetMinNormalized() {
  return FLOAT_MIN_NORMALIZED;
}
template <> IntStorage<double> FloatParts<double>::GetMinNormalized() {
  return DOUBLE_MIN_NORMALIZED;
}

template <> IntStorage<float> FloatParts<float>::GetSignMask() {
  return FLOAT_SIGN_MASK;
}
template <> IntStorage<double> FloatParts<double>::GetSignMask() {
  return DOUBLE_SIGN_MASK;
}

template <> IntStorage<float> FloatParts<float>::GetExpMask() {
  return FLOAT_EXP_MASK;
}
template <> IntStorage<double> FloatParts<double>::GetExpMask() {
  return DOUBLE_EXP_MASK;
}

template <> IntStorage<float> FloatParts<float>::GetMantMask() {
  return FLOAT_MANT_MASK;
}
template <> IntStorage<double> FloatParts<double>::GetMantMask() {
  return DOUBLE_MANT_MASK;
}

template <> int FloatParts<float>::GetSignificandSize() {
  return SIGNIFICAND_BITS_FLOAT;
}
template <> int FloatParts<double>::GetSignificandSize() {
  return SIGNIFICAND_BITS_DOUBLE;
}

/// Define comparison functions for CFloat16 type
#define DEFINE_CFLOAT16_CMP(FUNC)                                              \
  template <> bool FUNC(CFloat16 a, CFloat16 b) {                              \
    float aFloat = a;                                                          \
    float bFloat = b;                                                          \
    return FUNC##_float(aFloat, bFloat);                                       \
  }
DEFINE_CFLOAT16_CMP(lt)
DEFINE_CFLOAT16_CMP(le)
DEFINE_CFLOAT16_CMP(gt)
DEFINE_CFLOAT16_CMP(ge)
DEFINE_CFLOAT16_CMP(eq)

/// Define comparison functions for float and double
#define DEFINE_FLOAT_CMP(FUNC)                                                 \
  template <> bool FUNC(float a, float b) { return FUNC##_float(a, b); }       \
  template <> bool FUNC(double a, double b) { return FUNC##_float(a, b); }

DEFINE_FLOAT_CMP(lt)
DEFINE_FLOAT_CMP(le)
DEFINE_FLOAT_CMP(gt)
DEFINE_FLOAT_CMP(ge)
DEFINE_FLOAT_CMP(eq)

template <> bool lt(long double a, long double b) {
  if (IsNaN(a) || IsNaN(b))
    return false;

  uint80_t ap = AsUInt(a);
  uint80_t bp = AsUInt(b);

  // true is positive
  bool aSign = !(bool((ap.high_val & DOUBLE_80BIT_SIGN_MASK)));
  bool bSign = !(bool((bp.high_val & DOUBLE_80BIT_SIGN_MASK)));

  uint32_t aExp = uint32_t(ap.high_val & DOUBLE_80BIT_EXP_MASK);
  uint32_t bExp = uint32_t(bp.high_val & DOUBLE_80BIT_EXP_MASK);

  // low 64 bit is a mantissa
  uint64_t aMant = ap.low_val; // & (DOUBLE_80BIT_F_MASK | DOUBLE_80BIT_I_MASK);
  uint64_t bMant = bp.low_val; // & (DOUBLE_80BIT_F_MASK | DOUBLE_80BIT_I_MASK);

  if (aSign != bSign) {
    // if the values are not of the same sign
    //  we should return true if a is negative and false if it is positive
    return !aSign;
  }
  // aSign == bSign

  if (aExp != bExp) {
    return aSign ? (aExp < bExp) : (aExp > bExp);
  }
  // aExp == bExp

  if (aMant != bMant) {
    return aSign ? (aMant < bMant) : (aMant > bMant);
  }

  // values are bitwise equal
  return false;
}

template <> bool le(long double a, long double b) { return ge(b, a); }

template <> bool gt(long double a, long double b) { return lt(b, a); }

template <> bool ge(long double a, long double b) {
  if (IsNaN(a) || IsNaN(b))
    return false;
  return !lt(a, b);
}

template <> bool eq(long double a, long double b) {
  if (IsNaN(a) || IsNaN(b))
    return false;
  return !lt(a, b) && !lt(b, a);
}

template <> bool IsNaN(float a) {
  uint32_t u = AsUInt(a);
  return (((u & FLOAT_EXP_MASK) == FLOAT_EXP_MASK) && (u & FLOAT_MANT_MASK));
}

template <> bool IsInf(float a) {
  uint32_t u = AsUInt(a);
  return (((u & FLOAT_EXP_MASK) == FLOAT_EXP_MASK) &&
          ((u & FLOAT_MANT_MASK) == 0));
}

template <> bool IsPInf(float a) {
  uint32_t ahex = AsUInt(a);
  return (ahex == 0x7F800000);
}

template <> bool IsNInf(float a) {
  uint32_t ahex = AsUInt(a);
  return (ahex == 0xFF800000);
}

template <> bool IsDenorm(float a) {
  uint32_t u = AsUInt(a);
  return ((u & FLOAT_EXP_MASK) == 0) && ((u & FLOAT_MANT_MASK) != 0);
}

template <> bool IsNaN(double a) {
  uint64_t l = AsUInt(a);
  return (((l & DOUBLE_EXP_MASK) == DOUBLE_EXP_MASK) && (l & DOUBLE_MANT_MASK));
}

template <> bool IsInf(double a) {
  uint64_t l = AsUInt(a);
  return (((l & DOUBLE_EXP_MASK) == DOUBLE_EXP_MASK) &&
          ((l & DOUBLE_MANT_MASK) == 0));
}

template <> bool IsPInf(double a) {
  uint64_t ahex = AsUInt(a);
  return (ahex == 0x7FF0000000000000);
}

template <> bool IsNInf(double a) {
  uint64_t ahex = AsUInt(a);
  return (ahex == 0xFFF0000000000000);
}

template <> bool IsDenorm(double a) {
  uint64_t l = AsUInt(a);
  return ((l & DOUBLE_EXP_MASK) == 0) && ((l & DOUBLE_MANT_MASK) != 0);
}

// The 80-bit extended format is divided into four fields
// mantissa is 64 bit, i.e. fields i and f together
//  1      15      1             63
// [s][    e     ][i][           f            ]
//
// 0<=e<=32766 i==1   f==any   normalized
//    e==0     i==0   f!=0     denormalized
//    e==0     i==0   f=0      zero
// e=32767     i==any f=0      infinity
// e=32767     i==any f!=0     NaN

template <> bool IsNaN(long double a) {
  uint80_t l = AsUInt(a);
  return (((l.high_val & DOUBLE_80BIT_EXP_MASK) == DOUBLE_80BIT_EXP_MASK) &&
          (l.low_val & DOUBLE_80BIT_F_MASK));
}

template <> bool IsInf(long double a) {
  uint80_t l = AsUInt(a);
  return (((l.high_val & DOUBLE_80BIT_EXP_MASK) == DOUBLE_80BIT_EXP_MASK) &&
          ((l.low_val & DOUBLE_80BIT_F_MASK) == 0));
}

template <> bool IsPInf(long double a) {
  uint80_t l = AsUInt(a);
  return (l.high_val == DOUBLE_80BIT_EXP_MASK &&
          ((l.low_val & DOUBLE_80BIT_F_MASK) == 0));
}

template <> bool IsNInf(long double a) {
  uint80_t l = AsUInt(a);
  return (l.high_val == (DOUBLE_80BIT_SIGN_MASK | DOUBLE_80BIT_EXP_MASK) &&
          ((l.low_val & DOUBLE_80BIT_F_MASK) == 0));
}

template <> bool IsDenorm(long double a) {
  uint80_t l = AsUInt(a);
  return (((l.high_val & DOUBLE_80BIT_EXP_MASK) == 0) &&
          ((l.low_val & DOUBLE_80BIT_F_MASK) != 0) &&
          ((l.low_val & DOUBLE_80BIT_I_MASK) == 0));
}

template <> bool IsNaN(CFloat16 a) { return a.IsNaN(); }

template <> bool IsInf(CFloat16 a) { return (a.IsPInf() && a.IsNInf()); }

template <> bool IsPInf(CFloat16 a) { return a.IsPInf(); }

template <> bool IsNInf(CFloat16 a) { return a.IsNInf(); }

template <> bool IsDenorm(CFloat16 a) { return a.IsDenorm(); }
template <>
bool eq_tol<double>(const double &a, const double &b, const double &tol) {
  assert(tol >= 0.0);
  assert(sizeof(long double) > sizeof(double));
  FloatParts<double> aParts(a), bParts(b);
  if (a == double(0.0) && b == double(0.0f) && (aParts.sign != bParts.sign))
    return false;
  return (::fabs(Utils::ulpsDiffSamePrecision((long double)a,
                                              (long double)b)) <= tol);
}
double ulpsDiffSamePrecision(double reference, double testVal) {
  union {
    double d;
    uint64_t u;
  } u;
  u.d = reference;

  // Note: This function presumes that someone has
  // already tested whether the result is correctly
  // rounded before calling this function.  That test:
  //
  //    if( (float) reference == test )
  //        return 0.0f;
  //
  // would ensure that cases like fabs(reference) > FLT_MAX are weeded out
  // before we get here. Otherwise, we'll return inf ulp error here, for what
  // are otherwise correctly rounded results.

  if (IsInf(reference)) {
    if (testVal == reference)
      return 0.0;

    return testVal - reference;
  }

  if (IsInf(testVal)) { // infinite test value, but finite (but possibly
                        // overflowing in float) reference.
    //
    // The function probably overflowed prematurely here. Formally, the spec
    // says this is an infinite ulp error and should not be tolerated.
    // Unfortunately, this would mean that the internal precision of some
    // half_pow implementations would have to be 29+ bits at half_powr(
    // 0x1.fffffep+31, 4) to correctly determine that 4*log2( 0x1.fffffep+31 )
    // is not exactly 128.0. You might represent this for example as 4*(32 -
    // ~2**-24), which after rounding to single is 4*32 = 128, which will
    // ultimately result in premature overflow, even though a good faith
    // representation would be correct to within 2**-29 internally.

    // In the interest of not requiring the implementation go to extraordinary
    // lengths to deliver a half precision function, we allow premature overflow
    // within the limit of the allowed ulp error. Towards, that end, we
    // "pretend" the test value is actually 2**128, the next value that would
    // appear in the number line if float had sufficient range.
    testVal = Conformance::copysign(ldexp((double)(0x1LL), 128), testVal);

    // Note that the same hack may not work in long double, which is not
    // guaranteed to have more range than double.  It is not clear that
    // premature overflow should be tolerated for double.
  }

  if (u.u & 0x000fffffffffffffULL) { // Non-power of two and NaN
    if (IsNaN(reference) && IsNaN(testVal))
      return 0.0; // if we are expecting a NaN, any NaN is fine

    // The unbiased exponent of the ulp unit place
    int ulp_exp = FLT_MANT_DIG - 1 -
                  std::max(Conformance::ilogb(reference), FLT_MIN_EXP - 1);

    // Scale the exponent of the error
    return (double)ldexp(testVal - reference, ulp_exp);
  }

  // reference is a normal power of two or a zero
  // The unbiased exponent of the ulp unit place
  int ulp_exp = FLT_MANT_DIG - 1 -
                std::max(Conformance::ilogb(reference) - 1, FLT_MIN_EXP - 1);

  // Scale the exponent of the error
  return ldexp(testVal - reference, ulp_exp);
}

double ulpsDiffSamePrecision(long double reference, long double testVal) {
  // Check for Non-power-of-two and NaN
  //  head to tail 128-bit long double is appropriate here.
  //  Simply using a 64-bit double to check double is not appropriate here.
  //  If you are stuck in this situation, you may need to switch to mpfr+gmp to
  //  get more precise answers. This will unfortunately run rather slowly and
  //  require some work.
  if (sizeof(double) >= sizeof(long double)) {
    printf("This architecture needs something higher precision than double to "
           "check the precision of double.\nTest FAILED.\n");
    abort();
  }

  // Note: This function presumes that someone has already
  // tested whether the result is correctly
  // rounded before calling this function.  That test:
  //
  //    if( (float) reference == test )
  //        return 0.0f;
  //
  // would ensure that cases like fabs(reference) > FLT_MAX are weeded out
  // before we get here. Otherwise, we'll return inf ulp error here, for what
  // are otherwise correctly rounded results.

  int x;
  if (0.5L != CimathLibd::imf_fabs(CimathLibd::imf_frexp(
                  reference, &x))) { // Non-power of two and NaN
    if (Utils::IsInf(reference)) {
      if (testVal == reference)
        return 0.0;

      return (double)(testVal - reference);
    }

    if (Utils::IsNaN(reference) && Utils::IsNaN(testVal))
      return 0.0; // if we are expecting a NaN, any NaN is fine

    // The unbiased exponent of the ulp unit place
    int ulp_exp = DBL_MANT_DIG - 1 -
                  std::max(CimathLibd::imf_ilogb(reference), DBL_MIN_EXP - 1);

    long double res = CimathLibd::imf_ldexp(testVal - reference, ulp_exp);
    // Scale the exponent of the error
    return (double)res;
  }

  // reference is a normal power of two or a zero
  // The unbiased exponent of the ulp unit place
  int ulp_exp = DBL_MANT_DIG - 1 -
                std::max(CimathLibd::imf_ilogb(reference) - 1, DBL_MIN_EXP - 1);

  // Scale the exponent of the error
  long double res = CimathLibd::imf_ldexp(testVal - reference, ulp_exp);
  return (double)res;
}

float ulpsDiff(double ref, float test) {
  double testDouble = double(test);
  double res = ulpsDiffSamePrecision(ref, testDouble);
  return (float)res;
}

float ulpsDiff(long double ref, double test) {
  long double testDouble = (long double)test;
  return (float)(ulpsDiffSamePrecision(ref, testDouble));
}

float ulpsDiffDenormal(double ref, float test) {
  union {
    uint32_t u;
    float f;
  } convert;
  convert.f = float(ref);
  float round_error = ulpsDiffSamePrecision(ref, (double)convert.f);

  int32_t aInt = (int32_t)convert.u;
  if (aInt < 0)
    aInt = 0x80000000 - aInt;

  convert.f = test;
  int32_t bInt = (int32_t)convert.u;
  if (bInt < 0)
    bInt = 0x80000000 - bInt;
  return (float(aInt - bInt) - round_error);
}

float ulpsDiffDenormal(long double ref, double test) {
  union {
    uint64_t u;
    double f;
  } convert;
  convert.f = double(ref);
  float round_error = ulpsDiffSamePrecision(ref, (long double)convert.f);

  int64_t aInt = (int64_t)convert.u;
  if (aInt < 0)
    aInt = 0x8000000000000000 - aInt;

  convert.f = test;
  int64_t bInt = (int64_t)convert.u;
  if (bInt < 0)
    bInt = 0x8000000000000000 - bInt;
  return (float(aInt - bInt) - round_error);
}

} // namespace Utils
} // namespace Validation
