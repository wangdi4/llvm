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
namespace __imf_impl_sincospi_d_la {
namespace {
typedef struct {
  VUINT64 _AbsMask;
  VUINT64 _Shifter;
  VUINT64 _Two53;
  VUINT64 _Half;
  VUINT64 _SgnMask;
  VUINT64 _zero;
  VUINT64 _c8;
  VUINT64 _c7;
  VUINT64 _c6;
  VUINT64 _c5;
  VUINT64 _c4;
  VUINT64 _c3;
  VUINT64 _c2;
  VUINT64 _c1;
  VUINT64 _c0;
  VUINT64 _cx;
  VUINT64 _one;
} __devicelib_imf_internal_dsincospi_data_t;
static const __devicelib_imf_internal_dsincospi_data_t
    __devicelib_imf_internal_dsincospi_data = {
        0x7FFFFFFFFFFFFFFFuL, /* absolute value mask  */
        0x4330000000000000uL, /* Shifter */
        0x4340000000000000uL, /* 2^53 */
        0x3FE0000000000000uL, /* 0.5 */
        0x8000000000000000uL, /* sgn mask */
        0x0000000000000000uL, /* zero */
        0x3ea9d46b06ce620euL, /* c8 */
        0xbef6f7ad23b5cd51uL, /* c7 */
        0x3f3e8f3677c334d3uL, /* c6 */
        0xbf7e3074dfb5bb14uL, /* c5 */
        0x3fb50783485523f4uL, /* c4 */
        0xbfe32d2cce627c9euL, /* c3 */
        0x400466bc6775aa7duL, /* c2 */
        0xc014abbce625be52uL, /* c1 */
        0x400921fb54442d18uL, /* c0 */
        0x400466bc6775aa7cuL, /* c02 */
        0x3ff0000000000000uL, /* c5 */
};                            /*sSinCosPi_Table*/
} /* namespace */
} /* namespace __imf_impl_sincospi_d_la */
DEVICE_EXTERN_C_INLINE void __devicelib_imf_sincospi(double x, double *y,
                                                     double *z) {
  using namespace __imf_impl_sincospi_d_la;
  ;
  VUINT32 vm;
  double va1;
  double vr1;
  double vr2;
  va1 = x;
  ;
  {
    double AbsMask;
    double fN;
    double Rs;
    double Rs2;
    double Rc;
    double Rc2;
    double aRs;
    double aN;
    double Half;
    double c8;
    double c7;
    double c6;
    double c5;
    double c4;
    double c3;
    double c2;
    double c1;
    double c0;
    double sgn_N;
    double spoly;
    double cpoly;
    double zero;
    double one;
    double sgn_x;
    double sgn_c;
    double sgn_s;
    double SgnMask;
    double Rs_msk;
    double Rc_msk;
    double Shifter;
    double Nmask;
    double Two53;
    vm = 0;
    AbsMask = as_double(__devicelib_imf_internal_dsincospi_data._AbsMask);
    aN = as_double((as_ulong(va1) & as_ulong(AbsMask)));
    Shifter = as_double(__devicelib_imf_internal_dsincospi_data._Shifter);
    sgn_N = (aN + Shifter);
    fN = (sgn_N - Shifter);
    Nmask = as_double((VUINT64)((aN < Shifter) ? 0xffffffffffffffff : 0x0));
    sgn_N = as_double((((~as_ulong(Nmask)) & as_ulong(aN)) |
                       (as_ulong(Nmask) & as_ulong(sgn_N))));
    fN = as_double((((~as_ulong(Nmask)) & as_ulong(aN)) |
                    (as_ulong(Nmask) & as_ulong(fN))));
    Two53 = as_double(__devicelib_imf_internal_dsincospi_data._Two53);
    Nmask = as_double((VUINT64)((aN < Two53) ? 0xffffffffffffffff : 0x0));
    SgnMask = as_double(__devicelib_imf_internal_dsincospi_data._SgnMask);
    sgn_N = as_double((((~as_ulong(Nmask)) & as_ulong(SgnMask)) |
                       (as_ulong(Nmask) & as_ulong(sgn_N))));
    sgn_x = as_double((as_ulong(va1) & as_ulong(SgnMask)));
    fN = as_double((as_ulong(fN) ^ as_ulong(sgn_x)));
    Rs = (va1 - fN);
    // prepare reduced argument for cospi
    Half = as_double(__devicelib_imf_internal_dsincospi_data._Half);
    aRs = as_double((as_ulong(Rs) & as_ulong(AbsMask)));
    Rc = (Half - aRs);
    Rs2 = (Rs * Rs);
    Rc2 = (Rc * Rc);
    // sign, based on odd(iN)
    sgn_N = as_double(((VUINT64)as_ulong(sgn_N) << (63)));
    // polynomial coefficients
    c8 = as_double(__devicelib_imf_internal_dsincospi_data._c8);
    c7 = as_double(__devicelib_imf_internal_dsincospi_data._c7);
    c6 = as_double(__devicelib_imf_internal_dsincospi_data._c6);
    c5 = as_double(__devicelib_imf_internal_dsincospi_data._c5);
    c4 = as_double(__devicelib_imf_internal_dsincospi_data._c4);
    c3 = as_double(__devicelib_imf_internal_dsincospi_data._c3);
    c2 = as_double(__devicelib_imf_internal_dsincospi_data._c2);
    c1 = as_double(__devicelib_imf_internal_dsincospi_data._c1);
    c0 = as_double(__devicelib_imf_internal_dsincospi_data._c0);
    // polynomials
    spoly = __fma(c8, Rs2, c7);
    cpoly = __fma(c8, Rc2, c7);
    spoly = __fma(spoly, Rs2, c6);
    cpoly = __fma(cpoly, Rc2, c6);
    spoly = __fma(spoly, Rs2, c5);
    cpoly = __fma(cpoly, Rc2, c5);
    spoly = __fma(spoly, Rs2, c4);
    cpoly = __fma(cpoly, Rc2, c4);
    spoly = __fma(spoly, Rs2, c3);
    cpoly = __fma(cpoly, Rc2, c3);
    // prepare sign
    zero = as_double(__devicelib_imf_internal_dsincospi_data._zero);
    SgnMask = as_double(__devicelib_imf_internal_dsincospi_data._SgnMask);
    Rs_msk = as_double((VUINT64)(((!(Rs == zero)) ? 0xffffffffffffffff : 0x0)));
    sgn_x = as_double((as_ulong(va1) & as_ulong(SgnMask)));
    Rc_msk = as_double((VUINT64)(((!(Rc == zero)) ? 0xffffffffffffffff : 0x0)));
    sgn_s = as_double((((~as_ulong(Rs_msk)) & as_ulong(sgn_x)) |
                       (as_ulong(Rs_msk) & as_ulong(sgn_N))));
    sgn_c = as_double((as_ulong(sgn_N) & as_ulong(Rc_msk)));
    // polynomial evaluation
    spoly = __fma(spoly, Rs2, c2);
    cpoly = __fma(cpoly, Rc2, c2);
    spoly = __fma(spoly, Rs2, c1);
    cpoly = __fma(cpoly, Rc2, c1);
    spoly = __fma(spoly, Rs2, c0);
    cpoly = __fma(cpoly, Rc2, c0);
    Rs = as_double((as_ulong(Rs) ^ as_ulong(sgn_s)));
    Rc = as_double((as_ulong(Rc) ^ as_ulong(sgn_c)));
    vr1 = (spoly * Rs);
    vr2 = (cpoly * Rc);
  }
  ((double *)y)[0] = vr1;
  ((double *)z)[0] = vr2;
  return;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
