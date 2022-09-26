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
namespace __imf_impl_modf_d_xa {
namespace {
static const _iml_dp_union_t _zeros[] = {0x00000000, 0x00000000, 0x00000000,
                                         0x80000000};
inline int __devicelib_imf_internal_dmodf(const double *a, double *r1,
                                          double *r2) {
  int nRet = 0;
  int sign, fraction;
  unsigned ix, mask;
  double result, result2;
  double *iptr = &result2;
  double x = (*a);
  ix = (((int *)&x)[1] & ~0x80000000);
  if (ix - 0x3ff00000 < 0x43300000 - 0x3ff00000) { /* 1.0 <= |x| < 2^52 */
    if (ix < 0x41400000) {                         /* |x| <= 2^20 */
      mask = (0xffffffff << ((0x03FF + 20) - (ix >> 20)));
      fraction = (((int *)&x)[1] & ~mask) | ((int *)&x)[0];
      ((int *)&*iptr)[1] = (((int *)&x)[1] & mask);
      ((int *)&*iptr)[0] = 0;
    } else { /* |x| >= 2^21 */
      mask = (0xffffffff << ((0x03FF + 52) - (ix >> 20)));
      fraction = (((int *)&x)[0] & ~mask);
      ((int *)&*iptr)[0] = (((int *)&x)[0] & mask);
      ((int *)&*iptr)[1] = ((int *)&x)[1];
    }
    if (fraction) { /* non-zero result */
      result = (x - *iptr);
      (*r1) = result;
      (*r2) = result2;
      return nRet;
    } else { /* zero with proper sign */
      sign = ((unsigned)((int *)&x)[1] >> 31);
      result = ((const double *)_zeros)[sign];
      (*r1) = result;
      (*r2) = result2;
      return nRet;
    }
  } else { /* x - NaN,INF,or great integer */
    sign = ((unsigned)((int *)&x)[1] >> 31);
    if ((ix >= 0x7ff00000) &&
        (((ix - 0x7ff00000) | ((int *)&x)[0]) != 0)) { /* x - NaN */
      (*iptr = x + x);
      result = *iptr;
      (*r1) = result;
      (*r2) = result2;
      return nRet;                /* raise invalid for SNaN, return QNaN */
    } else if (ix > 0x3ff00000) { /* x - INF or great integer */
      *iptr = x;
      result = ((const double *)_zeros)[sign];
      (*r1) = result;
      (*r2) = result2;
      return nRet;
    } else { /* |x| < 1.0 */
      *iptr = ((const double *)_zeros)[sign];
      result = x;
      (*r1) = result;
      (*r2) = result2;
      return nRet;
    }
  }
  (*r1) = result;
  (*r2) = result2;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_modf_d_xa */
DEVICE_EXTERN_C_INLINE double __devicelib_imf_modf(double x, double *z) {
  using namespace __imf_impl_modf_d_xa;
  double r;
  VUINT32 vm;
  double va1;
  double vr1;
  double vr2;
  va1 = x;
  {
    VUINT32 iInf;
    VUINT32 iX;
    VUINT64 lX;
    double dSgnX;
    iInf = 0x7ff00000u;
    dSgnX = as_double(0x8000000000000000uLL);
    lX = as_ulong(va1);
    iX = ((VUINT32)((VUINT64)lX >> 32));
    iX = (iX & iInf);
    iX = ((VUINT32)(-(VSINT32)((VSINT32)iX == (VSINT32)iInf)));
    vm = 0;
    vm = iX;
    vr2 = __trunc(va1);
    dSgnX = as_double((as_ulong(dSgnX) & as_ulong(va1)));
    vr1 = (va1 - vr2);
    vr1 = as_double((as_ulong(vr1) | as_ulong(dSgnX)));
  }
  if (__builtin_expect((vm) != 0, 0)) {
    double __cout_a1;
    double __cout_r1;
    double __cout_r2;
    ((double *)&__cout_a1)[0] = va1;
    ((double *)&__cout_r1)[0] = vr1;
    ((double *)&__cout_r2)[0] = vr2;
    __devicelib_imf_internal_dmodf(&__cout_a1, &__cout_r1, &__cout_r2);
    vr1 = ((const double *)&__cout_r1)[0];
    vr2 = ((const double *)&__cout_r2)[0];
  }
  r = vr1;
  ((double *)z)[0] = vr2;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
