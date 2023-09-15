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
#if defined(INTEL_COLLAB) && defined(OMP_LIBDEVICE)
#pragma omp declare target
#endif
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_cospi_s_la {
namespace {
static const union {
  uint32_t w;
  float f;
} __scospi_la_c4 = {0x3d9f0be5u};
static const union {
  uint32_t w;
  float f;
} __scospi_la_c3 = {0xbf1929adu};
static const union {
  uint32_t w;
  float f;
} __scospi_la_c2 = {0x40233479u};
static const union {
  uint32_t w;
  float f;
} __scospi_la_c1 = {0xc0a55de2u};
static const union {
  uint32_t w;
  float f;
} __scospi_la_c0 = {0x3F921FB5u};
static const union {
  uint32_t w;
  float f;
} __scospi_la_max_norm = {0x7f7fffffu};
inline int __devicelib_imf_internal_scospi(const float *a, float *pres) {
  int nRet = 0;
  float x = *a;
  // float cospif(float x)
  float fN, fNi, R, R2, poly, Rx2;
  int iN, sgn;
  union {
    uint32_t w;
    float f;
  } res;
  fN = __rint(x);
  // using negative sign to get even output when conversion overflows
  fNi = -__fabs(fN);
  iN = (int)fNi;
  // sgn set if fN is odd, 0 otherwise
  sgn = iN << 31;
  R = x - fN;
  R = 0.5f - __fabs(R);
  R2 = R * R;
  poly = __fma(R2, __scospi_la_c4.f, __scospi_la_c3.f);
  poly = __fma(poly, R2, __scospi_la_c2.f);
  poly = __fma(poly, R2, __scospi_la_c1.f);
  poly = __fma(poly, R2, __scospi_la_c0.f);
  Rx2 = __fma(R, 1.0f, R);
  res.f = __fma(R, poly, Rx2);
  // fix sign
  res.w ^= sgn;
  *pres = res.f;
  nRet = (__fabs(x) > __scospi_la_max_norm.f) ? 1 : 0;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_cospi_s_la */
DEVICE_EXTERN_C_INLINE float __devicelib_imf_cospif(float x) {
  using namespace __imf_impl_cospi_s_la;
  float r;
  __devicelib_imf_internal_scospi(&x, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
#if defined(INTEL_COLLAB) && defined(OMP_LIBDEVICE)
DEVICE_EXTERN_C_DECLSIMD_INLINE
float __svml_device_cospif(float x) { return __devicelib_imf_cospif(x); }
#pragma omp end declare target
#endif
