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
namespace __imf_impl_cbrt_s_la {
namespace {
/* file: _vscbrt_cout_ats.i */
static const union {
  uint32_t w;
  float f;
} __scbrt_la___c9 = {0x3bcfaf07u};
static const union {
  uint32_t w;
  float f;
} __scbrt_la___c8 = {0xbc771231u};
static const union {
  uint32_t w;
  float f;
} __scbrt_la___c7 = {0x3ca35241u};
static const union {
  uint32_t w;
  float f;
} __scbrt_la___c6 = {0xbcc266f9u};
static const union {
  uint32_t w;
  float f;
} __scbrt_la___c5 = {0x3cf6f381u};
static const union {
  uint32_t w;
  float f;
} __scbrt_la___c4 = {0xbd2880e9u};
static const union {
  uint32_t w;
  float f;
} __scbrt_la___c3 = {0x3d7cd740u};
static const union {
  uint32_t w;
  float f;
} __scbrt_la___c2 = {0xbde38e56u};
static const union {
  uint32_t w;
  float f;
} __scbrt_la___c1 = {0x3eaaaaabu};
static const union {
  uint32_t w;
  float f;
} __scbrt_la___c0 = {0x30144e9bu};
static const union {
  uint32_t w;
  float f;
} __scbrt_la___maxnum = {0x7f7fffffu};
inline int __devicelib_imf_internal_scbrt(const float *a, float *pres) {
  int nRet = 0;
  float xin = *a;
  int_float xa, res;
  uint32_t sgn_x, ecorr, expon, k, j, P;
  float dR;
  int_float poly, pl, scale, two_j, two_jl, mant, r_expon;
  xa.f = xin;
  sgn_x = xa.w & 0x80000000;
  // |xin|
  xa.w ^= sgn_x;
  // will scale denormals by 2^69
  scale.w = (xa.w < 0x00800000) ? 0x62000000u : 0x3f800000u;
  // final exponent correction
  ecorr = (xa.w < 0x00800000) ? 85 - 23 : 85;
  xa.f = __fma(xa.f, scale.f, 0.0f);
  // input exponent (expon = 3*k+j); will subtract 1 from bias (so it is
  // divisible by 3)
  expon = (xa.w + 0xffc00000) >> 23;
  // (2^32+2)/3  * exponent
  P = (uint64_t)expon * (uint64_t)0x5556UL;
  k = P >> 16;
  j = expon - k - k - k;
  r_expon.w = (k + ecorr) << 23;
  // correction for xin==0.0
  r_expon.w = (!xa.w) ? 0 : r_expon.w;
  // mantissa
  mant.w = (xa.w - (expon << 23)) + 0x3f000000u;
  // reduced argument, range [-0.25,0.5]
  dR = mant.f - 1.0f;
  // polynomial evaluation
  poly.f = __fma(__scbrt_la___c9.f, dR, __scbrt_la___c8.f);
  poly.f = __fma(poly.f, dR, __scbrt_la___c7.f);
  poly.f = __fma(poly.f, dR, __scbrt_la___c6.f);
  poly.f = __fma(poly.f, dR, __scbrt_la___c5.f);
  poly.f = __fma(poly.f, dR, __scbrt_la___c4.f);
  poly.f = __fma(poly.f, dR, __scbrt_la___c3.f);
  poly.f = __fma(poly.f, dR, __scbrt_la___c2.f);
  poly.f = __fma(poly.f, dR, __scbrt_la___c1.f);
  poly.f = __fma(poly.f, dR, __scbrt_la___c0.f);
  // 2^j
  two_j.w = (!j) ? 0x3f800000u : 0x3FA14518u;
  two_j.w = (j <= 1) ? two_j.w : 0x3FCB2FF5u;
  // (2^j)_low/(2^j)_high
  two_jl.w = (!j) ? 0x0UL : 0xB223C16Cu;
  two_jl.w = (j <= 1) ? two_jl.w : 0x31D34318u;
  // attach exponent
  two_j.f = __fma(two_j.f, r_expon.f, 0.0f);
  pl.f = __fma(two_jl.f, poly.f, two_jl.f);
  poly.f = poly.f + pl.f;
  res.f = __fma(two_j.f, poly.f, two_j.f);
  // fixup for Inf/NaN
  res.f = (xa.f <= __scbrt_la___maxnum.f) ? res.f : (xa.f + xa.f);
  // set sign
  res.w ^= sgn_x;
  *pres = res.f;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_cbrt_s_la */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_cbrtf(float a) {
  using namespace __imf_impl_cbrt_s_la;
  float r;
  __devicelib_imf_internal_scbrt(&a, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
