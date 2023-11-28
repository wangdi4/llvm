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
namespace __imf_impl_atan2_s_la {
namespace {
/* file: _vsatan2_cout_ats.i */
static const union {
  uint32_t w;
  float f;
} __satan2_la_c9 = {0x3c909a7bu};
static const union {
  uint32_t w;
  float f;
} __satan2_la_c8 = {0xbd093579u};
static const union {
  uint32_t w;
  float f;
} __satan2_la_c7 = {0xbbacabd2u};
static const union {
  uint32_t w;
  float f;
} __satan2_la_c6 = {0x3d8c9f7bu};
static const union {
  uint32_t w;
  float f;
} __satan2_la_c5 = {0xbd85ac4eu};
static const union {
  uint32_t w;
  float f;
} __satan2_la_c4 = {0xbd39a551u};
static const union {
  uint32_t w;
  float f;
} __satan2_la_c3 = {0x3e348efbu};
static const union {
  uint32_t w;
  float f;
} __satan2_la_c2 = {0xbe05f628u};
static const union {
  uint32_t w;
  float f;
} __satan2_la_c1 = {0xbe8259b8u};
static const union {
  uint32_t w;
  float f;
} __satan2_la_c0 = {0xbd94e63du};
// 2.0^32
static const union {
  uint32_t w;
  float f;
} __satan2_la_two32 = {0x4f800000u};
// 0
static const union {
  uint32_t w;
  float f;
} __satan2_la_zero = {0x00000000u};
inline int __devicelib_imf_internal_satan2(const float *pyin, const float *pxin,
                                           float *pres) {
  int nRet = 0;
  float xin = *pxin, yin = *pyin;
  // float atan2f_la(float yin, float xin)
  {
    union {
      uint32_t w;
      float f;
    } y, x, ya, xa, hcorr, fx, fy, hcorr2, sres, Q00;
    unsigned sgn_x, sgn_y, sgn_r, sgn_c;
    int32_t sgnx_mask, smask;
    float frcp_x, R, poly;
    y.f = yin;
    x.f = xin;
    // absolute values
    xa.w = x.w & 0x7fffffff;
    ya.w = y.w & 0x7fffffff;
    // input signs
    sgn_x = x.w ^ xa.w;
    sgn_y = y.w ^ ya.w;
    // initialize result correction (0, pi, or pi/2)
    sgnx_mask = ((int32_t)sgn_x) >> 31;
    hcorr.w = sgnx_mask & 0x40490FDB;
    // now switch y, x if |y|>|x|
    fy.w = (((xa.w) < (ya.w)) ? (xa.w) : (ya.w));
    fx.w = (((xa.w) >= (ya.w)) ? (xa.w) : (ya.w));
    // set correction term to pi/2 if xa<ya
    smask = ((int32_t)(xa.w - ya.w)) >> 31;
    hcorr2.w = smask & 0x3fc90FDB;
    hcorr.f = hcorr2.f - hcorr.f;
    sgn_c = (smask & 0x80000000);
    hcorr.w ^= sgn_c;
    // also apply sign correction
    sgn_r = sgn_c ^ (sgn_x ^ sgn_y);
    // redirect special inputs:  NaN/Inf/zero, |x|>2^126
    // testing done on ordered inputs
    if (((unsigned)(fx.w - 0x00800000) > 0x7e000000) ||
        (fy.f == __satan2_la_zero.f))
      goto SPECIAL_ATAN2F;
  ATAN2F_MAIN:
    // reciprocal: rcp_x ~ 1/x
    frcp_x = 1.0f / (fx.f);
    // quotient estimate
    Q00.f = fy.f * frcp_x;
    // reduced argument:  Q-0.5
    R = __fma(fy.f, frcp_x, -(0.5f));
    poly = __fma(__satan2_la_c9.f, R, __satan2_la_c8.f);
    poly = __fma(poly, R, __satan2_la_c7.f);
    poly = __fma(poly, R, __satan2_la_c6.f);
    poly = __fma(poly, R, __satan2_la_c5.f);
    poly = __fma(poly, R, __satan2_la_c4.f);
    poly = __fma(poly, R, __satan2_la_c3.f);
    poly = __fma(poly, R, __satan2_la_c2.f);
    poly = __fma(poly, R, __satan2_la_c1.f);
    poly = __fma(poly, R, __satan2_la_c0.f);
    //  Q0*poly + hcorr
    sres.f = __fma(poly, Q00.f, Q00.f);
    sres.f += hcorr.f;
    sres.w = sres.w ^ sgn_r;
    *pres = sres.f;
    return nRet;
  SPECIAL_ATAN2F:
    // NaN input
    if (fx.w > 0x7f800000u) {
      if (xa.w > 0x7f800000u)
        sres.w = x.w | 0x00400000u;
      else
        sres.w = y.w | 0x00400000u;
      *pres = sres.f;
      return nRet;
    }
    // zero input?
    if (fy.f == __satan2_la_zero.f) {
      sres.w = sgn_y ^ (hcorr.w & 0x7fffffff);
      *pres = sres.f;
      return nRet;
    }
    // Inf input?
    if (fx.w == 0x7f800000) {
      if (fy.w < 0x7f800000u) {
        if (ya.w == 0x7f800000u)
          sres.w = sgn_y ^ 0x3fc90FDB;
        else
          sres.w = sgn_r ^ sgn_c ^ hcorr.w;
        *pres = sres.f;
        return nRet;
      }
      // both inputs are +/-Inf
      if (x.w == 0xff800000u)
        sres.w = 0x4016CBE4;
      else // +Inf
        sres.w = 0x3f490FDB;
      sres.w ^= sgn_y;
      *pres = sres.f;
      return nRet;
    }
    // very small |x|, |y|?
    if (fx.w < 0x00800000u) {
      // scale inputs
      fx.f *= __satan2_la_two32.f;
      fy.f *= __satan2_la_two32.f;
    } else if (fx.w > 0x7e800000u) {
      // large |x|
      fx.f *= 0.25f;
      fy.f *= 0.25f;
    }
    // return to main path
    goto ATAN2F_MAIN;
  }
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_atan2_s_la */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_atan2f(float a, float b) {
  using namespace __imf_impl_atan2_s_la;
  float r;
  __devicelib_imf_internal_satan2(&a, &b, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
