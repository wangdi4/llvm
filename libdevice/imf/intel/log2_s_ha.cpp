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
namespace __imf_impl_log2_s_ha {
namespace {
/* file: _vslog2_cout_ats.i */
static const union {
  uint32_t w;
  float f;
} __slog2_ha_two32 = {0x4f800000u};
static const union {
  uint32_t w;
  float f;
} __slog2_ha_c10 = {0xbd8770dcu};
static const union {
  uint32_t w;
  float f;
} __slog2_ha_c9 = {0x3e1c5827u};
static const union {
  uint32_t w;
  float f;
} __slog2_ha_c8 = {0xbe434bdeu};
static const union {
  uint32_t w;
  float f;
} __slog2_ha_c7 = {0x3e55e9b0u};
static const union {
  uint32_t w;
  float f;
} __slog2_ha_c6 = {0xbe75d6cau};
static const union {
  uint32_t w;
  float f;
} __slog2_ha_c5 = {0x3e93a7cbu};
static const union {
  uint32_t w;
  float f;
} __slog2_ha_c4 = {0xbeb8aabcu};
static const union {
  uint32_t w;
  float f;
} __slog2_ha_c3 = {0x3ef6389fu};
static const union {
  uint32_t w;
  float f;
} __slog2_ha_c2h = {0xbf38aa3bu};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __slog2_ha_dc1 = {0x3ff715476395bd86UL};
inline int __devicelib_imf_internal_slog2(const float *a, float *r) {
  float xin = *a;
  union {
    uint32_t w;
    float f;
  } x, mant, res;
  int32_t iexpon;
  uint32_t xa;
  float R, poly;
  double dR, dpoly, expon;
  int nRet = 0;
  x.f = xin;
  // normalize mantissa to [0.75, 1.5)
  // normalized, unbiased exponent
  iexpon = (x.w - 0x3f400000u) & 0xff800000u;
  // normalized mantissa
  mant.w = x.w - iexpon;
  // exponent
  iexpon >>= 23;
  // filter out denormals/zero/negative/Inf/NaN
  if ((uint32_t)(x.w - 0x00800000) >= 0x7f000000u)
    goto LOG2F_SPECIAL;
LOG2F_MAIN:
  // reduced argument
  R = mant.f - 1.0f;
  expon = (double)iexpon;
  // polynomial
  poly = __fma(__slog2_ha_c10.f, R, __slog2_ha_c9.f);
  poly = __fma(poly, R, __slog2_ha_c8.f);
  poly = __fma(poly, R, __slog2_ha_c7.f);
  poly = __fma(poly, R, __slog2_ha_c6.f);
  poly = __fma(poly, R, __slog2_ha_c5.f);
  poly = __fma(poly, R, __slog2_ha_c4.f);
  poly = __fma(poly, R, __slog2_ha_c3.f);
  poly = __fma(poly, R, __slog2_ha_c2h.f);
  dR = (double)R;
  dpoly = (double)poly;
  dpoly = __fma(dpoly, dR, __slog2_ha_dc1.f);
  dpoly = __fma(dpoly, dR, expon);
  poly = (float)dpoly;
  *r = poly;
  return nRet;
LOG2F_SPECIAL:
  xa = x.w & 0x7fffffffu;
  // zero
  if (!xa) {
    nRet = 2;
    res.w = 0xff800000u;
    *r = res.f;
    return nRet;
  }
  // Inf/NaN or negative?
  if (x.w >= 0x7f800000u) {
    // +Inf?
    if (x.w == 0x7f800000u) {
      *r = x.f;
      return nRet;
    }
    // return QNaN
    nRet = 1;
    x.w |= 0x7fc00000u;
    *r = x.f;
    return nRet;
  }
  // positive denormal?
  if (!(xa & 0x7f800000)) {
    // scale x by 2^32
    x.f *= __slog2_ha_two32.f;
    // normalized, unbiased exponent
    iexpon = (x.w - 0x3f400000u) & 0xff800000u;
    // normalized mantissa
    mant.w = x.w - iexpon;
    // exponent
    iexpon >>= 23;
    iexpon -= 32;
  }
  goto LOG2F_MAIN;
}
} /* namespace */
} /* namespace __imf_impl_log2_s_ha */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_log2f(float a) {
  using namespace __imf_impl_log2_s_ha;
  float r;
  __devicelib_imf_internal_slog2(&a, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
#if defined(INTEL_COLLAB) && defined(OMP_LIBDEVICE)
DEVICE_EXTERN_C_DECLSIMD_INLINE
float __svml_device_log2f(float x) { return __devicelib_imf_log2f(x); }
#pragma omp end declare target
#endif
