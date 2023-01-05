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
namespace __imf_impl_exp_s_la {
namespace {
/* file: _vsexp_cout_ats.i */
static const union {
  uint32_t w;
  float f;
} __sexp_la_Shifter = {0x4ac000feu};
// log2(e)
static const union {
  uint32_t w;
  float f;
} __sexp_la_L2E = {0x3FB8AA3Bu};
// log(2) high, low
static const union {
  uint32_t w;
  float f;
} __sexp_la_L2H = {0x3f317218u};
static const union {
  uint32_t w;
  float f;
} __sexp_la_L2L = {0xb102E308u};
static const union {
  uint32_t w;
  float f;
} __sexp_la_c5 = {0x3c08ba8bu};
static const union {
  uint32_t w;
  float f;
} __sexp_la_c4 = {0x3d2aec4eu};
static const union {
  uint32_t w;
  float f;
} __sexp_la_c3 = {0x3e2aaa9cu};
static const union {
  uint32_t w;
  float f;
} __sexp_la_c2 = {0x3effffe8u};
static const union {
  uint32_t w;
  float f;
} __sexp_la_c1 = {0x3f800000u};
inline int __devicelib_imf_internal_sexp(const float *a, float *r) {
  int nRet = 0;
  float x = *a;
  union {
    uint32_t w;
    float f;
  } S, Th, Tlr, Th2, xin, xa, res;
  float N, R, poly;
  int index_mask;
  S.f = __fma(x, __sexp_la_L2E.f, __sexp_la_Shifter.f);
  N = S.f - __sexp_la_Shifter.f;
  R = __fma(-(N), __sexp_la_L2H.f, x);
  R = __fma(-(N), __sexp_la_L2L.f, R);
  // set exponent in place
  Th.w = S.w << 22;
  // index_mask is based on last bit of S.w
  index_mask = 0 - (S.w & 1);
  // set Th mantissa
  Th.w ^= (index_mask & 0x7504F3u);
  // set Tl/Th value
  Tlr.w = index_mask & 0x329302AEu;
  // polynomial
  poly = __fma(R, __sexp_la_c5.f, __sexp_la_c4.f);
  poly = __fma(R, poly, __sexp_la_c3.f);
  poly = __fma(R, poly, __sexp_la_c2.f);
  poly = __fma(R, poly, __sexp_la_c1.f);
  poly = __fma(R, poly, Tlr.f);
  xin.f = x;
  xa.w = xin.w & 0x7fffffffu;
  // redirect special cases
  if (xa.w > 0x42AEAC4Fu)
    goto EXPF_SPECIAL;
  res.f = __fma(poly, Th.f, Th.f);
  *r = res.f;
  return nRet;
EXPF_SPECIAL:
  if (xa.w > 0x432EAC4Fu) {
    if (xa.w > 0x7f800000u) { // NaN?
      *r = x + x;
      return nRet;
    }
    if (x < 0) {
      *r = 0.0f;
      nRet = 4;
      return nRet; // underflow to 0
    }
    // overflow
    res.w = 0x7f800000;
    *r = res.f;
    nRet = 3;
    return nRet;
  }
  S.w += 0xfe;
  Th2.w = (S.w >> 2) & 0xff;
  S.w -= (Th2.w << 1);
  Th2.w <<= 23; // second exponent scale
  Th.w = S.w << 22;
  // set Th mantissa
  Th.w ^= (index_mask & 0x7504F3u);
  res.f = __fma(poly, Th.f, Th.f);
  res.f *= Th2.f;
  *r = res.f;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_exp_s_la */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_expf(float a) {
  using namespace __imf_impl_exp_s_la;
  float r;
  __devicelib_imf_internal_sexp(&a, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
#if defined(INTEL_COLLAB) && defined(OMP_LIBDEVICE)
DEVICE_EXTERN_C_DECLSIMD_INLINE
float __svml_device_expf(float x) { return __devicelib_imf_expf(x); }
#pragma omp end declare target
#endif
