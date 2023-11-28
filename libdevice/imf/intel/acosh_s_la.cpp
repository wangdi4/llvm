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
//      *  Compute acosh(x) as log(x + sqrt(x*x - 1))
//      *
//      *  Special cases:
//      *
//      *  acosh(NaN)  = quiet NaN, and raise invalid exception
//      *  acosh(-INF) = NaN
//      *  acosh(+INF) = +INF
//      *  acosh(x)    = NaN if x < 1
//      *  acosh(1)    = +0
//      
*/
#include "_imf_include_fp32.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_acosh_s_la {
namespace {
typedef struct {
  VUINT32 Log_tbl_H[32];
  VUINT32 Log_tbl_L[32];
  VUINT32 One;
  VUINT32 AbsMask;
  VUINT32 SmallThreshold;
  VUINT32 Threshold;
  VUINT32 LargeThreshold;
  VUINT32 ca1;
  VUINT32 c2s;
  VUINT32 c1s;
  VUINT32 AddB5;
  VUINT32 RcpBitMask;
  VUINT32 OneEighth;
  VUINT32 Four;
  VUINT32 poly_coeff3;
  VUINT32 poly_coeff2;
  VUINT32 poly_coeff1;
  VUINT32 L2H;
  VUINT32 L2L;
} __devicelib_imf_internal_sacosh_data_avx512_t;
static const __devicelib_imf_internal_sacosh_data_avx512_t
    __devicelib_imf_internal_sacosh_data_avx512 = {
        {/*== Log_tbl_H ==*/
         0x00000000u, 0xbcfc0000u, 0xbd788000u, 0xbdb78000u, 0xbdf14000u,
         0xbe14a000u, 0xbe300000u, 0xbe4aa000u, 0xbe648000u, 0xbe7dc000u,
         0xbe8b4000u, 0xbe974000u, 0xbea31000u, 0xbeae9000u, 0xbeb9d000u,
         0xbec4d000u, 0xbecfa000u, 0xbeda2000u, 0xbee48000u, 0xbeeea000u,
         0xbef89000u, 0xbf012800u, 0xbf05f000u, 0xbf0aa800u, 0xbf0f4000u,
         0xbf13c800u, 0xbf184000u, 0xbf1ca000u, 0xbf20f000u, 0xbf252800u,
         0xbf295000u, 0xbf2d6800u},
        {/*== Log_tbl_L ==*/
         0x80000000u, 0xb726c39eu, 0x3839e7feu, 0xb7528ae5u, 0x377891d5u,
         0xb8297c10u, 0x37cf8f58u, 0x3852b186u, 0x35838656u, 0xb80c36afu,
         0x38235454u, 0xb862bae1u, 0x37e87bc7u, 0x37848150u, 0x37202511u,
         0xb74e1b05u, 0x385c1340u, 0xb8777bcdu, 0x36038656u, 0xb7d40984u,
         0xb80f5fafu, 0xb8254b4cu, 0xb865c84au, 0x37f0b42du, 0xb83ebce1u,
         0xb83c2513u, 0x37a332c4u, 0x3779654fu, 0x38602f73u, 0x367449f8u,
         0xb7b4996fu, 0xb800986bu}
        /*== One ==*/
        ,
        0x3f800000u
        /*== AbsMask ==*/
        ,
        0x7fffffffu
        /*== SmallThreshold ==*/
        ,
        0x39800000u
        /*== Threshold ==*/
        ,
        0x5f000000u
        /*== LargeThreshold ==*/
        ,
        0x7f7fffffu
        /*== ca1 ==*/
        ,
        0xbe2AA5DEu
        /*== c2s ==*/
        ,
        0x3ec00000u
        /*== c1s ==*/
        ,
        0x3f000000u
        /*== AddB5 ==*/
        ,
        0x00020000u
        /*== RcpBitMask ==*/
        ,
        0xfffc0000u
        /*==OneEighth ==*/
        ,
        0x3e000000u
        /*== Four ==*/
        ,
        0x40800000u
        /*== poly_coeff3 ==*/
        ,
        0xbe800810u
        /*== poly_coeff2 ==*/
        ,
        0x3eaab11eu
        /*== poly_coeff1 ==*/
        ,
        0xbf000000u
        /*== L2H = log(2)_high ==*/
        ,
        0x3f317000u
        /*== L2L = log(2)_low ==*/
        ,
        0x3805fdf4u}; /*dLn_Table*/
typedef struct {
  VUINT32 Log_HA_table[(1 << 8) + 2];
  VUINT32 SgnMask;
  VUINT32 XThreshold;
  VUINT32 XhMask;
  VUINT32 ExpMask0;
  VUINT32 ExpMask2;
  VUINT32 ha_poly_coeff[2];
  VUINT32 ExpMask;
  VUINT32 Two10;
  VUINT32 MinLog1p;
  VUINT32 MaxLog1p;
  VUINT32 HalfMask;
  VUINT32 L2H;
  VUINT32 L2L;
  VUINT32 sOne;
  VUINT32 sPoly[8];
  VUINT32 iHiDelta;
  VUINT32 iLoRange;
  VUINT32 iBrkValue;
  VUINT32 iOffExpoMask;
  VUINT32 sBigThreshold;
  VUINT32 sC2;
  VUINT32 sC3;
  VUINT32 sHalf;
  VUINT32 sLargestFinite;
  VUINT32 sLittleThreshold;
  VUINT32 sSign;
  VUINT32 sThirtyOne;
  VUINT32 sTopMask11;
  VUINT32 sTopMask12;
  VUINT32 sTopMask8;
  VUINT32 XScale;
  VUINT32 sLn2;
  /* scalar part follow */
  VUINT32 sInfs[2];
  VUINT32 sOnes[2];
  VUINT32 sZeros[2];
} __devicelib_imf_internal_sacosh_data_t;
static const __devicelib_imf_internal_sacosh_data_t
    __devicelib_imf_internal_sacosh_data = {
        /* Log_HA_table */
        {0xc2aeac38u, 0xb93cbf08u, 0xc2aeb034u, 0xb93ce972u, 0xc2aeb424u,
         0xb95e1069u, 0xc2aeb814u, 0xb9412b26u, 0xc2aebbfcu, 0xb9272b41u,
         0xc2aebfd4u, 0xb950fcd7u, 0xc2aec3acu, 0xb93f86b8u, 0xc2aec77cu,
         0xb933aa90u, 0xc2aecb44u, 0xb92e4507u, 0xc2aecf04u, 0xb9302df1u,
         0xc2aed2bcu, 0xb93a3869u, 0xc2aed66cu, 0xb94d32f7u, 0xc2aeda1cu,
         0xb929e7b5u, 0xc2aeddbcu, 0xb9511c6au, 0xc2aee15cu, 0xb94392acu,
         0xc2aee4f4u, 0xb94207fdu, 0xc2aee884u, 0xb94d35eau, 0xc2aeec14u,
         0xb925d225u, 0xc2aeef94u, 0xb94c8ea1u, 0xc2aef314u, 0xb94219adu,
         0xc2aef68cu, 0xb9471e0bu, 0xc2aef9fcu, 0xb95c430bu, 0xc2aefd6cu,
         0xb9422ca0u, 0xc2af00d4u, 0xb9397b7bu, 0xc2af0434u, 0xb942cd1cu,
         0xc2af0794u, 0xb91ebbeau, 0xc2af0ae4u, 0xb94ddf49u, 0xc2af0e34u,
         0xb950cbabu, 0xc2af1184u, 0xb92812a5u, 0xc2af14c4u, 0xb9544303u,
         0xc2af1804u, 0xb955e8d7u, 0xc2af1b44u, 0xb92d8d8du, 0xc2af1e74u,
         0xb95bb7fau, 0xc2af21acu, 0xb920ec71u, 0xc2af24d4u, 0xb93dacccu,
         0xc2af27fcu, 0xb9327882u, 0xc2af2b1cu, 0xb93fccb3u, 0xc2af2e3cu,
         0xb9262434u, 0xc2af3154u, 0xb925f7a4u, 0xc2af3464u, 0xb93fbd72u,
         0xc2af3774u, 0xb933e9f2u, 0xc2af3a7cu, 0xb942ef61u, 0xc2af3d84u,
         0xb92d3dfbu, 0xc2af4084u, 0xb93343ffu, 0xc2af437cu, 0xb9556dbfu,
         0xc2af4674u, 0xb95425adu, 0xc2af496cu, 0xb92fd461u, 0xc2af4c5cu,
         0xb928e0a9u, 0xc2af4f44u, 0xb93faf8eu, 0xc2af522cu, 0xb934a465u,
         0xc2af550cu, 0xb94820d2u, 0xc2af57ecu, 0xb93a84d8u, 0xc2af5ac4u,
         0xb94c2eddu, 0xc2af5d9cu, 0xb93d7bb5u, 0xc2af606cu, 0xb94ec6aeu,
         0xc2af633cu, 0xb9406992u, 0xc2af6604u, 0xb952bcb6u, 0xc2af68ccu,
         0xb94616feu, 0xc2af6b8cu, 0xb95acde8u, 0xc2af6e4cu, 0xb951358fu,
         0xc2af710cu, 0xb929a0b7u, 0xc2af73c4u, 0xb92460d4u, 0xc2af7674u,
         0xb941c60fu, 0xc2af7924u, 0xb9421f4du, 0xc2af7bd4u, 0xb925ba37u,
         0xc2af7e7cu, 0xb92ce340u, 0xc2af811cu, 0xb957e5adu, 0xc2af83c4u,
         0xb9270b99u, 0xc2af865cu, 0xb95a9dfau, 0xc2af88fcu, 0xb932e4acu,
         0xc2af8b94u, 0xb9302671u, 0xc2af8e24u, 0xb952a8fau, 0xc2af90b4u,
         0xb95ab0eeu, 0xc2af9344u, 0xb94881e8u, 0xc2af95ccu, 0xb95c5e87u,
         0xc2af9854u, 0xb9568869u, 0xc2af9adcu, 0xb9374037u, 0xc2af9d5cu,
         0xb93ec5a6u, 0xc2af9fdcu, 0xb92d577du, 0xc2afa254u, 0xb9433399u,
         0xc2afa4ccu, 0xb94096f3u, 0xc2afa744u, 0xb925bda3u, 0xc2afa9b4u,
         0xb932e2e5u, 0xc2afac24u, 0xb928411du, 0xc2afae8cu, 0xb94611dau,
         0xc2afb0f4u, 0xb94c8ddbu, 0xc2afb35cu, 0xb93bed15u, 0xc2afb5bcu,
         0xb95466b2u, 0xc2afb81cu, 0xb9563119u, 0xc2afba7cu, 0xb94181f0u,
         0xc2afbcd4u, 0xb9568e1eu, 0xc2afbf2cu, 0xb95589d1u, 0xc2afc184u,
         0xb93ea881u, 0xc2afc3d4u, 0xb9521cf3u, 0xc2afc624u, 0xb950193bu,
         0xc2afc874u, 0xb938cec0u, 0xc2afcabcu, 0xb94c6e3fu, 0xc2afcd04u,
         0xb94b27d0u, 0xc2afcf4cu, 0xb9352ae6u, 0xc2afd18cu, 0xb94aa653u,
         0xc2afd3ccu, 0xb94bc84cu, 0xc2afd60cu, 0xb938be68u, 0xc2afd844u,
         0xb951b5a9u, 0xc2afda7cu, 0xb956da79u, 0xc2afdcb4u, 0xb94858aeu,
         0xc2afdeecu, 0xb9265b90u, 0xc2afe11cu, 0xb9310dd5u, 0xc2afe34cu,
         0xb92899abu, 0xc2afe574u, 0xb94d28b2u, 0xc2afe7a4u, 0xb91ee407u,
         0xc2afe9c4u, 0xb95df440u, 0xc2afebecu, 0xb94a8170u, 0xc2afee14u,
         0xb924b32au, 0xc2aff034u, 0xb92cb084u, 0xc2aff254u, 0xb922a015u,
         0xc2aff46cu, 0xb946a7fcu, 0xc2aff684u, 0xb958eddfu, 0xc2aff89cu,
         0xb95996edu, 0xc2affab4u, 0xb948c7e3u, 0xc2affcccu, 0xb926a508u,
         0xc2affedcu, 0xb9335235u, 0xc2b000ecu, 0xb92ef2d4u, 0xc2b002f4u,
         0xb959a9e1u, 0xc2b00504u, 0xb93399eeu, 0xc2b0070cu, 0xb93ce522u,
         0xc2b00914u, 0xb935ad3du, 0xc2b00b14u, 0xb95e1399u, 0xc2b00d1cu,
         0xb936392bu, 0xc2b00f1cu, 0xb93e3e84u}
        /*== SgnMask ==*/
        ,
        0x7fffffffu
        /*== XThreshold ==*/
        ,
        0x39800000u
        /*== XhMask ==*/
        ,
        0xffffff00u
        /*== ExpMask0 ==*/
        ,
        0x7f800000u
        /*== ExpMask2 ==*/
        ,
        0x7b000000u
        /*== ha_poly_coeff[2] ==*/
        ,
        {
            // VHEX_BROADCAST( S, 3fE35103 )    /* coeff3 */
            0x3eAAAB39u /* coeff2 */
            ,
            0xbf000036u /* coeff1 */
        }
        /*== ExpMask ==*/
        ,
        0x007fffffu
        /*== Two10 ==*/
        ,
        0x3b800000u
        /*== MinLog1p ==*/
        ,
        0xbf7fffffu
        /*== MaxLog1p ==*/
        ,
        0x7a800000u
        /*== HalfMask ==*/
        ,
        0xffffff00u
        /*== L2H ==*/
        ,
        0x3f317200u
        /*== L2L ==*/
        ,
        0x35bfbe00u
        /*== sOne = SP 1.0 ==*/
        ,
        0x3f800000u
        /*== sPoly[] = SP polynomial ==*/
        ,
        {
            0xbf000000u /* -5.0000000000000000000000000e-01 P0 */
            ,
            0x3eaaaa94u /*  3.3333265781402587890625000e-01 P1 */
            ,
            0xbe80058eu /* -2.5004237890243530273437500e-01 P2 */
            ,
            0x3e4ce190u /*  2.0007920265197753906250000e-01 P3 */
            ,
            0xbe28ad37u /* -1.6472326219081878662109375e-01 P4 */
            ,
            0x3e0fcb12u /*  1.4042308926582336425781250e-01 P5 */
            ,
            0xbe1ad9e3u /* -1.5122179687023162841796875e-01 P6 */
            ,
            0x3e0d84edu /*  1.3820238411426544189453125e-01 P7 */
        }
        /*== iHiDelta = SP 80000000-7f000000 ==*/
        ,
        0x01000000u
        /*== iLoRange = SP 00800000+iHiDelta ==*/
        ,
        0x01800000u
        /*== iBrkValue = SP 2/3 ==*/
        ,
        0x3f2aaaabu
        /*== iOffExpoMask = SP significand mask ==*/
        ,
        0x007fffffu
        /*== sBigThreshold ==*/
        ,
        0x4E800000u
        /*== sC2 ==*/
        ,
        0x3EC00000u
        /*== sC3 ==*/
        ,
        0x3EA00000u
        /*== sHalf ==*/
        ,
        0x3F000000u
        /*== sLargestFinite ==*/
        ,
        0x7F7FFFFFu
        /*== sLittleThreshold ==*/
        ,
        0x3D800000u
        /*== sSign ==*/
        ,
        0x80000000u
        /*== sThirtyOne ==*/
        ,
        0x41F80000u
        /*== sTopMask11 ==*/
        ,
        0xFFFFE000u
        /*== sTopMask12 ==*/
        ,
        0xFFFFF000u
        /*== sTopMask8 ==*/
        ,
        0xFFFF0000u
        /*== XScale ==*/
        ,
        0x30800000u
        /*== sLn2 = SP ln(2) ==*/
        ,
        0x3f317218u
        /* scalar part follow */
        /*== sInfs = SP infinity, +/- ==*/
        ,
        {0x7f800000u, 0xff800000u}
        /*== sOnes = SP one, +/- ==*/
        ,
        {0x3f800000u, 0xbf800000u}
        /*== sZeros = SP zero +/- ==*/
        ,
        {0x00000000u, 0x80000000u}}; /*sLn_Table*/
static const _iml_sp_union_t __sacosh_la__iml_sacosh_cout_tab[3] = {
    0x3F800000, /* ONE  = 1.0f */
    0x00000000, /* ZERO = 0.0f */
    0x7F800000  /* INF  = 0x7f800000 */
};
// For x = 1 we return +0
// For x = +inf we return +inf
// For NaNs just return a NaN; for others return a NaN and signal invalid.
inline int __devicelib_imf_internal_sacosh(const float *a, float *r) {
  int nRet = 0;
  float purex = *a;
  // First deal with NaN inputs: return a NaN and set necessary flags
  if (((((_iml_sp_union_t *)&purex)->bits.exponent) == 0xFF) &&
      ((((_iml_sp_union_t *)&purex)->bits.significand) != 0x0)) {
    (*r) = purex * purex;
    return nRet;
  }
  // For x = +1, return +0
  if (((_iml_sp_union_t *)&(purex))->hex[0] ==
      ((const _iml_sp_union_t *)&(
           ((const float *)__sacosh_la__iml_sacosh_cout_tab)[0]))
          ->hex[0]) {
    (*r) = (float)((const float *)__sacosh_la__iml_sacosh_cout_tab)[1];
    return nRet;
  }
  // For x = +infinity, return +infinity.
  if (((_iml_sp_union_t *)&(purex))->hex[0] ==
      ((const _iml_sp_union_t *)&(
           ((const float *)__sacosh_la__iml_sacosh_cout_tab)[2]))
          ->hex[0]) {
    (*r) = (float)((const float *)__sacosh_la__iml_sacosh_cout_tab)[2];
    return nRet;
  }
  // Otherwise return NaN and set invalid
  {
    (*r) = (float)(((const float *)__sacosh_la__iml_sacosh_cout_tab)[2] *
                   ((const float *)__sacosh_la__iml_sacosh_cout_tab)[1]);
    nRet = 1;
    return nRet;
  }
}
} /* namespace */
} /* namespace __imf_impl_acosh_s_la */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_acoshf(float a) {
  using namespace __imf_impl_acosh_s_la;
  VUINT32 vm;
  float va1;
  float vr1;
  float r;
  va1 = a;
  ;
  {
    float SgnMask;
    float FpExponPlus;
    float sU;
    float sUHi;
    float sULo;
    float sV;
    float sVHi;
    float sVLo;
    float sVTmp;
    float sTmp1;
    float sTmp2;
    float sTmp3;
    float sTmp4;
    float sTmp5;
    float sTmp6;
    float sTmp7;
    float sTmp8;
    float sY;
    float sW;
    float sZ;
    float sR;
    float sS;
    float sT;
    float sE;
    float sTopMask8;
    float sTopMask12;
    float sC1;
    float sC2;
    float sC3;
    float sPol1;
    float sPol2;
    float sCorr;
    float sTmpf1;
    float sTmpf2;
    float sTmpf3;
    float sTmpf4;
    float sTmpf5;
    float sTmpf6;
    float sH;
    float sL;
    float XScale;
    float sThirtyOne;
    float sInfinityMask;
    float sTooSmallMask;
    float sSpecialMask;
    VUINT32 iSpecialMask;
    float sBigThreshold;
    float sModerateMask;
    float sLargestFinite;
    VUINT32 iBrkValue;
    VUINT32 iOffExpoMask;
    float One;
    VUINT32 iOne;
    float sExp;
    float X;
    float Xl;
    float A;
    float B;
    float Rl;
    float Rh;
    float Rlh;
    float sR2;
    float Kh;
    float sLn2;
    float sPoly[8];
    VUINT32 iX;
    float sN;
    VUINT32 iN;
    VUINT32 iR;
    float sP;
    VUINT32 iExp;
    // Load constants, always including One = 1
    One = as_float(__devicelib_imf_internal_sacosh_data.sOne);
    sLargestFinite =
        as_float(__devicelib_imf_internal_sacosh_data.sLargestFinite);
    sInfinityMask = as_float(((VUINT32)(-(VSINT32)(!(va1 <= sLargestFinite)))));
    sTooSmallMask = as_float(((VUINT32)(-(VSINT32)(!(va1 > One)))));
    sSpecialMask = as_float((as_uint(sInfinityMask) | as_uint(sTooSmallMask)));
    iSpecialMask = as_uint(sSpecialMask);
    vm = 0;
    vm = iSpecialMask;
    // The following computation can go wrong for very large X, e.g.
    // the X^2 - 1 = U * V can overflow. But for large X we have
    // acosh(X) / log(2 X) - 1 =~= 1/(4 * X^2), so for X >= 2^30
    // we can just later stick X back into the log and tweak up the exponent.
    // Actually we scale X by 2^-30 and tweak the exponent up by 31,
    // to stay in the safe range for the later log computation.
    // Compute a flag now telling us when to do this.
    sBigThreshold =
        as_float(__devicelib_imf_internal_sacosh_data.sBigThreshold);
    sModerateMask = as_float(((VUINT32)(-(VSINT32)(va1 < sBigThreshold))));
    // sU is needed later on
    sU = (va1 - One);
    sTmp5 = __fma(va1, va1, -(One));
    // Finally, express Y + W = U * V accurately where Y has <= 8 bits
    sTopMask8 = as_float(__devicelib_imf_internal_sacosh_data.sTopMask8);
    sY = as_float((as_uint(sTmp5) & as_uint(sTopMask8)));
    sW = (sTmp5 - sY);
    // Compute R = 1/sqrt(Y + W) * (1 + d)
    // Force R to <= 8 significant bits.
    // This means that R * Y and R^2 * Y are exactly representable.
    sZ = (1.0f / __sqrt(sY));
    sR = as_float((as_uint(sZ) & as_uint(sTopMask8)));
    // Compute S = (Y/sqrt(Y + W)) * (1 + d)
    //     and T = (W/sqrt(Y + W)) * (1 + d)
    //
    // so that S + T = sqrt(Y + W) * (1 + d)
    // S is exact, and the rounding error in T is OK.
    sS = (sY * sR);
    sT = (sW * sR);
    // Compute e = -(2 * d + d^2)
    // The first FMR is exact, and the rounding error in the other is acceptable
    // since d and e are ~ 2^-8
    sE = __fma(-(sS), sR, One);
    sE = __fma(-(sT), sR, sE);
    // Now       1 / (1 + d)
    //         = 1 / (1 + (sqrt(1 - e) - 1))
    //         = 1 / sqrt(1 - e)
    //         = 1 + 1/2 * e + 3/8 * e^2 + 5/16 * e^3 + 35/128 * e^4 + ...
    //
    // So compute the first three nonconstant terms of that, so that
    // we have a relative correction (1 + Corr) to apply to S etc.
    //
    // C1 = 1/2
    // C2 = 3/8
    // C3 = 5/16
    sC3 = as_float(__devicelib_imf_internal_sacosh_data.sC3);
    sC2 = as_float(__devicelib_imf_internal_sacosh_data.sC2);
    sC2 = as_float(__devicelib_imf_internal_sacosh_data.sC2);
    sPol2 = __fma(sC3, sE, sC2);
    sC1 = as_float(__devicelib_imf_internal_sacosh_data.sHalf);
    sPol1 = __fma(sPol2, sE, sC1);
    sCorr = (sPol1 * sE);
    // For low-accuracy versions, the computation can be done
    // just as U + ((S + T) + (S + T) * Corr)
    sTmpf1 = (sS + sT);
    sTmpf2 = __fma(sTmpf1, sCorr, sTmpf1);
    sH = (sU + sTmpf2);
    // Now we feed into the log1p code, using H in place of _VARG1 and
    // also adding L into Xl.
    // compute 1+x as high, low parts
    A = ((One > sH) ? One : sH);
    B = ((One < sH) ? One : sH);
    X = (A + B);
    Xl = (A - X);
    Xl = (Xl + B);
    // Now multiplex to the case X = 2^-30 * input, Xl = 0 in the "big" case.
    XScale = as_float(__devicelib_imf_internal_sacosh_data.XScale);
    XScale = (va1 * XScale);
    X = as_float((((~as_uint(sModerateMask)) & as_uint(XScale)) |
                  (as_uint(sModerateMask) & as_uint(X))));
    Xl = as_float((as_uint(Xl) & as_uint(sModerateMask)));
    // Now resume the main code.
    iX = as_uint(X);
    /* reduction: compute r,n */
    iBrkValue = (__devicelib_imf_internal_sacosh_data.iBrkValue);
    iOffExpoMask = (__devicelib_imf_internal_sacosh_data.iOffExpoMask);
    iX = (iX - iBrkValue);
    iR = (iX & iOffExpoMask);
    iN = ((VSINT32)iX >> (23));
    iR = (iR + iBrkValue);
    sN = ((float)((VINT32)(iN)));
    sR = as_float(iR);
    iExp = ((VUINT32)(iN) << (23));
    iOne = as_uint(One);
    iExp = (iOne - iExp);
    sExp = as_float(iExp);
    Rl = (Xl * sExp);
    /* polynomial evaluation: */
    Rh = (sR - One);
    sR = (Rh + Rl);
    sPoly[7] = as_float(__devicelib_imf_internal_sacosh_data.sPoly[7]);
    sPoly[6] = as_float(__devicelib_imf_internal_sacosh_data.sPoly[6]);
    // P = C7*R + C6
    sP = __fma(sPoly[7], sR, sPoly[6]);
    sPoly[5] = as_float(__devicelib_imf_internal_sacosh_data.sPoly[5]);
    // P = P*R + C5
    sP = __fma(sP, sR, sPoly[5]);
    sPoly[4] = as_float(__devicelib_imf_internal_sacosh_data.sPoly[4]);
    // P = P*R + C4
    sP = __fma(sP, sR, sPoly[4]);
    sPoly[3] = as_float(__devicelib_imf_internal_sacosh_data.sPoly[3]);
    // P = P*R + C3
    sP = __fma(sP, sR, sPoly[3]);
    sPoly[2] = as_float(__devicelib_imf_internal_sacosh_data.sPoly[2]);
    // P = P*R + C2
    sP = __fma(sP, sR, sPoly[2]);
    sPoly[1] = as_float(__devicelib_imf_internal_sacosh_data.sPoly[1]);
    // P = P*R + C1
    sP = __fma(sP, sR, sPoly[1]);
    sPoly[0] = as_float(__devicelib_imf_internal_sacosh_data.sPoly[0]);
    // P = P*R + C0
    sP = __fma(sP, sR, sPoly[0]);
    // P = P*R
    sP = (sP * sR);
    // P = P*R + R
    sP = __fma(sP, sR, sR);
    // Add 31 to the exponent in the "large" case to get log(2 * input)
    sThirtyOne = as_float(__devicelib_imf_internal_sacosh_data.sThirtyOne);
    FpExponPlus = (sN + sThirtyOne);
    sN = as_float((((~as_uint(sModerateMask)) & as_uint(FpExponPlus)) |
                   (as_uint(sModerateMask) & as_uint(sN))));
    /* final reconstruction */
    sLn2 = as_float(__devicelib_imf_internal_sacosh_data.sLn2);
    // Result = N*log(2) + P
    vr1 = __fma(sN, sLn2, sP);
  }
  if (__builtin_expect((vm) != 0, 0)) {
    float __cout_a1;
    float __cout_r1;
    ((float *)&__cout_a1)[0] = va1;
    ((float *)&__cout_r1)[0] = vr1;
    __devicelib_imf_internal_sacosh(&__cout_a1, &__cout_r1);
    vr1 = ((const float *)&__cout_r1)[0];
  }
  r = vr1;
  ;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
