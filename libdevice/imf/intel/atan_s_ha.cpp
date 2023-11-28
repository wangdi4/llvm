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
namespace __imf_impl_atan_s_ha {
namespace {
static const union {
  uint32_t w;
  float f;
} __satan_ha_c11 = {0xbc8917bfu};
static const union {
  uint32_t w;
  float f;
} __satan_ha_c10 = {0x3c0afad8u};
static const union {
  uint32_t w;
  float f;
} __satan_ha_c9 = {0x3cf6afd7u};
static const union {
  uint32_t w;
  float f;
} __satan_ha_c8 = {0xbd1e9009u};
static const union {
  uint32_t w;
  float f;
} __satan_ha_c7 = {0xbc0e2ce0u};
static const union {
  uint32_t w;
  float f;
} __satan_ha_c6 = {0x3d8ee88fu};
static const union {
  uint32_t w;
  float f;
} __satan_ha_c5 = {0xbd84d3d8u};
static const union {
  uint32_t w;
  float f;
} __satan_ha_c4 = {0xbd3a0aceu};
static const union {
  uint32_t w;
  float f;
} __satan_ha_c3 = {0x3e34898cu};
static const union {
  uint32_t w;
  float f;
} __satan_ha_c2 = {0xbe05f565u};
static const union {
  uint32_t w;
  float f;
} __satan_ha_c1l = {0x31b69212u};
static const union {
  uint32_t w;
  float f;
} __satan_ha_c1 = {0xbe8259aeu};
static const union {
  uint32_t w;
  float f;
} __satan_ha_c0 = {0xbd94e63fu};
inline int __devicelib_imf_internal_satan(const float *pxin, float *pres) {
  int nRet = 0;
  float xin = *pxin;
  // float atanf_ha(float xin)
  {
    union {
      uint32_t w;
      float f;
    } x, xa, hcorr, lcorr, ya, R0, R1, eps, Rcorr, sres;
    int32_t sgn_x, smask, sgn_r, diff;
    float poly, R;
    x.f = xin;
    xa.w = x.w & 0x7fffffffu;
    sgn_x = x.w ^ xa.w;
    // y ~ 1/x
    ya.f = 1.0f / (xa.f);
    // smask = (|x|>1.0)? -1 : 0
    diff = ya.w - xa.w;
    smask = ((int32_t)diff) >> 31;
    // will compute pi/2 - atan(1/|x|) for |x|>1
    hcorr.w = smask & 0xbfc90FDB;
    lcorr.w = smask & 0x333BBD2E;
    sgn_r = sgn_x ^ (smask & 0x80000000u);
    // reciprocal relative error
    eps.f = __fma(-(xa.f), ya.f, 1.0f);
    // fixup for eps=NaN
    eps.w &= 0xbfffffffu;
    // reduced argument
    R0.w = xa.w + (diff & smask);
    // correction term for reduced argument
    Rcorr.f = ya.f * eps.f;
    // will not use Rcorr if |xa|<=|ya|
    // fixup for Rcorr=NaN
    Rcorr.w &= (smask & 0xbfffffffu);
    R1.f = R0.f - 0.5f;
    // add correction
    R = R1.f + Rcorr.f;
    poly = __fma(__satan_ha_c11.f, R, __satan_ha_c10.f);
    poly = __fma(poly, R, __satan_ha_c9.f);
    poly = __fma(poly, R, __satan_ha_c8.f);
    poly = __fma(poly, R, __satan_ha_c7.f);
    poly = __fma(poly, R, __satan_ha_c6.f);
    poly = __fma(poly, R, __satan_ha_c5.f);
    poly = __fma(poly, R, __satan_ha_c4.f);
    poly = __fma(poly, R, __satan_ha_c3.f);
    poly = __fma(poly, R, __satan_ha_c2.f);
    // _VSTATIC(c1l) can be used to reduce ulp error
    // poly = SP_FMA(poly, R, _VSTATIC(c1l).f);
    poly = __fma(poly, R, __satan_ha_c1.f);
    poly = __fma(poly, R, __satan_ha_c0.f);
    // used here when _VSTATIC(c1l) is used in poly evaluation
    // poly = SP_FMA(_VSTATIC(c1).f, R, poly);
    // Rcorr + Rcorr*poly
    sres.f = __fma(poly, Rcorr.f, Rcorr.f);
    // (R+ R*poly)+lcorr
    sres.f = sres.f + lcorr.f;
    // ((R+ R*poly)+lcorr)+R0*poly
    sres.f = __fma(R0.f, poly, sres.f);
    // ((R+ R*poly)+lcorr)+R0*poly + R0
    sres.f = sres.f + R0.f;
    // ((R+ R*poly)+lcorr)+Q0*poly + Q0 + hcorr
    sres.f = sres.f + hcorr.f;
    sres.w = sres.w ^ sgn_r;
    *pres = sres.f;
  }
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_atan_s_ha */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_atanf(float a) {
  using namespace __imf_impl_atan_s_ha;
  float r;
  __devicelib_imf_internal_satan(&a, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
