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
namespace __imf_impl_modf_s_xa {
namespace {
static const unsigned int _zeros[] = {0x00000000, 0x80000000};
inline int __devicelib_imf_internal_smodf(const float *a, float *r1,
                                          float *r2) {
  int nRet = 0;
  int sign;
  unsigned ix;
  float result, result2;
  float *iptr = &result2;
  float fx = *a;
  ix = (((_iml_sp_union_t *)&fx)->hex[0] & ~0x80000000);
  /* x - NaN,INF */
  if (ix > 0x7f800000) { /* x - NaN */
    /* raise invalid for SNaN, return QNaN */
    (*iptr = fx + fx);
    result = *iptr;
    (*r1) = (float)result;
    (*r2) = (float)result2;
    return nRet;
  } else { /* x - INF */
    sign = (((_iml_sp_union_t *)&fx)->bits.sign);
    *iptr = fx;
    result = ((const float *)_zeros)[sign];
    (*r1) = (float)result;
    (*r2) = (float)result2;
    return nRet;
  }
  (*r1) = (float)result;
  (*r2) = (float)result2;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_modf_s_xa */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_modff(float a, float *c) {
  using namespace __imf_impl_modf_s_xa;
  VUINT32 vm;
  float va1;
  float vr1;
  float vr2;
  float r;
  va1 = a;
  ;
  {
    VUINT32 iInf;
    VUINT32 iX;
    float sSgnX;
    iInf = 0x7f800000u;
    sSgnX = as_float(0x80000000u);
    iX = as_uint(va1);
    iX = (iX & iInf);
    iX = ((VUINT32)(-(VSINT32)((VSINT32)iX == (VSINT32)iInf)));
    vm = 0;
    vm = iX;
    vr2 = __trunc(va1);
    sSgnX = as_float((as_uint(sSgnX) & as_uint(va1)));
    vr1 = (va1 - vr2);
    vr1 = as_float((as_uint(vr1) | as_uint(sSgnX)));
  }
  if (__builtin_expect((vm) != 0, 0)) {
    float __cout_a1;
    float __cout_r1;
    float __cout_r2;
    ((float *)&__cout_a1)[0] = va1;
    ((float *)&__cout_r1)[0] = vr1;
    ((float *)&__cout_r2)[0] = vr2;
    __devicelib_imf_internal_smodf(&__cout_a1, &__cout_r1, &__cout_r2);
    vr1 = ((const float *)&__cout_r1)[0];
    vr2 = ((const float *)&__cout_r2)[0];
  }
  r = vr1;
  ((float *)c)[0] = vr2;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
