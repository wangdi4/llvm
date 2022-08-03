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
#include "_imf_include_fp32.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_remainder_s_xa {
namespace {
typedef struct {
  VUINT32 _sHighMask;
  VUINT32 _sAbsMask;
  VUINT32 _sSignMask;
  VUINT32 _sHalf;
  VUINT32 _iMaxQExp;
  VUINT32 _iYSub;
  VUINT32 _iYCmp;
} __devicelib_imf_internal_sremainder_data_t;
static const __devicelib_imf_internal_sremainder_data_t
    __devicelib_imf_internal_sremainder_data = {
        0xfffff000u, /* _sHighMask  */
        0x7fffffffu, /* _sAbsMask */
        0x80000000u, /* _sSignMask */
        0x3F000000u, /* _sHalf */
        0xBu,        /* _iMaxQExp = 11*/
/*
//         
//             To prevent YLow to be denormal it should be checked
//             that Exp(Y) <= -127+23 (worst case when only last bit is non zero)
//             Exp(Y) < -103 -> Y < 0x0C000000
//             That value is used to construct SubConst by setting up first bit
//            to 1. CmpConst is get from max acceptable value 0x797fffff:
//             0x797fffff - 0x8C000000 = 0x(1)ED7FFFFF
//         
*/
        0x8C000000u, /* _iSubConst */
        0xED7FFFFFu, /* _iCmpConst */
};
inline int __devicelib_imf_internal_sremainder(const float *a, const float *b,
                                               float *r) {
  int nRet = 0;
  VUINT32 signif_x, signif_y, rem_bit, quo_bit, tmp_x, tmp_y;
  VSINT32 exp_x, exp_y, i, j;
  VUINT32 expabs_diff, special_op = 0, signbit_x;
  float result, abs_x, abs_y;
  float zero = 0.0f;
  float x = (*a), y = (*b);
  // Remove sign bits
  tmp_x = ((*(int *)&x)) & (0x7FFFFFFFL);
  tmp_y = ((*(int *)&y)) & (0x7FFFFFFFL);
  signbit_x = (VUINT32)((*(int *)&x) >> 31);
  // Get float absolute values
  abs_x = *(float *)&tmp_x;
  abs_y = *(float *)&tmp_y;
  // Remove exponent bias
  exp_x = (VSINT32)((tmp_x & (0x7FF00000L)) >> 23) - 127;
  exp_y = (VSINT32)((tmp_y & (0x7FF00000L)) >> 23) - 127;
  // Test for NaNs, Infs, and Zeros
  if ((exp_x == (0x00000080L)) || (exp_y == (0x00000080L)) ||
      (tmp_x == (0x00000000L)) || (tmp_y == (0x00000000L)))
    special_op++;
  // Get significands
  signif_x = (tmp_x & (0x007FFFFFL));
  signif_y = (tmp_y & (0x007FFFFFL));
  // Process NaNs, Infs, and Zeros
  if (special_op) {
    // x is NaN
    if ((signif_x != (0x00000000L)) && (exp_x == (0x00000080L)))
      result = x * 1.7f;
    // y is NaN
    else if ((signif_y != (0x00000000L)) && (exp_y == (0x00000080L)))
      result = y * 1.7f;
    // y is zero
    else if (abs_y == zero) {
      result = zero / zero;
      nRet = 1;
    }
    // x is zero
    else if (abs_x == zero)
      result = x;
    // x is Inf
    else if ((signif_x == (0x00000000L)) && (exp_x == (0x00000080L)))
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
    if (exp_x != 127) {
      if ((2.0f * abs_x) <= abs_y) {
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
  if ((exp_x == -127) && (signif_x != (0x00000000L))) {
    exp_x = -126;
    while (signif_x <= (0x007FFFFFL)) {
      exp_x--;
      signif_x <<= 1;
    };
  } else
    signif_x = (signif_x | (0x00800000L));
  if ((exp_y == -127) && (signif_y != (0x00000000L))) {
    exp_y = -126;
    while (signif_y <= (0x007FFFFFL)) {
      exp_y--;
      signif_y <<= 1;
    };
  } else
    signif_y = (signif_y | (0x00800000L));
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
  if (rem_bit == (0x00000000L)) {
    (*r) = (zero * x);
    return nRet;
  }
  // Adjust remainder
  rem_bit >>= 1;
  //  Set exponent base, unbiased
  j = exp_y;
  //  Calculate normalization shift
  while (rem_bit <= (0x007FFFFFL)) {
    j--;
    rem_bit <<= 1;
  };
  //  Prepare normal results
  if (j >= -126) {
    // Remove explicit 1
    rem_bit &= (0x007FFFFFL);
    // Set final exponent ... add exponent bias
    j = j + 127;
  }
  //  Prepare denormal results
  else {
    // Determine denormalization shift count
    j = -j - 126;
    // Denormalization
    rem_bit >>= j;
    // Set final exponent ... denorms are 0
    j = 0;
  }
  rem_bit = (((VUINT32)(j)) << 23) | rem_bit;
  // Create float result and adjust if >= .5 * divisor
  result = *(float *)&rem_bit;
  if ((2.0f * result) >= abs_y) {
    if ((2.0f * result) == abs_y) {
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
} /* namespace __imf_impl_remainder_s_xa */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_remainderf(float a, float b) {
  using namespace __imf_impl_remainder_s_xa;
  VUINT32 vm;
  float va1;
  float va2;
  float vr1;
  float r;
  va1 = a;
  va2 = b;
  {
    float sZero;
    float sZeroRes;
    float sHighMask;
    float sAbsMask;
    float sSignMask;
    float sYHi;
    float sYLo;
    float sQ;
    float sXSign;
    float sQYHi;
    float sQYLo;
    float sXAbs;
    float sYAbs;
    float sRes;
    float sCorr;
    VUINT32 iXExp;
    VUINT32 iYExp;
    VUINT32 iQExp;
    VUINT32 iMaxQExp;
    VUINT32 iRangeMask;
    VUINT32 iYSub;
    VUINT32 iYCmp;
    VUINT32 iYSpec;
    float sHalf;
    float sAbsRes;
    float sResSign;
    sZero = as_float(0);
    /* Absolute values */
    sAbsMask = as_float(__devicelib_imf_internal_sremainder_data._sAbsMask);
    sXAbs = as_float((as_uint(va1) & as_uint(sAbsMask)));
    sYAbs = as_float((as_uint(va2) & as_uint(sAbsMask)));
    iXExp = as_uint(sXAbs);
    iYExp = as_uint(sYAbs);
    iYSub = (__devicelib_imf_internal_sremainder_data._iYSub);
    iYCmp = (__devicelib_imf_internal_sremainder_data._iYCmp);
    iYSpec = (iYExp - iYSub);
    iYSpec = ((VUINT32)(-(VSINT32)((VSINT32)iYSpec > (VSINT32)iYCmp)));
    iXExp = ((VUINT32)(iXExp) >> (23));
    iYExp = ((VUINT32)(iYExp) >> (23));
    iQExp = (iXExp - iYExp);
    iMaxQExp = (__devicelib_imf_internal_sremainder_data._iMaxQExp);
    iRangeMask = ((VUINT32)(-(VSINT32)((VSINT32)iQExp > (VSINT32)iMaxQExp)));
    iRangeMask = (iRangeMask | iYSpec);
    vm = 0;
    vm = iRangeMask;
    /* yhi = y & highmask */
    sHighMask = as_float(__devicelib_imf_internal_sremainder_data._sHighMask);
    sYHi = as_float((as_uint(va2) & as_uint(sHighMask)));
    /* ylo = y - yhi */
    sYLo = (va2 - sYHi);
    /* q = x/y */
    sQ = (va1 / va2);
    /* iq = trunc(q) */
    sQ = __rint(sQ);
    /* yhi*iq */
    sQYHi = (sQ * sYHi);
    /* ylo*iq */
    sQYLo = (sQ * sYLo);
    /* res = x - yhi*iq */
    sRes = (va1 - sQYHi);
    /* res = res - ylo*iq */
    sRes = (sRes - sQYLo);
    /* get result's abs value and sign */
    sSignMask = as_float(__devicelib_imf_internal_sremainder_data._sSignMask);
    sResSign = as_float((as_uint(sRes) & as_uint(sSignMask)));
    sAbsRes = as_float((as_uint(sRes) & as_uint(sAbsMask)));
    sXSign = as_float((as_uint(va1) & as_uint(sSignMask)));
    /* |y|*0.5 */
    sHalf = as_float(__devicelib_imf_internal_sremainder_data._sHalf);
    sCorr = (sYAbs * sHalf);
    /* if |res| > |y|*0.5 */
    sCorr = as_float(((VUINT32)(-(VSINT32)(sAbsRes > sCorr))));
    /* then res = res - sign(res)*|y| */
    sCorr = as_float((as_uint(sCorr) & as_uint(sYAbs)));
    sCorr = as_float((as_uint(sCorr) | as_uint(sResSign)));
    sRes = (sRes - sCorr);
    sZeroRes = as_float(((VUINT32)(-(VSINT32)(sRes == sZero))));
    sZeroRes = as_float((as_uint(sZeroRes) & as_uint(sXSign)));
    vr1 = as_float((as_uint(sRes) | as_uint(sZeroRes)));
  }
  if (__builtin_expect((vm) != 0, 0)) {
    float __cout_a1;
    float __cout_a2;
    float __cout_r1;
    ((float *)&__cout_a1)[0] = va1;
    ((float *)&__cout_a2)[0] = va2;
    ((float *)&__cout_r1)[0] = vr1;
    __devicelib_imf_internal_sremainder(&__cout_a1, &__cout_a2, &__cout_r1);
    vr1 = ((const float *)&__cout_r1)[0];
  }
  r = vr1;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
