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
namespace __imf_impl_erf_s_ha {
namespace {
/* VML 32 bit optimal UF :========= HA LA EP */
/* VML 64 bit optimal UF:========== HA LA EP */
static const union {
  uint32_t w;
  float f;
} __serf_ha___c6 = {0x38b5efa8u};
static const union {
  uint32_t w;
  float f;
} __serf_ha___c5 = {0xba573fc9u};
static const union {
  uint32_t w;
  float f;
} __serf_ha___c4 = {0x3baa9d5du};
static const union {
  uint32_t w;
  float f;
} __serf_ha___c3 = {0xbcdc0cd3u};
static const union {
  uint32_t w;
  float f;
} __serf_ha___c2 = {0x3de71742u};
static const union {
  uint32_t w;
  float f;
} __serf_ha___c1 = {0xbec093a2u};
static const union {
  uint32_t w;
  float f;
} __serf_ha___c0 = {0x3e0375d4u};
// polynomial coefficients for [.875, 2)
static const union {
  uint32_t w;
  float f;
} __serf_ha___b7 = {0x3bd995c8u};
static const union {
  uint32_t w;
  float f;
} __serf_ha___b6 = {0x3c92d52eu};
static const union {
  uint32_t w;
  float f;
} __serf_ha___b5 = {0xbd60734bu};
static const union {
  uint32_t w;
  float f;
} __serf_ha___b4 = {0x3c21a26bu};
static const union {
  uint32_t w;
  float f;
} __serf_ha___b3 = {0x3e2b2466u};
static const union {
  uint32_t w;
  float f;
} __serf_ha___b2 = {0xbea7785bu};
static const union {
  uint32_t w;
  float f;
} __serf_ha___b1 = {0x3e8d06e3u};
static const union {
  uint32_t w;
  float f;
} __serf_ha___b0 = {0xbdbe9fd3u};
// polynomial coefficients for [1.5, 4.0]
static const union {
  uint32_t w;
  float f;
} __serf_ha___r13 = {0x360f65a7u};
static const union {
  uint32_t w;
  float f;
} __serf_ha___r12 = {0xb68dd176u};
static const union {
  uint32_t w;
  float f;
} __serf_ha___r11 = {0xb76ca6edu};
static const union {
  uint32_t w;
  float f;
} __serf_ha___r10 = {0x387da91au};
static const union {
  uint32_t w;
  float f;
} __serf_ha___r9 = {0xb89a64e2u};
static const union {
  uint32_t w;
  float f;
} __serf_ha___r8 = {0xb8c57175u};
static const union {
  uint32_t w;
  float f;
} __serf_ha___r7 = {0x3a29cd1au};
static const union {
  uint32_t w;
  float f;
} __serf_ha___r6 = {0xbad8e48au};
static const union {
  uint32_t w;
  float f;
} __serf_ha___r5 = {0x3b349094u};
static const union {
  uint32_t w;
  float f;
} __serf_ha___r4 = {0xbb559086u};
static const union {
  uint32_t w;
  float f;
} __serf_ha___r3 = {0x3b34e93eu};
static const union {
  uint32_t w;
  float f;
} __serf_ha___r2 = {0xbad350a2u};
static const union {
  uint32_t w;
  float f;
} __serf_ha___r1 = {0x3a19b01cu};
static const union {
  uint32_t w;
  float f;
} __serf_ha___r0 = {0xb8d30572u};
inline int __devicelib_imf_internal_serf(const float *a, float *pres) {
  int nRet = 0;
  float xin = *a;
  union {
    uint32_t w;
    float f;
  } x, xa, res;
  uint32_t sgn_x;
  float dR, dR2;
  union {
    uint32_t w;
    float f;
  } poly;
  xa.f = xin;
  sgn_x = xa.w & 0x80000000;
  // |xin|
  xa.w ^= sgn_x;
  if (xa.f < 1.5f) {
    if (xa.f < 0.875f) {
      dR = xa.f;
      dR2 = __fma(dR, dR, 0.0f);
      // polynomial evaluation
      poly.f = __fma(__serf_ha___c6.f, dR2, __serf_ha___c5.f);
      poly.f = __fma(poly.f, dR2, __serf_ha___c4.f);
      poly.f = __fma(poly.f, dR2, __serf_ha___c3.f);
      poly.f = __fma(poly.f, dR2, __serf_ha___c2.f);
      poly.f = __fma(poly.f, dR2, __serf_ha___c1.f);
      poly.f = __fma(poly.f, dR2, __serf_ha___c0.f);
      res.f = __fma(poly.f, dR, dR);
      res.w ^= sgn_x;
      *pres = res.f;
    } else {
      dR = xa.f - 1.1875f;
      // polynomial evaluation
      poly.f = __fma(__serf_ha___b7.f, dR, __serf_ha___b6.f);
      poly.f = __fma(poly.f, dR, __serf_ha___b5.f);
      poly.f = __fma(poly.f, dR, __serf_ha___b4.f);
      poly.f = __fma(poly.f, dR, __serf_ha___b3.f);
      poly.f = __fma(poly.f, dR, __serf_ha___b2.f);
      poly.f = __fma(poly.f, dR, __serf_ha___b1.f);
      poly.f = __fma(poly.f, dR, __serf_ha___b0.f);
      res.f = poly.f + 1.0;
      res.w ^= sgn_x;
      *pres = res.f;
    }
  } else {
    // limit |x| range to [0,4]
    dR = (xa.f > 4.0f) ? 4.0f : xa.f;
    // compiler workaround for NaNs
    dR = (xa.w <= 0x7f800000) ? dR : xa.f;
    dR = dR - 2.75f;
    // polynomial evaluation
    poly.f = __fma(__serf_ha___r13.f, dR, __serf_ha___r12.f);
    poly.f = __fma(poly.f, dR, __serf_ha___r11.f);
    poly.f = __fma(poly.f, dR, __serf_ha___r10.f);
    poly.f = __fma(poly.f, dR, __serf_ha___r9.f);
    poly.f = __fma(poly.f, dR, __serf_ha___r8.f);
    poly.f = __fma(poly.f, dR, __serf_ha___r7.f);
    poly.f = __fma(poly.f, dR, __serf_ha___r6.f);
    poly.f = __fma(poly.f, dR, __serf_ha___r5.f);
    poly.f = __fma(poly.f, dR, __serf_ha___r4.f);
    poly.f = __fma(poly.f, dR, __serf_ha___r3.f);
    poly.f = __fma(poly.f, dR, __serf_ha___r2.f);
    poly.f = __fma(poly.f, dR, __serf_ha___r1.f);
    poly.f = __fma(poly.f, dR, __serf_ha___r0.f);
    res.f = poly.f + 1.0;
    res.w ^= sgn_x;
    *pres = res.f;
  }
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_erf_s_ha */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_erff(float a) {
  using namespace __imf_impl_erf_s_ha;
  float r;
  __devicelib_imf_internal_serf(&a, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
