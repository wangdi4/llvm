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
//      *
//      *   log(x) = exponent_x*log(2) + log(mantissa_x),         if mantissa_x<4/3
//      *   log(x) = (exponent_x+1)*log(2) + log(0.5*mantissa_x), if mantissa_x>4/3
//      *
//      *    R = mantissa_x - 1,     if mantissa_x<4/3
//      *    R = 0.5*mantissa_x - 1, if mantissa_x>4/3
//      *    |R|< 1/3
//      *
//      *    log(1+R) is approximated as a polynomial: degree 9 for 1-ulp, degree 7
//      * for 4-ulp, degree 3 for half-precision
//      
*/
#include "_imf_include_fp32.hpp"
#if defined(INTEL_COLLAB) && defined(OMP_LIBDEVICE)
#pragma omp declare target
#endif
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_ln_s_ha {
namespace {
typedef struct {
  VUINT32 AddBit;
  VUINT32 RcpMask;
  VUINT32 Log_tbl_H0[32];
  VUINT32 Log_tbl_L0[32];
  VUINT32 Log_tbl_H[32];
  VUINT32 Log_tbl_L[32];
  VUINT32 One;
  VUINT32 poly_coeff4;
  VUINT32 poly_coeff3;
  VUINT32 poly_coeff2;
  VUINT32 L2H;
  VUINT32 L2L;
  VUINT32 MinNorm;
  VUINT32 MaxNorm;
  VUINT32 IndexMask;
  VUINT32 AddBit_lrb2;
  VUINT32 RcpMask_lrb2;
  VUINT32 Log_lrb2_H0[16];
  VUINT32 Log_lrb2_L0[16];
  VUINT32 One_lrb2;
  VUINT32 L2H_lrb2;
  VUINT32 L2L_lrb2;
  VUINT32 poly_lrb2_coeff4;
  VUINT32 poly_lrb2_coeff3;
  VUINT32 poly_lrb2_coeff2;
  VUINT32 poly_lrb2_coeff1;
} __devicelib_imf_internal_sln_data_avx512_t;
static const __devicelib_imf_internal_sln_data_avx512_t
    __devicelib_imf_internal_sln_data_avx512 = {
        /*== AddBit ==*/
        0x00020000u
        /*== RcpMask ==*/
        ,
        0xfffc0000u
        // used in main path
        ,
        {
            0x00000000u, 0xbcfc0000u, 0xbd786000u, 0xbdb78000u, 0xbdf14000u,
            0xbe14a800u, 0xbe2ff800u, 0xbe4a9000u, 0xbe648000u, 0xbe7dc800u,
            0xbe8b3c00u, 0xbe974800u, 0xbea30c00u, 0xbeae8c00u, 0xbeb9d000u,
            0xbec4d000u, 0xbecf9800u, 0xbeda2800u, 0xbee48000u, 0xbeeea400u,
            0xbef89400u, 0xbf012a00u, 0xbf05f400u, 0xbf0aa600u, 0xbf0f4200u,
            0xbf13ca00u, 0xbf183e00u, 0xbf1ca000u, 0xbf20ec00u, 0xbf252800u,
            0xbf295200u, 0xbf2d6a00u,
        },
        {
            0x00000000u, 0xb726c39eu, 0x37679ff7u, 0xb7528ae5u, 0x377891d5u,
            0xb725f040u, 0xb6c1c29eu, 0xb73539e9u, 0x35838656u, 0xb6436af2u,
            0x370d5151u, 0x36ea28f7u, 0xb63c21c6u, 0xb776fd60u, 0x37202511u,
            0xb74e1b05u, 0xb70fb2feu, 0x36084337u, 0x36038656u, 0x36afd9f2u,
            0xb675faf0u, 0xb7152d2fu, 0x36d1bdadu, 0xb5f4bd35u, 0xb77af382u,
            0xb770944eu, 0xb7399a79u, 0x3779654fu, 0xb6fe8467u, 0x367449f8u,
            0x3716cd22u, 0xb4186b3eu,
        }
        // used in long path
        ,
        {
            0x3f317200u, 0x3f299200u, 0x3f21ec00u, 0x3f1a8200u, 0x3f134c00u,
            0x3f0c4800u, 0x3f057400u, 0x3efd9c00u, 0x3ef0a400u, 0x3ee40000u,
            0x3ed7a800u, 0x3ecb9c00u, 0x3ebfd800u, 0x3eb45800u, 0x3ea91400u,
            0x3e9e1400u, 0x3e934c00u, 0x3e88bc00u, 0x3e7cc800u, 0x3e688000u,
            0x3e54a000u, 0x3e412000u, 0x3e2df800u, 0x3e1b3000u, 0x3e08c000u,
            0x3ded4000u, 0x3dc9a000u, 0x3da6a000u, 0x3d843000u, 0x3d44a000u,
            0x3d020000u, 0x3c810000u,
        },
        {
            0x35bfbe8eu, 0xb70ecbccu, 0x377f97c9u, 0xb73a9314u, 0xb76f7659u,
            0xb70df86eu, 0xb691d2fbu, 0xb71d4217u, 0x3621a272u, 0xb5c71755u,
            0x37254923u, 0x370d0c4du, 0xb5b884fdu, 0xb75f058eu, 0x37381ce3u,
            0xb7362333u, 0xb6ef7659u, 0x3668227eu, 0x3663659eu, 0x36dfc995u,
            0xb6161ba9u, 0xb6fa6ab9u, 0x3700d6a8u, 0xb4d3fa9cu, 0xb762fbb0u,
            0xb7589c7cu, 0xb721a2a7u, 0xb76ea2e0u, 0xb6ce94c4u, 0x36aa14a0u,
            0x372ec4f4u, 0x35acb127u,
        }
        /*== One ==*/
        ,
        0x3f800000u
        /*== poly_coeff4 ==*/
        ,
        0xbe800810u
        /*== poly_coeff3 ==*/
        ,
        0x3eAAB11Eu
        /*== poly_coeff2 ==*/
        ,
        0xbf000000u
        /*== L2H = log(2)_high ==*/
        ,
        0x3f317200u
        /*== L2L = log(2)_low ==*/
        ,
        0x35bfbe8eu
        /*== MinNorm ==*/
        ,
        0x00800000u
        /*== MaxNorm ==*/
        ,
        0x7f7fffffu
        /*== IndexMask ==*/
        ,
        0x0000007cu
        ///////////////////////////////////////////
        /*== AddBit_lrb2 ==*/
        ,
        0x00040000u
        /*== RcpMask ==*/
        ,
        0xfff80000u,
        {
            0x00000000u,
            0xbd786000u,
            0xbdf14000u,
            0xbe2ff800u,
            0xbe648000u,
            0xbe8b3c00u,
            0xbea30c00u,
            0xbeb9d000u,
            0xbecf9800u,
            0xbee48000u,
            0xbef89400u,
            0xbf05f400u,
            0xbf0f4200u,
            0xbf183e00u,
            0xbf20ec00u,
            0xbf295200u,
        },
        {
            0x00000000u,
            0x37679ff7u,
            0x377891d5u,
            0xb6c1c29eu,
            0x35838656u,
            0x370d5151u,
            0xb63c21c6u,
            0x37202511u,
            0xb70fb2feu,
            0x36038656u,
            0xb675faf0u,
            0x36d1bdadu,
            0xb77af382u,
            0xb7399a79u,
            0xb6fe8467u,
            0x3716cd22u,
        }
        /*== One_lrb2 ==*/
        ,
        0x3f800000u
        /*== L2H = log(2)_high ==*/
        ,
        0x3f317200u
        /*== L2L = log(2)_low ==*/
        ,
        0x35bfbe8eu
        /*== poly_lrb2_coeff4 ==*/
        ,
        0x3e4D0CDEu
        /*== poly_lrb2_coeff3 ==*/
        ,
        0xbe801AB0u
        /*== poly_lrb2_coeff2 ==*/
        ,
        0x3eAAAAA7u
        /*== poly_lrb2_coeff1 ==*/
        ,
        0xbeFFFFFEu}; /*dLn_Table*/
typedef struct {
  VUINT32 sPoly[9];
  VUINT32 iHiDelta;
  VUINT32 iLoRange;
  VUINT32 iBrkValue;
  VUINT32 iOffExpoMask;
  VUINT32 sOne;
  VUINT32 sLn2Hi;
  VUINT32 sLn2Lo;
  /* scalar part follow */
  VUINT32 sInfs[2];
  VUINT32 sOnes[2];
  VUINT32 sZeros[2];
} __devicelib_imf_internal_sln_data_t;
static const __devicelib_imf_internal_sln_data_t
    __devicelib_imf_internal_sln_data = {
        {
            /* > Polynomial sPoly[] coefficients:  */
            0xbf000000u, /* -5.0000000000000000000000000e-01 */
            0x3eaaaa83u, /*  3.3333215117454528808593750e-01 */
            0xbe7fff78u, /* -2.4999797344207763671875000e-01 */
            0x3e4ce814u, /*  2.0010405778884887695312500e-01 */
            0xbe2acee6u, /* -1.6680487990379333496093750e-01 */
            0x3e0f6b8cu, /*  1.4005869626998901367187500e-01 */
            0xbdf9889eu, /* -1.2184260785579681396484375e-01 */
            0x3e0f335du, /*  1.3984437286853790283203125e-01 */
            0xbe0402c8u, /* -1.2891685962677001953125000e-01 */
        },
        /* Constant for work range check: Delta 80000000-7f800000 */
        0x00800000u,
        /* Constant for work range check: 00800000 + Delta */
        0x01000000u,
        /* Mantissa break point  SP 2/3 */
        0x3f2aaaabu,
        /* SP significand mask */
        0x007fffffu,
        /* 1.0f */
        0x3f800000u,
        /* SP log(2) in high+low parts */
        0x3f317200u,
        0x35bfbe8eu,
        /* SP infinity, +/- */
        {0x7f800000u, 0xff800000u},
        /* SP one, +/- */
        {0x3f800000u, 0xbf800000u},
        /* SP zero +/- */
        {0x00000000u, 0x80000000u}}; /*sLn_Table*/
/* Scale exponent for denormalized arguments */
/* Number of bits in j */
/* Mask to extract j */
/* Size of the table look-up */
/* Name of the table look-up */
/* Single precision pointer to the beginning of the table look-up */
/* Macros to access other precomputed constants */
/* Table look-up: all SP constants are presented in hexadecimal form */
static const _iml_sp_union_t __sln_ha_CoutTab[210] = {
    0x3F800000, /* RCPR_Y[0]       =  1.0000000000e+00 */
    0x00000000, /* LN_RCPR_Y_HI[0] =  0.0000000000e-01 */
    0x00000000, /* LN_RCPR_Y_LO[0] =  0.0000000000e-01 */
    0x3F7C0000, /* RCPR_Y[1]       =  9.8437500000e-01 */
    0x3C810000, /* LN_RCPR_Y_HI[1] =  1.5747070313e-02 */
    0x35ACB127, /* LN_RCPR_Y_LO[1] =  1.2866556744e-06 */
    0x3F780000, /* RCPR_Y[2]       =  9.6875000000e-01 */
    0x3D020000, /* LN_RCPR_Y_HI[2] =  3.1738281250e-02 */
    0x372EC4F4, /* LN_RCPR_Y_LO[2] =  1.0417064914e-05 */
    0x3F740000, /* RCPR_Y[3]       =  9.5312500000e-01 */
    0x3D44C000, /* LN_RCPR_Y_HI[3] =  4.8034667969e-02 */
    0xB7D57AD8, /* LN_RCPR_Y_LO[3] = -2.5448782253e-05 */
    0x3F700000, /* RCPR_Y[4]       =  9.3750000000e-01 */
    0x3D843000, /* LN_RCPR_Y_HI[4] =  6.4544677734e-02 */
    0xB6CE94C4, /* LN_RCPR_Y_LO[4] = -6.1565970100e-06 */
    0x3F6E0000, /* RCPR_Y[5]       =  9.2968750000e-01 */
    0x3D955000, /* LN_RCPR_Y_HI[5] =  7.2906494141e-02 */
    0x349488E3, /* LN_RCPR_Y_LO[5] =  2.7666746405e-07 */
    0x3F6A0000, /* RCPR_Y[6]       =  9.1406250000e-01 */
    0x3DB80000, /* LN_RCPR_Y_HI[6] =  8.9843750000e-02 */
    0x37530AEB, /* LN_RCPR_Y_LO[6] =  1.2579122085e-05 */
    0x3F660000, /* RCPR_Y[7]       =  8.9843750000e-01 */
    0x3DDB5000, /* LN_RCPR_Y_HI[7] =  1.0708618164e-01 */
    0x37488DAD, /* LN_RCPR_Y_LO[7] =  1.1953915418e-05 */
    0x3F640000, /* RCPR_Y[8]       =  8.9062500000e-01 */
    0x3DED4000, /* LN_RCPR_Y_HI[8] =  1.1584472656e-01 */
    0xB7589C7C, /* LN_RCPR_Y_LO[8] = -1.2911037629e-05 */
    0x3F600000, /* RCPR_Y[9]       =  8.7500000000e-01 */
    0x3E08BC00, /* LN_RCPR_Y_HI[9] =  1.3352966309e-01 */
    0x35E8227E, /* LN_RCPR_Y_LO[9] =  1.7295385533e-06 */
    0x3F5E0000, /* RCPR_Y[10]       =  8.6718750000e-01 */
    0x3E11EC00, /* LN_RCPR_Y_HI[10] =  1.4250183105e-01 */
    0xB5ED5B64, /* LN_RCPR_Y_LO[10] = -1.7684474187e-06 */
    0x3F5A0000, /* RCPR_Y[11]       =  8.5156250000e-01 */
    0x3E248800, /* LN_RCPR_Y_HI[11] =  1.6067504883e-01 */
    0x36F60CCF, /* LN_RCPR_Y_LO[11] =  7.3328624239e-06 */
    0x3F580000, /* RCPR_Y[12]       =  8.4375000000e-01 */
    0x3E2DFC00, /* LN_RCPR_Y_HI[12] =  1.6990661621e-01 */
    0xB6FE52AF, /* LN_RCPR_Y_LO[12] = -7.5794155237e-06 */
    0x3F540000, /* RCPR_Y[13]       =  8.2812500000e-01 */
    0x3E412000, /* LN_RCPR_Y_HI[13] =  1.8859863281e-01 */
    0xB6FA6AB9, /* LN_RCPR_Y_LO[13] = -7.4630047493e-06 */
    0x3F520000, /* RCPR_Y[14]       =  8.2031250000e-01 */
    0x3E4AD400, /* LN_RCPR_Y_HI[14] =  1.9807434082e-01 */
    0xB6948C24, /* LN_RCPR_Y_LO[14] = -4.4270582293e-06 */
    0x3F500000, /* RCPR_Y[15]       =  8.1250000000e-01 */
    0x3E54A000, /* LN_RCPR_Y_HI[15] =  2.0764160156e-01 */
    0xB6161BA9, /* LN_RCPR_Y_LO[15] = -2.2367842121e-06 */
    0x3F4C0000, /* RCPR_Y[16]       =  7.9687500000e-01 */
    0x3E688000, /* LN_RCPR_Y_HI[16] =  2.2705078125e-01 */
    0x36DFC995, /* LN_RCPR_Y_LO[16] =  6.6693851295e-06 */
    0x3F4A0000, /* RCPR_Y[17]       =  7.8906250000e-01 */
    0x3E729800, /* LN_RCPR_Y_HI[17] =  2.3690795898e-01 */
    0x35EFFE71, /* LN_RCPR_Y_LO[17] =  1.7880939822e-06 */
    0x3F480000, /* RCPR_Y[18]       =  7.8125000000e-01 */
    0x3E7CC800, /* LN_RCPR_Y_HI[18] =  2.4685668945e-01 */
    0x3663659E, /* LN_RCPR_Y_LO[18] =  3.3884784898e-06 */
    0x3F460000, /* RCPR_Y[19]       =  7.7343750000e-01 */
    0x3E838A00, /* LN_RCPR_Y_HI[19] =  2.5691223145e-01 */
    0xB5F3F655, /* LN_RCPR_Y_LO[19] = -1.8176602907e-06 */
    0x3F440000, /* RCPR_Y[20]       =  7.6562500000e-01 */
    0x3E88BC00, /* LN_RCPR_Y_HI[20] =  2.6705932617e-01 */
    0x3668227E, /* LN_RCPR_Y_LO[20] =  3.4590771065e-06 */
    0x3F400000, /* RCPR_Y[21]       =  7.5000000000e-01 */
    0x3E934B00, /* LN_RCPR_Y_HI[21] =  2.8768157959e-01 */
    0x35044D37, /* LN_RCPR_Y_LO[21] =  4.9286194326e-07 */
    0x3F3E0000, /* RCPR_Y[22]       =  7.4218750000e-01 */
    0x3E98A800, /* LN_RCPR_Y_HI[22] =  2.9815673828e-01 */
    0xB661E2CA, /* LN_RCPR_Y_LO[22] = -3.3659621295e-06 */
    0x3F3C0000, /* RCPR_Y[23]       =  7.3437500000e-01 */
    0x3E9E1300, /* LN_RCPR_Y_HI[23] =  3.0873870850e-01 */
    0xB6588CCD, /* LN_RCPR_Y_LO[23] = -3.2268465020e-06 */
    0x3F3A0000, /* RCPR_Y[24]       =  7.2656250000e-01 */
    0x3EA38C00, /* LN_RCPR_Y_HI[24] =  3.1942749023e-01 */
    0x365C271C, /* LN_RCPR_Y_LO[24] =  3.2805319279e-06 */
    0x3F380000, /* RCPR_Y[25]       =  7.1875000000e-01 */
    0x3EA91500, /* LN_RCPR_Y_HI[25] =  3.3023834229e-01 */
    0x3660738A, /* LN_RCPR_Y_LO[25] =  3.3445853660e-06 */
    0x3F360000, /* RCPR_Y[26]       =  7.1093750000e-01 */
    0x3EAEAE00, /* LN_RCPR_Y_HI[26] =  3.4117126465e-01 */
    0xB50829A8, /* LN_RCPR_Y_LO[26] = -5.0724565881e-07 */
    0x3F340000, /* RCPR_Y[27]       =  7.0312500000e-01 */
    0x3EB45600, /* LN_RCPR_Y_HI[27] =  3.5221862793e-01 */
    0x3603E9C7, /* LN_RCPR_Y_LO[27] =  1.9656597487e-06 */
    0x3F320000, /* RCPR_Y[28]       =  6.9531250000e-01 */
    0x3EBA0F00, /* LN_RCPR_Y_HI[28] =  3.6339569092e-01 */
    0xB5F12731, /* LN_RCPR_Y_LO[28] = -1.7967305439e-06 */
    0x3F300000, /* RCPR_Y[29]       =  6.8750000000e-01 */
    0x3EBFD800, /* LN_RCPR_Y_HI[29] =  3.7469482422e-01 */
    0xB5B884FD, /* LN_RCPR_Y_LO[29] = -1.3747772982e-06 */
    0x3F2E0000, /* RCPR_Y[30]       =  6.7968750000e-01 */
    0x3EC5B200, /* LN_RCPR_Y_HI[30] =  3.8612365723e-01 */
    0xB5CAEE9A, /* LN_RCPR_Y_LO[30] = -1.5119615000e-06 */
    0x3F2C0000, /* RCPR_Y[31]       =  6.7187500000e-01 */
    0x3ECB9D00, /* LN_RCPR_Y_HI[31] =  3.9768218994e-01 */
    0x3550C4D6, /* LN_RCPR_Y_LO[31] =  7.7772472196e-07 */
    0x3F2A0000, /* RCPR_Y[32]       =  6.6406250000e-01 */
    0x3ED19A00, /* LN_RCPR_Y_HI[32] =  4.0937805176e-01 */
    0x3580449F, /* LN_RCPR_Y_LO[32] =  9.5567145308e-07 */
    0x3F280000, /* RCPR_Y[33]       =  6.5625000000e-01 */
    0x3ED7A900, /* LN_RCPR_Y_HI[33] =  4.2121124268e-01 */
    0x3615248D, /* LN_RCPR_Y_LO[33] =  2.2224005534e-06 */
    0x3F280000, /* RCPR_Y[34]       =  6.5625000000e-01 */
    0x3ED7A900, /* LN_RCPR_Y_HI[34] =  4.2121124268e-01 */
    0x3615248D, /* LN_RCPR_Y_LO[34] =  2.2224005534e-06 */
    0x3F260000, /* RCPR_Y[35]       =  6.4843750000e-01 */
    0x3EDDCB00, /* LN_RCPR_Y_HI[35] =  4.3318939209e-01 */
    0x348DC071, /* LN_RCPR_Y_LO[35] =  2.6403316156e-07 */
    0x3F240000, /* RCPR_Y[36]       =  6.4062500000e-01 */
    0x3EE40000, /* LN_RCPR_Y_HI[36] =  4.4531250000e-01 */
    0xB5C71755, /* LN_RCPR_Y_LO[36] = -1.4833445903e-06 */
    0x3F220000, /* RCPR_Y[37]       =  6.3281250000e-01 */
    0x3EEA4800, /* LN_RCPR_Y_HI[37] =  4.5758056641e-01 */
    0x3511B7BF, /* LN_RCPR_Y_LO[37] =  5.4284095086e-07 */
    0x3F200000, /* RCPR_Y[38]       =  6.2500000000e-01 */
    0x3EF0A400, /* LN_RCPR_Y_HI[38] =  4.7000122070e-01 */
    0x3621A272, /* LN_RCPR_Y_LO[38] =  2.4085425139e-06 */
    0x3F200000, /* RCPR_Y[39]       =  6.2500000000e-01 */
    0x3EF0A400, /* LN_RCPR_Y_HI[39] =  4.7000122070e-01 */
    0x3621A272, /* LN_RCPR_Y_LO[39] =  2.4085425139e-06 */
    0x3F1E0000, /* RCPR_Y[40]       =  6.1718750000e-01 */
    0x3EF71500, /* LN_RCPR_Y_HI[40] =  4.8258209229e-01 */
    0x34AB5A0A, /* LN_RCPR_Y_LO[40] =  3.1916744092e-07 */
    0x3F1C0000, /* RCPR_Y[41]       =  6.0937500000e-01 */
    0x3EFD9B00, /* LN_RCPR_Y_HI[41] =  4.9532318115e-01 */
    0xB5EA10B7, /* LN_RCPR_Y_LO[41] = -1.7439223257e-06 */
    0x3F1A0000, /* RCPR_Y[42]       =  6.0156250000e-01 */
    0x3F021B00, /* LN_RCPR_Y_HI[42] =  5.0822448730e-01 */
    0x34BE7604, /* LN_RCPR_Y_LO[42] =  3.5476125504e-07 */
    0x3F1A0000, /* RCPR_Y[43]       =  6.0156250000e-01 */
    0x3F021B00, /* LN_RCPR_Y_HI[43] =  5.0822448730e-01 */
    0x34BE7604, /* LN_RCPR_Y_LO[43] =  3.5476125504e-07 */
    0x3F180000, /* RCPR_Y[44]       =  5.9375000000e-01 */
    0x3F0573C0, /* LN_RCPR_Y_HI[44] =  5.2129745483e-01 */
    0xB50E97D6, /* LN_RCPR_Y_LO[44] = -5.3120072607e-07 */
    0x3F160000, /* RCPR_Y[45]       =  5.8593750000e-01 */
    0x3F08D7C0, /* LN_RCPR_Y_HI[45] =  5.3454208374e-01 */
    0x338F1D6B, /* LN_RCPR_Y_LO[45] =  6.6643075058e-08 */
    0x3F140000, /* RCPR_Y[46]       =  5.7812500000e-01 */
    0x3F0C4780, /* LN_RCPR_Y_HI[46] =  5.4796600342e-01 */
    0xB55F86E2, /* LN_RCPR_Y_LO[46] = -8.3270253981e-07 */
    0x3F140000, /* RCPR_Y[47]       =  5.7812500000e-01 */
    0x3F0C4780, /* LN_RCPR_Y_HI[47] =  5.4796600342e-01 */
    0xB55F86E2, /* LN_RCPR_Y_LO[47] = -8.3270253981e-07 */
    0x3F120000, /* RCPR_Y[48]       =  5.7031250000e-01 */
    0x3F0FC300, /* LN_RCPR_Y_HI[48] =  5.6156921387e-01 */
    0x35D7F186, /* LN_RCPR_Y_LO[48] =  1.6089040855e-06 */
    0x3F100000, /* RCPR_Y[49]       =  5.6250000000e-01 */
    0x3F134B00, /* LN_RCPR_Y_HI[49] =  5.7536315918e-01 */
    0x35844D37, /* LN_RCPR_Y_LO[49] =  9.8572388652e-07 */
    0x3F100000, /* RCPR_Y[50]       =  5.6250000000e-01 */
    0x3F134B00, /* LN_RCPR_Y_HI[50] =  5.7536315918e-01 */
    0x35844D37, /* LN_RCPR_Y_LO[50] =  9.8572388652e-07 */
    0x3F0E0000, /* RCPR_Y[51]       =  5.5468750000e-01 */
    0x3F16DFC0, /* LN_RCPR_Y_HI[51] =  5.8935165405e-01 */
    0xB5AA13C8, /* LN_RCPR_Y_LO[51] = -1.2671744116e-06 */
    0x3F0E0000, /* RCPR_Y[52]       =  5.5468750000e-01 */
    0x3F16DFC0, /* LN_RCPR_Y_HI[52] =  5.8935165405e-01 */
    0xB5AA13C8, /* LN_RCPR_Y_LO[52] = -1.2671744116e-06 */
    0x3F0C0000, /* RCPR_Y[53]       =  5.4687500000e-01 */
    0x3F1A8140, /* LN_RCPR_Y_HI[53] =  6.0353469849e-01 */
    0x34AD9D8D, /* LN_RCPR_Y_LO[53] =  3.2338394362e-07 */
    0x3F0A0000, /* RCPR_Y[54]       =  5.3906250000e-01 */
    0x3F1E3040, /* LN_RCPR_Y_HI[54] =  6.1792373657e-01 */
    0x32C36BFB, /* LN_RCPR_Y_LO[54] =  2.2750091588e-08 */
    0x3F0A0000, /* RCPR_Y[55]       =  5.3906250000e-01 */
    0x3F1E3040, /* LN_RCPR_Y_HI[55] =  6.1792373657e-01 */
    0x32C36BFB, /* LN_RCPR_Y_LO[55] =  2.2750091588e-08 */
    0x3F080000, /* RCPR_Y[56]       =  5.3125000000e-01 */
    0x3F21ED00, /* LN_RCPR_Y_HI[56] =  6.3252258301e-01 */
    0xB2D06DC4, /* LN_RCPR_Y_LO[56] = -2.4264302567e-08 */
    0x3F080000, /* RCPR_Y[57]       =  5.3125000000e-01 */
    0x3F21ED00, /* LN_RCPR_Y_HI[57] =  6.3252258301e-01 */
    0xB2D06DC4, /* LN_RCPR_Y_LO[57] = -2.4264302567e-08 */
    0x3F060000, /* RCPR_Y[58]       =  5.2343750000e-01 */
    0x3F25B800, /* LN_RCPR_Y_HI[58] =  6.4733886719e-01 */
    0xB5A41A3D, /* LN_RCPR_Y_LO[58] = -1.2226588524e-06 */
    0x3F060000, /* RCPR_Y[59]       =  5.2343750000e-01 */
    0x3F25B800, /* LN_RCPR_Y_HI[59] =  6.4733886719e-01 */
    0xB5A41A3D, /* LN_RCPR_Y_LO[59] = -1.2226588524e-06 */
    0x3F040000, /* RCPR_Y[60]       =  5.1562500000e-01 */
    0x3F299180, /* LN_RCPR_Y_HI[60] =  6.6237640381e-01 */
    0xB56CBCC4, /* LN_RCPR_Y_LO[60] = -8.8191541181e-07 */
    0x3F040000, /* RCPR_Y[61]       =  5.1562500000e-01 */
    0x3F299180, /* LN_RCPR_Y_HI[61] =  6.6237640381e-01 */
    0xB56CBCC4, /* LN_RCPR_Y_LO[61] = -8.8191541181e-07 */
    0x3F020000, /* RCPR_Y[62]       =  5.0781250000e-01 */
    0x3F2D7A00, /* LN_RCPR_Y_HI[62] =  6.7764282227e-01 */
    0x34386C94, /* LN_RCPR_Y_LO[62] =  1.7175835865e-07 */
    0x3F020000, /* RCPR_Y[63]       =  5.0781250000e-01 */
    0x3F2D7A00, /* LN_RCPR_Y_HI[63] =  6.7764282227e-01 */
    0x34386C94, /* LN_RCPR_Y_LO[63] =  1.7175835865e-07 */
    0x3F000000, /* RCPR_Y[64]       =  5.0000000000e-01 */
    0x3F317200, /* LN_RCPR_Y_HI[64] =  6.9314575195e-01 */
    0x35BFBE8E, /* LN_RCPR_Y_LO[64] =  1.4286067653e-06 */
    /* Two parts of the ln(2.0) */
    0x3F317200, /* LN2_HI = 6.931457519531250e-01 */
    0x35BFBE8E, /* LN2_LO = 1.428606765330187e-06 */
    /* Right Shifter to obtain j */
    0x48000040, /* RSJ = 2^(17)+1 */
    /* Right Shifter to obtain YHi */
    0x46000000, /* RSY = 2^(7) */
    /* "Near 1" path (bound for u=1-x) */
    0x3C200000, /* NEAR0_BOUND */
    /* Scale for denormalized arguments */
    0x4D000000, /* DENORM_SCALE = 2^27 */
    /* Single precision constants: 0.0, 1.0 */
    0x00000000, 0x3F800000,
/*
//      Coefficients for (ln(1+u) - u)/u^2 approximation by
//        polynomial p0(u) 
*/
    0xBF000000, /* A0 = -.500000000000000 */
    0x3EAAAAAB, /* A1 =  .333333333333344 */
    0xBE800000, /* A2 = -.250000000000034 */
    0x3E4CCCCD, /* A3 =  .199999999115651 */
    0xBE2AAAAB, /* A4 = -.166666665188498 */
    0x3E124E01, /* A5 =  .142875688385215 */
    0xBE0005A0, /* A6 = -.125021460296036 */
};
inline int __devicelib_imf_internal_sln(const float *a, float *r) {
  float fap1, *ap1 = &fap1;
  float x, y, u;
  float fP;
  float fAbsU;
  float fN, fNLn2Hi, fNLn2Lo;
  float fRcprY, fLnRcprYHi, fLnRcprYLo, fWHi, fWLo;
  float fYHi, fYLo, fUHi, fULo, fResHi, fResLo;
  float fTmp;
  int iN, j;
  int i;
  int nRet = 0;
  int isDenorm = 0;
  /* Filter out Infs and NaNs */
  if (((((_iml_sp_union_t *)&a[0])->bits.exponent) != 0xFF)) {
    /* Here if argument is finite float precision number */
/*
//      Copy argument into temporary variable x,
//        and initially set iN equal to 0 
*/
    x = a[0];
    iN = 0;
    /* Check if x is denormalized number or [+/-]0 */
    if ((((_iml_sp_union_t *)&x)->bits.exponent) == 0) {
      //  Line below not needed; it breaks denormal accuracy
      //  isDenorm = 1;
      /* Here if argument is denormalized or [+/-]0 */
      /* Scale x and properly adjust iN */
      x = (x * ((const float *)__sln_ha_CoutTab)[200]);
      iN -= (27);
    }
    /* Starting from this point x is finite normalized number */
    if (x > ((const float *)__sln_ha_CoutTab)[201]) {
      /* Here if x is positive finite normalized number */
      /* Get absolute value of u=x-1 */
      u = x - 1.0f;
      fAbsU = u;
      (((_iml_sp_union_t *)&fAbsU)->bits.sign = 0);
      /* Check if a[0] falls into "Near 1" range */
      if (fAbsU > ((const float *)__sln_ha_CoutTab)[199]) {
        /* 6) "Main" path */
        /* a) Range reduction */
        /* Get N taking into account denormalized arguments */
        iN += (((_iml_sp_union_t *)&x)->bits.exponent) - 0x7F;
        fN = (float)iN;
/*
//          Compute N*Ln2Hi and N*Ln2Lo. Notice that N*Ln2Hi
//            is error-free for any N 
*/
        if (isDenorm == 1) {
          fNLn2Hi = (fN * (((const float *)__sln_ha_CoutTab)[195] +
                           ((const float *)__sln_ha_CoutTab)[196]));
          fNLn2Lo = 0.0f;
        } else {
          fNLn2Hi = (fN * ((const float *)__sln_ha_CoutTab)[195]);
          fNLn2Lo = (fN * ((const float *)__sln_ha_CoutTab)[196]);
        }
        /* Get y */
        y = x;
        (((_iml_sp_union_t *)&y)->bits.exponent = 0x7F);
        /* Obtain j */
        fTmp = (y + ((const float *)__sln_ha_CoutTab)[197]);
        j = (((_iml_sp_union_t *)&fTmp)->bits.significand) &
            ((1 << (6 + 1)) - 1);
        /* Get table values of RcprY, LnRcprYHi, LnRcprYLo */
        fRcprY = ((const float *)__sln_ha_CoutTab)[3 * (j)];
        fLnRcprYHi = ((const float *)__sln_ha_CoutTab)[3 * (j) + 1];
        fLnRcprYLo = ((const float *)__sln_ha_CoutTab)[3 * (j) + 2];
        /* Calculate WHi and WLo */
        fWHi = (fNLn2Hi + fLnRcprYHi);
        fWLo = (fNLn2Lo + fLnRcprYLo);
        /* Get YHi and YLo */
        fTmp = (y + ((const float *)__sln_ha_CoutTab)[198]);
        fYHi = (fTmp - ((const float *)__sln_ha_CoutTab)[198]);
        fYLo = (y - fYHi);
        /* Get UHi, ULo and U */
        fUHi = (fRcprY * fYHi - 1.0f);
        fULo = (fRcprY * fYLo);
        u = (fUHi + fULo);
        // printf("u = %g\n", u);
        /* b) Approximation */
        fP = ((((((((const float *)__sln_ha_CoutTab)[209] * u +
                   ((const float *)__sln_ha_CoutTab)[208]) *
                      u +
                  ((const float *)__sln_ha_CoutTab)[207]) *
                     u +
                 ((const float *)__sln_ha_CoutTab)[206]) *
                    u +
                ((const float *)__sln_ha_CoutTab)[205]) *
                   u +
               ((const float *)__sln_ha_CoutTab)[204]) *
                  u +
              ((const float *)__sln_ha_CoutTab)[203]);
        fP = (fP * u * u);
        /* c) Reconstruction */
        fResHi = (fWHi + fUHi);
        fResLo = ((fWLo + fULo) + fP);
        r[0] = (fResHi + fResLo);
      } else {
        /* 5) "Near 1" path (|u|<=NEAR0_BOUND) */
        fP = ((((((((const float *)__sln_ha_CoutTab)[209] * u +
                   ((const float *)__sln_ha_CoutTab)[208]) *
                      u +
                  ((const float *)__sln_ha_CoutTab)[207]) *
                     u +
                 ((const float *)__sln_ha_CoutTab)[206]) *
                    u +
                ((const float *)__sln_ha_CoutTab)[205]) *
                   u +
               ((const float *)__sln_ha_CoutTab)[204]) *
                  u +
              ((const float *)__sln_ha_CoutTab)[203]);
        fP = (fP * u * u);
        fP = (fP + u);
        r[0] = fP;
      }
    } else {
      /* Path 3) or 4). Here if argument is negative number or +/-0 */
      if (x == ((const float *)__sln_ha_CoutTab)[201]) {
        /* Path 3). Here if argument is +/-0 */
        r[0] = -((const float *)__sln_ha_CoutTab)[202] /
               ((const float *)__sln_ha_CoutTab)[201];
        nRet = 2;
      } else {
        /* Path 4). Here if argument is negative number */
        r[0] = ((const float *)__sln_ha_CoutTab)[201] /
               ((const float *)__sln_ha_CoutTab)[201];
        nRet = 1;
      }
    }
  } else {
    /* Path 1) or 2). Here if argument is NaN or +/-Infinity */
    if (((((_iml_sp_union_t *)&a[0])->bits.sign) == 1) &&
        ((((_iml_sp_union_t *)&a[0])->bits.significand) == 0)) {
      /* Path 2). Here if argument is -Infinity */
      r[0] = ((const float *)__sln_ha_CoutTab)[201] /
             ((const float *)__sln_ha_CoutTab)[201];
      nRet = 1;
    } else {
      /* Path 1). Here if argument is NaN or +Infinity */
      r[0] = (a[0] * a[0]);
    }
  }
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_ln_s_ha */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_logf(float a) {
  using namespace __imf_impl_ln_s_ha;
  VUINT32 vm;
  float va1;
  float vr1;
  float r;
  va1 = a;
  ;
  {
    VUINT32 iHiDelta;
    VUINT32 iLoRange;
    VUINT32 iBrkValue;
    VUINT32 iOffExpoMask;
    float sOne;
    float sLn2Hi;
    float sLn2Lo;
    float sPoly[9];
    VUINT32 iRangeMask;
    VUINT32 iX;
    VUINT32 iXTest;
    float sN;
    VUINT32 iN;
    float sR;
    VUINT32 iR;
    float sP;
    iHiDelta = (__devicelib_imf_internal_sln_data.iHiDelta);
    iLoRange = (__devicelib_imf_internal_sln_data.iLoRange);
    iX = as_uint(va1);
    /* check for working range, */
    /* set special argument mask (denormals/zero/Inf/NaN) */
    iXTest = (iX + iHiDelta);
    iRangeMask = ((VUINT32)(-(VSINT32)((VSINT32)iXTest < (VSINT32)iLoRange)));
    iBrkValue = (__devicelib_imf_internal_sln_data.iBrkValue);
    iOffExpoMask = (__devicelib_imf_internal_sln_data.iOffExpoMask);
    /* reduction: compute r,n */
    iX = (iX - iBrkValue);
    iR = (iX & iOffExpoMask);
    /* exponent_x (mantissa_x<4/3) or exponent_x+1 (mantissa_x>4/3) */
    iN = ((VSINT32)iX >> (23));
    /* mantissa_x (mantissa_x<4/3), or 0.5*mantissa_x (mantissa_x>4/3) */
    iR = (iR + iBrkValue);
    sN = ((float)((VINT32)(iN)));
    sR = as_float(iR);
    vm = 0;
    vm = iRangeMask;
    sOne = as_float(__devicelib_imf_internal_sln_data.sOne);
    /* reduced argument R */
    sR = (sR - sOne);
    sPoly[8] = as_float(__devicelib_imf_internal_sln_data.sPoly[8]);
    sPoly[7] = as_float(__devicelib_imf_internal_sln_data.sPoly[7]);
    /* polynomial evaluation starts here */
    sP = __fma(sPoly[8], sR, sPoly[7]);
    sPoly[6] = as_float(__devicelib_imf_internal_sln_data.sPoly[6]);
    sP = __fma(sP, sR, sPoly[6]);
    sPoly[5] = as_float(__devicelib_imf_internal_sln_data.sPoly[5]);
    sP = __fma(sP, sR, sPoly[5]);
    sPoly[4] = as_float(__devicelib_imf_internal_sln_data.sPoly[4]);
    sP = __fma(sP, sR, sPoly[4]);
    sPoly[3] = as_float(__devicelib_imf_internal_sln_data.sPoly[3]);
    sP = __fma(sP, sR, sPoly[3]);
    sPoly[2] = as_float(__devicelib_imf_internal_sln_data.sPoly[2]);
    sP = __fma(sP, sR, sPoly[2]);
    sPoly[1] = as_float(__devicelib_imf_internal_sln_data.sPoly[1]);
    sP = __fma(sP, sR, sPoly[1]);
    sPoly[0] = as_float(__devicelib_imf_internal_sln_data.sPoly[0]);
    sP = __fma(sP, sR, sPoly[0]);
    sP = (sP * sR);
    /* polynomial evaluation end */
    sP = __fma(sP, sR, sR);
    sLn2Hi = as_float(__devicelib_imf_internal_sln_data.sLn2Hi);
    sLn2Lo = as_float(__devicelib_imf_internal_sln_data.sLn2Lo);
    /* final reconstruction: */
    /* add exponent_value*(log2_hi + log2_lo) to polynomial result */
    vr1 = __fma(sN, sLn2Lo, sP);
    vr1 = __fma(sN, sLn2Hi, vr1);
  }
  if (__builtin_expect((vm) != 0, 0)) {
    float __cout_a1;
    float __cout_r1;
    ((float *)&__cout_a1)[0] = va1;
    ((float *)&__cout_r1)[0] = vr1;
    __devicelib_imf_internal_sln(&__cout_a1, &__cout_r1);
    vr1 = ((const float *)&__cout_r1)[0];
  }
  r = vr1;
  ;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
#if defined(INTEL_COLLAB) && defined(OMP_LIBDEVICE)
DEVICE_EXTERN_C_DECLSIMD_INLINE
float __svml_device_logf(float x) { return __devicelib_imf_logf(x); }
#pragma omp end declare target
#endif
