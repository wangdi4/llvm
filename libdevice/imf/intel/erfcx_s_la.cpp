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
//   LA implementation
//
//   x in [0,2):  polynomial approximation of erfcx((x-1.0)+1.0)
//   For x>=2, will approximate H(x)=erfcx(x)*x;  H(x)~1/sqrt(Pi) for very large
x
//             R=1/x, use polynomial approximation for H(1/R)
//   x<0:  Compute 2*exp(x^2) - erfcx(|x|)
//
//
*/
#include "_imf_include_fp32.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_erfcx_s_la {
namespace {
typedef struct {
  VUINT32 _AbsMask;
  VUINT32 _One;
  VUINT32 _Two;
  VUINT32 _cp012;
  VUINT32 _cp011;
  VUINT32 _cp010;
  VUINT32 _cp09;
  VUINT32 _cp08;
  VUINT32 _cp07;
  VUINT32 _cp06;
  VUINT32 _cp05;
  VUINT32 _cp04;
  VUINT32 _cp03;
  VUINT32 _cp02;
  VUINT32 _cp01;
  VUINT32 _cp00;
  VUINT32 _cp19;
  VUINT32 _cp18;
  VUINT32 _cp17;
  VUINT32 _cp16;
  VUINT32 _cp15;
  VUINT32 _cp14;
  VUINT32 _cp13;
  VUINT32 _cp12;
  VUINT32 _cp11;
  VUINT32 _cp10;
  VUINT32 _cp10h;
  VUINT32 _Zero;
  VUINT32 _L2E;
  VUINT32 _Shifter;
  VUINT32 _L2H;
  VUINT32 _L2L;
  VUINT32 _ce6;
  VUINT32 _ce5;
  VUINT32 _ce4;
  VUINT32 _ce3;
  VUINT32 _ce2;
  VUINT32 _ce1;
  VUINT32 _ce0;
  VUINT32 _Smax;
  VUINT32 _Inf;
  VUINT32 _HMask;
} __devicelib_imf_internal_serfcx_data_t;
static const __devicelib_imf_internal_serfcx_data_t
    __devicelib_imf_internal_serfcx_data = {
        /* AbsMask */ 0x7FFFFFFFu,
        /* One */ 0x3F800000u,
        /* Two */ 0x40000000u,
        /* __cp012 */ 0x37a106b5u,
        /* __cp011 */ 0xb884ad7au,
        /* __cp010 */ 0x39080063u,
        /* __cp09 */ 0xb9bbdf7du,
        /* __cp08 */ 0x3a89c274u,
        /* __cp07 */ 0xbb3668a2u,
        /* __cp06 */ 0x3be46aacu,
        /* __cp05 */ 0xbc8875d5u,
        /* __cp04 */ 0x3d19e520u,
        /* __cp03 */ 0xbda24203u,
        /* __cp02 */ 0x3e1e1396u,
        /* __cp01 */ 0xbe8be271u,
        /* __cp00 */ 0x3edaec3cu,
        /* __cp9 */ 0x3f806f08u,
        /* __cp8 */ 0xc04175e9u,
        /* __cp7 */ 0x406e4be1u,
        /* __cp6 */ 0xc00857e3u,
        /* __cp5 */ 0x3e3e70e5u,
        /* __cp4 */ 0x3ece9855u,
        /* __cp3 */ 0x3a9d2157u,
        /* __cp2 */ 0xbe9073a2u,
        /* __cp1 */ 0x34f43978u,
        /* __cp0 */ 0x32fc4fe5u,
        /* __cp0h */ 0x3f106ebau,
        /* Zero */ 0x00000000u,
        /* __L2E */ 0x3FB8AA3Bu,
        /* __Shifter */ 0x4b40007fu,
        /* __L2H */ 0x3f317200u,
        /* __L2L */ 0x35BFBE8Eu,
        /* __ce6 */ 0x3ab6ecc1u,
        /* __ce5 */ 0x3c0937d6u,
        /* __ce4 */ 0x3d2aaa0eu,
        /* __ce3 */ 0x3e2aaa02u,
        /* __ce2 */ 0x3f000000u,
        /* __ce1 */ 0x3f800000u,
        /* __ce0 */ 0xaeb1f936u,
        0x41161E5Fu, /* Smax */
        0x7f800000u, /* Inf */
        0xFFFFF000u, /* high mask  */
};                   /*dErfcx_Table*/
inline int __devicelib_imf_internal_serfcx(const float *a, float *r) {
  int nRet = 0;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_erfcx_s_la */
DEVICE_EXTERN_C_INLINE float __devicelib_imf_erfcxf(float x) {
  using namespace __imf_impl_erfcx_s_la;
  float r;
  VUINT32 vm;
  float va1;
  float vr1;
  va1 = x;
  {
    float AbsMask;
    float sgn_x;
    float xa;
    float One;
    VUINT32 ltTwo;
    float Two;
    float aR;
    float bR;
    float apoly;
    float bpoly;
    float cp0[23];
    float cp1[23];
    float cp1h;
    float Zero;
    float dNegMask;
    VUINT32 lNegMask;
    VUINT32 mNegMask;
    vm = 0;
    // |x|
    AbsMask = as_float(__devicelib_imf_internal_serfcx_data._AbsMask);
    xa = as_float((as_uint(va1) & as_uint(AbsMask)));
    // sign(x)
    sgn_x = as_float((as_uint(va1) ^ as_uint(xa)));
    // reduced argument for x in [0,2)
    One = as_float(__devicelib_imf_internal_serfcx_data._One);
    aR = (xa - One);
    // reduced argument for x >= 2
    bR = (One / xa);
    Two = as_float(__devicelib_imf_internal_serfcx_data._Two);
    // equivalent except for NaNs; this format may help performance in some
    // cases
    ltTwo = (xa < Two) ? 0x1L : 0x0L;
    // Start polynomial evaluations for both ranges
    cp0[12] = as_float(__devicelib_imf_internal_serfcx_data._cp012);
    cp0[11] = as_float(__devicelib_imf_internal_serfcx_data._cp011);
    cp1[9] = as_float(__devicelib_imf_internal_serfcx_data._cp19);
    cp1[8] = as_float(__devicelib_imf_internal_serfcx_data._cp18);
    apoly = __fma(aR, cp0[12], cp0[11]);
    bpoly = __fma(bR, cp1[9], cp1[8]);
    cp0[10] = as_float(__devicelib_imf_internal_serfcx_data._cp010);
    cp1[7] = as_float(__devicelib_imf_internal_serfcx_data._cp17);
    apoly = __fma(apoly, aR, cp0[10]);
    bpoly = __fma(bpoly, bR, cp1[7]);
    cp0[9] = as_float(__devicelib_imf_internal_serfcx_data._cp09);
    cp1[6] = as_float(__devicelib_imf_internal_serfcx_data._cp16);
    apoly = __fma(apoly, aR, cp0[9]);
    bpoly = __fma(bpoly, bR, cp1[6]);
    cp0[8] = as_float(__devicelib_imf_internal_serfcx_data._cp08);
    cp1[5] = as_float(__devicelib_imf_internal_serfcx_data._cp15);
    apoly = __fma(apoly, aR, cp0[8]);
    bpoly = __fma(bpoly, bR, cp1[5]);
    cp0[7] = as_float(__devicelib_imf_internal_serfcx_data._cp07);
    cp1[4] = as_float(__devicelib_imf_internal_serfcx_data._cp14);
    apoly = __fma(apoly, aR, cp0[7]);
    bpoly = __fma(bpoly, bR, cp1[4]);
    cp0[6] = as_float(__devicelib_imf_internal_serfcx_data._cp06);
    cp1[3] = as_float(__devicelib_imf_internal_serfcx_data._cp13);
    apoly = __fma(apoly, aR, cp0[6]);
    bpoly = __fma(bpoly, bR, cp1[3]);
    cp0[5] = as_float(__devicelib_imf_internal_serfcx_data._cp05);
    cp1[2] = as_float(__devicelib_imf_internal_serfcx_data._cp12);
    apoly = __fma(apoly, aR, cp0[5]);
    bpoly = __fma(bpoly, bR, cp1[2]);
    cp0[4] = as_float(__devicelib_imf_internal_serfcx_data._cp04);
    cp1[1] = as_float(__devicelib_imf_internal_serfcx_data._cp11);
    apoly = __fma(apoly, aR, cp0[4]);
    bpoly = __fma(bpoly, bR, cp1[1]);
    cp0[3] = as_float(__devicelib_imf_internal_serfcx_data._cp03);
    cp1[0] = as_float(__devicelib_imf_internal_serfcx_data._cp10);
    apoly = __fma(apoly, aR, cp0[3]);
    bpoly = __fma(bpoly, bR, cp1[0]);
    cp0[2] = as_float(__devicelib_imf_internal_serfcx_data._cp02);
    apoly = __fma(apoly, aR, cp0[2]);
    cp0[1] = as_float(__devicelib_imf_internal_serfcx_data._cp01);
    apoly = __fma(apoly, aR, cp0[1]);
    cp0[0] = as_float(__devicelib_imf_internal_serfcx_data._cp00);
    apoly = __fma(apoly, aR, cp0[0]);
    bpoly = (bpoly * bR);
    cp1h = as_float(__devicelib_imf_internal_serfcx_data._cp10h);
    bpoly = __fma(bR, cp1h, bpoly);
    vr1 = (ltTwo != 0x0) ? (apoly) : (bpoly);
    // negative input?
    Zero = as_float(__devicelib_imf_internal_serfcx_data._Zero);
    dNegMask = as_float(((VUINT32)(-(VSINT32)(va1 < Zero))));
    lNegMask = as_uint(dNegMask);
    mNegMask = 0;
    mNegMask = lNegMask;
    if (__builtin_expect((mNegMask) != 0, 0)) {
      float x2h;
      float x2l;
      float Sf;
      float Nf;
      float L2E;
      float L2H;
      float L2L;
      float Shifter;
      float poly;
      float R1;
      float Te;
      float ce6;
      float ce5;
      float ce4;
      float ce3;
      float ce2;
      float ce1;
      float ce0;
      float Two;
      float R;
      float res;
      float Smax;
      float Inf;
      VUINT32 OFMask;
      // x2h = DP_FMA(xa.f, xa.f, 0.0);
      x2h = (xa * xa);
      // x2l = DP_FMA(xa.f, xa.f, -x2h);
      x2l = __fma(xa, xa, -(x2h));
      // Sf = x2h*L2E + Shifter
      L2E = as_float(__devicelib_imf_internal_serfcx_data._L2E);
      Shifter = as_float(__devicelib_imf_internal_serfcx_data._Shifter);
      Sf = __fma(x2h, L2E, Shifter);
      // Nf = (int)(x2h*L2E)
      Nf = (Sf - Shifter);
      // R+Rl = x^2 - N*log(2)
      // SSE2 values
      L2H = as_float(__devicelib_imf_internal_serfcx_data._L2H);
      R = __fma(-(Nf), L2H, x2h);
      L2L = as_float(__devicelib_imf_internal_serfcx_data._L2L);
      R1 = __fma(-(Nf), L2L, x2l);
      R = (R + R1);
      // 2^(N) = Te.w = S.w << 52;
      Te = as_float(((VUINT32)as_uint(Sf) << (23)));
      // exp(R)-1
      ce6 = as_float(__devicelib_imf_internal_serfcx_data._ce6);
      ce5 = as_float(__devicelib_imf_internal_serfcx_data._ce5);
      ce4 = as_float(__devicelib_imf_internal_serfcx_data._ce4);
      ce3 = as_float(__devicelib_imf_internal_serfcx_data._ce3);
      ce2 = as_float(__devicelib_imf_internal_serfcx_data._ce2);
      ce1 = as_float(__devicelib_imf_internal_serfcx_data._ce1);
      ce0 = as_float(__devicelib_imf_internal_serfcx_data._ce0);
      poly = __fma(R, ce6, ce5);
      poly = __fma(poly, R, ce4);
      poly = __fma(poly, R, ce3);
      poly = __fma(poly, R, ce2);
      poly = __fma(poly, R, ce1);
      poly = __fma(poly, R, ce0);
      // 2^(N)*exp(R)
      // SP_FMA(poly.f, Te.f, Te.f);
      poly = __fma(poly, Te, Te);
      // res = 2^(N+1)*exp(R) - res  (i.e exp(x^2)*2-erfcx(|x|))
      Two = as_float(__devicelib_imf_internal_serfcx_data._Two);
      res = __fma(poly, Two, -(vr1));
      // fixup for overflow
      // res.f = (S.f >= _VSTATIC(__Smax).f) ? _VSTATIC(__Inf).f : res.f;
      // check whether |x| > overflow_threshold;  x<0 here
      Smax = as_float(__devicelib_imf_internal_serfcx_data._Smax);
      Inf = as_float(__devicelib_imf_internal_serfcx_data._Inf);
      OFMask = (!(xa <= Smax)) ? 0x1L : 0x0L;
      res = (OFMask != 0x0) ? (Inf) : (res);
      /* Merge results from main path and large arguments path */
      vr1 = as_float((((~as_uint(dNegMask)) & as_uint(vr1)) |
                      (as_uint(dNegMask) & as_uint(res))));
    }
  }
  if (__builtin_expect((vm) != 0, 0)) {
    float __cout_a1;
    float __cout_r1;
    ((float *)&__cout_a1)[0] = va1;
    ((float *)&__cout_r1)[0] = vr1;
    __devicelib_imf_internal_serfcx(&__cout_a1, &__cout_r1);
    vr1 = ((const float *)&__cout_r1)[0];
  }
  r = vr1;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
