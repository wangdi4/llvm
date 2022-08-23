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
namespace __imf_impl_frexp_d_xa {
namespace {
typedef struct {
  VUINT32 _iExpMask;
  VUINT32 _iZero;
  VUINT32 _iNewExp;
} __devicelib_imf_internal_dfrexp_data_t;
static const __devicelib_imf_internal_dfrexp_data_t
    __devicelib_imf_internal_dfrexp_data = {0x7ff00000u, 0x00000000u,
                                            0x3fe00000u};
inline int __devicelib_imf_internal_dfrexp(const double *a, double *r1,
                                           int *r2) {
  int nRet = 0;
  double arg;
  _iml_uint64_t lone[2] = {0x3FF0000000000000, 0xBFF0000000000000};
  double *done = (double *)lone;
  const _iml_uint64_t _TWO_55[] = {0x4360000000000000}; /* 2^55 */
  /* For DAZ flag */
  arg = (*a * done[0]);
  /* NaN or INF */
  if ((!((((_iml_dp_union_t *)&arg)->bits.exponent) != 0x7FF)) ||
      ((((((_iml_dp_union_t *)&arg)->bits.hi_significand) == 0) &&
        ((((_iml_dp_union_t *)&arg)->bits.lo_significand) == 0)) &&
       ((((_iml_dp_union_t *)&arg)->bits.exponent) == 0))) {
    *r1 = arg;
    *r2 = 0;
  } else {
    /* denormals */
    /* if ( !IML_IS_SIGNIFICAND_ZERO_DP(arg) && (IML_GET_EXP_DP(arg) == 0) )  */
    *r1 = arg * (*(const double *)_TWO_55);
    *r2 = (((_iml_dp_union_t *)&*r1)->bits.exponent) - (0x3FF - 1) - 55;
    (*(_iml_int64_t *)&*r1) =
        ((*(_iml_int64_t *)&*r1) & ~0x7ff0000000000000) | 0x3fe0000000000000;
  }
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_frexp_d_xa */
DEVICE_EXTERN_C_INLINE double __devicelib_imf_frexp(double x, int32_t *z) {
  using namespace __imf_impl_frexp_d_xa;
  double r;
  VUINT32 vm;
  double va1;
  double vr1;
  VUINT32 vr2;
  va1 = x;
  {
    VUINT64 lX;
    VUINT64 lHiX;
    VUINT64 lLoX;
    VUINT32 iHiX;
    VUINT32 iLoX;
    VUINT32 iExp;
    VUINT32 iNewExp;
    VUINT32 iZero;
    VUINT32 iExpMask;
    VUINT32 iBrMask1;
    VUINT32 iBrMask2;
    iExpMask = (__devicelib_imf_internal_dfrexp_data._iExpMask);
    iZero = (__devicelib_imf_internal_dfrexp_data._iZero);
    iNewExp = (__devicelib_imf_internal_dfrexp_data._iNewExp);
    lX = as_ulong(va1);
    iHiX = ((VUINT32)((VUINT64)lX >> 32));
    iLoX = (((VUINT32)lX & (VUINT32)-1));
    /* Check for NaN, INF */
    iExp = (iExpMask & iHiX);
    iBrMask1 = ((VUINT32)(-(VSINT32)((VSINT32)iExpMask == (VSINT32)iExp)));
    /* Check for zero and denormals */
    iBrMask2 = ((VUINT32)(-(VSINT32)((VSINT32)iZero == (VSINT32)iExp)));
    iBrMask1 = (iBrMask1 | iBrMask2);
    vm = 0;
    vm = iBrMask1;
    iExp = (iExp - iNewExp);
    vr2 = ((VSINT32)iExp >> (20));
    iHiX = (~(iExpMask)&iHiX);
    iHiX = (iHiX | iNewExp);
    lHiX = (((VUINT64)(VUINT32)iHiX << 32));
    lLoX = ((VUINT64)(VUINT32)iLoX);
    lX = (lLoX | lHiX);
    vr1 = as_double(lX);
    /* -------------- The end of implementation ---------------- */
  }
  if (__builtin_expect((vm) != 0, 0)) {
    double __cout_a1;
    double __cout_r1;
    int __cout_r2;
    ((double *)&__cout_a1)[0] = va1;
    ((double *)&__cout_r1)[0] = vr1;
    ((VUINT32 *)&__cout_r2)[0] = vr2;
    __devicelib_imf_internal_dfrexp(&__cout_a1, &__cout_r1, &__cout_r2);
    vr1 = ((const double *)&__cout_r1)[0];
    vr2 = ((const VUINT32 *)&__cout_r2)[0];
  }
  r = vr1;
  ((VUINT32 *)z)[0] = vr2;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
