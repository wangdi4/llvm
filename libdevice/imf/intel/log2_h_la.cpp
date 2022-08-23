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
//  *   log2(x) = VGETEXP(x) + log2(VGETMANT(x))
//  *   VGETEXP, VGETMANT will correctly treat special cases too (including
denormals)
//  *   mx = VGETMANT(x) is in [1,2) for all x>=0
//  *   log2(mx) = -log2(RCP(mx)) + log2(1 +(mx*RCP(mx)-1))
//  *   and the table lookup for log2(RCP(mx))
//  *
//
*/
#include "_imf_include_fp32.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_log2_h_la {
namespace {
typedef struct {
  // 1.0
  VUINT16 __One; // 0x3c00
  // polynomial coefficients
  VUINT16 __c4;
  VUINT16 __c3;
  VUINT16 __c2;
  VUINT16 __c1;
  VUINT16 __c0;
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
  VUINT16 _hNInf;
} __devicelib_imf_internal_hlog2_data_t;
static const __devicelib_imf_internal_hlog2_data_t
    __devicelib_imf_internal_hlog2_data = {
        0x3c00u, //__one
        0x328au, //__c4
        0xb5ffu, //__c3
        0x37cfu, //__c2
        0xb9c5u, //__c1
        0x3dc5u, //__c0
                 // non-UISA implementation
        0x1fu    // InfExpon
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
        0xfc00u // hNInf
};
inline _iml_half __devicelib_imf_log2f16_impl(_iml_half x) {
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
    InfExpon = (__devicelib_imf_internal_hlog2_data._InfExpon);
    not_inf_neg_nanmask = (iexpon < InfExpon) ? 0x1L : 0x0L;
    // predicate for x==0
    zero = as_half(__devicelib_imf_internal_hlog2_data._Zero);
    zeromask = (va1 == zero) ? 0x1L : 0x0L;
    // normalize and extract exponent, mantissa
    varg1 = ((float)(va1));
    // exponent, adjusted so mantissa is normalized to [0.75,1.5)
    ivarg1 = as_uint(varg1);
    iexpon2 = ((VUINT32)(ivarg1) >> (22));
    I1 = (__devicelib_imf_internal_hlog2_data._I1);
    iexpon2 = (iexpon2 + I1);
    iexpon2 = ((VUINT32)(iexpon2) >> (1));
    ibias = (__devicelib_imf_internal_hlog2_data._Ibias);
    iexpon2 = (iexpon2 - ibias);
    // exponent
    expon = ((float)((VINT32)(iexpon2)));
    iexpon2 = ((VUINT32)(iexpon2) << (23));
    imant = (ivarg1 - iexpon2);
    // mantissa
    mant = as_float(imant);
    // R = mantissa - 1
    sOne = as_float(__devicelib_imf_internal_hlog2_data._sOne);
    R = (mant - sOne);
    // polynomial ~ log2(1+R)/R
    c4 = as_float(__devicelib_imf_internal_hlog2_data._sc4);
    c3 = as_float(__devicelib_imf_internal_hlog2_data._sc3);
    c2 = as_float(__devicelib_imf_internal_hlog2_data._sc2);
    c1 = as_float(__devicelib_imf_internal_hlog2_data._sc1);
    c0 = as_float(__devicelib_imf_internal_hlog2_data._sc0);
    spoly = __fma(c4, R, c3);
    spoly = __fma(spoly, R, c2);
    spoly = __fma(spoly, R, c1);
    spoly = __fma(spoly, R, c0);
    // expon + log2(mantissa) ~ log2(x)
    spoly = __fma(spoly, R, expon);
    vr1 = ((_iml_half)(spoly));
    // response for neg., NaN, Inf
    hNInf = as_half(__devicelib_imf_internal_hlog2_data._hNInf);
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
} /* namespace __imf_impl_log2_h_la */
DEVICE_EXTERN_C_INLINE _iml_half_internal __devicelib_imf_log2f16(_iml_half_internal x) {
  using namespace __imf_impl_log2_h_la;
  _iml_half _warg1(x);
  _iml_half _wres = __devicelib_imf_log2f16_impl(_warg1);
  return _wres.get_internal();
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
