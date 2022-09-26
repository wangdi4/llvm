/*******************************************************************************
* INTEL CONFIDENTIAL
* Copyright 1996-2022 Intel Corporation.
*
* This software and the related documents are Intel copyrighted  materials,  and
* your use of  them is  governed by the  express license  under which  they were
* provided to you (License).  Unless the License provides otherwise, you may not
* use, modify, copy, publish, distribute,  disclose or transmit this software or
* the related documents without Intel's prior written permission.
*
* This software and the related documents  are provided as  is,  with no express
* or implied  warranties,  other  than those  that are  expressly stated  in the
* License.
*******************************************************************************/
/*
// ALGORITHM DESCRIPTION:
//
//  xsign = x & signmask
//
//  yhi = y & highmask
//  ylo = y - yhi
//
//  q = x/y
//
//  change sign for y if x is negative
//
//  iq = trunc(q)
//  res = |x| - yhi*iq - ylo*iq
//  if res < 0 then corr = |y| else corr = 0
//  res = res + corr
//  res = res | xsign
// --
//
*/
#include "_imf_include_fp64.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_fmod_d_xa {
namespace {
typedef struct {
  VUINT64 _dHighMask;
  VUINT64 _dAbsMask;
  VUINT64 _dSignMask;
  VUINT32 _iMaxQExp;
  VUINT32 _iYSub;
  VUINT32 _iYCmp;
  /* _lrb_ and _uisa_ data */
  VUINT64 _dMaxQExp;
  VUINT64 _dMinQExp;
  VUINT64 _dMaxYExp;
  VUINT64 _dShifter;
  VUINT64 _lBias;
  VUINT64 _dOne;
} __devicelib_imf_internal_dfmod_data_t;
static const __devicelib_imf_internal_dfmod_data_t
    __devicelib_imf_internal_dfmod_data = {
        0xfffffffff8000000uL, /* _dHighMask  */
        0x7fffffffffffffffuL, /* _dAbsMask */
        0x8000000000000000uL, /* _dSignMask */
        0x19u,                /* _iMaxQExp = 25 */
                              /*
                              //
                              //         To prevent YLow to be denormal it should be checked
                              //         that Exp(Y) < -1023+51 (worst case when only last bit is non
                              zero)
                              //         Exp(Y) < -972 -> Y < 0x`03300000`00000000
                              //         That value is used to construct SubConst by setting up first
                              bit to 1.
                              //         CmpConst is get from max acceptable value 0x7e8fffff:
                              //         0x7fefffff - 0x83300000 = 0x(1)FCBFFFFF
                              //
                              */
        0x83300000u,          /* _iSub */
        0xFCBFFFFFu,          /* _iCmp */
        /* _lrb_ and _uisa_ data */
        0x4039000000000000uL, /* _dMaxQExp */
        0xc08ff00000000000uL, /* _dMinQExp */
        0x408ff00000000000uL, /* _dMaxYExp */
        0x4138000000000000uL, /* _dShifter */
        0x3FF0000000000000uL, /* _lBias    */
        0x3FF0000000000000uL, /* _dOne     */
};
static const double _libm_zero_ = 0.0;
static const double _libm_one_ = 1.0;
static const _iml_int64_t _libm_2_to_n31 = 0x3E00000000000000;
static const _iml_int64_t _libm_2_to_p31_m1 = 0x41DFFFFFFFC00000;
inline int __devicelib_imf_internal_dfmod(const double *a, const double *b,
                                          double *r) {
  int nRet = 0;
  VUINT64 signif_x, signif_y, rem_bit, quo_bit, tmp_x;
  VUINT64 tmp_y, tmp_hold, signbit_x, tmp_n_double;
  VSINT32 exp_x, exp_y, i, j, int_32;
  VSINT64 int_64;
  VUINT32 expabs_diff, cw, n_int;
  double result, abs_x, abs_y, n_double;
  double x = (*a), y = (*b);
  // Remove sign bits
  tmp_x = (*(_iml_int64_t *)&x);
  tmp_y = ((*(_iml_int64_t *)&y)) & (0x7FFFFFFFFFFFFFFFL);
  signbit_x = tmp_x & (0x8000000000000000L);
  tmp_x = tmp_x & (0x7FFFFFFFFFFFFFFFL);
  // If abs of y == 1
  if (tmp_y == 0x3FF0000000000000L) {
    if (tmp_x <= tmp_y) {
      if (tmp_x == tmp_y) {
        (*r) = (x * _libm_zero_);
        return nRet;
      } else {
        (*r) = (x * _libm_one_);
        return nRet;
      }
    }
    // 1.0 < x <= 2^52
    else if (tmp_x < 0x4330000000000000L) {
      tmp_hold = tmp_x - tmp_y;
      tmp_hold = (52 - (tmp_hold >> 52));
      tmp_x = (tmp_x >> tmp_hold) << tmp_hold;
      tmp_x = signbit_x | tmp_x;
      result = x - *(double *)&tmp_x;
      // Correct the sign of the result with +/- ZERO is returned
      tmp_x = *(VUINT64 *)&result | signbit_x;
      result = *(double *)&tmp_x;
      (*r) = (result);
      return nRet;
    }
    // x is NaN
    else if (tmp_x < 0x7FF0000000000000L) {
      result = x * _libm_zero_;
      (*r) = (result);
      return nRet;
    }
  }
  // Raise denormal operand ... allow daz to take affect
  x *= _libm_one_;
  y *= _libm_one_;
  // x is NaN
  if (tmp_x > 0x7FF0000000000000L) {
    result = x * y;
    (*r) = (result);
    return nRet;
  }
  // y is NaN
  else if (tmp_y > 0x7FF0000000000000L) {
    result = y * x;
    (*r) = (result);
    return nRet;
  }
  // y is zero
  else if (tmp_y == 0L) {
    result = _libm_zero_ / _libm_zero_;
    //__libm_error_support (&x, &y, &result, fmod_by_zero);
    nRet = 2;
    (*r) = (result);
    return nRet;
  }
  // x is Inf
  else if (tmp_x == 0x7FF0000000000000L) {
    result = _libm_zero_ / _libm_zero_;
    nRet = 2;
    (*r) = (result);
    return nRet;
  }
  // y is Inf
  else if (tmp_y == 0x7FF0000000000000L) {
    result = x * _libm_one_;
    (*r) = (result);
    return nRet;
  }
  // If x <= y, fast return
  if (tmp_x < tmp_y) {
    (*r) = (x * _libm_one_);
    return nRet;
  }
  if (tmp_x == tmp_y) {
    (*r) = (x * _libm_zero_);
    return nRet;
  }
  abs_x = *(double *)&tmp_x;
  // if abs of y == 2147483647.0 = 2^31 - 1 and x < 2^52
  if ((tmp_y == 0x41dfffffffc00000L) && (tmp_x < 0x4330000000000000L)) {
    // Divide by y + 1
    n_double = abs_x * (*(const double *)&_libm_2_to_n31);
    //        tmp_n_double = (OWN_CONTENTS(n_double))&OWN_ZERO_MASK;
    // Quotient likely has a fractional part  ... remove
    //        tmp_hold = tmp_n_double - *(VUINT64*)_libm_one_;
    //        tmp_hold = (52 - (tmp_hold >> 52));
    //        tmp_n_double =  (tmp_n_double >> tmp_hold) << tmp_hold;
    n_int = (VUINT32)n_double;
    n_double = (double)n_int;
    // x - n * 2^31
    // n_double has no more than 20 binary digits
    // y has only 31 bits
    //      result = abs_x - *(double*)tmp_n_double * _libm_2_to_p31_m1;
    result = abs_x - n_double * (*(const double *)&_libm_2_to_p31_m1);
    // Possibly do one more correction
    if (result >= y)
      result = result - y;
    // Result may be negative ... correct if necessary
    if (signbit_x)
      result = -result;
    (*r) = (result);
    return nRet;
  }
  // Get double absolute values
  abs_y = *(double *)&tmp_y;
  // Remove exponent bias
  exp_x = (VSINT32)((tmp_x & (0x7FF0000000000000L)) >> 52) - 1023;
  exp_y = (VSINT32)((tmp_y & (0x7FF0000000000000L)) >> 52) - 1023;
  // Get significands
  signif_x = (tmp_x & (0x000FFFFFFFFFFFFFL));
  signif_y = (tmp_y & (0x000FFFFFFFFFFFFFL));
  // Check for denormal x and y, adjust and normalize
  if ((exp_x == -1023) && (signif_x != 0L)) {
    exp_x = -1022;
    while (signif_x <= (0x000FFFFFFFFFFFFFL)) {
      exp_x--;
      signif_x <<= 1;
    };
  } else
    signif_x = (signif_x | (0x0010000000000000L));
  if ((exp_y == -1023) && (signif_y != 0L)) {
    exp_y = -1022;
    while (signif_y <= (0x000FFFFFFFFFFFFFL)) {
      exp_y--;
      signif_y <<= 1;
    };
  } else
    signif_y = (signif_y | (0x0010000000000000L));
  //
  // Main computational path
  //
  // Calculate exponent difference
  expabs_diff = (exp_x - exp_y) + 1;
  rem_bit = signif_x;
  quo_bit = 0;
  for (i = 0; i < expabs_diff; i++) {
    if (rem_bit >= signif_y) {
      rem_bit -= signif_y;
      quo_bit = 1;
    } else
      quo_bit = 0;
    rem_bit <<= 1;
  }
  // Zero fmod ... return immediately with sign of x
  if (rem_bit == 0L) {
    (*r) = (_libm_zero_ * x);
    return nRet;
  }
  // Adjust remainder
  rem_bit >>= 1;
  //  Set exponent base, unbiased
  j = exp_y;
  //  Calculate normalization shift
  while (rem_bit <= (0x000FFFFFFFFFFFFFL)) {
    j--;
    rem_bit <<= 1;
  };
  //  Prepare normal results
  if (j >= -1022) {
    // Remove explicit 1
    rem_bit &= (0x000FFFFFFFFFFFFFL);
    // Set final exponent ... add exponent bias
    j = j + 1023;
  }
  //  Prepare denormal results
  else {
    // Determine denormalization shift count
    j = -j - 1022;
    // Denormalization
    rem_bit >>= j;
    // Set final exponent ... denorms are 0
    j = 0;
  }
  rem_bit = (((VUINT64)(j)) << 52) | rem_bit;
  // Create double result and adjust if >= .5 * divisor
  result = *(double *)&rem_bit;
  // Final adjust for sign of input
  if (signbit_x)
    result = -result;
  (*r) = (result);
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_fmod_d_xa */
DEVICE_EXTERN_C_INLINE double __devicelib_imf_fmod(double x, double y) {
  using namespace __imf_impl_fmod_d_xa;
  double r;
  VUINT32 vm;
  double va1;
  double va2;
  double vr1;
  va1 = x;
  va2 = y;
  {
    double dHighMask;
    double dAbsMask;
    double dSignMask;
    double dYHi;
    double dYLo;
    double dQ;
    double dXSign;
    double dQYHi;
    double dQYLo;
    double dXAbs;
    double dYAbs;
    double dRes;
    double dCorr;
    double dZero;
    VUINT64 lXAbs;
    VUINT64 lYAbs;
    VUINT32 iXExp;
    VUINT32 iYExp;
    VUINT32 iQExp;
    VUINT32 iMaxQExp;
    VUINT32 iRangeMask;
    VUINT32 iYSpec;
    VUINT32 iYSub;
    VUINT32 iYCmp;
    double dXExp;
    double dYExp;
    double dMaxYExp;
    double dAbsYExp;
    double dQExp;
    VUINT64 lQExp;
    double dMaxQExp;
    double dMinQExp;
    double dXMant;
    double dYMant;
    double dYSpec;
    double dRangeMask;
    double dRcp;
    double dE;
    double dR;
    double dOne;
    double dShifter;
    VUINT64 lBias;
    dZero = as_double(0L);
    /* Absolute values */
    dAbsMask = as_double(__devicelib_imf_internal_dfmod_data._dAbsMask);
    dXAbs = as_double((as_ulong(va1) & as_ulong(dAbsMask)));
    dYAbs = as_double((as_ulong(va2) & as_ulong(dAbsMask)));
    lXAbs = as_ulong(dXAbs);
    lYAbs = as_ulong(dYAbs);
    iXExp = ((VUINT32)((VUINT64)lXAbs >> 32));
    iYExp = ((VUINT32)((VUINT64)lYAbs >> 32));
    iYSub = (__devicelib_imf_internal_dfmod_data._iYSub);
    iYCmp = (__devicelib_imf_internal_dfmod_data._iYCmp);
    iYSpec = (iYExp - iYSub);
    iYSpec = ((VUINT32)(-(VSINT32)((VSINT32)iYSpec > (VSINT32)iYCmp)));
    iXExp = ((VUINT32)(iXExp) >> (20));
    iYExp = ((VUINT32)(iYExp) >> (20));
    iQExp = (iXExp - iYExp);
    iMaxQExp = (__devicelib_imf_internal_dfmod_data._iMaxQExp);
    iRangeMask = ((VUINT32)(-(VSINT32)((VSINT32)iQExp > (VSINT32)iMaxQExp)));
    iRangeMask = (iRangeMask | iYSpec);
    vm = 0;
    vm = iRangeMask;
    dYHi = va2;
    /* q = x/y */
    dQ = (va1 / va2);
    /* xsign = x & signmask */
    dSignMask = as_double(__devicelib_imf_internal_dfmod_data._dSignMask);
    dXSign = as_double((as_ulong(va1) & as_ulong(dSignMask)));
    /* change sign for y if x is negative */
    dYHi = as_double((as_ulong(dYHi) ^ as_ulong(dXSign)));
    /* iq = trunc(q) */
    dQ = __trunc(dQ);
    dRes = __fma(-(dQ), dYHi, dXAbs);
    /* if res < 0 */
    dCorr = as_double((VUINT64)((dRes < dZero) ? 0xffffffffffffffff : 0x0));
    /* corr = |y|, else corr = 0 */
    dCorr = as_double((as_ulong(dCorr) & as_ulong(dYAbs)));
    /* res = res + corr */
    dRes = (dRes + dCorr);
    /* res = res | xsign */
    vr1 = as_double((as_ulong(dRes) | as_ulong(dXSign)));
  }
  if (__builtin_expect((vm) != 0, 0)) {
    double __cout_a1;
    double __cout_a2;
    double __cout_r1;
    ((double *)&__cout_a1)[0] = va1;
    ((double *)&__cout_a2)[0] = va2;
    ((double *)&__cout_r1)[0] = vr1;
    __devicelib_imf_internal_dfmod(&__cout_a1, &__cout_a2, &__cout_r1);
    vr1 = ((const double *)&__cout_r1)[0];
  }
  r = vr1;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
