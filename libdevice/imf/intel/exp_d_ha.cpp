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
namespace __imf_impl_exp_d_ha {
namespace {
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp_ha_p_L2E = {0x3ff71547652B82FEUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp_ha_p_Shifter = {0x43280000000007feUL};
// -log(2)_high
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp_ha_p_NL2H = {0xbfe62e42fefa39efUL};
// -log(2)_low
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp_ha_p_NL2L = {0xbc7abc9e3b39803fUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp_ha_p_c0 = {0x3fdffffffffffe76UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp_ha_p_c1 = {0x3fc5555555555462UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp_ha_p_c2 = {0x3fa55555556228ceUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp_ha_p_c3 = {0x3f811111111ac486UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp_ha_p_c4 = {0x3f56c16b8144bd5bUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp_ha_p_c5 = {0x3f2a019f7560fba3UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp_ha_p_c6 = {0x3efa072e44b58159UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp_ha_p_c7 = {0x3ec722bccc270959UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp_ha_p_one = {0x3ff0000000000000UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp_ha_thres = {0x4086232A00000000UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp_ha_min_norm = {0x0010000000000000UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexp_ha_Inf = {0x7ff0000000000000UL};
inline int __devicelib_imf_internal_dexp(const double *a, double *r) {
  int nRet = 0;
  double x = *a;
  union {
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
  } xi, zero, res_special, scale;
  union {
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
  } idx, T, Tlr;
  double N, R, R0, poly, res;
  int32_t expon32, mask32, mask_h;
  uint32_t xa32, sgn_x, expon_corr;
  // x*log2(e) + Shifter
  idx.f = __fma(x, __dexp_ha_p_L2E.f, __dexp_ha_p_Shifter.f);
  // x*log2(e), rounded to 1 fractional bit
  N = idx.f - __dexp_ha_p_Shifter.f;
  // bit mask to select "table" value
  mask32 = idx.w32[0] << 31;
  // prepare exponent
  expon32 = idx.w32[0] << (20 + 31 - 32);
  // initial reduced argument
  R0 = __fma(__dexp_ha_p_NL2H.f, N, x);
  // reduced argument
  R = __fma(__dexp_ha_p_NL2L.f, N, R0);
  // start polynomial computation
  poly = __fma(__dexp_ha_p_c7.f, R, __dexp_ha_p_c6.f);
  poly = __fma(poly, R, __dexp_ha_p_c5.f);
  // bit mask to select "table" value
  mask32 = mask32 >> 31;
  // polynomial
  poly = __fma(poly, R, __dexp_ha_p_c4.f);
  poly = __fma(poly, R, __dexp_ha_p_c3.f);
  // "table" correction
  // mask.w &= 0x000EA09E667F3BCDUL;
  mask_h = mask32 & 0x000EA09E;
  // polynomial
  poly = __fma(poly, R, __dexp_ha_p_c2.f);
  poly = __fma(poly, R, __dexp_ha_p_c1.f);
  // combine exponent, "table" value
  T.w32[1] = expon32 ^ mask_h;
  T.w32[0] = mask32 & 0x667F3BCD;
  Tlr.w32[1] = 0x3C6E51C5 ^ (mask32 & (0xBC8FD36E ^ 0x3C6E51C5)); // 0xBC93B3EF;
  Tlr.w32[0] = 0;
  // polynomial
  poly = __fma(poly, R, __dexp_ha_p_c0.f);
  poly = __fma(poly, R, __dexp_ha_p_one.f);
  poly = __fma(poly, R, Tlr.f);
  // if (xa32 > 0x4086232Au)
  if (__fabs(x) >= __dexp_ha_thres.f)
    goto EXP_SPECIAL_PATH;
  // result
  res = __fma(T.f, poly, T.f);
  *r = res;
  return nRet;
EXP_SPECIAL_PATH:
  xi.f = x;
  xa32 = xi.w32[1] & 0x7fffffffu;
  // sign of x
  sgn_x = xa32 ^ xi.w32[1];
  if (xa32 < 0x40879127u) {
    expon_corr = sgn_x ? 0x08000000u : 0xF8000000u;
    scale.w = sgn_x ? 0x37f0000000000000UL : 0x47f0000000000000UL;
    // apply correction (+/-128) to exponent embedded in T
    T.w32[1] += expon_corr;
    // result
    res = __fma(T.f, poly, T.f);
    // final scaling
    res *= scale.f;
  } else {
    // underflow or overflow?
    res_special.w = sgn_x ? 0x0000000000000000UL : 0x7ff0000000000000UL;
    // check for NaNs
    xi.w32[1] = xa32;
    res_special.f = (xi.w <= 0x7ff0000000000000UL) ? res_special.f : x;
    // quietize NaNs
    zero.w = 0;
    res = res_special.f + zero.f;
  }
  nRet = (res < __dexp_ha_min_norm.f) ? 4 : nRet;
  nRet = (res == __dexp_ha_Inf.f) ? 3 : nRet;
  *r = res;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_exp_d_ha */
DEVICE_EXTERN_C_INLINE double __devicelib_imf_exp(double x) {
  using namespace __imf_impl_exp_d_ha;
  double r;
  __devicelib_imf_internal_dexp(&x, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
