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
namespace __imf_impl_ilogb_s_xa {
namespace {
typedef struct {
  VUINT32 _iExpMask;
  VUINT32 _iZero;
  VUINT32 _iBias;
} __devicelib_imf_internal_silogb_data_t;
static const __devicelib_imf_internal_silogb_data_t
    __devicelib_imf_internal_silogb_data = {0x7f800000u, 0x00000000u,
                                            0x0000007fu};
static const int _TWO_25[] = {0x4c000000}; /* 2^25 */
static const float _one_ = 1.0;
inline int __devicelib_imf_internal_silogb(const float *a, int *r1) {
  int nRet = 0;
  float arg;
  /* For DAZ, other flags */
  arg = (*a * _one_);
  if ((((*(int *)&(arg)) & ~0x80000000) == 0)) {
    nRet = 1;
    *r1 = 0x80000000;
  }
  if (!((((_iml_sp_union_t *)&arg)->bits.exponent) != 0xFF)) {
    /* NaN or INF */
    nRet = 1;
    if ((((_iml_sp_union_t *)&arg)->bits.significand) != 0)
      *r1 = 0x80000000; /* NaN */
    else
      *r1 = 0x7fffffff; /* INF */
  }
  if (!((((_iml_sp_union_t *)&arg)->bits.significand) == 0) &&
      ((((_iml_sp_union_t *)&arg)->bits.exponent) == 0)) {
    /* denormals */
    arg = arg * (*(const float *)_TWO_25);
    *r1 = (((_iml_sp_union_t *)&arg)->bits.exponent) - 0x7F - 25;
  }
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_ilogb_s_xa */

DEVICE_EXTERN_C_INLINE int32_t __devicelib_imf_ilogbf(float a) {
  using namespace __imf_impl_ilogb_s_xa;
  VUINT32 vm;
  float va1;
  VUINT32 vr1;
  int32_t r;
  va1 = a;
  ;
  {
    VUINT32 iX;
    VUINT32 iExp;
    VUINT32 iBias;
    VUINT32 iZero;
    VUINT32 iExpMask;
    VUINT32 iBrMask1;
    VUINT32 iBrMask2;
    iExpMask = (__devicelib_imf_internal_silogb_data._iExpMask);
    iZero = (__devicelib_imf_internal_silogb_data._iZero);
    iBias = (__devicelib_imf_internal_silogb_data._iBias);
    iX = as_uint(va1);
    /* Check for NaN, INF */
    iExp = (iExpMask & iX);
    iBrMask1 = ((VUINT32)(-(VSINT32)((VSINT32)iExpMask == (VSINT32)iExp)));
    /* Check for zero and denormals */
    iBrMask2 = ((VUINT32)(-(VSINT32)((VSINT32)iZero == (VSINT32)iExp)));
    iBrMask1 = (iBrMask1 | iBrMask2);
    vm = 0;
    vm = iBrMask1;
    iExp = ((VUINT32)(iExp) >> (23));
    vr1 = (iExp - iBias);
    /* -------------- The end of implementation ---------------- */
  }
  if (__builtin_expect((vm) != 0, 0)) {
    float __cout_a1;
    int __cout_r1;
    ((float *)&__cout_a1)[0] = va1;
    ((VUINT32 *)&__cout_r1)[0] = vr1;
    __devicelib_imf_internal_silogb(&__cout_a1, &__cout_r1);
    vr1 = ((const VUINT32 *)&__cout_r1)[0];
  }
  r = vr1;
  ;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
