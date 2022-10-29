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
namespace __imf_impl_exp10_s_la {
namespace {
/* file: _vsexp2_cout_ats.i */
static const union {
  uint32_t w;
  float f;
} __sexp10_la_Shifter = {0x4ac000feu};
// log2(e)
static const union {
  uint32_t w;
  float f;
} __sexp10_la_L2_10 = {0x40549A78u};
// log10(2) high, low
static const union {
  uint32_t w;
  float f;
} __sexp10_la_L2H = {0x3e9A209Bu};
static const union {
  uint32_t w;
  float f;
} __sexp10_la_L2L = {0xb2760860u};
static const union {
  uint32_t w;
  float f;
} __sexp10_la_c5 = {0x3f0a4794u};
static const union {
  uint32_t w;
  float f;
} __sexp10_la_c4 = {0x3f962559u};
static const union {
  uint32_t w;
  float f;
} __sexp10_la_c3 = {0x40023822u};
static const union {
  uint32_t w;
  float f;
} __sexp10_la_c2 = {0x4029a917u};
static const union {
  uint32_t w;
  float f;
} __sexp10_la_c1 = {0x40135d8eu};
inline int __devicelib_imf_internal_sexp10(const float *a, float *r) {
  int nRet = 0;
  float x = *a;
  union {
    uint32_t w;
    float f;
  } S, Th, Tlr, Th2, xin, xa, res;
  float N, R, poly;
  int index_mask;
  S.f = __fma(x, __sexp10_la_L2_10.f, __sexp10_la_Shifter.f);
  N = S.f - __sexp10_la_Shifter.f;
  R = __fma(-(N), __sexp10_la_L2H.f, x);
  R = __fma(-(N), __sexp10_la_L2L.f, R);
  // set exponent in place
  Th.w = S.w << 22;
  // index_mask is based on last bit of S.w
  index_mask = 0 - (S.w & 1);
  // set Th mantissa
  Th.w ^= (index_mask & 0x7504F3u);
  // set Tl/Th value
  Tlr.w = index_mask & 0x329302AEu;
  // polynomial
  poly = __fma(R, __sexp10_la_c5.f, __sexp10_la_c4.f);
  poly = __fma(R, poly, __sexp10_la_c3.f);
  poly = __fma(R, poly, __sexp10_la_c2.f);
  poly = __fma(R, poly, __sexp10_la_c1.f);
  poly = __fma(R, poly, Tlr.f);
  xin.f = x;
  xa.w = xin.w & 0x7fffffffu;
  // redirect special cases
  if (xa.w > 0x4217B818u)
    goto EXPF_SPECIAL;
  res.f = __fma(poly, Th.f, Th.f);
  *r = res.f;
  return nRet;
EXPF_SPECIAL:
  if (xa.w > 0x42349E35u) {
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
    nRet = 3;
    res.w = 0x7f800000;
    *r = res.f;
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
} /* namespace __imf_impl_exp10_s_la */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_exp10f(float a) {
  using namespace __imf_impl_exp10_s_la;
  float r;
  __devicelib_imf_internal_sexp10(&a, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
#if defined(INTEL_COLLAB) && defined(OMP_LIBDEVICE)
DEVICE_EXTERN_C_DECLSIMD_INLINE
float __svml_device_exp10f(float x) { return __devicelib_imf_exp10f(x); }
#pragma omp end declare target
#endif
