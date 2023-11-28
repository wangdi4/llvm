/*******************************************************************************
* INTEL CONFIDENTIAL
* Copyright 1996 Intel Corporation.
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
namespace __imf_impl_rhypot_s_la {
namespace {
typedef struct {
  VUINT32 _AbsMask;
  VUINT32 _MaxNorm;
  VUINT32 _Zero;
  VUINT32 _Inf;
  VUINT32 _Biasx2;
  VUINT32 _Two62;
  VUINT32 _One;
  VUINT32 _ac1;
  VUINT32 _c2;
  VUINT32 _c1;
} __devicelib_imf_internal_srhypot_data_t;
static const __devicelib_imf_internal_srhypot_data_t
    __devicelib_imf_internal_srhypot_data = {
        /* AbsMask */ 0x7FFFFFFFu,
        /* MaxNorm */ 0x7F7FFFFFu,
        /* Zero */ 0x00000000u,
        /* Inf */ 0x7F800000u,
        /* Biasx2 */ 0x7F000000u,
        /* Two62 */ 0x40000000u,
        /* One */ 0x3F800000u,
        /* ac1 */ 0xBF000000u,
        /* c2 */ 0x3ec00006u,
        /* c1 */ 0xbf000002u,
};
} /* namespace */
} /* namespace __imf_impl_rhypot_s_la */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_rhypotf(float a, float b) {
  using namespace __imf_impl_rhypot_s_la;
  VUINT32 vm;
  float va1;
  float va2;
  float vr1;
  float r;
  va1 = a;
  va2 = b;
  {
    VUINT32 lSpecialMask;
    VUINT32 mSpecialMask;
    float AbsMask;
    float ax;
    float ay;
    float asum;
    float MaxNorm;
    float Zero;
    float SpecialMask;
    float ZeroMask;
    float h12;
    float l12;
    float H0;
    float Inf;
    VUINT32 exponl;
    VUINT32 expon_corr;
    VUINT32 Biasx2;
    VUINT32 Two62;
    float expon0;
    float t0;
    float t1;
    float sum;
    float s0;
    float s1;
    float RS;
    vm = 0;
    AbsMask = as_float(__devicelib_imf_internal_srhypot_data._AbsMask);
    // absolute values
    ax = as_float((as_uint(va1) & as_uint(AbsMask)));
    ay = as_float((as_uint(va2) & as_uint(AbsMask)));
    // test whether any input is Inf/NaN
    // first add absolute values
    asum = (ax + ay);
    // asum <= max_norm?
    MaxNorm = as_float(__devicelib_imf_internal_srhypot_data._MaxNorm);
    SpecialMask = as_float(((VUINT32)(-(VSINT32)(!(asum <= MaxNorm)))));
    // asum == 0?
    Zero = as_float(__devicelib_imf_internal_srhypot_data._Zero);
    ZeroMask = as_float(((VUINT32)(-(VSINT32)(asum == Zero))));
    // mask for Inf/NaN input, or all zero
    SpecialMask = as_float((as_uint(SpecialMask) | as_uint(ZeroMask)));
    // Order inputs based on absolute value
    //	h12 = DP_FMAX(ax.f, ay.f);
    h12 = ((ax > ay) ? ax : ay);
    //  l12 = DP_FMIN(ax.f, ay.f);
    l12 = ((ax < ay) ? ax : ay);
    // scale inputs
    // expon0.w = H0 & 0x7ff0000000000000ull;
    Inf = as_float(__devicelib_imf_internal_srhypot_data._Inf);
    H0 = as_float((as_uint(h12) & as_uint(Inf)));
    // 2^(-expon_t0+ ((t0>=2)?1:0))
    // expon0.w = 0x7f000000 + ((expon0.w & 0x40000000) >> 7)- expon0.w;
    exponl = as_uint(H0);
    Biasx2 = (__devicelib_imf_internal_srhypot_data._Biasx2);
    Two62 = (__devicelib_imf_internal_srhypot_data._Two62);
    expon_corr = (exponl & Two62);
    expon_corr = ((VUINT32)(expon_corr) >> (7));
    expon_corr = (expon_corr + Biasx2);
    expon_corr = (expon_corr - exponl);
    expon0 = as_float(expon_corr);
    // scale inputs
    t0 = (h12 * expon0);
    t1 = (l12 * expon0);
    // sum of squares
    s1 = (t1 * t1);
    sum = __fma(t0, t0, s1);
    // fsum = (float)sum_h;  frs = SP_RSQRT(fsum); RS = (double)frs;
    RS = (1.0f / __sqrt(sum));
    // res.f = DP_FMA(RS, expon0.f, 0.0)
    vr1 = (RS * expon0);
    // fixup for special cases
    lSpecialMask = as_uint(SpecialMask);
    mSpecialMask = 0;
    mSpecialMask = lSpecialMask;
    if (__builtin_expect((mSpecialMask) != 0, 0)) {
      float InfMask;
      float InfMask2;
      Inf = as_float(__devicelib_imf_internal_srhypot_data._Inf);
      // Will return NaN if NaN inputs, no Inf
      SpecialMask = as_float(((VUINT32)(-(VSINT32)(!(asum == asum)))));
      vr1 = as_float((((~as_uint(SpecialMask)) & as_uint(vr1)) |
                      (as_uint(SpecialMask) & as_uint(asum))));
      // check if any input is Inf
      InfMask = as_float(((VUINT32)(-(VSINT32)(MaxNorm < ax))));
      InfMask2 = as_float(((VUINT32)(-(VSINT32)(MaxNorm < ay))));
      InfMask = as_float((as_uint(InfMask) | as_uint(InfMask2)));
      /* Return zero if one input is infinity */
      vr1 = as_float((((~as_uint(InfMask)) & as_uint(vr1)) |
                      (as_uint(InfMask) & as_uint(Zero))));
      /* Return Inf if all inputs are zero */
      vr1 = as_float((((~as_uint(ZeroMask)) & as_uint(vr1)) |
                      (as_uint(ZeroMask) & as_uint(Inf))));
    }
  }
  r = vr1;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
