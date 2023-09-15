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
namespace __imf_impl_expm1_s_ha {
namespace {
/* file: _vsexpm1_cout_ats.i */
static const union {
  uint32_t w;
  float f;
} __sexpm1_ha_c7 = {0x37d2d82du};
static const union {
  uint32_t w;
  float f;
} __sexpm1_ha_c6 = {0x3952d831u};
static const union {
  uint32_t w;
  float f;
} __sexpm1_ha_c5 = {0x3ab606ceu};
static const union {
  uint32_t w;
  float f;
} __sexpm1_ha_c4 = {0x3c08852cu};
static const union {
  uint32_t w;
  float f;
} __sexpm1_ha_c3 = {0x3d2aaab0u};
static const union {
  uint32_t w;
  float f;
} __sexpm1_ha_c2 = {0x3e2aaab0u};
static const union {
  uint32_t w;
  float f;
} __sexpm1_ha_c1 = {0xb07ec280u};
static const union {
  uint32_t w;
  float f;
} __sexpm1_ha_c0 = {0xb09f34cfu};
static const union {
  uint32_t w;
  float f;
} __sexpm1_ha_fL2E = {0x3FB8AA3Bu};
static const union {
  uint32_t w;
  float f;
} __sexpm1_ha_fShifter = {0x4b40007fu};
// log(2) high, low
static const union {
  uint32_t w;
  float f;
} __sexpm1_ha_NL2H = {0xbf317218u};
static const union {
  uint32_t w;
  float f;
} __sexpm1_ha_NL2L = {0x3102E308u};
inline int __devicelib_imf_internal_sexpm1(const float *a, float *pres) {
  int nRet = 0;
  float xin = *a;
  // float expm1f(float xin)
  union {
    uint32_t w;
    float f;
  } xf, fN, xL2E, fS, xa;
  union {
    uint32_t w;
    float f;
  } T, sc, res;
  float R, Rh, Rl, A, B, Bh, Th, Tl;
  float H, poly, Rhh, Rhl, ThRh, ThRh_l;
  float H1, H2, Rhh2, Rhl2, poly0;
  xf.f = xin;
  xL2E.f = __fma(xf.f, __sexpm1_ha_fL2E.f, 0.0f);
  fN.f = __trunc(xL2E.f);
  fS.f = __sexpm1_ha_fShifter.f + fN.f;
  // reduced argument
  Rh = __fma(fN.f, __sexpm1_ha_NL2H.f, xf.f);
  Rl = __fma(fN.f, __sexpm1_ha_NL2L.f, 0.0f);
  R = Rh + Rl;
  // 2^N
  T.w = fS.w << 23;
  // e^R - 1
  poly = __fma(__sexpm1_ha_c7.f, R, __sexpm1_ha_c6.f);
  poly = __fma(poly, R, __sexpm1_ha_c5.f);
  poly = __fma(poly, R, __sexpm1_ha_c4.f);
  poly = __fma(poly, R, __sexpm1_ha_c3.f);
  poly0 = __fma(poly, R, __sexpm1_ha_c2.f);
  poly = __fma(poly0, R, __sexpm1_ha_c1.f);
  poly = __fma(poly, R, __sexpm1_ha_c0.f);
  poly = __fma(poly, R, Rl);
  // maxabs(T,-1), minabs(T,-1)
  A = (xin >= 0.0f) ? T.f : -1.0f;
  B = (xin >= 0.0f) ? -1.0f : T.f;
  Th = T.f - 1.0f;
  Bh = Th - A;
  Tl = B - Bh;
  // T*Rh
  ThRh = __fma(T.f, Rh, 0.0f);
  ThRh_l = __fma(T.f, Rh, -ThRh);
  // Th + Th*Rh
  H = ThRh + Th;
  // 2*H
  H1 = H + H;
  // (Th*Rh)_high
  Rhh = H - Th;
  // (Th*Rh)_low
  Rhl = ThRh - Rhh;
  Tl = Tl + Rhl + ThRh_l;
  // H+H+Rh*Rh
  H2 = __fma(ThRh, Rh, H1);
  // Rh^2_high
  Rhh2 = H2 - H1;
  // Rh^2_low
  Rhl2 = __fma(ThRh, Rh, -Rhh2);
  // Tl += 0.5*Rhl2 + Rh*Rl
  Tl = __fma(ThRh, Rl, Tl);
  Tl = __fma(Rhl2, 0.5f, Tl);
  // 2^N*poly + Tl
  res.f = __fma(T.f, poly, Tl);
  res.f = __fma(H2, 0.5f, res.f);
  // ensure expm1(-0)=-0
  xa.f = __fabs(xf.f);
  res.w |= (xf.w ^ xa.w);
  if (__fabs(xf.f) <= 87.0f) {
    *pres = res.f;
    return nRet;
  }
  // special case and overflow path
  if (xf.f < 0) {
    *pres = -1.0f;
    return nRet;
  }
  if (!(xf.f < 128.0f)) {
    // +Inf or NaN?
    xa.w = xf.w & 0x7fffffff;
    if (xa.w > 0x7f800000) {
      *pres = xf.f + res.f;
      return nRet;
    }
    // overflow
    res.w = 0x7f800000 - 1;
    res.f = res.f * res.f; // to set OF flag
                           //
    nRet = 3;
    {
      *pres = res.f;
      return nRet;
    }
  }
  // at or near overflow
  // 2^(N-64), N=(int)dN
  T.w = (fS.w - 64) << 23;
  poly = __fma(poly0, R, 0.5f);
  poly = __fma(poly, R, __sexpm1_ha_c1.f);
  poly = __fma(poly, R, Rl);
  poly += Rh;
  res.f = __fma(T.f, poly, T.f);
  // final scaling
  sc.w = 0x5f800000u;
  res.f *= sc.f;
  // determine if overflow
  if (res.w == 0x7f800000)
    nRet = 3;
  *pres = res.f;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_expm1_s_ha */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_expm1f(float a) {
  using namespace __imf_impl_expm1_s_ha;
  float r;
  __devicelib_imf_internal_sexpm1(&a, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
