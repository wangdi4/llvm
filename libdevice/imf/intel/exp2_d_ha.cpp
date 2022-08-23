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
namespace __imf_impl_exp2_d_ha {
namespace {
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp2_ha_Shifter = {0x43380000000003ffUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp2_ha_Shifter0 = {0x43380000000007feUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp2_ha_c11 = {0x3dfea1c678ded0efUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp2_ha_c10 = {0x3e3e6228be5a9ffdUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp2_ha_c9 = {0x3e7b524ca9ff39ccUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp2_ha_c8 = {0x3eb62bfc2c7be078UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp2_ha_c7 = {0x3eeffcbfc7e3f872UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp2_ha_c6 = {0x3f2430913112cae8UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp2_ha_c5 = {0x3f55d87fe78a0586UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp2_ha_c4 = {0x3f83b2ab6fb9f1a3UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp2_ha_c3 = {0x3fac6b08d704a0dbUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp2_ha_c2 = {0x3fcebfbdff82c5aeUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp2_ha_c1 = {0x3fc8b90bfbe8e7bcUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp2_ha_thres = {0x408ff00000000000UL};
inline int __devicelib_imf_internal_dexp2(const double *a, double *r) {
  int nRet = 0;
  double x = *a;
  union {
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
  } dS, xin, res, T, T2, expon;
  double dN, R, poly, High, Rh, Rl;
  uint32_t xa32;
  int32_t sN, sN2;
  dS.f = x + __dexp2_ha_Shifter.f;
  dN = dS.f - __dexp2_ha_Shifter.f;
  R = x - dN;
  poly = __fma(__dexp2_ha_c11.f, R, __dexp2_ha_c10.f);
  poly = __fma(poly, R, __dexp2_ha_c9.f);
  poly = __fma(poly, R, __dexp2_ha_c8.f);
  poly = __fma(poly, R, __dexp2_ha_c7.f);
  poly = __fma(poly, R, __dexp2_ha_c6.f);
  poly = __fma(poly, R, __dexp2_ha_c5.f);
  // 1+0.5*R
  High = __fma(R, 0.5, 1.0);
  poly = __fma(poly, R, __dexp2_ha_c4.f);
  // (0.5*R)_high
  Rh = High - 1.0;
  poly = __fma(poly, R, __dexp2_ha_c3.f);
  // (0.5*R)_low
  Rl = __fma(R, 0.5, -(Rh));
  poly = __fma(poly, R, __dexp2_ha_c2.f);
  poly = __fma(poly, R, __dexp2_ha_c1.f);
  poly = __fma(poly, R, Rl);
  res.f = High + poly;
  if (__fabs(x) >= __dexp2_ha_thres.f)
    goto EXP2_SPECIAL;
  // final scaling
  // res.w32[1] += (dS.w32[0] << 20);
  expon.w = dS.w << 52;
  res.f *= expon.f;
  *r = res.f;
  return nRet;
EXP2_SPECIAL:
  xin.f = x;
  xa32 = xin.w32[1] & 0x7fffffffUL;
  if (xa32 >= 0x7ff00000u) {
    if (xin.w == 0xfff0000000000000UL) {
      *r = 0.0f;
      return nRet;
    } else // NaN or +Inf
    {
      *r = x + x;
      return nRet;
    }
  }
  x = __fmin(x, 1536.0);
  x = __fmax(x, -1536.0);
  dS.f = x + __dexp2_ha_Shifter0.f;
  sN = dS.w32[0];
  // fix res.f for very large |x|
  res.f = __fmin(res.f, 2.0);
  res.f = __fmax(res.f, 0.5);
  // split the scaling coefficients
  sN2 = sN >> 1;
  sN -= sN2;
  T.w = (sN /*+ 0x3ff*/);
  T.w <<= 52;
  T2.w = (sN2 /*+ 0x3ff*/);
  T2.w <<= 52;
  res.f *= T.f;
  res.f *= T2.f;
  nRet = (res.w < 0x0010000000000000UL) ? 4 : nRet;
  nRet = (res.w == 0x7ff0000000000000UL) ? 3 : nRet;
  *r = res.f;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_exp2_d_ha */
DEVICE_EXTERN_C_INLINE double __devicelib_imf_exp2(double x) {
  using namespace __imf_impl_exp2_d_ha;
  double r;
  __devicelib_imf_internal_dexp2(&x, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
