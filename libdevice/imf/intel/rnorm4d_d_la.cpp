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
#include "_imf_include_fp64.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_rnorm4d_d_la {
namespace {
typedef struct {
  VUINT64 _AbsMask;
  VUINT64 _MaxNorm;
  VUINT64 _Zero;
  VUINT64 _Inf;
  VUINT64 _Biasx2;
  VUINT64 _Two62;
  VUINT64 _One;
  VUINT64 _ac2;
  VUINT64 _ac1;
  VUINT64 _c4;
  VUINT64 _c3;
  VUINT64 _c2;
  VUINT64 _c1;
} __devicelib_imf_internal_drnorm4d_data_t;
static const __devicelib_imf_internal_drnorm4d_data_t
    __devicelib_imf_internal_drnorm4d_data = {
        /* AbsMask */ 0x7FFFFFFFFFFFFFFFuL,
        /* MaxNorm */ 0x7FEFFFFFFFFFFFFFuL,
        /* Zero */ 0x0000000000000000uL,
        /* Inf */ 0x7FF0000000000000uL,
        /* Biasx2 */ 0x7FE0000000000000uL,
        /* Two62 */ 0x4000000000000000uL,
        /* One */ 0x3FF0000000000000uL,
        /* ac2 */ 0x3FD8000000000000uL,
        /* ac1 */ 0xBFE0000000000000uL,
        /* c4 */ 0x3fd18000e3648413uL,
        /* c3 */ 0xbfd40000b1300588uL,
        /* c2 */ 0x3fd7fffffffffc4auL,
        /* c1 */ 0xbfdffffffffffe71uL,
};
} /* namespace */
} /* namespace __imf_impl_rnorm4d_d_la */
DEVICE_EXTERN_C_INLINE double __devicelib_imf_rnorm4d(double x, double y,
                                                      double z, double t) {
  using namespace __imf_impl_rnorm4d_d_la;
  double r;
  VUINT32 vm;
  double va1;
  double va2;
  double va3;
  double va4;
  double vr1;
  va1 = x;
  va2 = y;
  va3 = z;
  va4 = t;
  {
    VUINT64 lSpecialMask;
    VUINT32 mSpecialMask;
    double AbsMask;
    double ax;
    double ay;
    double az;
    double aw;
    double asum;
    double MaxNorm;
    double Zero;
    double SpecialMask;
    double ZeroMask;
    double h12;
    double l12;
    double h34;
    double l34;
    double H0;
    double Inf;
    VUINT64 exponl;
    VUINT64 expon_corr;
    VUINT64 Biasx2;
    VUINT64 Two62;
    double expon0;
    double t0;
    double t1;
    double t2;
    double t3;
    double p0h;
    double p1h;
    double p2h;
    double p3h;
    double p0l;
    double p1l;
    double p2l;
    double p3l;
    double p3hh;
    double p3hl;
    double s23;
    double s01;
    double p1hh;
    double p1hl;
    double sum_h;
    double sum_l;
    double Th;
    double Tl;
    double Tlh;
    double Tll;
    double RS;
    double RS2;
    double poly;
    double One;
    double me0;
    double m_eps;
    double ac2;
    double ac1;
    float ftmp;
    vm = 0;
    Zero = as_double(__devicelib_imf_internal_drnorm4d_data._Zero);
    AbsMask = as_double(__devicelib_imf_internal_drnorm4d_data._AbsMask);
    // absolute values
    ax = as_double((as_ulong(va1) & as_ulong(AbsMask)));
    ay = as_double((as_ulong(va2) & as_ulong(AbsMask)));
    az = as_double((as_ulong(va3) & as_ulong(AbsMask)));
    aw = as_double((as_ulong(va4) & as_ulong(AbsMask)));
    // test whether any input is Inf/NaN
    // first add absolute values
    asum = (ax + ay);
    asum = (asum + az);
    asum = (asum + aw);
    // asum <= max_norm?
    MaxNorm = as_double(__devicelib_imf_internal_drnorm4d_data._MaxNorm);
    SpecialMask =
        as_double((VUINT64)(((!(asum <= MaxNorm)) ? 0xffffffffffffffff : 0x0)));
    // asum == 0?
    Zero = as_double(__devicelib_imf_internal_drnorm4d_data._Zero);
    ZeroMask = as_double((VUINT64)((asum == Zero) ? 0xffffffffffffffff : 0x0));
    // mask for Inf/NaN input, or all zero
    SpecialMask = as_double((as_ulong(SpecialMask) | as_ulong(ZeroMask)));
    // Order inputs based on absolute value
    //	h12 = DP_FMAX(ax.f, ay.f);
    h12 = ((ax > ay) ? ax : ay);
    //  l12 = DP_FMIN(ax.f, ay.f);
    l12 = ((ax < ay) ? ax : ay);
    //  h34 = DP_FMAX(az.f, aw.f);
    h34 = ((az > aw) ? az : aw);
    //  l34 = DP_FMIN(az.f, aw.f);
    l34 = ((az < aw) ? az : aw);
    // largest input (in absolute value)
    // H0 = DP_FMAX(h12, h34);
    H0 = ((h12 > h34) ? h12 : h34);
    // scale inputs
    // expon0.w = H0 & 0x7ff0000000000000ull;
    Inf = as_double(__devicelib_imf_internal_drnorm4d_data._Inf);
    H0 = as_double((as_ulong(H0) & as_ulong(Inf)));
    // 2^(-expon_t0+ ((t0>=2)?1:0))
    // expon0.w = 0x7fe0000000000000ull + ((expon0.w & 0x4000000000000000) >>
    // 10)- expon0.w;
    exponl = as_ulong(H0);
    Biasx2 = (__devicelib_imf_internal_drnorm4d_data._Biasx2);
    Two62 = (__devicelib_imf_internal_drnorm4d_data._Two62);
    expon_corr = (exponl & Two62);
    expon_corr = ((VUINT64)(expon_corr) >> (10));
    expon_corr = (expon_corr + Biasx2);
    expon_corr = (expon_corr - exponl);
    expon0 = as_double(expon_corr);
    // t0 = DP_FMA(h12, expon0.f, 0.0);
    // t1 = DP_FMA(l12, expon0.f, 0.0);
    // t2 = DP_FMA(h34, expon0.f, 0.0);
    // t3 = DP_FMA(l34, expon0.f, 0.0);
    t0 = __fma(h12, expon0, Zero);
    t1 = __fma(l12, expon0, Zero);
    t2 = __fma(h34, expon0, Zero);
    t3 = __fma(l34, expon0, Zero);
    // high parts of squares
    // p3h = DP_FMA(t3, t3, 0.0);
    // p2h = DP_FMA(t2, t2, 0.0);
    // p1h = DP_FMA(t1, t1, 0.0);
    // p0h = DP_FMA(t0, t0, 0.0);
    p3h = __fma(t3, t3, Zero);
    p2h = __fma(t2, t2, Zero);
    p1h = __fma(t1, t1, Zero);
    p0h = __fma(t0, t0, Zero);
    // low parts of squares
    // p3l = DP_FMA(t3, t3, -p3h);
    // p2l = DP_FMA(t2, t2, -p2h);
    // p1l = DP_FMA(t1, t1, -p1h);
    // p0l = DP_FMA(t0, t0, -p0h);
    p3l = __fma(t3, t3, -(p3h));
    p2l = __fma(t2, t2, -(p2h));
    p1l = __fma(t1, t1, -(p1h));
    p0l = __fma(t0, t0, -(p0h));
    // compute sum in high-low parts
    // sum_l = p3l + p2l + p1l + p0l;
    sum_l = (p3l + p2l);
    sum_l = (sum_l + p1l);
    sum_l = (sum_l + p0l);
    One = as_double(__devicelib_imf_internal_drnorm4d_data._One);
    // s23 = p3h + p2h
    s23 = (p3h + p2h);
    // p3hh = DP_FMA(s23, 1.0, -p2h);
    p3hh = __fma(s23, One, -(p2h)); // VSUB(D, p3hh, s23, p2h);
    // p3hl = DP_FMA(p3h, 1.0, -p3hh);
    p3hl =
        __fma(p3h, One, -(p3hh)); // VSUB(D, p3hl, p3h, p3hh);
    // sum_l += p3hl;
    sum_l = (sum_l + p3hl);
    // s01 = p1h + p0h
    s01 = (p1h + p0h);
    // p1hh = DP_FMA(s01, 1.0, -p0h);
    p1hh =
        __fma(s01, One, -(p0h)); //  VSUB(D, p1hh, s01, p0h);
    // p1hl = DP_FMA(p1h, 1.0, -p1hh);
    p1hl =
        __fma(p1h, One, -(p1hh)); // VSUB(D, p1hl, p1h, p1hh);
    // sum_l += p1hl;
    sum_l = (sum_l + p1hl);
    // sum_h = s01 + s23
    sum_h = (s01 + s23);
    // These terms are not ordered; will order now
    // Th = DP_FMAX(s23, s01);
    Th = ((s23 > s01) ? s23 : s01);
    // Tl = DP_FMIN(s23, s01);
    Tl = ((s23 < s01) ? s23 : s01);
    // Tlh = DP_FMA(sum_h, 1.0, -Th);
    Tlh = __fma(sum_h, One, -(Th)); // VSUB(D, Tlh, sum_h, Th);
    // Tll = DP_FMA(Tl, 1.0, -Tlh);
    Tll = __fma(Tl, One, -(Tlh)); // VSUB(D, Tll, Tl, Tlh);
    // sum_l += Tll;
    sum_l = (sum_l + Tll);
    // fsum = (float)sum_h;  frs = SP_RSQRT(fsum); RS = (double)frs;
    //_VRSQRT_QK(D, RS, sum_h);
    ftmp = ((float)(sum_h));
    ftmp = (1.0f / __sqrt(ftmp));
    RS = ((double)(ftmp));
    // apply refinement iteration
    // RS2 = DP_FMA(RS, RS, 0.0);
    RS2 = __fma(RS, RS, Zero);
    // m_eps = DP_FMA(sum_h, RS2, -1.0);
    // m_eps = DP_FMA(sum_l, RS2, m_eps);
    One = as_double(__devicelib_imf_internal_drnorm4d_data._One);
    me0 = __fma(sum_h, RS2, -(One));
    m_eps = __fma(sum_l, RS2, me0);
    // poly = DP_FMA(m_eps, 0.375, -0.5);
    ac2 = as_double(__devicelib_imf_internal_drnorm4d_data._ac2);
    ac1 = as_double(__devicelib_imf_internal_drnorm4d_data._ac1);
    poly = __fma(ac2, m_eps, ac1);
    // poly = DP_FMA(poly, m_eps, 0.0);
    poly = __fma(poly, m_eps, Zero);
    // RS = DP_FMA(poly, RS, RS);
    RS = __fma(RS, poly, RS);
    // res.f = DP_FMA(RS, expon0.f, 0.0)
    vr1 = __fma(RS, expon0, Zero);
    // res.w = (sum_h != 0.0) ? res.w : 0x7ff0000000000000ull;
    // return res.f;
    lSpecialMask = as_ulong(SpecialMask);
    mSpecialMask = 0;
    mSpecialMask = lSpecialMask;
    if (__builtin_expect((mSpecialMask) != 0, 0)) {
      double InfMask;
      double InfMask2;
      double InfMask3;
      double InfMask4;
      Inf = as_double(__devicelib_imf_internal_drnorm4d_data._Inf);
      // Will return NaN if NaN inputs, no Inf
      SpecialMask =
          as_double((VUINT64)(((!(asum == asum)) ? 0xffffffffffffffff : 0x0)));
      vr1 = as_double((((~as_ulong(SpecialMask)) & as_ulong(vr1)) |
                       (as_ulong(SpecialMask) & as_ulong(asum))));
      // check if any input is Inf
      InfMask = as_double((VUINT64)((MaxNorm < ax) ? 0xffffffffffffffff : 0x0));
      InfMask2 =
          as_double((VUINT64)((MaxNorm < ay) ? 0xffffffffffffffff : 0x0));
      InfMask3 =
          as_double((VUINT64)((MaxNorm < az) ? 0xffffffffffffffff : 0x0));
      InfMask4 =
          as_double((VUINT64)((MaxNorm < aw) ? 0xffffffffffffffff : 0x0));
      InfMask = as_double((as_ulong(InfMask) | as_ulong(InfMask2)));
      InfMask3 = as_double((as_ulong(InfMask3) | as_ulong(InfMask4)));
      InfMask = as_double((as_ulong(InfMask) | as_ulong(InfMask3)));
      /* Return Inf if all inputs are zero */
      vr1 = as_double((((~as_ulong(InfMask)) & as_ulong(vr1)) |
                       (as_ulong(InfMask) & as_ulong(Zero))));
      /* Return Inf if all inputs are zero */
      vr1 = as_double((((~as_ulong(ZeroMask)) & as_ulong(vr1)) |
                       (as_ulong(ZeroMask) & as_ulong(Inf))));
    }
  }
  r = vr1;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
