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
//      xsign = x & signmask
//     
//      yhi = y & highmask
//      ylo = y - yhi
//     
//      q = x/y
//     
//      change sign for y if x is negative
//     
//      iq = trunc(q)
//      res = |x| - yhi*iq - ylo*iq
//      if res < 0 then corr = |y| else corr = 0
//      res = res + corr
//      res = res | xsign
//     --
//     
*/
#include "_imf_include_fp32.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_fmod_s_xa {
namespace {
typedef struct {
  VUINT32 _sHighMask;
  VUINT32 _sAbsMask;
  VUINT32 _sSignMask;
  VUINT32 _iMaxQExp;
  VUINT32 _iYSub;
  VUINT32 _iYCmp;
  VUINT32 _sMaxYExp;
  VUINT32 _sMaxQExp;
} __devicelib_imf_internal_sfmod_data_t;
static const __devicelib_imf_internal_sfmod_data_t
    __devicelib_imf_internal_sfmod_data = {
        0xfffff000u, /* _sHighMask  */
        0x7fffffffu, /* _sAbsMask */
        0x80000000u, /* _sSignMask */
        0xBu,        /* _iMaxQExp = 11*/
/*
//         
//             To prevent YLow to be denormal it should be checked
//             that Exp(Y) <= -127+23 (worst case when only last bit is non zero)
//             Exp(Y) < -103 -> Y < 0x0C000000
//             That value is used to construct SubConst by setting up first bit
//            to 1. CmpConst is get from max acceptable value 0x7e8fffff:
//             0x7e8fffff - 0x8C000000 = 0x(1)ED7FFFFF
//         
*/
        0x8C000000u, /* _iSubConst */
        0xF28FFFFFu, /* _iCmpConst */
        0x42FC0000u, /* _sMaxYExp */
        0x41300000u, /* _sMaxQExp */
};
static const float _libm_zero_ = 0.0;
static const float _libm_one_ = 1.0;
static const int _libm_2_to_n31 = 0x30000000;
static const int _libm_2_to_p31_m1 = 0x30000000;
inline int __devicelib_imf_internal_sfmod(const float *a, const float *b,
                                          float *r) {
  int nRet = 0;
  VUINT32 signif_x, signif_y, rem_bit, quo_bit, tmp_x;
  VUINT32 tmp_y, n_int, tmp_hold, signbit_x, tmp_n_float;
  VSINT32 exp_x, exp_y, i, j, int_32;
  VSINT32 int_64;
  VUINT32 expabs_diff, cw;
  float result, abs_x, abs_y, n_float;
  float x = (*a), y = (*b);
  // Remove sign bits
  tmp_x = (*(int *)&x);
  tmp_y = ((*(int *)&y)) & (0x7FFFFFFFL);
  signbit_x = tmp_x & (0x80000000L);
  tmp_x = tmp_x & (0x7FFFFFFFL);
  // If abs of y == 1
  if (tmp_y == (0x3F800000L)) {
    if (tmp_x <= tmp_y) {
      if (tmp_x == tmp_y) {
        (*r) = (x * _libm_zero_);
        return nRet;
      } else {
        (*r) = (x * _libm_one_);
        return nRet;
      }
    }
    // 1 < x < 2^23
    else if (tmp_x < (0x4B000000L)) {
      tmp_hold = tmp_x - tmp_y;
      tmp_hold = (23 - (tmp_hold >> 23));
      tmp_x = (tmp_x >> tmp_hold) << tmp_hold;
      tmp_x = signbit_x | tmp_x;
      result = x - *(float *)&tmp_x;
      // Correct the sign of the result with +/- ZERO is returned
      tmp_x = *(VUINT32 *)&result | signbit_x;
      result = *(float *)&tmp_x;
      (*r) = (result);
      return nRet;
    }
    // x is normal integer
    else if (tmp_x < (0x7F800000L)) {
      result = x * _libm_zero_;
      (*r) = (result);
      return nRet;
    }
  }
  // Raise denormal operand ... allow daz to take affect
  x *= _libm_one_;
  y *= _libm_one_;
  // x is NaN
  if (tmp_x > (0x7F800000L)) {
    result = x * y;
    (*r) = (result);
    return nRet;
  }
  // y is NaN
  else if (tmp_y > (0x7F800000L)) {
    result = y * x;
    (*r) = (result);
    return nRet;
  }
  // y is zero
  else if (tmp_y == (0x00000000L)) {
    result = _libm_zero_ / _libm_zero_;
    nRet = 2;
    (*r) = (result);
    return nRet;
  }
  // x is Inf
  else if (tmp_x == (0x7F800000L)) {
    result = _libm_zero_ / _libm_zero_;
    nRet = 2;
    (*r) = (result);
    return nRet;
  }
  // y is Inf
  else if (tmp_y == (0x7F800000L)) {
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
  abs_x = *(float *)&tmp_x;
  // Get float absolute values
  abs_y = *(float *)&tmp_y;
  // Remove exponent bias
  exp_x = (VSINT32)((tmp_x & (0x7FF00000L)) >> 23) - 127;
  exp_y = (VSINT32)((tmp_y & (0x7FF00000L)) >> 23) - 127;
  // Get significands
  signif_x = (tmp_x & (0x007FFFFFL));
  signif_y = (tmp_y & (0x007FFFFFL));
  // Check for denormal x and y, adjust and normalize
  if ((exp_x == -127) && (signif_x != 0L)) {
    exp_x = -126;
    while (signif_x <= (0x007FFFFFL)) {
      exp_x--;
      signif_x <<= 1;
    };
  } else
    signif_x = (signif_x | (0x00800000L));
  if ((exp_y == -127) && (signif_y != 0L)) {
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
  // Zero fmod ... return immediately with sign of x
  if (rem_bit == (0x00000000L)) {
    (*r) = (_libm_zero_ * x);
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
  // Possibly do one more correction
  if (rem_bit >= tmp_y)
    result = result - y;
  // Final adjust for sign of input
  if (signbit_x)
    result = -result;
  (*r) = (result);
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_fmod_s_xa */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_fmodf(float a, float b) {
  using namespace __imf_impl_fmod_s_xa;
  VUINT32 vm;
  float va1;
  float va2;
  float vr1;
  float r;
  va1 = a;
  va2 = b;
  {
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
    float sZero;
    VUINT32 iXExp;
    VUINT32 iYExp;
    VUINT32 iQExp;
    VUINT32 iMaxQExp;
    VUINT32 iRangeMask;
    VUINT32 iYSub;
    VUINT32 iYCmp;
    VUINT32 iYSpec;
    float sXExp;
    float sYExp;
    float sMaxYExp;
    float sAbsYExp;
    float sQExp;
    float sMaxQExp;
    float sRangeMask;
    float sYSpec;
    float sX;
    float sY;
    float sRcp;
    float sE;
    float sOne;
    sZero = as_float(0);
    /* Absolute values */
    sAbsMask = as_float(__devicelib_imf_internal_sfmod_data._sAbsMask);
    sXAbs = as_float((as_uint(va1) & as_uint(sAbsMask)));
    sYAbs = as_float((as_uint(va2) & as_uint(sAbsMask)));
    iXExp = as_uint(sXAbs);
    iYExp = as_uint(sYAbs);
    iYSub = (__devicelib_imf_internal_sfmod_data._iYSub);
    iYCmp = (__devicelib_imf_internal_sfmod_data._iYCmp);
    iYSpec = (iYExp - iYSub);
    iYSpec = ((VUINT32)(-(VSINT32)((VSINT32)iYSpec > (VSINT32)iYCmp)));
    iXExp = ((VUINT32)(iXExp) >> (23));
    iYExp = ((VUINT32)(iYExp) >> (23));
    iQExp = (iXExp - iYExp);
    iMaxQExp = (__devicelib_imf_internal_sfmod_data._iMaxQExp);
    iRangeMask = ((VUINT32)(-(VSINT32)((VSINT32)iQExp > (VSINT32)iMaxQExp)));
    iRangeMask = (iRangeMask | iYSpec);
    vm = 0;
    vm = iRangeMask;
    sYHi = va2;
    /* q = x/y */
    sQ = (va1 / va2);
    /* xsign = x & signmask */
    sSignMask = as_float(__devicelib_imf_internal_sfmod_data._sSignMask);
    sXSign = as_float((as_uint(va1) & as_uint(sSignMask)));
    /* change sign for y if x is negative */
    sYHi = as_float((as_uint(sYHi) ^ as_uint(sXSign)));
    /* iq = trunc(q) */
    sQ = __trunc(sQ);
    sRes = __fma(-(sQ), sYHi, sXAbs);
    /* if res < 0 */
    sCorr = as_float(((VUINT32)(-(VSINT32)(sRes < sZero))));
    /* corr = |y|, else corr = 0 */
    sCorr = as_float((as_uint(sCorr) & as_uint(sYAbs)));
    /* res = res + corr */
    sRes = (sRes + sCorr);
    /* res = res | xsign */
    vr1 = as_float((as_uint(sRes) | as_uint(sXSign)));
  }
  if (__builtin_expect((vm) != 0, 0)) {
    float __cout_a1;
    float __cout_a2;
    float __cout_r1;
    ((float *)&__cout_a1)[0] = va1;
    ((float *)&__cout_a2)[0] = va2;
    ((float *)&__cout_r1)[0] = vr1;
    __devicelib_imf_internal_sfmod(&__cout_a1, &__cout_a2, &__cout_r1);
    vr1 = ((const float *)&__cout_r1)[0];
  }
  r = vr1;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
