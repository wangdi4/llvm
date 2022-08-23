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
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_hypot_s_la {
namespace {
typedef struct {
  VUINT32 _sHiLoMask;
  VUINT32 _sS2HiLoMask;
  VUINT32 _sAbsMask;
  VUINT32 _sHalf;
  VUINT32 _ExpMask;
  VUINT32 _LargeDiffMask;
  VUINT32 _LowBoundary;
  VUINT32 _HighBoundary;
  VUINT32 _iExpBound;
} __devicelib_imf_internal_shypot_data_t;
static const __devicelib_imf_internal_shypot_data_t
    __devicelib_imf_internal_shypot_data = {
        /* legacy algorithm */
        0xFFF80000u, /* _sHiLoMask     */
        0xFFFFe000u, /* _sS2HiLoMask   */
        0x7fffffffu, /* _sAbsMask      */
        0x3f000000u, /* _sHalf         */
        0x7f800000u, /* _ExpMask       */
        0x02000000u, /* _LargeDiffMask */
        0x1E300000u, /* _LowBoundary   */
        0x60A00000u, /* _HighBoundary  */
        /* fma based algorithm*/
        0x427C0000u, /* _iExpBound     */
};                   /* _VAPI_DATA_TYPE */
inline int __devicelib_imf_internal_shypot(const float *pa, const float *pb,
                                           float *pres) {
  int nRet = 0;
  float x = *pa, y = *pb;
  union {
    uint32_t w;
    float f;
  } ax, ay, expon0, res;
  float h12, l12, t0, t1, sum;
  float RS, Sh, RS2, eps;
  // take absolute values
  ax.f = x;
  ax.w &= 0x7fffffffu;
  ay.f = y;
  ay.w &= 0x7fffffffu;
  // eliminate special cases
  if ((ax.w >= 0x7f800000) || (ay.w >= 0x7f800000)) {
    if ((ax.w == 0x7f800000) || (ay.w == 0x7f800000)) {
      res.w = 0x7f800000;
      *pres = res.f;
      return nRet;
    }
    // return NaN
    *pres = x + y;
    return nRet;
  }
  // Order inputs based on absolute value
  h12 = __fmax(ax.f, ay.f);
  l12 = __fmin(ax.f, ay.f);
  // scale inputs
  expon0.f = h12;
  expon0.w &= 0x7f800000;
  // 2^(-expon_t0+ ((t0>=2)?1:(-1)))
  expon0.w = 0x7e800000 + ((expon0.w & 0x40000000) >> 6) - expon0.w;
  t0 = __fma(h12, expon0.f, 0.0f);
  t1 = __fma(l12, expon0.f, 0.0f);
  sum = __fma(t1, t1, 0.0f);
  sum = __fma(t0, t0, sum);
  RS = 1.0f / __sqrt(sum);
  // Sh ~ sqrt(sum)
  Sh = __fma(sum, RS, 0.0f);
  RS2 = __fma(RS, 0.5f, 0.0f);
  // sum - Sh^2
  eps = __fma(Sh, -Sh, sum);
  // sqrt(sum) ~ Sh + eps*RS2
  res.f = __fma(RS2, eps, Sh);
  // 2^(-expon0)
  expon0.w = 0x7f000000 - expon0.w;
  res.f = __fma(res.f, expon0.f, 0.0f);
  res.w = (sum != 0.0f) ? res.w : 0;
  *pres = res.f;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_hypot_s_la */
DEVICE_EXTERN_C_INLINE float __devicelib_imf_hypotf(float x, float y) {
  using namespace __imf_impl_hypot_s_la;
  float r;
  __devicelib_imf_internal_shypot(&x, &y, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
