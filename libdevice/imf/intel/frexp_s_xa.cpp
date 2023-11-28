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
namespace __imf_impl_frexp_s_xa {
namespace {
typedef struct {
  VUINT32 _iExpMask;
  VUINT32 _iZero;
  VUINT32 _iNewExp;
} __devicelib_imf_internal_sfrexp_data_t;
static const __devicelib_imf_internal_sfrexp_data_t
    __devicelib_imf_internal_sfrexp_data = {0x7f800000u, 0x00000000u,
                                            0x3f000000u};
inline int __devicelib_imf_internal_sfrexp(const float *a, float *r1, int *r2) {
  int nRet = 0;
  float arg;
  VUINT32 ione[2] = {0x3f800000, 0xbf800000};
  float *fone = (float *)ione;
  const VUINT32 _TWO_25[] = {0x4c000000}; /* 2^25 */
  /* For DAZ flag */
  arg = (*a * fone[0]);
  /* NaN or INF */
  if ((!((((_iml_sp_union_t *)&arg)->bits.exponent) != 0xFF)) ||
      (((*(int *)&arg) & ~0x80000000) == 0)) {
    *r1 = arg;
    *r2 = 0;
  } else {
    /* denormals */
    /* if ( !IML_IS_SIGNIFICAND_ZERO_SP(arg) && (IML_GET_EXP_SP(arg) == 0) )  */
    *r1 = arg * (*(const float *)_TWO_25);
    *r2 = (((_iml_sp_union_t *)&*r1)->bits.exponent) - (0x7F - 1) - 25;
    (*(int *)&*r1) = ((*(int *)&*r1) & ~0x7f800000) | 0x3f000000;
  }
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_frexp_s_xa */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_frexpf(float a, int32_t *c) {
  using namespace __imf_impl_frexp_s_xa;
  VUINT32 vm;
  float va1;
  float vr1;
  VUINT32 vr2;
  float r;
  va1 = a;
  ;
  {
    VUINT32 iX;
    VUINT32 iExp;
    VUINT32 iNewExp;
    VUINT32 iZero;
    VUINT32 iExpMask;
    VUINT32 iBrMask1;
    VUINT32 iBrMask2;
    iExpMask = (__devicelib_imf_internal_sfrexp_data._iExpMask);
    iZero = (__devicelib_imf_internal_sfrexp_data._iZero);
    iNewExp = (__devicelib_imf_internal_sfrexp_data._iNewExp);
    iX = as_uint(va1);
    /* Check for NaN, INF */
    iExp = (iExpMask & iX);
    iBrMask1 = ((VUINT32)(-(VSINT32)((VSINT32)iExpMask == (VSINT32)iExp)));
    /* Check for zero and denormals */
    iBrMask2 = ((VUINT32)(-(VSINT32)((VSINT32)iZero == (VSINT32)iExp)));
    iBrMask1 = (iBrMask1 | iBrMask2);
    vm = 0;
    vm = iBrMask1;
    iExp = (iExp - iNewExp);
    vr2 = ((VSINT32)iExp >> (23));
    iX = (~(iExpMask)&iX);
    iX = (iX | iNewExp);
    vr1 = as_float(iX);
    /* -------------- The end of implementation ---------------- */
  }
  if (__builtin_expect((vm) != 0, 0)) {
    float __cout_a1;
    float __cout_r1;
    int __cout_r2;
    ((float *)&__cout_a1)[0] = va1;
    ((float *)&__cout_r1)[0] = vr1;
    ((VUINT32 *)&__cout_r2)[0] = vr2;
    __devicelib_imf_internal_sfrexp(&__cout_a1, &__cout_r1, &__cout_r2);
    vr1 = ((const float *)&__cout_r1)[0];
    vr2 = ((const VUINT32 *)&__cout_r2)[0];
  }
  r = vr1;
  ((VUINT32 *)c)[0] = vr2;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
