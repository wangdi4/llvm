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
namespace __imf_impl_sincospi_s_la {
namespace {
typedef struct {
  VUINT32 _AbsMask;
  VUINT32 _Half;
  VUINT32 _SgnMask;
  VUINT32 _zero;
  VUINT32 _c4;
  VUINT32 _c3;
  VUINT32 _c2;
  VUINT32 _c1;
  VUINT32 _c0;
} __devicelib_imf_internal_ssincospi_data_t;
static const __devicelib_imf_internal_ssincospi_data_t
    __devicelib_imf_internal_ssincospi_data = {
        0x7FFFFFFFu, /* absolute value mask  */
        0x3F000000u, /* 0.5f */
        0x80000000u, /* sgn mask */
        0x00000000u, /* zero */
        0x3d1f0000u, /* c4 */
        0xbe9929adu, /* c3 */
        0x3fa33479u, /* c2 */
        0xbfcabbc3u, /* c1 */
        0x3f121fb5u, /* c0 */
};                   /*sSinCosPi_Table*/

} /* namespace */
} /* namespace __imf_impl_sincospi_s_la */

DEVICE_EXTERN_C_INLINE void __devicelib_imf_sincospif(float x, float *y,
                                                      float *z)
{
  using namespace __imf_impl_sincospi_s_la;
  VUINT32 vm;
  float va1;
  float vr1;
  float vr2;
  va1 = x;
  {
    float AbsMask;
    float fN;
    VUINT32 iN;
    float Rs;
    float Rs2;
    float Rc;
    float Rc2;
    float aRs;
    float Half;
    float Ls;
    float Lc;
    float c4;
    float c3;
    float c2;
    float c1;
    float c0;
    float sgn_N;
    float spoly;
    float cpoly;
    float zero;
    float sgn_x;
    float sgn_c;
    float sgn_s;
    float SgnMask;
    float Rs_msk;
    float Rc_msk;
    vm = 0;
    AbsMask = as_float(__devicelib_imf_internal_ssincospi_data._AbsMask);
    // fN = round_to_int(x)
    fN = __rint(va1);
    // reduced sinpi argument
    Rs = (va1 - fN);
    // to deal with large |x| on Gen, use -fabs(fN) instead of fN here:
    iN = ((VINT32)((-__fabs(fN))));
    // prepare reduced argument for cospi
    Half = as_float(__devicelib_imf_internal_ssincospi_data._Half);
    aRs = as_float((as_uint(Rs) & as_uint(AbsMask)));
    Rc = (Half - aRs);
    Rs2 = (Rs * Rs);
    Rc2 = (Rc * Rc);
    // sign, based on odd(iN)
    iN = ((VUINT32)(iN) << (31));
    sgn_N = as_float(iN);
    // polynomial coefficients
    c4 = as_float(__devicelib_imf_internal_ssincospi_data._c4);
    c3 = as_float(__devicelib_imf_internal_ssincospi_data._c3);
    c2 = as_float(__devicelib_imf_internal_ssincospi_data._c2);
    // polynomials
    spoly = __fma(c4, Rs2, c3);
    cpoly = __fma(c4, Rc2, c3);
    spoly = __fma(spoly, Rs2, c2);
    cpoly = __fma(cpoly, Rc2, c2);
    // prepare sign
    zero = as_float(__devicelib_imf_internal_ssincospi_data._zero);
    SgnMask = as_float(__devicelib_imf_internal_ssincospi_data._SgnMask);
    Rs_msk = as_float(((VUINT32)(-(VSINT32)(!(Rs == zero)))));
    sgn_x = as_float((as_uint(va1) & as_uint(SgnMask)));
    Rc_msk = as_float(((VUINT32)(-(VSINT32)(!(Rc == zero)))));
    sgn_s = as_float((((~as_uint(Rs_msk)) & as_uint(sgn_x)) |
                      (as_uint(Rs_msk) & as_uint(sgn_N))));
    sgn_c = as_float((as_uint(sgn_N) & as_uint(Rc_msk)));
    c1 = as_float(__devicelib_imf_internal_ssincospi_data._c1);
    c0 = as_float(__devicelib_imf_internal_ssincospi_data._c0);
    // polynomial evaluation
    spoly = __fma(spoly, Rs2, c1);
    cpoly = __fma(cpoly, Rc2, c1);
    spoly = __fma(spoly, Rs2, c0);
    cpoly = __fma(cpoly, Rc2, c0);
    // CUDA-compliant build: more accurate polynomial evaluation (with modified
    // coefficients)
    vr1 = __fma(-(Rs), Rs, spoly);
    vr2 = __fma(-(Rc), Rc, cpoly);
    Ls = (Rs + Rs);
    Lc = (Rc + Rc);
    vr1 = __fma(vr1, Ls, Ls);
    vr2 = __fma(vr2, Lc, Lc);
    vr1 = as_float((as_uint(vr1) ^ as_uint(sgn_s)));
    vr2 = as_float((as_uint(vr2) ^ as_uint(sgn_c)));
  }
  ((float *)y)[0] = vr1;
  ((float *)z)[0] = vr2;
  return;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
