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
namespace __imf_impl_erf_d_ha {
namespace {
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___c12 = {0x3dd0579ab18bb034UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___c11 = {0xbe1386ac0e80f6f6UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___c10 = {0x3e4f7a41239424f0UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___c9 = {0xbe85f1d36c342bdcUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___c8 = {0x3ebb9def19e10c03UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___c7 = {0xbeef4d1df394dad1UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___c6 = {0x3f1f9a321a11df20UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___c5 = {0xbf4c02db3d8ed259UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___c4 = {0x3f7565bcd0db02e5UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___c3 = {0xbf9b82ce3128499cUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___c2 = {0x3fbce2f21a042b20UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___c1 = {0xbfd812746b0379e6UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___c0 = {0x3fc06eba8214db69UL};
// [.875, 2.5)
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r222 = {0x3e0aea9d809c0cc0UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r221 = {0xbe1585e5389852a1UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r220 = {0xbe44e3c7bb4e6badUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r219 = {0x3e60787bd6c23cdeUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r218 = {0x3e6c4a4f934ec97aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r217 = {0xbe9afbf4b1061a86UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r216 = {0x3e8c8f2572501791UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r215 = {0x3ecb5d5d47f87268UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r214 = {0xbee030574affb335UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r213 = {0xbee9d5c236c6c400UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r212 = {0x3f153fc67d1daecfUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r211 = {0xbf1037f1525a68bfUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r210 = {0xbf3b88dd5886d772UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r29 = {0x3f5319f780b3f6bbUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r28 = {0x3f23a95ae24531a6UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r27 = {0xbf79be731fd97edbUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r26 = {0x3f87e8755da3fd73UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r25 = {0x3f66981061df9ac9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r24 = {0xbfa9647a30b1671dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r23 = {0x3fba3687c1eaf1b3UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r22 = {0xbfbc435059d0978aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r21 = {0x3fb0bf97e95f2a64UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r20 = {0xbf916b24cb8f8f92UL};
// [2.5, 4)
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r319 = {0xbe0a2f1867e76952UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r318 = {0x3e21198ad6cc5141UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r317 = {0x3e2958c6ce203b89UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r316 = {0xbe5daf579fcb0d86UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r315 = {0x3e71dca84cc9234cUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r314 = {0x3e2ec6c1a451cbf6UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r313 = {0xbea265e39074a9d0UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r312 = {0x3ebff2dd7650fef1UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r311 = {0xbec806fecef280dcUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r310 = {0xbec3892b6d2cf282UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r39 = {0x3efb2c31c4d013eaUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r38 = {0xbf156747a650e49aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r37 = {0x3f2647f722962588UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r36 = {0xbf3145464e6e271fUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r35 = {0x3f3490a4d22a7f1aUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r34 = {0xbf32c7d5ef077ce8UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r33 = {0x3f29aa489e3d085bUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r32 = {0xbf18de3cd2908194UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r31 = {0x3efe9b5e8d00c879UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r30 = {0xbed20c13035510baUL};
// [4, 6)
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r417 = {0x3d987b4417eaf36dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r416 = {0xbdc085b75720c6a6UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r415 = {0x3dd78ecc9429a326UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r414 = {0xbdeb70e185e567c1UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r413 = {0x3dfd4cd2d3b0acb6UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r412 = {0xbe0b6e867ec36d97UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r411 = {0x3e162553731107f0UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r410 = {0xbe1f3c344c6247c2UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r49 = {0x3e234d95061910ebUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r48 = {0xbe24bd9ec47f0fbfUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r47 = {0x3e232d1bda20cff8UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r46 = {0xbe1e231ca9b2d22dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r45 = {0x3e13c4f1db039226UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r44 = {0xbe0516ddf2b5a8e2UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r43 = {0x3df196e40460a88bUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r42 = {0xbdd589af770a029eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r41 = {0x3db13af23e58ca63UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __derf_ha___r40 = {0xBD7B0C1B6D3B24ADUL};
inline int __devicelib_imf_internal_derf(const double *a, double *pres) {
  int nRet = 0;
  double xin = *a;
  union {
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
  } x, xa, res, cpoly;
  uint64_t sgn_x;
  double dR, dR2;
  xa.f = xin;
  sgn_x = xa.w & 0x8000000000000000UL;
  // |xin|
  xa.w ^= sgn_x;
  if (xa.f < 2.5) {
    if (xa.f < .875) {
      dR2 = __fma(xa.f, xa.f, 0.0);
      // polynomial evaluation
      cpoly.f =
          __fma(__derf_ha___c12.f, dR2, __derf_ha___c11.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___c10.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___c9.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___c8.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___c7.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___c6.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___c5.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___c4.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___c3.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___c2.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___c1.f);
      res.f = __fma(cpoly.f, dR2, __derf_ha___c0.f);
      res.f = __fma(res.f, xin, xin);
      *pres = res.f;
      return nRet;
    } else //(0.875 <= xa.f < 2.5)
    {
      dR2 = xa.f - 1.6875;
      // polynomial evaluation
      cpoly.f =
          __fma(__derf_ha___r222.f, dR2, __derf_ha___r221.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r220.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r219.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r218.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r217.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r216.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r215.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r214.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r213.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r212.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r211.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r210.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r29.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r28.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r27.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r26.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r25.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r24.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r23.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r22.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r21.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r20.f);
      res.f = cpoly.f + 1.0;
      res.w ^= sgn_x;
      *pres = res.f;
      return nRet;
    }
  } else // 2.5 <= x
  {
    if (xa.f < 4.0) {
      dR2 = xa.f - 3.25;
      // polynomial evaluation
      cpoly.f =
          __fma(__derf_ha___r319.f, dR2, __derf_ha___r318.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r317.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r316.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r315.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r314.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r313.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r312.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r311.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r310.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r39.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r38.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r37.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r36.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r35.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r34.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r33.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r32.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r31.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r30.f);
      res.f = cpoly.f + 1.0;
      res.w ^= sgn_x;
      *pres = res.f;
      return nRet;
    } else {
      // limit |x| range to [0,6]
      dR = (xa.f > 6.0) ? 6.0 : xa.f;
      dR = (xa.w <= 0x7ff0000000000000UL) ? dR : xa.f;
      dR2 = dR - 5.0;
      // polynomial evaluation
      cpoly.f =
          __fma(__derf_ha___r417.f, dR2, __derf_ha___r416.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r415.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r414.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r413.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r412.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r411.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r410.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r49.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r48.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r47.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r46.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r45.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r44.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r43.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r42.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r41.f);
      cpoly.f = __fma(cpoly.f, dR2, __derf_ha___r40.f);
      res.f = cpoly.f + 1.0;
      res.w ^= sgn_x;
      *pres = res.f;
      return nRet;
    }
  }
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_erf_d_ha */
DEVICE_EXTERN_C_INLINE double __devicelib_imf_erf(double x) {
  using namespace __imf_impl_erf_d_ha;
  double r;
  __devicelib_imf_internal_derf(&x, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
