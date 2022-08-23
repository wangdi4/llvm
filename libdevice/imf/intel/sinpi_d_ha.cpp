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
#include "_imf_include_fp64.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_sinpi_d_ha {
namespace {
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dsinpi_ha_two52 = {0x4330000000000000UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dsinpi_ha_two53 = {0x4340000000000000UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dsinpi_ha_dc8 = {0x4ea9d46b06ce620eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dsinpi_ha_dc7 = {0xcef6f7ad23b5cd51UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dsinpi_ha_dc6 = {0x4f3e8f3677c334d3UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dsinpi_ha_dc5 = {0xcf7e3074dfb5bb14UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dsinpi_ha_dc4 = {0x4fb50783485523f4UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dsinpi_ha_dc3 = {0xcfe32d2cce627c9eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dsinpi_ha_dc2 = {0x500466bc6775aa7dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dsinpi_ha_dc1 = {0xd014abbce625be52UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dsinpi_ha_dc0 = {0x500921fb54442d18UL};
// low parts of c0, c1
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dsinpi_ha_dc0l = {0x4c918E38E04F47B9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dsinpi_ha_dc1l = {0xcca15D7C17744D75UL};
// 2^(-256)
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dsinpi_ha_scm_256 = {0x2ff0000000000000UL};
// max. normal value
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dsinpi_ha_max_norm = {0x7f7fffffffffffffUL};
inline int __devicelib_imf_internal_dsinpi(const double *pxin, double *pres) {
  int nRet = 0;
  double xin = *pxin;
  // double sinpi(double xin)
  union {
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
  } x, xa, sgn_x, iN, sgn_N, res;
  double fN, dS, R, R2, R2l, poly;
  double High, Low, H, L, R2c1_h, R2c1_l;
  x.f = xin;
  xa.f = __fabs(xin);
  sgn_x.w = x.w ^ xa.w;
  // 2^52 + |x|
  dS = __dsinpi_ha_two52.f + xa.f;
  // rint(|x|), for |x|<2^52
  fN = dS - __dsinpi_ha_two52.f;
  // correction for |x|>=2^52
  fN = (xa.f < __dsinpi_ha_two52.f) ? fN : xa.f;
  // get last bit of rint(|x|)
  iN.f = (xa.f < __dsinpi_ha_two52.f) ? dS : xa.f;
  iN.f = (xa.f < __dsinpi_ha_two53.f) ? iN.f : 0.0;
  // sgn set if rint(|x|) is odd, 0 otherwise
  sgn_N.w = iN.w << 63;
  R = xa.f - fN;
  R2 = R * R;
  // R2_low
  R2l = __fma(R, R, -(R2));
  poly = __fma(R2, __dsinpi_ha_dc8.f, __dsinpi_ha_dc7.f);
  poly = __fma(poly, R2, __dsinpi_ha_dc6.f);
  poly = __fma(poly, R2, __dsinpi_ha_dc5.f);
  poly = __fma(poly, R2, __dsinpi_ha_dc4.f);
  poly = __fma(poly, R2, __dsinpi_ha_dc3.f);
  poly = __fma(poly, R2, __dsinpi_ha_dc2.f);
  poly = __fma(poly, R2, __dsinpi_ha_dc1l.f);
  // c0+(R2*c1)_h
  H = __fma(__dsinpi_ha_dc1.f, R2, __dsinpi_ha_dc0.f);
  // (R2*c1)_h
  R2c1_h = H - __dsinpi_ha_dc0.f;
  // (R2*c1)_l
  R2c1_l = __fma(R2, __dsinpi_ha_dc1.f, -(R2c1_h));
  // R2c1_l + R2l*poly
  R2c1_l = __fma(R2l, poly, R2c1_l);
  // c0l + R2l*c1
  L = __fma(R2l, __dsinpi_ha_dc1.f, __dsinpi_ha_dc0l.f);
  // c0l + R2l*c1 + R2c1_l + R2l*poly
  L = L + R2c1_l;
  // L = c0l + R2l*c1 + R2c1_l + R2l*poly + R2*poly
  L = __fma(poly, R2, L);
  // (R*H)_high
  High = __fma(R, H, 0.0);
  // (R*H)_low
  Low = __fma(R, H, -(High));
  // (R*H)_low + R*L
  Low = __fma(R, L, Low);
  res.f = High + Low;
  // final scaling
  res.f = __fma(res.f, __dsinpi_ha_scm_256.f, 0.0);
  // fix sign
  // res.w ^= sgn;
  // sign of zero correction
  res.w = (res.w == 0) ? sgn_x.w : (res.w ^ (sgn_x.w ^ sgn_N.w));
  *pres = res.f;
  nRet = (xa.f > __dsinpi_ha_max_norm.f) ? 1 : 0;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_sinpi_d_ha */
DEVICE_EXTERN_C_INLINE double __devicelib_imf_sinpi(double x) {
  using namespace __imf_impl_sinpi_d_ha;
  double r;
  __devicelib_imf_internal_dsinpi(&x, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
