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
namespace __imf_impl_fdim_s_ha {
namespace {
typedef struct {
  VUINT32 _iExpMask;
  VUINT32 _iBias;
  VUINT32 _sPosZero;
} __devicelib_imf_internal_sfdim_data_t;
static const __devicelib_imf_internal_sfdim_data_t
    __devicelib_imf_internal_sfdim_data = {0x7f800000u, 0x0000007fu,
                                           0x00000000u};
static const float ones[2] = {1.0, -1.0};
inline int __devicelib_imf_internal_sfdim(const float *a1, const float *a2,
                                          float *r1) {
  int nRet = 0;
  if ((((((_iml_sp_union_t *)&*a1)->bits.exponent) == 0x00FF) &&
       ((((_iml_sp_union_t *)&*a1)->bits.significand) != 0))) {
    *r1 = (*a1 * ones[0]);
  }
  if ((((((_iml_sp_union_t *)&*a2)->bits.exponent) == 0x00FF) &&
       ((((_iml_sp_union_t *)&*a2)->bits.significand) != 0))) {
    *r1 = (*a2 * ones[0]);
  }
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_fdim_s_ha */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_fdimf(float a, float b) {
  using namespace __imf_impl_fdim_s_ha;
  VUINT32 vm;
  float va1;
  float va2;
  float vr1;
  float r;
  va1 = a;
  va2 = b;
  {
    VUINT32 iX;
    VUINT32 iY;
    float sPosZero;
    float sDiff;
    float sCmpMask;
    VUINT32 iExpMask;
    VUINT32 iBrMask1;
    VUINT32 iBrMask2;
    iExpMask = (__devicelib_imf_internal_sfdim_data._iExpMask);
    sPosZero = as_float(__devicelib_imf_internal_sfdim_data._sPosZero);
    iX = as_uint(va1);
    iY = as_uint(va2);
    /* check for NaN */
    iX = (iExpMask & iX);
    iBrMask1 = ((VUINT32)(-(VSINT32)((VSINT32)iExpMask == (VSINT32)iX)));
    iY = (iExpMask & iY);
    iBrMask2 = ((VUINT32)(-(VSINT32)((VSINT32)iExpMask == (VSINT32)iY)));
    iBrMask1 = (iBrMask1 | iBrMask2);
    vm = 0;
    vm = iBrMask1;
    sCmpMask = as_float(((VUINT32)(-(VSINT32)(va1 > va2))));
    sDiff = (va1 - va2);
    vr1 = as_float((((~as_uint(sCmpMask)) & as_uint(sPosZero)) |
                    (as_uint(sCmpMask) & as_uint(sDiff))));
    /* -------------- The end of implementation ---------------- */
  }
  if (__builtin_expect((vm) != 0, 0)) {
    float __cout_a1;
    float __cout_a2;
    float __cout_r1;
    ((float *)&__cout_a1)[0] = va1;
    ((float *)&__cout_a2)[0] = va2;
    ((float *)&__cout_r1)[0] = vr1;
    __devicelib_imf_internal_sfdim(&__cout_a1, &__cout_a2, &__cout_r1);
    vr1 = ((const float *)&__cout_r1)[0];
  }
  r = vr1;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
