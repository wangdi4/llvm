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
/*
// ALGORITHM DESCRIPTION:
//      *
//      *  Compute cosh(x) as (exp(x)+exp(-x))/2,
//      *  where exp is calculated as
//      *  exp(M*ln2 + ln2*(j/2^k) + r) = 2^M * 2^(j/2^k) * exp(r)
//      *
//      *  Special cases:
//      *
//      *  cosh(NaN) = quiet NaN, and raise invalid exception
//      *  cosh(INF) = that INF
//      *  cosh(0)   = 1
//      *  cosh(x) overflows for big x and returns MAXLOG+log(2)
//      
*/
#include "_imf_include_fp32.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_cosh_s_la {
namespace {
typedef struct {
  VUINT32 _sExp_tbl_PH[32];
  VUINT32 _sExp_tbl_PL[32];
  VUINT32 _sExp_tbl_NH[32];
  VUINT32 _sShifter_int;
  VUINT32 _iIndexMask_int;
  VUINT32 _iDomainRange_int;
  VUINT32 _sPC1_int;
  VUINT32 _sPC2_int;
  VUINT32 _sPC3_int;
  VUINT32 _sInvLn2;
  VUINT32 _sInvLn2lo;
  VUINT32 _sLn2hi;
  VUINT32 _sLn2lo;
  VUINT32 _sSign;
  VUINT32 _sOne;
  VUINT32 _sNOne;
  VUINT32 _iExpMask;
  /* Shared name but with different value */
  VUINT32 _sShifter;
  VUINT32 _iIndexMask;
  VUINT32 _iDomainRange;
  VUINT32 _sPC1;
  VUINT32 _sPC2;
  VUINT32 _sPC3;
  VUINT32 _sPC4;
  VUINT32 _sPC5;
  VUINT32 _sPC6;
  // Ints
  VUINT32 _iHalf;
} __devicelib_imf_internal_scosh_data_t;
static const __devicelib_imf_internal_scosh_data_t
    __devicelib_imf_internal_scosh_data = {
        {/* _sExp_tbl_PH 2^(i/32-1), i=0..31 */
         0x3f000000u, 0x3f02cd87u, 0x3f05aac3u, 0x3f08980fu, 0x3f0b95c2u,
         0x3f0ea43au, 0x3f11c3d3u, 0x3f14f4f0u, 0x3f1837f0u, 0x3f1b8d3au,
         0x3f1ef532u, 0x3f227043u, 0x3f25fed7u, 0x3f29a15bu, 0x3f2d583fu,
         0x3f3123f6u, 0x3f3504f3u, 0x3f38fbafu, 0x3f3d08a4u, 0x3f412c4du,
         0x3f45672au, 0x3f49b9beu, 0x3f4e248cu, 0x3f52a81eu, 0x3f5744fdu,
         0x3f5bfbb8u, 0x3f60ccdfu, 0x3f65b907u, 0x3f6ac0c7u, 0x3f6fe4bau,
         0x3f75257du, 0x3f7a83b3u},
        {/*  for i in [|0,...,31|] do printsingle( 2^(i/32-1) -
            round(2^(i/32-1),SG,RN)); */
         0x00000000u, 0xb2cea7a9u, 0x32cf9891u, 0xb2feda4bu, 0xb1e0aba1u,
         0xb2e97465u, 0x32e75624u, 0xb2ae0212u, 0x32a31b71u, 0xb28c5563u,
         0x32c12342u, 0x3043125au, 0xb2ac9d5eu, 0xb2962b08u, 0xb1adeaf6u,
         0xb2fc5aa8u, 0x324fe77au, 0x328ec5f7u, 0xb2c14fe8u, 0xb256663eu,
         0x318aa837u, 0xb2f323a2u, 0x31a8fc24u, 0xb2dc1daau, 0xb254a58au,
         0xb2d04a1cu, 0xb19eab59u, 0xb1c41be6u, 0xb1c116deu, 0xb2c8464au,
         0x31a92436u, 0xb2123758u},
        {/* _sExp_tbl_NH 2^(-i/32-1), i=0..31 */
         0x3f000000u, 0x3efa83b3u, 0x3ef5257du, 0x3eefe4bau, 0x3eeac0c7u,
         0x3ee5b907u, 0x3ee0ccdfu, 0x3edbfbb8u, 0x3ed744fdu, 0x3ed2a81eu,
         0x3ece248cu, 0x3ec9b9beu, 0x3ec5672au, 0x3ec12c4du, 0x3ebd08a4u,
         0x3eb8fbafu, 0x3eb504f3u, 0x3eb123f6u, 0x3ead583fu, 0x3ea9a15bu,
         0x3ea5fed7u, 0x3ea27043u, 0x3e9ef532u, 0x3e9b8d3au, 0x3e9837f0u,
         0x3e94f4f0u, 0x3e91c3d3u, 0x3e8ea43au, 0x3e8b95c2u, 0x3e88980fu,
         0x3e85aac3u, 0x3e82cd87u},
        0x48c00000u, /* 1.5*2^18 _sShifter_int */
        0x0000001fu, /* _iIndexMask_int   */
        0x42AEAC4Eu, /* _iDomainRange_int */
        0x3F800000u, /* _sPC1_int=1       */
        0x3f00010fu, /* _sPC2_int         */
        0x3e2aaacdu, /* _sPC3_int         */
        0x3FB8AA3Bu,
        /* _sInvLn2  */ // k=0
        0x32A57060u,    /* _sInvLn2lo*/
        0x3F317000u,    /* _sLn2hi   */
        0x3805fdf4u,    /* _sLn2lo   */
        0x80000000u,    /* _sSign    */
        0x3f800000u,    /* _sOne     */
        0xbf800000u,    /* _sNOne    */
        0x7f800000u,    /* _iExpMask */
        0x4b400000u,    /* _sShifter */
        0x0000001fu,    /* _iIndexMask */
        0x42AEAC4Eu,    /* _iDomainRange */
        0x3F800000u,    /* _sPC1=1  */
        0x3f000000u,    /* _sPC2  */
        0x3e2aaa57u,    /* _sPC3  */
        0x3d2aaa72u,    /* _sPC4  */
        0x3c091461u,    /* _sPC5  */
        0x3ab6a8a3u,    /* _sPC6  */
        // Integer constants
        0x3f000000u /* _iHalf*/
};                  /*_VAPI_DATA_TYPE*/
/* file: _vscosh_cout_ats.i */
static const union {
  uint32_t w;
  float f;
} __scosh_la_Shifter = {0x4ac000feu};
// log2(e)
static const union {
  uint32_t w;
  float f;
} __scosh_la_L2E = {0x3FB8AA3Bu};
// log(2) high, low
static const union {
  uint32_t w;
  float f;
} __scosh_la_L2H = {0x3f317218u};
static const union {
  uint32_t w;
  float f;
} __scosh_la_L2L = {0xb102E308u};
static const union {
  uint32_t w;
  float f;
} __scosh_la_c5 = {0x3c08ba8bu};
static const union {
  uint32_t w;
  float f;
} __scosh_la_c4 = {0x3d2aec4eu};
static const union {
  uint32_t w;
  float f;
} __scosh_la_c3 = {0x3e2aaa9cu};
static const union {
  uint32_t w;
  float f;
} __scosh_la_c2 = {0x3effffe8u};
static const union {
  uint32_t w;
  float f;
} __scosh_la_c1 = {0x3f800000u};
inline int __devicelib_imf_internal_scosh(const float *a, float *r) {
  int nRet = 0;
  float x = __fabs(*a);
  union {
    uint32_t w;
    float f;
  } S, Th, Tlr, Th2, xin, xa, res;
  float N, R, poly;
  int index_mask;
  S.f = __fma(x, __scosh_la_L2E.f, __scosh_la_Shifter.f);
  N = S.f - __scosh_la_Shifter.f;
  R = __fma(-(N), __scosh_la_L2H.f, x);
  R = __fma(-(N), __scosh_la_L2L.f, R);
  // set exponent in place
  Th.w = S.w << 22;
  // index_mask is based on last bit of S.w
  index_mask = 0 - (S.w & 1);
  // set Th mantissa
  Th.w ^= (index_mask & 0x7504F3u);
  // set Tl/Th value
  Tlr.w = index_mask & 0x329302AEu;
  // polynomial
  poly = __fma(R, __scosh_la_c5.f, __scosh_la_c4.f);
  poly = __fma(R, poly, __scosh_la_c3.f);
  poly = __fma(R, poly, __scosh_la_c2.f);
  poly = __fma(R, poly, __scosh_la_c1.f);
  poly = __fma(R, poly, Tlr.f);
  xin.f = x;
  xa.w = xin.w & 0x7fffffffu;
  // redirect special cases
  if (xa.w > 0x42AEAC4Fu)
    goto COSHF_SPECIAL;
  res.f = __fma(poly, Th.f, Th.f);
  *r = 0.5f * res.f;
  return nRet;
COSHF_SPECIAL:
  if (xa.w > 0x42b2d4fcu) {
    if (xa.w > 0x7f800000u) { // NaN?
      *r = x + x;
      return nRet;
    }
    // overflow
    res.w = 0x7f800000;
    *r = res.f;
    nRet = 3;
    return nRet;
  }
  S.w += 0xfe;
  Th2.w = (S.w >> 2) & 0xff;
  S.w -= (Th2.w << 1);
  Th2.w <<= 23; // second exponent scale
  Th.w = S.w << 22;
  // set Th mantissa
  Th.w ^= (index_mask & 0x7504F3u);
  res.f = 0.5f * __fma(poly, Th.f, Th.f);
  res.f *= Th2.f;
  *r = res.f;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_cosh_s_la */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_coshf(float a) {
  using namespace __imf_impl_cosh_s_la;
  VUINT32 vm;
  float va1;
  float vr1;
  float r;
  va1 = a;
  ;
  {
    float sN;
    float sM;
    float sR;
    float sR2;
    float sSinh_r;
    float sOut;
    float sG1;
    float sG2;
    float sXSign;
    float sAbsX;
    VUINT32 iM;
    VUINT32 iAbsX;
    VUINT32 iRangeMask;
    float sInvLn2;
    float sShifter;
    float sPC[6];
    VUINT32 iHalf;
    VUINT32 iDomainRange;
    sInvLn2 = as_float(__devicelib_imf_internal_scosh_data._sInvLn2);
    sShifter = as_float(__devicelib_imf_internal_scosh_data._sShifter);
    sPC[0] = as_float(__devicelib_imf_internal_scosh_data._sPC1); // x^1
    sPC[1] = as_float(__devicelib_imf_internal_scosh_data._sPC2); // x^2
    sPC[2] = as_float(__devicelib_imf_internal_scosh_data._sPC3); //...
    sPC[3] = as_float(__devicelib_imf_internal_scosh_data._sPC4);
    sPC[4] = as_float(__devicelib_imf_internal_scosh_data._sPC5);
    sPC[5] = as_float(__devicelib_imf_internal_scosh_data._sPC6);
    sXSign = as_float(__devicelib_imf_internal_scosh_data._sSign);
    iHalf = (__devicelib_imf_internal_scosh_data._iHalf);
    iDomainRange = (__devicelib_imf_internal_scosh_data._iDomainRange);
    /* -------------------- Implementation  ------------------- */
    /* ............... Abs argument ............................ */
    sAbsX = as_float((~(as_uint(sXSign)) & as_uint(va1)));
    /* ............... Load argument ............................ */
    // dM = x/log(2) + RShifter
    sM = __fma(sAbsX, sInvLn2, sShifter);
    /* ...............Check for overflow\underflow ............. */
    // iAbsX = bitimage(abs(x))
    iAbsX = as_uint(sAbsX);
    iRangeMask =
        ((VUINT32)(-(VSINT32)((VSINT32)iAbsX >= (VSINT32)iDomainRange)));
    vm = 0;
    vm = iRangeMask;
    /* R computation: */
    // sN = sM - RShifter
    sN = (sM - sShifter);
    sOut = as_float(__devicelib_imf_internal_scosh_data._sLn2hi);
    // sR = sX - sN*Log2_hi
    sR = __fma(-(sOut), sN, sAbsX);
    sOut = as_float(__devicelib_imf_internal_scosh_data._sLn2lo);
    // sR = (sX - sN*Log2_hi) - sN*Log2_lo
    sR = __fma(-(sOut), sN, sR);
    /* G1,G2 2^N,2^(-N) computation: */
    iM = as_uint(sM);
    // iM now is an EXP(2^N)
    iM = ((VUINT32)(iM) << (23));
    // sR2 = sR^2,shaffled
    sR2 = (sR * sR);
    iAbsX = (iHalf + iM);
    // sG1=2^(N-1)
    sG1 = as_float(iAbsX);
    iAbsX = (iHalf - iM);
    // sG2=2^(-N-1)
    sG2 = as_float(iAbsX);
    sM = sG1;
    // sG1 = 2^(N-1)-2^(-N-1)
    sG1 = (sG1 - sG2);
    // sG2 = 2^(N-1)+2^(-N-1)
    sG2 = (sM + sG2);
    // sSinh_r = (a3+r^2*a5)
/*
//     ....sinh(r) = r*((a1=1)+r^2*(a3+r^2*(a5+{v1 r^2*a7})))) = r +
//      * r*(r^2*(a3+r^2*(a5+r^2*a7))) ....
*/
    sSinh_r = __fma(sPC[4], sR2, sPC[2]);
    sSinh_r = (sSinh_r * sR2);
    // sSinh_r = r + r*(r^2*(a3+r^2*a5))
    sSinh_r = __fma(sSinh_r, sR, sR);
    /*sinh(X) = sG2 + sG1*sinh(dR) + sG2*sR2*(a2+sR2*(a4+a6*sR2) */
    // sOut = (a4 +a6*sR2)
    sOut = __fma(sPC[5], sR2, sPC[3]);
    // sOut = a2+sR2*(a4+a6*sR2)
    sOut = __fma(sOut, sR2, sPC[1]);
    // sOut = sR2*(a2+sR2*(a4+a6*sR2)
    sOut = (sOut * sR2);
    // sOut = sG2*sR2*(a2+sR2*(a4+a6*sR2)
    sOut = (sOut * sG2);
    // sOut = sG1*sinh(dR)+sG2*sR2*(a2+sR2*(a4+a6*sR2)
    sOut = __fma(sG1, sSinh_r, sOut);
    // Result = sG2 + sG1*sinh(dR) + sG2*sR2*(a2+sR2*(a4+a6*sR2)
    vr1 = (sOut + sG2);
    /* ................... Ret H ...................... */
  }
  if (__builtin_expect((vm) != 0, 0)) {
    float __cout_a1;
    float __cout_r1;
    ((float *)&__cout_a1)[0] = va1;
    ((float *)&__cout_r1)[0] = vr1;
    __devicelib_imf_internal_scosh(&__cout_a1, &__cout_r1);
    vr1 = ((const float *)&__cout_r1)[0];
  }
  r = vr1;
  ;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
