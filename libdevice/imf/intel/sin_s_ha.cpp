/*******************************************************************************
* INTEL CONFIDENTIAL
* Copyright 1996-2022 Intel Corporation.
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
#if defined(INTEL_COLLAB) && defined(OMP_LIBDEVICE)
#pragma omp declare target
#endif
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_sin_s_ha {
namespace {
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __ssin_ha_reduction_tbl[] = {
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
} __ssin_ha_invpi_h = {0x3fe45f306e000000UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __ssin_ha_invpi_l = {0xbdfb1bbead603d8bUL};
static const union {
  uint32_t w;
  float f;
} __ssin_ha_invpi_s = {0x3f22F983u};
static const union {
  uint32_t w;
  float f;
} __ssin_ha_two19 = {0x49000000u};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __ssin_ha_Shifter = {0x4348000000000000UL};
static const union {
  uint32_t w;
  float f;
} __ssin_ha_fShifter = {0x4b800000u};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __ssin_ha_s_coeff[] = {{0x3fe243f6a4ef1cf2UL},
                         {0xbfe4abbc31a4beb9UL},
                         {0x3fb4668f1db6b48fUL},
                         {0xbf7325359954a527UL},
                         {0x3f23e17cab1bde11UL}};
// coefficients for long path (merged paths are best avoided on Gen)
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __ssin_ha_c8 = {0x3f23e17cab74ef22UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __ssin_ha_c7 = {0x3d23bc9096725dc0UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __ssin_ha_c6 = {0xbf732535995a7e2aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __ssin_ha_c5 = {0xbd256ff0934b8992UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __ssin_ha_c4 = {0x3fb4668f1db6ed47UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __ssin_ha_c3 = {0x3d0904b14cbd3fd1UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __ssin_ha_c2 = {0xbfe4abbc31a4c002UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __ssin_ha_c1 = {0xbccab0482a173390UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __ssin_ha_c0 = {0x3ff921fb52778e7dUL};
inline int __devicelib_imf_internal_ssin(const float *a, float *pres) {
  int nRet = 0;
  float xin = *a;
  union {
    uint32_t w;
    float f;
  } x0, x, fS, fN;
  uint32_t sgn_x, expon;
  int k;
  double xd, dN, dR, dRh, dRl, dR2;
  uint32_t R_sgn;
  union {
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
  } S, s_poly;
  union {
    uint32_t w;
    float f;
  } sin_res;
  x0.f = xin;
  x.f = __fabs(xin);
  sgn_x = x0.w ^ x.w;
  // convert to DP
  xd = (double)x.f;
  if (x.f < __ssin_ha_two19.f) {
    // main path: expon < 23
    // Shifter + (int)(x*2/pi)
    fS.f =
        __fma(x.f, __ssin_ha_invpi_s.f, __ssin_ha_fShifter.f);
    // (int)(x*1/pi)
    fN.f = fS.f - __ssin_ha_fShifter.f;
    // frac(x*1/pi)
    dR = __fma(xd, (__ssin_ha_invpi_h).f, -((double)fN.f));
    R_sgn = fS.w << 31;
    dR = __fma(xd, (__ssin_ha_invpi_l).f, dR);
    dR2 = dR * dR;
    s_poly.f = __fma(dR2, (__ssin_ha_s_coeff[4]).f,
                                      (__ssin_ha_s_coeff[3]).f);
    s_poly.f = __fma(dR2, s_poly.f, (__ssin_ha_s_coeff[2]).f);
    s_poly.f = __fma(dR2, s_poly.f, (__ssin_ha_s_coeff[1]).f);
    s_poly.f = __fma(dR2, s_poly.f, (__ssin_ha_s_coeff[0]).f);
    s_poly.f = __fma(dR, s_poly.f, dR);
    sin_res.f = (float)s_poly.f;
    sin_res.w ^= (R_sgn ^ sgn_x);
    *pres = sin_res.f;
    return nRet;
  } else {
    // large arguments
    // |x|=Inf?
    if (x.w == 0x7f800000) {
      sin_res.w = 0xffc00000;
      nRet = 1;
      *pres = sin_res.f;
      return nRet;
    }
    // biased exponent
    expon = x.w >> 23;
    // table index
    k = (expon - (23 - 8 + 0x7f)) >> 3;
    k += k;
    dRh = __fma(xd, (__ssin_ha_reduction_tbl[k]).f, 0.0);
    dRl = __fma(xd, (__ssin_ha_reduction_tbl[k]).f, -(dRh));
    // low part of (int)(x*1/pi)
    S.f = (__ssin_ha_Shifter).f + dRh;
    dN = S.f - (__ssin_ha_Shifter).f;
    // reduced argument
    dR = dRh - dN;
    dR = dR + dRl;
    dR = __fma(xd, (__ssin_ha_reduction_tbl[k + 1]).f, dR);
    R_sgn = (S.w32[0]) << 31;
    s_poly.f = __fma(dR, __ssin_ha_c8.f, __ssin_ha_c7.f);
    s_poly.f = __fma(dR, s_poly.f, __ssin_ha_c6.f);
    s_poly.f = __fma(dR, s_poly.f, __ssin_ha_c5.f);
    s_poly.f = __fma(dR, s_poly.f, __ssin_ha_c4.f);
    s_poly.f = __fma(dR, s_poly.f, __ssin_ha_c3.f);
    s_poly.f = __fma(dR, s_poly.f, __ssin_ha_c2.f);
    s_poly.f = __fma(dR, s_poly.f, __ssin_ha_c1.f);
    s_poly.f = __fma(dR, s_poly.f, __ssin_ha_c0.f);
    s_poly.f = __fma(dR, s_poly.f, 0.0);
    sin_res.f = (float)s_poly.f;
    sin_res.w ^= (R_sgn ^ sgn_x);
    *pres = sin_res.f;
    return nRet;
  }
}
} /* namespace */
} /* namespace __imf_impl_sin_s_ha */

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_sinf(float a) {
  using namespace __imf_impl_sin_s_ha;
  float r;
  __devicelib_imf_internal_ssin(&a, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
#if defined(INTEL_COLLAB) && defined(OMP_LIBDEVICE)
DEVICE_EXTERN_C_DECLSIMD_INLINE
float __svml_device_sinf(float x) { return __devicelib_imf_sinf(x); }
#pragma omp end declare target
#endif
