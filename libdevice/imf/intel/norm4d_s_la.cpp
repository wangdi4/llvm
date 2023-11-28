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
namespace __imf_impl_norm4d_s_la {
namespace {
typedef struct {
  VUINT32 _AbsMask;
  VUINT32 _MaxNorm;
  VUINT32 _Zero;
  VUINT32 _Inf;
  VUINT32 _Biasx2m1;
  VUINT32 _Biasx2;
  VUINT32 _Two62;
  VUINT32 _One;
  VUINT32 _ac1;
  VUINT32 _c2;
  VUINT32 _c1;
} __devicelib_imf_internal_snorm4d_data_t;
static const __devicelib_imf_internal_snorm4d_data_t
    __devicelib_imf_internal_snorm4d_data = {
        /* AbsMask */ 0x7FFFFFFFu,
        /* MaxNorm */ 0x7F7FFFFFu,
        /* Zero */ 0x00000000u,
        /* Inf */ 0x7F800000u,
        /* Biasx2m1 */ 0x7E800000u,
        /* Biasx2 */ 0x7F000000u,
        /* Two62 */ 0x40000000u,
        /* One */ 0x3F800000u,
        /* ac1 */ 0x3F000000u,
        /* c2 */ 0x3ec00006u,
        /* c1 */ 0xbf000002u,
};
} /* namespace */
} /* namespace __imf_impl_norm4d_s_la */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_norm4df(float a, float b, float c,
                                                     float _VAPI_ARG4) {
  using namespace __imf_impl_norm4d_s_la;
  VUINT32 vm;
  float va1;
  float va2;
  float va3;
  float va4;
  float vr1;
  float r;
  va1 = a;
  va2 = b;
  va3 = c;
  va4 = _VAPI_ARG4;
  {
    VUINT32 lSpecialMask;
    VUINT32 mSpecialMask;
    float AbsMask;
    float ax;
    float ay;
    float az;
    float aw;
    float asum;
    float MaxNorm;
    float Zero;
    float SpecialMask;
    float ZeroMask;
    float h12;
    float l12;
    float h34;
    float l34;
    float lt0;
    float ht3;
    float H0;
    float Inf;
    VUINT32 exponl;
    VUINT32 expon_corr;
    VUINT32 Biasx2;
    VUINT32 Two62;
    float expon0;
    float t0;
    float t1;
    float t2;
    float t3;
    float sum;
    float s3;
    float s2;
    float s1;
    float s0;
    float RS;
    float RS2;
    float poly;
    float Sh;
    float One;
    float me0;
    float m_eps;
    float ac2;
    float ac1;
    vm = 0;
    AbsMask = as_float(__devicelib_imf_internal_snorm4d_data._AbsMask);
    // absolute values
    ax = as_float((as_uint(va1) & as_uint(AbsMask)));
    ay = as_float((as_uint(va2) & as_uint(AbsMask)));
    az = as_float((as_uint(va3) & as_uint(AbsMask)));
    aw = as_float((as_uint(va4) & as_uint(AbsMask)));
    // test whether any input is Inf/NaN
    // first add absolute values
    asum = (ax + ay);
    asum = (asum + az);
    asum = (asum + aw);
    // asum <= max_norm?
    MaxNorm = as_float(__devicelib_imf_internal_snorm4d_data._MaxNorm);
    SpecialMask = as_float(((VUINT32)(-(VSINT32)(!(asum <= MaxNorm)))));
    // asum == 0?
    Zero = as_float(__devicelib_imf_internal_snorm4d_data._Zero);
    ZeroMask = as_float(((VUINT32)(-(VSINT32)(asum == Zero))));
    // mask for Inf/NaN input, or all zero
    SpecialMask = as_float((as_uint(SpecialMask) | as_uint(ZeroMask)));
    // Order inputs based on absolute value
    //	h12 = DP_FMAX(ax.f, ay.f);
    h12 = ((ax > ay) ? ax : ay);
    //  l12 = DP_FMIN(ax.f, ay.f);
    l12 = ((ax < ay) ? ax : ay);
    //  h34 = DP_FMAX(az.f, aw.f);
    h34 = ((az > aw) ? az : aw);
    //  l34 = DP_FMIN(az.f, aw.f);
    l34 = ((az < aw) ? az : aw);
    // largest input (in absolute value): t0 = SP_FMAX(h12, h34)
    t0 = ((h12 > h34) ? h12 : h34);
    // lt0 = SP_FMIN(h12, h34);
    lt0 = ((h12 < h34) ? h12 : h34);
    // smallest input (in absolute value): t3 = SP_FMIN(l12, l34)
    t3 = ((l12 < l34) ? l12 : l34);
    // ht3 = SP_FMAX(l12, l34);
    ht3 = ((l12 > l34) ? l12 : l34);
    // order inputs from largest to smallest
    t1 = ((lt0 > ht3) ? lt0 : ht3);
    t2 = ((lt0 < ht3) ? lt0 : ht3);
    // scale inputs
    // expon0.w = H0 & 0x7ff0000000000000ull;
    Inf = as_float(__devicelib_imf_internal_snorm4d_data._Inf);
    H0 = as_float((as_uint(t0) & as_uint(Inf)));
    // 2^(-expon_t0+ ((t0>=2)?1:(-1)))
    // expon0.w = 0x7e800000 + ((expon0.w & 0x40000000) >> 6)- expon0.w;
    exponl = as_uint(H0);
    Biasx2 = (__devicelib_imf_internal_snorm4d_data._Biasx2m1);
    Two62 = (__devicelib_imf_internal_snorm4d_data._Two62);
    expon_corr = (exponl & Two62);
    expon_corr = ((VUINT32)(expon_corr) >> (6));
    expon_corr = (expon_corr + Biasx2);
    expon_corr = (expon_corr - exponl);
    expon0 = as_float(expon_corr);
    // scale inputs
    t0 = (t0 * expon0);
    t1 = (t1 * expon0);
    t2 = (t2 * expon0);
    t3 = (t3 * expon0);
    // sum of squares
    s3 = (t3 * t3);
    s2 = __fma(t2, t2, s3);
    s1 = __fma(t1, t1, s2);
    sum = __fma(t0, t0, s1);
    // fsum = (float)sum_h;  frs = SP_RSQRT(fsum); RS = (double)frs;
    RS = (1.0f / __sqrt(sum));
/*
//     
//             Sh ~ sqrt(sum)
//            Sh = SP_FMA(sum, RS, 0.0f);
//            RS2 = SP_FMA(RS, 0.5f, 0.0f);
//             sum - Sh^2
//            eps = SP_FMA(Sh, -Sh, sum);
//             sqrt(sum) ~ Sh + eps*RS2
//            res.f = SP_FMA(RS2, eps, Sh); 
*/
    Sh = (sum * RS);
    ac1 = as_float(__devicelib_imf_internal_snorm4d_data._ac1); // ac1=+0.5
    RS2 = (RS * ac1);
    // eps = sum - Sh^2
    m_eps = __fma(-(Sh), Sh, sum);
    // sqrt(sum) ~ Sh + eps*RS2
    RS = __fma(RS2, m_eps, Sh);
    // expon0.w = 0x7f000000 - expon0.w;
    Biasx2 = (__devicelib_imf_internal_snorm4d_data._Biasx2);
    expon_corr = (Biasx2 - expon_corr);
    expon0 = as_float(expon_corr);
    // res.f = DP_FMA(RS, expon0.f, 0.0)
    vr1 = (RS * expon0);
    // fixup for special cases
    lSpecialMask = as_uint(SpecialMask);
    mSpecialMask = 0;
    mSpecialMask = lSpecialMask;
    if (__builtin_expect((mSpecialMask) != 0, 0)) {
      float InfMask;
      float InfMask2;
      float InfMask3;
      float InfMask4;
      Inf = as_float(__devicelib_imf_internal_snorm4d_data._Inf);
      // Will return NaN if NaN inputs, no Inf
      SpecialMask = as_float(((VUINT32)(-(VSINT32)(!(asum == asum)))));
      vr1 = as_float((((~as_uint(SpecialMask)) & as_uint(vr1)) |
                      (as_uint(SpecialMask) & as_uint(asum))));
      // check if any input is Inf
      InfMask = as_float(((VUINT32)(-(VSINT32)(MaxNorm < ax))));
      InfMask2 = as_float(((VUINT32)(-(VSINT32)(MaxNorm < ay))));
      InfMask3 = as_float(((VUINT32)(-(VSINT32)(MaxNorm < az))));
      InfMask4 = as_float(((VUINT32)(-(VSINT32)(MaxNorm < aw))));
      InfMask = as_float((as_uint(InfMask) | as_uint(InfMask2)));
      InfMask3 = as_float((as_uint(InfMask3) | as_uint(InfMask4)));
      InfMask = as_float((as_uint(InfMask) | as_uint(InfMask3)));
      /* Return Inf if one input is Inf */
      vr1 = as_float((((~as_uint(InfMask)) & as_uint(vr1)) |
                      (as_uint(InfMask) & as_uint(Inf))));
      /* Return 0 if all inputs are zero */
      vr1 = as_float((((~as_uint(ZeroMask)) & as_uint(vr1)) |
                      (as_uint(ZeroMask) & as_uint(Zero))));
    }
  }
  r = vr1;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
