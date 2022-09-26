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
namespace __imf_impl_scalbn_d_xa {
namespace {
typedef struct {
  VUINT32 _iExpMask;
  VUINT32 _iMaxExp;
  VUINT32 _iZero;
  VUINT32 _iPosMaxN;
  VUINT32 _iNegMaxN;
} __devicelib_imf_internal_dscalbn_data_t;
static const __devicelib_imf_internal_dscalbn_data_t
    __devicelib_imf_internal_dscalbn_data = {
        0x7ff00000u, 0x000007ffu, 0x00000000u, 0x00010000u, 0xffff0000u};
static const unsigned int _large_value_64[] = {
    0x00000000, 0x7e700000 /* +2^1000 */
    ,
    0x00000000, 0xfe700000 /* -2^1000 */
};
static const unsigned int _small_value_64[] = {
    0x00000000, 0x01700000 /* +2^(-1000) */
    ,
    0x00000000, 0x81700000 /* -2^(-1000) */
};
inline int __devicelib_imf_internal_dscalbn(const double *a1, const int *a2,
                                            double *r1) {
  int nRet = 0;
  double arg;
  int exp;
  int n;
  _iml_uint64_t lone[2] = {0x3FF0000000000000, 0xBFF0000000000000};
  double *done = (double *)lone;
  const _iml_uint64_t _TWO_55[] = {0x4360000000000000};  /* 2^55 */
  const _iml_uint64_t _TWO_M55[] = {0x3c80000000000000}; /* 2^(-55) */
                                                         /*
                                                         //
                                                         //      * Flush denormals, if needed
                                                         //      * Raise invalid for SNaN
                                                         //
                                                         */
  arg = (*a1 * done[0]);
  if (!((((_iml_dp_union_t *)&arg)->bits.exponent) != 0x7FF)) {
    *r1 = arg; /* INF, NaN */
    return nRet;
  }
  if ((((*(_iml_int64_t *)&arg) & ~0x8000000000000000) == 0)) {
    *r1 = arg; /* Zero */
    return nRet;
  }
  exp = (((_iml_dp_union_t *)&arg)->bits.exponent);
  n = *a2;
  if (exp == 0) {
    /* denormals */
    arg *= (*(const double *)_TWO_55);
    exp = (((_iml_dp_union_t *)&arg)->bits.exponent) - 55;
  }
  if (n > 0x10000)
    n = 0x10000;
  if (n < -0x10000)
    n = -0x10000;
  exp += n;
  if (exp > 0) {
    if (exp >= 0x7FF) {
      /*
      //
      //              * Raise overflow and inexact
      //
      */
      nRet = 3;
      *r1 = (((const double *)
                  _large_value_64)[((((_iml_dp_union_t *)&arg)->bits.sign))] *
             ((const double *)_large_value_64)[0]);
    } else {
      (((_iml_dp_union_t *)&arg)->bits.exponent = exp);
      *r1 = arg;
    }
    return nRet;
  } else {
    if (exp < -52) {
      /*
      //
      //              * Raise underflow and inexact
      //
      */
      nRet = 4;
      *r1 = (((const double *)
                  _small_value_64)[((((_iml_dp_union_t *)&arg)->bits.sign))] *
             ((const double *)_small_value_64)[0]);
      return nRet;
    }
    (((_iml_dp_union_t *)&arg)->bits.exponent = (exp + 55));
    *r1 = arg * (*(const double *)_TWO_M55);
    if ((((*(_iml_int64_t *)&*r1) & ~0x8000000000000000) == 0)) {
      nRet = 4;
      *r1 = (((const double *)
                  _small_value_64)[((((_iml_dp_union_t *)&arg)->bits.sign))] *
             ((const double *)_small_value_64)[0]);
    }
  }
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_scalbn_d_xa */
DEVICE_EXTERN_C_INLINE double __devicelib_imf_scalbn(double x, int32_t y) {
  using namespace __imf_impl_scalbn_d_xa;
  double r;
  VUINT32 vm;
  double va1;
  VUINT32 va2;
  double vr1;
  va1 = x;
  va2 = y;
  {
    VUINT64 lX;
    VUINT64 lHiX;
    VUINT64 lLoX;
    VUINT32 iHiX;
    VUINT32 iLoX;
    VUINT32 iN;
    VUINT32 iPosMaxN;
    VUINT32 iNegMaxN;
    VUINT32 iMaxExp;
    VUINT32 iZero;
    VUINT32 iExp;
    VUINT32 iExpMask;
    VUINT32 iCmpMask1;
    VUINT32 iCmpMask2;
    VUINT32 iCmpMask3;
    iMaxExp = (__devicelib_imf_internal_dscalbn_data._iMaxExp);
    iExpMask = (__devicelib_imf_internal_dscalbn_data._iExpMask);
    iZero = (__devicelib_imf_internal_dscalbn_data._iZero);
    iPosMaxN = (__devicelib_imf_internal_dscalbn_data._iPosMaxN);
    iNegMaxN = (__devicelib_imf_internal_dscalbn_data._iNegMaxN);
    lX = as_ulong(va1);
    iN = va2;
    iHiX = ((VUINT32)((VUINT64)lX >> 32));
    iLoX = (((VUINT32)lX & (VUINT32)-1));
    iExp = (iExpMask & iHiX);
    iExp = ((VUINT32)(iExp) >> (20));
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
        -(VSINT32)((VSINT32)iExp >= (VSINT32)iMaxExp))); /* newExp >= 2047 */
    iCmpMask2 = (iCmpMask2 | iCmpMask3);
    iCmpMask1 = (iCmpMask1 | iCmpMask2);
    vm = 0;
    vm = iCmpMask1;
    iExp = ((VUINT32)(iExp) << (20));
    iHiX = (~(iExpMask)&iHiX);
    iHiX = (iHiX | iExp);
    lHiX = (((VUINT64)(VUINT32)iHiX << 32));
    lLoX = ((VUINT64)(VUINT32)iLoX);
    lX = (lLoX | lHiX);
    vr1 = as_double(lX);
    /* -------------- The end of implementation ---------------- */
  }
  if (__builtin_expect((vm) != 0, 0)) {
    double __cout_a1;
    int __cout_a2;
    double __cout_r1;
    ((double *)&__cout_a1)[0] = va1;
    ((VUINT32 *)&__cout_a2)[0] = va2;
    ((double *)&__cout_r1)[0] = vr1;
    __devicelib_imf_internal_dscalbn(&__cout_a1, &__cout_a2, &__cout_r1);
    vr1 = ((const double *)&__cout_r1)[0];
  }
  r = vr1;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
