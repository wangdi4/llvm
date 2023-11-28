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
namespace __imf_impl_asinh_s_la {
namespace {
static const union {
  uint32_t w;
  float f;
} __sasinh_la_large_x = {0x49800000u};
// 2^(-12)
static const union {
  uint32_t w;
  float f;
} __sasinh_la_small_x = {0x39800000u};
// largest norm
static const union {
  uint32_t w;
  float f;
} __sasinh_la_largest_norm = {0x7f7fffffu};
// log(2)
static const union {
  uint32_t w;
  float f;
} __sasinh_la_ln2 = {0x3f317218u};
static const union {
  uint32_t w;
  float f;
} __sasinh_la_c8 = {0x3d63bde3u};
static const union {
  uint32_t w;
  float f;
} __sasinh_la_c7 = {0xbdfa61f2u};
static const union {
  uint32_t w;
  float f;
} __sasinh_la_c6 = {0x3e19c853u};
static const union {
  uint32_t w;
  float f;
} __sasinh_la_c5 = {0xbe2c204au};
static const union {
  uint32_t w;
  float f;
} __sasinh_la_c4 = {0x3e4c843bu};
static const union {
  uint32_t w;
  float f;
} __sasinh_la_c3 = {0xbe7fef6cu};
static const union {
  uint32_t w;
  float f;
} __sasinh_la_c2 = {0x3eaaab24u};
static const union {
  uint32_t w;
  float f;
} __sasinh_la_c1 = {0xbf00000au};
inline int __devicelib_imf_internal_sasinh(const float *a, float *r) {
  int nRet = 0;
  float x = *a;
  float x2h, z2h, x2l, z2l, A, B, Bh, Sh, S0h, Sl, RS, E, Yhh;
  float Bl, poly, R;
  union {
    uint32_t w;
    float f;
  } Yh, Yl, res, xin, sgn, xa, two_expon;
  int expon, e23, iexpon_corr;
  x2h = __fma(x, x, 0.0f);
  z2h = x2h + 1.0f;
  A = __fmax(x2h, 1.0f);
  B = __fmin(x2h, 1.0f);
  x2l = __fma(x, x, -(x2h));
  Bh = z2h - A;
  Bl = B - Bh;
  z2l = x2l + Bl;
  RS = 1.0f / __sqrt(z2h);
  // want Sh to overestimate sqrt(1+x^2)
  // RS = SP_FMA(RS, __rsqrt_corr.f, RS);
  S0h = __fma(z2h, RS, 0.0f);
  // rsqrt(z2h)*0.5
  RS *= 0.5f;
  // (1+x^2) - Sh^2
  E = __fma(-(S0h), S0h, z2h);
  E = E + z2l;
  // sqrt(1+x^2)_low
  Sl = __fma(E, RS, 0.0f);
  Sh = S0h + Sl;
  Yhh = Sh - S0h;
  Sl = Sl - Yhh;
  xa.f = __fabs(x);
  // |x| + Sh + Sl
  Yh.f = xa.f + Sh;
  Yhh = Yh.f - Sh;
  Yl.f = xa.f - Yhh;
  Yl.f = Yl.f + Sl;
  // set Yh, Yl for large |x|
  // will use exponent correction in log computation, for large x
  Yh.f = (xa.f < __sasinh_la_large_x.f) ? Yh.f : xa.f * 0.5f;
  Yl.f = (xa.f < __sasinh_la_large_x.f) ? Yl.f : 0;
  // fixup needed for x near largest normal
  iexpon_corr = (xa.f < __sasinh_la_large_x.f) ? 0 : 2;
  // expon(Yh) + 2
  expon = ((Yh.w + 0x00400000) >> 23) - 0x7f;
  // new expon
  e23 = expon << 23;
  // 2^(-expon)
  two_expon.w = 0x3f800000 - e23;
  // Yl * 2^(-expon)
  Yl.f *= two_expon.f;
  // Yh * 2^(-expon-2)
  Yh.w -= e23;
  // reduced argument
  R = Yh.f - 1.0f;
  R = Yl.f + R;
  // add exponent correction
  expon += iexpon_corr;
  // polynomial
  poly = __fma(__sasinh_la_c8.f, R, __sasinh_la_c7.f);
  poly = __fma(poly, R, __sasinh_la_c6.f);
  poly = __fma(poly, R, __sasinh_la_c5.f);
  poly = __fma(poly, R, __sasinh_la_c4.f);
  poly = __fma(poly, R, __sasinh_la_c3.f);
  poly = __fma(poly, R, __sasinh_la_c2.f);
  poly = __fma(poly, R, __sasinh_la_c1.f);
  xin.f = x;
  sgn.w = xin.w ^ xa.w;
  poly *= R;
  poly = __fma(poly, R, R);
  res.f = __fma(((float)expon), __sasinh_la_ln2.f, poly);
  res.w ^= sgn.w;
  // fixup for small or Inf/NaN
  res.f = ((xa.f < __sasinh_la_small_x.f) | (xa.w > __sasinh_la_largest_norm.w))
              ? (x + sgn.f)
              : res.f;
  *r = res.f;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_asinh_s_la */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_asinhf(float a) {
  using namespace __imf_impl_asinh_s_la;
  float r;
  __devicelib_imf_internal_sasinh(&a, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
