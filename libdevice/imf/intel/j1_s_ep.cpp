/*******************************************************************************
 * INTEL CONFIDENTIAL
 * Copyright 1996 Intel Corporation.
 *
 * This software and the related documents are Intel copyrighted  materials, and
 * your use of  them is  governed by the  express license  under which  they
 *were provided to you (License).  Unless the License provides otherwise, you
 *may not use, modify, copy, publish, distribute,  disclose or transmit this
 *software or the related documents without Intel's prior written permission.
 *
 * This software and the related documents  are provided as  is,  with no
 *express or implied  warranties,  other  than those  that are  expressly stated
 *in the License.
 *******************************************************************************/
#include "_imf_include_fp32.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_j1_s_ep {
namespace {
static const int32_t __sj1_ep___ip_h = 0x0517CC1B;
static const int32_t __sj1_ep___ip_m = 0x727220A9;
static const int32_t __sj1_ep___ip_l = 0x28;
static const union {
  uint32_t w;
  float f;
} __sj1_ep___cc4 = {0x3e6ce1b2u};
static const union {
  uint32_t w;
  float f;
} __sj1_ep___cc3 = {0xbfaae2beu};
static const union {
  uint32_t w;
  float f;
} __sj1_ep___cc2 = {0x4081e0eeu};
static const union {
  uint32_t w;
  float f;
} __sj1_ep___cc1 = {0xc09de9e6u};
static const union {
  uint32_t w;
  float f;
} __sj1_ep___cc1l = {0xb3e646a5u};
static const union {
  uint32_t w;
  float f;
} __sj1_ep___cc0 = {0x3f800000u};
static const union {
  uint32_t w;
  float f;
} __sj1_ep___cs3 = {0xbf16c981u};
static const union {
  uint32_t w;
  float f;
} __sj1_ep___cs2 = {0x40232f49u};
static const union {
  uint32_t w;
  float f;
} __sj1_ep___cs1 = {0xc0a55dddu};
static const union {
  uint32_t w;
  float f;
} __sj1_ep___cs0 = {0x40490fdbu};
static const union {
  uint32_t w;
  float f;
} __sj1_ep___cs0l = {0xb3d195e9u};
static const uint32_t __sj1_ep_invpi_tbl[] = {
    0,          0x28BE60DB, 0x9391054A, 0x7F09D5F4,
    0x7D4D3770, 0x36D8A566, 0x4F10E410, 0x7F9458EA};
static int __sj1_ep_sincos_kernel_fp32(float xf, int n, float *psin,
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
    ip_low = (((uint32_t)__sj1_ep_invpi_tbl[index]) * ((uint32_t)mx));
    IP = (((uint64_t)((uint32_t)(__sj1_ep_invpi_tbl[index + 1]))) *
          ((uint32_t)(mx))) +
         (((uint64_t)ip_low) << 32);
    // scaled by 2^(95-j)
    IP2 = (((uint64_t)((uint32_t)(__sj1_ep_invpi_tbl[index + 2]))) *
           ((uint32_t)(mx))) +
          ((((uint64_t)((uint32_t)(__sj1_ep_invpi_tbl[index + 3]))) *
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
    IP_s = (((int64_t)((int32_t)(mx))) * ((int32_t)(__sj1_ep___ip_h)));
    IP = (uint64_t)IP_s;
    IP2_s = (((int64_t)((int32_t)(mx))) * ((int32_t)(__sj1_ep___ip_m)));
    IP2 = (uint64_t)IP2_s;
    // scale (23-ex)*2^(28+32+7)
    ip_low_s = (((int32_t)mx) * ((int32_t)__sj1_ep___ip_l));
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
  cpoly.f = __fma(__sj1_ep___cc4.f, R2h, __sj1_ep___cc3.f);
  spoly.f = __fma(__sj1_ep___cs3.f, R2h, __sj1_ep___cs2.f);
  cpoly.f = __fma(cpoly.f, R2h, __sj1_ep___cc2.f);
  spoly.f = __fma(spoly.f, R2h, __sj1_ep___cs1.f);
  cpoly.f = __fma(cpoly.f, R2h, __sj1_ep___cc1.f);
  spoly.f = __fma(spoly.f, R2h, __sj1_ep___cs0.f);
  cpoly.f = __fma(cpoly.f, R2h, __sj1_ep___cc0.f);
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
/* Q2: polynomial minimax approximation J1(x) = y+y*z*Q2(z), y=x/2, z=y^2,
 * |x|=[0..2^(-2)], max.err .14e-11 */
static const float __sj1_ep_fQ2[] = {
    -0x1p-1,        //            -0.5 [0xbf000000]   -0x1.fffffffff9f45p-2 , //
                    //            -0.499999999998625377     [0xBFDFFFFFFFFF9F45]
    0x1.555556p-4,  //     0.083333336 [0x3daaaaab]   0x1.55555524f34dbp-4  , //
                    //     0.0833333326292690496     [0x3FB55555524F34DB]
    -0x1.c71b8p-8,  //   -0.0069443882 [0xbbe38dc0]   -0x1.c71b7fc14bb27p-8 , //
                    //   -0.00694438809413958687   [0xBF7C71B7FC14BB27]
    0x1.6a933ap-12, //   0.00034577856 [0x39b5499d]   0x1.6a933942150ecp-12 , //
                    //   0.000345778553428445793   [0x3F36A933942150EC]
};
/* Q1: polynomial minimax approximation J1(x) = Q1(x) on interval (0,s1). */
static const float __sj1_ep_fQ1[] = {
    0x1p-1,          //             0.5 [0x3f000000]
    0x1.0aae7p-33,   //   1.2127266e-10 [0x2f055738]
    -0x1p-4,         //         -0.0625 [0xbd800000]
    0x1.7f25d8p-28,  //   5.5755347e-09 [0x31bf92ec]
    0x1.5554cap-9,   //    0.0026041504 [0x3b2aaa65]
    0x1.06d88p-25,   //   3.0599267e-08 [0x33036c40]
    -0x1.c76fbp-15,  //  -5.4292235e-05 [0xb863b7d8]
    0x1.22ffb2p-25,  //    3.387672e-08 [0x33117fd9]
    0x1.610ccep-21,  //    6.576069e-07 [0x35308667]
    0x1.26b396p-27,  //   8.5769427e-09 [0x321359cb]
    -0x1.13d196p-27, //  -8.0273805e-09 [0xb209e8cb]
    0x1.bba226p-32,  //   4.0348222e-10 [0x2fddd113]
};
static const float __sj1_ep_fZ1[] = {
    0x1.ea7558p+1,   //        3.831706 [0x40753aac]
    -0x1.4a121ep-24, //   -7.685059e-08 [0x40753aac]
};
static const float __sj1_ep_fZ2[] = {
    0x1.c0ff6p+2,    //       7.0155869 [0x40e07fb0]
    -0x1.8971b6p-23, //  -1.8321172e-07 [0x40e07fb0]
};
static const float __sj1_ep_fZ3[] = {
    0x1.458d0ep+3,   //       10.173469 [0x4122c687]
    -0x1.e8407ap-22, //  -4.5471998e-07 [0x4122c687]
};
static const float __sj1_ep_fZ4[] = {
    0x1.aa5bbp+3,    //       13.323692 [0x41552dd8]
    -0x1.9de34cp-22, //  -3.8546312e-07 [0x41552dd8]
};
static const float __sj1_ep_fZ5[] = {
    0x1.0787b4p+4,   //       16.470631 [0x4183c3da]
    -0x1.3f5ee8p-21, //  -5.9487434e-07 [0x4183c3da]
};
/* P1: polynomial minimax approximation J1(x) = P1(x-z2) on interval (s1,s2). */
static const float __sj1_ep_fP1[] = {
    -0x1.1b9c1cp-54, //  -6.1498073e-17 [0xa48dce0e]
    -0x1.9c6cf6p-2,  //      -0.4027594 [0xbece367b]
    0x1.ae8a3ap-5,   //     0.052556146 [0x3d57451d]
    0x1.b589d2p-5,   //     0.053410444 [0x3d5ac4e9]
    -0x1.537546p-8,  //   -0.0051797195 [0xbba9baa3]
    -0x1.24b33ep-9,  //    -0.002233125 [0xbb12599f]
    0x1.6e4c84p-13,  //   0.00017466492 [0x39372642]
    0x1.839f2ap-15,  //   4.6208112e-05 [0x3841cf95]
    -0x1.97ad98p-19, //  -3.0374385e-06 [0xb64bd6cc]
    -0x1.334224p-21, //   -5.723133e-07 [0xb519a112]
    0x1.190906p-25,  //   3.2716809e-08 [0x330c8483]
    0x1.38e59cp-28,  //   4.5532493e-09 [0x319c72ce]
    -0x1.274c5cp-32, //  -2.6857222e-10 [0xaf93a62e]
};
/* P2: polynomial minimax approximation J1(x) = P2(x-z3) on interval (s2,s3). */
static const float __sj1_ep_fP2[] = {
    0x1.04977p-55,   //   2.8253393e-17 [0x24024bb8]
    0x1.33518cp-2,   //      0.30011576 [0x3e99a8c6]
    -0x1.5e70dcp-6,  //    -0.021389212 [0xbcaf386e]
    -0x1.80c83cp-5,  //    -0.046970479 [0xbd40641e]
    0x1.9a4b2ap-9,   //    0.0031302918 [0x3b4d2595]
    0x1.13fbc2p-9,   //    0.0021055865 [0x3b09fde1]
    -0x1.0735c2p-13, //   -0.0001255083 [0xb9039ae1]
    -0x1.79689cp-15, //  -4.4990615e-05 [0xb83cb44e]
    0x1.426118p-19,  //   2.4019128e-06 [0x3621308c]
    0x1.2fd368p-21,  //   5.6591966e-07 [0x3517e9b4]
    -0x1.d6b132p-26, //   -2.739789e-08 [0xb2eb5899]
    -0x1.36165p-28,  //   -4.512362e-09 [0xb19b0b28]
    0x1.e551e2p-33,  //   2.2069792e-10 [0x2f72a8f1]
};
/* P3: polynomial minimax approximation J1(x) = P3(x-z4) on interval (s3,s4). */
static const float __sj1_ep_fP3[] = {
    0x1.0212f4p-53,  //   1.1192177e-16 [0x2501097a]
    -0x1.ff6546p-3,  //     -0.24970488 [0xbe7fb2a3]
    0x1.9224p-7,     //     0.012272358 [0x3c491200]
    0x1.4b0c5ep-5,   //     0.040411171 [0x3d25862f]
    -0x1.f91aa2p-10, //    -0.001926819 [0xbafc8d51]
    -0x1.f51c1ap-10, //   -0.0019115821 [0xbafa8e0d]
    0x1.6b4ce6p-14,  //   8.6617561e-05 [0x38b5a673]
    0x1.63c34ep-15,  //   4.2410244e-05 [0x3831e1a7]
    -0x1.e381b4p-20, //  -1.8012026e-06 [0xb5f1c0da]
    -0x1.2569e2p-21, //  -5.4652543e-07 [0xb512b4f1]
    0x1.75ea44p-26,  //   2.1764723e-08 [0x32baf522]
    0x1.2fa7fcp-28,  //   4.4187791e-09 [0x3197d3fe]
    -0x1.8a59bap-33, //  -1.7932984e-10 [0xaf452cdd]
};
/* P4: polynomial minimax approximation J1(x) = P4(x-z5) on interval (s4,s5). */
static const float __sj1_ep_fP4[] = {
    -0x1.05dcc6p-54, //  -5.6782356e-17 [0xa482ee63]
    0x1.bf3338p-3,   //      0.21835941 [0x3e5f999c]
    -0x1.0c83a2p-7,  //   -0.0081944028 [0xbc0641d1]
    -0x1.251858p-5,  //     -0.03577821 [0xbd128c2c]
    0x1.59eb18p-10,  //    0.0013195737 [0x3aacf58c]
    0x1.c5bcd8p-10,  //    0.0017308719 [0x3ae2de6c]
    -0x1.04141ap-14, //  -6.2007552e-05 [0xb8820a0d]
    -0x1.4a650ap-15, //  -3.9386116e-05 [0xb8253285]
    0x1.6c4f28p-20,  //   1.3571575e-06 [0x35b62794]
    0x1.1654fap-21,  //   5.1843364e-07 [0x350b2a7d]
    -0x1.267d82p-26, //  -1.7141589e-08 [0xb2933ec1]
    -0x1.247a6ep-28, //  -4.2561186e-09 [0xb1923d37]
    0x1.3f25fap-33,  //   1.4513186e-10 [0x2f1f92fd]
};
/* P5: polynomial minimax approximation J1(x) = PS(x-z6) on interval (s5,s6). */
static const float __sj1_ep_fP5[] = {
    -0x1.6eb906p-52, //  -3.1808128e-16 [0xa5b75c83]
    -0x1.925c7p-3,   //     -0.19646537 [0xbe492e38]
    0x1.86dd32p-8,   //     0.005964112 [0x3bc36e99]
    0x1.09463cp-5,   //     0.032382123 [0x3d04a31e]
    -0x1.fda02cp-11, //  -0.00097203383 [0xba7ed016]
    -0x1.9f4bdap-10, //   -0.0015842296 [0xbacfa5ed]
    0x1.8779e6p-15,  //   4.6667596e-05 [0x3843bcf3]
    0x1.32c912p-15,  //   3.6571673e-05 [0x38196489]
    -0x1.19e174p-20, //  -1.0500873e-06 [0xb58cf0ba]
    -0x1.064b94p-21, //  -4.8856293e-07 [0xb50325ca]
    0x1.d52916p-27,  //   1.3654367e-08 [0x326a948b]
    0x1.171c38p-28,  //   4.0615884e-09 [0x318b8e1c]
    -0x1.03f3f6p-33, //  -1.1821293e-10 [0xaf01f9fb]
};
/* PP: polynomial pade approximation P1(x) = PP(256/x^2) in point 256/x^2 = 0.5
 */
static const float __sj1_ep_fPP[] = {
    0x1p+0,          //               1 [0x3f800000]
    0x1.ep-12,       //   0.00045776367 [0x39f00000]
    -0x1.274fbep-19, //  -2.2002421e-06 [0xb613a7df]
    0x1.5a3d1ep-25,  //   4.0307494e-08 [0x332d1e8f]
    -0x1.afbe9cp-30, //  -1.5706776e-09 [0xb0d7df4e]
    0x1.6be2b6p-34,  //   8.2738004e-11 [0x2eb5f15b]
};
/* QP: polynomial pade approximation Q1(x) = QP(256/x^2)*(16/x)) in point
 * 256/x^2 = 0.5 */
static const float __sj1_ep_fQP[] = {
    0x1.8p-6,        //       0.0234375 [0x3cc00000]
    -0x1.a4p-16,     //  -2.5033951e-05 [0xb7d20000]
    0x1.1c3c46p-22,  //   2.6471488e-07 [0x348e1e23]
    -0x1.fdd85cp-28, //  -7.4192235e-09 [0xb1feec2e]
    0x1.a76f66p-32,  //   3.8511203e-10 [0x2fd3b7b3]
    -0x1.ab6366p-36, //  -2.4294211e-11 [0xadd5b1b3]
};
inline int __devicelib_imf_internal_sj1(const float *a, float *r) {
  int nRet = 0;
  int32_t sign;
  uint32_t iax;
  float xf = *a;
  float resultf;
  iax = ((*(int32_t *)&xf) & ~0x80000000);
  sign = (((uint32_t)(*(int32_t *)&xf)) >> 31);
  if (iax < 0x7f800000) /* finite x */
  {
    if (iax < 0x4190918a) /* x < 18.0710639953613281250 */
    {
      if (iax < 0x4019e8a9) /*  x < 2.40482544898986816406250 */
      {
        if (iax < 0x3e800000) /* 0 <= |x| < 2^(-2) */
        {
          if (iax < 0x3b800000) /* 0 <= |x| < 2^(-8) */
          {
            if (iax < 0x32000000) /* |x| < 2^(-27) */
            {
              if (iax != 0) {
                /* return x/2 */
                const float small_value_32[] = {
                    0x1.000000p-100,
                    -0x1.000000p-100}; /* +2^(-100),-2^(-100) */
                resultf =
                    (xf * 0.5f - (small_value_32[(sign)] * small_value_32[0]));
              } else /* Zero */
              {
                resultf = xf;
              }
              *r = resultf;
              return nRet;
            } else /* |x| >= 2^(-27) */
            {
              float fx, fy, fresult;
              fx = (xf * 0.5f);
              fy = fx * fx;
              fresult =
                  ((__sj1_ep_fQ2[1] * fy + __sj1_ep_fQ2[0]) * fy) * fx + fx;
              *r = fresult;
              return nRet;
            }
          } else /* |x| >= 2^(-8) */
          {
            float fx, fy, fz, fresult;
            fx = xf * 0.5f;
            fy = fx * fx;
            fz = fy * fy;
            fresult = ((__sj1_ep_fQ2[3] * fz + __sj1_ep_fQ2[1]) * fz +
                       (__sj1_ep_fQ2[2] * fz + __sj1_ep_fQ2[0]) * fy) *
                          fx +
                      fx;
            *r = fresult;
            return nRet;
          }
        } else /* |x| >= 2^(-2) */
        {
          float px = __fabs(xf), presult;
          presult =
              px *
              (__sj1_ep_fQ1[0] +
               px *
                   (__sj1_ep_fQ1[1] +
                    px *
                        (__sj1_ep_fQ1[2] +
                         px *
                             (__sj1_ep_fQ1[3] +
                              px *
                                  (__sj1_ep_fQ1[4] +
                                   px *
                                       (__sj1_ep_fQ1[5] +
                                        px *
                                            (__sj1_ep_fQ1[6] +
                                             px *
                                                 (__sj1_ep_fQ1[7] +
                                                  px *
                                                      (__sj1_ep_fQ1[8] +
                                                       px *
                                                           (__sj1_ep_fQ1[9] +
                                                            px *
                                                                (__sj1_ep_fQ1
                                                                     [10] +
                                                                 px *
                                                                     (__sj1_ep_fQ1
                                                                          [11]))))))))))));
          *r = sign ? -presult : presult;
          return nRet;
        }
      } else /*  x >= 2.40482544898986816406250 */
      {
        const float *pP;
        const float *pZ;
        float px = __fabs(xf), py, pz, pp, pq, presult;
        if (iax < 0x416ee50a) /* x < 14.93091773986816406250 */
        {
          if (iax < 0x410a75ab) /* x < 8.653727531433105468750 */
          {
            if (iax < 0x40b0a47b) /* 5.5200781822204589843750 */
            {
              pP = __sj1_ep_fP1;
              pZ = __sj1_ep_fZ1;
            } else {
              pP = __sj1_ep_fP2;
              pZ = __sj1_ep_fZ2;
            }
          } else {
            if (iax < 0x413caa20) /* 11.7915344238281250 */
            {
              pP = __sj1_ep_fP3;
              pZ = __sj1_ep_fZ3;
            } else {
              pP = __sj1_ep_fP4;
              pZ = __sj1_ep_fZ4;
            }
          }
        } else /* x >= 14.93091773986816406250 */
        {
          pP = __sj1_ep_fP5;
          pZ = __sj1_ep_fZ5;
        }
        px = px - pZ[0];
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
        *r = (sign ? -presult : presult);
        return nRet;
      }
    } else /* x >= 18.0710639953613281250 Hancels asymptotic forms */
    {
      const float ptonpi = 0x1.45f306p-1; // 0.63661975 [0x3f22f983] 2/Pi
      float px = __fabs(xf), pxi = 1.0f / px, py = 16.0f * pxi, pz = py * py,
            psn, pcs, pp, pq, presult, psq = __sqrt(ptonpi * pxi);
      __sj1_ep_sincos_kernel_fp32(px, -3, &psn, &pcs);
      pp = __sj1_ep_fPP[0] +
           pz * (__sj1_ep_fPP[1] +
                 pz * (__sj1_ep_fPP[2] +
                       pz * (__sj1_ep_fPP[3] +
                             pz * (__sj1_ep_fPP[4] + pz * (__sj1_ep_fPP[5])))));
      ;
      pq = py * (__sj1_ep_fQP[0] +
                 pz * (__sj1_ep_fQP[1] +
                       pz * (__sj1_ep_fQP[2] +
                             pz * (__sj1_ep_fQP[3] +
                                   pz * (__sj1_ep_fQP[4] +
                                         pz * (__sj1_ep_fQP[5]))))));
      presult = psq * (pp * pcs - pq * psn);
      *r = (float)(sign ? -presult : presult);
      return nRet;
    }
  } else /* INF or NaN */
  {
    if (iax <= 0x7f800000) /* INF */
    {
      const float zeros[] = {0x0.0p+0, -0x0.0p+0};
      *r = zeros[sign];
      return nRet;
    } else /* NaN */
    {
      /* raise invalid on SNaN, return QNaN */
      *r = xf * 1.0f;
      return nRet;
    }
  }
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_j1_s_ep */
DEVICE_EXTERN_C_INLINE float __devicelib_imf_j1f(float x) {
  using namespace __imf_impl_j1_s_ep;
  float r;
  __devicelib_imf_internal_sj1(&x, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
