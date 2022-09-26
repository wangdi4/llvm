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
namespace __imf_impl_isfinite_s_xa {
namespace {} /* namespace */
} /* namespace __imf_impl_isfinite_s_xa */
DEVICE_EXTERN_C_INLINE int __devicelib_imf_isfinitef(float x) {
  using namespace __imf_impl_isfinite_s_xa;
  int r;
  VUINT32 vm;
  float va1;
  VUINT32 vr1;
  va1 = x;
  {
    VUINT32 iAbsMask;
    VUINT32 iInf;
    VUINT32 iArg;
    VUINT32 iAbsArg;
    VUINT32 iIsFinite;
    VUINT32 iOne;
    iOne = 0x00000001u;
    iAbsMask = 0x7fffffffu;
    iInf = 0x7f800000u;
    ;
    iArg = as_uint(va1);
    iAbsArg = (iArg & iAbsMask);
    iIsFinite = ((VUINT32)(-(VSINT32)((VSINT32)iAbsArg < (VSINT32)iInf)));
    vr1 = (iOne & iIsFinite);
  }
  r = vr1;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
