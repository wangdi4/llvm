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
//      For    0.0    <= x <=  7.0/16.0: atan(x) = atan(0.0) + atan(s), where
s=(x-0.0)/(1.0+0.0*x)
//      For  7.0/16.0 <= x <= 11.0/16.0: atan(x) = atan(0.5) + atan(s), where
s=(x-0.5)/(1.0+0.5*x)
//      For 11.0/16.0 <= x <= 19.0/16.0: atan(x) = atan(1.0) + atan(s), where
s=(x-1.0)/(1.0+1.0*x)
//      For 19.0/16.0 <= x <= 39.0/16.0: atan(x) = atan(1.5) + atan(s), where
s=(x-1.5)/(1.0+1.5*x)
//      For 39.0/16.0 <= x <=    inf   : atan(x) = atan(inf) + atan(s), where
s=-1.0/x
//      Where atan(s) ~= s+s^3*Poly11(s^2) on interval |s|<7.0/0.16.
//
*/
#include "_imf_include_fp64.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_atan_d_ha {
namespace {
typedef struct {
  VUINT64 AbsMask;
  VUINT64 Shifter;
  VUINT64 MaxThreshold;
  VUINT64 MOne;
  VUINT64 One;
  VUINT64 LargeX;
  VUINT64 Zero;
  VUINT64 Tbl_H[32];
  VUINT64 Tbl_L[32];
  VUINT64 dIndexMed;
  VUINT64 Pi2;
  VUINT64 Pi2_low;
  VUINT64 coeff[6];
} __devicelib_imf_internal_datan_data_avx512_t;
static const __devicelib_imf_internal_datan_data_avx512_t
    __devicelib_imf_internal_datan_data_avx512 = {
        /*== AbsMask ==*/
        0x7fffffffffffffffuL
        /*== Shifter ==*/
        ,
        0x4318000000000000uL
        /*== MaxThreshold ==*/
        ,
        0x401f800000000000uL
        /*== MOne ==*/
        ,
        0xbff0000000000000uL
        /*== One ==*/
        ,
        0x3ff0000000000000uL
        /*== LargeX ==*/
        ,
        0x47f0000000000000uL
        /*== Zero ==*/
        ,
        0x0000000000000000uL,
        {
            /*== Tbl_H ==*/
            0x0000000000000000uL, 0x3fcf5b75f92c80dduL, 0x3fddac670561bb4fuL,
            0x3fe4978fa3269ee1uL, 0x3fe921fb54442d18uL, 0x3fecac7c57846f9euL,
            0x3fef730bd281f69buL, 0x3ff0d38f2c5ba09fuL, 0x3ff1b6e192ebbe44uL,
            0x3ff270ef55a53a25uL, 0x3ff30b6d796a4da8uL, 0x3ff38d6a6ce13353uL,
            0x3ff3fc176b7a8560uL, 0x3ff45b54837351a0uL, 0x3ff4ae10fc6589a5uL,
            0x3ff4f68dea672617uL, 0x3ff5368c951e9cfduL, 0x3ff56f6f33a3e6a7uL,
            0x3ff5a25052114e60uL, 0x3ff5d013c41adabduL, 0x3ff5f97315254857uL,
            0x3ff61f06c6a92b89uL, 0x3ff6414d44094c7cuL, 0x3ff660b02c736a06uL,
            0x3ff67d8863bc99bduL, 0x3ff698213a9d5053uL, 0x3ff6b0bae830c070uL,
            0x3ff6c78c7edeb195uL, 0x3ff6dcc57bb565fduL, 0x3ff6f08f07435fecuL,
            0x3ff7030cf9403197uL, 0x3ff7145eac2088a4uL,
        },
        {
            /*== Tbl_L ==*/
            0x0000000000000000uL, 0x3c68ab6e3cf7afbduL, 0x3c7a2b7f222f65e2uL,
            0x3c72419a87f2a458uL, 0x3c81a62633145c07uL, 0x3c80dae13ad18a6buL,
            0x3c7007887af0cbbduL, 0xbc9bd0dc231bfd70uL, 0x3c9b1b466a88828euL,
            0xbc9a66b1af5f84fbuL, 0x3c96254cb03bb199uL, 0xbc812c77e8a80f5cuL,
            0xbc4441a3bd3f1084uL, 0x3c79e4a72eedacc4uL, 0xbc93b03e8a27f555uL,
            0x3c9934f9f2b0020euL, 0xbc996f47948a99f1uL, 0xbc7df6edd6f1ec3buL,
            0x3c78c2d0c89de218uL, 0x3c9f82bba194dd5duL, 0xbc831151a43b51cauL,
            0xbc8487d50bceb1a5uL, 0xbc9c5f60a65c7397uL, 0xbc7acb6afb332a0fuL,
            0xbc99b7bd2e1e8c9cuL, 0xbc9b9839085189e3uL, 0xbc97d1ab82ffb70buL,
            0x3c99239ad620ffe2uL, 0xbc929c86447928e7uL, 0xbc8957a7170df016uL,
            0xbc7cbe1896221608uL, 0xbc9fda5797b32a0buL,
        }
        /*== dIndexMed ==*/
        ,
        0x4318000000000010uL
        /*== Pi2 ==*/
        ,
        0x3ff921fb54442d18uL
        /*== Pi2_low ==*/
        ,
        0x3c91a62633145c07uL,
        {/*== coeff6 ==*/
         0x3fb2e9b9f5c4fe97uL
         /*== coeff5 ==*/
         ,
         0xbfb74257c46790ccuL
         /*== coeff4 ==*/
         ,
         0x3fbc71bfeff916a0uL
         /*== coeff3 ==*/
         ,
         0xbfc249248eef04dauL
         /*== coeff2 ==*/
         ,
         0x3fc999999998741euL
         /*== coeff1 ==*/
         ,
         0xbfd555555555554duL}}; /*dExp_Table*/
/* Table parameters */
typedef struct {
  // 6 lines by 4 items: dbBase,dbAtanBaseHi,dbAtanBaseLo,dbCAnd
  VUINT64 _ATAN_TBL[6][4];
  VUINT64 _dATAN_BOUND1;
  VUINT64 _dATAN_BOUND3;
  VUINT64 _dATAN_BOUND4;
  VUINT64 _dATAN_BOUND2;
  VUINT64 _dAtan1o2Hi;
  VUINT64 _dAtan1o2Lo;
  VUINT64 _dAtanOneHi;
  VUINT64 _dAtanOneLo;
  VUINT64 _dAtan3o2Hi;
  VUINT64 _dAtan3o2Lo;
  VUINT64 _dAtanInfHi;
  VUINT64 _dAtanInfLo;
  VUINT64 _dHalf;
  VUINT64 _dThreeHalf;
  VUINT64 _dSIGN_MASK;
  VUINT64 _dABS_MASK;
  VUINT64 _dPiHI;
  VUINT64 _dPiLO;
  VUINT64 _dHIGH_26_MASK;
  VUINT32 _iATAN_BOUND1;
  VUINT32 _iATAN_BOUND3;
  VUINT32 _iATAN_BOUND4;
  VUINT32 _iATAN_BOUND2;
  VUINT32 _i4;
  VUINT64 _dONE;
  VUINT64 _dPC11;
  VUINT64 _dPC10;
  VUINT64 _dPC9;
  VUINT64 _dPC8;
  VUINT64 _dPC7;
  VUINT64 _dPC6;
  VUINT64 _dPC5;
  VUINT64 _dPC4;
  VUINT64 _dPC3;
  VUINT64 _dPC2;
  VUINT64 _dPC1;
  VUINT64 _dPC0;
  VUINT32 _iCHK_WORK_SUB;
  VUINT32 _iCHK_WORK_CMP;
} __devicelib_imf_internal_datan_data_t;
static const __devicelib_imf_internal_datan_data_t
    __devicelib_imf_internal_datan_data = {
        {
            {0x3FF0000000000000uL, 0x0000000000000000uL, 0x3FF921FB54442D18uL,
             0x3C91A62633145C07uL},
            {0x3FF8000000000000uL, 0xffffffffffffffffuL, 0x3FEF730BD281F69BuL,
             0x3C7007887AF0CBBDuL},
            {0x3FF0000000000000uL, 0xffffffffffffffffuL, 0x3FE921FB54442D18uL,
             0x3C81A62633145C07uL},
            {0x3FE0000000000000uL, 0xffffffffffffffffuL, 0x3FDDAC670561BB4FuL,
             0x3C7A2B7F222F65E2uL},
            {0x0000000000000000uL, 0xffffffffffffffffuL, 0x0000000000000000uL,
             0x0000000000000000uL},
            {0x0000000000000000uL, 0x0000000000000000uL, 0x0000000000000000uL,
             0x0000000000000000uL},
        },
        0x3FDC000000000000uL, // dATAN_BOUND1
        0x3FF3000000000000uL, // dATAN_BOUND3
        0x4003800000000000uL, // dATAN_BOUND4
        0x3FE6000000000000uL, // dATAN_BOUND2
        0x3FDDAC670561BB4FuL, // dAtan1o2Hi
        0x3C7A2B7F222F65E2uL, // dAtan1o2Lo
        0x3FE921FB54442D18uL, // dAtanOneHi
        0x3C81A62633145C07uL, // dAtanOneLo
        0x3FEF730BD281F69BuL, // dAtan3o2Hi
        0x3C7007887AF0CBBDuL, // dAtan3o2Lo
        0x3FF921FB54442D18uL, // dAtanInfHi
        0x3C91A62633145C07uL, // dAtanInfLo
        0x3FE0000000000000uL, // dHalf
        0x3FF8000000000000uL, // dThreeHalf
        0x8000000000000000uL, // dSIGN_MASK
        0x7FFFFFFFFFFFFFFFuL, // dABS_MASK
        0x400921FB54442D18uL, // dPiHI
        0x3CA1A64000000000uL, // dPiLO
        0xfffffffff8000000uL, // dHIGH_26_MASK
        0x3FDC0000u,          // iATAN_BOUND1
        0x3FF30000u,          // iATAN_BOUND3
        0x40038000u,          // iATAN_BOUND4
        0x3FE60000u,          // iATAN_BOUND2
        0x00000004u,          // i4
        0x3FF0000000000000uL, // dONE
        0x3F8BE4FBE6733718uL, // dA11
        0xbFA04CD71F92185EuL, // dA10
        0x3FA6AD5558FE19C9uL, // dA09
        0xbFAA9E755CA13D23uL, // dA08
        0x3FAE12F1EDF7C393uL, // dA07
        0xbFB1108D326C68EDuL, // dA06
        0x3FB3B132B731E73AuL, // dA05
        0xbFB745D119677A4FuL, // dA04
        0x3FBC71C719F99F96uL, // dA03
        0xbFC2492492441A21uL, // dA02
        0x3FC9999999998F43uL, // dA01
        0xbFD5555555555552uL, // dA00
        0x80300000u,          // iCHK_WORK_SUB
        0xfdd00000u,          // iCHK_WORK_CMP
};                            /*dAtan_Table*/
static const _iml_dp_union_t __datan_ha_CoutTab[242] = {
    0xE8000000, 0x3FC3D6EE, /* atan(B[  0])hi = +1.549967415631e-01 */
    0x8B0D1D86, 0x3DF8CC4D, /* atan(B[  0])lo = +3.608592409707e-10 */
    0x50000000, 0x3FCB90D7, /* atan(B[  1])hi = +2.153576985002e-01 */
    0x1022F622, 0x3E149305, /* atan(B[  1])lo = +1.197581645437e-09 */
    0x38000000, 0x3FD36277, /* atan(B[  2])hi = +3.028848692775e-01 */
    0x8658E951, 0xBE0F0286, /* atan(B[  2])lo = -9.025058588437e-10 */
    0xC0000000, 0x3FDA64EE, /* atan(B[  3])hi = +4.124104380608e-01 */
    0xE5B6427D, 0x3E2E611F, /* atan(B[  3])lo = +3.536626808853e-09 */
    0xA8000000, 0x3FE1E00B, /* atan(B[  4])hi = +5.585993081331e-01 */
    0x9F9B5C83, 0x3E3EF7F5, /* atan(B[  4])lo = +7.210437130796e-09 */
    0xC8000000, 0x3FE700A7, /* atan(B[  5])hi = +7.188300043344e-01 */
    0x618C34D2, 0xBE343DCE, /* atan(B[  5])lo = -4.712825262649e-09 */
    0x58000000, 0x3FECAC7C, /* atan(B[  6])hi = +8.960553854704e-01 */
    0x6F251EC5, 0xBE0EE418, /* atan(B[  6])lo = -8.990463636494e-10 */
    0x30000000, 0x3FF0D38F, /* atan(B[  7])hi = +1.051650226116e+00 */
    0x0B7A1B84, 0xBE4D22FB, /* atan(B[  7])lo = -1.356780675246e-08 */
    0x78000000, 0x3FF30B6D, /* atan(B[  8])hi = +1.190289944410e+00 */
    0x8589532C, 0x3E36A4DA, /* atan(B[  8])lo = +5.272207636248e-09 */
    0x00000000, 0x3FF4AE11, /* atan(B[  9])hi = +1.292496681213e+00 */
    0xDA7607D1, 0xBE4CD3B2, /* atan(B[  9])lo = -1.342359363835e-08 */
    0x18000000, 0x3FF5F973, /* atan(B[ 10])hi = +1.373400777578e+00 */
    0x4931151A, 0xBE46D5BD, /* atan(B[ 10])lo = -1.063333802097e-08 */
    0x78000000, 0x3FF6DCC5, /* atan(B[ 11])hi = +1.428899258375e+00 */
    0xE5AC6F37, 0x3E4DAB2F, /* atan(B[ 11])lo = +1.381556484974e-08 */
    0x30000000, 0x3FF789BD, /* atan(B[ 12])hi = +1.471127688885e+00 */
    0x63E8AA08, 0xBE4F4FFD, /* atan(B[ 12])lo = -1.458100051557e-08 */
    0x08000000, 0x3FF7FDE8, /* atan(B[ 13])hi = +1.499488860369e+00 */
    0xFFBFDCA2, 0x3E1C30A7, /* atan(B[ 13])lo = +1.640877655104e-09 */
    0x28000000, 0x3FF8555A, /* atan(B[ 14])hi = +1.520837932825e+00 */
    0x3B43DC6C, 0xBE1E19F8, /* atan(B[ 14])lo = -1.752134643155e-09 */
    0x18000000, 0x3FF88FC2, /* atan(B[ 15])hi = +1.535097211599e+00 */
    0x6FF107D4, 0x3E259D3B, /* atan(B[ 15])lo = +2.516222674259e-09 */
    0x60000000, 0x3FF8BB9A, /* atan(B[ 16])hi = +1.545801520348e+00 */
    0x27F43144, 0x3E4B8C7A, /* atan(B[ 16])lo = +1.282838124489e-08 */
    0xC0000000, 0x3FF8D8D8, /* atan(B[ 17])hi = +1.552941083908e+00 */
    0x2931C287, 0xBE2359D2, /* atan(B[ 17])lo = -2.252736949720e-09 */
    0xD0000000, 0x3FF8EEC8, /* atan(B[ 18])hi = +1.558296978474e+00 */
    0x607FD6F0, 0xBE07FCCD, /* atan(B[ 18])lo = -6.981283892985e-10 */
    0x48000000, 0x3FF8FD69, /* atan(B[ 19])hi = +1.561867982149e+00 */
    0x8310236B, 0x3E4679B5, /* atan(B[ 19])lo = +1.046593575868e-08 */
    0xD0000000, 0x3FF90861, /* atan(B[ 20])hi = +1.564546406269e+00 */
    0x9CFF61AF, 0x3E205B36, /* atan(B[ 20])lo = +1.904124170821e-09 */
    0x38000000, 0x3FF90FB2, /* atan(B[ 21])hi = +1.566332072020e+00 */
    0x3F24AB6A, 0xBE160576, /* atan(B[ 21])lo = -1.281810432599e-09 */
    0x88000000, 0x3FF9152E, /* atan(B[ 22])hi = +1.567671328783e+00 */
    0xBC43465D, 0x3E419361, /* atan(B[ 22])lo = +8.184327778349e-09 */
    0xC0000000, 0x3FF918D6, /* atan(B[ 23])hi = +1.568564176559e+00 */
    0x1114E411, 0x3E47CE4F, /* atan(B[ 23])lo = +1.108548322808e-08 */
    0xF0000000, 0x3FF91B94, /* atan(B[ 24])hi = +1.569233834743e+00 */
    0x773951DF, 0xBE3CAD7B, /* atan(B[ 24])lo = -6.677039244062e-09 */
    0x08000000, 0x3FF91D69, /* atan(B[ 25])hi = +1.569680243731e+00 */
    0xFAA44CE6, 0x3E49FB97, /* atan(B[ 25])lo = +1.209917839922e-08 */
    0x20000000, 0x3FF91EC8, /* atan(B[ 26])hi = +1.570015072823e+00 */
    0x8B23FFA5, 0x3E31BE61, /* atan(B[ 26])lo = +4.131271479643e-09 */
    0x30000000, 0x3FF91FB2, /* atan(B[ 27])hi = +1.570238292217e+00 */
    0x87FC4A01, 0xBE128841, /* atan(B[ 27])lo = -1.078718919694e-09 */
    0xB8000000, 0x3FF92061, /* atan(B[ 28])hi = +1.570405691862e+00 */
    0x29392773, 0x3E455F88, /* atan(B[ 28])lo = +9.952658509095e-09 */
    0xC0000000, 0x3FF920D6, /* atan(B[ 29])hi = +1.570517301559e+00 */
    0x6C4176AE, 0x3E3FB87E, /* atan(B[ 29])lo = +7.385546122670e-09 */
    0x88000000, 0x3FF9212E, /* atan(B[ 30])hi = +1.570601016283e+00 */
    0x76D7A426, 0xBE210E80, /* atan(B[ 30])lo = -1.985655132280e-09 */
    0x08000000, 0x3FF92169, /* atan(B[ 31])hi = +1.570656806231e+00 */
    0xD9D720C0, 0x3E48FD55, /* atan(B[ 31])lo = +1.163668522209e-08 */
    0xF0000000, 0x3FF92194, /* atan(B[ 32])hi = +1.570698678493e+00 */
    0xC02EEAC5, 0xBE41119E, /* atan(B[ 32])lo = -7.948292695772e-09 */
    0x30000000, 0x3FF921B2, /* atan(B[ 33])hi = +1.570726573467e+00 */
    0xB22521F8, 0xBE138683, /* atan(B[ 33])lo = -1.136530599303e-09 */
    0x20000000, 0x3FF921C8, /* atan(B[ 34])hi = +1.570747494698e+00 */
    0x3D6A380D, 0x3E310FA9, /* atan(B[ 34])lo = +3.972364623557e-09 */
    0xC0000000, 0x3FF921D6, /* atan(B[ 35])hi = +1.570761442184e+00 */
    0x5ADD7895, 0x3E3FB08C, /* atan(B[ 35])lo = +7.378319661704e-09 */
    0xB8000000, 0x3FF921E1, /* atan(B[ 36])hi = +1.570771902800e+00 */
    0xA42B3618, 0x3E45549C, /* atan(B[ 36])lo = +9.932795146628e-09 */
    0x08000000, 0x3FF921E9, /* atan(B[ 37])hi = +1.570778876543e+00 */
    0xB8C09601, 0x3E48FCD6, /* atan(B[ 37])lo = +1.163578191444e-08 */
    0x88000000, 0x3FF921EE, /* atan(B[ 38])hi = +1.570784121752e+00 */
    0x3964E770, 0xBE2113F6, /* atan(B[ 38])lo = -1.988138052759e-09 */
    0x30000000, 0x3FF921F2, /* atan(B[ 39])hi = +1.570787608624e+00 */
    0xD33BBF6E, 0xBE138702, /* atan(B[ 39])lo = -1.136643512761e-09 */
    0xF0000000, 0x3FF921F4, /* atan(B[ 40])hi = +1.570790231228e+00 */
    0x6E436174, 0xBE4111CA, /* atan(B[ 40])lo = -7.948603060837e-09 */
    0xC0000000, 0x3FF921F6, /* atan(B[ 41])hi = +1.570791959763e+00 */
    0x61D4C384, 0x3E3FB088, /* atan(B[ 41])lo = +7.378305547522e-09 */
    0x20000000, 0x3FF921F8, /* atan(B[ 42])hi = +1.570793271065e+00 */
    0x51E51998, 0x3E310F9E, /* atan(B[ 42])lo = +3.972325827924e-09 */
    0x08000000, 0x3FF921F9, /* atan(B[ 43])hi = +1.570794135332e+00 */
    0x79300AAF, 0x3E48FCD6, /* atan(B[ 43])lo = +1.163578015017e-08 */
    0xB8000000, 0x3FF921F9, /* atan(B[ 44])hi = +1.570794790983e+00 */
    0xF572E42D, 0x3E45549B, /* atan(B[ 44])lo = +9.932790297173e-09 */
    0x30000000, 0x3FF921FA, /* atan(B[ 45])hi = +1.570795238018e+00 */
    0x12CC4AC0, 0xBE138703, /* atan(B[ 45])lo = -1.136643733295e-09 */
    0x88000000, 0x3FF921FA, /* atan(B[ 46])hi = +1.570795565844e+00 */
    0x90C11066, 0xBE2113F6, /* atan(B[ 46])lo = -1.988138658941e-09 */
    0xC0000000, 0x3FF921FA, /* atan(B[ 47])hi = +1.570795774460e+00 */
    0x5FD83F2A, 0x3E3FB088, /* atan(B[ 47])lo = +7.378305519955e-09 */
    0xF0000000, 0x3FF921FA, /* atan(B[ 48])hi = +1.570795953274e+00 */
    0x70FE42BC, 0xBE4111CA, /* atan(B[ 48])lo = -7.948603136610e-09 */
    0x08000000, 0x3FF921FB, /* atan(B[ 49])hi = +1.570796042681e+00 */
    0x79104269, 0x3E48FCD6, /* atan(B[ 49])lo = +1.163578014672e-08 */
    0x20000000, 0x3FF921FB, /* atan(B[ 50])hi = +1.570796132088e+00 */
    0x51366146, 0x3E310F9E, /* atan(B[ 50])lo = +3.972325818452e-09 */
    0x30000000, 0x3FF921FB, /* atan(B[ 51])hi = +1.570796191692e+00 */
    0x12EC1305, 0xBE138703, /* atan(B[ 51])lo = -1.136643733725e-09 */
    0x38000000, 0x3FF921FB, /* atan(B[ 52])hi = +1.570796221495e+00 */
    0xF567F8A8, 0x3E45549B, /* atan(B[ 52])lo = +9.932790295989e-09 */
    0x40000000, 0x3FF921FB, /* atan(B[ 53])hi = +1.570796251297e+00 */
    0x5FD740E8, 0x3E3FB088, /* atan(B[ 53])lo = +7.378305519901e-09 */
    0x48000000, 0x3FF921FB, /* atan(B[ 54])hi = +1.570796281099e+00 */
    0x90C68629, 0xBE2113F6, /* atan(B[ 54])lo = -1.988138659089e-09 */
    0x48000000, 0x3FF921FB, /* atan(B[ 55])hi = +1.570796281099e+00 */
    0x79103285, 0x3E48FCD6, /* atan(B[ 55])lo = +1.163578014671e-08 */
    0x50000000, 0x3FF921FB, /* atan(B[ 56])hi = +1.570796310902e+00 */
    0x70FE6E6A, 0xBE4111CA, /* atan(B[ 56])lo = -7.948603136628e-09 */
    0x50000000, 0x3FF921FB, /* atan(B[ 57])hi = +1.570796310902e+00 */
    0x12EC22EA, 0xBE138703, /* atan(B[ 57])lo = -1.136643733726e-09 */
    0x50000000, 0x3FF921FB, /* atan(B[ 58])hi = +1.570796310902e+00 */
    0x5136565A, 0x3E310F9E, /* atan(B[ 58])lo = +3.972325818450e-09 */
    0x50000000, 0x3FF921FB, /* atan(B[ 59])hi = +1.570796310902e+00 */
    0x5FD74068, 0x3E3FB088, /* atan(B[ 59])lo = +7.378305519901e-09 */
    0x50000000, 0x3FF921FB, /* atan(B[ 60])hi = +1.570796310902e+00 */
    0xF567F7FA, 0x3E45549B, /* atan(B[ 60])lo = +9.932790295989e-09 */
    0x50000000, 0x3FF921FB, /* atan(B[ 61])hi = +1.570796310902e+00 */
    0x7910327D, 0x3E48FCD6, /* atan(B[ 61])lo = +1.163578014671e-08 */
    0x50000000, 0x3FF921FB, /* atan(B[ 62])hi = +1.570796310902e+00 */
    0x5BCE5E60, 0x3E4BBB02, /* atan(B[ 62])lo = +1.291302253476e-08 */
    0x50000000, 0x3FF921FB, /* atan(B[ 63])hi = +1.570796310902e+00 */
    0x9DA27BA2, 0x3E4D8F1F, /* atan(B[ 63])lo = +1.376451746012e-08 */
    0x50000000, 0x3FF921FB, /* atan(B[ 64])hi = +1.570796310902e+00 */
    0x8F019193, 0x3E4EEE35, /* atan(B[ 64])lo = +1.440313865414e-08 */
    0x50000000, 0x3FF921FB, /* atan(B[ 65])hi = +1.570796310902e+00 */
    0x2FEBA034, 0x3E4FD844, /* atan(B[ 65])lo = +1.482888611682e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 66])hi = +1.570796340704e+00 */
    0xD764D4D3, 0xBE4F7830, /* atan(B[ 66])lo = -1.465412567386e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 67])hi = +1.570796340704e+00 */
    0x86EFCD83, 0xBE4F0329, /* atan(B[ 67])lo = -1.444125194252e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 68])hi = +1.570796340704e+00 */
    0x0A980806, 0xBE4EAB64, /* atan(B[ 68])lo = -1.428159664401e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 69])hi = +1.570796340704e+00 */
    0x625D845E, 0xBE4E70E0, /* atan(B[ 69])lo = -1.417515977834e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 70])hi = +1.570796340704e+00 */
    0xA431A1A0, 0xBE4E44FD, /* atan(B[ 70])lo = -1.409533212909e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 71])hi = +1.570796340704e+00 */
    0xD0145FCC, 0xBE4E27BB, /* atan(B[ 71])lo = -1.404211369626e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 72])hi = +1.570796340704e+00 */
    0x70FE6E6D, 0xBE4E11CA, /* atan(B[ 72])lo = -1.400219987163e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 73])hi = +1.570796340704e+00 */
    0x86EFCD83, 0xBE4E0329, /* atan(B[ 73])lo = -1.397559065521e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 74])hi = +1.570796340704e+00 */
    0xD764D4D3, 0xBE4DF830, /* atan(B[ 74])lo = -1.395563374290e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 75])hi = +1.570796340704e+00 */
    0x625D845E, 0xBE4DF0E0, /* atan(B[ 75])lo = -1.394232913469e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 76])hi = +1.570796340704e+00 */
    0x0A980806, 0xBE4DEB64, /* atan(B[ 76])lo = -1.393235067853e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 77])hi = +1.570796340704e+00 */
    0xD0145FCC, 0xBE4DE7BB, /* atan(B[ 77])lo = -1.392569837443e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 78])hi = +1.570796340704e+00 */
    0xA431A1A0, 0xBE4DE4FD, /* atan(B[ 78])lo = -1.392070914635e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 79])hi = +1.570796340704e+00 */
    0x86EFCD83, 0xBE4DE329, /* atan(B[ 79])lo = -1.391738299430e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 80])hi = +1.570796340704e+00 */
    0x70FE6E6D, 0xBE4DE1CA, /* atan(B[ 80])lo = -1.391488838026e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 81])hi = +1.570796340704e+00 */
    0x625D845E, 0xBE4DE0E0, /* atan(B[ 81])lo = -1.391322530423e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 82])hi = +1.570796340704e+00 */
    0xD764D4D3, 0xBE4DE030, /* atan(B[ 82])lo = -1.391197799721e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 83])hi = +1.570796340704e+00 */
    0xD0145FCC, 0xBE4DDFBB, /* atan(B[ 83])lo = -1.391114645920e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 84])hi = +1.570796340704e+00 */
    0x0A980806, 0xBE4DDF64, /* atan(B[ 84])lo = -1.391052280569e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 85])hi = +1.570796340704e+00 */
    0x86EFCD83, 0xBE4DDF29, /* atan(B[ 85])lo = -1.391010703668e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 86])hi = +1.570796340704e+00 */
    0xA431A1A0, 0xBE4DDEFD, /* atan(B[ 86])lo = -1.390979520993e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 87])hi = +1.570796340704e+00 */
    0x625D845E, 0xBE4DDEE0, /* atan(B[ 87])lo = -1.390958732543e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 88])hi = +1.570796340704e+00 */
    0x70FE6E6D, 0xBE4DDECA, /* atan(B[ 88])lo = -1.390943141205e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 89])hi = +1.570796340704e+00 */
    0xD0145FCC, 0xBE4DDEBB, /* atan(B[ 89])lo = -1.390932746980e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 90])hi = +1.570796340704e+00 */
    0xD764D4D3, 0xBE4DDEB0, /* atan(B[ 90])lo = -1.390924951311e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 91])hi = +1.570796340704e+00 */
    0x86EFCD83, 0xBE4DDEA9, /* atan(B[ 91])lo = -1.390919754198e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 92])hi = +1.570796340704e+00 */
    0x0A980806, 0xBE4DDEA4, /* atan(B[ 92])lo = -1.390915856364e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 93])hi = +1.570796340704e+00 */
    0x625D845E, 0xBE4DDEA0, /* atan(B[ 93])lo = -1.390913257807e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 94])hi = +1.570796340704e+00 */
    0xA431A1A0, 0xBE4DDE9D, /* atan(B[ 94])lo = -1.390911308890e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 95])hi = +1.570796340704e+00 */
    0xD0145FCC, 0xBE4DDE9B, /* atan(B[ 95])lo = -1.390910009612e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 96])hi = +1.570796340704e+00 */
    0x70FE6E6D, 0xBE4DDE9A, /* atan(B[ 96])lo = -1.390909035153e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 97])hi = +1.570796340704e+00 */
    0x86EFCD83, 0xBE4DDE99, /* atan(B[ 97])lo = -1.390908385514e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 98])hi = +1.570796340704e+00 */
    0xD764D4D3, 0xBE4DDE98, /* atan(B[ 98])lo = -1.390907898285e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[ 99])hi = +1.570796340704e+00 */
    0x625D845E, 0xBE4DDE98, /* atan(B[ 99])lo = -1.390907573466e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[100])hi = +1.570796340704e+00 */
    0x0A980806, 0xBE4DDE98, /* atan(B[100])lo = -1.390907329851e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[101])hi = +1.570796340704e+00 */
    0xD0145FCC, 0xBE4DDE97, /* atan(B[101])lo = -1.390907167441e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[102])hi = +1.570796340704e+00 */
    0xA431A1A0, 0xBE4DDE97, /* atan(B[102])lo = -1.390907045634e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[103])hi = +1.570796340704e+00 */
    0x86EFCD83, 0xBE4DDE97, /* atan(B[103])lo = -1.390906964429e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[104])hi = +1.570796340704e+00 */
    0x70FE6E6D, 0xBE4DDE97, /* atan(B[104])lo = -1.390906903525e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[105])hi = +1.570796340704e+00 */
    0x625D845E, 0xBE4DDE97, /* atan(B[105])lo = -1.390906862923e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[106])hi = +1.570796340704e+00 */
    0x5764D4D3, 0xBE4DDE97, /* atan(B[106])lo = -1.390906832471e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[107])hi = +1.570796340704e+00 */
    0x50145FCC, 0xBE4DDE97, /* atan(B[107])lo = -1.390906812170e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[108])hi = +1.570796340704e+00 */
    0x4A980806, 0xBE4DDE97, /* atan(B[108])lo = -1.390906796944e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[109])hi = +1.570796340704e+00 */
    0x46EFCD83, 0xBE4DDE97, /* atan(B[109])lo = -1.390906786793e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[110])hi = +1.570796340704e+00 */
    0x4431A1A0, 0xBE4DDE97, /* atan(B[110])lo = -1.390906779180e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[111])hi = +1.570796340704e+00 */
    0x425D845E, 0xBE4DDE97, /* atan(B[111])lo = -1.390906774105e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[112])hi = +1.570796340704e+00 */
    0x40FE6E6D, 0xBE4DDE97, /* atan(B[112])lo = -1.390906770298e-08 */
    0x58000000, 0x3FF921FB, /* atan(B[113])hi = +1.570796340704e+00 */
    0x40145FCC, 0xBE4DDE97, /* atan(B[113])lo = -1.390906767761e-08 */
    /* Coefficients of AtanPoly */
    0x55555555, 0xBFD55555, /* A00 = -3.3333333333333330e-01 */
    0x999991BB, 0x3FC99999, /* A01 = +1.9999999999994408e-01 */
    0x92382A27, 0xBFC24924, /* A02 = -1.4285714282625968e-01 */
    0xF9B157D3, 0x3FBC71C6, /* A03 = +1.1111110301985087e-01 */
    0x69CCA475, 0xBFB745BE, /* A04 = -9.0907955961158590e-02 */
    0x1048AFD1, 0x3FB3AB7C, /* A05 = +7.6835397697038190e-02 */
    0x34239994, 0xBFB029BD, /* A06 = -6.3136887768996455e-02 */
    /* Other constants */
    0x00000000, 0x3CA00000, /* BOUND0 = 2^(-53) */
    0x00000000, 0x3FC00000, /* BOUND1 = 2^(-3) */
    0x00000000, 0x43500000, /* BOUND2 = 2^(54) */
    0x00000000, 0x3FF00000, /* ONE = 1.0 */
    0x54442D18, 0x3FF921FB, /* PIO2HI = high part of Pi/2 */
    0x33145C07, 0x3C91A626, /* PIO2LO = low part of Pi/2 */
    /* Constant used to obtain high bits */
    0x02000000, 0x41A00000, /* T27 = 2^27+1 */
};
inline int __devicelib_imf_internal_datan(const double *a, double *r) {
  int nRet = 0;
  double dbX, dbB, dbU, dbTmp1, dbTmp2, dbUHi, dbULo, dbXHi, dbXLo, dbXB1Hi,
      dbXB1Lo, dbXBHi, dbXBLo, dbV1Hi, dbV2Lo, dbV1Lo, dbVHi, dbV3Lo, dbVLo,
      dbQHi, dbQLo, dbT1Hi, dbT1Lo, dbTHi, dbTLo, dbS, dbAtanPoly, dbAHi, dbALo,
      dbResHi, dbResLo, dbRes;
  double dbVTmp1, dbVTmp2, dbVTmp3;
  int i, iSign, iJ;
  /* Filter out INFs and NaNs */
  if (((((_iml_dp_union_t *)&(*a))->bits.exponent) != 0x7FF)) {
    /* Here if argument is finite double precision number */
    /* Get sign of argument */
    iSign = (((_iml_dp_union_t *)&(*a))->bits.sign);
    /* x = |(*a)| */
    dbX = (*a);
    (((_iml_dp_union_t *)&dbX)->bits.sign = 0);
    if (dbX >= ((const double *)__datan_ha_CoutTab)[236]) {
      /* Here if BOUND1 <= x < Inf */
      if (dbX < ((const double *)__datan_ha_CoutTab)[237]) {
        /* 5) "Main" path: Here if BOUND1 <= x < BOUND2 */
        /* 5.a) atan() argument reduction */
        /* 5.a.1) Getting index j */
        iJ = (((_iml_dp_union_t *)&(*a))->bits.exponent) << 20;
        iJ = iJ | (((_iml_dp_union_t *)&(*a))->bits.hi_significand);
        iJ = iJ - 0x3FC00000;
        iJ = iJ >> 19;
        /* 5.a.2) Getting base point b */
        dbB = dbX;
        (((_iml_dp_union_t *)&dbB)->bits.lo_significand = (0));
        (((_iml_dp_union_t *)&dbB)->dwords.hi_dword =
             (((_iml_dp_union_t *)&dbB)->dwords.hi_dword & 0xFFF00000) |
             ((((((_iml_dp_union_t *)&dbB)->bits.hi_significand) & 0x80000) |
               0x40000) &
              0x000FFFFF));
        /* 5.a.3) Getting t in multiprecision */
        /* 5.a.3.1) Obtain u = x - b as sum UHi+ULo */
        dbU = dbX - dbB;
        dbVTmp1 = ((dbU) * (((const double *)__datan_ha_CoutTab)[241]));
        dbVTmp2 = (dbVTmp1 - (dbU));
        dbVTmp1 = (dbVTmp1 - dbVTmp2);
        dbVTmp2 = ((dbU)-dbVTmp1);
        dbUHi = dbVTmp1;
        dbULo = dbVTmp2;
        /* 5.a.3.2) Obtain v = 1 + x*b as sum VHi+VLo */
        /* Obtain x*b as sum XB1Hi+XB1Lo */
        dbVTmp1 = ((dbX) * (((const double *)__datan_ha_CoutTab)[241]));
        dbVTmp2 = (dbVTmp1 - (dbX));
        dbVTmp1 = (dbVTmp1 - dbVTmp2);
        dbVTmp2 = ((dbX)-dbVTmp1);
        dbXHi = dbVTmp1;
        dbXLo = dbVTmp2;
        dbXB1Hi = dbXHi * dbB;
        dbXB1Lo = dbXLo * dbB;
        /* Rebreak XB1Hi+XB1Lo into XBHi+XBLo */
        dbVTmp1 = ((dbXB1Hi) + (dbXB1Lo));
        dbTmp1 = ((dbXB1Hi)-dbVTmp1);
        dbVTmp2 = (dbTmp1 + (dbXB1Lo));
        dbXBHi = dbVTmp1;
        dbXBLo = dbVTmp2;
        /* Obtain v as sum V1Hi+V1Lo */
        dbVTmp1 = ((((const double *)__datan_ha_CoutTab)[238]) + (dbXBHi));
        dbVTmp2 = ((((const double *)__datan_ha_CoutTab)[238]) - dbVTmp1);
        dbVTmp3 = (dbVTmp1 + dbVTmp2);
        dbVTmp2 = ((dbXBHi) + dbVTmp2);
        dbVTmp3 = ((((const double *)__datan_ha_CoutTab)[238]) - dbVTmp3);
        dbVTmp3 = (dbVTmp2 + dbVTmp3);
        dbV1Hi = dbVTmp1;
        dbV2Lo = dbVTmp3;
        dbV1Lo = dbV2Lo + dbXBLo;
        /* Rebreak V1Hi+V1Lo into VHi+VLo */
        dbVTmp1 = ((dbV1Hi) * (((const double *)__datan_ha_CoutTab)[241]));
        dbVTmp2 = (dbVTmp1 - (dbV1Hi));
        dbVTmp1 = (dbVTmp1 - dbVTmp2);
        dbVTmp2 = ((dbV1Hi)-dbVTmp1);
        dbVHi = dbVTmp1;
        dbV3Lo = dbVTmp2;
        dbVLo = dbV3Lo + dbV1Lo;
        /* 5.a.3.3) Obtain q = 1 / v as sum QHi+QLo */
        dbTmp1 = (((const double *)__datan_ha_CoutTab)[238] / dbVHi);
        dbVTmp2 = (dbTmp1 * ((const double *)__datan_ha_CoutTab)[241]);
        dbVTmp3 = (dbVTmp2 - dbTmp1);
        dbVTmp3 = (dbVTmp2 - dbVTmp3);
        dbTmp1 = (dbVHi * dbVTmp3);
        dbTmp1 = (((const double *)__datan_ha_CoutTab)[238] - dbTmp1);
        dbVTmp2 = (dbVLo * dbVTmp3);
        dbVTmp2 = (dbTmp1 - dbVTmp2);
        dbTmp1 = (((const double *)__datan_ha_CoutTab)[238] + dbVTmp2);
        dbQHi = dbVTmp3;
        dbTmp1 = (dbTmp1 * dbVTmp2);
        dbQLo = (dbTmp1 * dbVTmp3);
        /* 5.a.3.4) Obtain t = u * q as sum THi+TLo */
        /* Represent t as sum T1Hi+T1Lo */
        dbTmp1 = ((dbQHi) * (dbUHi));
        dbTmp2 = ((dbQLo) * (dbULo));
        dbTmp2 = (dbTmp2 + (dbQHi) * (dbULo));
        dbVTmp1 = (dbTmp2 + (dbQLo) * (dbUHi));
        dbT1Hi = dbTmp1;
        dbT1Lo = dbVTmp1;
        /* Rebreak T1Hi+T1Lo into THi+TLo */
        dbVTmp1 = ((dbT1Hi) + (dbT1Lo));
        dbTmp1 = ((dbT1Hi)-dbVTmp1);
        dbVTmp2 = (dbTmp1 + (dbT1Lo));
        dbTHi = dbVTmp1;
        dbTLo = dbVTmp2;
        /* 5.b) atan() approximation */
        dbS = dbTHi * dbTHi;
        dbAtanPoly = ((((((((const double *)__datan_ha_CoutTab)[234]) * dbS +
                          ((const double *)__datan_ha_CoutTab)[233]) *
                             dbS +
                         ((const double *)__datan_ha_CoutTab)[232]) *
                            dbS +
                        ((const double *)__datan_ha_CoutTab)[231]) *
                           dbS +
                       ((const double *)__datan_ha_CoutTab)[230]) *
                          dbS +
                      ((const double *)__datan_ha_CoutTab)[229]) *
                         dbS +
                     ((const double *)__datan_ha_CoutTab)[228];
        /* 5.c) atan() reconstruction */
        /* Rebreak ATAN_B_HI[j]+THi into AHi+ALo */
        dbVTmp1 = ((((const double *)__datan_ha_CoutTab)[0 + 2 * (iJ) + 0]) +
                   (dbTHi));
        dbTmp1 = ((((const double *)__datan_ha_CoutTab)[0 + 2 * (iJ) + 0]) -
                  dbVTmp1);
        dbVTmp2 = (dbTmp1 + (dbTHi));
        dbAHi = dbVTmp1;
        dbALo = dbVTmp2;
        /* ResHi := AHi */
        dbResHi = dbAHi;
        /* ResLo := ((TLo + THi*S*AtanPoly) + ATAN_B_LO[j]) + ALo */
        dbTmp1 = dbAtanPoly * dbS;
        dbTmp1 = dbTmp1 * dbTHi;
        dbTmp1 = dbTmp1 + dbTLo;
        dbTmp1 =
            dbTmp1 + ((const double *)__datan_ha_CoutTab)[0 + 2 * (iJ) + 1];
        dbResLo = dbTmp1 + dbALo;
        /* (*r) := Sign * (ResHi + ResLo) */
        dbRes = dbResHi + dbResLo;
        (((_iml_dp_union_t *)&dbRes)->bits.sign = iSign);
        (*r) = dbRes;
      } else {
        /* 4) "Large arguments" path */
        /* Here if BOUND2 <= x < Inf */
        /* Res := Pi/2 */
        dbRes = ((const double *)__datan_ha_CoutTab)[239] +
                ((const double *)__datan_ha_CoutTab)[240];
        /* (*r) := Sign * Res */
        (((_iml_dp_union_t *)&dbRes)->bits.sign = iSign);
        (*r) = dbRes;
      }
    } else {
      /* 3) "Near 0" path. Here if 0 <= x < BOUND1 */
      if (dbX >= ((const double *)__datan_ha_CoutTab)[235]) {
        /* Path 3.3) Here if BOUND0 <= x < BOUND1 */
        /* atan() approximation */
        dbS = dbX * dbX;
        dbAtanPoly = ((((((((const double *)__datan_ha_CoutTab)[234]) * dbS +
                          ((const double *)__datan_ha_CoutTab)[233]) *
                             dbS +
                         ((const double *)__datan_ha_CoutTab)[232]) *
                            dbS +
                        ((const double *)__datan_ha_CoutTab)[231]) *
                           dbS +
                       ((const double *)__datan_ha_CoutTab)[230]) *
                          dbS +
                      ((const double *)__datan_ha_CoutTab)[229]) *
                         dbS +
                     ((const double *)__datan_ha_CoutTab)[228];
        /* atan() reconstruction */
        dbRes = dbAtanPoly * dbS;
        dbRes = dbRes * dbX;
        dbRes = dbRes + dbX;
        /* (*r) := Sign * Res */
        (((_iml_dp_union_t *)&dbRes)->bits.sign = iSign);
        (*r) = dbRes;
      } else {
        /* Here if 0 <= x < BOUND0 */
        if ((((_iml_dp_union_t *)&dbX)->bits.exponent) != 0) {
          /* Path 3.2) Here if MIN_NORMAL <= x < BOUND0 */
          /* Tmp := 1 + x */
          dbVTmp1 = ((const double *)__datan_ha_CoutTab)[238] + dbX;
          /* Res := Tmp * x */
          dbRes = dbVTmp1 * dbX;
          /* (*r) := Sign * Res */
          (((_iml_dp_union_t *)&dbRes)->bits.sign = iSign);
          (*r) = dbRes;
        } else {
          /* Path 3.2) Here if x is zero or denormalized */
          /* number in target precision                  */
          /* Res := x + x * x */
          dbVTmp1 = dbX * dbX;
          dbRes = dbX + dbVTmp1;
          /* (*r) := Sign * Res */
          (((_iml_dp_union_t *)&dbRes)->bits.sign = iSign);
          (*r) = dbRes;
        }
      }
    }
  } else {
    if ((((((_iml_dp_union_t *)&(*a))->bits.hi_significand) == 0) &&
         ((((_iml_dp_union_t *)&(*a))->bits.lo_significand) == 0))) {
      /* Path 2). Here if argument is Infinity */
      /* (*r) := Sign * Pi/2 */
      dbRes = ((const double *)__datan_ha_CoutTab)[239] +
              ((const double *)__datan_ha_CoutTab)[240];
      iSign = (((_iml_dp_union_t *)&(*a))->bits.sign);
      (((_iml_dp_union_t *)&dbRes)->bits.sign = iSign);
      (*r) = dbRes;
    } else {
      /* Path 1). Here if argument is NaN */
      (*r) = (*a) + (*a);
    }
  }
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_atan_d_ha */
DEVICE_EXTERN_C_INLINE double __devicelib_imf_atan(double x) {
  using namespace __imf_impl_atan_d_ha;
  double r;
  VUINT32 vm;
  double va1;
  double vr1;
  va1 = x;
  {
    double dSignY;
    double dAY;
    double dYHi;
    double dYLo;
    VUINT64 lAY;
    VUINT32 iAY;
    VUINT32 iAY1;
    VUINT32 iYBad;
    VUINT32 iB1AXLessAY1;
    VUINT32 iB2AXLessAY1;
    VUINT32 iB3AXLessAY1;
    VUINT32 iB4AXLessAY1;
    VUINT32 iB1AXLessAY;
    VUINT32 iB2AXLessAY;
    VUINT32 iB3AXLessAY;
    VUINT32 iB4AXLessAY;
    VUINT32 iIndex;
    double dBase;
    double dAtanBaseHi;
    double dAtanBaseLo;
    double dC;
    double dA1Hi;
    double dA1Lo;
    double dA2Hi;
    double dA2Lo;
    double dA2HiLo;
    double dAHi;
    double dALo;
    double dALoHi;
    double dCXHi;
    double dCYHi;
    double dB1Hi;
    double dB1Lo;
    double dB2Hi;
    double dB2Lo;
    double dB2HiLo;
    double dMB1LoHi;
    double dBHi;
    double dBLo;
    double dEHi;
    double dE;
    double dInv;
    double dInvLo;
    double dT0;
    double dT1;
    double dRLo;
    double dR;
    double dR0;
    double dR2;
    double dR4;
    double dP1;
    double dP2;
    double dP3;
    double dSIGN_MASK;
    double dABS_MASK;
    double dHIGH_26_MASK;
    VUINT32 iATAN_BOUND1;
    VUINT32 iATAN_BOUND3;
    VUINT32 iATAN_BOUND4;
    VUINT32 iATAN_BOUND2;
    VUINT32 i4;
    double dONE;
    double dPC[12];
    VUINT32 iCHK_WORK_SUB;
    VUINT32 iCHK_WORK_CMP;
    // Argument absolute values
    dABS_MASK = as_double(__devicelib_imf_internal_datan_data._dABS_MASK);
    dAY = as_double((as_ulong(va1) & as_ulong(dABS_MASK)));
    // Argument signs
    dSignY = as_double((as_ulong(va1) ^ as_ulong(dAY)));
    // Check if y is on main path.
    lAY = as_ulong(dAY);
    iAY = ((VUINT32)((VUINT64)lAY >> 32));
    iCHK_WORK_SUB = (__devicelib_imf_internal_datan_data._iCHK_WORK_SUB);
    iAY1 = (iAY - iCHK_WORK_SUB);
    iCHK_WORK_CMP = (__devicelib_imf_internal_datan_data._iCHK_WORK_CMP);
    iYBad = ((VUINT32)(-(VSINT32)((VSINT32)iAY1 >= (VSINT32)iCHK_WORK_CMP)));
    vm = 0;
    vm = iYBad;
    // Determining table index (to which interval is subject y/x)
    // x = 1
    dONE = as_double(__devicelib_imf_internal_datan_data._dONE);
    iATAN_BOUND1 = (__devicelib_imf_internal_datan_data._iATAN_BOUND1);
    iATAN_BOUND3 = (__devicelib_imf_internal_datan_data._iATAN_BOUND3);
    iATAN_BOUND4 = (__devicelib_imf_internal_datan_data._iATAN_BOUND4);
    iATAN_BOUND2 = (__devicelib_imf_internal_datan_data._iATAN_BOUND2);
    iB1AXLessAY1 = (iATAN_BOUND1 - iAY);
    iB2AXLessAY1 = (iATAN_BOUND2 - iAY);
    iB3AXLessAY1 = (iATAN_BOUND3 - iAY);
    iB4AXLessAY1 = (iATAN_BOUND4 - iAY);
    iB1AXLessAY = ((VSINT32)iB1AXLessAY1 >> (31));
    iB2AXLessAY = ((VSINT32)iB2AXLessAY1 >> (31));
    iB3AXLessAY = ((VSINT32)iB3AXLessAY1 >> (31));
    iB4AXLessAY = ((VSINT32)iB4AXLessAY1 >> (31));
    iB1AXLessAY = (iB1AXLessAY + iB2AXLessAY);
    iB3AXLessAY = (iB3AXLessAY + iB4AXLessAY);
    iIndex = (iB1AXLessAY + iB3AXLessAY);
    i4 = (__devicelib_imf_internal_datan_data._i4);
    iIndex = (iIndex + i4);
    // load table values
    dBase = as_double(
        ((const VUINT64 *)(__devicelib_imf_internal_datan_data
                               ._ATAN_TBL))[(((0 + iIndex) * (4 * 8)) >> (3)) +
                                            0]);
    dC = as_double(
        ((const VUINT64 *)(__devicelib_imf_internal_datan_data
                               ._ATAN_TBL))[(((0 + iIndex) * (4 * 8)) >> (3)) +
                                            1]);
    dAtanBaseHi = as_double(
        ((const VUINT64 *)(__devicelib_imf_internal_datan_data
                               ._ATAN_TBL))[(((0 + iIndex) * (4 * 8)) >> (3)) +
                                            2]);
    dAtanBaseLo = as_double(
        ((const VUINT64 *)(__devicelib_imf_internal_datan_data
                               ._ATAN_TBL))[(((0 + iIndex) * (4 * 8)) >> (3)) +
                                            3]);
    // Reduction of |y/x| to the interval [0;7/16].
    // 0. If 39/16 < |y/x| <  Inf  then a=0*y-  1*x, b=0*x+  1*y,
    // AtanHi+AtanLo=atan(Inf).
    // 1. If 19/16 < |y/x| < 39/16 then a=1*y-1.5*x, b=1*x+1.5*y,
    // AtanHi+AtanLo=atan(3/2).
    // 2. If 11/16 < |y/x| < 19/16 then a=1*y-1.0*x, b=1*x+1.0*y,
    // AtanHi+AtanLo=atan( 1 ).
    // 3. If  7/16 < |y/x| < 11/16 then a=1*y-0.5*x, b=1*x+0.5*y,
    // AtanHi+AtanLo=atan(1/2).
    // 4. If   0   < |y/x| <  7/16 then a=1*y-0  *x, b=1*x+0  *y,
    // AtanHi+AtanLo=atan( 0 ). Hence common formulas are:       a=c*y-  d*x,
    // b=c*x+  d*y (c is mask in our case) (b is always positive)
    dCYHi = as_double((as_ulong(dC) & as_ulong(dAY)));
    dAHi = (dCYHi - dBase);
    dCXHi = as_double((as_ulong(dC) & as_ulong(dONE)));
    dBHi = __fma(dBase, dAY, dCXHi);
    // Divide r:=a*(1/b),    where a==AHi+ALo, b==BHi+BLo, 1/b~=InvHi+InvLo
    // Get r0~=1/BHi
    dR0 = ((double)(1.0f / ((float)(dBHi))));
    // Now refine r0 by several iterations (hidden in polynomial)
    // e = 1-Bhi*r0
    dE = __fma(-(dBHi), dR0, dONE);
    // dR0 ~= 1/B*(1-e)(e*e+e+1) = 1/B(1-e^3)
    dE = __fma(dE, dE, dE);
    dR0 = __fma(dR0, dE, dR0);
    // e' = 1-Bhi*r0 = e^3
    dE = __fma(-(dBHi), dR0, dONE);
    // dR0 ~= 1/B*(1-e')(1+e') = 1/B(1-e'^2)
    dR0 = __fma(dR0, dE, dR0);
    // Now 1/B ~= R0 + InvLo
    // Get r:=a*(1/b),    where a==AHi+ALo, 1/b~=InvHi(R0)+InvLo
    dR = (dR0 * dAHi);
    dInvLo = __fma(-(dBHi), dR, dAHi);
    dRLo = (dInvLo * dR0);
    // Atan polynomial approximation
    dR2 = (dR * dR);
    dR4 = (dR2 * dR2);
    dPC[11] = as_double(__devicelib_imf_internal_datan_data._dPC11);
    dPC[10] = as_double(__devicelib_imf_internal_datan_data._dPC10);
    dPC[9] = as_double(__devicelib_imf_internal_datan_data._dPC9);
    dPC[8] = as_double(__devicelib_imf_internal_datan_data._dPC8);
    dP1 = __fma(dPC[11], dR4, dPC[9]);
    dP2 = __fma(dPC[10], dR4, dPC[8]);
    dPC[7] = as_double(__devicelib_imf_internal_datan_data._dPC7);
    dPC[6] = as_double(__devicelib_imf_internal_datan_data._dPC6);
    dP1 = __fma(dP1, dR4, dPC[7]);
    dP2 = __fma(dP2, dR4, dPC[6]);
    dPC[5] = as_double(__devicelib_imf_internal_datan_data._dPC5);
    dPC[4] = as_double(__devicelib_imf_internal_datan_data._dPC4);
    dP1 = __fma(dP1, dR4, dPC[5]);
    dP2 = __fma(dP2, dR4, dPC[4]);
    dPC[3] = as_double(__devicelib_imf_internal_datan_data._dPC3);
    dPC[2] = as_double(__devicelib_imf_internal_datan_data._dPC2);
    dP1 = __fma(dP1, dR4, dPC[3]);
    dP2 = __fma(dP2, dR4, dPC[2]);
    dPC[1] = as_double(__devicelib_imf_internal_datan_data._dPC1);
    dPC[0] = as_double(__devicelib_imf_internal_datan_data._dPC0);
    dP1 = __fma(dP1, dR4, dPC[1]);
    dP2 = __fma(dP2, dR4, dPC[0]);
    dP1 = __fma(dP1, dR2, dP2);
    dP1 = (dP1 * dR2);
    //  Res = ( (RLo + AtanBaseLo + PiOrZero*sx + Poly(R^2)*R^3 + RHi +
    //  AtanBaseHi) * sx + PiOrZeroHi) * sy Res = (RLo + AtanBaseLo +
    //  Poly(R^2)*R^3 + RHi + AtanBaseHi) * sy
    dRLo = (dRLo + dAtanBaseLo);
    dP1 = __fma(dP1, dR, dRLo); // P1 = P1 * R + RLo
    dP1 = (dR + dP1);
    dP1 = (dP1 + dAtanBaseHi);
    vr1 = as_double((as_ulong(dP1) | as_ulong(dSignY)));
  }
  if (__builtin_expect((vm) != 0, 0)) {
    double __cout_a1;
    double __cout_r1;
    ((double *)&__cout_a1)[0] = va1;
    ((double *)&__cout_r1)[0] = vr1;
    __devicelib_imf_internal_datan(&__cout_a1, &__cout_r1);
    vr1 = ((const double *)&__cout_r1)[0];
  }
  r = vr1;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
