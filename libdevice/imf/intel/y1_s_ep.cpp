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
namespace __imf_impl_y1_s_ep {
namespace {
static const int32_t __sy1_ep___ip_h = 0x0517CC1B;
static const int32_t __sy1_ep___ip_m = 0x727220A9;
static const int32_t __sy1_ep___ip_l = 0x28;
static const union {
  uint32_t w;
  float f;
} __sy1_ep___cc4 = {0x3e6ce1b2u};
static const union {
  uint32_t w;
  float f;
} __sy1_ep___cc3 = {0xbfaae2beu};
static const union {
  uint32_t w;
  float f;
} __sy1_ep___cc2 = {0x4081e0eeu};
static const union {
  uint32_t w;
  float f;
} __sy1_ep___cc1 = {0xc09de9e6u};
static const union {
  uint32_t w;
  float f;
} __sy1_ep___cc1l = {0xb3e646a5u};
static const union {
  uint32_t w;
  float f;
} __sy1_ep___cc0 = {0x3f800000u};
static const union {
  uint32_t w;
  float f;
} __sy1_ep___cs3 = {0xbf16c981u};
static const union {
  uint32_t w;
  float f;
} __sy1_ep___cs2 = {0x40232f49u};
static const union {
  uint32_t w;
  float f;
} __sy1_ep___cs1 = {0xc0a55dddu};
static const union {
  uint32_t w;
  float f;
} __sy1_ep___cs0 = {0x40490fdbu};
static const union {
  uint32_t w;
  float f;
} __sy1_ep___cs0l = {0xb3d195e9u};
static const uint32_t __sy1_ep_invpi_tbl[] = {
    0,          0x28BE60DB, 0x9391054A, 0x7F09D5F4,
    0x7D4D3770, 0x36D8A566, 0x4F10E410, 0x7F9458EA};
static int __sy1_ep_sincos_kernel_fp32(float xf, int n, float *psin,
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
    ip_low = (((uint32_t)__sy1_ep_invpi_tbl[index]) * ((uint32_t)mx));
    IP = (((uint64_t)((uint32_t)(__sy1_ep_invpi_tbl[index + 1]))) *
          ((uint32_t)(mx))) +
         (((uint64_t)ip_low) << 32);
    // scaled by 2^(95-j)
    IP2 = (((uint64_t)((uint32_t)(__sy1_ep_invpi_tbl[index + 2]))) *
           ((uint32_t)(mx))) +
          ((((uint64_t)((uint32_t)(__sy1_ep_invpi_tbl[index + 3]))) *
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
    IP_s = (((int64_t)((int32_t)(mx))) * ((int32_t)(__sy1_ep___ip_h)));
    IP = (uint64_t)IP_s;
    IP2_s = (((int64_t)((int32_t)(mx))) * ((int32_t)(__sy1_ep___ip_m)));
    IP2 = (uint64_t)IP2_s;
    // scale (23-ex)*2^(28+32+7)
    ip_low_s = (((int32_t)mx) * ((int32_t)__sy1_ep___ip_l));
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
  cpoly.f = __fma(__sy1_ep___cc4.f, R2h, __sy1_ep___cc3.f);
  spoly.f = __fma(__sy1_ep___cs3.f, R2h, __sy1_ep___cs2.f);
  cpoly.f = __fma(cpoly.f, R2h, __sy1_ep___cc2.f);
  spoly.f = __fma(spoly.f, R2h, __sy1_ep___cs1.f);
  cpoly.f = __fma(cpoly.f, R2h, __sy1_ep___cc1.f);
  spoly.f = __fma(spoly.f, R2h, __sy1_ep___cs0.f);
  cpoly.f = __fma(cpoly.f, R2h, __sy1_ep___cc0.f);
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
/* file: _vsln_kernel_cout.i */
static int __sy1_ep_ln_kernel_fp32(float x, float *r) {
  int32_t lessmin = (x < 1.17549435e-38F);
  float s = x * (1 << 23);
  float e = lessmin ? -23 : 0.0f;
  uint32_t ux = as_uint(x);
  uint32_t uax = ux & 0x7fffffff;
  if (ux == 0x7f800000) {
    *r = x;
    return 0;
  } else if (uax > 0x7f800000) {
    *r = x;
    return 0;
  } else if (x == 0.0) {
    uint32_t ires = 0xff800000;
    *r = *(float *)&ires;
    return -1;
  } else if (x < 0.0) {
    uint32_t ires = 0xffffffff;
    *r = *(float *)&ires;
    return -2;
  }
  x = lessmin ? s : x;
  const int mm = 0x3f2aaaab;
  int iX = (*(int *)&(x)) - mm;
  int iR = (iX & 0x007FFFFF) + mm;
  e += iX >> 23;
  float sR = as_float(iR) - 1.0f;
  float sP = -0x1.080590p-3F;
  sP = __fma(sP, sR, 0x1.1e66bap-3F);
  sP = __fma(sP, sR, -0x1.f3113cp-4F);
  sP = __fma(sP, sR, 0x1.1ed718p-3F);
  sP = __fma(sP, sR, -0x1.559dccp-3F);
  sP = __fma(sP, sR, 0x1.99d028p-3F);
  sP = __fma(sP, sR, -0x1.fffef0p-3F);
  sP = __fma(sP, sR, 0x1.555506p-2F);
  sP = __fma(sP, sR, -0x1.000000p-1F);
  sP = sP * sR;
  sP = __fma(sP, sR, sR);
  sP = __fma(e, 0x1.7f7d1cp-20F, sP);
  sP = __fma(e, 0x1.62e400p-1F, sP);
  *r = sP;
  return 0;
}
/* polynomial minimax approximation Y1(x) = x*Q1(x^2)+2/pi*(j0(x)*log(x)-1/x) on
 * interval (MinVal,1.77). */
static const float __sy1_ep_fQ1[] = {
    -0x1.918662p-3,  //      -0.1960571 [0xbe48c331]
    0x1.bd3974p-5,   //     0.054348685 [0x3d5e9cba]
    -0x1.835b76p-9,  //   -0.0029553014 [0xbb41adbb]
    0x1.2c795cp-14,  //   7.1638598e-05 [0x38963cae]
    -0x1.09e396p-20, //  -9.9051374e-07 [0xb584f1cb]
    0x1.1fc6b2p-27,  //   8.3753884e-09 [0x320fe359]
};
static const float __sy1_ep_fQ1_MP[] = {
    -0x1.91866p-3,
    -0x1.3b5d12p-27, // HI + LO:     -0.19605708 +  -9.1782875e-09 [0xbe48c330 +
                     // 0xb21dae89]
    0x1.bd3974p-5,
    0x1.c599f6p-30, // HI + LO:     0.054348685 +   1.6501923e-09 [0x3d5e9cba +
                    // 0x30e2ccfb]
    -0x1.835b76p-9,
    -0x1.825414p-35, // HI + LO:   -0.0029553014 +  -4.3920458e-11 [0xbb41adbb +
                     // 0xae412a0a]
    0x1.2c795cp-14,
    0x1.33037ap-40, // HI + LO:   7.1638598e-05 +   1.0907313e-12 [0x38963cae +
                    // 0x2b9981bd]
    -0x1.09e394p-20,
    -0x1.25761p-44, // HI + LO:  -9.9051363e-07 +  -6.5161472e-14 [0xb584f1ca +
                    // 0xa992bb08]
    0x1.1fc6bp-27,
    0x1.590678p-51, // HI + LO:   8.3753875e-09 +   5.9852343e-16 [0x320fe358 +
                    // 0x262c833c]
};
/* polynomial minimax approximation of J0(x) = 1+y*Q2(y), y=x^2,
 * |x|=[0..1.7699999809265136718750], max.err .15e-10 */
static const float __sy1_ep_fQ2[] = {
    0x1p-1,          //             0.5 [0x3f000000]
    -0x1p-4,         //         -0.0625 [0xbd800000]
    0x1.55554ap-9,   //    0.0026041653 [0x3b2aaaa5]
    -0x1.c718b6p-15, //  -5.4251734e-05 [0xb8638c5b]
    0x1.6b8304p-21,  //   6.7709345e-07 [0x3535c182]
    -0x1.6f3516p-28, //  -5.3435705e-09 [0xb1b79a8b]
};
static const float __sy1_ep_fQ2_MP[] = {
    0x1.fffffep-2,
    0x1.ffbef6p-26, // HI + LO:      0.49999997 +   2.9787534e-08 [0x3effffff +
                    // 0x32ffdf7b]
    -0x1.fffffep-5,
    -0x1.d12e0ep-29, // HI + LO:    -0.062499996 +  -3.3846292e-09 [0xbd7fffff +
                     // 0xb1689707]
    0x1.55554ap-9,
    0x1.a22f18p-35, // HI + LO:    0.0026041653 +   4.7542012e-11 [0x3b2aaaa5 +
                    // 0x2e51178c]
    -0x1.c718b6p-15,
    -0x1.ce322cp-41, // HI + LO:  -5.4251734e-05 +    -8.21025e-13 [0xb8638c5b +
                     // 0xab671916]
    0x1.6b8302p-21,
    0x1.12c5dp-45, // HI + LO:    6.770934e-07 +   3.0505898e-14 [0x3535c181 +
                   // 0x290962e8]
    -0x1.6f3514p-28,
    -0x1.dca096p-52, // HI + LO:    -5.34357e-09 +  -4.1340827e-16 [0xb1b79a8a +
                     // 0xa5ee504b]
};
/* polynomial minimax approximation Y1(x) = P1(x-z1) on interval (1.77,s1). */
static const float __sy1_ep_fZ1_MP[] = {
    0x1.193becp+1, 0x1.4dff24p-23, // HI + LO:       2.1971412 +   1.5552931e-07
                                   // [0x400c9df6 + 0x3426ff92]
};
static const float __sy1_ep_fP1[] = {
    0x1.cf9f8ep-56,  //   2.5133067e-17 [0x23e7cfc7]
    0x1.0aa484p-1,   //       0.5207864 [0x3f055242]
    -0x1.e56f82p-4,  //     -0.11851455 [0xbdf2b7c1]
    -0x1.0d2af4p-5,  //    -0.032857396 [0xbd06957a]
    -0x1.3a6e26p-8,  //   -0.0047978251 [0xbb9d3713]
    0x1.e6719ap-8,   //    0.0074225427 [0x3bf338cd]
    -0x1.542476p-9,  //   -0.0025950808 [0xbb2a123b]
    0x1.17a122p-10,  //    0.0010667016 [0x3a8bd091]
    -0x1.0b84bp-11,  //  -0.00051025068 [0xba05c258]
    0x1.f20a68p-13,  //   0.00023748429 [0x39790534]
    -0x1.c6778ep-14, //  -0.00010835338 [0xb8e33bc7]
    0x1.85f5bep-15,  //   4.6486846e-05 [0x3842fadf]
    -0x1.1b4ee8p-16, //  -1.6886486e-05 [0xb78da774]
    0x1.31e66cp-18,  //   4.5582665e-06 [0x3698f336]
    -0x1.a196cap-21, //  -7.7782016e-07 [0xb550cb65]
    0x1.0849eap-24,  //   6.1534514e-08 [0x338424f5]
};
/* polynomial minimax approximation Y1(x) = P2(x-z2) on interval (s1,s2). */
static const float __sy1_ep_fZ2_MP[] = {
    0x1.5b7fe4p+2, 0x1.d0f606p-23, // HI + LO:       5.4296808 +   2.1651435e-07
                                   // [0x40adbff2 + 0x34687b03]
};
static const float __sy1_ep_fP2[] = {
    0x1.46a40cp-53,  //   1.4165787e-16 [0x25235206]
    -0x1.5c7c56p-2,  //     -0.34031805 [0xbeae3e2b]
    0x1.00b9f8p-5,   //     0.031338677 [0x3d005cfc]
    0x1.a15d92p-5,   //     0.050947938 [0x3d50aec9]
    -0x1.10a32ap-8,  //    -0.004160116 [0xbb885195]
    -0x1.1be6dcp-9,  //   -0.0021659988 [0xbb0df36e]
    0x1.337c9ap-13,  //    0.0001466211 [0x3919be4d]
    0x1.85b966p-15,  //   4.6458747e-05 [0x3842dcb3]
    -0x1.8069b6p-19, //  -2.8640995e-06 [0xb64034db]
    -0x1.25631cp-21, //  -5.4647614e-07 [0xb512b18e]
    0x1.b8b014p-26,  //   2.5651406e-08 [0x32dc580a]
    0x1.80972cp-28,  //   5.5965286e-09 [0x31c04b96]
    -0x1.d7f07ap-32, //  -4.2922635e-10 [0xafebf83d]
    0x1.21937ap-37,  //   8.2302472e-12 [0x2d10c9bd]
};
/* polynomial minimax approximation Y1(x) = P3(x-z3) on interval (s2,s3). */
static const float __sy1_ep_fZ3_MP[] = {
    0x1.13127ap+3, 0x1.cc2d36p-22, // HI + LO:       8.5960054 +   4.2857286e-07
                                   // [0x4109893d + 0x34e6169b]
};
static const float __sy1_ep_fP3[] = {
    -0x1.63bc02p-54, //  -7.7137603e-17 [0xa4b1de01]
    0x1.15f994p-2,   //      0.27145988 [0x3e8afcca]
    -0x1.02b394p-6,  //    -0.015789885 [0xbc8159ca]
    -0x1.6395ep-5,   //    -0.043406427 [0xbd31caf0]
    0x1.3ced2cp-9,   //    0.0024179569 [0x3b1e7696]
    0x1.07a674p-9,   //    0.0020114915 [0x3b03d33a]
    -0x1.b50dd6p-14, //   -0.0001042018 [0xb8da86eb]
    -0x1.6f79bcp-15, //  -4.3806496e-05 [0xb837bcde]
    0x1.1777aep-19,  //   2.0821951e-06 [0x360bbbd7]
    0x1.2b9662p-21,  //   5.5802508e-07 [0x3515cb31]
    -0x1.a5371p-26,  //  -2.4517945e-08 [0xb2d29b88]
    -0x1.335ab2p-28, //  -4.4725934e-09 [0xb199ad59]
    0x1.b52aa6p-33,  //   1.9880035e-10 [0x2f5a9553]
};
/* polynomial minimax approximation Y1(x) = P4(x-z4) on interval (s3,s4). */
static const float __sy1_ep_fZ4_MP[] = {
    0x1.77f912p+3, 0x1.8d432p-21, // HI + LO:       11.749154 +   7.3995852e-07
                                  // [0x413bfc89 + 0x3546a190]
};
static const float __sy1_ep_fP4[] = {
    0x1.f96d32p-58,  //    6.849807e-18 [0x22fcb699]
    -0x1.dc14eap-3,  //     -0.23246177 [0xbe6e0a75]
    0x1.4429fep-7,   //    0.0098927012 [0x3c2214ff]
    0x1.367d7ep-5,   //     0.037901636 [0x3d1b3ebf]
    -0x1.9d6eb2p-10, //   -0.0015771195 [0xbaceb759]
    -0x1.dc4f98p-10, //   -0.0018169819 [0xbaee27cc]
    0x1.315eb8p-14,  //    7.280588e-05 [0x3898af5c]
    0x1.571808p-15,  //   4.0899977e-05 [0x382b8c04]
    -0x1.a294c6p-20, //  -1.5593363e-06 [0xb5d14a63]
    -0x1.1e837ap-21, //  -5.3367313e-07 [0xb50f41bd]
    0x1.4a11c4p-26,  //   1.9212568e-08 [0x32a508e2]
    0x1.387092p-28,  //   4.5465964e-09 [0x319c3849]
    -0x1.449c6ap-33, //  -1.4761599e-10 [0xaf224e35]
    -0x1.c50aa6p-36, //  -2.5752433e-11 [0xade28553]
};
/* polynomial minimax approximation Y1(x) = P5(x-z5) on interval (s4,s5). */
static const float __sy1_ep_fZ5_MP[] = {
    0x1.dcb7d8p+3, 0x1.1bd092p-22, // HI + LO:       14.897442 +   2.6432306e-07
                                   // [0x416e5bec + 0x348de849]
};
static const float __sy1_ep_fP5[] = {
    0x1.213202p-53,  //   1.2541849e-16 [0x25109901]
    0x1.a7022cp-3,   //      0.20654711 [0x3e538116]
    -0x1.c650b6p-8,  //    -0.006932301 [0xbbe3285b]
    -0x1.163192p-5,  //    -0.033959184 [0xbd0b18c9]
    0x1.26b046p-10,  //    0.0011241477 [0x3a935823]
    0x1.b175f8p-10,  //    0.0016535218 [0x3ad8bafc]
    -0x1.c0aa2ep-15, //  -5.3485008e-05 [0xb8605517]
    -0x1.3e3796p-15, //  -3.7934438e-05 [0xb81f1bcb]
    0x1.3f3fa8p-20,  //   1.1892939e-06 [0x359f9fd4]
    0x1.0e4578p-21,  //   5.0341964e-07 [0x350722bc]
    -0x1.06481ap-26, //  -1.5266801e-08 [0xb283240d]
    -0x1.1deb74p-28, //  -4.1606798e-09 [0xb18ef5ba]
    0x1.1fa85cp-33,  //   1.3081156e-10 [0x2f0fd42e]
};
/* polynomial minimax approximation Y1(x) = P6(x-z6) on interval (s5,s6). */
static const float __sy1_ep_fZ6_MP[] = {
    0x1.20b1c6p+4, 0x1.2be3c8p-21, // HI + LO:       18.043402 +   5.5858823e-07
                                   // [0x419058e3 + 0x3515f1e4]
};
static const float __sy1_ep_fP6[] = {
    -0x1.39d4c4p-52, //   -2.722051e-16 [0xa59cea62]
    -0x1.80781cp-3,  //     -0.18772909 [0xbe403c0e]
    0x1.54eda6p-8,   //    0.0052021532 [0x3baa76d3]
    0x1.fbe6ep-6,    //     0.030999869 [0x3cfdf370]
    -0x1.be318ep-11, //   -0.0008510467 [0xba5f18c7]
    -0x1.8efee4p-10, //   -0.0015220477 [0xbac77f72]
    0x1.59144ep-15,  //    4.113666e-05 [0x382c8a27]
    0x1.282d1ep-15,  //   3.5306959e-05 [0x3814168f]
    -0x1.f56836p-21, //  -9.3394345e-07 [0xb57ab41b]
    -0x1.fdcc5cp-22, //  -4.7478665e-07 [0xb4fee62e]
    0x1.a3b264p-27,  //   1.2214786e-08 [0x3251d932]
    0x1.1d892ap-28,  //   4.1550927e-09 [0x318ec495]
    -0x1.b2a37p-34,  //  -9.8825337e-11 [0xaed951b8]
    -0x1.ab2b06p-36, //  -2.4281693e-11 [0xadd59583]
};
/* polynomial pade approximation P0(x) = PP(256/x^2) in point 256/x^2 = 0.5 */
static const float __sy1_ep_fPP[] = {
    0x1p+0,          //               1 [0x3f800000]
    0x1.ep-12,       //   0.00045776367 [0x39f00000]
    -0x1.274fbep-19, //  -2.2002421e-06 [0xb613a7df]
    0x1.5a3d1ep-25,  //   4.0307494e-08 [0x332d1e8f]
    -0x1.afbe9cp-30, //  -1.5706776e-09 [0xb0d7df4e]
    0x1.6be2b6p-34,  //   8.2738004e-11 [0x2eb5f15b]
};
static const float __sy1_ep_fPP_MP[] = {
    0x1p+0,
    0x1.138p-43, // HI + LO:               1 +   1.2234658e-13 [0x3f800000 +
                 // 0x2a09c000]
    0x1.dffffep-12,
    0x1.e5e8a2p-36, // HI + LO:   0.00045776364 +   2.7620713e-11 [0x39efffff +
                    // 0x2df2f451]
    -0x1.274fbcp-19,
    -0x1.bfe60ep-43, // HI + LO:  -2.2002419e-06 +  -1.9890696e-13 [0xb613a7de +
                     // 0xaa5ff307]
    0x1.5a3d1ep-25,
    0x1.4dad62p-50, // HI + LO:   4.0307494e-08 +   1.1576756e-15 [0x332d1e8f +
                    // 0x26a6d6b1]
    -0x1.afbe9cp-30,
    -0x1.7a291cp-58, // HI + LO:  -1.5706776e-09 +  -5.1250316e-18 [0xb0d7df4e +
                     // 0xa2bd148e]
    0x1.6be2b6p-34,
    0x1.2b74cep-60, // HI + LO:   8.2738004e-11 +   1.0145973e-18 [0x2eb5f15b +
                    // 0x2195ba67]
};
/* polynomial pade approximation Q0(x) = QP(256/x^2)*(16/x)) in point 256/x^2 =
 * 0.5 */
static const float __sy1_ep_fQP[] = {
    0x1.8p-6,        //       0.0234375 [0x3cc00000]
    -0x1.a4p-16,     //  -2.5033951e-05 [0xb7d20000]
    0x1.1c3c46p-22,  //   2.6471488e-07 [0x348e1e23]
    -0x1.fdd85cp-28, //  -7.4192235e-09 [0xb1feec2e]
    0x1.a76f66p-32,  //   3.8511203e-10 [0x2fd3b7b3]
    -0x1.ab6366p-36, //  -2.4294211e-11 [0xadd5b1b3]
};
static const float __sy1_ep_fQP_MP[] = {
    0x1.7ffffep-6,
    0x1.fffcfcp-30, // HI + LO:     0.023437498 +   1.8626023e-09 [0x3cbfffff +
                    // 0x30fffe7e]
    -0x1.a3fffep-16,
    -0x1.6d894cp-40, // HI + LO:  -2.5033949e-05 +  -1.2986459e-12 [0xb7d1ffff +
                     // 0xabb6c4a6]
    0x1.1c3c44p-22,
    0x1.96a0fep-46, // HI + LO:   2.6471486e-07 +   2.2572437e-14 [0x348e1e22 +
                    // 0x28cb507f]
    -0x1.fdd85cp-28,
    -0x1.47f8dep-55, // HI + LO:  -7.4192235e-09 +   -3.555881e-17 [0xb1feec2e +
                     // 0xa423fc6f]
    0x1.a76f64p-32,
    0x1.8cb7ep-56, // HI + LO:     3.85112e-10 +    2.150614e-17 [0x2fd3b7b2 +
                   // 0x23c65bf0]
    -0x1.ab6366p-36,
    -0x1.769e9ap-61, // HI + LO:  -2.4294211e-11 +  -6.3463018e-19 [0xadd5b1b3 +
                     // 0xa13b4f4d]
};
inline int __devicelib_imf_internal_sy1(const float *a, float *r) {
  int nRet = 0;
  float xf = *a;
  uint32_t ix, iax;
  ix = (*(int *)&xf);
  iax = ix & (~0x80000000);
  if (ix - 0x00000001 < 0x7f800000 - 0x00000001) /* finite positive x */
  {
    if (ix < 0x419d2167) /* 0 < x < 19.64130973 */
    {
      if (ix < 0x418401e4) /* 0 < x < 16.500923 */
      {
        if (ix < 0x4155c70e) /* 0 < x < 13.361097 */
        {
          if (ix < 0x41238eba) /* 0 < x < 10.222345 */
          {
            if (ix < 0x40e2c0ee) /* 0 < x < 7.086050 */
            {
              if (ix < 0x407d4a9a) /* 0 < x < 3.9576783 */
              {
                if (ix < 0x3fe28f5c) /* 0 < x < 1.7699999 */
                {
                  const float ptonpi =
                      0x1.45f306p-1; // 0.63661975 [0x3f22f983] 2/Pi
                  float px = xf, plnx, ps1, ps2, py, pp, pq, ps, presult;
                  __sy1_ep_ln_kernel_fp32(px, &plnx);
                  ps1 = ptonpi * plnx;
                  ps2 = ptonpi / px;
                  py = px * px;
                  pp =
                      px * (__sy1_ep_fQ1[0] +
                            py * (__sy1_ep_fQ1[1] +
                                  py * (__sy1_ep_fQ1[2] +
                                        py * (__sy1_ep_fQ1[3] +
                                              py * (__sy1_ep_fQ1[4] +
                                                    py * (__sy1_ep_fQ1[5]))))));
                  pq =
                      px * (__sy1_ep_fQ2[0] +
                            py * (__sy1_ep_fQ2[1] +
                                  py * (__sy1_ep_fQ2[2] +
                                        py * (__sy1_ep_fQ2[3] +
                                              py * (__sy1_ep_fQ2[4] +
                                                    py * (__sy1_ep_fQ2[5]))))));
                  ps1 = ps1 * pq;
                  ps = ps1 - ps2;
                  presult = pp + ps;
                  *r = (float)presult;
                  return nRet;
                } else /* 1.76999998 <= x < 3.95767831 */
                {
                  float presult, px = xf - __sy1_ep_fZ1_MP[0];
                  px = px - __sy1_ep_fZ1_MP[1];
                  presult =
                      __sy1_ep_fP1[0] +
                      px *
                          (__sy1_ep_fP1[1] +
                           px *
                               (__sy1_ep_fP1[2] +
                                px *
                                    (__sy1_ep_fP1[3] +
                                     px *
                                         (__sy1_ep_fP1[4] +
                                          px *
                                              (__sy1_ep_fP1[5] +
                                               px *
                                                   (__sy1_ep_fP1[6] +
                                                    px *
                                                        (__sy1_ep_fP1[7] +
                                                         px *
                                                             (__sy1_ep_fP1[8] +
                                                              px *
                                                                  (__sy1_ep_fP1
                                                                       [9] +
                                                                   px *
                                                                       (__sy1_ep_fP1
                                                                            [10] +
                                                                        px *
                                                                            (__sy1_ep_fP1
                                                                                 [11] +
                                                                             px *
                                                                                 (__sy1_ep_fP1
                                                                                      [12] +
                                                                                  px *
                                                                                      (__sy1_ep_fP1
                                                                                           [13] +
                                                                                       px *
                                                                                           (__sy1_ep_fP1
                                                                                                [14] +
                                                                                            px *
                                                                                                (__sy1_ep_fP1
                                                                                                     [15])))))))))))))));
                  *r = (float)presult;
                  return nRet;
                }
              } else /* 3.9576783 <= x < 7.0860509 */
              {
                float presult, px = xf - __sy1_ep_fZ2_MP[0];
                px = px - __sy1_ep_fZ2_MP[1];
                presult =
                    __sy1_ep_fP2[0] +
                    px *
                        (__sy1_ep_fP2[1] +
                         px *
                             (__sy1_ep_fP2[2] +
                              px *
                                  (__sy1_ep_fP2[3] +
                                   px *
                                       (__sy1_ep_fP2[4] +
                                        px *
                                            (__sy1_ep_fP2[5] +
                                             px *
                                                 (__sy1_ep_fP2[6] +
                                                  px *
                                                      (__sy1_ep_fP2[7] +
                                                       px *
                                                           (__sy1_ep_fP2[8] +
                                                            px *
                                                                (__sy1_ep_fP2
                                                                     [9] +
                                                                 px *
                                                                     (__sy1_ep_fP2
                                                                          [10] +
                                                                      px *
                                                                          (__sy1_ep_fP2
                                                                               [11] +
                                                                           px *
                                                                               (__sy1_ep_fP2
                                                                                    [12] +
                                                                                px *
                                                                                    (__sy1_ep_fP2
                                                                                         [13])))))))))))));
                *r = (float)presult;
                return nRet;
              }
            } else /* 7.0860509 <= x < 10.222345 */
            {
              float presult, px = xf - __sy1_ep_fZ3_MP[0];
              px = px - __sy1_ep_fZ3_MP[1];
              presult =
                  __sy1_ep_fP3[0] +
                  px *
                      (__sy1_ep_fP3[1] +
                       px *
                           (__sy1_ep_fP3[2] +
                            px *
                                (__sy1_ep_fP3[3] +
                                 px *
                                     (__sy1_ep_fP3[4] +
                                      px *
                                          (__sy1_ep_fP3[5] +
                                           px *
                                               (__sy1_ep_fP3[6] +
                                                px *
                                                    (__sy1_ep_fP3[7] +
                                                     px *
                                                         (__sy1_ep_fP3[8] +
                                                          px *
                                                              (__sy1_ep_fP3[9] +
                                                               px *
                                                                   (__sy1_ep_fP3
                                                                        [10] +
                                                                    px *
                                                                        (__sy1_ep_fP3
                                                                             [11] +
                                                                         px *
                                                                             __sy1_ep_fP3
                                                                                 [12])))))))))));
              *r = (float)presult;
              return nRet;
            }
          } else /* 10.2223453 <= x < 13.36109733 */
          {
            float presult, px = xf - __sy1_ep_fZ4_MP[0];
            px = px - __sy1_ep_fZ4_MP[1];
            presult =
                __sy1_ep_fP4[0] +
                px *
                    (__sy1_ep_fP4[1] +
                     px *
                         (__sy1_ep_fP4[2] +
                          px *
                              (__sy1_ep_fP4[3] +
                               px *
                                   (__sy1_ep_fP4[4] +
                                    px *
                                        (__sy1_ep_fP4[5] +
                                         px *
                                             (__sy1_ep_fP4[6] +
                                              px *
                                                  (__sy1_ep_fP4[7] +
                                                   px *
                                                       (__sy1_ep_fP4[8] +
                                                        px *
                                                            (__sy1_ep_fP4[9] +
                                                             px *
                                                                 (__sy1_ep_fP4
                                                                      [10] +
                                                                  px *
                                                                      (__sy1_ep_fP4
                                                                           [11] +
                                                                       px *
                                                                           (__sy1_ep_fP4
                                                                                [12] +
                                                                            px *
                                                                                (__sy1_ep_fP4
                                                                                     [13])))))))))))));
            *r = (float)presult;
            return nRet;
          }
        } else /* 13.3610973 <= x < 16.500923 */
        {
          float presult, px = xf - __sy1_ep_fZ5_MP[0];
          px = px - __sy1_ep_fZ5_MP[1];
          presult =
              __sy1_ep_fP5[0] +
              px *
                  (__sy1_ep_fP5[1] +
                   px *
                       (__sy1_ep_fP5[2] +
                        px *
                            (__sy1_ep_fP5[3] +
                             px *
                                 (__sy1_ep_fP5[4] +
                                  px *
                                      (__sy1_ep_fP5[5] +
                                       px *
                                           (__sy1_ep_fP5[6] +
                                            px *
                                                (__sy1_ep_fP5[7] +
                                                 px *
                                                     (__sy1_ep_fP5[8] +
                                                      px *
                                                          (__sy1_ep_fP5[9] +
                                                           px *
                                                               (__sy1_ep_fP5
                                                                    [10] +
                                                                px *
                                                                    (__sy1_ep_fP5
                                                                         [11] +
                                                                     px *
                                                                         __sy1_ep_fP5
                                                                             [12])))))))))));
          *r = (float)presult;
          return nRet;
        }
      } else /* 16.500923 <= x < 19.64130973 */
      {
        float presult, px = xf - __sy1_ep_fZ6_MP[0];
        px = px - __sy1_ep_fZ6_MP[1];
        presult =
            __sy1_ep_fP6[0] +
            px *
                (__sy1_ep_fP6[1] +
                 px *
                     (__sy1_ep_fP6[2] +
                      px *
                          (__sy1_ep_fP6[3] +
                           px *
                               (__sy1_ep_fP6[4] +
                                px *
                                    (__sy1_ep_fP6[5] +
                                     px *
                                         (__sy1_ep_fP6[6] +
                                          px *
                                              (__sy1_ep_fP6[7] +
                                               px *
                                                   (__sy1_ep_fP6[8] +
                                                    px *
                                                        (__sy1_ep_fP6[9] +
                                                         px *
                                                             (__sy1_ep_fP6[10] +
                                                              px *
                                                                  (__sy1_ep_fP6
                                                                       [11] +
                                                                   px *
                                                                       (__sy1_ep_fP6
                                                                            [12] +
                                                                        px *
                                                                            (__sy1_ep_fP6
                                                                                 [13])))))))))))));
        *r = (float)presult;
        return nRet;
        return nRet;
      }
    } else /* finite x >= 19.6413097 Hancels asymptotic forms */
    {
      const float ptonpi = 0x1.45f306p-1; // 0.63661975 [0x3f22f983] 2/Pi
      float px = xf, pxi = (1.0f / px), py = (16.0f * pxi), pz = py * py,
            pt = pz * pz, psq = __sqrt(ptonpi * pxi), psn, pcs, pp, pq, presult;
      __sy1_ep_sincos_kernel_fp32(px, -3, &psn, &pcs);
      pp = __sy1_ep_fPP[0] +
           pz * (__sy1_ep_fPP[1] +
                 pz * (__sy1_ep_fPP[2] +
                       pz * (__sy1_ep_fPP[3] +
                             pz * (__sy1_ep_fPP[4] + pz * (__sy1_ep_fPP[5])))));
      pq = py * (__sy1_ep_fQP[0] +
                 pz * (__sy1_ep_fQP[1] +
                       pz * (__sy1_ep_fQP[2] +
                             pz * (__sy1_ep_fQP[3] +
                                   pz * (__sy1_ep_fQP[4] +
                                         pz * (__sy1_ep_fQP[5]))))));
      presult = psq * (pp * psn + pq * pcs);
      *r = (float)presult;
      return nRet;
    }
  } else /* NaN, INF, negative x, zero */
  {
    if (iax > 0x7f800000) /* NaN */
    {
      *r = xf * 1.0f; /* raise invalid on SNaN */
      return nRet;
    } else if (iax == 0) /* +-0 */
    {
      const uint32_t _ones[] = {0x3f800000, 0xbf800000}; /* +1,-1 */
      *r = (((const float *)_ones)[(1)] /
            0.0f); /* raise div-by-zero, return -INF; */
      return nRet;
    } else if (ix & 0x80000000) /* negative x */
    {
      const uint32_t _infs[] = {0x7f800000, 0xff800000}; /* +INF,-INF */
      *r = 0.0f * ((const float *)_infs)[0]; /* raise invalid, return QNaN */
      return nRet;
    } else /* +INF */
    {
      *r = 0.0f;
      return nRet;
    }
  }
}
} /* namespace */
} /* namespace __imf_impl_y1_s_ep */
DEVICE_EXTERN_C_INLINE float __devicelib_imf_y1f(float x) {
  using namespace __imf_impl_y1_s_ep;
  float r;
  __devicelib_imf_internal_sy1(&x, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
