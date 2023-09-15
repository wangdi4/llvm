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
#include "_imf_include_fp32.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_nextafter_s_xa {
namespace {
typedef struct {
  VUINT32 _iAbsMask;
  VUINT32 _iSubConst;
  VUINT32 _iCmpConst;
  VUINT32 _iPOne;
  VUINT32 _iMOne;
} __devicelib_imf_internal_snextafter_data_t;
static const __devicelib_imf_internal_snextafter_data_t
    __devicelib_imf_internal_snextafter_data = {
        0x7fffffffu, 0x80800000u, 0xfeffffffu, 0x00000001u, 0xffffffffu};
static const unsigned _min_subnormal_32[] = {0x00000001, 0x80000001};
static const unsigned int ione[2] = {0x3f800000, 0xbf800000};
static const unsigned int _large_value_32[] = {0x71800000,
                                               0xf1800000}; /* +2^100,-2^100 */
static const unsigned int _small_value_32[] = {
    0x0d800000, 0x8d800000}; /* +2^(-100),-2^(-100) */
inline int __devicelib_imf_internal_snextafter(const float *a1, const float *a2,
                                               float *r1) {
  int nRet = 0;
  float service;
  float x, y, fres;
  unsigned int ures;
  const float *fone = (const float *)ione;
/*
//   
//    * Flush denormals, if needed
//    * Raise invalid for SNaN
//    
*/
  x = (*a1 * fone[0]);
  y = (*a2 * fone[0]);
  if ((((((_iml_sp_union_t *)&x)->bits.exponent) == 0x00FF) &&
       ((((_iml_sp_union_t *)&x)->bits.significand) != 0)) ||
      (((((_iml_sp_union_t *)&y)->bits.exponent) == 0x00FF) &&
       ((((_iml_sp_union_t *)&y)->bits.significand) != 0))) {
    *r1 = x + y;
    return nRet;
  }
  if (x == y) {
    *r1 = y;
    return nRet;
  }
  if ((((*(int *)&x) & ~0x80000000) == 0)) {
    nRet = 4;
    (*(int *)&*r1) = (*(const int *)&((
        const float *)_min_subnormal_32)[(((_iml_sp_union_t *)&y)->bits.sign)]);
    service = (((const float *)_small_value_32)[(0)] *
               ((const float *)_small_value_32)[0]);
  } else {
    (*(int *)&fres) = (*(int *)&x);
    if ((((*(int *)&x) ^ (*(int *)&y)) & 0x80000000) ||
        (((((_iml_sp_union_t *)&x)->bits.exponent) >
          (((_iml_sp_union_t *)&y)->bits.exponent)) ||
         (((((_iml_sp_union_t *)&x)->bits.exponent) ==
           (((_iml_sp_union_t *)&y)->bits.exponent)) &&
          (((((_iml_sp_union_t *)&x)->bits.significand) >
            (((_iml_sp_union_t *)&y)->bits.significand)))))) {
      (*(int *)&fres)--;
    } else {
      (*(int *)&fres)++;
    }
    *r1 = fres;
    ures = ((*(int *)&fres) & ~0x80000000);
    if (ures - 0x00800000 >= 0x7f800000 - 0x00800000) {
      if (ures < 0x00800000) {
        nRet = 4;
        service = (((const float *)_small_value_32)[(0)] *
                   ((const float *)_small_value_32)[0]);
      } else {
        nRet = 3;
        service = (((const float *)_large_value_32)[(0)] *
                   ((const float *)_large_value_32)[0]);
        *r1 = fres;
      }
    }
  }
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_nextafter_s_xa */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_nextafterf(float a, float b) {
  using namespace __imf_impl_nextafter_s_xa;
  VUINT32 vm;
  float va1;
  float va2;
  float vr1;
  float r;
  va1 = a;
  va2 = b;
  {
    VUINT32 iX1;
    VUINT32 iX2;
    VUINT32 iAbsX;
    VUINT32 iAbsY;
    VUINT32 iAbsR;
    VUINT32 iXSpec;
    VUINT32 iYSpec;
    VUINT32 iXYSpec;
    VUINT32 iRSpec;
    VUINT32 iEqual;
    VUINT32 iXor;
    VUINT32 iDiffSign;
    VUINT32 iX1GTX2;
    VUINT32 iMMask;
    VUINT32 iX1Diff;
    /* Constants */
    VUINT32 iAbsMask;
    VUINT32 iSubConst;
    VUINT32 iCmpConst;
    VUINT32 iZero;
    VUINT32 iPOne;
    VUINT32 iMOne;
    iAbsMask = (__devicelib_imf_internal_snextafter_data._iAbsMask);
    iSubConst = (__devicelib_imf_internal_snextafter_data._iSubConst);
    iCmpConst = (__devicelib_imf_internal_snextafter_data._iCmpConst);
    iZero = 0;
    iPOne = (__devicelib_imf_internal_snextafter_data._iPOne);
    iMOne = (__devicelib_imf_internal_snextafter_data._iMOne);
    // Cast to integer
    iX1 = as_uint(va1);
    iX2 = as_uint(va2);
    // Check if X and Y is special
    iAbsX = (iX1 & iAbsMask);
    iXSpec = (iAbsX - iSubConst);
    iXSpec = ((VUINT32)(-(VSINT32)((VSINT32)iXSpec > (VSINT32)iCmpConst)));
    iAbsY = (iX2 & iAbsMask);
    iYSpec = (iAbsY - iSubConst);
    iYSpec = ((VUINT32)(-(VSINT32)((VSINT32)iYSpec > (VSINT32)iCmpConst)));
    iXYSpec = (iXSpec | iYSpec);
    // Check if iX1 == iX2
    iEqual = ((VUINT32)(-(VSINT32)((VSINT32)iX1 == (VSINT32)iX2)));
    // Check if iX1 and iX2 have different sign
    iXor = (iX1 ^ iX2);
    iDiffSign = ((VUINT32)(-(VSINT32)((VSINT32)iZero > (VSINT32)iXor)));
    // Check if iX1 is greater than iX2
    iX1GTX2 = ((VUINT32)(-(VSINT32)((VSINT32)iX1 > (VSINT32)iX2)));
    // Now do the adjustment to X
    iMMask = (iDiffSign | iX1GTX2);
    iX1Diff = (((~(iMMask)) & (iPOne)) | ((iMMask) & (iMOne)));
    iX1Diff = (~(iEqual)&iX1Diff);
    iX1 = (iX1 + iX1Diff);
    // Check if result overflow or underflow
    iAbsR = (iX1 & iAbsMask);
    iRSpec = (iAbsR - iSubConst);
    iRSpec = ((VUINT32)(-(VSINT32)((VSINT32)iRSpec > (VSINT32)iCmpConst)));
    // or if the result overflow/underflow
    iRSpec = (iXYSpec | iRSpec);
    vm = 0;
    vm = iRSpec;
    // Cast back to float
    vr1 = as_float(iX1);
    /* -------------- The end of implementation ---------------- */
  }
  if (__builtin_expect((vm) != 0, 0)) {
    float __cout_a1;
    float __cout_a2;
    float __cout_r1;
    ((float *)&__cout_a1)[0] = va1;
    ((float *)&__cout_a2)[0] = va2;
    ((float *)&__cout_r1)[0] = vr1;
    __devicelib_imf_internal_snextafter(&__cout_a1, &__cout_a2, &__cout_r1);
    vr1 = ((const float *)&__cout_r1)[0];
  }
  r = vr1;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
