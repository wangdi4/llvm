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
namespace __imf_impl_remainder_d_xa {
namespace {
typedef struct {
  VUINT64 _dHighMask;
  VUINT64 _dAbsMask;
  VUINT64 _dSignMask;
  VUINT64 _dHalf;
  VUINT32 _iMaxQExp;
  VUINT32 _iYSub;
  VUINT32 _iYCmp;
} __devicelib_imf_internal_dremainder_data_t;
static const __devicelib_imf_internal_dremainder_data_t
    __devicelib_imf_internal_dremainder_data = {
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
};
inline int __devicelib_imf_internal_dremainder(const double *a, const double *b,
                                               double *r) {
  int nRet = 0;
  VUINT64 signif_x, signif_y, rem_bit, quo_bit, tmp_x, tmp_y;
  VSINT32 exp_x, exp_y, i, j;
  VUINT32 expabs_diff, special_op = 0, signbit_x;
  double result, abs_x, abs_y;
  double zero = 0.0;
  double x = (*a), y = (*b);
  // Remove sign bits
  tmp_x = ((*(_iml_int64_t *)&x)) & (0x7FFFFFFFFFFFFFFFL);
  tmp_y = ((*(_iml_int64_t *)&y)) & (0x7FFFFFFFFFFFFFFFL);
  signbit_x = (VUINT32)((*(_iml_int64_t *)&x) >> 63);
  // Get double absolute values
  abs_x = *(double *)&tmp_x;
  abs_y = *(double *)&tmp_y;
  // Remove exponent bias
  exp_x = (VSINT32)((tmp_x & (0x7FF0000000000000L)) >> 52) - 1023;
  exp_y = (VSINT32)((tmp_y & (0x7FF0000000000000L)) >> 52) - 1023;
  // Test for NaNs, Infs, and Zeros
  if ((exp_x == (0x00000400L)) || (exp_y == (0x00000400L)) || (tmp_x == 0L) ||
      (tmp_y == 0L))
    special_op++;
  // Get significands
  signif_x = (tmp_x & (0x000FFFFFFFFFFFFFL));
  signif_y = (tmp_y & (0x000FFFFFFFFFFFFFL));
  // Process NaNs, Infs, and Zeros
  if (special_op) {
    // x is NaN
    if ((signif_x != 0L) && (exp_x == (0x00000400L)))
      result = x * 1.7;
    // y is NaN
    else if ((signif_y != 0L) && (exp_y == (0x00000400L)))
      result = y * 1.7;
    // y is zero
    else if (abs_y == zero) {
      result = zero / zero;
      nRet = 1;
    }
    // x is zero
    else if (abs_x == zero)
      result = x;
    // x is Inf
    else if ((signif_x == 0L) && (exp_x == (0x00000400L)))
      result = zero / zero;
    // y is Inf
    else
      result = x;
    (*r) = result;
    return nRet;
  }
  // If x < y, fast return
  if (abs_x <= abs_y) {
    if (abs_x == abs_y) {
      (*r) = (zero * x);
      return nRet;
    }
    // Is x too big to scale up by 2.0?
    if (exp_x != 1023) {
      if ((2.0 * abs_x) <= abs_y) {
        (*r) = (x);
        return nRet;
      }
    }
    result = abs_x - abs_y;
    if (signbit_x)
      result = -result;
    (*r) = result;
    return nRet;
  }
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
  // Zero remainder ... return immediately with sign of x
  if (rem_bit == 0L) {
    (*r) = (zero * x);
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
  if ((2.0 * result) >= abs_y) {
    if ((2.0 * result) == abs_y) {
      if (quo_bit & 0x01)
        result = -result;
    } else
      result = result - abs_y;
  }
  // Final adjust for sign of input
  if (signbit_x)
    result = -result;
  (*r) = result;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_remainder_d_xa */
DEVICE_EXTERN_C_INLINE double __devicelib_imf_remainder(double x, double y) {
  using namespace __imf_impl_remainder_d_xa;
  double r;
  VUINT32 vm;
  double va1;
  double va2;
  double vr1;
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
    dZero = as_double(0L);
    /* Absolute values */
    dAbsMask = as_double(__devicelib_imf_internal_dremainder_data._dAbsMask);
    dXAbs = as_double((as_ulong(va1) & as_ulong(dAbsMask)));
    dYAbs = as_double((as_ulong(va2) & as_ulong(dAbsMask)));
    lXAbs = as_ulong(dXAbs);
    lYAbs = as_ulong(dYAbs);
    iXExp = ((VUINT32)((VUINT64)lXAbs >> 32));
    iYExp = ((VUINT32)((VUINT64)lYAbs >> 32));
    iYSub = (__devicelib_imf_internal_dremainder_data._iYSub);
    iYCmp = (__devicelib_imf_internal_dremainder_data._iYCmp);
    iYSpec = (iYExp - iYSub);
    iYSpec = ((VUINT32)(-(VSINT32)((VSINT32)iYSpec > (VSINT32)iYCmp)));
    iXExp = ((VUINT32)(iXExp) >> (20));
    iYExp = ((VUINT32)(iYExp) >> (20));
    iQExp = (iXExp - iYExp);
    iMaxQExp = (__devicelib_imf_internal_dremainder_data._iMaxQExp);
    iRangeMask = ((VUINT32)(-(VSINT32)((VSINT32)iQExp > (VSINT32)iMaxQExp)));
    iRangeMask = (iRangeMask | iYSpec);
    vm = 0;
    vm = iRangeMask;
    /* yhi = y & highmask */
    dHighMask = as_double(__devicelib_imf_internal_dremainder_data._dHighMask);
    dYHi = as_double((as_ulong(va2) & as_ulong(dHighMask)));
    /* ylo = y - yhi */
    dYLo = (va2 - dYHi);
    /* q = x/y */
    dQ = (va1 / va2);
    /* iq = trunc(q) */
    dQ = __rint(dQ);
    /* yhi*iq */
    dQYHi = (dQ * dYHi);
    /* ylo*iq */
    dQYLo = (dQ * dYLo);
    /* res = x - yhi*iq */
    dRes = (va1 - dQYHi);
    /* res = res - ylo*iq */
    dRes = (dRes - dQYLo);
    /* get result's abs value and sign */
    dSignMask = as_double(__devicelib_imf_internal_dremainder_data._dSignMask);
    dResSign = as_double((as_ulong(dRes) & as_ulong(dSignMask)));
    dAbsRes = as_double((as_ulong(dRes) & as_ulong(dAbsMask)));
    dXSign = as_double((as_ulong(va1) & as_ulong(dSignMask)));
    /* |y|*0.5 */
    dHalf = as_double(__devicelib_imf_internal_dremainder_data._dHalf);
    dCorr = (dYAbs * dHalf);
    /* if |res| > |y|*0.5 */
    dCorr = as_double((VUINT64)((dAbsRes > dCorr) ? 0xffffffffffffffff : 0x0));
    /* then res = res - sign(res)*|y| */
    dCorr = as_double((as_ulong(dCorr) & as_ulong(dYAbs)));
    dCorr = as_double((as_ulong(dCorr) | as_ulong(dResSign)));
    dRes = (dRes - dCorr);
    dZeroRes = as_double((VUINT64)((dRes == dZero) ? 0xffffffffffffffff : 0x0));
    dZeroRes = as_double((as_ulong(dZeroRes) & as_ulong(dXSign)));
    vr1 = as_double((as_ulong(dRes) | as_ulong(dZeroRes)));
  }
  if (__builtin_expect((vm) != 0, 0)) {
    double __cout_a1;
    double __cout_a2;
    double __cout_r1;
    ((double *)&__cout_a1)[0] = va1;
    ((double *)&__cout_a2)[0] = va2;
    ((double *)&__cout_r1)[0] = vr1;
    __devicelib_imf_internal_dremainder(&__cout_a1, &__cout_a2, &__cout_r1);
    vr1 = ((const double *)&__cout_r1)[0];
  }
  r = vr1;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
