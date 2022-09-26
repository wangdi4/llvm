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
//     2^X = 2^Xo  x  2^{X-Xo}
//     2^X = 2^K  x  2^fo  x  2^{X-Xo}
//     2^X = 2^K  x  2^fo  x  2^r
//
//     2^K  --> Manual scaling
//     2^fo --> Table lookup
//     r    --> 1 + poly    (r = X - Xo)
//
//     Xo = K  +  fo
//     Xo = K  +  0.x1x2x3x4
//
//     r = X - Xo
//       = Vreduce(X, imm)
//       = X - VRndScale(X, imm),    where Xo = VRndScale(X, imm)
//
//     Rnd(S + X) = S + Xo,    where S is selected as S = 2^19 x 1.5
//         S + X = S + floor(X) + 0.x1x2x3x4
//     Rnd(S + X) = Rnd(2^19 x 1.5 + X)
//     (Note: 2^exp x 1.b1b2b3 ... b23,  2^{exp-23} = 2^-4 for exp=19)
//
//     exp2(x) =  2^K  x  2^fo  x (1 + poly(r)),   where 2^r = 1 + poly(r)
//
//     Scale back:
//     dest = src1 x 2^floor(src2)
//
//
*/
#include "_imf_include_fp32.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_exp2_h_la {
namespace {
typedef struct {
  VUINT16 __c3;
  VUINT16 __c2;
  VUINT16 __c1;
  VUINT16 __c0;
  // non-UISA implementation
  VUINT16 __M26;
  VUINT16 __P16;
  VUINT16 __Shifter;
  VUINT16 __gc3;
  VUINT16 __gc2;
  VUINT16 __gc1;
  VUINT16 __gc0;
  VUINT16 __Inf;
} __devicelib_imf_internal_hexp2_data_t;
static const __devicelib_imf_internal_hexp2_data_t
    __devicelib_imf_internal_hexp2_data = {
        0x2d12u, //__c3
        0x332eu, //__c2
        0x3992u, //__c1
        0x3c00u, //__c0
        0xce80u, //__M26
        0x4c00u, //__P16
        0x6a0fu, //__Shifter
        0x2110u, //__gc3
        0x2b52u, //__gc2
        0x33afu, //__gc1
        0x398bu, //__gc0
        0x7c00u, //__Inf
};
inline _iml_half __devicelib_imf_exp2f16_impl(_iml_half x) {
  _iml_half r;
  VUINT32 vm;
  _iml_half va1;
  _iml_half vr1;
  va1 = x;
  {
    _iml_half R;
    _iml_half Poly;
    _iml_half X;
    _iml_half P16;
    _iml_half M26;
    _iml_half Shifter;
    _iml_half Y;
    _iml_half N;
    _iml_half PN2;
    _iml_half Inf;
    _iml_half c3;
    _iml_half c2;
    _iml_half c1;
    _iml_half c0;
    VUINT16 NanMask;
    vm = 0;
    // restrict input range to [-26.0,16.0]
    P16 = as_half(__devicelib_imf_internal_hexp2_data.__P16);
    M26 = as_half(__devicelib_imf_internal_hexp2_data.__M26);
    X = ((va1 < P16) ? va1 : P16);
    X = ((X > M26) ? X : M26);
    // (2^11*1.5 + bias) + x
    Shifter = as_half(__devicelib_imf_internal_hexp2_data.__Shifter);
    Y = (X + Shifter);
    // N = 2*(int)(x/2)
    N = (Y - Shifter);
    // reduced arg.: x - N
    R = (X - N);
    // 2^(N/2)
    PN2 = as_half(((VUINT16)as_short(Y) << (10)));
    // polynomial ~ 2^R
    c3 = as_half(__devicelib_imf_internal_hexp2_data.__gc3);
    c2 = as_half(__devicelib_imf_internal_hexp2_data.__gc2);
    c1 = as_half(__devicelib_imf_internal_hexp2_data.__gc1);
    c0 = as_half(__devicelib_imf_internal_hexp2_data.__gc0);
    // start polynomial
    Poly = __fma(c3, R, c2);
    Poly = __fma(Poly, R, c1);
    Poly = __fma(Poly, R, c0);
    Poly = (Poly * R);
    // fixup for input=NaN
    Inf = as_half(__devicelib_imf_internal_hexp2_data.__Inf);
    NanMask = (!(va1 <= Inf)) ? 0x1L : 0x0L;
    PN2 = (NanMask != 0x0) ? (va1) : (PN2);
    // result:  (1+poly)*2^(N/2)*2^(N/2)
    vr1 = __fma(Poly, PN2, PN2);
    vr1 = (vr1 * PN2);
  }
  r = vr1;
  return r;
}
} /* namespace */
} /* namespace __imf_impl_exp2_h_la */
DEVICE_EXTERN_C_INLINE _iml_half_internal __devicelib_imf_exp2f16(_iml_half_internal x) {
  using namespace __imf_impl_exp2_h_la;
  _iml_half _warg1(x);
  _iml_half _wres = __devicelib_imf_exp2f16_impl(_warg1);
  return _wres.get_internal();
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
