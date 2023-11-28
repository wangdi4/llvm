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
//  The method consists of three cases.
//
//  If       2 <= x < OVERFLOW_BOUNDARY
//  else if  0 < x < 2
//  else if  -(i+1) <  x < -i, i = 0...43
//
//  Case 2 <= x < OVERFLOW_BOUNDARY
//  -------------------------------
//    Here we use algorithm based on the recursive formula
//    GAMMA(x+1) = x*GAMMA(x). For that we subdivide interval
//    [2; OVERFLOW_BOUNDARY] into intervals [8*n; 8*(n+1)] and
//    approximate GAMMA(x) by polynomial of 22th degree on each
//    [8*n; 8*n+1], recursive formula is used to expand GAMMA(x)
//    to [8*n; 8*n+1]. In other words we need to find n, i and r
//    such that x = 8 * n + i + r where n and i are integer numbers
//    and r is fractional part of x. So GAMMA(x) = GAMMA(8*n+i+r) =
//    = (x-1)*(x-2)*...*(x-i)*GAMMA(x-i) =
//    = (x-1)*(x-2)*...*(x-i)*GAMMA(8*n+r) ~
//    ~ (x-1)*(x-2)*...*(x-i)*P12n(r).
//
//    Step 1: Reduction
//    -----------------
//     N = [x] with truncate
//     r = x - N, note 0 <= r < 1
//
//     n = N & ~0x7 - index of table that contains coefficient of
//                    polynomial approximation
//     i = N & 0x7  - is used in recursive formula
//
//
//    Step 2: Approximation
//    ---------------------
//     We use factorized minimax approximation polynomials
//     P12n(r) = A12*(r^2+C01(n)*r+C00(n))*
//               *(r^2+C11(n)*r+C10(n))*...*(r^2+C51(n)*r+C50(n))
//
//    Step 3: Recursion
//    -----------------
//     In case when i > 0 we need to multiply P12n(r) by product
//     R(i,x)=(x-1)*(x-2)*...*(x-i). To reduce number of fp-instructions
//     we can calculate R as follow:
//     R(i,x) = ((x-1)*(x-2))*((x-3)*(x-4))*...*((x-(i-1))*(x-i)) if i is
//     even or R = ((x-1)*(x-2))*((x-3)*(x-4))*...*((x-(i-2))*(x-(i-1)))*
//     *(i-1) if i is odd. In both cases we need to calculate
//     R2(i,x) = (x^2-3*x+2)*(x^2-7*x+12)*...*(x^2+x+2*j*(2*j-1)) =
//     = ((x^2-x)+2*(1-x))*((x^2-x)+6*(2-x))*...*((x^2-x)+2*(2*j-1)*(j-x)) =
//     = (RA+2*RB)*(RA+6*(1-RB))*...*(RA+2*(2*j-1)*(j-1+RB))
//     where j = 1..[i/2], RA = x^2-x, RB = 1-x.
//
//    Step 4: Reconstruction
//    ----------------------
//     Reconstruction is just simple multiplication i.e.
//     GAMMA(x) = P12n(r)*R(i,x)
//
//  Case 0 < x < 2
//  --------------
//     To calculate GAMMA(x) on this interval we do following
//         if 1.0  <= x < 1.25  than  GAMMA(x) = P7(x-1)
//         if 1.25 <= x < 1.5   than  GAMMA(x) = P7(x-x_min) where
//               x_min is point of local minimum on [1; 2] interval.
//         if 1.5  <= x < 1.75  than  GAMMA(x) = P7(x-1)
//         if 1.75 <= x < 2.0   than  GAMMA(x) = P7(x-1)
//     and
//         if 0 < x < 1 than GAMMA(x) = GAMMA(x+1)/x
//
//  Case -(i+1) <  x < -i, i = 0...43
//  ----------------------------------
//     Here we use the fact that GAMMA(-x) = PI/(x*GAMMA(x)*sin(PI*x)) and
//     so we need to calculate GAMMA(x), sin(PI*x)/PI. Calculation of
//     GAMMA(x) is described above.
//
//    Step 1: Reduction
//    -----------------
//     Note that period of sin(PI*x) is 2 and range reduction for
//     sin(PI*x) is like to range reduction for GAMMA(x)
//     i.e rs = x - round(x) and |rs| <= 0.5.
//
//    Step 2: Approximation
//    ---------------------
//     To approximate sin(PI*x)/PI = sin(PI*(2*n+rs))/PI =
//     = (-1)^n*sin(PI*rs)/PI Taylor series is used.
//     sin(PI*rs)/PI ~ S17(rs).
//
//    Step 3: Division
//    ----------------
//     1/x and 1/(GAMMA(x)*S12(rs))
// --
//
*/
#include "_imf_include_fp32.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_tgamma_s_la {
namespace {

static const union {
  uint32_t w;
  float f;
} __stgamma_la___g9 = {0xbab6176du};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___g9l = {0xae714a6cu};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___g8 = {0x3b1d38b7u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___g8l = {0x2dded6e2u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___g7 = {0xbb2eb2f6u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___g7l = {0xae5beafbu};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___g6 = {0x3c35c4efu};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___g6l = {0xafdb433bu};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___g5 = {0xb99a96dau};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___g5l = {0xacf746afu};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___g4 = {0x3d98127du};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___g4l = {0xb16eb15bu};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___g3 = {0x3da712a1u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___g3l = {0x3118f2c2u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___g2 = {0x3ed2dcb7u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___g2l = {0x3220405du};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___g1 = {0x3ed8772fu};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___g1l = {0x324fa556u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___g0 = {0x3f800000u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___g0l = {0x304df064u};
// log2() coefficients
static const union {
  uint32_t w;
  float f;
} __stgamma_la___lg10 = {0x3e33f722u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___lg10l = {0xb1e71721u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___lg9 = {0xbe40bc25u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___lg9l = {0xb0303e2eu};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___lg8 = {0x3e1e1fe6u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___lg8l = {0x319418ffu};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___lg7 = {0xbe335ec0u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___lg7l = {0xb189b659u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___lg6 = {0x3e536662u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___lg6l = {0xb10a9fcfu};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___lg5 = {0xbe767c1eu};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___lg5l = {0x31024e0eu};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___lg4 = {0x3e93ba33u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___lg4l = {0xb27d981au};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___lg3 = {0xbeb8a98cu};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___lg3l = {0x31099373u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___lg2 = {0x3ef63852u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___lg2l = {0xb1f29daeu};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___lg1 = {0xbf38aa3cu};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___lg1l = {0x32b1212fu};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___lg0 = {0x3fb8aa3bu};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___lg0l = {0x32a451aeu};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___mL2E = {0xbfb8aa3bu};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___mL2EL = {0xb2a57060u};

// using intepolation interval over [__fxmax+1, __fxmax+2]
static const union {
  uint32_t w;
  float f;
} __stgamma_la___fxmax = {0x3EEC5A00u};
// __fxmax+1
static const union {
  uint32_t w;
  float f;
} __stgamma_la___fxmax_p1 = {0x3FBB1680u};
// __fxmax+2
static const union {
  uint32_t w;
  float f;
} __stgamma_la___fxmax_p2 = {0x401D8B40u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___small_thres = {0x01000000u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___scale_smallx = {0x5f800000u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___scale_corr = {0x42800000u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___qnan_indef = {0xffc00000u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___Inf = {0x7f800000u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___Shifter = {0x4b40007fu};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___minShifter = {0x4b400000u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___maxShifter = {0x4b4000f0u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___ce8 = {0x39b3ff0fu};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___ce7 = {0x3b01e213u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___ce6 = {0x3c217f67u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___ce5 = {0x3d2ebe2fu};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___ce4 = {0x3e1d9564u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___ce4l = {0xb110a789u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___ce3 = {0x3ee35854u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___ce3l = {0xb230eac0u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___ce2 = {0x3f75fdf0u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___ce2l = {0xb1bc3d42u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___ce1 = {0x3fb17218u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___ce1l = {0xb23db304u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___ce0 = {0x3f800000u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___ce0l = {0x2df51500u};
// poly(z) ~ GAMMA(1/z)*exp(1/z)/(1/z)^(1/z-.5)
static const union {
  uint32_t w;
  float f;
} __stgamma_la___p5 = {0x3a9a5493u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___p4 = {0xb8dc3482u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___p3 = {0xbbdfc448u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___p2 = {0x3c0ec610u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___p1 = {0x3e55e608u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___p0 = {0x3f01b264u};
// poly(r) ~ sin(pi*r)/(pi*r)
static const union {
  uint32_t w;
  float f;
} __stgamma_la___sp5 = {0xbb127cd6u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___sp5l = {0xae3fb1cau};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___sp4 = {0x3cd5f1a7u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___sp4l = {0xae78ae4fu};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___sp3 = {0xbe435358u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___sp3l = {0xb1898a74u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___sp2 = {0x3f4fce56u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___sp2l = {0xb28778f2u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___sp1 = {0xbfd28d33u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___sp1l = {0xb14abc67u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___sp0 = {0x3f800000u};
static const union {
  uint32_t w;
  float f;
} __stgamma_la___sp0l = {0xadba0bc0u};

inline int __devicelib_imf_internal_stgamma(const float *px, float *pres) 
{
  int nRet = 0;
  float x = *px;
  float fn, fx, Ph, Pl, x_next, Qh, Ql, fx0, eps, lR, lpoly;
  float poly_h, poly_l, G0h, G0l, Gh, Gl, Rh, Rl, Kh, poly, res;
  float zr_h, zr_l, spoly_h, spoly_l, p0, fN, y, xL_h, xL_l;
  union {
    uint32_t w;
    float f;
  } iGh, iGl, R_h, R_l, ix, ix0, fS, ires;
  int32_t iexpon, iN, sgn;
  ix0.f = ix.f = x;
  // |x|
  ix.w &= 0x7fffffffu;
  // filter out Inf/NaN inputs
  if (ix.w >= 0x7f800000u) {
    *pres = x + __stgamma_la___Inf.f;
    nRet = (ix0.w != 0xff800000) ? nRet : 1;
    return nRet;
  }
  // integer and fractional parts of x
  fn = __rint(x);
  fx0 = x - fn;
  if (ix.f < __stgamma_la___fxmax_p2.f) {
    // adjust fraction range to [__fxmax-1, __fxmax]
    fx = (fx0 <= __stgamma_la___fxmax.f) ? fx0 : (fx0 - 1.0f);
    Qh = 1.0f;
    Ql = 0;
    if (x <= __stgamma_la___fxmax_p1.f) {
      if (fx0 == 0.0f) {
        *pres = ((x < 0) ? __stgamma_la___qnan_indef.f : 1.0f / (x));
        nRet = (x < 0) ? 1 : nRet;
        nRet = (x == 0.0f) ? 2 : nRet;
        return nRet;
      }
      // For x=N+fx (N<2), compute x*(x+1)*(x+2)* ...*fx*(fx+1)
      Ph = x_next = x;
      Pl = 0;
      // correction for very small |x|
      Ph = (__fabs(x) <= __stgamma_la___small_thres.f)
               ? (Ph * __stgamma_la___scale_smallx.f)
               : Ph;
      while (x_next <= __stgamma_la___fxmax.f) {
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
      // Qh + Ql = 1/(Ph+Pl)
      Qh = 1.0f / (Ph);
      eps = __fma(Qh, -Ph, 1.0f);
      eps = __fma(Qh, -Pl, eps);
      // Ql = eps*(1+eps)*Qh
      Ql = __fma(eps, __fma(eps, Qh, Qh),
                                  0.0f);
    }
    // polynomial evaluation
    poly_h = __stgamma_la___g9.f;
    poly_h = __fma(poly_h, fx, __stgamma_la___g8.f);
    poly_h = __fma(poly_h, fx, __stgamma_la___g7.f);
    poly_h = __fma(poly_h, fx, __stgamma_la___g6.f);
    poly_h = __fma(poly_h, fx, __stgamma_la___g5.f);
    poly_h = __fma(poly_h, fx, __stgamma_la___g4.f);
    poly_h = __fma(poly_h, fx, __stgamma_la___g3.f);
    lpoly = __fma(poly_h, fx, __stgamma_la___g2.f);
    poly_h = __fma(lpoly, fx, __stgamma_la___g1.f);
    poly_h = __fma(poly_h, fx, __stgamma_la___g0.f);
    res = __fma(Qh, poly_h,
                                 __fma(Ql, poly_h, 0.0f));
    res = (__fabs(x) <= __stgamma_la___small_thres.f)
              ? (res * __stgamma_la___scale_smallx.f)
              : res;
    *pres = res;
    return nRet;
  }
  // separating HA/LA and EP paths here, for better readability
  else // |x| > = __fxmax_p2 ~ 2.46
  {
    // Compute x*log(x) - 0.5*log(x) - x
    // log(x)
    iGh.f = ix.f;
    // unbiased exponent, mantissa normalized to [2/3, 4/3]
    iexpon = (iGh.w + 0xc0d55555u);
    // signed shift
    iexpon >>= 23;
    R_h.w = iGh.w - (iexpon << 23);
    // reduced argument
    lR = R_h.f - 1.0f;
    // unbiased_expon
    Kh = (float)iexpon;
    // log2 polynomial
    poly_h = __stgamma_la___lg10.f;
    poly_h = __fma(poly_h, lR, __stgamma_la___lg9.f);
    poly_h = __fma(poly_h, lR, __stgamma_la___lg8.f);
    poly_h = __fma(poly_h, lR, __stgamma_la___lg7.f);
    poly_h = __fma(poly_h, lR, __stgamma_la___lg6.f);
    poly_h = __fma(poly_h, lR, __stgamma_la___lg5.f);
    poly_h = __fma(poly_h, lR, __stgamma_la___lg4.f);
    poly_h = __fma(poly_h, lR, __stgamma_la___lg3.f);
    poly_h = __fma(poly_h, lR, __stgamma_la___lg2.f);
    Ph = __fma(poly_h, lR, 0.0f);
    poly_l = __fma(poly_h, lR, -Ph);
    poly_h = Ph;
    {
      float __ph, __ahl, __ahh;
      __ph = __fma(poly_h, 1.0f, __stgamma_la___lg1.f);
      __ahh = __fma(__ph, 1.0f, -__stgamma_la___lg1.f);
      __ahl = __fma(poly_h, 1.0f, -__ahh);
      poly_l = (poly_l + __stgamma_la___lg1l.f) + __ahl;
      poly_h = __ph;
    };
    {
      float __ph, __phl;
      __ph = __fma(poly_h, lR, 0.0f);
      __phl = __fma(poly_h, lR, -__ph);
      poly_l = __fma(poly_l, lR, __phl);
      poly_h = __ph;
    };
    {
      float __ph, __ahl, __ahh;
      __ph = __fma(poly_h, 1.0f, __stgamma_la___lg0.f);
      __ahh = __fma(__ph, 1.0f, -__stgamma_la___lg0.f);
      __ahl = __fma(poly_h, 1.0f, -__ahh);
      poly_l = (poly_l + __stgamma_la___lg0l.f) + __ahl;
      poly_h = __ph;
    };
    {
      float __ph, __phl;
      __ph = __fma(poly_h, lR, 0.0f);
      __phl = __fma(poly_h, lR, -__ph);
      poly_l = __fma(poly_l, lR, __phl);
      poly_h = __ph;
    };
    {
      float __ph, __ahl, __ahh;
      __ph = __fma(poly_h, 1.0f, Kh);
      __ahh = __fma(__ph, 1.0f, -Kh);
      __ahl = __fma(poly_h, 1.0f, -__ahh);
      poly_l = poly_l + __ahl;
      poly_h = __ph;
    };
    // x*log2(x)
    // clamp ix.f
    ix.f = __fmin(ix.f, 512.0f);
    xL_h = poly_h;
    xL_l = poly_l;
    {
      float __ph, __phl;
      __ph = __fma(xL_h, ix.f, 0.0f);
      __phl = __fma(xL_h, ix.f, -__ph);
      xL_l = __fma(xL_l, ix.f, __phl);
      xL_h = __ph;
    };
    // -0.5*log2(x)
    poly_h = __fma(poly_h, -0.5f, 0.0f);
    poly_l = __fma(poly_l, -0.5f, 0.0f);
    // -x *log2(e)
    Rh = __fma(ix.f, __stgamma_la___mL2E.f, 0.0f);
    Rl = __fma(ix.f, __stgamma_la___mL2E.f, -Rh);
    Rl = __fma(ix.f, __stgamma_la___mL2EL.f, Rl);
    {
      float __ph, __ahl, __ahh;
      __ph = __fma(poly_h, 1.0f, Rh);
      __ahh = __fma(__ph, 1.0f, -Rh);
      __ahl = __fma(poly_h, 1.0f, -__ahh);
      poly_l = (poly_l + Rl) + __ahl;
      poly_h = __ph;
    };
    // x*log(x) - 0.5*log(x) - x
    {
      float __ph, __ahl, __ahh;
      float __ah, __bh;
      __bh = (__fabs(poly_h) <= __fabs(xL_h))
                 ? (xL_h)
                 : (poly_h);
      __ah = (__fabs(poly_h) <= __fabs(xL_h))
                 ? (poly_h)
                 : (xL_h);
      __ph = __fma(__ah, 1.0f, __bh);
      __ahh = __fma(__ph, 1.0f, -__bh);
      __ahl = __fma(__ah, 1.0f, -__ahh);
      poly_l = (poly_l + xL_l) + __ahl;
      poly_h = __ph;
    };
    {
      float __alh;
      xL_h = __fma(poly_h, 1.0f, poly_l);
      __alh = __fma(xL_h, 1.0f, -poly_h);
      xL_l = __fma(poly_l, 1.0f, -__alh);
    };
    // 2^(xL_h + xL_l)
    fS.f = __fma(xL_h, 0.5f, __stgamma_la___Shifter.f);
    fN = __fma(fS.f, 1.0f, -(__stgamma_la___Shifter.f));
    // reduced argument
    R_h.f = __fma(xL_h, 0.5f, -fN);
    Rh = __fma(xL_l, 0.5f, R_h.f);
    // clamp range of integer exponent
    fS.f = __fmin(fS.f, __stgamma_la___maxShifter.f);
    fS.f = __fmax(fS.f, __stgamma_la___minShifter.f);
    fS.w <<= 23;
    // exp2 polynomial
    poly_h = __stgamma_la___ce8.f;
    poly_h = __fma(poly_h, Rh, __stgamma_la___ce7.f);
    poly_h = __fma(poly_h, Rh, __stgamma_la___ce6.f);
    poly_h = __fma(poly_h, Rh, __stgamma_la___ce5.f);
    poly_h = __fma(poly_h, Rh, __stgamma_la___ce4.f);
    poly_h = __fma(poly_h, Rh, __stgamma_la___ce3.f);
    poly_h = __fma(poly_h, Rh, __stgamma_la___ce2.f);
    lpoly = __fma(poly_h, Rh, __stgamma_la___ce1.f);
    /// 	poly_h = SP_FMA(lpoly, Rh, _VSTATIC(__ce0).f);
    // Boost accuracy
    poly_h = __fma(lpoly, Rh, 0.0f);
    poly_l = __fma(lpoly, Rh, -poly_h);
    // poly(1/x) ~ GAMMA(x)*exp(x)/(x)^(x-.5)
    y = 1.0f / (ix.f);
    poly = __stgamma_la___p5.f;
    poly = __fma(poly, y, __stgamma_la___p4.f);
    poly = __fma(poly, y, __stgamma_la___p3.f);
    poly = __fma(poly, y, __stgamma_la___p2.f);
    poly = __fma(poly, y, __stgamma_la___p1.f);
    poly = __fma(poly, y, __stgamma_la___p0.f);
    // (2+poly)*(1+poly_h+poly_l)
    ////
    // Boost accuracy
    poly_l += poly_l;
    res = __fma(poly_h, poly, poly_l);
    res = __fma(res, 1.0f, poly);
    poly_h += poly_h;
    res = __fma(res, 1.0f, poly_h);
    res = __fma(res, 1.0f, 2.0f);
    /////
    if (x < 0) {
      // tgamma(-x)= 1/(tgamma(x)*x*frac(x)*(sin(pi*frac(x))/(pi*frac(x)))
      // sin(pi*frac(x)/(pi*frac(x))
      spoly_h = __stgamma_la___sp5.f;
      Rh = __fma(fx0, fx0, 0.0f);
      Rl = __fma(fx0, fx0, -Rh);
      spoly_h = __fma(spoly_h, Rh, __stgamma_la___sp4.f);
      spoly_h = __fma(spoly_h, Rh, __stgamma_la___sp3.f);
      lpoly = __fma(spoly_h, Rh, __stgamma_la___sp2.f);
      spoly_h = __fma(lpoly, Rh, 0.0f);
      spoly_l = __fma(lpoly, Rh, -spoly_h);
      spoly_l = __fma(lpoly, Rl, spoly_l);
      {
        float __ph, __ahl, __ahh;
        __ph = __fma(spoly_h, 1.0f, __stgamma_la___sp1.f);
        __ahh = __fma(__ph, 1.0f, -__stgamma_la___sp1.f);
        __ahl = __fma(spoly_h, 1.0f, -__ahh);
        spoly_l = (spoly_l + __stgamma_la___sp1l.f) + __ahl;
        spoly_h = __ph;
      };
      {
        float __ph, __phl;
        __ph = __fma(spoly_h, Rh, 0.0f);
        __phl = __fma(spoly_h, Rh, -__ph);
        spoly_l = __fma(spoly_l, Rh, __phl);
        spoly_l = __fma(spoly_h, Rl, spoly_l);
        spoly_h = __ph;
      };
      {
        float __ph, __ahl, __ahh;
        __ph = __fma(spoly_h, 1.0f, __stgamma_la___sp0.f);
        __ahh = __fma(__ph, 1.0f, -__stgamma_la___sp0.f);
        __ahl = __fma(spoly_h, 1.0f, -__ahh);
        spoly_l = (spoly_l + __stgamma_la___sp0l.f) + __ahl;
        spoly_h = __ph;
      };
      // |x|*frac(x)
      zr_h = __fma(ix.f, fx0, 0.0f);
      zr_l = __fma(ix.f, fx0, -zr_h);
      // x*frac(x)*(sin(pi*frac(x))/(pi*frac(x))
      {
        float __ph, __phl;
        __ph = __fma(spoly_h, zr_h, 0.0f);
        __phl = __fma(spoly_h, zr_h, -__ph);
        spoly_l = __fma(spoly_l, zr_h, __phl);
        spoly_l = __fma(spoly_h, zr_l, spoly_l);
        spoly_h = __ph;
      };
      // tgamma(x)*x*frac(x)*(sin(pi*frac(x))/(pi*frac(x))
      R_h.f = __fma(
          spoly_h, res, __fma(spoly_l, res, 0.0f));
      // adjust sign of sin(pi*x)
      iN = (int)fn;
      sgn = iN << 31;
      R_h.w ^= sgn;
      res = 1.0f / (R_h.f);
      eps = __fma(-res, R_h.f, 1.0f);
      res = __fma(res, eps, res);
      // for negative integer inputs
      nRet = (fx0 == 0.0f) ? 1 : nRet;
      // adjust scale factor
      fS.w = 0x7f000000 - fS.w;
    }
    // final scaling
    res = __fma(res, fS.f, 0.0f);
    ires.f = res = __fma(res, fS.f, 0.0f);
    // overflow ?
    nRet = ((ires.w & 0x7fffffffu) == 0x7f800000u) ? 3 : nRet;
    // underflow ?
    nRet = (res == 0.0f) ? 4 : nRet;
    *pres = res;
    return nRet;
  }
  *pres = res;
  return nRet;
}

} /* namespace */
} /* namespace __imf_impl_tgamma_s_la */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_tgammaf(float x) 
{
  using namespace __imf_impl_tgamma_s_la;
  float r;
  __devicelib_imf_internal_stgamma(&x, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
