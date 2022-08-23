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
#include "_imf_include_fp32.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_scalbn_s_xa {
namespace {
typedef struct {
  VUINT32 _iExpMask;
  VUINT32 _iMaxExp;
  VUINT32 _iZero;
  VUINT32 _iPosMaxN;
  VUINT32 _iNegMaxN;
} __devicelib_imf_internal_sscalbn_data_t;
static const __devicelib_imf_internal_sscalbn_data_t
    __devicelib_imf_internal_sscalbn_data = {
        0x7f800000u, 0x000000ffu, 0x00000000u, 0x00010000u, 0xffff0000u};
static const float _one_ = 1.0;
static const int _TWO_25[] = {0x4c000000};  /* 2^25 */
static const int _TWO_M25[] = {0x33000000}; /* 2^(-25) */
static const unsigned int _large_value_32[] = {0x71800000,
                                               0xf1800000}; /* +2^100,-2^100 */
static const unsigned int _small_value_32[] = {
    0x0d800000, 0x8d800000}; /* +2^(-100),-2^(-100) */
inline int __devicelib_imf_internal_sscalbn(const float *a1, const int *a2,
                                            float *r1) {
  int nRet = 0;
  float arg;
  int exp;
  int n;
  /*
  //
  //      * Flush denormals, if needed
  //      * Raise invalid for SNaN
  //
  */
  arg = (*a1 * _one_);
  if (!((((_iml_sp_union_t *)&arg)->bits.exponent) != 0xFF)) {
    *r1 = arg; /* INF, NaN */
    return nRet;
  }
  if ((((*(int *)&arg) & ~0x80000000) == 0)) {
    *r1 = arg; /* Zero */
    return nRet;
  }
  exp = (((_iml_sp_union_t *)&arg)->bits.exponent);
  n = *a2;
  if (exp == 0) {
    /* denormals */
    arg *= (*(const float *)_TWO_25);
    exp = (((_iml_sp_union_t *)&arg)->bits.exponent) - 25;
  }
  if (n > 0x10000)
    n = 0x10000;
  if (n < -0x10000)
    n = -0x10000;
  exp += n;
  if (exp > 0) {
    if (exp >= 0xFF) {
      /*
      //
      //              * Raise overflow and inexact
      //
      */
      nRet = 3;
      *r1 = (((const float *)
                  _large_value_32)[((((_iml_sp_union_t *)&arg)->bits.sign))] *
             ((const float *)_large_value_32)[0]);
    } else {
      (((_iml_sp_union_t *)&arg)->bits.exponent = exp);
      *r1 = arg;
    }
    return nRet;
  } else {
    if (exp < -23) {
      /*
      //
      //              * Raise underflow and inexact
      //
      */
      nRet = 4;
      *r1 = (((const float *)
                  _small_value_32)[((((_iml_sp_union_t *)&arg)->bits.sign))] *
             ((const float *)_small_value_32)[0]);
      return nRet;
    }
    (((_iml_sp_union_t *)&arg)->bits.exponent = (exp + 25));
    *r1 = arg * (*(const float *)_TWO_M25);
    if ((((*(int *)&*r1) & ~0x80000000) == 0)) {
      nRet = 4;
      *r1 = (((const float *)
                  _small_value_32)[((((_iml_sp_union_t *)&arg)->bits.sign))] *
             ((const float *)_small_value_32)[0]);
    }
  }
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_scalbn_s_xa */
DEVICE_EXTERN_C_INLINE float __devicelib_imf_scalbnf(float x, int32_t y) {
  using namespace __imf_impl_scalbn_s_xa;
  float r;
  VUINT32 vm;
  float va1;
  VUINT32 va2;
  float vr1;
  va1 = x;
  va2 = y;
  {
    VUINT32 iX;
    VUINT32 iExp;
    VUINT32 iN;
    VUINT32 iPosMaxN;
    VUINT32 iNegMaxN;
    VUINT32 iZero;
    VUINT32 iExpMask;
    VUINT32 iMaxExp;
    VUINT32 iCmpMask1;
    VUINT32 iCmpMask2;
    VUINT32 iCmpMask3;
    iMaxExp = (__devicelib_imf_internal_sscalbn_data._iMaxExp);
    iExpMask = (__devicelib_imf_internal_sscalbn_data._iExpMask);
    iZero = (__devicelib_imf_internal_sscalbn_data._iZero);
    iPosMaxN = (__devicelib_imf_internal_sscalbn_data._iPosMaxN);
    iNegMaxN = (__devicelib_imf_internal_sscalbn_data._iNegMaxN);
    iX = as_uint(va1);
    iN = va2;
    iExp = (iExpMask & iX);
    iExp = ((VUINT32)(iExp) >> (23));
    iCmpMask1 = ((VUINT32)(
        -(VSINT32)((VSINT32)iMaxExp == (VSINT32)iExp))); /* NaN, INF        */
    iCmpMask2 = ((VUINT32)(
        -(VSINT32)((VSINT32)iZero == (VSINT32)iExp))); /* zero, denormals */
    iCmpMask1 = (iCmpMask1 | iCmpMask2);
    /*
    //
    //  * To avoid over/underflow for integer operation
    //
    */
    iCmpMask2 = ((VUINT32)(
        -(VSINT32)((VSINT32)iN > (VSINT32)iPosMaxN))); /* n >  0x10000 */
    iN = (((~(iCmpMask2)) & (iN)) | ((iCmpMask2) & (iPosMaxN)));
    iCmpMask2 = ((VUINT32)(
        -(VSINT32)((VSINT32)iN < (VSINT32)iNegMaxN))); /* n < -0x10000 */
    iN = (((~(iCmpMask2)) & (iN)) | ((iCmpMask2) & (iNegMaxN)));
    iExp = (iExp + iN);
    iCmpMask2 = ((VUINT32)(
        -(VSINT32)((VSINT32)iExp <= (VSINT32)iZero))); /* newExp <= 0   */
    iCmpMask3 = ((VUINT32)(
        -(VSINT32)((VSINT32)iExp >= (VSINT32)iMaxExp))); /* newExp >= 255 */
    iCmpMask2 = (iCmpMask2 | iCmpMask3);
    iCmpMask1 = (iCmpMask1 | iCmpMask2);
    vm = 0;
    vm = iCmpMask1;
    iExp = ((VUINT32)(iExp) << (23));
    iX = (~(iExpMask)&iX);
    iX = (iX | iExp);
    vr1 = as_float(iX);
    /* -------------- The end of implementation ---------------- */
  }
  if (__builtin_expect((vm) != 0, 0)) {
    float __cout_a1;
    int __cout_a2;
    float __cout_r1;
    ((float *)&__cout_a1)[0] = va1;
    ((VUINT32 *)&__cout_a2)[0] = va2;
    ((float *)&__cout_r1)[0] = vr1;
    __devicelib_imf_internal_sscalbn(&__cout_a1, &__cout_a2, &__cout_r1);
    vr1 = ((const float *)&__cout_r1)[0];
  }
  r = vr1;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
