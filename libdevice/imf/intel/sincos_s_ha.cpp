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
namespace __imf_impl_sincos_s_ha {
namespace {
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __ssincos_ha_reduction_tbl[] = {
    {0x3fe45f306e000000UL}, {0xbdfb1bbead603d8bUL}, {0x3fe45f306dc9c883UL},
    {0xbc86b01ec5417056UL}, {0x3f87cc1b727220a9UL}, {0x3c23f84eafa3ea6aUL},
    {0x3ef836e4e44152a0UL}, {0xbb8ec54170565912UL}, {0x3e8b727220a94fe1UL},
    {0x3b1d5f47d4d37703UL}, {0x3e027220a94fe13bUL}, {0xbaa05c1596447e49UL},
    {0x3d8220a94fe13abfUL}, {0xba2c1596447e493bUL}, {0x3cb529fc2757d1f5UL},
    {0x394a6ee06db14acdUL}, {0x3c729fc2757d1f53UL}, {0x391377036d8a5665UL},
    {0x3bffc2757d1f534eUL}, {0xb881f924eb53361eUL}, {0x3b43abe8fa9a6ee0UL},
    {0x37eb6c52b3278872UL}, {0x3b0abe8fa9a6ee07UL}, {0xb79275a99b0ef1bfUL},
    {0x3a8e8fa9a6ee06dbUL}, {0x3704acc9e21c8210UL}, {0x39ff534ddc0db629UL},
    {0x369664f10e41080eUL}, {0x39734ddc0db6295aUL}, {0xb61b0ef1bef7fac6UL},
    {0x38ebb81b6c52b328UL}, {0xb58de37defff0eceUL}, {0x387c0db6295993c4UL},
    {0x350c820ff0d954bbUL},
};
// 1.0/(pi/2), for main path
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __ssincos_ha_invpi_h = {0x3fe45f306e000000UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __ssincos_ha_invpi_l = {0xbdfb1bbead603d8bUL};
static const uint32_t __ssincos_ha_AbsMask = 0x7fffffffu;
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __ssincos_ha_Shifter = {0x4338000000000000UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __ssincos_ha_s_coeff[] = {
    {0x3fe243f6a5f0c8e5UL},
    {0xbfe4abbb99e5a6dbUL},
    {0x3fb465ec65afe9deUL},
    {0xbf72d9bb7b168b2bUL},
};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __ssincos_ha_c_coeff[] = {{0x3feFFFFFFFF97C47UL},
                            {0xbff3BD3CC7323529UL},
                            {0x3fd03C1DC1BAFA81UL},
                            {0xbf955C57B06D183CUL},
                            {0x3f4D9C364E89E2CCUL}};
static const union {
  uint32_t w;
  float f;
} __ssincos_ha_invpi_s = {0x3f22F983u};
static const union {
  uint32_t w;
  float f;
} __ssincos_ha_two19 = {0x49000000u};
static const union {
  uint32_t w;
  float f;
} __ssincos_ha_fShifter = {0x4b400000u};
inline int __devicelib_imf_internal_ssincos(const float *a, float *psin,
                                            float *pcos) {
  int nRet = 0;
  float xin = *a;
  uint32_t sgn_x, expon;
  int k, index, c_sgn;
  double xd, dN, dR, dRh, dRl, dR2;
  uint32_t R_sgn;
  union {
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
  } S, s_poly, c_poly;
  union {
    uint32_t w;
    float f;
  } fres0, fres1, sin_res, cos_res, x0, x, fS, fN;
  x0.f = xin;
  x.w = x0.w & __ssincos_ha_AbsMask;
  sgn_x = x0.w ^ x.w;
  // convert to DP
  xd = (double)x.f;
  if (x.f < __ssincos_ha_two19.f) {
    // Shifter + (x*2/pi)
    // Shifter + (int)(x*2/pi)
    fS.f = __fma(x.f, __ssincos_ha_invpi_s.f,
                                  __ssincos_ha_fShifter.f);
    // (int)(x*1/pi)
    fN.f = fS.f - __ssincos_ha_fShifter.f;
    // frac(x*2/pi)
    dR = __fma(xd, __ssincos_ha_invpi_h.f, -((double)fN.f));
    dR = __fma(xd, (__ssincos_ha_invpi_l).f, dR);
    R_sgn = (fS.w >> 1);
    index = fS.w;
  } else {
    // |x|=Inf?
    if (x.w == 0x7f800000) {
      sin_res.w = 0xffc00000;
      nRet = 1;
      *psin = *pcos = sin_res.f;
      return nRet;
    }
    // biased exponent
    expon = x.w >> 23;
    // large arguments
    // table index
    k = (expon - (23 - 8 + 0x7f)) >> 3;
    k += k;
    dRh = __fma(xd, (__ssincos_ha_reduction_tbl[k]).f, 0.0);
    dRl = __fma(xd, (__ssincos_ha_reduction_tbl[k]).f, -(dRh));
    // low part of (int)(x*2/pi)
    S.f = (__ssincos_ha_Shifter).f + dRh;
    dN = S.f - (__ssincos_ha_Shifter).f;
    // reduced argument
    dR = dRh - dN;
    dR = dR + dRl;
    dR = __fma(xd, (__ssincos_ha_reduction_tbl[k + 1]).f, dR);
    R_sgn = (S.w32[0] >> 1);
    index = S.w32[0];
  }
  dR2 = dR * dR;
  R_sgn <<= 31;
  index <<= 31;
  c_sgn = index;
  sgn_x ^= R_sgn;
  c_sgn ^= R_sgn;
  c_poly.f = __fma(dR2, (__ssincos_ha_c_coeff[4]).f,
                                    (__ssincos_ha_c_coeff[3]).f);
  s_poly.f = __fma(dR2, (__ssincos_ha_s_coeff[3]).f,
                                    (__ssincos_ha_s_coeff[2]).f);
  c_poly.f = __fma(dR2, c_poly.f, (__ssincos_ha_c_coeff[2]).f);
  s_poly.f = __fma(dR2, s_poly.f, (__ssincos_ha_s_coeff[1]).f);
  c_poly.f = __fma(dR2, c_poly.f, (__ssincos_ha_c_coeff[1]).f);
  s_poly.f = __fma(dR2, s_poly.f, (__ssincos_ha_s_coeff[0]).f);
  c_poly.f = __fma(dR2, c_poly.f, (__ssincos_ha_c_coeff[0]).f);
  s_poly.f = __fma(dR, s_poly.f, dR);
  fres0.f = (float)s_poly.f;
  fres1.f = (float)c_poly.f;
  sin_res.w = (index == 0) ? fres0.w : fres1.w;
  cos_res.w = (index == 0) ? fres1.w : fres0.w;
  sin_res.w ^= sgn_x;
  cos_res.w ^= c_sgn;
  *pcos = cos_res.f;
  *psin = sin_res.f;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_sincos_s_ha */

DEVICE_EXTERN_C_INLINE void __devicelib_imf_sincosf(float a, float *b,
                                                    float *c) {
  using namespace __imf_impl_sincos_s_ha;
  ;
  __devicelib_imf_internal_ssincos(&a, b, c);
  return;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
