/*******************************************************************************
 * INTEL CONFIDENTIAL
 * Copyright 1996 Intel Corporation.
 *
 * This software and the related documents are Intel copyrighted  materials, and
 * your use of  them is  governed by the  express license  under which  they
 *were provided to you (License).  Unless the License provides otherwise, you
 *may not use, modify, copy, publish, distribute,  disclose or transmit this
 *software or the related documents without Intel's prior written permission.
 *
 * This software and the related documents  are provided as  is,  with no
 *express or implied  warranties,  other  than those  that are  expressly stated
 *in the License.
 *******************************************************************************/
#include "_imf_include_fp32.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_i1_s_ep {
namespace {
static const union {
  uint32_t w;
  float f;
} __si1_ep___c0_13 = {0x10beba82u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c0_13l = {0x8430b2e1u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c0_12 = {0x954238f8u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c0_12l = {0x07cd28ffu};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c0_11 = {0x19fe51e1u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c0_11l = {0x0d11c077u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c0_10 = {0x9cfdf2afu};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c0_10l = {0x906d2bbfu};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c0_9 = {0x22261164u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c0_9l = {0x953f9833u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c0_8 = {0x25fab973u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c0_8l = {0x989c1e7eu};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c0_7 = {0x2a302053u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c0_7l = {0x9d878469u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c0_6 = {0x2e12a1c2u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c0_6l = {0x21fa6d0bu};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c0_5 = {0x31c27ec6u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c0_5l = {0xa3e179bcu};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c0_4 = {0x3535ffb6u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c0_4l = {0xa8e2001cu};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c0_3 = {0x3863901au};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c0_3l = {0xab9ccdbcu};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c0_2 = {0x3b2aaa86u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c0_2l = {0xaeaa6253u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c0_1 = {0x3d800002u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c0_1l = {0x2f598d9au};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c0_0 = {0x3f000000u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c0_0l = {0xb185fccbu};
// coefficients for i1(x)/exp(x)*2^64,  |x| in [16, 39]
static const union {
  uint32_t w;
  float f;
} __si1_ep___c1_8 = {0x4960bdfcu};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c1_8l = {0xbcfa9716u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c1_7 = {0xcd5aff2bu};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c1_7l = {0xc0a2df83u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c1_6 = {0x50bc46f3u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c1_6l = {0xc3655348u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c1_5 = {0xd3bbb5e6u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c1_5l = {0x470970efu};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c1_4 = {0x566fdc59u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c1_4l = {0x499a4c98u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c1_3 = {0xd8ccef5cu};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c1_3l = {0x4bfc6e5cu};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c1_2 = {0x5aed99d5u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c1_2l = {0xcd8e62ceu};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c1_1 = {0xdcbca846u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c1_1l = {0xcfaa4befu};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c1_0 = {0x5e7554bfu};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c1_0l = {0xd1dfc8d5u};
// coefficients for i1(x)/exp(x)*2^64,  |x| in [39, 92]
static const union {
  uint32_t w;
  float f;
} __si1_ep___c2_8 = {0x442d2be6u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c2_8l = {0x37c406a2u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c2_7 = {0xc8c83c68u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c2_7l = {0xbc3bad87u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c2_6 = {0x4ccc3726u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c2_6l = {0xc01f9d9au};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c2_5 = {0xd0715cd9u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c2_5l = {0x43a9fec8u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c2_4 = {0x53b68f90u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c2_4l = {0x46f89ad3u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c2_3 = {0xd6b82af4u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c2_3l = {0xc982bc5cu};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c2_2 = {0x597aca78u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c2_2l = {0xccc5094bu};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c2_1 = {0xdbe7354cu};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c2_1l = {0x4f7e780du};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c2_0 = {0x5e292ff5u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___c2_0l = {0xd0a9eec6u};
// exp(R) coefficients
static const union {
  uint32_t w;
  float f;
} __si1_ep___ce7 = {0x3950eb8au};
static const union {
  uint32_t w;
  float f;
} __si1_ep___ce6 = {0x3ab6d3abu};
static const union {
  uint32_t w;
  float f;
} __si1_ep___ce5 = {0x3c08882eu};
static const union {
  uint32_t w;
  float f;
} __si1_ep___ce4 = {0x3d2aaa32u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___ce3 = {0x3e2aaaabu};
static const union {
  uint32_t w;
  float f;
} __si1_ep___ce2 = {0x3f000000u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___ce1 = {0x3f800000u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___ce0 = {0xae31e6e0u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___L2E = {0x3FB8AA3Bu};
static const union {
  uint32_t w;
  float f;
} __si1_ep___Shifter = {0x4b40003fu};
static const union {
  uint32_t w;
  float f;
} __si1_ep___L2H = {0x3f317218u};
static const union {
  uint32_t w;
  float f;
} __si1_ep___L2L = {0xb102e308u};
/////////////////////////////////////////////////////////////////////
//
//  Multi-precision macros (single precision)
//
/////////////////////////////////////////////////////////////////////
//  (Ah + Al) *= B
//  (Ah + Al) * B
//  (Ah + Al) *= (Bh + Bl)
//  (Ah + Al) += (Bh, Bl)
// |Bh| >= |Ah|
//  (Ah + Al) += Bh
// |Bh| >= |Ah|
//  (Ah + Al) += (Bh, Bl)
// no restrictions on A, B ordering
//  Rh + Rl = (Ah + Al) * (Bh + Bl)
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
inline int __devicelib_imf_internal_si1(const float *pa, float *pres) {
  int nRet = 0;
  float xin = *pa;
  union {
    uint32_t w;
    float f;
  } x, S, N, epoly, epoly_l, Te, RTe, H, L, LH, epoly2, sgn_x;
  float x2, poly, x2l, poly_l, R, R0, R1, R1h, Rl;
  float bc_h[14], bc_l[14];
  x.f = xin;
  // |xin|
  sgn_x.w = x.w & 0x80000000u;
  x.w ^= sgn_x.w;
  if (x.f <= 16.0f) {
    x2 = __fma(xin, xin, 0.0f);
    if (x.f <= 11.0f) {
      poly = __fma(x2, __si1_ep___c0_13.f, __si1_ep___c0_12.f);
      poly = __fma(poly, x2, __si1_ep___c0_11.f);
      poly = __fma(poly, x2, __si1_ep___c0_10.f);
      poly = __fma(poly, x2, __si1_ep___c0_9.f);
      poly = __fma(poly, x2, __si1_ep___c0_8.f);
      poly = __fma(poly, x2, __si1_ep___c0_7.f);
      poly = __fma(poly, x2, __si1_ep___c0_6.f);
      poly = __fma(poly, x2, __si1_ep___c0_5.f);
      poly = __fma(poly, x2, __si1_ep___c0_4.f);
      poly = __fma(poly, x2, __si1_ep___c0_3.f);
      poly = __fma(poly, x2, __si1_ep___c0_2.f);
      poly = __fma(poly, x2, __si1_ep___c0_1.f);
      poly = __fma(poly, x2, __si1_ep___c0_0.f);
      *pres = __fma(poly, xin, 0.0f);
      return nRet;
    } else // 11 .. 16.0
    {
      x2l = __fma(xin, xin, -x2);
      poly = __fma(x2, __si1_ep___c0_13.f, __si1_ep___c0_12.f);
      poly = __fma(poly, x2, __si1_ep___c0_11.f);
      poly = __fma(poly, x2, __si1_ep___c0_10.f);
      poly = __fma(poly, x2, __si1_ep___c0_9.f);
      poly = __fma(poly, x2, __si1_ep___c0_8.f);
      poly = __fma(poly, x2, __si1_ep___c0_7.f);
      poly = __fma(poly, x2, __si1_ep___c0_6.f);
      poly = __fma(poly, x2, __si1_ep___c0_5.f);
      poly = __fma(poly, x2, __si1_ep___c0_4.f);
      poly = __fma(poly, x2, __si1_ep___c0_3.f);
      poly = __fma(poly, x2, __si1_ep___c0_2.f);
      poly_l = 0.0f;
      // __MUL_F2_F2(poly, poly_l, x2, x2l);
      // __ADD_F2_F2_G(poly, poly_l, _VSTATIC(__c0_2).f, _VSTATIC(__c0_2l).f);
      {
        float __ph, __phl;
        __ph = __fma(poly, x2, 0.0f);
        __phl = __fma(poly, x2, -__ph);
        poly_l = __fma(poly_l, x2, __phl);
        poly_l = __fma(poly, x2l, poly_l);
        poly = __ph;
      };
      {
        float __ph, __ahl, __ahh;
        float __ah, __bh;
        __bh = (__fabs(poly) <= __fabs(__si1_ep___c0_1.f)) ? (__si1_ep___c0_1.f)
                                                           : (poly);
        __ah = (__fabs(poly) <= __fabs(__si1_ep___c0_1.f))
                   ? (poly)
                   : (__si1_ep___c0_1.f);
        __ph = __fma(__ah, 1.0f, __bh);
        __ahh = __fma(__ph, 1.0f, -__bh);
        __ahl = __fma(__ah, 1.0f, -__ahh);
        poly_l = (poly_l + __si1_ep___c0_1l.f) + __ahl;
        poly = __ph;
      };
      {
        float __ph, __phl;
        __ph = __fma(poly, x2, 0.0f);
        __phl = __fma(poly, x2, -__ph);
        poly_l = __fma(poly_l, x2, __phl);
        poly_l = __fma(poly, x2l, poly_l);
        poly = __ph;
      };
      {
        float __ph, __ahl, __ahh;
        __ph = __fma(poly, 1.0f, __si1_ep___c0_0.f);
        __ahh = __fma(__ph, 1.0f, -__si1_ep___c0_0.f);
        __ahl = __fma(poly, 1.0f, -__ahh);
        poly_l = (poly_l + __si1_ep___c0_0l.f) + __ahl;
        poly = __ph;
      };
      *pres = __fma(xin, poly, __fma(xin, poly_l, 0.0f));
      return nRet;
    }
  } else if (x.f <= 92.0f) {
    bc_h[8] = (x.f <= 39.0f) ? __si1_ep___c1_8.f : __si1_ep___c2_8.f;
    bc_h[7] = (x.f <= 39.0f) ? __si1_ep___c1_7.f : __si1_ep___c2_7.f;
    bc_h[6] = (x.f <= 39.0f) ? __si1_ep___c1_6.f : __si1_ep___c2_6.f;
    bc_h[5] = (x.f <= 39.0f) ? __si1_ep___c1_5.f : __si1_ep___c2_5.f;
    bc_h[4] = (x.f <= 39.0f) ? __si1_ep___c1_4.f : __si1_ep___c2_4.f;
    bc_h[3] = (x.f <= 39.0f) ? __si1_ep___c1_3.f : __si1_ep___c2_3.f;
    bc_h[2] = (x.f <= 39.0f) ? __si1_ep___c1_2.f : __si1_ep___c2_2.f;
    bc_h[1] = (x.f <= 39.0f) ? __si1_ep___c1_1.f : __si1_ep___c2_1.f;
    bc_h[0] = (x.f <= 39.0f) ? __si1_ep___c1_0.f : __si1_ep___c2_0.f;
    bc_l[8] = (x.f <= 39.0f) ? __si1_ep___c1_8l.f : __si1_ep___c2_8l.f;
    bc_l[7] = (x.f <= 39.0f) ? __si1_ep___c1_7l.f : __si1_ep___c2_7l.f;
    bc_l[6] = (x.f <= 39.0f) ? __si1_ep___c1_6l.f : __si1_ep___c2_6l.f;
    bc_l[5] = (x.f <= 39.0f) ? __si1_ep___c1_5l.f : __si1_ep___c2_5l.f;
    bc_l[4] = (x.f <= 39.0f) ? __si1_ep___c1_4l.f : __si1_ep___c2_4l.f;
    bc_l[3] = (x.f <= 39.0f) ? __si1_ep___c1_3l.f : __si1_ep___c2_3l.f;
    bc_l[2] = (x.f <= 39.0f) ? __si1_ep___c1_2l.f : __si1_ep___c2_2l.f;
    bc_l[1] = (x.f <= 39.0f) ? __si1_ep___c1_1l.f : __si1_ep___c2_1l.f;
    bc_l[0] = (x.f <= 39.0f) ? __si1_ep___c1_0l.f : __si1_ep___c2_0l.f;
    // poly ~ 2^64*BesselI(0, x.f)/exp(x.f)
    poly = bc_h[8];
    poly_l = bc_l[8];
    {
      float __ph, __phl;
      __ph = __fma(poly, x.f, 0.0f);
      __phl = __fma(poly, x.f, -__ph);
      poly_l = __fma(poly_l, x.f, __phl);
      poly = __ph;
    };
    {
      float __ph, __ahl, __ahh;
      __ph = __fma(poly, 1.0f, bc_h[7]);
      __ahh = __fma(__ph, 1.0f, -bc_h[7]);
      __ahl = __fma(poly, 1.0f, -__ahh);
      poly_l = (poly_l + bc_l[7]) + __ahl;
      poly = __ph;
    };
    {
      float __ph, __phl;
      __ph = __fma(poly, x.f, 0.0f);
      __phl = __fma(poly, x.f, -__ph);
      poly_l = __fma(poly_l, x.f, __phl);
      poly = __ph;
    };
    {
      float __ph, __ahl, __ahh;
      __ph = __fma(poly, 1.0f, bc_h[6]);
      __ahh = __fma(__ph, 1.0f, -bc_h[6]);
      __ahl = __fma(poly, 1.0f, -__ahh);
      poly_l = (poly_l + bc_l[6]) + __ahl;
      poly = __ph;
    };
    {
      float __ph, __phl;
      __ph = __fma(poly, x.f, 0.0f);
      __phl = __fma(poly, x.f, -__ph);
      poly_l = __fma(poly_l, x.f, __phl);
      poly = __ph;
    };
    {
      float __ph, __ahl, __ahh;
      __ph = __fma(poly, 1.0f, bc_h[5]);
      __ahh = __fma(__ph, 1.0f, -bc_h[5]);
      __ahl = __fma(poly, 1.0f, -__ahh);
      poly_l = (poly_l + bc_l[5]) + __ahl;
      poly = __ph;
    };
    {
      float __ph, __phl;
      __ph = __fma(poly, x.f, 0.0f);
      __phl = __fma(poly, x.f, -__ph);
      poly_l = __fma(poly_l, x.f, __phl);
      poly = __ph;
    };
    {
      float __ph, __ahl, __ahh;
      __ph = __fma(poly, 1.0f, bc_h[4]);
      __ahh = __fma(__ph, 1.0f, -bc_h[4]);
      __ahl = __fma(poly, 1.0f, -__ahh);
      poly_l = (poly_l + bc_l[4]) + __ahl;
      poly = __ph;
    };
    {
      float __ph, __phl;
      __ph = __fma(poly, x.f, 0.0f);
      __phl = __fma(poly, x.f, -__ph);
      poly_l = __fma(poly_l, x.f, __phl);
      poly = __ph;
    };
    {
      float __ph, __ahl, __ahh;
      __ph = __fma(poly, 1.0f, bc_h[3]);
      __ahh = __fma(__ph, 1.0f, -bc_h[3]);
      __ahl = __fma(poly, 1.0f, -__ahh);
      poly_l = (poly_l + bc_l[3]) + __ahl;
      poly = __ph;
    };
    {
      float __ph, __phl;
      __ph = __fma(poly, x.f, 0.0f);
      __phl = __fma(poly, x.f, -__ph);
      poly_l = __fma(poly_l, x.f, __phl);
      poly = __ph;
    };
    {
      float __ph, __ahl, __ahh;
      __ph = __fma(poly, 1.0f, bc_h[2]);
      __ahh = __fma(__ph, 1.0f, -bc_h[2]);
      __ahl = __fma(poly, 1.0f, -__ahh);
      poly_l = (poly_l + bc_l[2]) + __ahl;
      poly = __ph;
    };
    {
      float __ph, __phl;
      __ph = __fma(poly, x.f, 0.0f);
      __phl = __fma(poly, x.f, -__ph);
      poly_l = __fma(poly_l, x.f, __phl);
      poly = __ph;
    };
    {
      float __ph, __ahl, __ahh;
      __ph = __fma(poly, 1.0f, bc_h[1]);
      __ahh = __fma(__ph, 1.0f, -bc_h[1]);
      __ahl = __fma(poly, 1.0f, -__ahh);
      poly_l = (poly_l + bc_l[1]) + __ahl;
      poly = __ph;
    };
    {
      float __ph, __phl;
      __ph = __fma(poly, x.f, 0.0f);
      __phl = __fma(poly, x.f, -__ph);
      poly_l = __fma(poly_l, x.f, __phl);
      poly = __ph;
    };
    {
      float __ph, __ahl, __ahh;
      __ph = __fma(poly, 1.0f, bc_h[0]);
      __ahh = __fma(__ph, 1.0f, -bc_h[0]);
      __ahl = __fma(poly, 1.0f, -__ahh);
      poly_l = (poly_l + bc_l[0]) + __ahl;
      poly = __ph;
    };
    // H+L ~exp(x.f)*2^(-64)
    // x2h*L2E + Shifter
    S.f = __fma(x.f, __si1_ep___L2E.f, __si1_ep___Shifter.f);
    // (int)(x2h*L2E)
    N.f = S.f - __si1_ep___Shifter.f;
    // x^2 - N*log(2)
    R0 = __fma(-N.f, __si1_ep___L2H.f, x.f);
    R1 = __fma(-N.f, __si1_ep___L2L.f, 0.0f);
    R = R0 + R1;
    R1h = R - R0;
    Rl = R1 - R1h;
    // 2^(N)
    Te.w = S.w << 23;
    // exp(R)-1
    epoly.f = __fma(__si1_ep___ce7.f, R, __si1_ep___ce6.f);
    epoly.f = __fma(epoly.f, R, __si1_ep___ce5.f);
    epoly.f = __fma(epoly.f, R, __si1_ep___ce4.f);
    epoly.f = __fma(epoly.f, R, __si1_ep___ce3.f);
    epoly2.f = __fma(epoly.f, R, __si1_ep___ce2.f);
    epoly.f = __fma(epoly2.f, R, __si1_ep___ce1.f);
    epoly_l.f = __fma(epoly.f, Rl, __si1_ep___ce0.f);
    // 2^(N)*exp(R)
    RTe.f = R * Te.f;
    H.f = __fma(epoly.f, RTe.f, Te.f);
    LH.f = H.f - Te.f;
    L.f = __fma(epoly.f, RTe.f, -LH.f);
    L.f = __fma(epoly_l.f, Te.f, L.f);
    // (H+L)*(poly+poly_l)
    L.f = __fma(L.f, poly, __fma(H.f, poly_l, 0.0f));
    H.f = __fma(H.f, poly, L.f);
    // check for overflow
    nRet = (H.w == 0x7f800000) ? 3 : nRet;
    H.w ^= sgn_x.w;
    *pres = H.f;
    return nRet;
  }
  H.w = 0x7f800000;
  H.f = H.f + x.f;
  // large inputs overflow
  nRet = (x.w < 0x7f800000) ? 3 : nRet;
  H.w ^= sgn_x.w;
  *pres = H.f;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_i1_s_ep */
DEVICE_EXTERN_C_INLINE float __devicelib_imf_i1f(float x) {
  using namespace __imf_impl_i1_s_ep;
  float r;
  __devicelib_imf_internal_si1(&x, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
