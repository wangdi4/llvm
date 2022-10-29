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
#if defined(INTEL_COLLAB) && defined(OMP_LIBDEVICE)
#pragma omp declare target
#endif
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_logb_s_xa {
namespace {
typedef struct {
  VUINT32 _iExpMask;
  VUINT32 _iZero;
  VUINT32 _iBias;
} __devicelib_imf_internal_slogb_data_t;
static const __devicelib_imf_internal_slogb_data_t
    __devicelib_imf_internal_slogb_data = {0x7f800000u, 0x00000000u,
                                           0x0000007fu};
static const int _TWO_25[] = {0x4c000000}; /* 2^25 */
static const float _one_ = 1.0;
inline int __devicelib_imf_internal_slogb(const float *a, float *r1) {
  int nRet = 0;
  float arg;
/*
//   
//    * For DAZ zero argument
//    * For SNaN raise invalid
//    
*/
  arg = ((*a) * (_one_));
/*
//   
//    * For zero return -INF, and raise divide-by-zero
//    
*/
  if ((((*(int *)&(arg)) & ~0x80000000) == 0)) {
    nRet = 1;
    *r1 = 0.0;
    *r1 = (-(_one_) / (*r1));
  }
/*
//   
//    * For NaNs returns QNaN, for
//    * For INF  returns +INF
//    
*/
  else if (!((((_iml_sp_union_t *)&arg)->bits.exponent) != 0xFF)) {
    (*(int *)&(*r1)) = (*(int *)&(arg)) & ~0x80000000;
  }
/*
//   
//    * Denormals
//    
*/
  else if (!((((_iml_sp_union_t *)&arg)->bits.significand) == 0) &&
           ((((_iml_sp_union_t *)&arg)->bits.exponent) == 0)) {
    arg = arg * (*(const float *)_TWO_25);
    *r1 = (float)((int)(((_iml_sp_union_t *)&arg)->bits.exponent) - 0x7F - 25);
  }
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_logb_s_xa */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_logbf(float a) {
  using namespace __imf_impl_logb_s_xa;
  VUINT32 vm;
  float va1;
  float vr1;
  float r;
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
    iExpMask = (__devicelib_imf_internal_slogb_data._iExpMask);
    iZero = (__devicelib_imf_internal_slogb_data._iZero);
    iBias = (__devicelib_imf_internal_slogb_data._iBias);
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
    iExp = (iExp - iBias);
    vr1 = ((float)((VINT32)(iExp)));
    /* -------------- The end of implementation ---------------- */
  }
  if (__builtin_expect((vm) != 0, 0)) {
    float __cout_a1;
    float __cout_r1;
    ((float *)&__cout_a1)[0] = va1;
    ((float *)&__cout_r1)[0] = vr1;
    __devicelib_imf_internal_slogb(&__cout_a1, &__cout_r1);
    vr1 = ((const float *)&__cout_r1)[0];
  }
  r = vr1;
  ;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
#if defined(INTEL_COLLAB) && defined(OMP_LIBDEVICE)
DEVICE_EXTERN_C_DECLSIMD_INLINE
float __svml_device_logbf(float x) { return __devicelib_imf_logbf(x); }
#pragma omp end declare target
#endif
