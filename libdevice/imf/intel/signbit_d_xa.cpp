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
namespace __imf_impl_signbit_d_xa {
namespace {} /* namespace */
} /* namespace __imf_impl_signbit_d_xa */
DEVICE_EXTERN_C_INLINE int __devicelib_imf_signbit(double x) {
  using namespace __imf_impl_signbit_d_xa;
  int r;
  VUINT32 vm;
  double va1;
  VUINT32 vr1;
  va1 = x;
  {
    VUINT64 SignMask;
    VUINT64 lArg;
    SignMask = 0x8000000000000000uLL;
    ;
    lArg = as_ulong(va1);
    lArg = (lArg & SignMask);
    vr1 = ((VUINT64)(lArg) >> (63));
  }
  r = vr1;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
