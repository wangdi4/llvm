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
/*
// ALGORITHM DESCRIPTION:
//
//  1) Range reduction to [-Pi/2; +Pi/2] interval
//     a) We remove sign using AND operation
//     b) Add Pi/2 value to argument X for Cos to Sin transformation
//     c) Getting octant Y by 1/Pi multiplication
//     d) Add "Right Shifter" value
//     e) Treat obtained value as integer for destination sign setting.
//        Shift first bit of this value to the last (sign) position
//     f) Subtract "Right Shifter"  value
//     g) Subtract 0.5 from result for octant correction
//     h) Subtract Y*PI from X argument, where PI divided to 4 parts:
//        X = X - Y*PI1 - Y*PI2 - Y*PI3 - Y*PI4;
//  2) Polynomial (minimax for sin within [-Pi/2; +Pi/2] interval)
//     a) Calculate X^2 = X * X
//     b) Calculate polynomial:
//        R = X + X * X^2 * (A3 + x^2 * (A5 + .....
//  3) Destination sign setting
//     a) Set shifted destination sign using XOR operation:
//        R = XOR( R, S );
//
*/
#include "_imf_include_fp32.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_cos_h_la {
namespace {
typedef struct {
  VUINT16 __absmask_h;    // 0x7fff
  VUINT16 __threshold_h;  // 0x4bd9
  VUINT16 __Pio2_h;       // 0x3e48
  VUINT16 __RShifter_h;   // 0x5200
  VUINT16 __InvPi_h;      // 0x2118
  VUINT16 __half_sc_h;    // 0x2400
  VUINT16 __nPi1_h;       // 0xd648
  VUINT16 __nPi2_h;       // 0xa7ed
  VUINT16 __sgnmask_h;    // 0x0001
  VUINT16 __c4_h;         // 0x8a2d
  VUINT16 __c3_h;         // 0x2042
  VUINT16 __c2_h;         // 0xb155
  VUINT32 __RShifter_s;   // 0x4b000000
  VUINT32 __RShifterm5_s; // 0x4affffff
  VUINT32 __InvPi_s;      // 0x3EA2F983
  VUINT32 __nPi1_s;       // 0x40490FDB
  VUINT32 __nPi2_s;       // 0xb3BBBD2E
  VUINT32 __c1_s;         // 0xbe2A026E
  VUINT32 __c2_s;         // 0x3BF9F9B6
  VUINT32 __Half_s;
  VUINT32 __HalfPi_s;
} __devicelib_imf_internal_hcos_data_t;
static const __devicelib_imf_internal_hcos_data_t
    __devicelib_imf_internal_hcos_data = {
        0x7fffu,     //__absmask_h
        0x4bd9u,     //__threshold_h
        0x3e48u,     //__Pio2_h
        0x5200u,     //__RShifter_h
        0x2118u,     //__InvPi_h
        0x2400u,     //__half_sc_h
        0xd648u,     //__nPi1_h
        0xa7edu,     //__nPi2_h
        0x0001u,     //__sgnmask_h
        0x8a2du,     //__c4_h
        0x2042u,     //__c3_h
        0xb155u,     //__c2_h
        0x4b000000u, //__RShifter_s
        0x4affffffu, //__RShifterm5_s
        0x3EA2F983u, //__InvPi_s
        0x40490FDBu, //__nPi1_s
        0xb3BBBD2Eu, //__nPi2_s
        0xbe2A026Eu, //__c1_s
        0x3BF9F9B6u, //__c2_s
        0x3F000000u, //__Half_s
        0x3FC90FDBu, //__HalfPi_s
};
inline _iml_half __devicelib_imf_cosf16_impl(_iml_half x) {
  _iml_half r;
  VUINT32 vm;
  _iml_half va1;
  _iml_half vr1;
  va1 = x;
  {
    _iml_half hArg;
    _iml_half hRes;
    _iml_half hSgn;
    _iml_half hArgAbs;
    _iml_half hAbsMask;
    _iml_half hPio2;
    _iml_half hHalfSC;
    _iml_half hC4;
    _iml_half hC3;
    _iml_half hC2;
    _iml_half hR2;
    _iml_half hPoly;
    _iml_half hR3;
    _iml_half hFastRes;
    _iml_half hX;
    _iml_half hY;
    _iml_half hN;
    _iml_half hR;
    _iml_half hInvPi;
    _iml_half hRShifter;
    _iml_half hFastThreshold;
    _iml_half hNPi1;
    _iml_half hNPi2;
    // S
    float sX;
    float sY;
    float sN;
    float sR;
    float sR2;
    float sPiSgn;
    float sPoly;
    float sR3;
    float sRes;
    float sInvPi;
    float sRShifter;
    float sHalfPi;
    float sHalf;
    float sNPi1;
    float sNPi2;
    float sC2;
    float sC1;
    VUINT16 wArgAbs;
    VUINT16 wFastThreshold;
    // HM
    VUINT16 mLongPath;
    vm = 0;
    // Copy argument
    hArg = va1;
    hAbsMask = as_half(__devicelib_imf_internal_hcos_data.__absmask_h);
    hArgAbs = as_half((as_short(hArg) & as_short(hAbsMask)));
    // |sX| > threshold?
    hFastThreshold = as_half(__devicelib_imf_internal_hcos_data.__threshold_h);
    wArgAbs = as_short(hArgAbs);
    wFastThreshold = as_short(hFastThreshold);
    mLongPath = (wArgAbs > wFastThreshold) ? 0x1L : 0x0L;
    // cos(a) = sin(|a|+pi/2)
    hPio2 = as_half(__devicelib_imf_internal_hcos_data.__Pio2_h);
    hX = (hArgAbs + hPio2);
    hInvPi = as_half(__devicelib_imf_internal_hcos_data.__InvPi_h);
    hRShifter = as_half(__devicelib_imf_internal_hcos_data.__RShifter_h);
    hHalfSC = as_half(__devicelib_imf_internal_hcos_data.__half_sc_h);
    hY = __fma(hX, hInvPi, hRShifter);
    // fN0 = (int)(sX/pi)*(2^(-5))
    hN = (hY - hRShifter);
    // hN = ((int)(sX/pi)-0.5)*(2^(-5))
    hN = (hN - hHalfSC);
    // Argument reduction:  sX + N*(sNPi1+sNPi2), where sNPi1+sNPi2 ~ -pi
    hNPi1 = as_half(__devicelib_imf_internal_hcos_data.__nPi1_h);
    hNPi2 = as_half(__devicelib_imf_internal_hcos_data.__nPi2_h);
    hR = __fma(hN, hNPi1, hArgAbs);
    hR = __fma(hN, hNPi2, hR);
    // sign bit, will treat hY as integer value to look at last bit
    // shift to FP16 sign position
    hSgn = as_half(((VUINT16)as_short(hY) << (15)));
    // hR*hR
    hR2 = (hR * hR);
    // set sign of R
    hR = as_half((as_short(hR) ^ as_short(hSgn)));
    // (hR2*c4 + c3)*hR2 + c2
    hC4 = as_half(__devicelib_imf_internal_hcos_data.__c4_h);
    hC3 = as_half(__devicelib_imf_internal_hcos_data.__c3_h);
    hC2 = as_half(__devicelib_imf_internal_hcos_data.__c2_h);
    hPoly = __fma(hR2, hC4, hC3);
    hPoly = __fma(hPoly, hR2, hC2);
    // hR2*hR
    hR3 = (hR2 * hR);
    // hR + hR3*fPoly
    hFastRes = __fma(hR3, hPoly, hR);
    if (__builtin_expect((mLongPath) == 0, 1)) // short path
    {
      vr1 = hFastRes;
    } else // long path
    {
      // convert to FP32
      sX = ((float)(hArgAbs));
      // sRShifter + sX*(1/pi) will round to sRShifter+N, where N=(int)(sX/pi)
      sInvPi = as_float(__devicelib_imf_internal_hcos_data.__InvPi_s);
      sRShifter = as_float(__devicelib_imf_internal_hcos_data.__RShifter_s);
      sHalfPi = as_float(__devicelib_imf_internal_hcos_data.__HalfPi_s);
      sHalf = as_float(__devicelib_imf_internal_hcos_data.__Half_s);
      sR = (sX + sHalfPi);
      sY = __fma(sR, sInvPi, sRShifter);
      sN = (sY - sRShifter);
      sN = (sN - sHalf);
      // Argument reduction:  sX - N*(sPi1+sPi2)
      sNPi1 = as_float(__devicelib_imf_internal_hcos_data.__nPi1_s);
      sNPi2 = as_float(__devicelib_imf_internal_hcos_data.__nPi2_s);
      sR = __fma(-(sN), sNPi1, sX);
      sR = __fma(-(sN), sNPi2, sR);
      // sign bit, will treat sY as integer value to look at last bit
      sPiSgn = as_float(((VUINT32)as_uint(sY) << (31)));
      // sR*sR
      sR2 = (sR * sR);
      // c2*sR2 + c1
      sC1 = as_float(__devicelib_imf_internal_hcos_data.__c1_s);
      sC2 = as_float(__devicelib_imf_internal_hcos_data.__c2_s);
      sPoly = __fma(sR2, sC2, sC1);
      // sR2*sR
      sR3 = (sR2 * sR);
      // sR + sR3*sPoly
      sPoly = __fma(sR3, sPoly, sR);
      // add sign bit to result (logical XOR between sPoly and i32_sgn bits)
      sRes = as_float((as_uint(sPoly) ^ as_uint(sPiSgn)));
      hRes = ((_iml_half)(sRes));
      // ensure results are always exactly the same for common arguments
      // (return fast path result for common args)
      vr1 = (mLongPath != 0x0) ? (hRes) : (hFastRes);
    } // VELSE
  }
  r = vr1;
  return r;
}
} /* namespace */
} /* namespace __imf_impl_cos_h_la */
DEVICE_EXTERN_C_INLINE _iml_half_internal __devicelib_imf_cosf16(_iml_half_internal x) {
  using namespace __imf_impl_cos_h_la;
  _iml_half _warg1(x);
  _iml_half _wres = __devicelib_imf_cosf16_impl(_warg1);
  return _wres.get_internal();
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
