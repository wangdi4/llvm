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
namespace __imf_impl_fdim_d_ha {
namespace {
typedef struct {
  VUINT32 _iExpMask;
  VUINT32 _iBias;
  VUINT64 _dPosZero;
} __devicelib_imf_internal_dfdim_data_t;
static const __devicelib_imf_internal_dfdim_data_t
    __devicelib_imf_internal_dfdim_data = {0x7ff00000u, 0x000003ffu,
                                           0x0000000000000000uL};
static const double ones[2] = {1.0, -1.0};
inline int __devicelib_imf_internal_dfdim(const double *a1, const double *a2,
                                          double *r1) {
  int nRet = 0;
  if ((((((_iml_dp_union_t *)&*a1)->bits.exponent) == 0x07FF) &&
       (((((_iml_dp_union_t *)&*a1)->bits.hi_significand) != 0) ||
        ((((_iml_dp_union_t *)&*a1)->bits.lo_significand) != 0)))) {
    *r1 = (*a1 * ones[0]);
  }
  if ((((((_iml_dp_union_t *)&*a2)->bits.exponent) == 0x07FF) &&
       (((((_iml_dp_union_t *)&*a2)->bits.hi_significand) != 0) ||
        ((((_iml_dp_union_t *)&*a2)->bits.lo_significand) != 0)))) {
    *r1 = (*a2 * ones[0]);
  }
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_fdim_d_ha */
DEVICE_EXTERN_C_INLINE double __devicelib_imf_fdim(double x, double y) {
  using namespace __imf_impl_fdim_d_ha;
  double r;
  VUINT32 vm;
  double va1;
  double va2;
  double vr1;
  va1 = x;
  va2 = y;
  {
    VUINT64 lX;
    VUINT64 lY;
    VUINT32 iHiX;
    VUINT32 iHiY;
    double dPosZero;
    double dDiff;
    double dCmpMask;
    VUINT32 iExpMask;
    VUINT32 iBrMask1;
    VUINT32 iBrMask2;
    iExpMask = (__devicelib_imf_internal_dfdim_data._iExpMask);
    dPosZero = as_double(__devicelib_imf_internal_dfdim_data._dPosZero);
    lX = as_ulong(va1);
    lY = as_ulong(va2);
    iHiX = ((VUINT32)((VUINT64)lX >> 32));
    iHiY = ((VUINT32)((VUINT64)lY >> 32));
    /* check for NaN */
    iHiX = (iExpMask & iHiX);
    iBrMask1 = ((VUINT32)(-(VSINT32)((VSINT32)iExpMask == (VSINT32)iHiX)));
    iHiY = (iExpMask & iHiY);
    iBrMask2 = ((VUINT32)(-(VSINT32)((VSINT32)iExpMask == (VSINT32)iHiY)));
    iBrMask1 = (iBrMask1 | iBrMask2);
    vm = 0;
    vm = iBrMask1;
    dCmpMask = as_double((VUINT64)((va1 > va2) ? 0xffffffffffffffff : 0x0));
    dDiff = (va1 - va2);
    vr1 = as_double((((~as_ulong(dCmpMask)) & as_ulong(dPosZero)) |
                     (as_ulong(dCmpMask) & as_ulong(dDiff))));
    /* -------------- The end of implementation ---------------- */
  }
  if (__builtin_expect((vm) != 0, 0)) {
    double __cout_a1;
    double __cout_a2;
    double __cout_r1;
    ((double *)&__cout_a1)[0] = va1;
    ((double *)&__cout_a2)[0] = va2;
    ((double *)&__cout_r1)[0] = vr1;
    __devicelib_imf_internal_dfdim(&__cout_a1, &__cout_a2, &__cout_r1);
    vr1 = ((const double *)&__cout_r1)[0];
  }
  r = vr1;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
