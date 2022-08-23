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
//  *
//  *  exp10(x)  = 2^x/log10(2) = 2^n * (1 + T[j]) * (1 + P(y))
//  *
//  *  x = m*log10(2)/K + y,  y in [-log10(2)/K..log10(2)/K]
//  *  m = n*K + j,           m,n,j - signed integer, j in [-K/2..K/2]
//  *
//  *  values of 2^j/K are tabulated
//  *
//  *  P(y) is a minimax polynomial approximation of exp10(x)-1
//  *  on small interval [-log10(2)/K..log10(2)/K]
//  *
//  * Special cases:
//  *
//  *  exp10(NaN)  = NaN
//  *  exp10(+INF) = +INF
//  *  exp10(-INF) = 0
//  *  exp10(x)    = 1 for subnormals
//
*/
#include "_imf_include_fp32.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_exp10_h_la {
namespace {
typedef struct {
  VUINT16 __M18;
  VUINT16 __P12;
  VUINT16 __Shifter2;
  VUINT16 __L2E;
  VUINT16 __L2EL;
  VUINT16 __gc3;
  VUINT16 __gc2;
  VUINT16 __gc1;
  VUINT16 __gc0;
  VUINT16 __Inf;
} __devicelib_imf_internal_hexp10_data_t;
static const __devicelib_imf_internal_hexp10_data_t
    __devicelib_imf_internal_hexp10_data = {
        0xc800u, //__M18
        0x4600u, //__P12
        0x6a0fu, //__Shifter2
        0x42a5u, //__L2E
        0x8d88u, //__L2EL
        0x2110u, //__gc3
        0x2b52u, //__gc2
        0x33afu, //__gc1
        0x398bu, //__gc0
        0x7c00u, //__Inf
};
inline _iml_half __devicelib_imf_exp10f16_impl(_iml_half x) {
  _iml_half r;
  VUINT32 vm;
  _iml_half va1;
  _iml_half vr1;
  va1 = x;
  {
    _iml_half X;
    _iml_half P12;
    _iml_half M18;
    _iml_half L2E;
    _iml_half L2EL;
    _iml_half Shifter;
    _iml_half Y;
    _iml_half N;
    _iml_half PN2;
    _iml_half Poly;
    _iml_half Inf;
    VUINT16 NanMask;
    _iml_half R;
    _iml_half c3;
    _iml_half c2;
    _iml_half c1;
    _iml_half c0;
    vm = 0;
    // restrict input range to [-8.0,6.0]
    P12 = as_half(__devicelib_imf_internal_hexp10_data.__P12);
    M18 = as_half(__devicelib_imf_internal_hexp10_data.__M18);
    X = ((va1 < P12) ? va1 : P12);
    X = ((X > M18) ? X : M18);
    // (2^11*1.5 + bias) + x*log2(e)
    Shifter = as_half(__devicelib_imf_internal_hexp10_data.__Shifter2);
    L2E = as_half(__devicelib_imf_internal_hexp10_data.__L2E);
    Y = __fma(X, L2E, Shifter);
    // N = 2*(int)(x*log2(e)/2)
    N = (Y - Shifter);
    // 2^(N/2)
    PN2 = as_half(((VUINT16)as_short(Y) << (10)));
    // reduced arg.: x*log2(e) - N
    R = __fma(X, L2E, -(N));
    L2EL = as_half(__devicelib_imf_internal_hexp10_data.__L2EL);
    R = __fma(X, L2EL, R);
    // polynomial ~ 2^R
    c3 = as_half(__devicelib_imf_internal_hexp10_data.__gc3);
    c2 = as_half(__devicelib_imf_internal_hexp10_data.__gc2);
    c1 = as_half(__devicelib_imf_internal_hexp10_data.__gc1);
    c0 = as_half(__devicelib_imf_internal_hexp10_data.__gc0);
    // start polynomial
    Poly = __fma(c3, R, c2);
    Poly = __fma(Poly, R, c1);
    Poly = __fma(Poly, R, c0);
    Poly = (Poly * R);
    vr1 = __fma(Poly, PN2, PN2);
    // fixup for input=NaN
    Inf = as_half(__devicelib_imf_internal_hexp10_data.__Inf);
    NanMask = (!(va1 <= Inf)) ? 0x1L : 0x0L;
    PN2 = (NanMask != 0x0) ? (va1) : (PN2);
    // result:  (1+poly)*2^(N/2)*2^(N/2)
    vr1 = (vr1 * PN2);
  }
  r = vr1;
  return r;
}
} /* namespace */
} /* namespace __imf_impl_exp10_h_la */
DEVICE_EXTERN_C_INLINE _iml_half_internal __devicelib_imf_exp10f16(_iml_half_internal x) {
  using namespace __imf_impl_exp10_h_la;
  _iml_half _warg1(x);
  _iml_half _wres = __devicelib_imf_exp10f16_impl(_warg1);
  return _wres.get_internal();
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
