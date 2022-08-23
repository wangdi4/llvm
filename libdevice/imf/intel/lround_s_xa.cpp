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
namespace __imf_impl_lround_s_xa {
namespace {
typedef struct {
  VUINT32 _sSignMask;
  VUINT32 _sOneHalf;
  VUINT32 _sOne;
  VUINT32 _s2p23;
} __devicelib_imf_internal_slround_data_t;
static const __devicelib_imf_internal_slround_data_t
    __devicelib_imf_internal_slround_data = {
        0x80000000u, /* _sSignMask */
        0x3f000000u, /* _sOneHalf  */
        0x3f800000u, /* _sOne  */
        0x4b000000u  /* _s2p23  */
};
} /* namespace */
} /* namespace __imf_impl_lround_s_xa */
DEVICE_EXTERN_C_INLINE int64_t __devicelib_imf_lroundf(float x) {
  using namespace __imf_impl_lround_s_xa;
  int64_t r;
  VUINT32 vm;
  float va1;
  VUINT64 vr1;
  va1 = x;
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
    float sSignMask;
    float sOneHalf;
    float sOne;
    VUINT64 lExpMask;
    VUINT64 lIndRes;
    VUINT64 lRes;
    VUINT32 iArgExp;
    VUINT32 iExpMask;
    VUINT32 iMaxExp;
    float sRes;
    vm = 0;
    iExpMask = 0x7f800000u;
    iMaxExp = 0x5f000000u;
    lIndRes = 0x8000000000000000uLL;
    // If |x| >= 2^63 then return indefinite (0x8000000000000000)
    iArgExp = as_uint(va1);
    iArgExp = (iArgExp & iExpMask);
    iExpMask = ((VUINT32)(-(VSINT32)((VSINT32)iArgExp >= (VSINT32)iMaxExp)));
    lExpMask =
        (((VUINT64)(VUINT32)iExpMask << 32) | (VUINT64)(VUINT32)iExpMask);
    sSignMask = as_float(__devicelib_imf_internal_slround_data._sSignMask);
    sSign = as_float((as_uint(sSignMask) & as_uint(va1)));
    sX = as_float((~(as_uint(sSignMask)) & as_uint(va1)));
    sRoundX = __rint(sX);
    sFractionX = (sX - sRoundX);
    sOneHalf = as_float(__devicelib_imf_internal_slround_data._sOneHalf);
    sIsOneHalf = as_float(((VUINT32)(-(VSINT32)(sFractionX == sOneHalf))));
    sOne = as_float(__devicelib_imf_internal_slround_data._sOne);
    sAddOne = as_float((as_uint(sIsOneHalf) & as_uint(sOne)));
    sRes = (sRoundX + sAddOne);
    sRes = as_float((as_uint(sRes) | as_uint(sSign)));
    lRes = ((VINT64)(sRes));
    // Blend main and special paths
    vr1 = (((~(lExpMask)) & (lRes)) | ((lExpMask) & (lIndRes)));
  }
  r = vr1;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
