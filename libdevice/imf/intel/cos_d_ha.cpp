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
namespace __imf_impl_cos_d_ha {
namespace {
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcos_ha_INV_PI64 = {0x3fd45f306dc9c883UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcos_ha_SHIFTER = {0x4328000000000000UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcos_ha_PI1_BITS = {0xc00921fb54442d18UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcos_ha_PI2_BITS = {0xbca1a62633000000UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcos_ha_PI3_BITS = {0xbaa45c06e0e68948UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcos_ha_two = {0x4000000000000000UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcos_ha_half = {0x3fe0000000000000UL};
// cos(x) polynomial. |x|<pi/4
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcos_ha_c_coeff[] = {
    {0x3ff0000000000000UL}, {0xbfe0000000000000UL}, {0x3fa555555555554bUL},
    {0xbf56C16C16C14E4AUL}, {0x3efA01A019C687A9UL}, {0xbe927E4F7D8E4BF3UL},
    {0x3e21EE9CCB7C6DBFUL}, {0xbda8F9F637C8424CUL},
};
// sin(x) polynomial, |x|<pi/4
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dcos_ha_s_coeff[] = {
    {0},
    {0},
    {0xbfc5555555555543UL},
    {0x3f8111111110f3e0UL},
    {0xbf2a01a019ba9d47UL},
    {0x3ec71de35310c44bUL},
    {0xbe5ae5e382f0f31cUL},
    {0x3de5d877cbbe8daaUL},
};
static const uint64_t __dcos_ha_AbsMask = 0x7fffffffffffffffUL;
static const uint64_t __dcos_ha_zero = 0x0000000000000000UL;
///////
// 2^1152/(2*Pi)
static const uint32_t __dcos_ha_InvPi_tbl[] = {
    0,           0,           0x28BE60DBu, 0x9391054Au,
    0x7F09D5F4u, 0x7D4D3770u, 0x36D8A566u, 0x4F10E410u,
    0x7F9458EAu, 0xF7AEF158u, 0x6DC91B8Eu, 0x909374B8u,
    0x01924BBAu, 0x82746487u, 0x3F877AC7u, 0x2C4A69CFu,
    0xBA208D7Du, 0x4BAED121u, 0x3A671C09u, 0xAD17DF90u,
    0x4E64758Eu, 0x60D4CE7Du, 0x272117E2u, 0xEF7E4A0Eu,
    0xC7FE25FFu, 0xF7816603u, 0xFBCBC462u, 0xD6829B47u,
    0xDB4D9FB3u, 0xC9F2C26Du, 0xD3D18FD9u, 0xA797FA8Bu,
    0x5D49EEB1u, 0xFAF97C5Eu, 0xCF41CE7Du, 0xE294A4BAu,
    0x9AFED7ECu, 0x47E35742u, 0x1580CC11u, 0xBF1EDAEAu,
    0,           0,           0,           0};
// (2*pi)*2^61
static const uint32_t __dcos_ha_two_pi[2] = {0x2168C235u, 0xC90FDAA2u};
// unsigned 32-bit shift
// signed 32-bit shift
// unsigned 64-bit shift
// signed 64-bit shift
// reduce argument to (-2*pi/(2^kbits), 2*pi/(2^kbits)) range
static inline double __dcos_ha_trig_reduction(double x, int kbits,
                                              double *py_low, int *pinterv) {
  uint64_t ix, sgn_x, abs_x, mx, R;
  uint64_t S[2], P2, P3, P4, P5, L, L2, Lh, P23, P23_l, Msk;
  int64_t e_yl, e_yh;
  volatile uint32_t T[7];
  uint32_t mant[2], Sh, Sl;
  int32_t Syl;
  int expon_x, j, shift, shift2, scale, interv = 0;
  int cond;
  double yl, res;
  union {
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
  } ixd;
  ixd.f = x;
  ix = ixd.w;
  abs_x = ix & 0x7fffffffffffffffUL;
  sgn_x = ix ^ abs_x;
  // biased exponent
  expon_x = (((uint32_t)((uint32_t)(abs_x >> 32))) >> (20));
  // mantissa
  mant[0] = (uint32_t)abs_x;
  mant[1] = (((uint32_t)(abs_x >> 32)) & 0xfffffu) | 0x100000u;
  // eliminate smaller |x|, as well as Inf/NaN
  cond = ((unsigned)(expon_x - 0x400) >= (0x7ff - 0x400));
  if (cond) {
    *py_low = 0;
    *pinterv = 0;
    return x;
  }
  // starting table index for argument reduction
  // j >= 1 for expon_x >= 20+0x3ff
  expon_x = expon_x + 12 - 0x3ff;
  j = (((uint32_t)(expon_x)) >> (5));
  // look up table values
  T[0] = __dcos_ha_InvPi_tbl[j];
  T[1] = __dcos_ha_InvPi_tbl[j + 1];
  T[2] = __dcos_ha_InvPi_tbl[j + 2];
  T[3] = __dcos_ha_InvPi_tbl[j + 3];
  T[4] = __dcos_ha_InvPi_tbl[j + 4];
  T[5] = __dcos_ha_InvPi_tbl[j + 5];
  T[6] = __dcos_ha_InvPi_tbl[j + 6];
  // shift in [0, 31]
  shift = expon_x - (j << 5);
  // shift left
  if (shift) {
    shift2 = 32 - shift;
    T[0] = (T[0] << shift) | (((uint32_t)(T[1])) >> (shift2));
    T[1] = (T[1] << shift) | (((uint32_t)(T[2])) >> (shift2));
    T[2] = (T[2] << shift) | (((uint32_t)(T[3])) >> (shift2));
    T[3] = (T[3] << shift) | (((uint32_t)(T[4])) >> (shift2));
    T[4] = (T[4] << shift) | (((uint32_t)(T[5])) >> (shift2));
    T[5] = (T[5] << shift) | (((uint32_t)(T[6])) >> (shift2));
  }
  // InvPi*mant_x
  S[0] = (((uint64_t)((uint32_t)(T[3]))) * ((uint32_t)(mant[0])));
  P4 = (((uint64_t)((uint32_t)(T[4]))) * ((uint32_t)(mant[1])));
  L = (((uint64_t)((uint32_t)(T[4]))) * ((uint32_t)(mant[0])));
  L2 = (((uint64_t)((uint32_t)(T[5]))) * ((uint32_t)(mant[1])));
  L += L2;
  if (L < L2)
    S[0] += 0x100000000UL;
  P2 = (((uint64_t)((uint32_t)(T[2]))) * ((uint32_t)(mant[0])));
  S[1] = (((uint64_t)((uint32_t)(T[1]))) * ((uint32_t)(mant[0]))) +
         (((uint64_t)(((uint32_t)T[0]) * ((uint32_t)mant[0]))) << 32);
  S[0] += P4;
  if (S[0] < P4)
    S[1]++;
  Lh = (((uint64_t)(L)) >> (32));
  L <<= 32;
  S[0] += Lh;
  if (S[0] < Lh)
    S[1]++;
  P3 = (((uint64_t)((uint32_t)(T[3]))) * ((uint32_t)(mant[1])));
  S[1] = S[1] + (((uint64_t)((uint32_t)(T[2]))) * ((uint32_t)(mant[1]))) +
         (((uint64_t)(((uint32_t)T[1]) * ((uint32_t)mant[1]))) << 32);
  // accumulate terms
  P23 = P2 + P3;
  // add carry
  if (P23 < P3)
    S[1] += 0x100000000UL;
  S[1] += (((uint64_t)(P23)) >> (32));
  P23_l = P23 << 32;
  S[0] += P23_l;
  if (S[0] < P23_l)
    S[1]++;
  if (kbits) {
    shift2 = 32 - kbits;
    interv = (((uint64_t)((((uint64_t)(S[1])) >> (32)))) >> (shift2));
    S[1] = (S[1] << kbits) |
           (((uint64_t)((((uint64_t)(S[0])) >> (32)))) >> (shift2));
    S[0] =
        (S[0] << kbits) | (((uint64_t)((((uint64_t)(L)) >> (32)))) >> (shift2));
    L <<= kbits;
  }
  // round intev to nearest
  Msk = (((int64_t)(S[1])) >> (63));
  interv = interv - (int32_t)Msk;
  S[1] ^= Msk;
  S[0] ^= Msk;
  L ^= Msk;
  // apply sign to interv, then correction to sign of fraction
  sgn_x = (((int64_t)(sgn_x)) >> (63));
  *pinterv = (interv ^ (int32_t)sgn_x) - (int32_t)sgn_x;
  sgn_x = (sgn_x ^ Msk) & 0x8000000000000000UL;
  scale = -64 - kbits;
  // normalization: leading bit of S[1] should be 1
  while (((int64_t)S[1]) > 0) {
    scale--;
    S[1] = (S[1] << 1) | (((uint64_t)(S[0])) >> (63));
    S[0] = (S[0] << 1) | (((uint64_t)(L)) >> (63));
    L <<= 1;
  }
  // multiply by 2*Pi*(2^61)
  Sh = (uint32_t)(((uint64_t)(S[1])) >> (32));
  Sl = (uint32_t)S[1];
  R = (((uint64_t)((uint32_t)(Sh))) * ((uint32_t)(__dcos_ha_two_pi[1])));
  P2 = (((uint64_t)((uint32_t)(Sh))) * ((uint32_t)(__dcos_ha_two_pi[0])));
  P3 = (((uint64_t)((uint32_t)(Sl))) * ((uint32_t)(__dcos_ha_two_pi[1])));
  // accumulate terms
  P23 = P2 + P3;
  // add carry
  if (P23 < P3)
    R++;
  // R is result*2^(scale+3)
  R += (((uint64_t)(P23)) >> (32));
  scale += 3;
  // normalize
  if (((int64_t)R) < 0) {
    R = (((uint64_t)(R)) >> (1));
    scale++;
  }
  // round upper 53 bits
  Syl = (uint32_t)R;
  R += (uint64_t)(1 << 9);
  // determine y_low
  Syl <<= (32 - 10);
  Syl = (((int32_t)(Syl)) >> (32 - 10));
  // SINT32 to double conversion
  yl = (double)Syl;
  // adjust exponent of yl
  e_yl = (int64_t)scale;
  e_yl = ((e_yl + 0x3ff) << 52) | sgn_x;
  // y_low
  *py_low = yl * (*(double *)&e_yl);
  // exponent of high part
  e_yh = (uint64_t)(scale + 62 - 1 + 0x3ff);
  e_yh = (e_yh << 52);
  // high part of result
  R = e_yh + (((uint64_t)(R)) >> (10));
  *(uint64_t *)&res = sgn_x ^ R;
  return res;
}
inline int __devicelib_imf_internal_dcos(const double *a, double *pres) {
  int nRet = 0;
  double xin = *a;
  unsigned leading_xh;
  int cond;
  int index;
  unsigned rsgn;
  double dN, Rh, Rm, R, dNP2_h, R2, dNP3_h;
  double mNP2_l, mNP3_l, Rl, R3;
  union {
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
  } x, x0, res[2], dS, cos_res;
  double s_poly, c_poly, R_Rl, R2h, R2l, Ch;
  uint64_t R_sgn, C_sgn, lindex;
  union {
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
  } c7, c6, c5, c4, c3, c2, c_poly2, s_poly2;
  // x0.f = xin;
  x.f = __fabs(xin); // x.w = x0.w & (_VSTATIC(AbsMask));
                                      // redirect special cases
  leading_xh = ((unsigned)x.w32[1]);  // & 0x7fffffff;
  cond = ((unsigned)(leading_xh) >= (0x41300000 - 0));
  if (cond)
    goto COS_SPECIAL;
  // _VSTATIC(SHIFTER) + x*(1/pi)
  dS.f = __fma(x.f, (__dcos_ha_INV_PI64).f,
                                (__dcos_ha_SHIFTER).f);
  // N ~ x*(1/pi)
  dN = dS.f - (__dcos_ha_SHIFTER).f;
  C_sgn = (dS).w;
  R_sgn = ((dS).w >> 1) ^ C_sgn;
  R_sgn <<= 63;
  lindex = dS.w & 1;
  // Rh = x - N*PI1
  Rh = __fma(dN, (__dcos_ha_PI1_BITS).f, x.f);
  // Rm = Rh - N*PI2
  Rm = __fma(dN, (__dcos_ha_PI2_BITS).f,
                              Rh); // FENCE(Rh - dNP2);
  // R = Rm - N*PI3
  R = __fma(dN, (__dcos_ha_PI3_BITS).f,
                             Rm); // FENCE(Rm - dNP3);
  // (N*PI2)_high
  dNP2_h = Rh - Rm;
  // R^2
  R2 = R * R;
  // (N*PI3)_high
  dNP3_h = Rm - R;
  // -(N*PI2)_l
  mNP2_l = __fma(dN, (__dcos_ha_PI2_BITS).f,
                                  dNP2_h); // FENCE(dNP2_h - dNP2);
  // -(N*PI3)_l
  mNP3_l = __fma(dN, (__dcos_ha_PI3_BITS).f,
                                  dNP3_h); // FENCE(dNP3_h - dNP3);
  // Rl=-(N*PI2)_l-(N*PI3)_l
  Rl = mNP2_l + mNP3_l;
COS_MAIN_PATH:
  // R*Rl
  R_Rl = R * Rl;
  c7.w = (lindex == 0) ? __dcos_ha_c_coeff[7].w : __dcos_ha_s_coeff[7].w;
  c6.w = (lindex == 0) ? __dcos_ha_c_coeff[6].w : __dcos_ha_s_coeff[6].w;
  c5.w = (lindex == 0) ? __dcos_ha_c_coeff[5].w : __dcos_ha_s_coeff[5].w;
  c4.w = (lindex == 0) ? __dcos_ha_c_coeff[4].w : __dcos_ha_s_coeff[4].w;
  c3.w = (lindex == 0) ? __dcos_ha_c_coeff[3].w : __dcos_ha_s_coeff[3].w;
  c2.w = (lindex == 0) ? __dcos_ha_c_coeff[2].w : __dcos_ha_s_coeff[2].w;
  c_poly = __fma(R2, c7.f, c6.f);
  c_poly = __fma(R2, c_poly, c5.f);
  c_poly = __fma(R2, c_poly, c4.f);
  c_poly = __fma(R2, c_poly, c3.f);
  c_poly = __fma(R2, c_poly, c2.f) * R2;
  // 2.0 - R^2
  Ch = __fma(-(R), R, (__dcos_ha_two).f);
  // (-R^2)_high
  R2h = Ch - (__dcos_ha_two).f;
  // (R^2)_low
  R2l = __fma(R, R, R2h);
  // 0.5*R2l+ R*Rl
  R2l = __fma(R2l, (__dcos_ha_half).f, R_Rl);
  c_poly2.f = __fma(R2, c_poly, -(R2l));
  c_poly2.f = __fma(Ch, (__dcos_ha_half).f, c_poly2.f);
  s_poly2.f = __fma(
      R, c_poly, Rl); //      s_poly = DP_FMA(R3, s_poly, Rl);
  s_poly2.f = s_poly2.f + R;
  // hopefully, a SEL will be generated
  cos_res.w = (lindex == 0) ? c_poly2.w : s_poly2.w;
  cos_res.w ^= R_sgn;
  *pres = cos_res.f;
  return nRet;
COS_SPECIAL:
  // Inf/NaN?
  if (leading_xh >= 0x7ff00000) {
    //*psin = *pcos = xin*U64_TO_DP(_VSTATIC(zero));
    // NaN?
    if ((x.w & (__dcos_ha_AbsMask)) > 0x7ff0000000000000UL) {
      *pres = x.f * x.f;
      return nRet;
    }
    res[0].w = 0xfff8000000000000UL;
    *pres = res[0].f;
    nRet = 1;
    return nRet;
  }
  R = __dcos_ha_trig_reduction(x.f, 2, &Rl, &index);
  R2 = R * R;
  lindex = (uint64_t)index;
  C_sgn = (lindex);
  lindex = (index >> 1);
  lindex ^= C_sgn;
  lindex <<= 63;
  R_sgn = (lindex);
  lindex = C_sgn & 1; // index &= 1;
                      // goto COS_MAIN_PATH;
                      // R^3
  R3 = R2 * R;
  // add low part of initial reduction (for large arguments)
  // Rl = FENCE(Rl + xlow);
  // R*Rl
  R_Rl = R * Rl;
  c_poly = __fma(R2, (__dcos_ha_c_coeff[7]).f,
                                  (__dcos_ha_c_coeff[6]).f);
  s_poly = __fma(R2, (__dcos_ha_s_coeff[7]).f,
                                  (__dcos_ha_s_coeff[6]).f);
  c_poly = __fma(R2, c_poly, (__dcos_ha_c_coeff[5]).f);
  s_poly = __fma(R2, s_poly, (__dcos_ha_s_coeff[5]).f);
  c_poly = __fma(R2, c_poly, (__dcos_ha_c_coeff[4]).f);
  s_poly = __fma(R2, s_poly, (__dcos_ha_s_coeff[4]).f);
  c_poly = __fma(R2, c_poly, (__dcos_ha_c_coeff[3]).f);
  s_poly = __fma(R2, s_poly, (__dcos_ha_s_coeff[3]).f);
  c_poly = __fma(R2, c_poly, (__dcos_ha_c_coeff[2]).f) * R2;
  s_poly = __fma(R2, s_poly, (__dcos_ha_s_coeff[2]).f);
  // 2.0 - R^2
  Ch = __fma(-(R), R, (__dcos_ha_two).f);
  // (-R^2)_high
  R2h = Ch - (__dcos_ha_two).f;
  // (R^2)_low
  R2l = __fma(R, R, R2h);
  // 0.5*R2l+ R*Rl
  R2l = __fma(R2l, (__dcos_ha_half).f, R_Rl);
  c_poly = __fma(R2, c_poly, -(R2l));
  res[0].f = __fma(Ch, (__dcos_ha_half).f, c_poly);
  s_poly = __fma(R3, s_poly, Rl);
  res[1].f = s_poly + R;
  // hopefully, a SEL will be generated
  // cos_res.f = (index == 0) ? res[0].f : res[1].f;
  lindex = 0 - lindex;
  cos_res.w = res[0].w ^ ((res[1].w ^ res[0].w) & lindex);
  (cos_res).w ^= R_sgn;
  *pres = cos_res.f;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_cos_d_ha */
DEVICE_EXTERN_C_INLINE double __devicelib_imf_cos(double x) {
  using namespace __imf_impl_cos_d_ha;
  double r;
  __devicelib_imf_internal_dcos(&x, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
