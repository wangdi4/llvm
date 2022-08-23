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
namespace __imf_impl_cbrt_d_ha {
namespace {
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcbrt_ha___c20 = {0xbf4aab4475c7ef1dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcbrt_ha___c19 = {0x3f6845d681374289UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcbrt_ha___c18 = {0xbf750d677402621aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcbrt_ha___c17 = {0x3f7919308ccde911UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcbrt_ha___c16 = {0xbf7a28cf5604db64UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcbrt_ha___c15 = {0x3f7ba63c9b8e020dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcbrt_ha___c14 = {0xbf7e454bb3957abaUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcbrt_ha___c13 = {0x3f80c67d884efe94UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcbrt_ha___c12 = {0xbf82b43f401b165dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcbrt_ha___c11 = {0x3f850a3bd549b4daUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcbrt_ha___c10 = {0xbf87f0ec417b8c4cUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcbrt_ha___c9 = {0x3f8b9fd77d05593fUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcbrt_ha___c8 = {0xbf9036de5b78af16UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcbrt_ha___c7 = {0x3f93750adbcc9f27UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcbrt_ha___c6 = {0xbf98090d625599c1UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcbrt_ha___c5 = {0x3f9ee71134f3a579UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcbrt_ha___c4 = {0xbfa511e8d2b2bc58UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcbrt_ha___c3 = {0x3faf9add3c0cb194UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcbrt_ha___c2 = {0xbfbc71c71c71c74cUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcbrt_ha___c1 = {0x3fd5555555555555UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcbrt_ha___c0 = {0x3c1c5f9b7eb4a982UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcbrt_ha___maxnum = {0x7fefffffffffffffUL};
inline int __devicelib_imf_internal_dcbrt(const double *a, double *pres) {
  int nRet = 0;
  double xin = *a;
  int_double xa, res;
  uint64_t sgn_x, ecorr, expon, k, j, P;
  double dR;
  int_double poly, scale, two_j, two_jl, mant, r_expon, pl;
  xa.f = xin;
  sgn_x = xa.w & 0x8000000000000000UL;
  // |xin|
  xa.w ^= sgn_x;
  // will scale denormals by 2^69
  scale.w = (xa.w < 0x0010000000000000UL) ? 0x4440000000000000UL
                                          : 0x3ff0000000000000UL;
  // final exponent correction
  ecorr = (xa.w < 0x0010000000000000UL) ? 0x2aa - 23 : 0x2aa;
  xa.f = __fma(xa.f, scale.f, 0.0);
  // input exponent (expon = 3*k+j)
  expon = (xa.w + 0x0008000000000000UL) >> 52;
  // (2^32+2)/3  * exponent
  P = (uint64_t)expon * (uint64_t)0x55555556UL;
  k = P >> 32;
  j = expon - k - k - k;
  r_expon.w = (k + ecorr) << 52;
  // correction for xin==0.0
  r_expon.w = (!xa.w) ? 0 : r_expon.w;
  // mantissa
  mant.w = (xa.w - (expon << 52)) + 0x3ff0000000000000UL;
  // reduced argument
  dR = mant.f - 1;
  // polynomial evaluation
  poly.f = __fma(__dcbrt_ha___c20.f, dR, __dcbrt_ha___c19.f);
  poly.f = __fma(poly.f, dR, __dcbrt_ha___c18.f);
  poly.f = __fma(poly.f, dR, __dcbrt_ha___c17.f);
  poly.f = __fma(poly.f, dR, __dcbrt_ha___c16.f);
  poly.f = __fma(poly.f, dR, __dcbrt_ha___c15.f);
  poly.f = __fma(poly.f, dR, __dcbrt_ha___c14.f);
  poly.f = __fma(poly.f, dR, __dcbrt_ha___c13.f);
  poly.f = __fma(poly.f, dR, __dcbrt_ha___c12.f);
  poly.f = __fma(poly.f, dR, __dcbrt_ha___c11.f);
  poly.f = __fma(poly.f, dR, __dcbrt_ha___c10.f);
  poly.f = __fma(poly.f, dR, __dcbrt_ha___c9.f);
  poly.f = __fma(poly.f, dR, __dcbrt_ha___c8.f);
  poly.f = __fma(poly.f, dR, __dcbrt_ha___c7.f);
  poly.f = __fma(poly.f, dR, __dcbrt_ha___c6.f);
  poly.f = __fma(poly.f, dR, __dcbrt_ha___c5.f);
  poly.f = __fma(poly.f, dR, __dcbrt_ha___c4.f);
  poly.f = __fma(poly.f, dR, __dcbrt_ha___c3.f);
  poly.f = __fma(poly.f, dR, __dcbrt_ha___c2.f);
  poly.f = __fma(poly.f, dR, __dcbrt_ha___c1.f);
  poly.f = __fma(poly.f, dR, __dcbrt_ha___c0.f);
  // 2^j
  two_j.w = (!j) ? 0x3ff0000000000000UL : 0x3ff428A2F98D728BUL;
  two_j.w = (j <= 1) ? two_j.w : 0x3ff965FEA53D6E3DUL;
  // (2^j)_low/(2^j)_high
  two_jl.w = (!j) ? 0x0UL : 0xBC77B3273BEB351CUL;
  two_jl.w = (j <= 1) ? two_jl.w : 0xBC93BC3A711B3CF7UL;
  // attach exponent
  two_j.f = __fma(two_j.f, r_expon.f, 0.0);
  pl.f = __fma(two_jl.f, poly.f, two_jl.f);
  poly.f = poly.f + pl.f;
  res.f = __fma(two_j.f, poly.f, two_j.f);
  // fixup for Inf/NaN
  res.f = (xa.f <= __dcbrt_ha___maxnum.f) ? res.f : (xa.f + xa.f);
  // set sign
  res.w ^= sgn_x;
  *pres = res.f;
  return res.f;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_cbrt_d_ha */
DEVICE_EXTERN_C_INLINE double __devicelib_imf_cbrt(double x) {
  using namespace __imf_impl_cbrt_d_ha;
  double r;
  __devicelib_imf_internal_dcbrt(&x, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
