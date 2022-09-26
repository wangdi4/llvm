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
/*
// ALGORITHM DESCRIPTION:
//
//   gentype nan(uintn nancode) function returns a quiet NaN. The nancode may
//                           be placed in the significand of the resulting NaN
//                           as described in [1].
//
//   All binary NaN bit strings have all the bits of the biased exponent field
//   set to 1. A quiet NaN bit string should be encoded with the first bit (d1)
//   of the trailing significand field being 1. A signaling NaN bit string
should
//   be encoded with the first bit of the trailing significand field being 0. If
//   the first bit of the trailing significand field is 0, some other bit of the
//   trailing significand field must be non-zero to distinguish the NaN from
//   infinity. In the preferred encoding just described, a signaling NaN shall
be
//   quieted by setting d1 to 1, leaving the remaining bits unchanged.
//   For binary formats, the payload is encoded in the p - 2 least significant
//   bits of the trailing significand field. See [2].
//
//   Implementation is as follows:
//     return bits of nancode | 0x7ff8000000000000
//                                         for double precision
//     return bits of nancode | 0x7fc00000 for single precision
//                                         leaving sign and payload intact,
//                                         ensuring exponent is all ones, MSB
//                                         in significant is one.
//
//   References:
//       [1] The OpenCL specification version 1.0, rev. 43
//       [2] IEEE Std 754-2008, IEEE Standard for Floating-Point Arithmetic
// --
//
*/
#include "_imf_include_fp32.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_nan_s_xa {
namespace {} /* namespace */
} /* namespace __imf_impl_nan_s_xa */
DEVICE_EXTERN_C_INLINE float __devicelib_imf_nanf(const char *x) {
  using namespace __imf_impl_nan_s_xa;
  float r;
  VUINT32 vm;
  VUINT32 va1;
  float vr1;
  va1 = *((VUINT32 *)(x));
  {
    VUINT32 iMask;
    VUINT32 iX;
    iMask = 0x7fc00000u;
    iX = (iMask | va1);
    vr1 = as_float(iX);
  }
  r = vr1;
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
