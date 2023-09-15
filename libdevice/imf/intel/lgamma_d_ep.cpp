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
namespace __imf_impl_lgamma_d_ep {
namespace {

/* -------------------------------------- */
/*       internal log implementation      */
/* -------------------------------------- */
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep_two52 = {0x4330000000000000UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep_two53 = {0x4340000000000000UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep_dc8 = {0x4ea9d46b06ce620eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep_dc7 = {0xcef6f7ad23b5cd51UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep_dc6 = {0x4f3e8f3677c334d3UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep_dc5 = {0xcf7e3074dfb5bb14UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep_dc4 = {0x4fb50783485523f4UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep_dc3 = {0xcfe32d2cce627c9eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep_dc2 = {0x500466bc6775aa7dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep_dc1 = {0xd014abbce625be52UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep_dc0 = {0x500921fb54442d18UL};
// low parts of c0, c1
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep_dc0l = {0x4c918E38E04F47B9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep_dc1l = {0xcca15D7C17744D75UL};
// 2^(-256)
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep_scm_256 = {0x2ff0000000000000UL};
// max. normal value
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep_max_norm = {0x7f7fffffffffffffUL};

// sinpi routine
static inline double __dlgamma_ep_internal_sinpi(double xin)
{
  int nRet = 0;
  double result;
  // double sinpi(double xin)
  union {
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
  } x, xa, sgn_x, iN, sgn_N, res;
  double fN, dS, R, R2, R2l, poly;
  double High, Low, H, L, R2c1_h, R2c1_l;
  x.f = xin;
  xa.f = __fabs(xin);
  sgn_x.w = x.w ^ xa.w;
  // 2^52 + |x|
  dS = __dlgamma_ep_two52.f + xa.f;
  // rint(|x|), for |x|<2^52
  fN = dS - __dlgamma_ep_two52.f;
  // correction for |x|>=2^52
  fN = (xa.f < __dlgamma_ep_two52.f) ? fN : xa.f;
  // get last bit of rint(|x|)
  iN.f = (xa.f < __dlgamma_ep_two52.f) ? dS : xa.f;
  iN.f = (xa.f < __dlgamma_ep_two53.f) ? iN.f : 0.0;
  // sgn set if rint(|x|) is odd, 0 otherwise
  sgn_N.w = iN.w << 63;
  R = xa.f - fN;
  R2 = R * R;
  // R2_low
  R2l = __fma(R, R, -(R2));
  poly = __fma(R2, __dlgamma_ep_dc8.f, __dlgamma_ep_dc7.f);
  poly = __fma(poly, R2, __dlgamma_ep_dc6.f);
  poly = __fma(poly, R2, __dlgamma_ep_dc5.f);
  poly = __fma(poly, R2, __dlgamma_ep_dc4.f);
  poly = __fma(poly, R2, __dlgamma_ep_dc3.f);
  poly = __fma(poly, R2, __dlgamma_ep_dc2.f);
  poly = __fma(poly, R2, __dlgamma_ep_dc1l.f);
  // c0+(R2*c1)_h
  H = __fma(__dlgamma_ep_dc1.f, R2, __dlgamma_ep_dc0.f);
  // (R2*c1)_h
  R2c1_h = H - __dlgamma_ep_dc0.f;
  // (R2*c1)_l
  R2c1_l = __fma(R2, __dlgamma_ep_dc1.f, -(R2c1_h));
  // R2c1_l + R2l*poly
  R2c1_l = __fma(R2l, poly, R2c1_l);
  // c0l + R2l*c1
  L = __fma(R2l, __dlgamma_ep_dc1.f, __dlgamma_ep_dc0l.f);
  // c0l + R2l*c1 + R2c1_l + R2l*poly
  L = L + R2c1_l;
  // L = c0l + R2l*c1 + R2c1_l + R2l*poly + R2*poly
  L = __fma(poly, R2, L);
  // (R*H)_high
  High = __fma(R, H, 0.0);
  // (R*H)_low
  Low = __fma(R, H, -(High));
  // (R*H)_low + R*L
  Low = __fma(R, L, Low);
  res.f = High + Low;
  // final scaling
  res.f = __fma(res.f, __dlgamma_ep_scm_256.f, 0.0);
  // fix sign
  // res.w ^= sgn;
  // sign of zero correction
  res.w = (res.w == 0) ? sgn_x.w : (res.w ^ (sgn_x.w ^ sgn_N.w));
  result = res.f;
  return result;
}

/* -------------------------------------- */
/*     internal sinpi implementation      */
/* -------------------------------------- */
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep___c19 = {0xbfb6e22682c05596UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep___c18 = {0x3fb6c694b21a9875UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep___c17 = {0xbfa68f0acee35e2dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep___c16 = {0x3fa9474ccd075ce5UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep___c15 = {0xbfb0750f4f9c34f9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep___c14 = {0x3fb16608748ab72dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep___c13 = {0xbfb23e2ec341eba0UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep___c12 = {0x3fb3aa521d980cd0UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep___c11 = {0xbfb555fa23866d76UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep___c10 = {0x3fb74629a554d880UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep___c9 = {0xbfb999938abcf213UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep___c8 = {0x3fbc71c472fb2195UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep___c7 = {0xbfc00000112830d9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep___c6 = {0x3fc24924982c2697UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep___c5 = {0xbfc55555551fbbdbUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep___c4 = {0x3fc99999998c68b5UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep___c3 = {0xbfd0000000002697UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep___c2 = {0x3fd5555555555b0eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep___c1 = {0xbfdffffffffffff0UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dlgamma_ep___c0 = {0xbc8a30cfded694ffUL};

// log routine
static inline double __dlgamma_ep_internal_log(double a)
{
  int nRet = 0;
  union {
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
  } x, expon, expon_r, one, l2;
  double R, d_expon;
  union {
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
  } denorm_scale;
  double poly, res;
  union {
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
  } _res;
  int denorm_scale_exp;
  x.f = a;
  // special branch for +/-0, negatives, +INFs, +NaNs
  if ((x.w == 0x0uL) || (x.w >= 0x7ff0000000000000uL)) {
    // x = +/-0
    if ((x.w & 0x7fffffffffffffff) == 0x0uL) {
      nRet = 2;
      _res.w = 0xfff0000000000000uL;
      return _res.f;
    }
    // x = any negative
    else if (x.w > 0x8000000000000000uL) {
      nRet = 1;
      _res.w = x.w | 0xfff8000000000000uL;
      return _res.f;
    }
    // x = +NaN or +INF
    else {
      // x = +NaN
      if (x.w > 0x7ff0000000000000uL) {
        _res.f = x.f + x.f;
      }
      // x = +INF
      else {
        _res.w = x.w;
      }
      return _res.f;
    } // x = +NaN or +INF
  }   // special branch for +/-0, negatives, +INFs, +NaNs
  // scale denormals
  denorm_scale.w = 0x43B0000000000000ul;
  denorm_scale_exp = (x.w <= 0x000fffffffffffffuL) ? (60 + 0x3FF) : 0x3FF;
  x.f = (x.w <= 0x000fffffffffffffuL) ? (x.f * denorm_scale.f) : x.f;
  // argument reduction to (-1/3, 1/3)
  // reduced exponent
  expon.w = x.w + 0x000AAAAAAAAAAAAAul;
  expon.w >>= 52;
  expon_r.w = expon.w << 52;
  // reduced mantissa
  one.w = 0x3FF0000000000000ul;
  x.w = (x.w + one.w) - expon_r.w;
  // reduced argument:  reduced_mantissa - 1.0
  R = x.f - one.f;
  // polynomial
  poly = __fma(__dlgamma_ep___c19.f, R, __dlgamma_ep___c18.f);
  poly = __fma(poly, R, __dlgamma_ep___c17.f);
  poly = __fma(poly, R, __dlgamma_ep___c16.f);
  poly = __fma(poly, R, __dlgamma_ep___c15.f);
  poly = __fma(poly, R, __dlgamma_ep___c14.f);
  poly = __fma(poly, R, __dlgamma_ep___c13.f);
  poly = __fma(poly, R, __dlgamma_ep___c12.f);
  poly = __fma(poly, R, __dlgamma_ep___c11.f);
  poly = __fma(poly, R, __dlgamma_ep___c10.f);
  poly = __fma(poly, R, __dlgamma_ep___c9.f);
  poly = __fma(poly, R, __dlgamma_ep___c8.f);
  poly = __fma(poly, R, __dlgamma_ep___c7.f);
  poly = __fma(poly, R, __dlgamma_ep___c6.f);
  poly = __fma(poly, R, __dlgamma_ep___c5.f);
  poly = __fma(poly, R, __dlgamma_ep___c4.f);
  poly = __fma(poly, R, __dlgamma_ep___c3.f);
  poly = __fma(poly, R, __dlgamma_ep___c2.f);
  poly = __fma(poly, R, __dlgamma_ep___c1.f);
  poly = __fma(poly, R, __dlgamma_ep___c0.f);
  // prepare exponent
  // scale back denormals
  expon.s32[0] -= denorm_scale_exp;
  // exponent
  d_expon = (double)expon.s32[0];
  // full polynomial = log(1+R)
  poly = __fma(poly, R, R);
  // result:  reduced_exponent*log(2)+log(1+R)
  l2.w = 0x3FE62E42FEFA39EFul;
  res = __fma(d_expon, l2.f, poly);
  return res;
}

/*========================== begin_copyright_notice ============================
Copyright (C) 2014 Advanced Micro Devices, Inc.
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
============================= end_copyright_notice ===========================*/
/*
// ====================================================
// Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
//
// Developed at SunPro, a Sun Microsystems, Inc. business.
// Permission to use, copy, modify, and distribute this
// software is freely granted, provided that this notice
// is preserved.
// ====================================================
// lgamma_r(x, i)
// Reentrant version of the logarithm of the Gamma function
// with user provide pointer for the sign of Gamma(x).
//
// Method:
//   1. Argument Reduction for 0 < x <= 8
//      Since gamma(1+s)=s*gamma(s), for x in [0,8], we may
//      reduce x to a number in [1.5,2.5] by
//              lgamma(1+s) = log(s) + lgamma(s)
//      for example,
//              lgamma(7.3) = log(6.3) + lgamma(6.3)
//                          = log(6.3*5.3) + lgamma(5.3)
//                          = log(6.3*5.3*4.3*3.3*2.3) + lgamma(2.3)
//   2. Polynomial approximation of lgamma around its
//      minimun ymin=1.461632144968362245 to maintain monotonicity.
//      On [ymin-0.23, ymin+0.27] (i.e., [1.23164,1.73163]), use
//              Let z = x-ymin;
//              lgamma(x) = -1.214862905358496078218 + z^2*poly(z)
//      where
//              poly(z) is a 14 degree polynomial.
//   2. Rational approximation in the primary interval [2,3]
//      We use the following approximation:
//              s = x-2.0;
//              lgamma(x) = 0.5*s + s*P(s)/Q(s)
//      with accuracy
//              |P/Q - (lgamma(x)-0.5s)| < 2**-61.71
//      Our algorithms are based on the following observation
//
//                             zeta(2)-1    2    zeta(3)-1    3
// lgamma(2+s) = s*(1-Euler) + --------- * s  -  --------- * s  + ...
//                                 2                 3
//
//      where Euler = 0.5771... is the Euler constant, which is very
//      close to 0.5.
//
//   3. For x>=8, we have
//      lgamma(x)~(x-0.5)log(x)-x+0.5*log(2pi)+1/(12x)-1/(360x**3)+....
//      (better formula:
//         lgamma(x)~(x-0.5)*(log(x)-1)-.5*(log(2pi)-1) + ...)
//      Let z = 1/x, then we approximation
//              f(z) = lgamma(x) - (x-0.5)(log(x)-1)
//      by
//                                  3       5             11
//              w = w0 + w1*z + w2*z  + w3*z  + ... + w6*z
//      where
//              |w - f(z)| < 2**-58.74
//
//   4. For negative x, since (G is gamma function)
//              -x*G(-x)*G(x) = pi/sin(pi*x),
//      we have
//              G(x) = pi/(sin(pi*x)*(-x)*G(-x))
//      since G(-x) is positive, sign(G(x)) = sign(sin(pi*x)) for x<0
//      Hence, for x<0, signgam = sign(sin(pi*x)) and
//              lgamma(x) = log(|Gamma(x)|)
//                        = log(pi/(|x*sin(pi*x)|)) - lgamma(-x);
//      Note: one should avoid compute pi*(-x) directly in the
//            computation of sin(pi*(-x)).
//
//   5. Special Cases
//              lgamma(2+s) ~ s*(1-Euler) for tiny s
//              lgamma(1)=lgamma(2)=0
//              lgamma(x) ~ -log(x) for tiny x
//              lgamma(0) = lgamma(inf) = inf
//              lgamma(-integer) = +-inf
//
*/
/* LIBCLC lgamma_r implementation */
static inline double __dlgamma_ep_internal_lgamma_r(const double *arg,
                                                    double *res, int *ip)
{
  int nRet = 0;
  double x = *arg, r;
  uint64_t ux = as_ulong(x);
  uint64_t ax = ux & 0x7fffffffffffffffL;
  double absx = as_double(ax);
  // x = +-Inf, NaN
  if (ax >= 0x7ff0000000000000UL) {
    *ip = 1;
    *res = absx;
  }
  // small |x| or 0.0
  else if (absx < 0x1.0p-70) {
    *ip = (ax == ux) ? 1 : -1;
    if (ax == 0x0) {
      *res = 1.0 / absx;
      nRet = 2;
    } else {
      *res = -__dlgamma_ep_internal_log(absx);
    }
  }
  // overflow x > ~2.5e305
  else if (x > 0x1.754d9278b51a7p+1014) {
    *res = 0x1.754d9278b51a7p+1014 * 0x1.754d9278b51a7p+1014;
    nRet = 3;
  }
  // other x
  else {
    if (absx < 2.0) {
      int i = 0;
      double y = 2.0 - absx;
      int c = absx < 0x1.bb4c3p+0;
      double t = absx - 0x1.762d86356be3fp+0;
      i = c ? 1 : i;
      y = c ? t : y;
      c = absx < 0x1.3b4c4p+0;
      t = absx - 1.0;
      i = c ? 2 : i;
      y = c ? t : y;
      c = absx <= 0x1.cccccp-1;
      t = -__dlgamma_ep_internal_log(absx);
      r = c ? t : 0.0;
      t = 1.0 - absx;
      i = c ? 0 : i;
      y = c ? t : y;
      c = absx < 0x1.76944p-1;
      t = absx - (0x1.762d86356be3fp+0 - 1.0);
      i = c ? 1 : i;
      y = c ? t : y;
      c = absx < 0x1.da661p-3;
      i = c ? 2 : i;
      y = c ? absx : y;
      double p, q;
      switch (i) {
      case 0:
        p = __fma(
            y,
            __fma(
                y,
                __fma(
                    y,
                    __fma(y, 0x1.7858e90a45837p-15,
                                           0x1.a7074428cfa52p-16),
                    0x1.c5088987dfb07p-14),
                0x1.cf2eced10e54dp-13),
            0x1.0b6c689b99cp-11);
        p = __fma(
            y,
            __fma(
                y,
                __fma(
                    y, __fma(y, p, 0x1.38a94116f3f5dp-10),
                    0x1.7add8ccb7926bp-9),
                0x1.e404fb68fefe8p-8),
            0x1.51322ac92547bp-6);
        p = __fma(
            y,
            __fma(
                y, __fma(y, p, 0x1.13e001a5562a7p-4),
                0x1.4a34cc4a60fadp-2),
            0x1.3c467e37db0c8p-4);
        r = __fma(y, p - 0.5, r);
        break;
      case 1:
        p = __fma(
            y,
            __fma(
                y,
                __fma(
                    y,
                    __fma(y, 0x1.5fd3ee8c2d3f4p-12,
                                           -0x1.47f24ecc38c38p-12),
                    0x1.4af6d6c0ebbf7p-12),
                -0x1.1a6109c73e0ecp-11),
            0x1.cdf0cef61a8e9p-11);
        p = __fma(
            y,
            __fma(
                y,
                __fma(
                    y,
                    __fma(
                        y, __fma(y, p, -0x1.6fe8ebf2d1af1p-10),
                        0x1.282d32e15c915p-9),
                    -0x1.e2effb3e914d7p-9),
                0x1.8fce0e370e344p-8),
            -0x1.51f9fba91ec6ap-7);
        p = __fma(
            y,
            __fma(
                y,
                __fma(
                    y,
                    __fma(
                        y, __fma(y, p, 0x1.266e7970af9ecp-6),
                        -0x1.0c9a8df35b713p-5),
                    0x1.08b4294d5419bp-4),
                -0x1.2e4278dc6c509p-3),
            0x1.ef72bc8ee38a2p-2);
        p = __fma(y * y, p, -(-0x1.0c7caa48a971fp-58));
        r += (-0x1.f19b9bcc38a42p-4 + p);
        break;
      case 2:
        p = y * __fma(
                    y,
                    __fma(
                        y,
                        __fma(
                            y,
                            __fma(
                                y,
                                __fma(y, 0x1.b678bbf2bab09p-7,
                                                       0x1.d4eaef6010924p-3),
                                0x1.f497644ea845p-1),
                            0x1.7475cd119bd6fp+0),
                        0x1.4401e8b005dffp-1),
                    -0x1.3c467e37db0c8p-4);
        q = __fma(
            y,
            __fma(
                y,
                __fma(
                    y,
                    __fma(
                        y,
                        __fma(y, 0x1.a5abb57d0cf61p-9,
                                               0x1.aae55d6537c88p-4),
                        0x1.89dfbe45050afp-1),
                    0x1.10725a42b18f5p+1),
                0x1.3a5d7c2bd619cp+1),
            1.0);
        r += __fma(-0.5, y, p / q);
      }
    } else if (absx < 8.0) {
      int i = absx;
      double y = absx - (double)i;
      double p =
          y *
          __fma(
              y,
              __fma(
                  y,
                  __fma(
                      y,
                      __fma(
                          y,
                          __fma(
                              y,
                              __fma(y, 0x1.0bfecdd17e945p-15,
                                                     0x1.e26b67368f239p-10),
                              0x1.b481c7e939961p-6),
                          0x1.2bb9cbee5f2f7p-3),
                      0x1.4d98f4f139f59p-2),
                  0x1.b848b36e20878p-3),
              -0x1.3c467e37db0c8p-4);
      double q = __fma(
          y,
          __fma(
              y,
              __fma(
                  y,
                  __fma(
                      y,
                      __fma(
                          y,
                          __fma(y, 0x1.ebaf7a5b3814p-18,
                                                 0x1.97ddaca41a95bp-11),
                          0x1.317ea742ed475p-6),
                      0x1.601edccfbdf27p-3),
                  0x1.71a1893d3dcdcp-1),
              0x1.645a762c4ab74p+0),
          1.0);
      r = __fma(0.5, y, p / q);
      double z = 1.0;
      // lgamma(1+s) = log(s) + lgamma(s)
      double y6 = y + 6.0;
      double y5 = y + 5.0;
      double y4 = y + 4.0;
      double y3 = y + 3.0;
      double y2 = y + 2.0;
      z *= i > 6 ? y6 : 1.0;
      z *= i > 5 ? y5 : 1.0;
      z *= i > 4 ? y4 : 1.0;
      z *= i > 3 ? y3 : 1.0;
      z *= i > 2 ? y2 : 1.0;
      r += __dlgamma_ep_internal_log(z);
    } else {
      double z = 1.0 / absx;
      double z2 = z * z;
      double w = __fma(
          z,
          __fma(
              z2,
              __fma(
                  z2,
                  __fma(
                      z2,
                      __fma(
                          z2,
                          __fma(z2, -0x1.ab89d0b9e43e4p-10,
                                                 0x1.b67ba4cdad5d1p-11),
                          -0x1.380cb8c0fe741p-11),
                      0x1.a019f98cf38b6p-11),
                  -0x1.6c16c16b02e5cp-9),
              0x1.555555555553bp-4),
          0x1.acfe390c97d69p-2);
      r = (absx - 0.5) * (__dlgamma_ep_internal_log(absx) - 1.0) + w;
    }
    if (x < 0.0) {
      double t = __dlgamma_ep_internal_sinpi(x);
      r = __dlgamma_ep_internal_log(0x1.921fb54442d18p+1 / __fabs(t * x)) - r;
      if (t == 0.0) {
        r = as_double(0x7ff0000000000000L);
        nRet = 2;
      }
      *ip = t < 0.0 ? -1 : 1;
    } else {
      *ip = 1;
    }
    *res = r;
  }
  return nRet;
}

static inline int __devicelib_imf_internal_dlgamma(const double *a, double *r)
{
  int s;
  return __dlgamma_ep_internal_lgamma_r(a, r, &s);
}

} /* namespace */
} /* namespace __imf_impl_lgamma_d_ep */

DEVICE_EXTERN_C_INLINE double __devicelib_imf_lgamma(double x)
{
  using namespace __imf_impl_lgamma_d_ep;
  double r;
  __devicelib_imf_internal_dlgamma(&x, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
