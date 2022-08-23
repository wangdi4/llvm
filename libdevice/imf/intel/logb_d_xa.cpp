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
namespace __imf_impl_logb_d_xa {
namespace {
typedef struct {
  VUINT32 _iExpMask;
  VUINT32 _iZero;
  VUINT32 _iBias;
} __devicelib_imf_internal_dlogb_data_t;
static const __devicelib_imf_internal_dlogb_data_t
    __devicelib_imf_internal_dlogb_data = {0x7ff00000u, 0x00000000u,
                                           0x000003ffu};
static const _iml_int64_t _TWO_55[] = {0x4360000000000000}; /* 2^55 */
static const double _one_ = 1.0;
inline int __devicelib_imf_internal_dlogb(const double *a, double *r1) {
  int nRet = 0;
  double arg;
  /*
  //
  //      * For DAZ zero argument
  //      * For SNaN raise invalid
  //
  */
  arg = ((*a) * (_one_));
  /*
  //
  //      * For zero return -INF, and raise divide-by-zero
  //
  */
  if ((((*(_iml_int64_t *)&(arg)) & ~0x8000000000000000) == 0)) {
    nRet = 1;
    *r1 = 0.0;
    *r1 = (-(_one_) / (*r1));
  }
  /*
  //
  //      * For NaNs returns QNaN, for
  //      * For INF  returns +INF
  //
  */
  else if (!((((_iml_dp_union_t *)&arg)->bits.exponent) != 0x7FF)) {
    (*(_iml_int64_t *)&(*r1)) = (*(_iml_int64_t *)&(arg)) & ~0x8000000000000000;
  }
  /*
  //
  //      * Denormals
  //
  */
  else if (!(((((_iml_dp_union_t *)&arg)->bits.hi_significand) == 0) &&
             ((((_iml_dp_union_t *)&arg)->bits.lo_significand) == 0)) &&
           ((((_iml_dp_union_t *)&arg)->bits.exponent) == 0)) {
    arg = arg * (*(const double *)_TWO_55);
    *r1 =
        (double)((int)(((_iml_dp_union_t *)&arg)->bits.exponent) - 0x3FF - 55);
  }
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_logb_d_xa */
DEVICE_EXTERN_C_INLINE double __devicelib_imf_logb(double x) {
  using namespace __imf_impl_logb_d_xa;
  double r;
  VUINT32 vm;
  double va1;
  double vr1;
  va1 = x;
  {
    VUINT64 lX;
    VUINT32 iHiX;
    VUINT32 iExp;
    VUINT32 iBias;
    VUINT32 iZero;
    VUINT32 iExpMask;
    VUINT32 iBrMask1;
    VUINT32 iBrMask2;
    iExpMask = (__devicelib_imf_internal_dlogb_data._iExpMask);
    iZero = (__devicelib_imf_internal_dlogb_data._iZero);
    iBias = (__devicelib_imf_internal_dlogb_data._iBias);
    lX = as_ulong(va1);
    iHiX = ((VUINT32)((VUINT64)lX >> 32));
    /* Check for NaN, INF */
    iExp = (iExpMask & iHiX);
    iBrMask1 = ((VUINT32)(-(VSINT32)((VSINT32)iExpMask == (VSINT32)iExp)));
    /* Check for zero and denormals */
    iBrMask2 = ((VUINT32)(-(VSINT32)((VSINT32)iZero == (VSINT32)iExp)));
    iBrMask1 = (iBrMask1 | iBrMask2);
    vm = 0;
    vm = iBrMask1;
    iExp = ((VUINT32)(iExp) >> (20));
    iExp = (iExp - iBias);
    vr1 = ((double)((VINT32)(iExp)));
    /* -------------- The end of implementation ---------------- */
  }
  if (__builtin_expect((vm) != 0, 0)) {
    double __cout_a1;
    double __cout_r1;
    ((double *)&__cout_a1)[0] = va1;
    ((double *)&__cout_r1)[0] = vr1;
    __devicelib_imf_internal_dlogb(&__cout_a1, &__cout_r1);
    vr1 = ((const double *)&__cout_r1)[0];
  }
  r = vr1;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
