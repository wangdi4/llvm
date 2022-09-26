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
namespace __imf_impl_llrint_d_xa {
namespace {} /* namespace */
} /* namespace __imf_impl_llrint_d_xa */
DEVICE_EXTERN_C_INLINE int64_t __devicelib_imf_llrint(double x) {
  using namespace __imf_impl_llrint_d_xa;
  int64_t r;
  VUINT32 vm;
  double va1;
  VUINT64 vr1;
  va1 = x;
  {
    VUINT64 lExpMask;
    VUINT64 lIndRes;
    VUINT64 lRes;
    VUINT64 lArgExp;
    VUINT64 lMaxExp;
    double dRes;
    vm = 0;
    lExpMask = 0x7ff0000000000000uLL;
    lMaxExp = 0x43e0000000000000uLL;
    lIndRes = 0x8000000000000000uLL;
    // If |x| >= 2^63 then return indefinite (0x8000000000000000)
    lArgExp = as_ulong(va1);
    lArgExp = (lArgExp & lExpMask);
    lExpMask = ((VUINT64)(-(VSINT64)((VSINT64)lArgExp >= (VSINT64)lMaxExp)));
    // If |x| < 2^63 then return (long int)rint(x)
    dRes = __rint(va1);
    lRes = ((VINT64)(dRes));
    // Blend main and special paths
    vr1 = (((~(lExpMask)) & (lRes)) | ((lExpMask) & (lIndRes)));
  }
  r = vr1;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
