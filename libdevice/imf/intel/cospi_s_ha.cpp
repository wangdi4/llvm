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
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_cospi_s_ha {
namespace {
/* file: _vscospi_cout_ats.i */
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __scospi_ha_dc4 = {0x3fb3e17cab1bde11UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __scospi_ha_dc3 = {0xbfe325359954a527UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __scospi_ha_dc2 = {0x4004668f1db6b48fUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __scospi_ha_dc1 = {0xc014abbc31a4beb9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __scospi_ha_dc0 = {0x400921fb52778e79UL};
static const union {
  uint32_t w;
  float f;
} __scospi_ha_max_norm = {0x7f7fffffu};
inline int __devicelib_imf_internal_scospi(const float *a, float *pres) {
  int nRet = 0;
  float x = *a;
  // float cospif(float x)
  float fN, fNi, R;
  int iN, sgn;
  union {
    uint32_t w;
    float f;
  } res;
  double dR, dR2, dpoly, dres;
  fN = __rint(x);
  // using negative sign to get even output when conversion overflows
  fNi = -__fabs(fN);
  iN = (int)fNi;
  // sgn set if fN is odd, 0 otherwise
  sgn = iN << 31;
  R = x - fN;
  dR = (double)__fabs(R);
  dR = 0.5 - dR;
  dR2 = dR * dR;
  dpoly = __fma(dR2, __scospi_ha_dc4.f, __scospi_ha_dc3.f);
  dpoly = __fma(dpoly, dR2, __scospi_ha_dc2.f);
  dpoly = __fma(dpoly, dR2, __scospi_ha_dc1.f);
  dpoly = __fma(dpoly, dR2, __scospi_ha_dc0.f);
  dres = dR * dpoly;
  res.f = (float)dres;
  // fix sign
  res.w ^= sgn;
  *pres = res.f;
  nRet = (__fabs(x) > __scospi_ha_max_norm.f) ? 1 : 0;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_cospi_s_ha */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_cospif(float a) {
  using namespace __imf_impl_cospi_s_ha;
  float r;
  __devicelib_imf_internal_scospi(&a, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
