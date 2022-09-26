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
//  yhi = y & highmask
//  ylo = y - yhi
//  q = x/y
//  iq = trunc(q)
//  res = x - yhi*iq - ylo*iq
//  if |res| > |y|*0.5 then res = res - sign(res)*|y|
// --
//
*/
#include "_imf_include_fp64.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_remquo_d_xa {
namespace {
typedef struct {
  VUINT64 _dHighMask;
  VUINT64 _dAbsMask;
  VUINT64 _dSignMask;
  VUINT64 _dHalf;
  VUINT32 _iMaxQExp;
  VUINT32 _iYSub;
  VUINT32 _iYCmp;
  VUINT32 _iOne;
} __devicelib_imf_internal_dremquo_data_t;
static const __devicelib_imf_internal_dremquo_data_t
    __devicelib_imf_internal_dremquo_data = {
        0xfffffffff8000000uL, /* _dHighMask  */
        0x7fffffffffffffffuL, /* _dAbsMask */
        0x8000000000000000uL, /* _dSignMask */
        0x3FE0000000000000uL, /* _dHalf */
        0x19u,                /* _iMaxQExp = 25 */
                              /*
                              //
                              //         To prevent YLow to be denormal it should be checked
                              //         that Exp(Y) < -1023+51 (worst case when only last bit is non
                              zero)
                              //         Exp(Y) < -972 -> Y < 0x`03300000`00000000
                              //         That value is used to construct SubConst by setting up first
                              bit to 1.
                              //         CmpConst is get from max acceptable value 0x77EFFFFF:
                              //         0x77EFFFFF - 0x83300000 = 0x(1)FB1FFFFF
                              //
                              */
        0x83300000u,          /* _iSub */
        0xFB1FFFFFu,          /* _iCmp */
        0x00000001u,          /* _iOne */
};
inline int __devicelib_imf_internal_dremquo(const double *a, const double *b,
                                            double *r, int *q) {
  double x = (*a);
  double y = (*b);
  VUINT64 signif_x, signif_y, rem_bit, quo_bit, tmp_x, tmp_y;
  VSINT32 exp_x, exp_y, i, j;
  VUINT32 expabs_diff, special_op = 0, signbit_x, signbit_y, sign = 1;
  double result, abs_x, abs_y;
  double zero = 0.0;
  int nRet = 0;
  // Remove sign bits
  tmp_x = ((*(_iml_int64_t *)&x)) & (0x7FFFFFFFFFFFFFFFLL);
  tmp_y = ((*(_iml_int64_t *)&y)) & (0x7FFFFFFFFFFFFFFFLL);
  signbit_x = (VUINT32)((*(_iml_int64_t *)&x) >> 63);
  signbit_y = (VUINT32)((*(_iml_int64_t *)&y) >> 63);
  if (signbit_x ^ signbit_y)
    sign = (-sign);
  // Get double absolute values
  abs_x = *(double *)&tmp_x;
  abs_y = *(double *)&tmp_y;
  // Remove exponent bias
  exp_x = (VSINT32)((tmp_x & (0x7FF0000000000000LL)) >> 52) - 1023;
  exp_y = (VSINT32)((tmp_y & (0x7FF0000000000000LL)) >> 52) - 1023;
  // Test for NaNs, Infs, and Zeros
  if ((exp_x == (0x00000400L)) || (exp_y == (0x00000400L)) || (tmp_x == 0LL) ||
      (tmp_y == 0LL))
    special_op++;
  // Get significands
  signif_x = (tmp_x & (0x000FFFFFFFFFFFFFLL));
  signif_y = (tmp_y & (0x000FFFFFFFFFFFFFLL));
  // Process NaNs, Infs, and Zeros
  if (special_op) {
    (*q) = 0;
    // x is NaN
    if ((signif_x != 0LL) && (exp_x == (0x00000400L)))
      result = x * 1.7;
    // y is NaN
    else if ((signif_y != 0LL) && (exp_y == (0x00000400L)))
      result = y * 1.7;
    // y is zero
    else if (abs_y == zero)
      result = zero / zero;
    // x is zero
    else if (abs_x == zero)
      result = x;
    // x is Inf
    else if ((signif_x == 0LL) && (exp_x == (0x00000400L)))
      result = zero / zero;
    // y is Inf
    else
      result = x;
    (*r) = result;
    return nRet;
  }
  // If x < y, fast return
  if (abs_x <= abs_y) {
    (*q) = 1 * sign;
    if (abs_x == abs_y) {
      (*r) = (zero * x);
      return nRet;
    }
    // Is x too big to scale up by 2.0?
    if (exp_x != 1023) {
      if ((2.0 * abs_x) <= abs_y) {
        (*q) = 0;
        (*r) = x;
        return nRet;
      }
    }
    result = abs_x - abs_y;
    if (signbit_x) {
      result = -result;
    }
    (*r) = result;
    return nRet;
  }
  // Check for denormal x and y, adjust and normalize
  if ((exp_x == -1023) && (signif_x != 0LL)) {
    exp_x = -1022;
    while (signif_x <= (0x000FFFFFFFFFFFFFLL)) {
      exp_x--;
      signif_x <<= 1;
    };
  } else
    signif_x = (signif_x | (0x0010000000000000LL));
  if ((exp_y == -1023) && (signif_y != 0LL)) {
    exp_y = -1022;
    while (signif_y <= (0x000FFFFFFFFFFFFFLL)) {
      exp_y--;
      signif_y <<= 1;
    };
  } else
    signif_y = (signif_y | (0x0010000000000000LL));
  //
  // Main computational path
  //
  // Calculate exponent difference
  expabs_diff = (exp_x - exp_y) + 1;
  rem_bit = signif_x;
  quo_bit = 0;
  for (i = 0; i < expabs_diff; i++) {
    quo_bit = quo_bit << 1;
    if (rem_bit >= signif_y) {
      rem_bit -= signif_y;
      quo_bit++;
    }
    rem_bit <<= 1;
  }
  // Zero remquo ... return immediately with sign of x
  if (rem_bit == 0LL) {
    (*q) = ((VUINT32)(0x000000007FFFFFFFLL & quo_bit)) * sign;
    (*r) = (zero * x);
    return nRet;
  }
  // Adjust remquo
  rem_bit >>= 1;
  //  Set exponent base, unbiased
  j = exp_y;
  //  Calculate normalization shift
  while (rem_bit <= (0x000FFFFFFFFFFFFFLL)) {
    j--;
    rem_bit <<= 1;
  };
  //  Prepare normal results
  if (j >= -1022) {
    // Remove explicit 1
    rem_bit &= (0x000FFFFFFFFFFFFFLL);
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
  if ((2.0 * result) >= abs_y) {
    if ((2.0 * result) == abs_y) {
      if (quo_bit & 0x01) {
        result = -result;
        quo_bit++;
      }
    } else {
      result = result - abs_y;
      quo_bit++;
    }
  }
  // Final adjust for sign of input
  (*q) = ((VUINT32)(0x000000007FFFFFFFLL & (quo_bit))) * sign;
  if (signbit_x)
    result = -result;
  (*r) = result;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_remquo_d_xa */
DEVICE_EXTERN_C_INLINE double __devicelib_imf_remquo(double x, double y,
                                                     int32_t *z) {
  using namespace __imf_impl_remquo_d_xa;
  double r;
  VUINT32 vm;
  double va1;
  double va2;
  double vr1;
  VUINT32 vr2;
  va1 = x;
  va2 = y;
  {
    double dZero;
    double dZeroRes;
    double dHighMask;
    double dAbsMask;
    double dSignMask;
    double dYHi;
    double dYLo;
    double dQ;
    double dXSign;
    double dYSign;
    double dQSign;
    double dResSign;
    double dQYHi;
    double dQYLo;
    double dXAbs;
    double dYAbs;
    double dRes;
    double dCorr;
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
    double dHalf;
    double dAbsRes;
    VUINT32 iQ;
    VUINT32 iOne;
    VUINT32 iCorr;
    VUINT64 lCorr;
    VUINT32 iCorrValue;
    VUINT64 lCorrValue;
    dZero = as_double(0L);
    /* Absolute values */
    dAbsMask = as_double(__devicelib_imf_internal_dremquo_data._dAbsMask);
    dXAbs = as_double((as_ulong(va1) & as_ulong(dAbsMask)));
    dYAbs = as_double((as_ulong(va2) & as_ulong(dAbsMask)));
    lXAbs = as_ulong(dXAbs);
    lYAbs = as_ulong(dYAbs);
    iXExp = ((VUINT32)((VUINT64)lXAbs >> 32));
    iYExp = ((VUINT32)((VUINT64)lYAbs >> 32));
    iYSub = (__devicelib_imf_internal_dremquo_data._iYSub);
    iYCmp = (__devicelib_imf_internal_dremquo_data._iYCmp);
    iYSpec = (iYExp - iYSub);
    iYSpec = ((VUINT32)(-(VSINT32)((VSINT32)iYSpec > (VSINT32)iYCmp)));
    iXExp = ((VUINT32)(iXExp) >> (20));
    iYExp = ((VUINT32)(iYExp) >> (20));
    iQExp = (iXExp - iYExp);
    iMaxQExp = (__devicelib_imf_internal_dremquo_data._iMaxQExp);
    iRangeMask = ((VUINT32)(-(VSINT32)((VSINT32)iQExp > (VSINT32)iMaxQExp)));
    iRangeMask = (iRangeMask | iYSpec);
    vm = 0;
    vm = iRangeMask;
    /* yhi = y & highmask */
    dHighMask = as_double(__devicelib_imf_internal_dremquo_data._dHighMask);
    dYHi = as_double((as_ulong(va2) & as_ulong(dHighMask)));
    /* ylo = y - yhi */
    dYLo = (va2 - dYHi);
    /* q = x/y */
    dQ = (va1 / va2);
    /* iq = trunc(q) */
    dQ = __rint(dQ);
    iQ = ((VINT32)(dQ));
    /* yhi*iq */
    dQYHi = (dQ * dYHi);
    /* ylo*iq */
    dQYLo = (dQ * dYLo);
    /* res = x - yhi*iq */
    dRes = (va1 - dQYHi);
    /* res = res - ylo*iq */
    dRes = (dRes - dQYLo);
    /* get result's abs value and sign */
    dSignMask = as_double(__devicelib_imf_internal_dremquo_data._dSignMask);
    dResSign = as_double((as_ulong(dRes) & as_ulong(dSignMask)));
    dAbsRes = as_double((as_ulong(dRes) & as_ulong(dAbsMask)));
    dYSign = as_double((as_ulong(va2) & as_ulong(dSignMask)));
    dQSign = as_double((as_ulong(dResSign) ^ as_ulong(dYSign)));
    dXSign = as_double((as_ulong(va1) & as_ulong(dSignMask)));
    /* prepare integer correction term */
    iOne = (__devicelib_imf_internal_dremquo_data._iOne);
    lCorrValue = as_ulong(dQSign);
    iCorrValue = ((VUINT32)((VUINT64)lCorrValue >> 32));
    iCorrValue = ((VSINT32)iCorrValue >> (31));
    iCorrValue = (iCorrValue | iOne);
    /* |y|*0.5 */
    dHalf = as_double(__devicelib_imf_internal_dremquo_data._dHalf);
    dCorr = (dYAbs * dHalf);
    /* if |res| > |y|*0.5 */
    dCorr = as_double((VUINT64)((dAbsRes > dCorr) ? 0xffffffffffffffff : 0x0));
    lCorr = as_ulong(dCorr);
    iCorr = ((VUINT32)((VUINT64)lCorr >> 32));
    iCorr = (iCorr & iCorrValue);
    /* then res = res - sign(res)*|y| */
    dCorr = as_double((as_ulong(dCorr) & as_ulong(dYAbs)));
    dCorr = as_double((as_ulong(dCorr) | as_ulong(dResSign)));
    dRes = (dRes - dCorr);
    vr2 = (iQ + iCorr);
    dZeroRes = as_double((VUINT64)((dRes == dZero) ? 0xffffffffffffffff : 0x0));
    dZeroRes = as_double((as_ulong(dZeroRes) & as_ulong(dXSign)));
    vr1 = as_double((as_ulong(dRes) | as_ulong(dZeroRes)));
  }
  if (__builtin_expect((vm) != 0, 0)) {
    double __cout_a1;
    double __cout_a2;
    double __cout_r1;
    int __cout_r2;
    ((double *)&__cout_a1)[0] = va1;
    ((double *)&__cout_a2)[0] = va2;
    ((double *)&__cout_r1)[0] = vr1;
    ((VUINT32 *)&__cout_r2)[0] = vr2;
    __devicelib_imf_internal_dremquo(&__cout_a1, &__cout_a2, &__cout_r1,
                                     &__cout_r2);
    vr1 = ((const double *)&__cout_r1)[0];
    vr2 = ((const VUINT32 *)&__cout_r2)[0];
  }
  r = vr1;
  ((VUINT32 *)z)[0] = vr2;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
