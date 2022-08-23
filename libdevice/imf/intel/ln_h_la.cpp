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
//  *   log(x) = VGETEXP(x)*log(2) + log(VGETMANT(x))
//  *   VGETEXP, VGETMANT will correctly treat special cases too (including
denormals)
//  *   mx = VGETMANT(x) is in [1,2) for all x>=0
//  *   log(mx) = -log(RCP(mx)) + log(1 +(mx*RCP(mx)-1))
//  *   and the table lookup for log(RCP(mx))
//  *
//
*/
#include "_imf_include_fp32.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_ln_h_la {
namespace {
typedef struct {
  // 1.0
  VUINT16 __one; // 0x3c00
  // log(2)
  VUINT16 __LN2; // 0x398C
  // polynomial coefficients
  VUINT16 __Tbl_c1[32][1]; // 2^(k/32) (k in 0..31)
  VUINT16 __Tbl_c0[32][1]; // 2^(k/32) (k in 0..31)
  // non-UISA implementation
  VUINT16 _InfExpon;
  VUINT16 _Zero;
  VUINT32 _I1;
  VUINT32 _Ibias;
  VUINT32 _sOne;
  VUINT32 _sc4;
  VUINT32 _sc3;
  VUINT32 _sc2;
  VUINT32 _sc1;
  VUINT32 _sc0;
  VUINT32 _sL2;
  VUINT16 _hNInf;
} __devicelib_imf_internal_hln_data_t;
static const __devicelib_imf_internal_hln_data_t
    __devicelib_imf_internal_hln_data = {
        0x3c00u, //__one
        0x398Cu, //__LN2
        {
            //__Tbl_c1
            0xb7d6u, 0xb787u, 0xb73cu, 0xb6f6u, 0xb6b5u, 0xb677u, 0xb63eu,
            0xb607u, 0xb5d3u, 0xb5a3u, 0xb575u, 0xb549u, 0xb520u, 0xb4f8u,
            0xb4d3u, 0xb4afu, 0xb9c4u, 0xb99du, 0xb978u, 0xb955u, 0xb933u,
            0xb912u, 0xb8f3u, 0xb8d5u, 0xb8b8u, 0xb89cu, 0xb882u, 0xb868u,
            0xb850u, 0xb838u, 0xb821u, 0xb80bu,
        },
        {
            //__Tbl_c0
            0x3c00u, 0x3bffu, 0x3bfcu, 0x3bf9u, 0x3bf5u, 0x3bf0u, 0x3bebu,
            0x3be5u, 0x3bdeu, 0x3bd8u, 0x3bd0u, 0x3bc9u, 0x3bc1u, 0x3bb9u,
            0x3bb1u, 0x3ba9u, 0x3bc4u, 0x3bcdu, 0x3bd5u, 0x3bdcu, 0x3be2u,
            0x3be8u, 0x3bedu, 0x3bf1u, 0x3bf5u, 0x3bf8u, 0x3bfau, 0x3bfcu,
            0x3bfeu, 0x3bffu, 0x3c00u, 0x3c00u,
        }
        // non-UISA implementation
        ,
        0x1fu // InfExpon
        ,
        0x0u // Zero
        ,
        0x1u // I1
        ,
        0x7fu // Ibias
        ,
        0x3f800000u // 1.0
        ,
        0x3e51367bu // sc4
        ,
        0xbebfd356u // sc3
        ,
        0x3ef9e953u // sc2
        ,
        0xbf389f48u // sc1
        ,
        0x3fb8a7e4u // sc0
        ,
        0x3f317218u // sLn2
        ,
        0xfc00u // hNInf
};
inline _iml_half __devicelib_imf_logf16_impl(_iml_half x) {
  _iml_half r;
  VUINT32 vm;
  _iml_half va1;
  _iml_half vr1;
  va1 = x;
  {
    _iml_half h_iexpon;
    VUINT16 iexpon;
    VUINT16 InfExpon;
    VUINT16 not_inf_neg_nanmask;
    VUINT16 zeromask;
    _iml_half zero;
    float varg1;
    VUINT32 ivarg1;
    VUINT32 iexpon2;
    VUINT32 I1;
    VUINT32 ibias;
    VUINT32 imant;
    float mant;
    float expon;
    _iml_half hNInf;
    _iml_half HSpec1;
    float R;
    float c4;
    float c3;
    float c2;
    float c1;
    float c0;
    float spoly;
    float L2;
    float sOne;
    vm = 0;
    // predicate for x negative, NaN, or +Inf
    h_iexpon = as_half(((VUINT16)as_short(va1) >> (10)));
    iexpon = as_short(h_iexpon);
    InfExpon = (__devicelib_imf_internal_hln_data._InfExpon);
    not_inf_neg_nanmask = (iexpon < InfExpon) ? 0x1L : 0x0L;
    // predicate for x==0
    zero = as_half(__devicelib_imf_internal_hln_data._Zero);
    zeromask = (va1 == zero) ? 0x1L : 0x0L;
    // normalize and extract exponent, mantissa
    varg1 = ((float)(va1));
    // exponent, adjusted so mantissa is normalized to [0.75,1.5)
    ivarg1 = as_uint(varg1);
    iexpon2 = ((VUINT32)(ivarg1) >> (22));
    I1 = (__devicelib_imf_internal_hln_data._I1);
    iexpon2 = (iexpon2 + I1);
    iexpon2 = ((VUINT32)(iexpon2) >> (1));
    ibias = (__devicelib_imf_internal_hln_data._Ibias);
    iexpon2 = (iexpon2 - ibias);
    // exponent
    expon = ((float)((VINT32)(iexpon2)));
    iexpon2 = ((VUINT32)(iexpon2) << (23));
    imant = (ivarg1 - iexpon2);
    // mantissa
    mant = as_float(imant);
    // R = mantissa - 1
    sOne = as_float(__devicelib_imf_internal_hln_data._sOne);
    R = (mant - sOne);
    // polynomial ~ log2(1+R)/R
    c4 = as_float(__devicelib_imf_internal_hln_data._sc4);
    c3 = as_float(__devicelib_imf_internal_hln_data._sc3);
    c2 = as_float(__devicelib_imf_internal_hln_data._sc2);
    c1 = as_float(__devicelib_imf_internal_hln_data._sc1);
    c0 = as_float(__devicelib_imf_internal_hln_data._sc0);
    spoly = __fma(c4, R, c3);
    spoly = __fma(spoly, R, c2);
    spoly = __fma(spoly, R, c1);
    spoly = __fma(spoly, R, c0);
    // expon + log2(mantissa) ~ log2(x)
    spoly = __fma(spoly, R, expon);
    // log2(x)*log(2)
    L2 = as_float(__devicelib_imf_internal_hln_data._sL2);
    spoly = (spoly * L2);
    vr1 = ((_iml_half)(spoly));
    // response for neg., NaN, Inf
    hNInf = as_half(__devicelib_imf_internal_hln_data._hNInf);
    HSpec1 = as_half(0x7c00u);
    HSpec1 = __fma(va1, HSpec1, HSpec1);
    vr1 = (not_inf_neg_nanmask != 0x0) ? (vr1) : (HSpec1);
    // response for 0 input
    vr1 = (zeromask != 0x0) ? (hNInf) : (vr1);
  }
  r = vr1;
  return r;
}
} /* namespace */
} /* namespace __imf_impl_ln_h_la */
DEVICE_EXTERN_C_INLINE _iml_half_internal __devicelib_imf_logf16(_iml_half_internal x) {
  using namespace __imf_impl_ln_h_la;
  _iml_half _warg1(x);
  _iml_half _wres = __devicelib_imf_logf16_impl(_warg1);
  return _wres.get_internal();
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
