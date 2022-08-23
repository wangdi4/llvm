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
//  *  Typical exp() implementation, except that:
//  *   - tables are small, allowing for fast gathers
//  *   - all arguments processed in the main path
//  *       - final VSCALEF assists branch-free design (correct
overflow/underflow and special case responses)
//  *       - a VAND is used to ensure the reduced argument |R|<2, even for
large inputs
//  *       - RZ mode used to avoid oveflow to +/-Inf for x*log2(e); helps with
special case handling
//  *       - SAE used to avoid spurious flag settings
//
*/
#include "_imf_include_fp32.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_exp_h_la {
namespace {
typedef struct {
  VUINT16 __Sbit;           // 0x0100
  VUINT16 __Shifter;        // 0x5300
  VUINT16 __Log2E;          // 0x3dc5
  VUINT16 __LN2;            // 0xb98C
  VUINT16 __LN2L;           // 0x0af4
  VUINT16 __Tbl_exp[32][1]; // 2^(k/32) (k in 0..31)
  VUINT16 __Rmask;          // 0xbfff
  VUINT16 __Absmask;        // 0x7fff
  // non-UISA implementation
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
} __devicelib_imf_internal_hexp_data_t;
static const __devicelib_imf_internal_hexp_data_t
    __devicelib_imf_internal_hexp_data = {
        0x0100u, //__Sbit
        0x5300u, //__Shifter
        0x3dc5u, //__Log2E
        0xb98Cu, //__LN2
        0x0af4u, //__LN2L
        {
            //__Tbl_exp
            0x3c00u, 0x3c16u, 0x3c2du, 0x3c45u, 0x3c5du, 0x3c75u, 0x3c8eu,
            0x3ca8u, 0x3cc2u, 0x3cdcu, 0x3cf8u, 0x3d14u, 0x3d30u, 0x3d4du,
            0x3d6bu, 0x3d89u, 0x3da8u, 0x3dc8u, 0x3de8u, 0x3e09u, 0x3e2bu,
            0x3e4eu, 0x3e71u, 0x3e95u, 0x3ebau, 0x3ee0u, 0x3f06u, 0x3f2eu,
            0x3f56u, 0x3f7fu, 0x3fa9u, 0x3fd4u,
        },
        0xbfffu, //__Rmask
        0x7fffu, //__Absmask
        0xcc80u, //__M18
        0x4a00u, //__P12
        0x6a0fu, //__Shifter2
        0x3dc5u, //__L2E
        0x0d1eu, //__L2EL
        0x2110u, //__gc3
        0x2b52u, //__gc2
        0x33afu, //__gc1
        0x398bu, //__gc0
        0x7c00u, //__Inf
};
inline _iml_half __devicelib_imf_expf16_impl(_iml_half x) {
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
    // restrict input range to [-18.0,12.0]
    P12 = as_half(__devicelib_imf_internal_hexp_data.__P12);
    M18 = as_half(__devicelib_imf_internal_hexp_data.__M18);
    X = ((va1 < P12) ? va1 : P12);
    X = ((X > M18) ? X : M18);
    // (2^11*1.5 + bias) + x*log2(e)
    Shifter = as_half(__devicelib_imf_internal_hexp_data.__Shifter2);
    L2E = as_half(__devicelib_imf_internal_hexp_data.__L2E);
    Y = __fma(X, L2E, Shifter);
    // N = 2*(int)(x*log2(e)/2)
    N = (Y - Shifter);
    // 2^(N/2)
    PN2 = as_half(((VUINT16)as_short(Y) << (10)));
    // reduced arg.: x*log2(e) - N
    R = __fma(X, L2E, -(N));
    L2EL = as_half(__devicelib_imf_internal_hexp_data.__L2EL);
    R = __fma(X, L2EL, R);
    // polynomial ~ 2^R
    c3 = as_half(__devicelib_imf_internal_hexp_data.__gc3);
    c2 = as_half(__devicelib_imf_internal_hexp_data.__gc2);
    c1 = as_half(__devicelib_imf_internal_hexp_data.__gc1);
    c0 = as_half(__devicelib_imf_internal_hexp_data.__gc0);
    // start polynomial
    Poly = __fma(c3, R, c2);
    Poly = __fma(Poly, R, c1);
    Poly = __fma(Poly, R, c0);
    Poly = (Poly * R);
    vr1 = __fma(Poly, PN2, PN2);
    // fixup for input=NaN
    Inf = as_half(__devicelib_imf_internal_hexp_data.__Inf);
    NanMask = (!(va1 <= Inf)) ? 0x1L : 0x0L;
    PN2 = (NanMask != 0x0) ? (va1) : (PN2);
    // result:  (1+poly)*2^(N/2)*2^(N/2)
    vr1 = (vr1 * PN2);
  }
  r = vr1;
  return r;
}
} /* namespace */
} /* namespace __imf_impl_exp_h_la */
DEVICE_EXTERN_C_INLINE _iml_half_internal __devicelib_imf_expf16(_iml_half_internal x) {
  using namespace __imf_impl_exp_h_la;
  _iml_half _warg1(x);
  _iml_half _wres = __devicelib_imf_expf16_impl(_warg1);
  return _wres.get_internal();
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
