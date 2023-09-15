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
namespace __imf_impl_rnorm_s_ep {
namespace {

inline int __devicelib_imf_internal_srnorm(int n, const float *x, float *r) 
{
  int nRet = 0;
  float fres = 0.0f, fsum = 0.0f, fax = 0.0f, famaxx = 0.0f, fscale = 1.0f;
  uint32_t iamaxxexp = 0u, iscale = 0u;
  int i = 0;

  for (i = 0; i < n; i++)
  {
    // Absolute value of argument
    fax = __fabs(x[i]);
    // Return 0.0 if at least one of arguments is +/-INF
    if (as_uint(fax) == 0x7f800000) {
      *r = 0.0f;
      return nRet;
    }
    // Find maximum absolute argument
    famaxx = __fmax(famaxx, fax);
  }

  // Scale all inputs by 2^(-K) where K is maximum absolute argument exponent:
  // Masked exponent field of argument K
  iamaxxexp = (as_uint(famaxx)) & 0x7f800000u;
  // Integer scale 2^(-K+((x >= 2)?1:(-1))): -1 for x less than 2
  iscale = (((iamaxxexp & 0x40000000u) >> 7) + 0x7f000000u) - iamaxxexp;
  // Floating point scale
  fscale = as_float(iscale);

  // Compute sum of squares of scaled absolute arguments
  for (i = 0; i < n; i++)
  {
    fax = __fabs(x[i]) * fscale;
    fsum = __fma(fax, fax, fsum);
  }

  // Result = (1.0/sqrt(sum))*2^(K)
  fres = (1.0f / __sqrt(fsum)) * fscale;
  *r = fres;
  return nRet;
}

} /* namespace */
} /* namespace __imf_impl_rnorm_s_ep */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_rnormf(int n, const float *x) 
{
  using namespace __imf_impl_rnorm_s_ep;
  float r;
  __devicelib_imf_internal_srnorm(n, x, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
