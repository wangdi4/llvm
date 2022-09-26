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
#include "_imf_include_fp64.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_erfcx_d_la {
namespace {
typedef struct {
  VUINT64 _AbsMask;
  VUINT64 _One;
  VUINT64 _Two;
  VUINT64 _cp022;
  VUINT64 _cp021;
  VUINT64 _cp020;
  VUINT64 _cp019;
  VUINT64 _cp018;
  VUINT64 _cp017;
  VUINT64 _cp016;
  VUINT64 _cp015;
  VUINT64 _cp014;
  VUINT64 _cp013;
  VUINT64 _cp012;
  VUINT64 _cp011;
  VUINT64 _cp010;
  VUINT64 _cp09;
  VUINT64 _cp08;
  VUINT64 _cp07;
  VUINT64 _cp06;
  VUINT64 _cp05;
  VUINT64 _cp04;
  VUINT64 _cp03;
  VUINT64 _cp02;
  VUINT64 _cp01;
  VUINT64 _cp00;
  VUINT64 _cp122;
  VUINT64 _cp121;
  VUINT64 _cp120;
  VUINT64 _cp119;
  VUINT64 _cp118;
  VUINT64 _cp117;
  VUINT64 _cp116;
  VUINT64 _cp115;
  VUINT64 _cp114;
  VUINT64 _cp113;
  VUINT64 _cp112;
  VUINT64 _cp111;
  VUINT64 _cp110;
  VUINT64 _cp19;
  VUINT64 _cp18;
  VUINT64 _cp17;
  VUINT64 _cp16;
  VUINT64 _cp15;
  VUINT64 _cp14;
  VUINT64 _cp13;
  VUINT64 _cp12;
  VUINT64 _cp11;
  VUINT64 _cp10;
  VUINT64 _cp10h;
  VUINT64 _Zero;
  VUINT64 _L2E;
  VUINT64 _Shifter;
  VUINT64 _L2H;
  VUINT64 _L2L;
  VUINT64 _ce11;
  VUINT64 _ce10;
  VUINT64 _ce9;
  VUINT64 _ce8;
  VUINT64 _ce7;
  VUINT64 _ce6;
  VUINT64 _ce5;
  VUINT64 _ce4;
  VUINT64 _ce3;
  VUINT64 _ce2;
  VUINT64 _ce1;
  VUINT64 _Smax;
  VUINT64 _Inf;
  VUINT64 _HMask;
} __devicelib_imf_internal_derfcx_data_t;
static const __devicelib_imf_internal_derfcx_data_t
    __devicelib_imf_internal_derfcx_data = {
        /* AbsMask */ 0x7FFFFFFFFFFFFFFFuL,
        /* One */ 0x3FF0000000000000uL,
        /* Two */ 0x4000000000000000uL,
        /* __cp022 */ 0x3dd246505f3689e0uL,
        /* __cp021 */ 0xbdf2fda03640b3cduL,
        /* __cp020 */ 0x3e07c5e45b7bae89uL,
        /* __cp019 */ 0xbe257e7ca71fa31buL,
        /* __cp018 */ 0x3e4592e7c3fda873uL,
        /* __cp017 */ 0xbe63778d35674d2euL,
        /* __cp016 */ 0x3e80e9deb3f56b61uL,
        /* __cp015 */ 0xbe9cef11c0c7f533uL,
        /* __cp014 */ 0x3eb8274b6f06add9uL,
        /* __cp013 */ 0xbed39c76b6915163uL,
        /* __cp012 */ 0x3eeef08e7eb82100uL,
        /* __cp011 */ 0xbf07ab1afe5b4a8euL,
        /* __cp010 */ 0x3f2184fc29bd0f13uL,
        /* __cp09 */ 0xbf39082246483e26uL,
        /* __cp08 */ 0x3f5135262eb4ac05uL,
        /* __cp07 */ 0xbf66af265490f481uL,
        /* __cp06 */ 0x3f7c8cb958c5052cuL,
        /* __cp05 */ 0xbf910fcf1b5668f7uL,
        /* __cp04 */ 0x3fa33cad0ef5f11duL,
        /* __cp03 */ 0xbfb44837f8906dcbuL,
        /* __cp02 */ 0x3fc3c27283c32cb8uL,
        /* __cp01 */ 0xbfd17c4e3f17c052uL,
        /* __cp00 */ 0x3fdb5d8780f956b2uL,
        /* __cp22 */ 0xc0ac276c3d1db6c3uL,
        /* __cp21 */ 0x40d4df49bd7ac3f0uL,
        /* __cp20 */ 0xc0ecd227966a1099uL,
        /* __cp19 */ 0x40f887b9efd9ed0duL,
        /* __cp18 */ 0xc0fcaca638f55e37uL,
        /* __cp17 */ 0x40f8374d75394b22uL,
        /* __cp16 */ 0xc0ee28054df30e06uL,
        /* __cp15 */ 0x40db9213cfc9a317uL,
        /* __cp14 */ 0xc0c1e1dc3c0770c5uL,
        /* __cp13 */ 0x409e566f466330e8uL,
        /* __cp12 */ 0xc06ea09aa733280auL,
        /* __cp11 */ 0x4046671557785e22uL,
        /* __cp10 */ 0xc03530fb954cc684uL,
        /* __cp9 */ 0x3fd5a70649b7fce4uL,
        /* __cp8 */ 0x400d7acca9d7915cuL,
        /* __cp7 */ 0x3f41522846bb5367uL,
        /* __cp6 */ 0xbff0ecfb04e94f14uL,
        /* __cp5 */ 0xbea5ed7d5f3b4beduL,
        /* __cp4 */ 0x3fdb14c316dcd6c9uL,
        /* __cp3 */ 0xbe036213f47795a8uL,
        /* __cp2 */ 0xbfd20dd750412359uL,
        /* __cp1 */ 0xbd15a264a54b1e56uL,
        /* __cp0 */ 0x3c75E79ED14C7A19uL,
        /* __cp0h */ 0x3fe20dd750429b6duL,
        /* Zero */ 0x0000000000000000uL,
        /* __L2E */ 0x3ff71547652B82FEuL,
        /* __Shifter */ 0x43380000000003ffuL,
        /* __L2H */ 0x3fe62E42FEFA4000uL,
        /* __L2L */ 0xbd48432A1B0E2634uL,
        /* __ce11 */ 0x3e5af8da0090dd28uL,
        /* __ce10 */ 0x3e928b4062a7f01fuL,
        /* __ce9 */ 0x3ec71ddd95f14ecauL,
        /* __ce8 */ 0x3efa01991abb9723uL,
        /* __ce7 */ 0x3f2a01a01bec1aceuL,
        /* __ce6 */ 0x3f56c16c187fc36auL,
        /* __ce5 */ 0x3f8111111110c4c4uL,
        /* __ce4 */ 0x3fa555555554f0cduL,
        /* __ce3 */ 0x3fc555555555556auL,
        /* __ce2 */ 0x3fe0000000000011uL,
        /* __ce1 */ 0x3ff0000000000000uL,
        /* __Smax */ 0x403AA0F4D2E063CEuL,
        /* __Inf */ 0x7ff0000000000000uL,
        /* HMask  */ 0xFFFFFFFFF8000000uL,
}; /*dErfcx_Table*/
inline int __devicelib_imf_internal_derfcx(const double *a, double *r) {
  int nRet = 0;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_erfcx_d_la */
DEVICE_EXTERN_C_INLINE double __devicelib_imf_erfcx(double x) {
  using namespace __imf_impl_erfcx_d_la;
  double r;
  VUINT32 vm;
  double va1;
  double vr1;
  va1 = x;
  {
    ///////////////////////////////////////////////////////////////////////////////////////////
    double AbsMask;
    double sgn_x;
    double xa;
    double One;
    VUINT64 ltTwo;
    double Two;
    double aR;
    double bR;
    double apoly;
    double bpoly;
    double cp0[23];
    double cp1[23];
    double cp1h;
    double Zero;
    double dNegMask;
    VUINT64 lNegMask;
    VUINT32 mNegMask;
    vm = 0;
    // |x|
    AbsMask = as_double(__devicelib_imf_internal_derfcx_data._AbsMask);
    xa = as_double((as_ulong(va1) & as_ulong(AbsMask)));
    // sign(x)
    sgn_x = as_double((as_ulong(va1) ^ as_ulong(xa)));
    // reduced argument for x in [0,2)
    One = as_double(__devicelib_imf_internal_derfcx_data._One);
    aR = (xa - One);
    // reduced argument for x >= 2
    bR = (One / xa);
    Two = as_double(__devicelib_imf_internal_derfcx_data._Two);
    // equivalent except for NaNs; this format may help performance in some
    // cases
    ltTwo = (xa < Two) ? 0x1L : 0x0L;
    // Start polynomial evaluations for both ranges
    cp0[22] = as_double(__devicelib_imf_internal_derfcx_data._cp022);
    cp0[21] = as_double(__devicelib_imf_internal_derfcx_data._cp021);
    cp1[22] = as_double(__devicelib_imf_internal_derfcx_data._cp122);
    cp1[21] = as_double(__devicelib_imf_internal_derfcx_data._cp121);
    apoly = __fma(aR, cp0[22], cp0[21]);
    bpoly = __fma(bR, cp1[22], cp1[21]);
    cp0[20] = as_double(__devicelib_imf_internal_derfcx_data._cp020);
    cp1[20] = as_double(__devicelib_imf_internal_derfcx_data._cp120);
    apoly = __fma(apoly, aR, cp0[20]);
    bpoly = __fma(bpoly, bR, cp1[20]);
    cp0[19] = as_double(__devicelib_imf_internal_derfcx_data._cp019);
    cp1[19] = as_double(__devicelib_imf_internal_derfcx_data._cp119);
    apoly = __fma(apoly, aR, cp0[19]);
    bpoly = __fma(bpoly, bR, cp1[19]);
    cp0[18] = as_double(__devicelib_imf_internal_derfcx_data._cp018);
    cp1[18] = as_double(__devicelib_imf_internal_derfcx_data._cp118);
    apoly = __fma(apoly, aR, cp0[18]);
    bpoly = __fma(bpoly, bR, cp1[18]);
    cp0[17] = as_double(__devicelib_imf_internal_derfcx_data._cp017);
    cp1[17] = as_double(__devicelib_imf_internal_derfcx_data._cp117);
    apoly = __fma(apoly, aR, cp0[17]);
    bpoly = __fma(bpoly, bR, cp1[17]);
    cp0[16] = as_double(__devicelib_imf_internal_derfcx_data._cp016);
    cp1[16] = as_double(__devicelib_imf_internal_derfcx_data._cp116);
    apoly = __fma(apoly, aR, cp0[16]);
    bpoly = __fma(bpoly, bR, cp1[16]);
    cp0[15] = as_double(__devicelib_imf_internal_derfcx_data._cp015);
    cp1[15] = as_double(__devicelib_imf_internal_derfcx_data._cp115);
    apoly = __fma(apoly, aR, cp0[15]);
    bpoly = __fma(bpoly, bR, cp1[15]);
    cp0[14] = as_double(__devicelib_imf_internal_derfcx_data._cp014);
    cp1[14] = as_double(__devicelib_imf_internal_derfcx_data._cp114);
    apoly = __fma(apoly, aR, cp0[14]);
    bpoly = __fma(bpoly, bR, cp1[14]);
    cp0[13] = as_double(__devicelib_imf_internal_derfcx_data._cp013);
    cp1[13] = as_double(__devicelib_imf_internal_derfcx_data._cp113);
    apoly = __fma(apoly, aR, cp0[13]);
    bpoly = __fma(bpoly, bR, cp1[13]);
    cp0[12] = as_double(__devicelib_imf_internal_derfcx_data._cp012);
    cp1[12] = as_double(__devicelib_imf_internal_derfcx_data._cp112);
    apoly = __fma(apoly, aR, cp0[12]);
    bpoly = __fma(bpoly, bR, cp1[12]);
    cp0[11] = as_double(__devicelib_imf_internal_derfcx_data._cp011);
    cp1[11] = as_double(__devicelib_imf_internal_derfcx_data._cp111);
    apoly = __fma(apoly, aR, cp0[11]);
    bpoly = __fma(bpoly, bR, cp1[11]);
    cp0[10] = as_double(__devicelib_imf_internal_derfcx_data._cp010);
    cp1[10] = as_double(__devicelib_imf_internal_derfcx_data._cp110);
    apoly = __fma(apoly, aR, cp0[10]);
    bpoly = __fma(bpoly, bR, cp1[10]);
    cp0[9] = as_double(__devicelib_imf_internal_derfcx_data._cp09);
    cp1[9] = as_double(__devicelib_imf_internal_derfcx_data._cp19);
    apoly = __fma(apoly, aR, cp0[9]);
    bpoly = __fma(bpoly, bR, cp1[9]);
    cp0[8] = as_double(__devicelib_imf_internal_derfcx_data._cp08);
    cp1[8] = as_double(__devicelib_imf_internal_derfcx_data._cp18);
    apoly = __fma(apoly, aR, cp0[8]);
    bpoly = __fma(bpoly, bR, cp1[8]);
    cp0[7] = as_double(__devicelib_imf_internal_derfcx_data._cp07);
    cp1[7] = as_double(__devicelib_imf_internal_derfcx_data._cp17);
    apoly = __fma(apoly, aR, cp0[7]);
    bpoly = __fma(bpoly, bR, cp1[7]);
    cp0[6] = as_double(__devicelib_imf_internal_derfcx_data._cp06);
    cp1[6] = as_double(__devicelib_imf_internal_derfcx_data._cp16);
    apoly = __fma(apoly, aR, cp0[6]);
    bpoly = __fma(bpoly, bR, cp1[6]);
    cp0[5] = as_double(__devicelib_imf_internal_derfcx_data._cp05);
    cp1[5] = as_double(__devicelib_imf_internal_derfcx_data._cp15);
    apoly = __fma(apoly, aR, cp0[5]);
    bpoly = __fma(bpoly, bR, cp1[5]);
    cp0[4] = as_double(__devicelib_imf_internal_derfcx_data._cp04);
    cp1[4] = as_double(__devicelib_imf_internal_derfcx_data._cp14);
    apoly = __fma(apoly, aR, cp0[4]);
    bpoly = __fma(bpoly, bR, cp1[4]);
    cp0[3] = as_double(__devicelib_imf_internal_derfcx_data._cp03);
    cp1[3] = as_double(__devicelib_imf_internal_derfcx_data._cp13);
    apoly = __fma(apoly, aR, cp0[3]);
    bpoly = __fma(bpoly, bR, cp1[3]);
    cp0[2] = as_double(__devicelib_imf_internal_derfcx_data._cp02);
    cp1[2] = as_double(__devicelib_imf_internal_derfcx_data._cp12);
    apoly = __fma(apoly, aR, cp0[2]);
    bpoly = __fma(bpoly, bR, cp1[2]);
    cp0[1] = as_double(__devicelib_imf_internal_derfcx_data._cp01);
    cp1[1] = as_double(__devicelib_imf_internal_derfcx_data._cp11);
    apoly = __fma(apoly, aR, cp0[1]);
    bpoly = __fma(bpoly, bR, cp1[1]);
    cp0[0] = as_double(__devicelib_imf_internal_derfcx_data._cp00);
    cp1[0] = as_double(__devicelib_imf_internal_derfcx_data._cp10);
    apoly = __fma(apoly, aR, cp0[0]);
    bpoly = __fma(bpoly, bR, cp1[0]);
    bpoly = (bpoly * bR);
    cp1h = as_double(__devicelib_imf_internal_derfcx_data._cp10h);
    bpoly = __fma(bR, cp1h, bpoly);
    vr1 = (ltTwo != 0x0L) ? (apoly) : (bpoly);
    // negative input?
    Zero = as_double(__devicelib_imf_internal_derfcx_data._Zero);
    dNegMask = as_double((VUINT64)((va1 < Zero) ? 0xffffffffffffffff : 0x0));
    lNegMask = as_ulong(dNegMask);
    mNegMask = 0;
    mNegMask = lNegMask;
    if (__builtin_expect((mNegMask) != 0, 0)) {
      double x2h;
      double x2l;
      double Sf;
      double Nf;
      double L2E;
      double L2H;
      double L2L;
      double Shifter;
      double poly;
      double R1;
      double Te;
      double ce11;
      double ce10;
      double ce9;
      double ce8;
      double ce7;
      double ce6;
      double ce5;
      double ce4;
      double ce3;
      double ce2;
      double ce1;
      double Two;
      double R;
      double res;
      double Smax;
      double Inf;
      VUINT64 OFMask;
      // x2h = DP_FMA(xa.f, xa.f, 0.0);
      x2h = (xa * xa);
      // x2l = DP_FMA(xa.f, xa.f, -x2h);
      x2l = __fma(xa, xa, -(x2h));
      // Sf = x2h*L2E + Shifter
      L2E = as_double(__devicelib_imf_internal_derfcx_data._L2E);
      Shifter = as_double(__devicelib_imf_internal_derfcx_data._Shifter);
      Sf = __fma(x2h, L2E, Shifter);
      // Nf = (int)(x2h*L2E)
      Nf = (Sf - Shifter);
      // R+Rl = x^2 - N*log(2)
      // SSE2 values
      L2H = as_double(__devicelib_imf_internal_derfcx_data._L2H);
      R = __fma(-(Nf), L2H, x2h);
      L2L = as_double(__devicelib_imf_internal_derfcx_data._L2L);
      R1 = __fma(-(Nf), L2L, x2l);
      R = (R + R1);
      // 2^(N) = Te.w = S.w << 52;
      Te = as_double(((VUINT64)as_ulong(Sf) << (52)));
      // exp(R)-1
      ce11 = as_double(__devicelib_imf_internal_derfcx_data._ce11);
      ce10 = as_double(__devicelib_imf_internal_derfcx_data._ce10);
      ce9 = as_double(__devicelib_imf_internal_derfcx_data._ce9);
      ce8 = as_double(__devicelib_imf_internal_derfcx_data._ce8);
      ce7 = as_double(__devicelib_imf_internal_derfcx_data._ce7);
      ce6 = as_double(__devicelib_imf_internal_derfcx_data._ce6);
      ce5 = as_double(__devicelib_imf_internal_derfcx_data._ce5);
      ce4 = as_double(__devicelib_imf_internal_derfcx_data._ce4);
      ce3 = as_double(__devicelib_imf_internal_derfcx_data._ce3);
      ce2 = as_double(__devicelib_imf_internal_derfcx_data._ce2);
      ce1 = as_double(__devicelib_imf_internal_derfcx_data._ce1);
      poly = __fma(R, ce11, ce10);
      poly = __fma(poly, R, ce9);
      poly = __fma(poly, R, ce8);
      poly = __fma(poly, R, ce7);
      poly = __fma(poly, R, ce6);
      poly = __fma(poly, R, ce5);
      poly = __fma(poly, R, ce4);
      poly = __fma(poly, R, ce3);
      poly = __fma(poly, R, ce2);
      poly = __fma(poly, R, ce1);
      poly = (poly * R);
      // 2^(N)*exp(R)
      // DP_FMA(poly.f, Te.f, Te.f);
      poly = __fma(poly, Te, Te);
      // res = 2^(N+1)*exp(R) - res  (i.e exp(x^2)*2-erfcx(|x|))
      Two = as_double(__devicelib_imf_internal_derfcx_data._Two);
      res = __fma(poly, Two, -(vr1));
      // fixup for overflow
      // res.f = (S.f >= _VSTATIC(__Smax).f) ? _VSTATIC(__Inf).f : res.f;
      // check whether |x| > overflow_threshold;  x<0 here
      Smax = as_double(__devicelib_imf_internal_derfcx_data._Smax);
      Inf = as_double(__devicelib_imf_internal_derfcx_data._Inf);
      OFMask = (!(xa <= Smax)) ? 0x1L : 0x0L;
      res = (OFMask != 0x0L) ? (Inf) : (res);
      /* Merge results from main path and large arguments path */
      vr1 = as_double((((~as_ulong(dNegMask)) & as_ulong(vr1)) |
                       (as_ulong(dNegMask) & as_ulong(res))));
    }
  }
  if (__builtin_expect((vm) != 0, 0)) {
    double __cout_a1;
    double __cout_r1;
    ((double *)&__cout_a1)[0] = va1;
    ((double *)&__cout_r1)[0] = vr1;
    __devicelib_imf_internal_derfcx(&__cout_a1, &__cout_r1);
    vr1 = ((const double *)&__cout_r1)[0];
  }
  r = vr1;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
