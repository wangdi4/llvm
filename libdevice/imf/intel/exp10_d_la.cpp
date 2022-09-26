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
//  *
//  *  exp10(x)  = 2^x/log10(2) = 2^n * (1 + T[j]) * (1 + P(y))
//  *  where
//  *       x = m*log10(2)/K + y,  y in [-log10(2)/K..log10(2)/K]
//  *       m = n*K + j,           m,n,j - signed integer, j in [-K/2..K/2]
//  *
//  *       values of 2^j/K are tabulated
//  *
//  *       P(y) is a minimax polynomial approximation of exp10(x)-1
//  *       on small interval [-log10(2)/K..log10(2)/K]
//  *
//  * Special cases:
//  *
//  *  exp10(NaN)  = NaN
//  *  exp10(+INF) = +INF
//  *  exp10(-INF) = 0
//  *  exp10(x)    = 1 for subnormals
//  *  For IEEE double
//  *    if x >  3.39782712893383973096e+02 then exp10(x) overflow
//  *    if x < -3.45133219101941108420e+02 then exp10(x) underflow
//
*/
#include "_imf_include_fp64.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_exp10_d_la {
namespace {
typedef struct {
  VUINT64 Exp_tbl_H[16];
  VUINT64 Exp_tbl_L[16];
  VUINT64 L2E;
  VUINT64 Shifter;
  VUINT64 L2H;
  VUINT64 L2L;
  VUINT64 EMask;
  VUINT64 poly_coeff6;
  VUINT64 poly_coeff5;
  VUINT64 poly_coeff4;
  VUINT64 poly_coeff3;
  VUINT64 poly_coeff2;
  VUINT64 poly_coeff1;
  VUINT64 AbsMask;
  VUINT64 Threshold;
  VUINT64 SmallX;
  VUINT64 IndexMask;
  VUINT64 ExponMask;
} __devicelib_imf_internal_dexp10_data_avx512_t;
static const __devicelib_imf_internal_dexp10_data_avx512_t
    __devicelib_imf_internal_dexp10_data_avx512 = {
        {/*== Exp_tbl_H ==*/
         0x3ff0000000000000uL, 0x3ff0b5586cf9890fuL, 0x3ff172b83c7d517buL,
         0x3ff2387a6e756238uL, 0x3ff306fe0a31b715uL, 0x3ff3dea64c123422uL,
         0x3ff4bfdad5362a27uL, 0x3ff5ab07dd485429uL, 0x3ff6a09e667f3bcduL,
         0x3ff7a11473eb0187uL, 0x3ff8ace5422aa0dbuL, 0x3ff9c49182a3f090uL,
         0x3ffae89f995ad3aduL, 0x3ffc199bdd85529cuL, 0x3ffd5818dcfba487uL,
         0x3ffea4afa2a490dauL},
        {0x0000000000000000uL, 0x3c979aa65d837b6duL, 0xbc801b15eaa59348uL,
         0x3c968efde3a8a894uL, 0x3c834d754db0abb6uL, 0x3c859f48a72a4c6duL,
         0x3c7690cebb7aafb0uL, 0x3c9063e1e21c5409uL, 0xbc93b3efbf5e2228uL,
         0xbc7b32dcb94da51duL, 0x3c8db72fc1f0eab4uL, 0x3c71affc2b91ce27uL,
         0x3c8c1a7792cb3387uL, 0x3c736eae30af0cb3uL, 0x3c74a385a63d07a7uL,
         0xbc8ff7128fd391f0uL}
        /*== log2(e) ==*/
        ,
        0x400A934F0979A371uL
        /*== Shifter=2^(52-4)*1.5 ==*/
        ,
        0x42f8000000003ff0uL
        /*== L2H = log(2)_high ==*/
        ,
        0x3fd34413509f79ffuL
        /*== L2L = log(2)_low ==*/
        ,
        0xbc49dc1da994fd21uL
        /*== EMask ==*/
        ,
        0xbfffffffffffffffuL
        /*== poly_coeff6 ==*/
        ,
        0x3fcb137ed8ac2020uL
        /*== poly_coeff5 ==*/
        ,
        0x3fe141a8e24f9424uL
        /*== poly_coeff4 ==*/
        ,
        0x3ff2bd77a0926c9duL
        /*== poly_coeff3 ==*/
        ,
        0x40004705908704c8uL
        /*== poly_coeff2 ==*/
        ,
        0x40053524c73dfe25uL
        /*== poly_coeff1 ==*/
        ,
        0x40026bb1bbb554c2uL
        /*== AbsMask ==*/
        ,
        0x7fffffffffffffffuL
        /*== Threshold ==*/
        ,
        0x40733A7146F72A41uL
        /*== SmallX=2^(-64) ==*/
        ,
        0x3bf0000000000000uL
        /*== IndexMask ==*/
        ,
        0x0000000000000078uL
        /*== ExponMask ==*/
        ,
        0x0000000000007ff0uL}; /*dExp_Table*/
typedef struct {
  VUINT64 _dbT[(1 << 7)];
  VUINT64 _dbLg2_10;
  VUINT64 _dbShifter;
  VUINT64 _dbInvLg2_10hi;
  VUINT64 _dbInvLg2_10lo;
  VUINT64 _dPC1;
  VUINT64 _dPC2;
  VUINT64 _dPC3;
  VUINT64 _dPC4;
  VUINT64 _dPC5;
  VUINT64 _lExpMask;
  VUINT32 _iIndexMask;
  // common
  VUINT32 _iAbsMask;
  VUINT32 _iDomainRange;
} __devicelib_imf_internal_dexp10_data_t;
static const __devicelib_imf_internal_dexp10_data_t
    __devicelib_imf_internal_dexp10_data = {
        {
            /*== _dbT ==*/
            0x3ff0000000000000uL, /*2^( 0 /128)*/
            0x3ff0163da9fb3335uL, /*2^( 1 /128)*/
            0x3ff02c9a3e778061uL, /*2^( 2 /128)*/
            0x3ff04315e86e7f85uL, /*2^( 3 /128)*/
            0x3ff059b0d3158574uL, /*2^( 4 /128)*/
            0x3ff0706b29ddf6deuL, /*2^( 5 /128)*/
            0x3ff0874518759bc8uL, /*2^( 6 /128)*/
            0x3ff09e3ecac6f383uL, /*2^( 7 /128)*/
            0x3ff0b5586cf9890fuL, /*2^( 8 /128)*/
            0x3ff0cc922b7247f7uL, /*2^( 9 /128)*/
            0x3ff0e3ec32d3d1a2uL, /*2^( 10 /128)*/
            0x3ff0fb66affed31buL, /*2^( 11 /128)*/
            0x3ff11301d0125b51uL, /*2^( 12 /128)*/
            0x3ff12abdc06c31ccuL, /*2^( 13 /128)*/
            0x3ff1429aaea92de0uL, /*2^( 14 /128)*/
            0x3ff15a98c8a58e51uL, /*2^( 15 /128)*/
            0x3ff172b83c7d517buL, /*2^( 16 /128)*/
            0x3ff18af9388c8deauL, /*2^( 17 /128)*/
            0x3ff1a35beb6fcb75uL, /*2^( 18 /128)*/
            0x3ff1bbe084045cd4uL, /*2^( 19 /128)*/
            0x3ff1d4873168b9aauL, /*2^( 20 /128)*/
            0x3ff1ed5022fcd91duL, /*2^( 21 /128)*/
            0x3ff2063b88628cd6uL, /*2^( 22 /128)*/
            0x3ff21f49917ddc96uL, /*2^( 23 /128)*/
            0x3ff2387a6e756238uL, /*2^( 24 /128)*/
            0x3ff251ce4fb2a63fuL, /*2^( 25 /128)*/
            0x3ff26b4565e27cdduL, /*2^( 26 /128)*/
            0x3ff284dfe1f56381uL, /*2^( 27 /128)*/
            0x3ff29e9df51fdee1uL, /*2^( 28 /128)*/
            0x3ff2b87fd0dad990uL, /*2^( 29 /128)*/
            0x3ff2d285a6e4030buL, /*2^( 30 /128)*/
            0x3ff2ecafa93e2f56uL, /*2^( 31 /128)*/
            0x3ff306fe0a31b715uL, /*2^( 32 /128)*/
            0x3ff32170fc4cd831uL, /*2^( 33 /128)*/
            0x3ff33c08b26416ffuL, /*2^( 34 /128)*/
            0x3ff356c55f929ff1uL, /*2^( 35 /128)*/
            0x3ff371a7373aa9cbuL, /*2^( 36 /128)*/
            0x3ff38cae6d05d866uL, /*2^( 37 /128)*/
            0x3ff3a7db34e59ff7uL, /*2^( 38 /128)*/
            0x3ff3c32dc313a8e5uL, /*2^( 39 /128)*/
            0x3ff3dea64c123422uL, /*2^( 40 /128)*/
            0x3ff3fa4504ac801cuL, /*2^( 41 /128)*/
            0x3ff4160a21f72e2auL, /*2^( 42 /128)*/
            0x3ff431f5d950a897uL, /*2^( 43 /128)*/
            0x3ff44e086061892duL, /*2^( 44 /128)*/
            0x3ff46a41ed1d0057uL, /*2^( 45 /128)*/
            0x3ff486a2b5c13cd0uL, /*2^( 46 /128)*/
            0x3ff4a32af0d7d3deuL, /*2^( 47 /128)*/
            0x3ff4bfdad5362a27uL, /*2^( 48 /128)*/
            0x3ff4dcb299fddd0duL, /*2^( 49 /128)*/
            0x3ff4f9b2769d2ca7uL, /*2^( 50 /128)*/
            0x3ff516daa2cf6642uL, /*2^( 51 /128)*/
            0x3ff5342b569d4f82uL, /*2^( 52 /128)*/
            0x3ff551a4ca5d920fuL, /*2^( 53 /128)*/
            0x3ff56f4736b527dauL, /*2^( 54 /128)*/
            0x3ff58d12d497c7fduL, /*2^( 55 /128)*/
            0x3ff5ab07dd485429uL, /*2^( 56 /128)*/
            0x3ff5c9268a5946b7uL, /*2^( 57 /128)*/
            0x3ff5e76f15ad2148uL, /*2^( 58 /128)*/
            0x3ff605e1b976dc09uL, /*2^( 59 /128)*/
            0x3ff6247eb03a5585uL, /*2^( 60 /128)*/
            0x3ff6434634ccc320uL, /*2^( 61 /128)*/
            0x3ff6623882552225uL, /*2^( 62 /128)*/
            0x3ff68155d44ca973uL, /*2^( 63 /128)*/
            0x3ff6a09e667f3bcduL, /*2^( 64 /128)*/
            0x3ff6c012750bdabfuL, /*2^( 65 /128)*/
            0x3ff6dfb23c651a2fuL, /*2^( 66 /128)*/
            0x3ff6ff7df9519484uL, /*2^( 67 /128)*/
            0x3ff71f75e8ec5f74uL, /*2^( 68 /128)*/
            0x3ff73f9a48a58174uL, /*2^( 69 /128)*/
            0x3ff75feb564267c9uL, /*2^( 70 /128)*/
            0x3ff780694fde5d3fuL, /*2^( 71 /128)*/
            0x3ff7a11473eb0187uL, /*2^( 72 /128)*/
            0x3ff7c1ed0130c132uL, /*2^( 73 /128)*/
            0x3ff7e2f336cf4e62uL, /*2^( 74 /128)*/
            0x3ff80427543e1a12uL, /*2^( 75 /128)*/
            0x3ff82589994cce13uL, /*2^( 76 /128)*/
            0x3ff8471a4623c7aduL, /*2^( 77 /128)*/
            0x3ff868d99b4492eduL, /*2^( 78 /128)*/
            0x3ff88ac7d98a6699uL, /*2^( 79 /128)*/
            0x3ff8ace5422aa0dbuL, /*2^( 80 /128)*/
            0x3ff8cf3216b5448cuL, /*2^( 81 /128)*/
            0x3ff8f1ae99157736uL, /*2^( 82 /128)*/
            0x3ff9145b0b91ffc6uL, /*2^( 83 /128)*/
            0x3ff93737b0cdc5e5uL, /*2^( 84 /128)*/
            0x3ff95a44cbc8520fuL, /*2^( 85 /128)*/
            0x3ff97d829fde4e50uL, /*2^( 86 /128)*/
            0x3ff9a0f170ca07bauL, /*2^( 87 /128)*/
            0x3ff9c49182a3f090uL, /*2^( 88 /128)*/
            0x3ff9e86319e32323uL, /*2^( 89 /128)*/
            0x3ffa0c667b5de565uL, /*2^( 90 /128)*/
            0x3ffa309bec4a2d33uL, /*2^( 91 /128)*/
            0x3ffa5503b23e255duL, /*2^( 92 /128)*/
            0x3ffa799e1330b358uL, /*2^( 93 /128)*/
            0x3ffa9e6b5579fdbfuL, /*2^( 94 /128)*/
            0x3ffac36bbfd3f37auL, /*2^( 95 /128)*/
            0x3ffae89f995ad3aduL, /*2^( 96 /128)*/
            0x3ffb0e07298db666uL, /*2^( 97 /128)*/
            0x3ffb33a2b84f15fbuL, /*2^( 98 /128)*/
            0x3ffb59728de5593auL, /*2^( 99 /128)*/
            0x3ffb7f76f2fb5e47uL, /*2^( 100 /128)*/
            0x3ffba5b030a1064auL, /*2^( 101 /128)*/
            0x3ffbcc1e904bc1d2uL, /*2^( 102 /128)*/
            0x3ffbf2c25bd71e09uL, /*2^( 103 /128)*/
            0x3ffc199bdd85529cuL, /*2^( 104 /128)*/
            0x3ffc40ab5fffd07auL, /*2^( 105 /128)*/
            0x3ffc67f12e57d14buL, /*2^( 106 /128)*/
            0x3ffc8f6d9406e7b5uL, /*2^( 107 /128)*/
            0x3ffcb720dcef9069uL, /*2^( 108 /128)*/
            0x3ffcdf0b555dc3fauL, /*2^( 109 /128)*/
            0x3ffd072d4a07897cuL, /*2^( 110 /128)*/
            0x3ffd2f87080d89f2uL, /*2^( 111 /128)*/
            0x3ffd5818dcfba487uL, /*2^( 112 /128)*/
            0x3ffd80e316c98398uL, /*2^( 113 /128)*/
            0x3ffda9e603db3285uL, /*2^( 114 /128)*/
            0x3ffdd321f301b460uL, /*2^( 115 /128)*/
            0x3ffdfc97337b9b5fuL, /*2^( 116 /128)*/
            0x3ffe264614f5a129uL, /*2^( 117 /128)*/
            0x3ffe502ee78b3ff6uL, /*2^( 118 /128)*/
            0x3ffe7a51fbc74c83uL, /*2^( 119 /128)*/
            0x3ffea4afa2a490dauL, /*2^( 120 /128)*/
            0x3ffecf482d8e67f1uL, /*2^( 121 /128)*/
            0x3ffefa1bee615a27uL, /*2^( 122 /128)*/
            0x3fff252b376bba97uL, /*2^( 123 /128)*/
            0x3fff50765b6e4540uL, /*2^( 124 /128)*/
            0x3fff7bfdad9cbe14uL, /*2^( 125 /128)*/
            0x3fffa7c1819e90d8uL, /*2^( 126 /128)*/
            0x3fffd3c22b8f71f1uL  /*2^( 127 /128)*/
        },
        0x407a934f0979a371uL, /* _dbLg2_10*2^K */
        0x4338800000000000uL, /* _dbShifter */
        0x3f63441350a00000uL, /* _dbInvLg2_10hi/2^K 53-11-K bits*/
        0xbd10c0219dc1da99uL, /* _dbInvLg2_10lo/2^K */
        // PC0 = 1.0
        0x40026bb1bbb55516uL, /* _dPC1 */
        0x40053524c73ce8e3uL, /* _dPC2 */
        0x4000470591ccea8buL, /* _dPC3 */
        0x3ff2bd767584db59uL, /* _dPC4 */
        0x3fe144c03efafb54uL, /* _dPC5 */
        0xfff0000000000000uL, /* _lExpMask */
        0x0000007fu,          /* _iIndexMask =(2^K-1)*/
        // common
        0x7fffffffu, /* _iAbsMask */
        0x40733a70u  /* _iDomainRange */
};                   /*dExp10_Table*/
static const _iml_dp_union_t __dexp10_la__imldExp10Tab[147] = {
    0x00000000, 0x3FF00000, /* T[0] = 1                       */
    0x00000000, 0x00000000, /* D[0] = 0                       */
    0x3E778061, 0x3FF02C9A, /* T[1] = 1.010889286051700475255 */
    0x9CD8DC5D, 0xBC716013, /* D[1] = -1.5070669769260387e-17 */
    0xD3158574, 0x3FF059B0, /* T[2] = 1.021897148654116627142 */
    0x3567F613, 0x3C8CD252, /* D[2] = 4.99974487227263278e-17 */
    0x18759BC8, 0x3FF08745, /* T[3] = 1.033024879021228414899 */
    0x61E6C861, 0x3C60F74E, /* D[3] = 7.35784687124741889e-18 */
    0x6CF9890F, 0x3FF0B558, /* T[4] = 1.044273782427413754803 */
    0x5D837B6D, 0x3C979AA6, /* D[4] = 8.18931763819551438e-17 */
    0x32D3D1A2, 0x3FF0E3EC, /* T[5] = 1.055645178360557157049 */
    0x702F9CD1, 0x3C3EBE3D, /* D[5] = 1.66658814423267471e-18 */
    0xD0125B51, 0x3FF11301, /* T[6] = 1.067140400676823697168 */
    0x2A2FBD0E, 0xBC955652, /* D[6] = -7.4028253094261770e-17 */
    0xAEA92DE0, 0x3FF1429A, /* T[7] = 1.078760797757119860307 */
    0xB9D5F416, 0xBC91C923, /* D[7] = -6.1706547456086947e-17 */
    0x3C7D517B, 0x3FF172B8, /* T[8] = 1.090507732665257689675 */
    0xEAA59348, 0xBC801B15, /* D[8] = -2.7939114859515731e-17 */
    0xEB6FCB75, 0x3FF1A35B, /* T[9] = 1.102382583307840890896 */
    0x3F1353BF, 0x3C8B898C, /* D[9] = 4.77695942525622342e-17 */
    0x3168B9AA, 0x3FF1D487, /* T[10] = 1.114386742595892432206 */
    0x3E3A2F60, 0x3C9AECF7, /* D[10] = 9.34171060990504538e-17 */
    0x88628CD6, 0x3FF2063B, /* T[11] = 1.126521618608241848136 */
    0x44A6C38D, 0x3C8A6F41, /* D[11] = 4.58567032666235091e-17 */
    0x6E756238, 0x3FF2387A, /* T[12] = 1.138788634756691564576 */
    0xE3A8A894, 0x3C968EFD, /* D[12] = 7.82657325863607593e-17 */
    0x65E27CDD, 0x3FF26B45, /* T[13] = 1.151189229952982673311 */
    0x981FE7F2, 0x3C80472B, /* D[13] = 2.82378442595106114e-17 */
    0xF51FDEE1, 0x3FF29E9D, /* T[14] = 1.163724858777577475522 */
    0x6D09AB31, 0x3C82F7E1, /* D[14] = 3.29047266460084171e-17 */
    0xA6E4030B, 0x3FF2D285, /* T[15] = 1.176396991650281220743 */
    0x720C0AB4, 0x3C8B3782, /* D[15] = 4.72136812117012784e-17 */
    0x0A31B715, 0x3FF306FE, /* T[16] = 1.189207115002721026897 */
    0x4DB0ABB6, 0x3C834D75, /* D[16] = 3.34846233362515239e-17 */
    0xB26416FF, 0x3FF33C08, /* T[17] = 1.202156731452703075647 */
    0x5DD3F84A, 0x3C8FDD39, /* D[17] = 5.52755004850524931e-17 */
    0x373AA9CB, 0x3FF371A7, /* T[18] = 1.215247359980468955243 */
    0xCC4B5068, 0xBC924AED, /* D[18] = -6.3465521067294830e-17 */
    0x34E59FF7, 0x3FF3A7DB, /* T[19] = 1.228480536106870024682 */
    0x3E9436D2, 0xBC71D1E8, /* D[19] = -1.5456342819397734e-17 */
    0x4C123422, 0x3FF3DEA6, /* T[20] = 1.241857812073484002013 */
    0xA72A4C6D, 0x3C859F48, /* D[20] = 3.75085420130312695e-17 */
    0x21F72E2A, 0x3FF4160A, /* T[21] = 1.255380757024691096291 */
    0x4817895B, 0xBC58A78F, /* D[21] = -5.3460990091987506e-18 */
    0x6061892D, 0x3FF44E08, /* T[22] = 1.269050957191733219886 */
    0x60C2AC11, 0x3C4363ED, /* D[22] = 2.10230496752157159e-18 */
    0xB5C13CD0, 0x3FF486A2, /* T[23] = 1.282870016078778263591 */
    0xDAA10379, 0x3C6ECCE1, /* D[23] = 1.33575100888345409e-17 */
    0xD5362A27, 0x3FF4BFDA, /* T[24] = 1.296839554651009640551 */
    0xBB7AAFB0, 0x3C7690CE, /* D[24] = 1.95725852931120371e-17 */
    0x769D2CA7, 0x3FF4F9B2, /* T[25] = 1.310961211524764413738 */
    0x0071A38E, 0xBC8F9434, /* D[25] = -5.4780691239267781e-17 */
    0x569D4F82, 0x3FF5342B, /* T[26] = 1.325236643159741323217 */
    0xBD0F385F, 0xBC78DEC6, /* D[26] = -2.1571477251208754e-17 */
    0x36B527DA, 0x3FF56F47, /* T[27] = 1.339667524053302916087 */
    0x18FDD78E, 0x3C933505, /* D[27] = 6.66380458923219497e-17 */
    0xDD485429, 0x3FF5AB07, /* T[28] = 1.354255546936892651289 */
    0xE21C5409, 0x3C9063E1, /* D[28] = 5.68648095791174040e-17 */
    0x15AD2148, 0x3FF5E76F, /* T[29] = 1.369002422974590515992 */
    0x2B64C035, 0x3C9432E6, /* D[29] = 7.00787504690699497e-17 */
    0xB03A5585, 0x3FF6247E, /* T[30] = 1.383909881963832022578 */
    0x3BEF4DA8, 0xBC8C33C5, /* D[30] = -4.8923067513522756e-17 */
    0x82552225, 0x3FF66238, /* T[31] = 1.398979672538311236352 */
    0x78565858, 0xBC93CEDD, /* D[31] = -6.8723037209020178e-17 */
    0x667F3BCD, 0x3FF6A09E, /* T[32] = 1.414213562373095145475 */
    0xBF5E2228, 0xBC93B3EF, /* D[32] = -6.8358086576619225e-17 */
    0x3C651A2F, 0x3FF6DFB2, /* T[33] = 1.429613338391970023267 */
    0xB86DA9EE, 0xBC6367EF, /* D[33] = -8.4160116347171566e-18 */
    0xE8EC5F74, 0x3FF71F75, /* T[34] = 1.445180806977046650275 */
    0x7E5A3ECF, 0xBC781F64, /* D[34] = -2.0923043818433529e-17 */
    0x564267C9, 0x3FF75FEB, /* T[35] = 1.460917794180647044655 */
    0x1E55E68A, 0xBC861932, /* D[35] = -3.8334649686542952e-17 */
    0x73EB0187, 0x3FF7A114, /* T[36] = 1.476826145939499346227 */
    0xB94DA51D, 0xBC7B32DC, /* D[36] = -2.3591094770850052e-17 */
    0x36CF4E62, 0x3FF7E2F3, /* T[37] = 1.492907728291264835008 */
    0xABD66C55, 0x3C65EBE1, /* D[37] = 9.50689710108795902e-18 */
    0x994CCE13, 0x3FF82589, /* T[38] = 1.509164427593422841412 */
    0xF13B3734, 0xBC9369B6, /* D[38] = -6.7352192323746824e-17 */
    0x9B4492ED, 0x3FF868D9, /* T[39] = 1.525598150744538417101 */
    0xD872576E, 0xBC94D450, /* D[39] = -7.2266354721012565e-17 */
    0x422AA0DB, 0x3FF8ACE5, /* T[40] = 1.542210825407940744114 */
    0xC1F0EAB4, 0x3C8DB72F, /* D[40] = 5.15483011707867861e-17 */
    0x99157736, 0x3FF8F1AE, /* T[41] = 1.559004400237836929222 */
    0x59F35F44, 0x3C7BF683, /* D[41] = 2.42539857666898064e-17 */
    0xB0CDC5E5, 0x3FF93737, /* T[42] = 1.575980845107886496592 */
    0x8B6C1E29, 0xBC5DA9B8, /* D[42] = -6.4321317754241888e-18 */
    0x9FDE4E50, 0x3FF97D82, /* T[43] = 1.593142151342266998881 */
    0x22F4F9AA, 0xBC924343, /* D[43] = -6.3361618634012930e-17 */
    0x82A3F090, 0x3FF9C491, /* T[44] = 1.610490331949254283472 */
    0x2B91CE27, 0x3C71AFFC, /* D[44] = 1.53414100536037243e-17 */
    0x7B5DE565, 0x3FFA0C66, /* T[45] = 1.628027421857347833978 */
    0x22622263, 0xBC87C504, /* D[45] = -4.1233673306611485e-17 */
    0xB23E255D, 0x3FFA5503, /* T[46] = 1.645755478153964945776 */
    0xD3BCBB15, 0xBC91BBD1, /* D[46] = -6.1526028915502644e-17 */
    0x5579FDBF, 0x3FFA9E6B, /* T[47] = 1.663676580326736376136 */
    0x6E735AB3, 0x3C846984, /* D[47] = 3.54094826264618289e-17 */
    0x995AD3AD, 0x3FFAE89F, /* T[48] = 1.681792830507429004072 */
    0x92CB3387, 0x3C8C1A77, /* D[48] = 4.87516052622706162e-17 */
    0xB84F15FB, 0x3FFB33A2, /* T[49] = 1.700106353718523477525 */
    0x56DCAEBA, 0xBC55C3D9, /* D[49] = -4.7195396645909715e-18 */
    0xF2FB5E47, 0x3FFB7F76, /* T[50] = 1.718619298122477934143 */
    0x38AD9334, 0xBC68D6F4, /* D[50] = -1.0772487078934056e-17 */
    0x904BC1D2, 0x3FFBCC1E, /* T[51] = 1.73733383527370621735  */
    0x0A5FDDCD, 0x3C74FFD7, /* D[51] = 1.82140544036225899e-17 */
    0xDD85529C, 0x3FFC199B, /* T[52] = 1.756252160373299453511 */
    0x30AF0CB3, 0x3C736EAE, /* D[52] = 1.68548729062897320e-17 */
    0x2E57D14B, 0x3FFC67F1, /* T[53] = 1.775376492526521188253 */
    0xD10959AC, 0x3C84E08F, /* D[53] = 3.62161593533689428e-17 */
    0xDCEF9069, 0x3FFCB720, /* T[54] = 1.7947090750031071682   */
    0x6C921968, 0x3C676B2C, /* D[54] = 1.01562190116415002e-17 */
    0x4A07897C, 0x3FFD072D, /* T[55] = 1.814252175500398855945 */
    0x3FFFFA6F, 0xBC8FAD5D, /* D[55] = -5.4951189661220046e-17 */
    0xDCFBA487, 0x3FFD5818, /* T[56] = 1.834008086409342430656 */
    0xA63D07A7, 0x3C74A385, /* D[56] = 1.79012690760451328e-17 */
    0x03DB3285, 0x3FFDA9E6, /* T[57] = 1.853979125083385470774 */
    0xD5C192AC, 0x3C8E5A50, /* D[57] = 5.26537076855627411e-17 */
    0x337B9B5F, 0x3FFDFC97, /* T[58] = 1.874167634110299962558 */
    0x07B43E1F, 0xBC82D521, /* D[58] = -3.2669241009013180e-17 */
    0xE78B3FF6, 0x3FFE502E, /* T[59] = 1.894575981586965607306 */
    0x603A88D3, 0x3C74B604, /* D[59] = 1.79639326598330224e-17 */
    0xA2A490DA, 0x3FFEA4AF, /* T[60] = 1.915206561397147400072 */
    0x8FD391F0, 0xBC8FF712, /* D[60] = -5.5450656186394270e-17 */
    0xEE615A27, 0x3FFEFA1B, /* T[61] = 1.936061793492294347274 */
    0x41AA2008, 0x3C8EC3BC, /* D[61] = 5.33680587851415081e-17 */
    0x5B6E4540, 0x3FFF5076, /* T[62] = 1.957144124175400179411 */
    0x31D185EE, 0x3C8A64A9, /* D[62] = 4.57849152770600937e-17 */
    0x819E90D8, 0x3FFFA7C1, /* T[63] = 1.978456026387950927869 */
    0x4D91CD9D, 0x3C77893B, /* D[63] = 2.04142788975783020e-17 */
    /* TWO_TO_THE_K_DIV_LN2 = 2^6*log2(10) rounded to double */
    0x0979a371, 0x406a934f, /* 2^6*log2(10) */
    /* Right Shifter */
    0x00000000, 0x43380000, /* RS = 2^52 + 2^51 */
    /* Coefficients for exp10(R) - 1 approximation by polynomial p(R) */
    0xbbb55515, 0x40026bb1, /* A1 */
    0xc73cd20a, 0x40053524, /* _A2 */
    0x91e2bc10, 0x40004705, /* _A3 */
    0xb840f0bf, 0x3ff2bd77, /* A4 */
    0x87c70a85, 0x3fe1427c, /* A5 */
    /* Overflow and Underflow Thresholds */
    0x509f79fe, 0x40734413, /* OVF = 308.2547155599166899 */
    0x46f72a41, 0xc0733a71, /* UDF1 = -307.65265556858878160 */
    0x46e36b52, 0xc07439b7, /* UDF2 = -323.306215343115803659*/
    /* Two parts of 1/(log2(10)*64)*/
    0x509f0000, 0x3f734413, /* LOG_HI = .010830424696223 */
    0xc47c4acd, 0x3d1e7fbc, /* LOG_LO = 2.572804622327669e-14 */
    /* TINY and HUGE_VALUE values to process over- underflow */
    0x00000001, 0x00100000, 0xFFFFFFFF, 0x7FEFFFFF,
    /* Double precision constants: 0.0, 1.0, 2.0 */
    0x00000000, 0x00000000, 0x00000000, 0x3FF00000, 0x00000000, 0x40000000,
    /* UNSCALE multiplier for gradual underflow case */
    0x00000000, 0x3C300000, /* 2^(-60) */
    /* TWO_32H = 2^32 + 2^31 */
    0x00000000, 0x41F80000};
inline int __devicelib_imf_internal_dexp10(const double *a, double *r) {
  int nRet = 0;
  double w, Nj;
  double udfResHi, udfResLo;
  double udfScaledResHi, udfScaledResLo, tmp1, tmp2;
  double R, RHi, RLo, p, scale, tmp3, dbIn;
  _iml_uint32_t N, j;
  //_IML_DCOREFN_PROLOG2_IN_C(0, _IML_MODEX87_NEAR53_IN_C, _MODE_UNCHANGED, uf,
  //pA, pRes)
  /* Set all bits of scale to 0.                                           */
  /* Only bits of exponent field will be updated then before each use of   */
  /* scale. Notice, that all other bits (i.e. sign and significand) should */
  /* be kept 0 across iterations. Otherwise, they should be explicitly set */
  /* to 0 before each use of scale                                         */
  scale = ((const double *)__dexp10_la__imldExp10Tab)[142];
  dbIn = (*a);
  /* Filter out INFs and NaNs */
  if (((((_iml_dp_union_t *)&dbIn)->bits.exponent) != 0x7FF)) {
    /* Here if argument is finite double precision number */
    /* Check if dbIn falls into "Near 0" range */
    if ((((_iml_dp_union_t *)&dbIn)->bits.exponent) > (0x3FF - 53)) {
      /* Here if argument is finite double precision number */
      /* and doesn't fall into "Near 0" range               */
      if (dbIn <= ((const double *)__dexp10_la__imldExp10Tab)[135]) {
        /* Here if argument is finite and exp doesn't overflow */
        if (dbIn >= ((const double *)__dexp10_la__imldExp10Tab)[137]) {
          /* Here if argument is finite and exp doesn't    */
          /* oferflow and completely undeflow. But gradual */
          /* underflow is still possible: exp still may    */
          /* produce denormalized result. Thus, here if    */
          /* 6.* paths                                     */
          /* Range Reduction part */
          w = dbIn * ((const double *)__dexp10_la__imldExp10Tab)[128];
          Nj = (w + ((const double *)__dexp10_la__imldExp10Tab)[129]);
          w = (Nj - ((const double *)__dexp10_la__imldExp10Tab)[129]);
          RHi = (w * ((const double *)__dexp10_la__imldExp10Tab)[138]);
          RLo = (w * ((const double *)__dexp10_la__imldExp10Tab)[139]);
          R = (dbIn - RHi);
          R = (R - RLo);
          /* Approximation part: polynomial series */
          p = (((((((const double *)__dexp10_la__imldExp10Tab)[134] * R +
                   ((const double *)__dexp10_la__imldExp10Tab)[133]) *
                      R +
                  ((const double *)__dexp10_la__imldExp10Tab)[132]) *
                     R +
                 ((const double *)__dexp10_la__imldExp10Tab)[131]) *
                    R +
                ((const double *)__dexp10_la__imldExp10Tab)[130]) *
               R);
          /* Final reconstruction starts here */
          /* get N and j from Nj's significand */
          N = (((_iml_dp_union_t *)&Nj)->bits.lo_significand);
          j = N & ((1 << 6) - 1);
          N = N >> 6;
          N = N + 0x3FF;
          /* T[j] * ( D[j] + p) */
          p = (p + ((const double *)__dexp10_la__imldExp10Tab)[2 * j + 1]) *
              ((const double *)__dexp10_la__imldExp10Tab)[2 * j];
          if (dbIn >= ((const double *)__dexp10_la__imldExp10Tab)[136]) {
            /* Here if exp(dbIn) is always finite and */
            /* normalized. Paths 6.2 and 6.3          */
            /* (T[j] + T[j] * ( D[j] + p)) */
            p = (p + ((const double *)__dexp10_la__imldExp10Tab)[2 * j]);
            N = N & 0x7FF;
            /* Check if path 6.3 or 6.2 should follow */
            if (N <= (0x7FF - 1)) {
              /* Path 6.3 */
              /* scale = 2^N */
              (((_iml_dp_union_t *)&scale)->bits.exponent = N);
              /* scale * (T[j] + T[j] * ( D[j] + p)) */
              (*r) = (scale * p);
            } else {
              /* Path 6.2: Case "scale overflow" */
              /* scale = 2^(N - 1) */
              (((_iml_dp_union_t *)&scale)->bits.exponent = N - 1);
              /* 2.0*(scale * (T[j] + T[j] * ( D[j] + p))) */
              p = (p * scale);
              (*r) = (((const double *)__dexp10_la__imldExp10Tab)[144] * p);
            }
          } else //(dbIn < UDF1)
          {
            /* Here if exp gradually underflows */
            N = (N + 60);
            N = N & 0x7FF;
            /* scale = 2^(N + UNSCALE_SHIFT_EXP) */
            (((_iml_dp_union_t *)&scale)->bits.exponent = N);
            /* 2^(N + UNSCALE_SHIFT_EXP) * (T[j]*(D[j] + p) */
            p *= scale;
            /* (2^(N + UNSCALE_SHIFT_EXP) * T[j]) */
            tmp3 = (scale * ((const double *)__dexp10_la__imldExp10Tab)[2 * j]);
            /* Check if path 6.1.1 or 6.1.2 should follow */
            if (N <= (60 - 10)) {
              /* Here if "small" denormalized result */
              /* path 6.1.1 */
              p = (p + tmp3);
              (*r) = (p * ((const double *)__dexp10_la__imldExp10Tab)[145]);
            } else {
              /* Here if "large" denormalized result */
              /* path 6.1.2 */
              /* Get high and low parts of scaled answer */
              udfScaledResHi = (p + tmp3);
              udfScaledResLo = (tmp3 - udfScaledResHi);
              udfScaledResLo += p;
              /* udfResHi is udfScaledResHi rounded to */
              /* 32 most significant bits of mantissa  */
              tmp1 = (((const double *)__dexp10_la__imldExp10Tab)[146] *
                      udfScaledResHi);
              tmp2 = (udfScaledResHi + tmp1);
              udfResHi = (tmp2 - tmp1);
              /* udfResLo = (udfScaledResHi - udfResHi) + */
              /*            + udfScaledResLo              */
              udfResLo = (udfScaledResHi - udfResHi);
              udfResLo += udfScaledResLo;
              /* Return (*r) = (UNSCALE * udfResHi) +  */
              /*               + (UNSCALE * udfResLo)  */
              udfResHi =
                  (udfResHi * ((const double *)__dexp10_la__imldExp10Tab)[145]);
              udfResLo =
                  (udfResLo * ((const double *)__dexp10_la__imldExp10Tab)[145]);
              (*r) = (udfResHi + udfResLo);
            }
            nRet = 4;
          }
        } else {
          /* Here if argument is finite but exp completely  */
          /* underflows in round to nearest mode and should */
          /* be rounded 0.0                                 */
          (*r) = ((const double *)__dexp10_la__imldExp10Tab)[140] *
                 ((const double *)__dexp10_la__imldExp10Tab)[140];
          nRet = 4;
        }
      } else {
        /* Here if exp overflows */
        (*r) = ((const double *)__dexp10_la__imldExp10Tab)[141] *
               ((const double *)__dexp10_la__imldExp10Tab)[141];
        nRet = 3;
      }
    } else {
      /* "Near 0" path */
      (*r) = ((const double *)__dexp10_la__imldExp10Tab)[143] + dbIn;
    }
  } else {
    /* Here if argument is NaN or Infinity */
    if (((((_iml_dp_union_t *)&dbIn)->bits.sign) == 1) &&
        (((((_iml_dp_union_t *)&dbIn)->bits.hi_significand) == 0) &&
         ((((_iml_dp_union_t *)&dbIn)->bits.lo_significand) == 0))) {
      /* Here if argument is negative infinity */
      (*r) = ((const double *)__dexp10_la__imldExp10Tab)[142];
    } else {
      /* Here if argument is positive infinity or NaN */
      (*r) = dbIn * dbIn;
    }
  }
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_exp10_d_la */
DEVICE_EXTERN_C_INLINE double __devicelib_imf_exp10(double x) {
  using namespace __imf_impl_exp10_d_la;
  double r;
  VUINT32 vm;
  double va1;
  double vr1;
  va1 = x;
  {
    double dN;
    double dM;
    double dR;
    VUINT64 lM;
    VUINT64 lX;
    VUINT32 iAbsX;
    VUINT32 iRangeMask;
    VUINT32 iIndex;
    double dTj;
    double dbLg2_10;
    double dbShifter;
    double dbInvLg2_10hi;
    double dbInvLg2_10lo;
    double dPC[5];
    VUINT64 lExpMask;
    VUINT32 iIndexMask;
    VUINT32 iAbsMask;
    VUINT32 iDomainRange;
    dbLg2_10 = as_double(__devicelib_imf_internal_dexp10_data._dbLg2_10);
    dbInvLg2_10hi =
        as_double(__devicelib_imf_internal_dexp10_data._dbInvLg2_10hi);
    dbInvLg2_10lo =
        as_double(__devicelib_imf_internal_dexp10_data._dbInvLg2_10lo);
    dbShifter = as_double(__devicelib_imf_internal_dexp10_data._dbShifter);
    lExpMask = (__devicelib_imf_internal_dexp10_data._lExpMask);
    iIndexMask = (__devicelib_imf_internal_dexp10_data._iIndexMask);
    dPC[0] = as_double(__devicelib_imf_internal_dexp10_data._dPC1);
    dPC[1] = as_double(__devicelib_imf_internal_dexp10_data._dPC2);
    dPC[2] = as_double(__devicelib_imf_internal_dexp10_data._dPC3);
    dPC[3] = as_double(__devicelib_imf_internal_dexp10_data._dPC4);
    dPC[4] = as_double(__devicelib_imf_internal_dexp10_data._dPC5);
    iAbsMask = (__devicelib_imf_internal_dexp10_data._iAbsMask);
    iDomainRange = (__devicelib_imf_internal_dexp10_data._iDomainRange);
    /* ............... Load arument ............................ */
    dM = __fma(va1, dbLg2_10, dbShifter);
    dN = (dM - dbShifter);
    /* ...............Check for overflow\underflow ............. */
    lX = as_ulong(va1);
    iAbsX = ((VUINT32)((VUINT64)lX >> 32));
    iAbsX = (iAbsX & iAbsMask);
    iRangeMask =
        ((VUINT32)(-(VSINT32)((VSINT32)iAbsX > (VSINT32)iDomainRange)));
    vm = 0;
    vm = iRangeMask;
    /* .............. Index and lookup ......................... */
    lM = as_ulong(dM);
    iIndex = (((VUINT32)lM & (VUINT32)-1));
    iIndex = (iIndex & iIndexMask);
    iIndex = ((VUINT32)(iIndex) << (3)); // iIndex*=sizeof(D);
    dTj = as_double(((const VUINT64 *)(__devicelib_imf_internal_dexp10_data
                                           ._dbT))[iIndex >> 3]);
    /* ............... 2^N ..................................... */
    lM = ((VUINT64)(lM) << ((52 - 7)));
    lM = (lM & lExpMask); // lM==EXP(2^N)
    /* ................... R ................................... */
    dR = __fma(-(dN), dbInvLg2_10hi, va1);
    dR = __fma(-(dN), dbInvLg2_10lo, dR);
    /* ................... Polynomial .......................... */
    // poly(dN) = a1*dR+...+a5*dR^5
    dN = __fma(dPC[4], dR, dPC[3]);
    dN = __fma(dN, dR, dPC[2]);
    dN = __fma(dN, dR, dPC[1]);
    dN = __fma(dN, dR, dPC[0]); // a1+...+a5*dR^4 !
    dN = (dN * dR);                              // a1*dR+...+a5*dR^5
    dN = __fma(dN, dTj, dTj);   // Tj*poly
    /* ..................quick 2^N............................... */
    lX = as_ulong(dN);
    lX = (lX + lM);
    /* ................... Finish  ............ */
    vr1 = as_double(lX);
  }
  if (__builtin_expect((vm) != 0, 0)) {
    double __cout_a1;
    double __cout_r1;
    ((double *)&__cout_a1)[0] = va1;
    ((double *)&__cout_r1)[0] = vr1;
    __devicelib_imf_internal_dexp10(&__cout_a1, &__cout_r1);
    vr1 = ((const double *)&__cout_r1)[0];
  }
  r = vr1;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
