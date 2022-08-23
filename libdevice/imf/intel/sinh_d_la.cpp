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
//  *  Compute sinh(x) as (exp(x)-exp(-x))/2,
//  *  where exp is calculated as
//  *  exp(M*ln2 + ln2*(j/2^k) + r) = 2^M * 2^(j/2^k) * exp(r)
//  *
//  *  Special cases:
//  *
//  *  sinh(NaN) = quiet NaN, and raise invalid exception
//  *  sinh(INF) = that INF
//  *  sinh(x)   = x for subnormals
//  *  sinh(x) overflows for big x and returns MAXLOG+log(2)
//
*/
#include "_imf_include_fp64.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_sinh_d_la {
namespace {
typedef struct {
  /* Shared */
  VUINT64 _dExp_tbl_PH[16];
  VUINT64 _dExp_tbl_PL[16];
  VUINT64 _dExp_tbl_NH[16];
  VUINT64 _dExp_tbl_NL[16];
  VUINT64 _dbShifter_UISA;
  VUINT64 _dbShifterP1_UISA;
  VUINT32 _iDomainRange_UISA;
  VUINT64 _lIndexMask_UISA; //(1<<K)1-
  VUINT64 _dPC2_UISA;
  VUINT64 _dPC3_UISA;
  VUINT64 _dPC4_UISA;
  VUINT64 _dPC5_UISA;
  VUINT64 _dPC6_UISA;
  VUINT64 _dPC7_UISA;
  VUINT64 _dPC8_UISA;
  VUINT64 _dbInvLn2;
  VUINT64 _dbLn2hi;
  VUINT64 _dbLn2lo;
  VUINT64 _dSign; // 0x8000000000000000
  VUINT64 _dHalf;
  VUINT64 _dZero;
  /* Shared name but with different value */
  VUINT64 _dbT[(1 << 7)][2]; // precalc poly coeff
  VUINT64 _dbShifter;
  VUINT32 _iDomainRange;
  VUINT64 _dPC1; //=1
  VUINT64 _dPC2;
  VUINT64 _dPC3;
  VUINT64 _dPC4;
  VUINT64 _dPC5;
  VUINT64 _lExpMask;
  VUINT64 _lBias;
  VUINT64 _lIndexMask;
} __devicelib_imf_internal_dsinh_data_t;
//
static const __devicelib_imf_internal_dsinh_data_t
    __devicelib_imf_internal_dsinh_data = {
        {/*== _dExp_tbl_PH ==*/
         0x3fe0000000000000uL, 0x3fe0b5586cf9890fuL, 0x3fe172b83c7d517buL,
         0x3fe2387a6e756238uL, 0x3fe306fe0a31b715uL, 0x3fe3dea64c123422uL,
         0x3fe4bfdad5362a27uL, 0x3fe5ab07dd485429uL, 0x3fe6a09e667f3bcduL,
         0x3fe7a11473eb0187uL, 0x3fe8ace5422aa0dbuL, 0x3fe9c49182a3f090uL,
         0x3feae89f995ad3aduL, 0x3fec199bdd85529cuL, 0x3fed5818dcfba487uL,
         0x3feea4afa2a490dauL}, /* dTp_h */
        {0x0000000000000000uL, 0x3c88a62e4adc610buL, 0xbc719041b9d78a76uL,
         0x3c89b07eb6c70573uL, 0x3c76f46ad23182e4uL, 0x3c7ada0911f09ebcuL,
         0x3c6d4397afec42e2uL, 0x3c86324c054647aduL, 0xbc8bdd3413b26456uL,
         0xbc741577ee04992fuL, 0x3c86e9f156864b27uL, 0x3c6c7c46b071f2beuL,
         0x3c87a1cd345dcc81uL, 0x3c711065895048dduL, 0x3c72ed02d75b3707uL,
         0xbc8e9c23179c2893uL}, /* dTp_l */
        {0x3fe0000000000000uL, 0x3fdea4afa2a490dauL, 0x3fdd5818dcfba487uL,
         0x3fdc199bdd85529cuL, 0x3fdae89f995ad3aduL, 0x3fd9c49182a3f090uL,
         0x3fd8ace5422aa0dbuL, 0x3fd7a11473eb0187uL, 0x3fd6a09e667f3bcduL,
         0x3fd5ab07dd485429uL, 0x3fd4bfdad5362a27uL, 0x3fd3dea64c123422uL,
         0x3fd306fe0a31b715uL, 0x3fd2387a6e756238uL, 0x3fd172b83c7d517buL,
         0x3fd0b5586cf9890fuL}, /* dTn_h */
        {0x0000000000000000uL, 0xbc7e9c23179c2893uL, 0x3c62ed02d75b3707uL,
         0x3c611065895048dduL, 0x3c77a1cd345dcc81uL, 0x3c5c7c46b071f2beuL,
         0x3c76e9f156864b27uL, 0xbc641577ee04992fuL, 0xbc7bdd3413b26456uL,
         0x3c76324c054647aduL, 0x3c5d4397afec42e2uL, 0x3c6ada0911f09ebcuL,
         0x3c66f46ad23182e4uL, 0x3c79b07eb6c70573uL, 0xbc619041b9d78a76uL,
         0x3c78a62e4adc610buL}, /* dTn_l */
        0x42F8000000000000uL,   /* _dbShifter_UISA    */
        0x42F8000000000001uL,   /* _dbShifterP1_UISA  */
        0x4084ee33u,            /* _iDomainRange_UISA */
        0x000000000000000FuL,   /* _lIndexMask_UISA   */
        0x3fe0000000000000uL,   /* _dPC2_UISA         */
        0x3fc5555555555ca6uL,   /* _dPC3_UISA         */
        0x3fa5555555554b4cuL,   /* _dPC4_UISA         */
        0x3f8111110edc6edauL,   /* _dPC5_UISA         */
        0x3f56c16c1902580buL,   /* _dPC6_UISA         */
        0x3f2a026d06015ec6uL,   /* _dPC7_UISA         */
        0x3efa01ac1b806259uL,   /* _dPC8_UISA         */
        0x3FF71547652B82FEuL,   /* _dbInvLn2 = 1/log(2) */
        0x3FE62E42FEFA0000uL,   /* _dbLn2hi  = log(2) hi*/
        0x3D7CF79ABC9E3B3AuL,   /* _dbLn2lo  = log(2) lo*/
        0x8000000000000000uL,   /* _dSign */
        0x3FE0000000000000uL,   /* _dHalf */
        0x0000000000000000uL,   /* _dZero */
        {
            //_dbT
            {0x0000000000000000uL,
             0x3FE0000000000000uL}, // 2^( 0 /128-1) - 2^(- 0 /128-1), 2^(- 0
                                    // /128-1)
            {0x3F762E4A19BD1E74uL,
             0x3FDFD3C22B8F71F1uL}, // 2^( 1 /128-1) - 2^(- 1 /128-1), 2^(- 1
                                    // /128-1)
            {0x3F862E5F6A0DFD36uL,
             0x3FDFA7C1819E90D8uL}, // 2^( 2 /128-1) - 2^(- 2 /128-1), 2^(- 2
                                    // /128-1)
            {0x3F90A2E234040F5FuL,
             0x3FDF7BFDAD9CBE14uL}, // 2^( 3 /128-1) - 2^(- 3 /128-1), 2^(- 3
                                    // /128-1)
            {0x3F962EB4ABCC5A81uL,
             0x3FDF50765B6E4540uL}, // 2^( 4 /128-1) - 2^(- 4 /128-1), 2^(- 4
                                    // /128-1)
            {0x3F9BBAB1C5033244uL,
             0x3FDF252B376BBA97uL}, // 2^( 5 /128-1) - 2^(- 5 /128-1), 2^(- 5
                                    // /128-1)
            {0x3FA0A372144EEB45uL,
             0x3FDEFA1BEE615A27uL}, // 2^( 6 /128-1) - 2^(- 6 /128-1), 2^(- 6
                                    // /128-1)
            {0x3FA369AB3FFBF8B0uL,
             0x3FDECF482D8E67F1uL}, // 2^( 7 /128-1) - 2^(- 7 /128-1), 2^(- 7
                                    // /128-1)
            {0x3FA63009BA740A2AuL,
             0x3FDEA4AFA2A490DAuL}, // 2^( 8 /128-1) - 2^(- 8 /128-1), 2^(- 8
                                    // /128-1)
            {0x3FA8F692D8EA1B5AuL,
             0x3FDE7A51FBC74C83uL}, // 2^( 9 /128-1) - 2^(- 9 /128-1), 2^(- 9
                                    // /128-1)
            {0x3FABBD4BF0E31A6FuL,
             0x3FDE502EE78B3FF6uL}, // 2^( 10 /128-1) - 2^(- 10 /128-1), 2^(- 10
                                    // /128-1)
            {0x3FAE843A5840286AuL,
             0x3FDE264614F5A129uL}, // 2^( 11 /128-1) - 2^(- 11 /128-1), 2^(- 11
                                    // /128-1)
            {0x3FB0A5B1B2A46D0AuL,
             0x3FDDFC97337B9B5FuL}, // 2^( 12 /128-1) - 2^(- 12 /128-1), 2^(- 12
                                    // /128-1)
            {0x3FB20966375ABCDFuL,
             0x3FDDD321F301B460uL}, // 2^( 13 /128-1) - 2^(- 13 /128-1), 2^(- 13
                                    // /128-1)
            {0x3FB36D3D65DCA4E8uL,
             0x3FDDA9E603DB3285uL}, // 2^( 14 /128-1) - 2^(- 14 /128-1), 2^(- 14
                                    // /128-1)
            {0x3FB4D139EA06642AuL,
             0x3FDD80E316C98398uL}, // 2^( 15 /128-1) - 2^(- 15 /128-1), 2^(- 15
                                    // /128-1)
            {0x3FB6355E6FFBF9BAuL,
             0x3FDD5818DCFBA487uL}, // 2^( 16 /128-1) - 2^(- 16 /128-1), 2^(- 16
                                    // /128-1)
            {0x3FB799ADA42E4788uL,
             0x3FDD2F87080D89F2uL}, // 2^( 17 /128-1) - 2^(- 17 /128-1), 2^(- 17
                                    // /128-1)
            {0x3FB8FE2A336035BCuL,
             0x3FDD072D4A07897CuL}, // 2^( 18 /128-1) - 2^(- 18 /128-1), 2^(- 18
                                    // /128-1)
            {0x3FBA62D6CAABD6B6uL,
             0x3FDCDF0B555DC3FAuL}, // 2^( 19 /128-1) - 2^(- 19 /128-1), 2^(- 19
                                    // /128-1)
            {0x3FBBC7B617878BAFuL,
             0x3FDCB720DCEF9069uL}, // 2^( 20 /128-1) - 2^(- 20 /128-1), 2^(- 20
                                    // /128-1)
            {0x3FBD2CCAC7CB2A11uL,
             0x3FDC8F6D9406E7B5uL}, // 2^( 21 /128-1) - 2^(- 21 /128-1), 2^(- 21
                                    // /128-1)
            {0x3FBE921789B52185uL,
             0x3FDC67F12E57D14BuL}, // 2^( 22 /128-1) - 2^(- 22 /128-1), 2^(- 22
                                    // /128-1)
            {0x3FBFF79F0BEFA2C7uL,
             0x3FDC40AB5FFFD07AuL}, // 2^( 23 /128-1) - 2^(- 23 /128-1), 2^(- 23
                                    // /128-1)
            {0x3FC0AEB1FECAE3A9uL,
             0x3FDC199BDD85529CuL}, // 2^( 24 /128-1) - 2^(- 24 /128-1), 2^(- 24
                                    // /128-1)
            {0x3FC161B4871C5CECuL,
             0x3FDBF2C25BD71E09uL}, // 2^( 25 /128-1) - 2^(- 25 /128-1), 2^(- 25
                                    // /128-1)
            {0x3FC214D876F26FD0uL,
             0x3FDBCC1E904BC1D2uL}, // 2^( 26 /128-1) - 2^(- 26 /128-1), 2^(- 26
                                    // /128-1)
            {0x3FC2C81F2693816FuL,
             0x3FDBA5B030A1064AuL}, // 2^( 27 /128-1) - 2^(- 27 /128-1), 2^(- 27
                                    // /128-1)
            {0x3FC37B89EE88BEF7uL,
             0x3FDB7F76F2FB5E47uL}, // 2^( 28 /128-1) - 2^(- 28 /128-1), 2^(- 28
                                    // /128-1)
            {0x3FC42F1A27A0B3CDuL,
             0x3FDB59728DE5593AuL}, // 2^( 29 /128-1) - 2^(- 29 /128-1), 2^(- 29
                                    // /128-1)
            {0x3FC4E2D12AF1E037uL,
             0x3FDB33A2B84F15FBuL}, // 2^( 30 /128-1) - 2^(- 30 /128-1), 2^(- 30
                                    // /128-1)
            {0x3FC596B051DD508DuL,
             0x3FDB0E07298DB666uL}, // 2^( 31 /128-1) - 2^(- 31 /128-1), 2^(- 31
                                    // /128-1)
            {0x3FC64AB8F61134FAuL,
             0x3FDAE89F995AD3ADuL}, // 2^( 32 /128-1) - 2^(- 32 /128-1), 2^(- 32
                                    // /128-1)
            {0x3FC6FEEC718B79D1uL,
             0x3FDAC36BBFD3F37AuL}, // 2^( 33 /128-1) - 2^(- 33 /128-1), 2^(- 33
                                    // /128-1)
            {0x3FC7B34C1E9C607FuL,
             0x3FDA9E6B5579FDBFuL}, // 2^( 34 /128-1) - 2^(- 34 /128-1), 2^(- 34
                                    // /128-1)
            {0x3FC867D957E91912uL,
             0x3FDA799E1330B358uL}, // 2^( 35 /128-1) - 2^(- 35 /128-1), 2^(- 35
                                    // /128-1)
            {0x3FC91C95786E5C72uL,
             0x3FDA5503B23E255DuL}, // 2^( 36 /128-1) - 2^(- 36 /128-1), 2^(- 36
                                    // /128-1)
            {0x3FC9D181DB83072FuL,
             0x3FDA309BEC4A2D33uL}, // 2^( 37 /128-1) - 2^(- 37 /128-1), 2^(- 37
                                    // /128-1)
            {0x3FCA869FDCDAB512uL,
             0x3FDA0C667B5DE565uL}, // 2^( 38 /128-1) - 2^(- 38 /128-1), 2^(- 38
                                    // /128-1)
            {0x3FCB3BF0D8885D4CuL,
             0x3FD9E86319E32323uL}, // 2^( 39 /128-1) - 2^(- 39 /128-1), 2^(- 39
                                    // /128-1)
            {0x3FCBF1762B00EF69uL,
             0x3FD9C49182A3F090uL}, // 2^( 40 /128-1) - 2^(- 40 /128-1), 2^(- 40
                                    // /128-1)
            {0x3FCCA731311DF0FBuL,
             0x3FD9A0F170CA07BAuL}, // 2^( 41 /128-1) - 2^(- 41 /128-1), 2^(- 41
                                    // /128-1)
            {0x3FCD5D2348201C09uL,
             0x3FD97D829FDE4E50uL}, // 2^( 42 /128-1) - 2^(- 42 /128-1), 2^(- 42
                                    // /128-1)
            {0x3FCE134DCDB1FE3EuL,
             0x3FD95A44CBC8520FuL}, // 2^( 43 /128-1) - 2^(- 43 /128-1), 2^(- 43
                                    // /128-1)
            {0x3FCEC9B21FEA98EAuL,
             0x3FD93737B0CDC5E5uL}, // 2^( 44 /128-1) - 2^(- 44 /128-1), 2^(- 44
                                    // /128-1)
            {0x3FCF80519D5001D3uL,
             0x3FD9145B0B91FFC6uL}, // 2^( 45 /128-1) - 2^(- 45 /128-1), 2^(- 45
                                    // /128-1)
            {0x3FD01B96D26D026AuL,
             0x3FD8F1AE99157736uL}, // 2^( 46 /128-1) - 2^(- 46 /128-1), 2^(- 46
                                    // /128-1)
            {0x3FD07723CAFA6331uL,
             0x3FD8CF3216B5448CuL}, // 2^( 47 /128-1) - 2^(- 47 /128-1), 2^(- 47
                                    // /128-1)
            {0x3FD0D2D06841B373uL,
             0x3FD8ACE5422AA0DBuL}, // 2^( 48 /128-1) - 2^(- 48 /128-1), 2^(- 48
                                    // /128-1)
            {0x3FD12E9D5A715381uL,
             0x3FD88AC7D98A6699uL}, // 2^( 49 /128-1) - 2^(- 49 /128-1), 2^(- 49
                                    // /128-1)
            {0x3FD18A8B51F5C661uL,
             0x3FD868D99B4492EDuL}, // 2^( 50 /128-1) - 2^(- 50 /128-1), 2^(- 50
                                    // /128-1)
            {0x3FD1E69AFF7B04D7uL,
             0x3FD8471A4623C7ADuL}, // 2^( 51 /128-1) - 2^(- 51 /128-1), 2^(- 51
                                    // /128-1)
            {0x3FD242CD13EDD0F1uL,
             0x3FD82589994CCE13uL}, // 2^( 52 /128-1) - 2^(- 52 /128-1), 2^(- 52
                                    // /128-1)
            {0x3FD29F22407D0A0CuL,
             0x3FD80427543E1A12uL}, // 2^( 53 /128-1) - 2^(- 53 /128-1), 2^(- 53
                                    // /128-1)
            {0x3FD2FB9B369B0153uL,
             0x3FD7E2F336CF4E62uL}, // 2^( 54 /128-1) - 2^(- 54 /128-1), 2^(- 54
                                    // /128-1)
            {0x3FD35838A7FECEC8uL,
             0x3FD7C1ED0130C132uL}, // 2^( 55 /128-1) - 2^(- 55 /128-1), 2^(- 55
                                    // /128-1)
            {0x3FD3B4FB46A5A6CCuL,
             0x3FD7A11473EB0187uL}, // 2^( 56 /128-1) - 2^(- 56 /128-1), 2^(- 56
                                    // /128-1)
            {0x3FD411E3C4D4302FuL,
             0x3FD780694FDE5D3FuL}, // 2^( 57 /128-1) - 2^(- 57 /128-1), 2^(- 57
                                    // /128-1)
            {0x3FD46EF2D517DAC8uL,
             0x3FD75FEB564267C9uL}, // 2^( 58 /128-1) - 2^(- 58 /128-1), 2^(- 58
                                    // /128-1)
            {0x3FD4CC292A48369EuL,
             0x3FD73F9A48A58174uL}, // 2^( 59 /128-1) - 2^(- 59 /128-1), 2^(- 59
                                    // /128-1)
            {0x3FD5298777884B96uL,
             0x3FD71F75E8EC5F74uL}, // 2^( 60 /128-1) - 2^(- 60 /128-1), 2^(- 60
                                    // /128-1)
            {0x3FD5870E7047F1BCuL,
             0x3FD6FF7DF9519484uL}, // 2^( 61 /128-1) - 2^(- 61 /128-1), 2^(- 61
                                    // /128-1)
            {0x3FD5E4BEC8452A1AuL,
             0x3FD6DFB23C651A2FuL}, // 2^( 62 /128-1) - 2^(- 62 /128-1), 2^(- 62
                                    // /128-1)
            {0x3FD64299338D7827uL,
             0x3FD6C012750BDABFuL}, // 2^( 63 /128-1) - 2^(- 63 /128-1), 2^(- 63
                                    // /128-1)
            {0x3FD6A09E667F3BCDuL,
             0x3FD6A09E667F3BCDuL}, // 2^( 64 /128-1) - 2^(- 64 /128-1), 2^(- 64
                                    // /128-1)
            {0x3FD6FECF15CB0C0BuL,
             0x3FD68155D44CA973uL}, // 2^( 65 /128-1) - 2^(- 65 /128-1), 2^(- 65
                                    // /128-1)
            {0x3FD75D2BF6751239uL,
             0x3FD6623882552225uL}, // 2^( 66 /128-1) - 2^(- 66 /128-1), 2^(- 66
                                    // /128-1)
            {0x3FD7BBB5BDD665E8uL,
             0x3FD6434634CCC320uL}, // 2^( 67 /128-1) - 2^(- 67 /128-1), 2^(- 67
                                    // /128-1)
            {0x3FD81A6D219E6963uL,
             0x3FD6247EB03A5585uL}, // 2^( 68 /128-1) - 2^(- 68 /128-1), 2^(- 68
                                    // /128-1)
            {0x3FD87952D7D426DFuL,
             0x3FD605E1B976DC09uL}, // 2^( 69 /128-1) - 2^(- 69 /128-1), 2^(- 69
                                    // /128-1)
            {0x3FD8D86796D7AE49uL,
             0x3FD5E76F15AD2148uL}, // 2^( 70 /128-1) - 2^(- 70 /128-1), 2^(- 70
                                    // /128-1)
            {0x3FD937AC156373C8uL,
             0x3FD5C9268A5946B7uL}, // 2^( 71 /128-1) - 2^(- 71 /128-1), 2^(- 71
                                    // /128-1)
            {0x3FD997210A8DAEE4uL,
             0x3FD5AB07DD485429uL}, // 2^( 72 /128-1) - 2^(- 72 /128-1), 2^(- 72
                                    // /128-1)
            {0x3FD9F6C72DC9BA68uL,
             0x3FD58D12D497C7FDuL}, // 2^( 73 /128-1) - 2^(- 73 /128-1), 2^(- 73
                                    // /128-1)
            {0x3FDA569F36E974EAuL,
             0x3FD56F4736B527DAuL}, // 2^( 74 /128-1) - 2^(- 74 /128-1), 2^(- 74
                                    // /128-1)
            {0x3FDAB6A9DE1EA215uL,
             0x3FD551A4CA5D920FuL}, // 2^( 75 /128-1) - 2^(- 75 /128-1), 2^(- 75
                                    // /128-1)
            {0x3FDB16E7DBFC4CA3uL,
             0x3FD5342B569D4F82uL}, // 2^( 76 /128-1) - 2^(- 76 /128-1), 2^(- 76
                                    // /128-1)
            {0x3FDB7759E9782918uL,
             0x3FD516DAA2CF6642uL}, // 2^( 77 /128-1) - 2^(- 77 /128-1), 2^(- 77
                                    // /128-1)
            {0x3FDBD800BFEBF932uL,
             0x3FD4F9B2769D2CA7uL}, // 2^( 78 /128-1) - 2^(- 78 /128-1), 2^(- 78
                                    // /128-1)
            {0x3FDC38DD1916F025uL,
             0x3FD4DCB299FDDD0DuL}, // 2^( 79 /128-1) - 2^(- 79 /128-1), 2^(- 79
                                    // /128-1)
            {0x3FDC99EFAF1F1790uL,
             0x3FD4BFDAD5362A27uL}, // 2^( 80 /128-1) - 2^(- 80 /128-1), 2^(- 80
                                    // /128-1)
            {0x3FDCFB393C92B539uL,
             0x3FD4A32AF0D7D3DEuL}, // 2^( 81 /128-1) - 2^(- 81 /128-1), 2^(- 81
                                    // /128-1)
            {0x3FDD5CBA7C69B19CuL,
             0x3FD486A2B5C13CD0uL}, // 2^( 82 /128-1) - 2^(- 82 /128-1), 2^(- 82
                                    // /128-1)
            {0x3FDDBE742A06FF34uL,
             0x3FD46A41ED1D0057uL}, // 2^( 83 /128-1) - 2^(- 83 /128-1), 2^(- 83
                                    // /128-1)
            {0x3FDE2067013A029DuL,
             0x3FD44E086061892DuL}, // 2^( 84 /128-1) - 2^(- 84 /128-1), 2^(- 84
                                    // /128-1)
            {0x3FDE8293BE3FFB87uL,
             0x3FD431F5D950A897uL}, // 2^( 85 /128-1) - 2^(- 85 /128-1), 2^(- 85
                                    // /128-1)
            {0x3FDEE4FB1DC56E75uL,
             0x3FD4160A21F72E2AuL}, // 2^( 86 /128-1) - 2^(- 86 /128-1), 2^(- 86
                                    // /128-1)
            {0x3FDF479DDCE78F58uL,
             0x3FD3FA4504AC801CuL}, // 2^( 87 /128-1) - 2^(- 87 /128-1), 2^(- 87
                                    // /128-1)
            {0x3FDFAA7CB935ACFEuL,
             0x3FD3DEA64C123422uL}, // 2^( 88 /128-1) - 2^(- 88 /128-1), 2^(- 88
                                    // /128-1)
            {0x3FE006CC38594EB1uL,
             0x3FD3C32DC313A8E5uL}, // 2^( 89 /128-1) - 2^(- 89 /128-1), 2^(- 89
                                    // /128-1)
            {0x3FE03878E0EB1569uL,
             0x3FD3A7DB34E59FF7uL}, // 2^( 90 /128-1) - 2^(- 90 /128-1), 2^(- 90
                                    // /128-1)
            {0x3FE06A44B5C74101uL,
             0x3FD38CAE6D05D866uL}, // 2^( 91 /128-1) - 2^(- 91 /128-1), 2^(- 91
                                    // /128-1)
            {0x3FE09C3016A0D077uL,
             0x3FD371A7373AA9CBuL}, // 2^( 92 /128-1) - 2^(- 92 /128-1), 2^(- 92
                                    // /128-1)
            {0x3FE0CE3B63676360uL,
             0x3FD356C55F929FF1uL}, // 2^( 93 /128-1) - 2^(- 93 /128-1), 2^(- 93
                                    // /128-1)
            {0x3FE10066FC47F240uL,
             0x3FD33C08B26416FFuL}, // 2^( 94 /128-1) - 2^(- 94 /128-1), 2^(- 94
                                    // /128-1)
            {0x3FE132B341AD8761uL,
             0x3FD32170FC4CD831uL}, // 2^( 95 /128-1) - 2^(- 95 /128-1), 2^(- 95
                                    // /128-1)
            {0x3FE165209441F823uL,
             0x3FD306FE0A31B715uL}, // 2^( 96 /128-1) - 2^(- 96 /128-1), 2^(- 96
                                    // /128-1)
            {0x3FE197AF54EE9EBBuL,
             0x3FD2ECAFA93E2F56uL}, // 2^( 97 /128-1) - 2^(- 97 /128-1), 2^(- 97
                                    // /128-1)
            {0x3FE1CA5FE4DD1475uL,
             0x3FD2D285A6E4030BuL}, // 2^( 98 /128-1) - 2^(- 98 /128-1), 2^(- 98
                                    // /128-1)
            {0x3FE1FD32A577EC72uL,
             0x3FD2B87FD0DAD990uL}, // 2^( 99 /128-1) - 2^(- 99 /128-1), 2^(- 99
                                    // /128-1)
            {0x3FE23027F86B6ED6uL,
             0x3FD29E9DF51FDEE1uL}, // 2^( 100 /128-1) - 2^(- 100 /128-1), 2^(-
                                    // 100 /128-1)
            {0x3FE263403FA65489uL,
             0x3FD284DFE1F56381uL}, // 2^( 101 /128-1) - 2^(- 101 /128-1), 2^(-
                                    // 101 /128-1)
            {0x3FE2967BDD5A8364uL,
             0x3FD26B4565E27CDDuL}, // 2^( 102 /128-1) - 2^(- 102 /128-1), 2^(-
                                    // 102 /128-1)
            {0x3FE2C9DB33FDCAE9uL,
             0x3FD251CE4FB2A63FuL}, // 2^( 103 /128-1) - 2^(- 103 /128-1), 2^(-
                                    // 103 /128-1)
            {0x3FE2FD5EA64AA180uL,
             0x3FD2387A6E756238uL}, // 2^( 104 /128-1) - 2^(- 104 /128-1), 2^(-
                                    // 104 /128-1)
            {0x3FE331069740E22FuL,
             0x3FD21F49917DDC96uL}, // 2^( 105 /128-1) - 2^(- 105 /128-1), 2^(-
                                    // 105 /128-1)
            {0x3FE364D36A268AE0uL,
             0x3FD2063B88628CD6uL}, // 2^( 106 /128-1) - 2^(- 106 /128-1), 2^(-
                                    // 106 /128-1)
            {0x3FE398C582887B27uL,
             0x3FD1ED5022FCD91DuL}, // 2^( 107 /128-1) - 2^(- 107 /128-1), 2^(-
                                    // 107 /128-1)
            {0x3FE3CCDD443B3394uL,
             0x3FD1D4873168B9AAuL}, // 2^( 108 /128-1) - 2^(- 108 /128-1), 2^(-
                                    // 108 /128-1)
            {0x3FE4011B135B9590uL,
             0x3FD1BBE084045CD4uL}, // 2^( 109 /128-1) - 2^(- 109 /128-1), 2^(-
                                    // 109 /128-1)
            {0x3FE4357F544FA3C1uL,
             0x3FD1A35BEB6FCB75uL}, // 2^( 110 /128-1) - 2^(- 110 /128-1), 2^(-
                                    // 110 /128-1)
            {0x3FE46A0A6BC742FDuL,
             0x3FD18AF9388C8DEAuL}, // 2^( 111 /128-1) - 2^(- 111 /128-1), 2^(-
                                    // 111 /128-1)
            {0x3FE49EBCBEBCFBCAuL,
             0x3FD172B83C7D517BuL}, // 2^( 112 /128-1) - 2^(- 112 /128-1), 2^(-
                                    // 112 /128-1)
            {0x3FE4D396B276BC6FuL,
             0x3FD15A98C8A58E51uL}, // 2^( 113 /128-1) - 2^(- 113 /128-1), 2^(-
                                    // 113 /128-1)
            {0x3FE50898AC869B96uL,
             0x3FD1429AAEA92DE0uL}, // 2^( 114 /128-1) - 2^(- 114 /128-1), 2^(-
                                    // 114 /128-1)
            {0x3FE53DC312CB9B7AuL,
             0x3FD12ABDC06C31CCuL}, // 2^( 115 /128-1) - 2^(- 115 /128-1), 2^(-
                                    // 115 /128-1)
            {0x3FE573164B726DB6uL,
             0x3FD11301D0125B51uL}, // 2^( 116 /128-1) - 2^(- 116 /128-1), 2^(-
                                    // 116 /128-1)
            {0x3FE5A892BCF6379BuL,
             0x3FD0FB66AFFED31BuL}, // 2^( 117 /128-1) - 2^(- 117 /128-1), 2^(-
                                    // 117 /128-1)
            {0x3FE5DE38CE215725uL,
             0x3FD0E3EC32D3D1A2uL}, // 2^( 118 /128-1) - 2^(- 118 /128-1), 2^(-
                                    // 118 /128-1)
            {0x3FE61408E60E2888uL,
             0x3FD0CC922B7247F7uL}, // 2^( 119 /128-1) - 2^(- 119 /128-1), 2^(-
                                    // 119 /128-1)
            {0x3FE64A036C27CC52uL,
             0x3FD0B5586CF9890FuL}, // 2^( 120 /128-1) - 2^(- 120 /128-1), 2^(-
                                    // 120 /128-1)
            {0x3FE68028C82AEE2FuL,
             0x3FD09E3ECAC6F383uL}, // 2^( 121 /128-1) - 2^(- 121 /128-1), 2^(-
                                    // 121 /128-1)
            {0x3FE6B67962268C43uL,
             0x3FD0874518759BC8uL}, // 2^( 122 /128-1) - 2^(- 122 /128-1), 2^(-
                                    // 122 /128-1)
            {0x3FE6ECF5A27CBF28uL,
             0x3FD0706B29DDF6DEuL}, // 2^( 123 /128-1) - 2^(- 123 /128-1), 2^(-
                                    // 123 /128-1)
            {0x3FE7239DF1E38286uL,
             0x3FD059B0D3158574uL}, // 2^( 124 /128-1) - 2^(- 124 /128-1), 2^(-
                                    // 124 /128-1)
            {0x3FE75A72B9657E51uL,
             0x3FD04315E86E7F85uL}, // 2^( 125 /128-1) - 2^(- 125 /128-1), 2^(-
                                    // 125 /128-1)
            {0x3FE791746262D0A8uL,
             0x3FD02C9A3E778061uL}, // 2^( 126 /128-1) - 2^(- 126 /128-1), 2^(-
                                    // 126 /128-1)
            {0x3FE7C8A35691D856uL,
             0x3FD0163DA9FB3335uL} // 2^( 127 /128-1) - 2^(- 127 /128-1), 2^(-
                                   // 127 /128-1)
        },
        0x42C8000000000000uL, /* _dbShifter = 1.5 * 2^(52-k)*/
        0x40861d99u,          /* _iDomainRange 0x40861d9ac12a3e85
                                 =(1021*2^K-0.5)*log(2)/2^K -needed for quick exp*/
        0x3FF0000000000000uL, /* _dPC1 */
        0x3FDFFFFFFFFFFDBDuL, /* _dPC2 */
        0x3FC55555555554ADuL, /* _dPC3 */
        0x3FA55555CF16D299uL, /* _dPC4 */
        0x3F8111115712F425uL, /* _dPC5 */
        0x7ff0000000000000uL, /* _lExpMask */
        0x000000000000FFC0uL, /* _lBias */
        ((1 << 7) - 1)        /* _lIndexMask */
};                            /*dCosh_Table*/
static const _iml_dp_union_t __dsinh_la_CoutTab[149] = {
    0x00000000, 0x3FF00000, /* T[0] = 1                       */
    0x00000000, 0x00000000, /* D[0] = 0                       */
    0x3E778061, 0x3FF02C9A, /* T[1] = 1.010889286051700475255 */
    0x535B085D, 0xBC719083, /* D[1] = -1.5234778603368578e-17 */
    0xD3158574, 0x3FF059B0, /* T[2] = 1.021897148654116627142 */
    0xA475B465, 0x3C8D73E2, /* D[2] = 5.10922502897344397e-17 */
    0x18759BC8, 0x3FF08745, /* T[3] = 1.033024879021228414899 */
    0x4BB284FF, 0x3C6186BE, /* D[3] = 7.60083887402708891e-18 */
    0x6CF9890F, 0x3FF0B558, /* T[4] = 1.044273782427413754803 */
    0x4ADC610B, 0x3C98A62E, /* D[4] = 8.55188970553796446e-17 */
    0x32D3D1A2, 0x3FF0E3EC, /* T[5] = 1.055645178360557157049 */
    0x27C57B53, 0x3C403A17, /* D[5] = 1.75932573877209185e-18 */
    0xD0125B51, 0x3FF11301, /* T[6] = 1.067140400676823697168 */
    0x39449B3A, 0xBC96C510, /* D[6] = -7.8998539668415819e-17 */
    0xAEA92DE0, 0x3FF1429A, /* T[7] = 1.078760797757119860307 */
    0x9AF1369E, 0xBC932FBF, /* D[7] = -6.6566604360565930e-17 */
    0x3C7D517B, 0x3FF172B8, /* T[8] = 1.090507732665257689675 */
    0xB9D78A76, 0xBC819041, /* D[8] = -3.0467820798124709e-17 */
    0xEB6FCB75, 0x3FF1A35B, /* T[9] = 1.102382583307840890896 */
    0x7B4968E4, 0x3C8E5B4C, /* D[9] = 5.26603687157069445e-17 */
    0x3168B9AA, 0x3FF1D487, /* T[10] = 1.114386742595892432206 */
    0x00A2643C, 0x3C9E016E, /* D[10] = 1.04102784568455711e-16 */
    0x88628CD6, 0x3FF2063B, /* T[11] = 1.126521618608241848136 */
    0x814A8495, 0x3C8DC775, /* D[11] = 5.16585675879545668e-17 */
    0x6E756238, 0x3FF2387A, /* T[12] = 1.138788634756691564576 */
    0xB6C70573, 0x3C99B07E, /* D[12] = 8.91281267602540758e-17 */
    0x65E27CDD, 0x3FF26B45, /* T[13] = 1.151189229952982673311 */
    0x9940E9D9, 0x3C82BD33, /* D[13] = 3.25071021886382730e-17 */
    0xF51FDEE1, 0x3FF29E9D, /* T[14] = 1.163724858777577475522 */
    0xAFAD1255, 0x3C8612E8, /* D[14] = 3.82920483692409357e-17 */
    0xA6E4030B, 0x3FF2D285, /* T[15] = 1.176396991650281220743 */
    0x54DB41D5, 0x3C900247, /* D[15] = 5.55420325421807881e-17 */
    0x0A31B715, 0x3FF306FE, /* T[16] = 1.189207115002721026897 */
    0xD23182E4, 0x3C86F46A, /* D[16] = 3.98201523146564623e-17 */
    0xB26416FF, 0x3FF33C08, /* T[17] = 1.202156731452703075647 */
    0x843659A6, 0x3C932721, /* D[17] = 6.64498149925230086e-17 */
    0x373AA9CB, 0x3FF371A7, /* T[18] = 1.215247359980468955243 */
    0xBF42EAE2, 0xBC963AEA, /* D[18] = -7.7126306926814877e-17 */
    0x34E59FF7, 0x3FF3A7DB, /* T[19] = 1.228480536106870024682 */
    0xD661F5E3, 0xBC75E436, /* D[19] = -1.8987816313025299e-17 */
    0x4C123422, 0x3FF3DEA6, /* T[20] = 1.241857812073484002013 */
    0x11F09EBC, 0x3C8ADA09, /* D[20] = 4.65802759183693656e-17 */
    0x21F72E2A, 0x3FF4160A, /* T[21] = 1.255380757024691096291 */
    0x1C309278, 0xBC5EF369, /* D[21] = -6.7113898212968785e-18 */
    0x6061892D, 0x3FF44E08, /* T[22] = 1.269050957191733219886 */
    0x04EF80D0, 0x3C489B7A, /* D[22] = 2.66793213134218605e-18 */
    0xB5C13CD0, 0x3FF486A2, /* T[23] = 1.282870016078778263591 */
    0xB69062F0, 0x3C73C1A3, /* D[23] = 1.71359491824356104e-17 */
    0xD5362A27, 0x3FF4BFDA, /* T[24] = 1.296839554651009640551 */
    0xAFEC42E2, 0x3C7D4397, /* D[24] = 2.53825027948883151e-17 */
    0x769D2CA7, 0x3FF4F9B2, /* T[25] = 1.310961211524764413738 */
    0xD25957E3, 0xBC94B309, /* D[25] = -7.1815361355194539e-17 */
    0x569D4F82, 0x3FF5342B, /* T[26] = 1.325236643159741323217 */
    0x1DB13CAD, 0xBC807ABE, /* D[26] = -2.8587312100388613e-17 */
    0x36B527DA, 0x3FF56F47, /* T[27] = 1.339667524053302916087 */
    0x011D93AD, 0x3C99BB2C, /* D[27] = 8.92728259483173191e-17 */
    0xDD485429, 0x3FF5AB07, /* T[28] = 1.354255546936892651289 */
    0x054647AD, 0x3C96324C, /* D[28] = 7.70094837980298924e-17 */
    0x15AD2148, 0x3FF5E76F, /* T[29] = 1.369002422974590515992 */
    0x3080E65E, 0x3C9BA6F9, /* D[29] = 9.59379791911884828e-17 */
    0xB03A5585, 0x3FF6247E, /* T[30] = 1.383909881963832022578 */
    0x7E40B497, 0xBC9383C1, /* D[30] = -6.7705116587947862e-17 */
    0x82552225, 0x3FF66238, /* T[31] = 1.398979672538311236352 */
    0x87591C34, 0xBC9BB609, /* D[31] = -9.6142132090513227e-17 */
    0x667F3BCD, 0x3FF6A09E, /* T[32] = 1.414213562373095145475 */
    0x13B26456, 0xBC9BDD34, /* D[32] = -9.6672933134529130e-17 */
    0x3C651A2F, 0x3FF6DFB2, /* T[33] = 1.429613338391970023267 */
    0x683C88AB, 0xBC6BBE3A, /* D[33] = -1.2031642489053655e-17 */
    0xE8EC5F74, 0x3FF71F75, /* T[34] = 1.445180806977046650275 */
    0x86887A99, 0xBC816E47, /* D[34] = -3.0237581349939875e-17 */
    0x564267C9, 0x3FF75FEB, /* T[35] = 1.460917794180647044655 */
    0x57316DD3, 0xBC902459, /* D[35] = -5.6003771860752163e-17 */
    0x73EB0187, 0x3FF7A114, /* T[36] = 1.476826145939499346227 */
    0xEE04992F, 0xBC841577, /* D[36] = -3.4839945568927958e-17 */
    0x36CF4E62, 0x3FF7E2F3, /* T[37] = 1.492907728291264835008 */
    0xBA15797E, 0x3C705D02, /* D[37] = 1.41929201542840360e-17 */
    0x994CCE13, 0x3FF82589, /* T[38] = 1.509164427593422841412 */
    0xD41532D8, 0xBC9D4C1D, /* D[38] = -1.0164553277542950e-16 */
    0x9B4492ED, 0x3FF868D9, /* T[39] = 1.525598150744538417101 */
    0x9BD4F6BA, 0xBC9FC6F8, /* D[39] = -1.1024941712342561e-16 */
    0x422AA0DB, 0x3FF8ACE5, /* T[40] = 1.542210825407940744114 */
    0x56864B27, 0x3C96E9F1, /* D[40] = 7.94983480969762076e-17 */
    0x99157736, 0x3FF8F1AE, /* T[41] = 1.559004400237836929222 */
    0xA2E3976C, 0x3C85CC13, /* D[41] = 3.78120705335752751e-17 */
    0xB0CDC5E5, 0x3FF93737, /* T[42] = 1.575980845107886496592 */
    0x81B57EBC, 0xBC675FC7, /* D[42] = -1.0136916471278303e-17 */
    0x9FDE4E50, 0x3FF97D82, /* T[43] = 1.593142151342266998881 */
    0x7C1B85D1, 0xBC9D185B, /* D[43] = -1.0094406542311963e-16 */
    0x82A3F090, 0x3FF9C491, /* T[44] = 1.610490331949254283472 */
    0xB071F2BE, 0x3C7C7C46, /* D[44] = 2.47071925697978889e-17 */
    0x7B5DE565, 0x3FFA0C66, /* T[45] = 1.628027421857347833978 */
    0x5D1CD533, 0xBC935949, /* D[45] = -6.7129550847070839e-17 */
    0xB23E255D, 0x3FFA5503, /* T[46] = 1.645755478153964945776 */
    0xDB8D41E1, 0xBC9D2F6E, /* D[46] = -1.0125679913674773e-16 */
    0x5579FDBF, 0x3FFA9E6B, /* T[47] = 1.663676580326736376136 */
    0x0EF7FD31, 0x3C90FAC9, /* D[47] = 5.89099269671309991e-17 */
    0x995AD3AD, 0x3FFAE89F, /* T[48] = 1.681792830507429004072 */
    0x345DCC81, 0x3C97A1CD, /* D[48] = 8.19901002058149703e-17 */
    0xB84F15FB, 0x3FFB33A2, /* T[49] = 1.700106353718523477525 */
    0x3084D708, 0xBC62805E, /* D[49] = -8.0237193703976998e-18 */
    0xF2FB5E47, 0x3FFB7F76, /* T[50] = 1.718619298122477934143 */
    0x7E54AC3B, 0xBC75584F, /* D[50] = -1.8513804182631109e-17 */
    0x904BC1D2, 0x3FFBCC1E, /* T[51] = 1.73733383527370621735  */
    0x7A2D9E84, 0x3C823DD0, /* D[51] = 3.16438929929295719e-17 */
    0xDD85529C, 0x3FFC199B, /* T[52] = 1.756252160373299453511 */
    0x895048DD, 0x3C811065, /* D[52] = 2.96014069544887343e-17 */
    0x2E57D14B, 0x3FFC67F1, /* T[53] = 1.775376492526521188253 */
    0xFF483CAD, 0x3C92884D, /* D[53] = 6.42973179655657173e-17 */
    0xDCEF9069, 0x3FFCB720, /* T[54] = 1.7947090750031071682   */
    0xD1E949DB, 0x3C7503CB, /* D[54] = 1.82274584279120882e-17 */
    0x4A07897C, 0x3FFD072D, /* T[55] = 1.814252175500398855945 */
    0x43797A9C, 0xBC9CBC37, /* D[55] = -9.9695315389203494e-17 */
    0xDCFBA487, 0x3FFD5818, /* T[56] = 1.834008086409342430656 */
    0xD75B3707, 0x3C82ED02, /* D[56] = 3.28310722424562714e-17 */
    0x03DB3285, 0x3FFDA9E6, /* T[57] = 1.853979125083385470774 */
    0x696DB532, 0x3C9C2300, /* D[57] = 9.76188749072759400e-17 */
    0x337B9B5F, 0x3FFDFC97, /* T[58] = 1.874167634110299962558 */
    0x4F184B5C, 0xBC91A5CD, /* D[58] = -6.1227634130041420e-17 */
    0xE78B3FF6, 0x3FFE502E, /* T[59] = 1.894575981586965607306 */
    0x80A9CC8F, 0x3C839E89, /* D[59] = 3.40340353521652984e-17 */
    0xA2A490DA, 0x3FFEA4AF, /* T[60] = 1.915206561397147400072 */
    0x179C2893, 0xBC9E9C23, /* D[60] = -1.0619946056195963e-16 */
    0xEE615A27, 0x3FFEFA1B, /* T[61] = 1.936061793492294347274 */
    0x86A4B6B0, 0x3C9DC7F4, /* D[61] = 1.03323859606763264e-16 */
    0x5B6E4540, 0x3FFF5076, /* T[62] = 1.957144124175400179411 */
    0x2DD8A18B, 0x3C99D3E1, /* D[62] = 8.96076779103666767e-17 */
    0x819E90D8, 0x3FFFA7C1, /* T[63] = 1.978456026387950927869 */
    0xF3A5931E, 0x3C874853, /* D[63] = 4.03887531092781669e-17 */
    0x00000000, 0x40000000, /* T[64] = 2                       */
    0x00000000, 0x00000000, /* D[64] = 0                       */
    /* Coefficients for exp(R) - 1 approximation by polynomial p(R) */
    0x00000000, 0x3FE00000, /* A2 = .500000000000000 */
    0x555548F8, 0x3FC55555, /* A3 = .166666666666579 */
    0x55558FCC, 0x3FA55555, /* A4 = .041666666666771 */
    0x3AAF20D3, 0x3F811112, /* A5 = .008333341995140 */
    0x1C2A3FFD, 0x3F56C16A, /* A6 = .001388887045923 */
    /* Coefficients for Taylor approximation of sinh(.) */
    0x55555555, 0x3FC55555, /* Q3 = .166666666666667 */
    0x11111111, 0x3F811111, /* Q5 = .008333333333333 */
    0x1A01A01A, 0x3F2A01A0, /* Q7 = .000198412698413 */
    0xA556C734, 0x3EC71DE3, /* Q9 = .000002755731922 */
    /* TWO_TO_THE_K_DIV_LN2 = 2^6/ln(2.0) rounded to double */
    0x652B82FE, 0x40571547, /* 92.332482616893658 */
    /* Right Shifter */
    0x00000000, 0x43380000, /* RS = 2^52 + 2^51 */
    /* RS_MUL = 2^27 + 1 stored in double */
    0x02000000, 0x41A00000,
    /* Overflow Threshold */
    0x8FB9F87E, 0x408633CE, /* OVF = 710.475860073943977 */
    /* Two parts of ln(2.0)/65 */
    0xFEFA0000, 0x3F862E42, /* LOG_HI = .010830424696223 */
    0xBC9E3B3A, 0x3D1CF79A, /* LOG_LO = 2.572804622327669e-14 */
    /* TINY and HUGE_VALUE values to process (under-) overflow */
    0x00000001, 0x00100000, 0xFFFFFFFF, 0x7FEFFFFF,
    /* TAYLOR_THRESHOLD = 2^{-5.2} rounded to double */
    0xDADBE120, 0x3F9BDB8C,
    /* EXP_OF_X_DIV_2_THRESHOLD = (52+10)/2*ln(2) rounded to double */
    0xE7026820, 0x40357CD0};
/*
//
//   Implementation of HA (High Accuracy) version of double precision vector
//   hyperbolic sine function starts here.
//
*/
inline int __devicelib_imf_internal_dsinh(const double *a, double *r) {
  int nRet = 0;
  double M;
  double Nj;
  double R, RHi, RMid, RLo;
  double p, pHi, pLo;
  double ph, pl;
  double scale;
  double MLog;
  double absAi;
  double rsq, podd, peven;
  double TpHi, TpLo, TmHi, TmLo;
  double TdH, TdL;
  double TsHi, TsLo, TsH, TsL;
  double tmp1, dbIn;
  double v1, v2, v3;
  _iml_uint32_t N, j;
  int exp;
  //    _IML_DCOREFN_PROLOG2_IN_C(0, _IML_MODEX87_NEAR53_IN_C, _MODE_UNCHANGED,
  //    n, a, r );
  /* Set all bits of scale to 0.                                           */
  /* Only bits of exponent field will be updated then before each use of   */
  /* scale. Notice, that all other bits (i.e. sign and significand) should */
  /* be kept 0 across iterations. Otherwise, they should be explicitly set */
  /* to 0 before each use of scale                                         */
  scale = ((const double *)__dsinh_la_CoutTab)[1];
  dbIn = (*a);
  /* Filter out INFs and NaNs */
  if (((((_iml_dp_union_t *)&dbIn)->bits.exponent) != 0x7FF)) {
    /* Here if argument is finite double precision number */
    exp = (((_iml_dp_union_t *)&dbIn)->bits.exponent);
    /* Check if argument is normalized */
    if (exp > 0) {
      absAi = dbIn;
      (((_iml_dp_union_t *)&absAi)->bits.sign = 0);
      /* Check if dbIn falls into "Near 0" range */
      if (exp > (0x3FF - 54)) {
        /* Here if argument is not within "Near 0" interval */
        /* Check if sinh(dbIn) overflows */
        if (absAi < ((const double *)__dsinh_la_CoutTab)[142]) {
          /* Here if sinh doesn't overflow */
          /* Check if |dbIn| is big enough to             */
          /* approximate sinh(|dbIn|) by exp( |dbIn| )/2  */
          if (absAi < ((const double *)__dsinh_la_CoutTab)[148]) {
            /* Here if |dbIn| is not big enough to         */
            /* approximate sinh(|dbIn|) by exp( |dbIn| )/2 */
            /* Check if path 5) or 6) should follow */
            if (absAi >= ((const double *)__dsinh_la_CoutTab)[147]) {
              /* Path 6 */
              /* Range Reduction part, path 6a) */
              tmp1 = (absAi * ((const double *)__dsinh_la_CoutTab)[139]);
              Nj = (tmp1 + ((const double *)__dsinh_la_CoutTab)[140]);
              M = (Nj - ((const double *)__dsinh_la_CoutTab)[140]);
              tmp1 = (absAi - M * ((const double *)__dsinh_la_CoutTab)[143]);
              MLog = (-M * ((const double *)__dsinh_la_CoutTab)[144]);
              /* R + RLo := tmp1 + MLog */
              v1 = ((tmp1) + (MLog));
              v2 = ((tmp1)-v1);
              v3 = (v1 + v2);
              v2 = ((MLog) + v2);
              v3 = ((tmp1)-v3);
              v3 = (v2 + v3);
              R = v1;
              RLo = v3;
              ;
              /* Splitting R into RHi and RMid */
              v1 = ((R) * (((const double *)__dsinh_la_CoutTab)[141]));
              v2 = (v1 - (R));
              v1 = (v1 - v2);
              v2 = ((R)-v1);
              RHi = v1;
              RMid = v2;
              ;
              /* Approximation part: polynomial series, */
              /*                              path 6b)  */
              rsq = R * R;
              podd = ((((const double *)__dsinh_la_CoutTab)[133] * rsq +
                       ((const double *)__dsinh_la_CoutTab)[131]) *
                      rsq) *
                     R;
              peven = (((((const double *)__dsinh_la_CoutTab)[134] * rsq +
                         ((const double *)__dsinh_la_CoutTab)[132]) *
                        rsq) +
                       ((const double *)__dsinh_la_CoutTab)[130]) *
                      rsq;
              /* Final reconstuction starts here, path 6c) */
              /* Get N and j from Nj's significand */
              N = (((_iml_dp_union_t *)&Nj)->bits.lo_significand);
              j = N & ((1 << 6) - 1);
              N = N >> 6;
              /* Obtain scale = 2^{N - 1 + bias} */
              N = N + 0x3FF;
              N = N & 0x7FF;
              (((_iml_dp_union_t *)&scale)->bits.exponent = N - 1);
              TpHi = ((const double *)__dsinh_la_CoutTab)[2 * (j)] * scale;
              TpLo = ((const double *)__dsinh_la_CoutTab)[2 * (j) + 1] * scale;
              /* Obtain scale = 2^{-N - 2 + bias} */
              N = 2 * 0x3FF - N;
              (((_iml_dp_union_t *)&scale)->bits.exponent = N - 2);
              TmHi =
                  ((const double *)__dsinh_la_CoutTab)[2 * (64 - (j))] * scale;
              TmLo = ((const double *)__dsinh_la_CoutTab)[2 * (64 - (j)) + 1] *
                     scale;
              /* Get intermediate values */
              /* TdH + TdL = TpHi + TpLo - TmHi - TmLo */
              v1 = ((TpHi) + (-TmHi));
              tmp1 = ((TpHi)-v1);
              v2 = (tmp1 + (-TmHi));
              TdH = v1;
              TdL = v2;
              ;
              TdL -= TmLo;
              TdL += TpLo;
              /* Re-split TdH + TdL so that the most */
              /* significant part of the sum resides */
              /* in TdH                              */
              v1 = ((TdH) + (TdL));
              tmp1 = ((TdH)-v1);
              v2 = (tmp1 + (TdL));
              TdH = v1;
              TdL = v2;
              ;
              /* TsH + TsL = TpHi + TpLo + TmHi + TmLo */
              v1 = ((TpHi) + (TmHi));
              tmp1 = ((TpHi)-v1);
              v2 = (tmp1 + (TmHi));
              TsH = v1;
              TsL = v2;
              ;
              tmp1 = (TpLo + TmLo);
              TsL += tmp1;
              /* Re-split TsH + TsL so that the most */
              /* significant part of the sum resides */
              /* in TsH                              */
              v1 = ((TsH) + (TsL));
              tmp1 = ((TsH)-v1);
              v2 = (tmp1 + (TsL));
              TsH = v1;
              TsL = v2;
              ;
              /* Splitting TsH into high and low parts */
              v1 = ((TsH) * (((const double *)__dsinh_la_CoutTab)[141]));
              v2 = (v1 - (TsH));
              v1 = (v1 - v2);
              v2 = ((TsH)-v1);
              TsHi = v1;
              TsLo = v2;
              ;
              /* Gather the items in pLo and pHi */
              pLo = (RLo * TsL);
              pLo += (podd * TsL);
              pLo += (peven * TdL);
              pLo += (R * TsL);
              pLo += (RLo * TsH);
              pLo += TdL;
              ph = (podd * TsH);
              pl = (peven * TdH);
              /* pHi + pl = podd * TsH + peven * TdH */
              v1 = ((ph) + (pl));
              tmp1 = ((ph)-v1);
              v2 = (tmp1 + (pl));
              pHi = v1;
              pl = v2;
              ;
              pLo += pl;
              pLo += (RMid * TsLo);
              pLo += (RHi * TsLo);
              pLo += (RMid * TsHi);
              RHi = (RHi * TsHi);
              /* pHi + pl = RHi + pHi */
              v1 = ((RHi) + (pHi));
              tmp1 = ((RHi)-v1);
              v2 = (tmp1 + (pHi));
              pHi = v1;
              pl = v2;
              ;
              pLo += pl;
              /* pHi + pl = TdH + pHi */
              v1 = ((TdH) + (pHi));
              tmp1 = ((TdH)-v1);
              v2 = (tmp1 + (pHi));
              pHi = v1;
              pl = v2;
              ;
              pLo += pl;
              p = (pHi + pLo);
              (((_iml_dp_union_t *)&p)->bits.sign =
                   (((_iml_dp_union_t *)&dbIn)->bits.sign));
              (*r) = p;
            } else {
              /* Here if |dbIn| < TAYLOR_THRESHOLD, Path 5 */
              rsq = (absAi * absAi);
              p = (absAi *
                   (rsq *
                    (((const double *)__dsinh_la_CoutTab)[135] +
                     rsq * (((const double *)__dsinh_la_CoutTab)[136] +
                            rsq * (((const double *)__dsinh_la_CoutTab)[137] +
                                   rsq * ((const double *)
                                              __dsinh_la_CoutTab)[138])))));
              p += absAi;
              (((_iml_dp_union_t *)&p)->bits.sign =
                   (((_iml_dp_union_t *)&dbIn)->bits.sign));
              (*r) = p;
            }
          } else {
            /* Path 7 */
            /* Range Reduction part, 7a) */
            tmp1 = absAi * ((const double *)__dsinh_la_CoutTab)[139];
            Nj = (tmp1 + ((const double *)__dsinh_la_CoutTab)[140]);
            M = (Nj - ((const double *)__dsinh_la_CoutTab)[140]);
            R = (absAi - M * ((const double *)__dsinh_la_CoutTab)[143]);
            R -= (M * ((const double *)__dsinh_la_CoutTab)[144]);
            /* Approximation part: polynomial series, 7b) */
            p = ((((((const double *)__dsinh_la_CoutTab)[134] * R +
                    ((const double *)__dsinh_la_CoutTab)[133]) *
                       R +
                   ((const double *)__dsinh_la_CoutTab)[132]) *
                      R +
                  ((const double *)__dsinh_la_CoutTab)[131]) *
                     R +
                 ((const double *)__dsinh_la_CoutTab)[130]);
            p = (p * R);
            p = (p * R + R);
            /* Final reconstruction starts here, 7c) */
            /* Get N and j from Nj's significand */
            N = (((_iml_dp_union_t *)&Nj)->bits.lo_significand);
            j = N & ((1 << 6) - 1);
            N = N >> 6;
            N += 0x3FF;
            /* p = (T[j] * p +  D[j]) + T[j] */
            p *= ((const double *)__dsinh_la_CoutTab)[2 * (j)];
            p += ((const double *)__dsinh_la_CoutTab)[2 * (j) + 1];
            p += ((const double *)__dsinh_la_CoutTab)[2 * (j)];
            /* N = N - 1 */
            N = (N - 1) & 0x7FF;
            /* Check if path 7.1) or 7.2) should follow */
            if (N <= (0x7FF - 1)) {
              /* Path 7.1) */
              /* scale = 2^N */
              (((_iml_dp_union_t *)&scale)->bits.exponent = N);
              /* scale * (T[j] + (D[j] + T[j] * p)) */
              p = (p * scale);
            } else {
              /* Path 7.2) "scale overflow" */
              /* scale = 2^(N - 1) */
              (((_iml_dp_union_t *)&scale)->bits.exponent = N - 1);
              /* 2.0*(scale * (T[j] + (D[j] + T[j] * p))) */
              p = (p * scale);
              p = (p * ((const double *)__dsinh_la_CoutTab)[128]);
            }
            (((_iml_dp_union_t *)&p)->bits.sign =
                 (((_iml_dp_union_t *)&dbIn)->bits.sign));
            (*r) = p;
          }
        } else {
          /* Here if sinh overflows, Path 4 */
          (*r) = ((const double *)__dsinh_la_CoutTab)[146] * dbIn;
          //_IML_FUNC_NAME_CALL_EM(dError,(_IML_SCODE_IN_C() =
          //IML_STATUS_OVERFLOW,i,a,a, r, r, _IML_THISFUNC_NAME));
          nRet = 3;
        }
      } else {
        /* Here if argument is within "Near 0" interval, Path 3 */
        (*r) = (((const double *)__dsinh_la_CoutTab)[0] +
                ((const double *)__dsinh_la_CoutTab)[145]) *
               dbIn;
      }
    } else {
      /* Here if dbIn is zero or denormalized, Path 2 */
      v1 = dbIn * ((const double *)__dsinh_la_CoutTab)[145];
      (*r) = dbIn + v1;
    }
  } else {
    /* Here if argument is infinity or NaN, Path 1 */
    (*r) = dbIn + dbIn;
  }
  //_IML_COREFN_EPILOG2_IN_C();
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_sinh_d_la */
DEVICE_EXTERN_C_INLINE double __devicelib_imf_sinh(double x) {
  using namespace __imf_impl_sinh_d_la;
  double r;
  VUINT32 vm;
  double va1;
  double vr1;
  va1 = x;
  {
    double dN;
    double dM;
    double dR;
    double dR2;
    double dSinh_r;
    double dOut;
    double dTdif;
    double dTn;
    double dG1;
    double dG2;
    double dG3;
    VUINT64 lM;
    double dXSign;
    VUINT32 iAbsX;
    double dAbsX;
    VUINT64 lIndex;
    VUINT64 lX;
    VUINT32 iRangeMask;
    double dbInvLn2;
    double dbShifter;
    double dbLn2[2];
    double dPC[5];
    VUINT64 lIndexMask;
    VUINT32 iDomainRange;
    dbInvLn2 = as_double(__devicelib_imf_internal_dsinh_data._dbInvLn2);
    dbShifter = as_double(__devicelib_imf_internal_dsinh_data._dbShifter);
    lIndexMask = (__devicelib_imf_internal_dsinh_data._lIndexMask);
    dPC[1] = as_double(__devicelib_imf_internal_dsinh_data._dPC2);
    dPC[2] = as_double(__devicelib_imf_internal_dsinh_data._dPC3);
    dPC[3] = as_double(__devicelib_imf_internal_dsinh_data._dPC4);
    dPC[4] = as_double(__devicelib_imf_internal_dsinh_data._dPC5);
    dXSign = as_double(__devicelib_imf_internal_dsinh_data._dSign);
    iDomainRange = (__devicelib_imf_internal_dsinh_data._iDomainRange);
    // Compute argument sign and absolute argument
    dXSign = as_double((as_ulong(dXSign) & as_ulong(va1)));
    dAbsX = as_double((as_ulong(dXSign) ^ as_ulong(va1)));
    // dM = x*2^K/log(2) + RShifter
    dM = __fma(dAbsX, dbInvLn2, dbShifter);
    // Check for overflow\underflow
    lX = as_ulong(dAbsX);
    iAbsX = ((VUINT32)((VUINT64)lX >> 32));
    iRangeMask =
        ((VUINT32)(-(VSINT32)((VSINT32)iAbsX > (VSINT32)iDomainRange)));
    vm = 0;
    vm = iRangeMask;
    // Index and lookup
    lM = as_ulong(dM);
    // j index compute
    lIndex = (lM & lIndexMask);
    // Split j and N
    lM = (lM ^ lIndex);
    // R compute:
    // dN = dM - RShifter
    dN = (dM - dbShifter);
    dbLn2[0] = as_double(__devicelib_imf_internal_dsinh_data._dbLn2hi);
    // dR = dX - dN*Log2_hi/2^K
    dR = __fma(-(dbLn2[0]), dN, dAbsX);
    dbLn2[1] = as_double(__devicelib_imf_internal_dsinh_data._dbLn2lo);
    dTn = as_double(
        ((const VUINT64 *)(__devicelib_imf_internal_dsinh_data
                               ._dbT))[(((0 + lIndex) * (2 * 8)) >> (3)) + 1]);
    dTdif = as_double(
        ((const VUINT64 *)(__devicelib_imf_internal_dsinh_data
                               ._dbT))[(((0 + lIndex) * (2 * 8)) >> (3)) + 0]);
    // dR = (dX - dN*Log2_hi/2^K) - dN*Log2_lo/2^K
    dR = __fma(-(dbLn2[1]), dN, dR);
    // dR2 = dR^2
    dR2 = (dR * dR);
    // Compute G1,G2,G3: dTdif,dTn * 2^N,2^(-N):
    // lM now is an EXP(2^N)
    lM = ((VUINT64)(lM) << ((52 - 7)));
    // lX=dTdif
    lX = as_ulong(dTdif);
    lX = (lX + lM);
    // dG1 = dTdif*2^N
    dG1 = as_double(lX);
    lX = as_ulong(dTn);
    // dG2 = dTn*2^N
    lX = (lX + lM);
    dG2 = as_double(lX);
    // lX == dTn
    lX = as_ulong(dTn);
    lX = (lX - lM);
    // dM = dTn*2^-N
    dM = as_double(lX);
    // dG3 = dTn*2^N + dTn*2^-N
    dG3 = (dG2 + dM);
    // dG2 = dTn*2^N - dTn*2^-N
    dG2 = (dG2 - dM);
    //....sinh(r) = r*((a1=1)+r^2*(a3+r^2*a5)) = r + r*(r^2*(a3+r^2*a5)):
    // dSinh_r = (a3+r^2*a5)
    dSinh_r = __fma(dPC[4], dR2, dPC[2]);
    // dSinh_r = r^2*(a3+r^2*a5)
    dSinh_r = (dSinh_r * dR2);
    // dSinh_r = r + r*(r^2*(a3+r^2*a5))
    dSinh_r = __fma(dSinh_r, dR, dR);
    // poly(r) = (dG2+dG1)+dG3*sinh(dR)+dG1*sinh(dR)+(dG1+dG2)*dR2*(a2 +a4*dR2)
    // dOut = (a2 +a4*dR2)
    dOut = __fma(dPC[3], dR2, dPC[1]);
    // dOut = dR2*(a2 +a4*dR2)
    dOut = (dOut * dR2);
    // dG2 += dG1
    dG2 = (dG1 + dG2);
    // dOut = dG2*dR2*(a2 +a4*dR2)
    dOut = (dOut * dG2);
    // dG1 += dG3
    dG1 = (dG1 + dG3);
    // dOut = dG1*sinh(dR)+dG2*dR2*(a2 +a4*dR2)
    dOut = __fma(dSinh_r, dG1, dOut);
    // dOut = dG2 + dG1*sinh(dR)+dG2*dR2*(a2 +a4*dR2)
    dOut = (dOut + dG2);
    // Set result sign
    vr1 = as_double((as_ulong(dXSign) | as_ulong(dOut)));
  }
  if (__builtin_expect((vm) != 0, 0)) {
    double __cout_a1;
    double __cout_r1;
    ((double *)&__cout_a1)[0] = va1;
    ((double *)&__cout_r1)[0] = vr1;
    __devicelib_imf_internal_dsinh(&__cout_a1, &__cout_r1);
    vr1 = ((const double *)&__cout_r1)[0];
  }
  r = vr1;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
