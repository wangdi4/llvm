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
namespace __imf_impl_round_s_xa {
namespace {
typedef struct {
  VUINT32 _sSignMask;
  VUINT32 _sOneHalf;
  VUINT32 _sOne;
  VUINT32 _s2p23;
} __devicelib_imf_internal_sround_data_t;
static const __devicelib_imf_internal_sround_data_t
    __devicelib_imf_internal_sround_data = {
        0x80000000u, /* _sSignMask */
        0x3f000000u, /* _sOneHalf  */
        0x3f800000u, /* _sOne  */
        0x4b000000u  /* _s2p23  */
};
} /* namespace */
} /* namespace __imf_impl_round_s_xa */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_roundf(float a) {
  using namespace __imf_impl_round_s_xa;
  VUINT32 vm;
  float va1;
  float vr1;
  float r;
  va1 = a;
  ;
  {
    float sSign;
    float sSignedFractionX;
    float sX;
    float sRoundX;
    float sFractionX;
    float sIsOneHalf;
    float sAddOne;
    float s2p23;
    float sRange;
    /* Constants */
    float sSignMask;
    float sOneHalf;
    float sOne;
    /* -------------------- Implementation --------------------- */
    vm = 0;
    sSignMask = as_float(__devicelib_imf_internal_sround_data._sSignMask);
    sSign = as_float((as_uint(sSignMask) & as_uint(va1)));
    sX = as_float((~(as_uint(sSignMask)) & as_uint(va1)));
    sRoundX = __rint(sX);
    sFractionX = (sX - sRoundX);
    sOneHalf = as_float(__devicelib_imf_internal_sround_data._sOneHalf);
    sIsOneHalf = as_float(((VUINT32)(-(VSINT32)(sFractionX == sOneHalf))));
    sOne = as_float(__devicelib_imf_internal_sround_data._sOne);
    sAddOne = as_float((as_uint(sIsOneHalf) & as_uint(sOne)));
    vr1 = (sRoundX + sAddOne);
    vr1 = as_float((as_uint(vr1) | as_uint(sSign)));
    /* -------------- The end of implementation ---------------- */
  }
  r = vr1;
  ;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
