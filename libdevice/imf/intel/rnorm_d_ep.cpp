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
#include "_imf_include_fp64.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_rnorm_d_ep {
namespace {

inline int __devicelib_imf_internal_drnorm(int n, const double *x, double *r)
{
  int nRet = 0;
  double fres = 0.0, fsum = 0.0, fax = 0.0, famaxx = 0.0, fscale = 1.0;
  uint64_t iamaxxexp = 0ul, iscale = 0ul;
  int i = 0;

  for (i = 0; i < n; i++)
  {
    // Absolute value of argument
    fax = __fabs(x[i]);
    // Return 0.0 if at least one of arguments is +/-INF
    if (as_ulong(fax) == 0x7ff0000000000000ul) {
      *r = 0.0f;
      return nRet;
    }
    // Find maximum absolute argument
    famaxx = __fmax(famaxx, fax);
  }

  // Scale all inputs by 2^(-K) where K is maximum absolute argument exponent:
  // Masked exponent field of argument K
  iamaxxexp = (as_ulong(famaxx)) & 0x7ff0000000000000ul;
  // Integer scale 2^(-K+((x >= 2)?1:(-1))): -1 for x less than 2
  iscale = (((iamaxxexp & 0x4000000000000000ul) >> 10) + 0x7fe0000000000000ul) -
           iamaxxexp;
  // doubleing point scale
  fscale = as_double(iscale);

  // Compute sum of squares of scaled absolute arguments
  for (i = 0; i < n; i++)
  {
    fax = __fabs(x[i]) * fscale;
    fsum = __fma(fax, fax, fsum);
  }

  // Result = (1.0/sqrt(sum))*2^(K)
  fres = (1.0 / __sqrt(fsum)) * fscale;
  *r = fres;
  return nRet;
}

} /* namespace */
} /* namespace __imf_impl_rnorm_d_ep */

DEVICE_EXTERN_C_INLINE double __devicelib_imf_rnorm(int n, const double *x)
{
  using namespace __imf_impl_rnorm_d_ep;
  double r;
  __devicelib_imf_internal_drnorm(n, x, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
