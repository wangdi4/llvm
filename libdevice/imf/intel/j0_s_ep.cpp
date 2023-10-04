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
#include "_imf_include_fp32.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_j0_s_ep {
namespace {
static const int32_t __sj0_sincosf_ep___ip_h = 0x0517CC1B;
static const int32_t __sj0_sincosf_ep___ip_m = 0x727220A9;
static const int32_t __sj0_sincosf_ep___ip_l = 0x28;
static const union {
  uint32_t w;
  float f;
} __sj0_sincosf_ep___cc4 = {0x3e6ce1b2u};
static const union {
  uint32_t w;
  float f;
} __sj0_sincosf_ep___cc3 = {0xbfaae2beu};
static const union {
  uint32_t w;
  float f;
} __sj0_sincosf_ep___cc2 = {0x4081e0eeu};
static const union {
  uint32_t w;
  float f;
} __sj0_sincosf_ep___cc1 = {0xc09de9e6u};
static const union {
  uint32_t w;
  float f;
} __sj0_sincosf_ep___cc1l = {0xb3e646a5u};
static const union {
  uint32_t w;
  float f;
} __sj0_sincosf_ep___cc0 = {0x3f800000u};
static const union {
  uint32_t w;
  float f;
} __sj0_sincosf_ep___cs3 = {0xbf16c981u};
static const union {
  uint32_t w;
  float f;
} __sj0_sincosf_ep___cs2 = {0x40232f49u};
static const union {
  uint32_t w;
  float f;
} __sj0_sincosf_ep___cs1 = {0xc0a55dddu};
static const union {
  uint32_t w;
  float f;
} __sj0_sincosf_ep___cs0 = {0x40490fdbu};
static const union {
  uint32_t w;
  float f;
} __sj0_sincosf_ep___cs0l = {0xb3d195e9u};
static const uint32_t __sj0_sincosf_ep_invpi_tbl[] = {
    0,          0x28BE60DB, 0x9391054A, 0x7F09D5F4,
    0x7D4D3770, 0x36D8A566, 0x4F10E410, 0x7F9458EA};
static inline int __sj0_sincos_ep_kernel_fp32(float xf, int n, float *psin,
                                              float *pcos) {
  int nRet = 0;
  float xin = xf;
  uint64_t IP, IP2, N64 = (uint64_t)n;
  int64_t IP_s, IP2_s;
  int32_t ip_low_s;
  uint32_t ip_low;
  int_float x, Rh, Rl, res, scale, cres, sres, spoly, cpoly, cpoly_l;
  int mx, sgn_x, ex, ip_h, shift, index, j, sgn_p, sgn_xp;
  float High, Low, R2h, R2l, Ph, Pl;
  x.f = xin;
  mx = (x.w & 0x007fffff) | 0x00800000;
  sgn_x = x.w & 0x80000000;
  ex = ((x.w ^ sgn_x) >> 23);
  // redirect large or very small inputs
  if (__builtin_expect(((unsigned)(ex - 0x7f + 12)) > (20 + 12), (0 == 1))) {
    // small input: to be updated
    if (__builtin_expect((ex < 0x7f - 11), (1 == 1))) {
      psin[0] = xin;
      pcos[0] = 1.0f;
      return nRet;
    }
    // Inf/NaN
    if (ex == 0xff) {
      nRet = ((x.w << 1) == 0xff000000) ? 1 : nRet;
      x.w |= 0x00400000;
      psin[0] = x.f;
      pcos[0] = x.f;
      return nRet;
    }
    ex = ex - 0x7f - 23;
    index = 1 + (ex >> 5);
    // expon % 32
    j = ex & 0x1f;
    // x/Pi, scaled by 2^(63-j)
    ip_low = (((uint32_t)__sj0_sincosf_ep_invpi_tbl[index]) * ((uint32_t)mx));
    IP = (((uint64_t)((uint32_t)(__sj0_sincosf_ep_invpi_tbl[index + 1]))) *
          ((uint32_t)(mx))) +
         (((uint64_t)ip_low) << 32);
    // scaled by 2^(95-j)
    IP2 = (((uint64_t)((uint32_t)(__sj0_sincosf_ep_invpi_tbl[index + 2]))) *
           ((uint32_t)(mx))) +
          ((((uint64_t)((uint32_t)(__sj0_sincosf_ep_invpi_tbl[index + 3]))) *
            ((uint32_t)(mx))) >>
           32);
    IP = IP + (IP2 >> 32);
    // scale 2^63
    IP <<= j;
    // shift low part by 32-j, j in [0,31]
    ip_low = (uint32_t)IP2;
    ip_low >>= (31 - j);
    ip_low >>= 1;
    IP |= (uint64_t)ip_low;
  } else // main path
  {
    // products are really unsigned; operands are small enough so that signed
    // MUL works as well x*(23-ex)*(1/Pi)*2^28 p[k] products fit in 31 bits each
    IP_s = (((int64_t)((int32_t)(mx))) * ((int32_t)(__sj0_sincosf_ep___ip_h)));
    IP = (uint64_t)IP_s;
    IP2_s = (((int64_t)((int32_t)(mx))) * ((int32_t)(__sj0_sincosf_ep___ip_m)));
    IP2 = (uint64_t)IP2_s;
    // scale (23-ex)*2^(28+32+7)
    ip_low_s = (((int32_t)mx) * ((int32_t)__sj0_sincosf_ep___ip_l));
    ip_low = (uint32_t)ip_low_s;
    IP2 = (IP2 << 7) + ip_low;
    // (x/Pi)*2^63
    IP <<= (ex - 0x7f + 12);
    // IP3 = IP2 << (37 -0x7f + ex);
    IP2 >>= (27 + 0x7f - ex);
    IP += IP2;
  }
  // add (n*Pi/4)*(2^63/Pi)
  IP += (N64 << 61);
  // return to 32-bit, scale 2^31
  ip_h = IP >> 32;
  // sign bit
  sgn_xp = ((ip_h + 0x20000000) & 0xc0000000);
  // reduced argument (signed, high-low), scale 2^32
  ip_h <<= 2;
  Rh.f = (float)ip_h;
  // reduced argument will need to be normalized
  shift = 2 + 30 + 0x7f - ((Rh.w >> 23) & 0xff);
  // correction for shift=0
  shift = (shift >= 2) ? shift : 2;
  // normalize
  IP <<= shift; // IP = (IP << shift) | (IP3 >> (64-shift));
  ip_h = IP >> 32;
  Rh.f = (float)ip_h;
  // adjust scale
  scale.w = (0x7f - 31 - shift) << 23;
  Rh.f = __fma(Rh.f, scale.f, 0.0f);
  // (Rh)^2
  R2h = __fma(Rh.f, Rh.f, 0.0f);
  cpoly.f = __fma(__sj0_sincosf_ep___cc4.f, R2h,
                                   __sj0_sincosf_ep___cc3.f);
  spoly.f = __fma(__sj0_sincosf_ep___cs3.f, R2h,
                                   __sj0_sincosf_ep___cs2.f);
  cpoly.f = __fma(cpoly.f, R2h, __sj0_sincosf_ep___cc2.f);
  spoly.f = __fma(spoly.f, R2h, __sj0_sincosf_ep___cs1.f);
  cpoly.f = __fma(cpoly.f, R2h, __sj0_sincosf_ep___cc1.f);
  spoly.f = __fma(spoly.f, R2h, __sj0_sincosf_ep___cs0.f);
  cpoly.f = __fma(cpoly.f, R2h, __sj0_sincosf_ep___cc0.f);
  spoly.f = __fma(spoly.f, Rh.f, 0.0f);
  sgn_p = sgn_xp & 0x80000000;
  // adjust sign
  spoly.w ^= sgn_p ^ (sgn_xp << 1);
  cpoly.w ^= sgn_p;
  sres.w = (sgn_xp & 0x40000000) ? cpoly.w : spoly.w;
  sres.w ^= sgn_x;
  cres.w = (sgn_xp & 0x40000000) ? spoly.w : cpoly.w;
  pcos[0] = cres.f;
  psin[0] = sres.f;
  return nRet;
}
/* polynomial minimax approximation of J0(x) = 1+y*Q2(y), y=x^2, |x|=[0..2^(-2)]
 */
static const float __sj0_j0f_ep_fQ2[] = {
    -0x1p-2f, //           -0.25 [0xbe800000]  -.24999999999999192312749585e-00
              //           [0xbfcffffffffffedd]
    0x1p-6f,  //        0.015625 [0x3c800000]   .15624999995864828628011622e-01
              //        [0x3f8fffffffdba06c]
    -0x1.c71c5cp-12f, //  -0.00043402746 [0xb9e38e2e]
                      //  -.43402744692099296756199189e-03 [0xbf3c71c5b0a9df3a]
    0x1.c68ae6p-18f,  //   6.7732121e-06 [0x36e34573]
                      //   .67732122096095540643738083e-05 [0x3edc68ae653d320e]
};
/* polynomial minimax approximation of J0(x) = 1+y*Q3(y), y=x^2, |x|=[0..2^(-8)]
 */
static const float __sj0_j0f_ep_fQ3[] = {
    -0x1p-2f, //           -0.25 [0xbe800000] -.2499999999999873712130948e-00
              //           [0xbfcffffffffffe39]
    0x1.fffff2p-7f, //     0.015624993 [0x3c7ffff9]
                    //     .1562499337726327118536229e-01 [0x3f8fffff1c71caab]
};
/* polynomial minimax approximation of J0(x) = Q1(x-z0) on interval (0,s1). */
static const float __sj0_j0f_ep_fZ0[] = {
    0x1.33d152p+1f,  //       2.4048254 [0x4019e8a9]
    0x1.d2e368p-24f, //    1.087059e-07 [0x33e971b4] Low part
};
static const float __sj0_j0f_ep_fQ1[] = {
    -0x1.19b792p-54f, //  -6.1087652e-17 [0xa48cdbc9]
    -0x1.09cdb4p-1f,  //     -0.51914752 [0xbf04e6da]
    0x1.ba1deep-4f,   //       0.1079387 [0x3ddd0ef7]
    0x1.cfae86p-5f,   //     0.056601774 [0x3d67d743]
    -0x1.1bb1ccp-7f,  //   -0.0086576696 [0xbc0dd8e6]
    -0x1.1f9926p-9f,  //   -0.0021942004 [0xbb0fcc93]
    0x1.153838p-12f,  //   0.00026437722 [0x398a9c1c]
    0x1.6ed3dcp-15f,  //   4.3729255e-05 [0x383769ee]
    -0x1.232edcp-18f, //  -4.3389655e-06 [0xb691976e]
    -0x1.1cd86p-21f,  //  -5.3056556e-07 [0xb50e6c30]
    0x1.805988p-25f,  //   4.4744198e-08 [0x33402cc4]
    0x1.2bcba6p-28f,  //   4.3625987e-09 [0x3195e5d3]
    -0x1.5c973cp-32f, //  -3.1704145e-10 [0xafae4b9e]
    -0x1.1327d2p-35f, //  -3.1281564e-11 [0xae0993e9]
};
/* polynomial minimax approximation of J0(x) = P1(x-z1) on interval (s1,s2). */
static const float __sj0_j0f_ep_fZ1[] = {
    0x1.6148f6p+2f,   //       5.5200782 [0x40b0a47b]
    -0x1.34f46ep-24f, //  -7.1934146e-08 [0xb39a7a37] Low part
};
static const float __sj0_j0f_ep_fP1[] = {
    -0x1.fbb40ap-56f, //   -2.752265e-17 [0xa3fdda05]
    0x1.5c6e6p-2f,    //       0.3402648 [0x3eae3730]
    -0x1.f8f72ep-6f,  //    -0.030820651 [0xbcfc7b97]
    -0x1.b2150cp-5f,  //    -0.052988552 [0xbd590a86]
    0x1.2f7ffcp-8f,   //    0.0046310415 [0x3b97bffe]
    0x1.27e31ap-9f,   //    0.0022574395 [0x3b13f18d]
    -0x1.6f63b8p-13f, //  -0.00017518498 [0xb937b1dc]
    -0x1.863d84p-15f, //  -4.6520268e-05 [0xb8431ec2]
    0x1.ad60e8p-19f,  //   3.1991194e-06 [0x3656b074]
    0x1.3296cep-21f,  //   5.7106666e-07 [0x35194b67]
    -0x1.2b4334p-25f, //  -3.4838742e-08 [0xb315a19a]
    -0x1.34006ap-28f, //  -4.4820134e-09 [0xb19a0035]
    0x1.d368acp-33f,  //   2.1255295e-10 [0x2f69b456]
};
/* polynomial minimax approximation of J0(x) = P2(x-z2) on interval (s2,s3). */
static const float __sj0_j0f_ep_fZ2[] = {
    0x1.14eb56p+3f,  //       8.6537275 [0x410a75ab]
    0x1.999bdap-22f, //   3.8147792e-07 [0x34cccded] Low part
};
static const float __sj0_j0f_ep_fP2[] = {
    -0x1.6e8eecp-54f, //  -7.9484659e-17 [0xa4b74776]
    -0x1.15f798p-2f,  //     -0.27145231 [0xbe8afbcc]
    0x1.00f7fcp-6f,   //     0.015684124 [0x3c807bfe]
    0x1.68b984p-5f,   //     0.044033773 [0x3d345cc2]
    -0x1.48e634p-9f,  //    -0.002509302 [0xbb24731a]
    -0x1.0e0d5ap-9f,  //   -0.0020603344 [0xbb0706ad]
    0x1.d79582p-14f,  //   0.00011243439 [0x38ebcac1]
    0x1.77feb8p-15f,  //   4.4822096e-05 [0x383bff5c]
    -0x1.3315c8p-19f, //  -2.2879622e-06 [0xb6198ae4]
    -0x1.309016p-21f, //  -5.6729249e-07 [0xb518480b]
    0x1.cbc15ap-26f,  //   2.6761279e-08 [0x32e5e0ad]
    0x1.36bdecp-28f,  //   4.5218895e-09 [0x319b5ef6]
    -0x1.85c2dep-33f, //  -1.7724287e-10 [0xaf42e16f]
};
/* polynomial minimax approximation of J0(x) = P3(x-z3) on interval (s3,s4). */
static const float __sj0_j0f_ep_fZ3[] = {
    0x1.79544p+3f,   //       11.791534 [0x413caa20]
    0x1.04e56cp-26f, //   1.5186156e-08 [0x328272b6] Low part
};
static const float __sj0_j0f_ep_fP3[] = {
    -0x1.2d8ed4p-54f, //  -6.5389951e-17 [0xa496c76a]
    0x1.dc13e6p-3f,   //      0.23245983 [0x3e6e09f3]
    -0x1.42ff0cp-7f,  //   -0.0098570641 [0xbc217f86]
    -0x1.38d1dep-5f,  //     -0.03818601 [0xbd1c68ef]
    0x1.a55e98p-10f,  //    0.0016073971 [0x3ad2af4c]
    0x1.e2e164p-10f,  //    0.0018420427 [0x3af170b2]
    -0x1.3dfbd8p-14f, //  -7.5813237e-05 [0xb89efdec]
    -0x1.5ce5e8p-15f, //  -4.1591891e-05 [0xb82e72f4]
    0x1.bb020ep-20f,  //   1.6503335e-06 [0x35dd8107]
    0x1.22ed3ap-21f,  //   5.4189314e-07 [0x3511769d]
    -0x1.5ee806p-26f, //  -2.0425437e-08 [0xb2af7403]
    -0x1.2eb18p-28f,  //  -4.4047681e-09 [0xb19758c0]
    0x1.39fb7p-33f,   //   1.4278256e-10 [0x2f1cfdb8]
};
/* polynomial minimax approximation of J0(x) = P4(x-z4) on interval (s4,s5). */
static const float __sj0_j0f_ep_fZ4[] = {
    0x1.ddca14p+3f,   //       14.930918 [0x416ee50a]
    -0x1.0d8e2ep-25f, //  -3.1380377e-08 [0xb306c717] Low part
};
static const float __sj0_j0f_ep_fP4[] = {
    -0x1.50be2ep-53f, //  -1.4603895e-16 [0xa5285f17]
    -0x1.a701dp-3f,   //     -0.20654643 [0xbe5380e8]
    0x1.c54b92p-8f,   //    0.0069167358 [0x3be2a5c9]
    0x1.17798ap-5f,   //     0.034115572 [0x3d0bbcc5]
    -0x1.2a214ep-10f, //   -0.0011372761 [0xba9510a7]
    -0x1.b541ecp-10f, //    -0.001668005 [0xbadaa0f6]
    0x1.cc0b46p-15f,  //   5.4841523e-05 [0x386605a3]
    0x1.41f1aep-15f,  //   3.8378723e-05 [0x3820f8d7]
    -0x1.4b127ap-20f, //    -1.23334e-06 [0xb5a5893d]
    -0x1.11cc2ap-21f, //  -5.0998761e-07 [0xb508e615]
    0x1.0fd2b2p-26f,  //   1.5822183e-08 [0x3287e959]
    0x1.219112p-28f,  //   4.2137498e-09 [0x3190c889]
    -0x1.f88bbp-34f,  //   -1.147204e-10 [0xaefc45d8]
};
/* polynomial pade approximation P0(x) = PP(256/x^2) in point 256/x^2 = 0.5 */
static const float __sj0_j0f_ep_fPP[] = {
    0x1p+0f,          //               1 [0x3f800000]
    -0x1.2p-12f,      //   -0.0002746582 [0xb9900000]
    0x1.cb5f86p-20f,  //   1.7112983e-06 [0x35e5afc3]
    -0x1.24f578p-25f, //    -3.41049e-08 [0xb3127abc]
    0x1.7ca5eep-30f,  //   1.3847899e-09 [0x30be52f7]
    -0x1.47a91p-34f,  //   -7.450135e-11 [0xaea3d488]
};
/* polynomial pade approximation Q0(x) = QP(256/x^2)*(16/x)) in point 256/x^2 =
 * 0.5 */
static const float __sj0_j0f_ep_fQP[] = {
    -0x1p-7f,         //      -0.0078125 [0xbc000000]
    0x1.2cp-16f,      //   1.7881393e-05 [0x37960000]
    -0x1.d11ca8p-23f, //  -2.1658462e-07 [0xb4688e54]
    0x1.b9d68ep-28f,  //   6.4295906e-09 [0x31dceb47]
    -0x1.7a8362p-32f, //  -3.4425576e-10 [0xafbd41b1]
    0x1.845fecp-36f,  //   2.2076545e-11 [0x2dc22ff6]
};
static const float __sj0_j0f_ep_fPP_MP[] = {
    0x1.fffffep-1f,
    0x1.ffffcp-25f, // HI + LO:      0.99999994 +   5.9604531e-08 [0x3f7fffff +
                    // 0x337fffe0]
    -0x1.1ffffep-12f,
    -0x1.e81b16p-36f, // HI + LO:  -0.00027465817 +  -2.7745603e-11 [0xb98fffff
                      // + 0xadf40d8b]
    0x1.cb5f86p-20f,
    0x1.449b0ep-45f, // HI + LO:   1.7112983e-06 +    3.603847e-14 [0x35e5afc3 +
                     // 0x29224d87]
    -0x1.24f578p-25f,
    -0x1.081276p-50f, // HI + LO:    -3.41049e-08 +  -9.1618419e-16 [0xb3127abc
                      // + 0xa684093b]
    0x1.7ca5eep-30f,
    0x1.0136f8p-55f, // HI + LO:   1.3847899e-09 +   2.7887276e-17 [0x30be52f7 +
                     // 0x24009b7c]
    -0x1.47a91p-34f,
    -0x1.1e8214p-59f, // HI + LO:   -7.450135e-11 +  -1.9414545e-18 [0xaea3d488
                      // + 0xa20f410a]
};
static const float __sj0_j0f_ep_fQP_MP[] = {
    -0x1.fffffep-8f,
    -0x1.fff4e4p-32f, // HI + LO:   -0.0078124995 +  -4.6562182e-10 [0xbbffffff
                      // + 0xaffffa72]
    0x1.2bfffep-16f,
    0x1.790014p-40f, // HI + LO:   1.7881392e-05 +   1.3393741e-12 [0x3795ffff +
                     // 0x2bbc800a]
    -0x1.d11ca8p-23f,
    -0x1.2ce594p-49f, // HI + LO:  -2.1658462e-07 +  -2.0878909e-15 [0xb4688e54
                      // + 0xa71672ca]
    0x1.b9d68ep-28f,
    0x1.5d6bdcp-54f, // HI + LO:   6.4295906e-09 +   7.5768672e-17 [0x31dceb47 +
                     // 0x24aeb5ee]
    -0x1.7a8362p-32f,
    -0x1.e14eecp-58f, // HI + LO:  -3.4425576e-10 +  -6.5229437e-18 [0xafbd41b1
                      // + 0xa2f0a776]
    0x1.845fecp-36f,
    0x1.b972dap-62f, // HI + LO:   2.2076545e-11 +   3.7392154e-19 [0x2dc22ff6 +
                     // 0x20dcb96d]
};
static const float __sj0_j0f_ep_fptonpi_MP[] = {
    0x1.45f306p-1f, 0x1.b9391p-26f // HI + LO:      0.63661975 +   2.5682553e-08
                                   // [0x3f22f983 + 0x32dc9c88]
};
static inline int __sj0_j0_ep_kernel_fp32(const float *a, float *r) {
  int nRet = 0;
  float xf = *a;
  uint32_t ix = ((*(uint32_t *)&xf) & ~0x80000000);
  float fax = __fabs(xf);
  if (ix < 0x7f800000) /* finite x */
  {
    if (ix < 0x4183c3da) /* 0 <= |x| < 16.4706306457 */
    {
      if (ix < 0x40753aac) /* 0 <= |x| < 3.83170604 */
      {
        if (ix < 0x3e800000) /* 0 <= |x| < 2^(-2) */
        {
          if (ix < 0x3b800000) /* 0 <= |x| < 2^(-8) */
          {
            if (ix < 0x38800000) /* 0 <= |x| < 2^(-14) */
            {
              if (ix < 0x32000000) /* 0 <= |x| < 2^(-27) */
              {
                *r = (1.0f - fax);
                return nRet;
              } else /* 2^(-27) <= |x| < 2^(-14) */
              {
                *r = (1.0f - xf * xf);
                return nRet;
              }
            } else /* 2^(-14) <= |x| < 2^(-8) */
            {
              float fx, fy, fresult;
              fx = xf, fy = fx * fx;
              fresult =
                  (__sj0_j0f_ep_fQ3[1] * fy + __sj0_j0f_ep_fQ3[0]) * fy + 1.0f;
              *r = fresult;
              return nRet;
            }
          } else /* 2^(-8) <= |x| < 2^(-2) */
          {
            float fx, fy, fz, fresult;
            fx = xf, fy = fx * fx, fz = fy * fy;
            fresult = (__sj0_j0f_ep_fQ2[3] * fz + __sj0_j0f_ep_fQ2[1]) * fz +
                      (__sj0_j0f_ep_fQ2[2] * fz + __sj0_j0f_ep_fQ2[0]) * fy +
                      1.0f;
            *r = fresult;
            return nRet;
          }
        } else /* 2^(-2) <= |x| < 3.8317060470 */
        {
          float pax = fax, px, py, pz, pp, pq, presult;
          px = pax - __sj0_j0f_ep_fZ0[0];
          px = px - __sj0_j0f_ep_fZ0[1];
          presult =
              __sj0_j0f_ep_fQ1[0] +
              px *
                  (__sj0_j0f_ep_fQ1[1] +
                   px *
                       (__sj0_j0f_ep_fQ1[2] +
                        px *
                            (__sj0_j0f_ep_fQ1[3] +
                             px *
                                 (__sj0_j0f_ep_fQ1[4] +
                                  px *
                                      (__sj0_j0f_ep_fQ1[5] +
                                       px *
                                           (__sj0_j0f_ep_fQ1[6] +
                                            px *
                                                (__sj0_j0f_ep_fQ1[7] +
                                                 px *
                                                     (__sj0_j0f_ep_fQ1[8] +
                                                      px *
                                                          (__sj0_j0f_ep_fQ1[9] +
                                                           px *
                                                               (__sj0_j0f_ep_fQ1
                                                                    [10] +
                                                                px *
                                                                    (__sj0_j0f_ep_fQ1
                                                                         [11] +
                                                                     px *
                                                                         (__sj0_j0f_ep_fQ1
                                                                              [12] +
                                                                          px *
                                                                              __sj0_j0f_ep_fQ1
                                                                                  [13]))))))))))));
          *r = (float)presult;
          return nRet;
        }
      } else /* 3.831706047 <= |x| < 16.4706306457 */
      {
        const float *pP;
        const float *pZ;
        float px, py, pz, pp, pq, presult, pax = fax;
        if (ix < 0x4122c687) /* 3.83170604705 <= |x| < 10.173468589 */
        {
          if (ix < 0x40e07fb0) /* 7.0155868 */
          {
            pP = __sj0_j0f_ep_fP1;
            pZ = __sj0_j0f_ep_fZ1;
          } else {
            pP = __sj0_j0f_ep_fP2;
            pZ = __sj0_j0f_ep_fZ2;
          }
        } else /* 10.173468589782714843750 <= |x| < 16.4706306457519531250 */
        {
          if (ix < 0x41552dd8) /* 13.323692321777343750 */
          {
            pP = __sj0_j0f_ep_fP3;
            pZ = __sj0_j0f_ep_fZ3;
          } else {
            pP = __sj0_j0f_ep_fP4;
            pZ = __sj0_j0f_ep_fZ4;
          }
        }
        px = pax - pZ[0];
        px = px - pZ[1];
        presult =
            pP[0] +
            px *
                (pP[1] +
                 px *
                     (pP[2] +
                      px *
                          (pP[3] +
                           px *
                               (pP[4] +
                                px *
                                    (pP[5] +
                                     px *
                                         (pP[6] +
                                          px *
                                              (pP[7] +
                                               px *
                                                   (pP[8] +
                                                    px *
                                                        (pP[9] +
                                                         px *
                                                             (pP[10] +
                                                              px *
                                                                  (pP[11] +
                                                                   px *
                                                                       (pP[12]))))))))))));
        *r = (float)presult;
        return nRet;
      }
    } else /* finite |x| >= 16.4706306457519 Hancels asymptotic forms */
    {
      const float ptonpi = 0x1.45f306p-1f; // 0.63661975 [0x3f22f983] 2/Pi
      float px = fax, pxi = 1.0f / px, py = 16.0f * pxi, pz = py * py, pp, pq,
            presult, ps, pc, psq = __sqrt(ptonpi * pxi);
      __sj0_sincos_ep_kernel_fp32(fax, -1, &ps, &pc);
      pp = __sj0_j0f_ep_fPP[0] +
           pz * (__sj0_j0f_ep_fPP[1] +
                 pz * (__sj0_j0f_ep_fPP[2] +
                       pz * (__sj0_j0f_ep_fPP[3] +
                             pz * (__sj0_j0f_ep_fPP[4] +
                                   pz * (__sj0_j0f_ep_fPP[5])))));
      pq = py * (__sj0_j0f_ep_fQP[0] +
                 pz * (__sj0_j0f_ep_fQP[1] +
                       pz * (__sj0_j0f_ep_fQP[2] +
                             pz * (__sj0_j0f_ep_fQP[3] +
                                   pz * (__sj0_j0f_ep_fQP[4] +
                                         pz * (__sj0_j0f_ep_fQP[5]))))));
      *r = psq * (pp * pc - pq * ps);
      return nRet;
    }
  } else /* INF or NaN */
  {
    if (ix <= 0x7f800000) /* INF */
    {
      *r = 0.0f;
      return nRet;
    } else /* NaN */
    {
      *r = xf * 1.0f; /* raise invalid on SNaN, return QNaN */
      return nRet;
    }
  }
}

inline int __devicelib_imf_internal_sj0(const float *a, float *r) {
  return __sj0_j0_ep_kernel_fp32(a, r);
}
} /* namespace */
} /* namespace __imf_impl_j0_s_ep */
DEVICE_EXTERN_C_INLINE float __devicelib_imf_j0f(float x) {
  using namespace __imf_impl_j0_s_ep;
  float r;
  __devicelib_imf_internal_sj0(&x, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
