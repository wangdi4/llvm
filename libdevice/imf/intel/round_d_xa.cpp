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
namespace __imf_impl_round_d_xa {
namespace {
typedef struct {
  VUINT64 _dSignMask;
  VUINT64 _dOneHalf;
  VUINT64 _dOne;
  VUINT64 _d2p52;
} __devicelib_imf_internal_dround_data_t;
static const __devicelib_imf_internal_dround_data_t
    __devicelib_imf_internal_dround_data = {
        0x8000000000000000uL, /* _dSignMask */
        0x3fe0000000000000uL, /* _dOneHalf  */
        0x3ff0000000000000uL, /* _dOne  */
        0x4330000000000000uL  /* _d2p52  */
};                            /*dRound_Table*/
} /* namespace */
} /* namespace __imf_impl_round_d_xa */
DEVICE_EXTERN_C_INLINE double __devicelib_imf_round(double x) {
  using namespace __imf_impl_round_d_xa;
  double r;
  VUINT32 vm;
  double va1;
  double vr1;
  va1 = x;
  {
    double dSign;
    double dX;
    double dRoundX;
    double dFractionX;
    double dIsOneHalf;
    double dAddOne;
    double dSignMask;
    double dOneHalf;
    double dOne;
    double d2p52;
    double dRange;
    /* -------------------- Implementation --------------------- */
    dSignMask = as_double(__devicelib_imf_internal_dround_data._dSignMask);
    dSign = as_double((as_ulong(dSignMask) & as_ulong(va1)));
    dX = as_double((~(as_ulong(dSignMask)) & as_ulong(va1)));
    vm = 0;
    dRoundX = __rint(dX);
    dFractionX = (dX - dRoundX);
    dOneHalf = as_double(__devicelib_imf_internal_dround_data._dOneHalf);
    dIsOneHalf = as_double(
        (VUINT64)((dFractionX == dOneHalf) ? 0xffffffffffffffff : 0x0));
    dOne = as_double(__devicelib_imf_internal_dround_data._dOne);
    dAddOne = as_double((as_ulong(dIsOneHalf) & as_ulong(dOne)));
    vr1 = (dRoundX + dAddOne);
    vr1 = as_double((as_ulong(vr1) | as_ulong(dSign)));
    /* -------------- The end of implementation ---------------- */
  }
  r = vr1;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
