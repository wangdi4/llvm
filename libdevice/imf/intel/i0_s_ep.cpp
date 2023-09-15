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
namespace __imf_impl_i0_s_ep {
namespace {
static const union {
  uint32_t w;
  float f;
} __si0_ep___c0_13 = {0x12f6856bu};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c0_13l = {0x85f5919cu};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c0_12 = {0x97064d97u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c0_12l = {0x89884881u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c0_11 = {0x1bdb4336u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c0_11l = {0x8f55a885u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c0_10 = {0x1f18eb5fu};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c0_10l = {0x12e31d41u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c0_9 = {0x241c3949u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c0_9l = {0x17f51667u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c0_8 = {0x2821d7ccu};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c0_8l = {0x9bc81e54u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c0_7 = {0x2c2aa224u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c0_7l = {0x9d068142u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c0_6 = {0x30013c27u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c0_6l = {0xa380da93u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c0_5 = {0x3391acc5u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c0_5l = {0x26c35429u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c0_4 = {0x36e38c0cu};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c0_4l = {0xa951c77cu};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c0_3 = {0x39e38e7bu};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c0_3l = {0xac2e95b4u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c0_2 = {0x3c7ffff9u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c0_2l = {0xafb4396fu};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c0_1 = {0x3e800000u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c0_1l = {0x32115942u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c0_0 = {0x3f800000u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c0_0l = {0xb0cbe2c0u};
// coefficients for i0(x)/exp(x)*2^64,  |x| in [14, 36]
static const union {
  uint32_t w;
  float f;
} __si0_ep___c1_9 = {0xc8051c6cu};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c1_9l = {0x3b145e48u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c1_8 = {0x4c01e458u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c1_8l = {0x3fc6547du};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c1_7 = {0xcf61edf1u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c1_7l = {0xc0cced98u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c1_6 = {0x5266a669u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c1_6l = {0x45d1b854u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c1_5 = {0xd51924efu};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c1_5l = {0x47becb0cu};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c1_4 = {0x578a6772u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c1_4l = {0x4b3810b2u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c1_3 = {0xd9ad040au};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c1_3l = {0x4d7f604eu};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c1_2 = {0x5b950f5fu};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c1_2l = {0x4efdbbc0u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c1_1 = {0xdd2ef06cu};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c1_1l = {0xd0e5514au};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c1_0 = {0x5e9ef810u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c1_0l = {0xd24362b4u};
// coefficients for i0(x)/exp(x)*2^64,  |x| in [36, 92]
static const union {
  uint32_t w;
  float f;
} __si0_ep___c2_9 = {0xc1846d5cu};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c2_9l = {0xb4504ab6u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c2_8 = {0x462594afu};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c2_8l = {0x3956f69eu};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c2_7 = {0xca389511u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c2_7l = {0xbd445018u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c2_6 = {0x4df1a676u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c2_6l = {0xc0dd2f0eu};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c2_5 = {0xd14de114u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c2_5l = {0x44210e4cu};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c2_4 = {0x546ef339u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c2_4l = {0xc7a749eau};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c2_3 = {0xd7400dd7u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c2_3l = {0xcac2865au};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c2_2 = {0x59d53e2fu};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c2_2l = {0x4d744cc9u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c2_1 = {0xdc220117u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c2_1l = {0x4f9e3dcdu};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c2_0 = {0x5e4111e8u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___c2_0l = {0x510c730du};
// exp(R) coefficients
static const union {
  uint32_t w;
  float f;
} __si0_ep___ce6 = {0x3ab60b53u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___ce5 = {0x3c091e6bu};
static const union {
  uint32_t w;
  float f;
} __si0_ep___ce4 = {0x3d2aab29u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___ce3 = {0x3e2aaa3fu};
static const union {
  uint32_t w;
  float f;
} __si0_ep___ce2 = {0x3effffffu};
static const union {
  uint32_t w;
  float f;
} __si0_ep___ce1 = {0x3f800000u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___L2E = {0x3FB8AA3Bu};
static const union {
  uint32_t w;
  float f;
} __si0_ep___Shifter = {0x4b40003fu};
static const union {
  uint32_t w;
  float f;
} __si0_ep___L2H = {0x3f317218u};
static const union {
  uint32_t w;
  float f;
} __si0_ep___L2L = {0xb102e308u};
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
inline int __devicelib_imf_internal_si0(const float *pa, float *pres) {
  int nRet = 0;
  float xin = *pa;
  union {
    uint32_t w;
    float f;
  } x, S, N, epoly, epoly_l, Te, RTe, H, L, LH, epoly2;
  float x2, poly, x2l, poly_l, R, R0, R1, R1h, Rl;
  float bc_h[14], bc_l[14];
  x.f = xin;
  // |xin|
  x.w = x.w & 0x7fffffffu;
  if (x.f <= 14.0f) {
    x2 = __fma(xin, xin, 0.0f);
    if (x.f <= 10.0f) {
      poly = __fma(x2, __si0_ep___c0_13.f, __si0_ep___c0_12.f);
      poly = __fma(poly, x2, __si0_ep___c0_11.f);
      poly = __fma(poly, x2, __si0_ep___c0_10.f);
      poly = __fma(poly, x2, __si0_ep___c0_9.f);
      poly = __fma(poly, x2, __si0_ep___c0_8.f);
      poly = __fma(poly, x2, __si0_ep___c0_7.f);
      poly = __fma(poly, x2, __si0_ep___c0_6.f);
      poly = __fma(poly, x2, __si0_ep___c0_5.f);
      poly = __fma(poly, x2, __si0_ep___c0_4.f);
      poly = __fma(poly, x2, __si0_ep___c0_3.f);
      poly = __fma(poly, x2, __si0_ep___c0_2.f);
      poly = __fma(poly, x2, __si0_ep___c0_1.f);
      poly = __fma(poly, x2, __si0_ep___c0_0.f);
      *pres = poly;
      return nRet;
    } else // 5.5 .. 14.0
    {
      x2l = __fma(xin, xin, -x2);
      poly = __fma(x2, __si0_ep___c0_13.f, __si0_ep___c0_12.f);
      poly = __fma(poly, x2, __si0_ep___c0_11.f);
      poly = __fma(poly, x2, __si0_ep___c0_10.f);
      poly = __fma(poly, x2, __si0_ep___c0_9.f);
      poly = __fma(poly, x2, __si0_ep___c0_8.f);
      poly = __fma(poly, x2, __si0_ep___c0_7.f);
      poly = __fma(poly, x2, __si0_ep___c0_6.f);
      poly = __fma(poly, x2, __si0_ep___c0_5.f);
      poly_l = 0.0f;
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
        __bh = (__fabs(poly) <= __fabs(__si0_ep___c0_4.f)) ? (__si0_ep___c0_4.f)
                                                           : (poly);
        __ah = (__fabs(poly) <= __fabs(__si0_ep___c0_4.f))
                   ? (poly)
                   : (__si0_ep___c0_4.f);
        __ph = __fma(__ah, 1.0f, __bh);
        __ahh = __fma(__ph, 1.0f, -__bh);
        __ahl = __fma(__ah, 1.0f, -__ahh);
        poly_l = (poly_l + __si0_ep___c0_4l.f) + __ahl;
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
        float __ah, __bh;
        __bh = (__fabs(poly) <= __fabs(__si0_ep___c0_3.f)) ? (__si0_ep___c0_3.f)
                                                           : (poly);
        __ah = (__fabs(poly) <= __fabs(__si0_ep___c0_3.f))
                   ? (poly)
                   : (__si0_ep___c0_3.f);
        __ph = __fma(__ah, 1.0f, __bh);
        __ahh = __fma(__ph, 1.0f, -__bh);
        __ahl = __fma(__ah, 1.0f, -__ahh);
        poly_l = (poly_l + __si0_ep___c0_3l.f) + __ahl;
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
        float __ah, __bh;
        __bh = (__fabs(poly) <= __fabs(__si0_ep___c0_2.f)) ? (__si0_ep___c0_2.f)
                                                           : (poly);
        __ah = (__fabs(poly) <= __fabs(__si0_ep___c0_2.f))
                   ? (poly)
                   : (__si0_ep___c0_2.f);
        __ph = __fma(__ah, 1.0f, __bh);
        __ahh = __fma(__ph, 1.0f, -__bh);
        __ahl = __fma(__ah, 1.0f, -__ahh);
        poly_l = (poly_l + __si0_ep___c0_2l.f) + __ahl;
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
        float __ah, __bh;
        __bh = (__fabs(poly) <= __fabs(__si0_ep___c0_1.f)) ? (__si0_ep___c0_1.f)
                                                           : (poly);
        __ah = (__fabs(poly) <= __fabs(__si0_ep___c0_1.f))
                   ? (poly)
                   : (__si0_ep___c0_1.f);
        __ph = __fma(__ah, 1.0f, __bh);
        __ahh = __fma(__ph, 1.0f, -__bh);
        __ahl = __fma(__ah, 1.0f, -__ahh);
        poly_l = (poly_l + __si0_ep___c0_1l.f) + __ahl;
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
        __ph = __fma(poly, 1.0f, __si0_ep___c0_0.f);
        __ahh = __fma(__ph, 1.0f, -__si0_ep___c0_0.f);
        __ahl = __fma(poly, 1.0f, -__ahh);
        poly_l = (poly_l + __si0_ep___c0_0l.f) + __ahl;
        poly = __ph;
      };
      *pres = poly + poly_l;
      return nRet;
    }
  } else if (x.f <= 92.0f) {
    bc_h[9] = (x.f <= 36.0f) ? __si0_ep___c1_9.f : __si0_ep___c2_9.f;
    bc_h[8] = (x.f <= 36.0f) ? __si0_ep___c1_8.f : __si0_ep___c2_8.f;
    bc_h[7] = (x.f <= 36.0f) ? __si0_ep___c1_7.f : __si0_ep___c2_7.f;
    bc_h[6] = (x.f <= 36.0f) ? __si0_ep___c1_6.f : __si0_ep___c2_6.f;
    bc_h[5] = (x.f <= 36.0f) ? __si0_ep___c1_5.f : __si0_ep___c2_5.f;
    bc_h[4] = (x.f <= 36.0f) ? __si0_ep___c1_4.f : __si0_ep___c2_4.f;
    bc_h[3] = (x.f <= 36.0f) ? __si0_ep___c1_3.f : __si0_ep___c2_3.f;
    bc_h[2] = (x.f <= 36.0f) ? __si0_ep___c1_2.f : __si0_ep___c2_2.f;
    bc_h[1] = (x.f <= 36.0f) ? __si0_ep___c1_1.f : __si0_ep___c2_1.f;
    bc_h[0] = (x.f <= 36.0f) ? __si0_ep___c1_0.f : __si0_ep___c2_0.f;
    bc_l[9] = (x.f <= 36.0f) ? __si0_ep___c1_9l.f : __si0_ep___c2_9l.f;
    bc_l[8] = (x.f <= 36.0f) ? __si0_ep___c1_8l.f : __si0_ep___c2_8l.f;
    bc_l[7] = (x.f <= 36.0f) ? __si0_ep___c1_7l.f : __si0_ep___c2_7l.f;
    bc_l[6] = (x.f <= 36.0f) ? __si0_ep___c1_6l.f : __si0_ep___c2_6l.f;
    bc_l[5] = (x.f <= 36.0f) ? __si0_ep___c1_5l.f : __si0_ep___c2_5l.f;
    bc_l[4] = (x.f <= 36.0f) ? __si0_ep___c1_4l.f : __si0_ep___c2_4l.f;
    bc_l[3] = (x.f <= 36.0f) ? __si0_ep___c1_3l.f : __si0_ep___c2_3l.f;
    bc_l[2] = (x.f <= 36.0f) ? __si0_ep___c1_2l.f : __si0_ep___c2_2l.f;
    bc_l[1] = (x.f <= 36.0f) ? __si0_ep___c1_1l.f : __si0_ep___c2_1l.f;
    bc_l[0] = (x.f <= 36.0f) ? __si0_ep___c1_0l.f : __si0_ep___c2_0l.f;
    // poly ~ BesselI(0, x.f)/exp(x.f)
    poly = bc_h[9];
    poly_l = bc_l[9];
    {
      float __ph, __phl;
      __ph = __fma(poly, x.f, 0.0f);
      __phl = __fma(poly, x.f, -__ph);
      poly_l = __fma(poly_l, x.f, __phl);
      poly = __ph;
    };
    {
      float __ph, __ahl, __ahh;
      __ph = __fma(poly, 1.0f, bc_h[8]);
      __ahh = __fma(__ph, 1.0f, -bc_h[8]);
      __ahl = __fma(poly, 1.0f, -__ahh);
      poly_l = (poly_l + bc_l[8]) + __ahl;
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
    // H+L ~exp(x.f)
    // x2h*L2E + Shifter
    S.f = __fma(x.f, __si0_ep___L2E.f, __si0_ep___Shifter.f);
    // (int)(x2h*L2E)
    N.f = S.f - __si0_ep___Shifter.f;
    // x^2 - N*log(2)
    R0 = __fma(-N.f, __si0_ep___L2H.f, x.f);
    R = __fma(-N.f, __si0_ep___L2L.f, R0);
    // 2^(N)
    Te.w = S.w << 23;
    // exp(R)-1
    epoly.f = __fma(__si0_ep___ce6.f, R, __si0_ep___ce5.f);
    epoly.f = __fma(epoly.f, R, __si0_ep___ce4.f);
    epoly.f = __fma(epoly.f, R, __si0_ep___ce3.f);
    epoly.f = __fma(epoly.f, R, __si0_ep___ce2.f);
    epoly.f = __fma(epoly.f, R, __si0_ep___ce1.f);
    // 2^(N)*exp(R)
    RTe.f = R * Te.f;
    H.f = __fma(epoly.f, RTe.f, Te.f);
    H.f = __fma(H.f, poly, __fma(H.f, poly_l, 0.0f));
    // check for overflow
    nRet = (H.f == 0x7f800000) ? 3 : nRet;
    *pres = H.f;
    return nRet;
  }
  H.w = 0x7f800000;
  *pres = H.f + x.f;
  // large inputs overflow
  nRet = (x.w < 0x7f800000) ? 3 : nRet;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_i0_s_ep */
DEVICE_EXTERN_C_INLINE float __devicelib_imf_i0f(float x) {
  using namespace __imf_impl_i0_s_ep;
  float r;
  __devicelib_imf_internal_si0(&x, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
