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
/*
// ALGORITHM DESCRIPTION:
//   ---------------------
//
//   Case 2^13 <= x < OVERFLOW_BOUNDARY
//   ----------------------------------
//     Here we use algorithm based on the Stirling formula:
//       ln(GAMMA(x)) = ln(sqrt(2*Pi)) + (x-0.5)*ln(x) - x
//
//   Case 1 < x < 2^13
//   -----------------
//     To calculate ln(GAMMA(x)) for such arguments we use polynomial
//     approximation on following intervals: [1.0; 1.25), [1.25; 1.5),
//     [1.5, 1.75), [1.75; 2), [2; 4), [2^i; 2^(i+1)), i=1..8
//
//     Following variants of approximation and argument reduction are used:
//      1. [1.0; 1.25)
//         ln(GAMMA(x)) ~ (x-1.0)*P7(x)
//
//      2. [1.25; 1.5)
//         ln(GAMMA(x)) ~ ln(GAMMA(x0))+(x-x0)*P7(x-x0),
//         where x0 - point of local minimum on [1;2] rounded to nearest double
//         precision number.
//
//      3. [1.5; 1.75)
//         ln(GAMMA(x)) ~ P8(x)
//
//      4. [1.75; 2.0)
//         ln(GAMMA(x)) ~ (x-2)*P7(x)
//
//      5. [2; 4)
//         ln(GAMMA(x)) ~ (x-2)*P10(x)
//
//      6. [2^i; 2^(i+1)), i=2..8
//         ln(GAMMA(x)) ~ P10((x-2^i)/2^i)
//
//   Case -9 < x < 1
//   ---------------
//     Here we use the recursive formula:
//     ln(GAMMA(x)) = ln(GAMMA(x+1)) - ln(x)
//
//     Using this formula we reduce argument to base interval [1.0; 2.0]
//
//   Case -2^13 < x < -9
//   --------------------
//     Here we use the formula:
//     ln(GAMMA(x)) = ln(Pi/(|x|*GAMMA(|x|)*sin(Pi*|x|))) =
//     = -ln(|x|) - ln((GAMMA(|x|)) - ln(sin(Pi*r)/(Pi*r)) - ln(|r|)
//     where r = x - rounded_to_nearest(x), i.e |r| <= 0.5 and
//     ln(sin(Pi*r)/(Pi*r)) is approximated by 8-degree polynomial of r^2
//
//   Case x < -2^13
//   --------------
//     Here we use algorithm based on the Stirling formula:
//     ln(GAMMA(x)) = -ln(sqrt(2*Pi)) + (|x|-0.5)ln(x) - |x| -
//     - ln(sin(Pi*r)/(Pi*r)) - ln(|r|)
//     where r = x - rounded_to_nearest(x).
//
//   Neighbourhoods of negative roots
//   --------------------------------
//     Here we use polynomial approximation
//     ln(GAMMA(x-x0)) = ln(GAMMA(x0)) + (x-x0)*P14(x-x0),
//     where x0 is a root of ln(GAMMA(x)) rounded to nearest double
//     precision number.
//
//
//   Claculation of logarithm
//   ------------------------
//     Consider  x = 2^N * xf so
//     ln(x) = ln(frcpa(x)*x/frcpa(x))
//           = ln(1/frcpa(x)) + ln(frcpa(x)*x)
//
//     frcpa(x) = 2^(-N) * frcpa(xf)
//
//     ln(1/frcpa(x)) = -ln(2^(-N)) - ln(frcpa(xf))
//                    = N*ln(2) - ln(frcpa(xf))
//                    = N*ln(2) + ln(1/frcpa(xf))
//
//     ln(x) = ln(1/frcpa(x)) + ln(frcpa(x)*x) =
//           = N*ln(2) + ln(1/frcpa(xf)) + ln(frcpa(x)*x)
//           = N*ln(2) + T + ln(frcpa(x)*x)
//
//     Let r = 1 - frcpa(x)*x, note that r is quite small by
//     absolute value so
//
//     ln(x) = N*ln(2) + T + ln(1+r) ~ N*ln(2) + T + Series(r),
//     where T - is precomputed tabular value,
//     Series(r) = (P3*r + P2)*r^2 + (P1*r + 1)
//
// --
//
*/
#include "_imf_include_fp32.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_lgamma_s_ep {
namespace {
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g15 = {0x376b77ddu};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g15l = {0xaaa92ad4u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g14 = {0xb79b7330u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g14l = {0x2abdc4e4u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g13 = {0x37da5593u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g13l = {0xa9885c19u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g12 = {0xb87a94deu};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g12l = {0x2a87d43cu};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g11 = {0x390103e6u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g11l = {0xac469c4du};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g10 = {0xb97cd244u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g10l = {0x2cad915eu};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g9 = {0x3a007e85u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g9l = {0xadbf8abdu};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g8 = {0xba710e94u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g8l = {0xade7bef8u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g7 = {0x3b09e279u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g7l = {0x2e9660c2u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g6 = {0xbb3af377u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g6l = {0xaedcfdefu};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g5 = {0x3c36bf74u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g5l = {0x2fda4f2au};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g4 = {0xb98bf9b9u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g4l = {0xad753339u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g3 = {0x3d980fdeu};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g3l = {0xb16f97eau};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g2 = {0x3da711cdu};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g2l = {0xb0f73b75u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g1 = {0x3ed2dcbcu};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g1l = {0x32396b72u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g0 = {0x3ed87730u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___g0l = {0x31e4127au};
// using intepolation interval over [__fxmax+1, __fxmax+2]
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___fxmax = {0x3EEC5A00u};
// __fxmax+1
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___fxmax_p1 = {0x3FBB1680u};
// __fxmax+2
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___fxmax_p2 = {0x401D8B40u};
// log(1+x) polynomial approximation, |x|<1/3
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___lg9 = {0xbe08f385u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___lg8 = {0x3e10413fu};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___lg7 = {0xbdf6fb25u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___lg6 = {0x3e0f298eu};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___lg5 = {0xbe2aeb93u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___lg4 = {0x3e4ced61u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___lg3 = {0xbe7ffe80u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___lg2 = {0x3eaaaa70u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___lg1 = {0xbf000001u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___lg0 = {0x31875d06u};
// log(2.0)
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___L2 = {0x3f317218u};
//
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___small_thres = {0x01000000u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___scale_smallx = {0x5f800000u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___scale_corr = {0x42800000u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___Inf = {0x7f800000u};
// coefficients for log(GAMMA(8/z)*exp(8/z)/(8/z)^(8/z-.5))
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___p3 = {0xb6a9a594u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___p2 = {0xb5657785u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___p1 = {0x3c2aad91u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___p0 = {0x3f6b3f8bu};
// coefficients for log(Pi*x/sin(Pi*x)) approximation, 0<=x<=0.5
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___ls7 = {0x3fa7f315u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___ls6 = {0xbf904a6au};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___ls5 = {0x3f3d9587u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___ls4 = {0x3eb0ee9bu};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___ls3 = {0x3cdf47adu};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___ls2 = {0x3fd25105u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___ls1 = {0x38443173u};
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___ls0 = {0xb4515d7du};
// cutoff for multi-precision path
static const union {
  uint32_t w;
  float f;
} __slgamma_ep___main_thres = {0x41500126u};

inline int __devicelib_imf_internal_slgamma(const float *a, float *pres) {
  int nRet = 0;
  float x = *a;
  float fn, fx, Ph, Pl, x_next, Qh, Ql, f_expon, eps, lR, log_x, lpoly;
  float poly_h, poly_l, G0h, G0l, Gh, Gl, Rh, Rl, Kh, Kl, poly;
  float shalf, ls_poly, log_afx, p0;
  int_float iGh, iGl, R_h, R_l, escale, ix, afx, res;
  int32_t iexpon;
  uint32_t sgn;
  ix.f = x;
  // |x|
  ix.w &= 0x7fffffffu;
  // filter out +/-Inf inputs
  if (ix.w == 0x7f800000u) {
    *pres = ix.f;
    return nRet;
  }
  // integer and fractional parts of x
  fn = __rint(x);
  fx = x - fn;
  // Cutoff ix.f<16.0:  2.57 ulp
  // Cutoff ix.f<32.0:  2.01 ulp (worst input -0x1.2df3d6p+006)
  // Selected thresholds: 3.57 ulp
  if ((ix.f < __slgamma_ep___main_thres.f) && (x < 4.0f)) {
    // adjust fraction range to [__fxmax-1, __fxmax]
    fx = (fx <= __slgamma_ep___fxmax.f) ? fx : (fx - 1.0f);
    if (x <= __slgamma_ep___fxmax_p1.f) {
      // For x=N+fx (N<2), compute x*(x+1)*(x+2)* ...*fx*(fx+1)
      Ph = x_next = x;
      Pl = 0;
      // correction for very small |x|
      Ph = (__fabs(x) <= __slgamma_ep___small_thres.f)
               ? (Ph * __slgamma_ep___scale_smallx.f)
               : Ph;
      while (x_next <= __slgamma_ep___fxmax.f) {
        x_next = x_next + 1.0f;
        // (Ph + Pl)*= x_next
        {
          float __ph, __phl;
          __ph = __fma(Ph, x_next, 0.0f);
          __phl = __fma(Ph, x_next, -__ph);
          Pl = __fma(Pl, x_next, __phl);
          Ph = __ph;
        };
      }
      if (Ph == 0.0f) {
        *pres = __slgamma_ep___Inf.f;
        nRet = 2;
        return nRet;
      }
      // Qh + Ql = 1/(Ph+Pl)
      Qh = 1.0f / (Ph);
      eps = __fma(Qh, -Ph, 1.0f);
      eps = __fma(Qh, -Pl, eps);
      // Ql = eps*(1+eps)*Qh
      Ql = __fma(eps, __fma(eps, Qh, Qh),
                                  0.0f);
    } else {
      Qh = 1.0f;
      Ql = 0;
      x_next = x;
      while (x_next > __slgamma_ep___fxmax_p2.f) {
        x_next = x_next - 1.0f;
        // (Qh + Ql)*= x_next
        {
          float __ph, __phl;
          __ph = __fma(Qh, x_next, 0.0f);
          __phl = __fma(Qh, x_next, -__ph);
          Ql = __fma(Ql, x_next, __phl);
          Qh = __ph;
        };
      }
    }
    // _EP_
    poly_h = __slgamma_ep___g15.f;
    poly_h = __fma(poly_h, fx, __slgamma_ep___g14.f);
    poly_h = __fma(poly_h, fx, __slgamma_ep___g13.f);
    poly_h = __fma(poly_h, fx, __slgamma_ep___g12.f);
    poly_h = __fma(poly_h, fx, __slgamma_ep___g11.f);
    poly_h = __fma(poly_h, fx, __slgamma_ep___g10.f);
    poly_h = __fma(poly_h, fx, __slgamma_ep___g9.f);
    poly_h = __fma(poly_h, fx, __slgamma_ep___g8.f);
    poly_h = __fma(poly_h, fx, __slgamma_ep___g7.f);
    poly_h = __fma(poly_h, fx, __slgamma_ep___g6.f);
    poly_h = __fma(poly_h, fx, __slgamma_ep___g5.f);
    poly_h = __fma(poly_h, fx, __slgamma_ep___g4.f);
    poly_l = 0;
    {
      float __ph, __phl;
      __ph = __fma(poly_h, fx, 0.0f);
      __phl = __fma(poly_h, fx, -__ph);
      poly_l = __fma(poly_l, fx, __phl);
      poly_h = __ph;
    };
    {
      float __ph, __ahl, __ahh;
      __ph = __fma(poly_h, 1.0f, __slgamma_ep___g3.f);
      __ahh = __ph - __slgamma_ep___g3.f;
      __ahl = poly_h - __ahh;
      poly_l = (poly_l + __slgamma_ep___g3l.f) + __ahl;
      poly_h = __ph;
    };
    {
      float __ph, __phl;
      __ph = __fma(poly_h, fx, 0.0f);
      __phl = __fma(poly_h, fx, -__ph);
      poly_l = __fma(poly_l, fx, __phl);
      poly_h = __ph;
    };
    {
      float __ph, __ahl, __ahh;
      __ph = __fma(poly_h, 1.0f, __slgamma_ep___g2.f);
      __ahh = __ph - __slgamma_ep___g2.f;
      __ahl = poly_h - __ahh;
      poly_l = (poly_l + __slgamma_ep___g2l.f) + __ahl;
      poly_h = __ph;
    };
    {
      float __ph, __phl;
      __ph = __fma(poly_h, fx, 0.0f);
      __phl = __fma(poly_h, fx, -__ph);
      poly_l = __fma(poly_l, fx, __phl);
      poly_h = __ph;
    };
    {
      float __ph, __ahl, __ahh;
      __ph = __fma(poly_h, 1.0f, __slgamma_ep___g1.f);
      __ahh = __ph - __slgamma_ep___g1.f;
      __ahl = poly_h - __ahh;
      poly_l = (poly_l + __slgamma_ep___g1l.f) + __ahl;
      poly_h = __ph;
    };
    {
      float __ph, __phl;
      __ph = __fma(poly_h, fx, 0.0f);
      __phl = __fma(poly_h, fx, -__ph);
      poly_l = __fma(poly_l, fx, __phl);
      poly_h = __ph;
    };
    {
      float __ph, __ahl, __ahh;
      __ph = __fma(poly_h, 1.0f, __slgamma_ep___g0.f);
      __ahh = __ph - __slgamma_ep___g0.f;
      __ahl = poly_h - __ahh;
      poly_l = (poly_l + __slgamma_ep___g0l.f) + __ahl;
      poly_h = __ph;
    };
    // 1 + fx*(poly_h+poly_l)
    {
      float __ph, __phl;
      __ph = __fma(poly_h, fx, 0.0f);
      __phl = __fma(poly_h, fx, -__ph);
      poly_l = __fma(poly_l, fx, __phl);
      poly_h = __ph;
    };
    {
      float __ph, __ahl, __ahh;
      __ph = __fma(poly_h, 1.0f, 1.0f);
      __ahh = __ph - 1.0f;
      __ahl = poly_h - __ahh;
      poly_l = (poly_l + 0.0f) + __ahl;
      poly_h = __ph;
    };
    // (Qh + Ql)*(poly_h+poly_l)
    {
      G0h = __fma(poly_h, Qh, 0.0f);
      G0l = __fma(poly_h, Qh, -G0h);
      G0l = __fma(poly_l, Qh, G0l);
      G0l = __fma(poly_h, Ql, G0l);
    };
    // (Gh + Gl) == (G0h + G0l)
    {
      float __alh;
      Gh = __fma(G0h, 1.0f, G0l);
      __alh = Gh - G0h;
      Gl = G0l - __alh;
    };
    // compute log(|Gh + Gl|)
    iGh.f = Gh;
    iGl.f = Gl;
    // |Gh + Gl|
    sgn = iGh.w & 0x80000000u;
    iGh.w ^= sgn;
    iGl.w ^= sgn;
    // unbiased exponent, mantissa normalized to [2/3, 4/3]
    iexpon = (iGh.w + 0xc0d55555u);
    // signed shift
    iexpon >>= 23;
    R_h.w = iGh.w - (iexpon << 23);
    escale.w = (0x7f - iexpon) << 23;
    // reduced argument
    R_h.f = __fma(R_h.f, 1.0f, -1.0f);
    // R_l.f = iGl.f * escale.f;
    // Rh = FENCE(R_h.f + R_l.f);
    // combining ops
    Rh = __fma(iGl.f, escale.f, R_h.f);
    // polynomial
    poly_h = __slgamma_ep___lg9.f;
    poly_h = __fma(poly_h, Rh, __slgamma_ep___lg8.f);
    poly_h = __fma(poly_h, Rh, __slgamma_ep___lg7.f);
    poly_h = __fma(poly_h, Rh, __slgamma_ep___lg6.f);
    poly_h = __fma(poly_h, Rh, __slgamma_ep___lg5.f);
    poly_h = __fma(poly_h, Rh, __slgamma_ep___lg4.f);
    poly_h = __fma(poly_h, Rh, __slgamma_ep___lg3.f);
    poly_h = __fma(poly_h, Rh, __slgamma_ep___lg2.f);
    poly_h = __fma(poly_h, Rh, __slgamma_ep___lg1.f);
    poly_h = __fma(poly_h, Rh, __slgamma_ep___lg0.f);
    poly_h = __fma(poly_h, Rh, Rh);
    // unbiased_expon*log(2)
    Kh = __slgamma_ep___L2.f;
    f_expon = (float)iexpon;
    // add in correction for very small inputs
    f_expon = (__fabs(x) <= __slgamma_ep___small_thres.f)
                  ? (f_expon + __slgamma_ep___scale_corr.f)
                  : f_expon;
    poly_h = __fma(Kh, f_expon, poly_h);
    *pres = poly_h;
  } else {
    if (x < 0) {
      //
      afx.f = fx;
      afx.w &= 0x7fffffffu;
      // negative integer input?
      if (afx.f == 0.0f) {
        *pres = __slgamma_ep___Inf.f;
        nRet = 2;
        return nRet;
      }
      // approximate log(Pi*afx/sin(Pi*afx))
      ls_poly = __fma(afx.f, __slgamma_ep___ls7.f,
                                       __slgamma_ep___ls6.f);
      ls_poly = __fma(ls_poly, afx.f, __slgamma_ep___ls5.f);
      ls_poly = __fma(ls_poly, afx.f, __slgamma_ep___ls4.f);
      ls_poly = __fma(ls_poly, afx.f, __slgamma_ep___ls3.f);
      ls_poly = __fma(ls_poly, afx.f, __slgamma_ep___ls2.f);
      ls_poly = __fma(ls_poly, afx.f, __slgamma_ep___ls1.f);
      ls_poly = __fma(ls_poly, afx.f, __slgamma_ep___ls0.f);
      // log(afx.f)
      // unbiased exponent, mantissa normalized to [2/3, 4/3]
      iexpon = (afx.w + 0xc0d55555u);
      // signed shift
      iexpon >>= 23;
      R_h.w = afx.w - (iexpon << 23);
      // reduced argument
      lR = R_h.f - 1.0f;
      lpoly = __fma(lR, __slgamma_ep___lg9.f,
                                     __slgamma_ep___lg8.f);
      lpoly = __fma(lpoly, lR, __slgamma_ep___lg7.f);
      lpoly = __fma(lpoly, lR, __slgamma_ep___lg6.f);
      lpoly = __fma(lpoly, lR, __slgamma_ep___lg5.f);
      lpoly = __fma(lpoly, lR, __slgamma_ep___lg4.f);
      lpoly = __fma(lpoly, lR, __slgamma_ep___lg3.f);
      lpoly = __fma(lpoly, lR, __slgamma_ep___lg2.f);
      lpoly = __fma(lpoly, lR, __slgamma_ep___lg1.f);
      lpoly = __fma(lpoly, lR, __slgamma_ep___lg0.f);
      lpoly = __fma(lpoly, lR, lR);
      Kh = __slgamma_ep___L2.f;
      f_expon = (float)iexpon;
      log_afx = __fma(f_expon, Kh, lpoly);
    }
    // For x>0: lgamma(x) ~ (x-0.5)*log(x) - x + poly(1/x)
    // x<0:  lgamma(x) ~ log(Pi*afx/sin(Pi*afx)) - log(afx) - ((x+0.5)*log(x)- x
    // + poly(1/x))
    log_afx = (x < 0) ? log_afx : 0.0;
    ls_poly = (x < 0) ? ls_poly : 0.0f;
    shalf = (x < 0) ? 0.5f : -0.5f;
    // log(x)
    iGh.f = ix.f;
    // unbiased exponent, mantissa normalized to [2/3, 4/3]
    iexpon = (iGh.w + 0xc0d55555u);
    // signed shift
    iexpon >>= 23;
    R_h.w = iGh.w - (iexpon << 23);
    // reduced argument
    lR = R_h.f - 1.0f;
    lpoly =
        __fma(lR, __slgamma_ep___lg9.f, __slgamma_ep___lg8.f);
    lpoly = __fma(lpoly, lR, __slgamma_ep___lg7.f);
    lpoly = __fma(lpoly, lR, __slgamma_ep___lg6.f);
    lpoly = __fma(lpoly, lR, __slgamma_ep___lg5.f);
    lpoly = __fma(lpoly, lR, __slgamma_ep___lg4.f);
    lpoly = __fma(lpoly, lR, __slgamma_ep___lg3.f);
    lpoly = __fma(lpoly, lR, __slgamma_ep___lg2.f);
    lpoly = __fma(lpoly, lR, __slgamma_ep___lg1.f);
    lpoly = __fma(lpoly, lR, __slgamma_ep___lg0.f);
    lpoly = __fma(lpoly, lR, lR);
    Kh = __slgamma_ep___L2.f;
    f_expon = (float)iexpon;
    log_x = __fma(f_expon, Kh, lpoly);
    // 8/x (to avoid underflow)
    R_h.f = 1.0f / ((ix.f * 0.125f));
    poly =
        __fma(__slgamma_ep___p3.f, R_h.f, __slgamma_ep___p2.f);
    poly = __fma(poly, R_h.f, __slgamma_ep___p1.f);
    p0 = __slgamma_ep___p0.f;
    p0 -= ls_poly;
    poly = __fma(poly, R_h.f, p0);
    // shalf*log(x) +poly(1/x)
    poly = __fma(log_x, shalf, poly);
    // shalf*log(x) +poly(1/x) + log_afx
    poly = __fma(poly, 1.0f, log_afx);
    // shalf*log(x) - x + poly(1/x) + log_afx
    res.f = __fma(poly, 1.0f, (-ix.f));
    // (x+shalf)*log(x) - x + poly(1/x)
    res.f = __fma(ix.f, log_x, res.f);
    res.f = (x < 0) ? (-res.f) : (res.f);
    nRet = (res.w == 0x7f800000u) ? 3 : nRet;
    *pres = res.f;
  }
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_lgamma_s_ep */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_lgammaf(float a) {
  using namespace __imf_impl_lgamma_s_ep;
  float r;
  __devicelib_imf_internal_slgamma(&a, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
