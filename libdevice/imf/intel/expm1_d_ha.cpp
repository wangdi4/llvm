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
namespace __imf_impl_expm1_d_ha {
namespace {
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexpm1_ha_Tbl_exp[] = {
    {0x0000000000000000UL}, {0x0000000000000000UL}, {0x0001b5586cf9890fUL},
    {0x3c979aa65d837b6dUL}, {0x000372b83c7d517bUL}, {0xbc801b15eaa59348UL},
    {0x0001387a6e756238UL}, {0x3c968efde3a8a894UL}, {0x000706fe0a31b715UL},
    {0x3c834d754db0abb6UL}, {0x0006dea64c123422UL}, {0x3c859f48a72a4c6dUL},
    {0x0002bfdad5362a27UL}, {0x3c7690cebb7aafb0UL}, {0x0002ab07dd485429UL},
    {0x3c9063e1e21c5409UL}, {0x000ea09e667f3bcdUL}, {0xbc93b3efbf5e2229UL},
    {0x000ea11473eb0187UL}, {0xbc7b32dcb94da51dUL}, {0x0002ace5422aa0dbUL},
    {0x3c8db72fc1f0eab5UL}, {0x0002c49182a3f090UL}, {0x3c71affc2b91ce27UL},
    {0x0006e89f995ad3adUL}, {0x3c8c1a7792cb3386UL}, {0x0001199bdd85529cUL},
    {0x3c736eae30af0cb3UL}, {0x00035818dcfba487UL}, {0x3c74a385a63d07a8UL},
    {0x0001a4afa2a490daUL}, {0xbc8ff7128fd391f0UL},
};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexpm1_ha_dc7 = {0x3efa01f8f4be0535UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexpm1_ha_dc6 = {0x3f2a01f8f4b129ccUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexpm1_ha_dc5 = {0x3f56c16c16304601UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexpm1_ha_dc4 = {0x3f81111110a65711UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexpm1_ha_dc3 = {0x3fa555555555560aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexpm1_ha_dc2 = {0x3fc55555555555f9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexpm1_ha_dc1 = {0x3fe0000000000000UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexpm1_ha_dc0 = {0xbc13b588106b310fUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexpm1_ha_Shifter = {0x4338000000003ff0UL};
//__STATIC __CONST INT_DOUBLE_TYPE _VSTATIC(L2E) = { 0x3ff71547652B82FEUL };
static const union {
  uint32_t w;
  float f;
} __dexpm1_ha_fL2E = {0x41B8AA3Bu};
static const union {
  uint32_t w;
  float f;
} __dexpm1_ha_fShifter = {0x4b403ff0u};
// -log(2)_high/16
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexpm1_ha_p_NL2H = {0xbfa62e42fefa39f0UL};
// -log(2)_low/16
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dexpm1_ha_p_NL2L = {0x3c5950d871319ff0UL};
inline int __devicelib_imf_internal_dexpm1(const double *pxin, double *pres) {
  int nRet = 0;
  double xin = *pxin;
  // double expm1(double xin)
  union {
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
  } x, T, Tlr, sc, xa, res;
  double dN, Rh, Rl, R, poly, A, B, Bh, Th, Tl;
  double H, Rhh, Rhl, ThRh, ThRh_l;
  int index;
  union {
    uint32_t w;
    float f;
  } xf, fN, xL2E, fS;
  x.f = xin;
  xf.f = (float)xin;
  xL2E.f = __fma(xf.f, __dexpm1_ha_fL2E.f, 0.0f);
  fN.f = __trunc(xL2E.f);
  fS.f = __dexpm1_ha_fShifter.f + fN.f;
  dN = (double)fN.f;
  index = (fS.w & 0xf) << 1;
  // reduced argument
  Rh = __fma(dN, __dexpm1_ha_p_NL2H.f, x.f);
  Rl = __fma(dN, __dexpm1_ha_p_NL2L.f, 0.0);
  R = Rh + Rl;
  // 2^N, N=(int)dN
  T.w32[1] = (fS.w << (20 - 4)) ^ __dexpm1_ha_Tbl_exp[index].w32[1];
  T.w32[0] = __dexpm1_ha_Tbl_exp[index].w32[0];
  Tlr.w32[1] = __dexpm1_ha_Tbl_exp[index + 1].w32[1];
  Tlr.w32[0] = 0;
  // Tlr.f = _VSTATIC(Tbl_exp)[index + 1].f + Rl;
  Tlr.f += Rl;
  // e^R - 1
  poly = __fma(__dexpm1_ha_dc7.f, R, __dexpm1_ha_dc6.f);
  poly = __fma(poly, R, __dexpm1_ha_dc5.f);
  poly = __fma(poly, R, __dexpm1_ha_dc4.f);
  poly = __fma(poly, R, __dexpm1_ha_dc3.f);
  poly = __fma(poly, R, __dexpm1_ha_dc2.f);
  poly = __fma(poly, R, __dexpm1_ha_dc1.f);
  poly = __fma(poly, R, __dexpm1_ha_dc0.f);
  poly = __fma(poly, R, Tlr.f);
  // maxabs(T,-1), minabs(T,-1)
  A = (xin >= 0.0) ? T.f : -1.0;
  B = (xin >= 0.0) ? -1.0 : T.f;
  Th = T.f - 1.0;
  Bh = Th - A;
  Tl = B - Bh;
  // T*Rh
  ThRh = __fma(T.f, Rh, 0.0);
  ThRh_l = __fma(T.f, Rh, -ThRh);
  // Th + Th*Rh
  H = ThRh + Th;
  // (Th*Rh)_high
  Rhh = H - Th;
  // (Th*Rh)_low
  Rhl = ThRh - Rhh;
  Tl = Tl + Rhl + ThRh_l;
  // 2^N*poly + Tl
  res.f = __fma(T.f, poly, Tl);
  res.f = res.f + H;
  if (__fabs(xf.f) <= 708.0f) {
    *pres = res.f;
    return nRet;
  }
  // special case and overflow path
  if (xf.f < 0) {
    *pres = -1.0;
    return nRet;
  }
  if (!(xf.f < 1024.0)) {
    // +Inf or NaN?
    xa.w = x.w & 0x7fffffffffffffffUL;
    if (xa.w > 0x7ff0000000000000UL) {
      *pres = x.f + res.f;
      return nRet;
    }
    // overflow
    res.w = 0x7ff0000000000000UL - 1;
    res.f = res.f * res.f; // to set OF flag
    nRet = 3;
    {
      *pres = res.f;
      return nRet;
    }
  }
  // at or near overflow
  // 2^(N-512), N=(int)dN
  T.w32[1] =
      ((fS.w - 512 * 16) << (20 - 4)) ^ __dexpm1_ha_Tbl_exp[index].w32[1];
  T.w32[0] = __dexpm1_ha_Tbl_exp[index].w32[0];
  // T.w = ((S.w - 512 * 16) << (52 - 4)) ^ _VSTATIC(Tbl_exp)[index].w;
  poly += Rh;
  res.f = __fma(T.f, poly, T.f);
  // final scaling
  sc.w = 0x5ff0000000000000UL;
  res.f *= sc.f;
  // determine if overflow
  if (res.w == 0x7ff0000000000000UL)
    nRet = 3;
  *pres = res.f;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_expm1_d_ha */
DEVICE_EXTERN_C_INLINE double __devicelib_imf_expm1(double x) {
  using namespace __imf_impl_expm1_d_ha;
  double r;
  __devicelib_imf_internal_dexpm1(&x, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
