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
namespace __imf_impl_exp2_s_la {
namespace {
/* file: _vsexp2_cout_ats.i */
static const union {
  uint32_t w;
  float f;
} __sexp2_la_c6 = {0x39224c80u};
static const union {
  uint32_t w;
  float f;
} __sexp2_la_c5 = {0x3aafa463u};
static const union {
  uint32_t w;
  float f;
} __sexp2_la_c4 = {0x3c1d94cbu};
static const union {
  uint32_t w;
  float f;
} __sexp2_la_c3 = {0x3d635766u};
static const union {
  uint32_t w;
  float f;
} __sexp2_la_c2 = {0x3e75fdf1u};
static const union {
  uint32_t w;
  float f;
} __sexp2_la_c1 = {0x3e45c862u};
inline int __devicelib_imf_internal_sexp2(const float *a, float *r) {
  int nRet = 0;
  float x = *a;
  float fN, R, poly, High, Rh, Rl;
  int32_t sN, sN2;
  uint32_t N;
  union {
    uint32_t w;
    float f;
  } T, T2, xi, res;
  fN = __rint(x);
  R = x - fN;
  sN = (int32_t)fN;
  // exponent
  N = sN;
  poly = __fma(__sexp2_la_c6.f, R, __sexp2_la_c5.f);
  // 1+0.5*R
  High = __fma(R, 0.5f, 1.0f);
  poly = __fma(poly, R, __sexp2_la_c4.f);
  // (0.5*R)_high
  Rh = High - 1.0f;
  poly = __fma(poly, R, __sexp2_la_c3.f);
  // (0.5*R)_low
  Rl = __fma(R, 0.5f, -(Rh));
  poly = __fma(poly, R, __sexp2_la_c2.f);
  poly = __fma(poly, R, __sexp2_la_c1.f);
  poly = __fma(poly, R, Rl);
  res.f = High + poly;
  if (((uint32_t)(N + 0x7f - 2)) > 124 + 0x7f)
    goto EXP2F_SPECIAL;
  res.w += (N << 23);
  *r = res.f;
  return nRet;
EXP2F_SPECIAL:
  xi.f = x;
  if ((xi.w & 0x7fffffffu) >= 0x7f800000u) {
    if (xi.w == 0xff800000) {
      *r = 0.0f;
      return nRet;
    } else {
      *r = x + x;
      return nRet; // NaN or +Inf
    }
  }
  x = __fmin(x, 192.0f);
  x = __fmax(x, -192.0f);
  fN = __rint(x);
  sN = (int32_t)fN;
  // split the scaling coefficients
  sN2 = sN >> 1;
  sN -= sN2;
  T.w = (sN + 0x7f) << 23;
  T2.w = (sN2 + 0x7f) << 23;
  res.f *= T.f;
  res.f *= T2.f;
  nRet = (res.w < 0x00800000u) ? 4 : nRet;
  nRet = (res.w == 0x7f800000) ? 3 : nRet;
  *r = res.f;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_exp2_s_la */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_exp2f(float a) {
  using namespace __imf_impl_exp2_s_la;
  float r;
  __devicelib_imf_internal_sexp2(&a, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
#if defined(INTEL_COLLAB) && defined(OMP_LIBDEVICE)
DEVICE_EXTERN_C_DECLSIMD_INLINE
float __svml_device_exp2f(float x) { return __devicelib_imf_exp2f(x); }
#pragma omp end declare target
#endif
