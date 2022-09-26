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
namespace __imf_impl_nextafter_d_xa {
namespace {
typedef struct {
  VUINT32 _iAbsMask;
  VUINT32 _iSubConst;
  VUINT32 _iCmpConst;
  VUINT64 _lPOne;
  VUINT64 _lMOne;
} __devicelib_imf_internal_dnextafter_data_t;
static const __devicelib_imf_internal_dnextafter_data_t
    __devicelib_imf_internal_dnextafter_data = {
        0x7fffffffu, 0x80100000u, 0xffbfffffu, 0x0000000000000001uL,
        0xffffffffffffffffuL};
static const _iml_dp_union_t _min_subnormal_64[] = {0x00000001, 0x00000000,
                                                    0x00000001, 0x80000000};
static const _iml_uint64_t lone[2] = {0x3FF0000000000000, 0xBFF0000000000000};
static const _iml_dp_union_t _large_value_64[] = {
    0x00000000, 0x7e700000 /* +2^1000 */
    ,
    0x00000000, 0xfe700000 /* -2^1000 */
};
static const _iml_dp_union_t _small_value_64[] = {
    0x00000000, 0x01700000 /* +2^(-1000) */
    ,
    0x00000000, 0x81700000 /* -2^(-1000) */
};
inline int __devicelib_imf_internal_dnextafter(const double *a1,
                                               const double *a2, double *r1) {
  int nRet = 0;
  double service;
  double x, y, fres;
  unsigned int ures;
  const double *done = (const double *)lone;
  /*
  //
  //      * Flush denormals, if needed
  //      * Raise invalid for SNaN
  //
  */
  x = (*a1 * done[0]);
  y = (*a2 * done[0]);
  if ((((((_iml_dp_union_t *)&x)->bits.exponent) == 0x07FF) &&
       (((((_iml_dp_union_t *)&x)->bits.hi_significand) != 0) ||
        ((((_iml_dp_union_t *)&x)->bits.lo_significand) != 0))) ||
      (((((_iml_dp_union_t *)&y)->bits.exponent) == 0x07FF) &&
       (((((_iml_dp_union_t *)&y)->bits.hi_significand) != 0) ||
        ((((_iml_dp_union_t *)&y)->bits.lo_significand) != 0)))) {
    *r1 = x + y;
    return nRet;
  }
  if (x == y) {
    *r1 = y;
    return nRet;
  }
  if (((((((int *)&x)[1]) & ~0x80000000) | (((int *)&x)[0])) == 0)) {
    nRet = 4;
    (*(_iml_int64_t *)&*r1) = (*(const _iml_int64_t *)&(
        (const double *)
            _min_subnormal_64)[(((_iml_dp_union_t *)&y)->bits.sign)]);
    service = (((const double *)_small_value_64)[(0)] *
               ((const double *)_small_value_64)[0]);
  } else {
    (*(_iml_int64_t *)&fres) = (*(_iml_int64_t *)&x);
    if ((((((int *)&x)[1]) ^ (((int *)&y)[1])) & 0x80000000) ||
        (((((_iml_dp_union_t *)&x)->bits.exponent) >
          (((_iml_dp_union_t *)&y)->bits.exponent)) ||
         (((((_iml_dp_union_t *)&x)->bits.exponent) ==
           (((_iml_dp_union_t *)&y)->bits.exponent)) &&
          ((((((_iml_dp_union_t *)&x)->bits.hi_significand) >
             (((_iml_dp_union_t *)&y)->bits.hi_significand)) ||
            (((((_iml_dp_union_t *)&x)->bits.hi_significand) ==
              (((_iml_dp_union_t *)&y)->bits.hi_significand)) &&
             ((((_iml_dp_union_t *)&x)->bits.lo_significand) >
              (((_iml_dp_union_t *)&y)->bits.lo_significand)))))))) {
      (*(_iml_int64_t *)&fres)--;
    } else {
      (*(_iml_int64_t *)&fres)++;
    }
    *r1 = fres;
    ures = ((((int *)&fres)[1]) & ~0x80000000);
    if (ures - 0x00100000 >= 0x7ff00000 - 0x00100000) {
      if (ures < 0x00800000) {
        nRet = 4;
        service = (((const double *)_small_value_64)[(0)] *
                   ((const double *)_small_value_64)[0]);
      } else {
        nRet = 3;
        service = (((const double *)_large_value_64)[(0)] *
                   ((const double *)_large_value_64)[0]);
        *r1 = fres;
      }
    }
  }
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_nextafter_d_xa */
DEVICE_EXTERN_C_INLINE double __devicelib_imf_nextafter(double x, double y) {
  using namespace __imf_impl_nextafter_d_xa;
  double r;
  VUINT32 vm;
  double va1;
  double va2;
  double vr1;
  va1 = x;
  va2 = y;
  {
    VUINT64 lX1;
    VUINT64 lX2;
    VUINT32 iX1Hi;
    VUINT32 iX2Hi;
    VUINT32 iAbsX;
    VUINT32 iAbsY;
    VUINT32 iAbsR;
    VUINT32 iXSpec;
    VUINT32 iYSpec;
    VUINT32 iXYSpec;
    VUINT32 iRSpec;
    double dEqual;
    VUINT64 lEqual;
    VUINT64 lXor;
    VUINT64 lDiffSign;
    VUINT32 iXorHi;
    VUINT32 iDiffSign;
    VUINT64 lX1GTX2;
    VUINT32 iX1GTX2Hi;
    VUINT64 lMMask;
    VUINT64 lX1Diff;
    /* Constants */
    VUINT32 iAbsMask;
    VUINT32 iSubConst;
    VUINT32 iCmpConst;
    VUINT64 lZero;
    VUINT32 iZero;
    VUINT64 lPOne;
    VUINT64 lMOne;
    iAbsMask = (__devicelib_imf_internal_dnextafter_data._iAbsMask);
    iSubConst = (__devicelib_imf_internal_dnextafter_data._iSubConst);
    iCmpConst = (__devicelib_imf_internal_dnextafter_data._iCmpConst);
    iZero = 0;
    lZero = 0L;
    lPOne = (__devicelib_imf_internal_dnextafter_data._lPOne);
    lMOne = (__devicelib_imf_internal_dnextafter_data._lMOne);
    // Cast to integer
    lX1 = as_ulong(va1);
    lX2 = as_ulong(va2);
    // Check if X and Y is special
    iX1Hi = ((VUINT32)((VUINT64)lX1 >> 32));
    iX2Hi = ((VUINT32)((VUINT64)lX2 >> 32));
    iAbsX = (iX1Hi & iAbsMask);
    iXSpec = (iAbsX - iSubConst);
    iXSpec = ((VUINT32)(-(VSINT32)((VSINT32)iXSpec > (VSINT32)iCmpConst)));
    iAbsY = (iX2Hi & iAbsMask);
    iYSpec = (iAbsY - iSubConst);
    iYSpec = ((VUINT32)(-(VSINT32)((VSINT32)iYSpec > (VSINT32)iCmpConst)));
    iXYSpec = (iXSpec | iYSpec);
    // Use this code path if vcmp for 64bit int is supported
    // Check if lX1 == lX2
    lEqual = ((VUINT64)(-(VSINT64)((VSINT64)lX1 == (VSINT64)lX2)));
    // Check if lX1 and lX2 have different sign
    lXor = (lX1 ^ lX2);
    lDiffSign = ((VUINT64)(-(VSINT64)((VSINT64)lZero > (VSINT64)lXor)));
    // Check if lX1 is greater than lX2
    lX1GTX2 = ((VUINT64)(-(VSINT64)((VSINT64)lX1 > (VSINT64)lX2)));
    // Now do the adjustment to X
    lMMask = (lDiffSign | lX1GTX2);
    lX1Diff = (((~(lMMask)) & (lPOne)) | ((lMMask) & (lMOne)));
    lX1Diff = (~(lEqual)&lX1Diff);
    lX1 = (lX1 + lX1Diff);
    // Check if result overflow or underflow
    iX1Hi = ((VUINT32)((VUINT64)lX1 >> 32));
    iAbsR = (iX1Hi & iAbsMask);
    iRSpec = (iAbsR - iSubConst);
    iRSpec = ((VUINT32)(-(VSINT32)((VSINT32)iRSpec > (VSINT32)iCmpConst)));
    // or if the result overflow/underflow
    iRSpec = (iXYSpec | iRSpec);
    vm = 0;
    vm = iRSpec;
    // Cast back to float
    vr1 = as_double(lX1);
    /* -------------- The end of implementation ---------------- */
  }
  if (__builtin_expect((vm) != 0, 0)) {
    double __cout_a1;
    double __cout_a2;
    double __cout_r1;
    ((double *)&__cout_a1)[0] = va1;
    ((double *)&__cout_a2)[0] = va2;
    ((double *)&__cout_r1)[0] = vr1;
    __devicelib_imf_internal_dnextafter(&__cout_a1, &__cout_a2, &__cout_r1);
    vr1 = ((const double *)&__cout_r1)[0];
  }
  r = vr1;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
