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
namespace __imf_impl_y0_s_ep {
namespace {
static const int32_t __sy0_ep___ip_h = 0x0517CC1B;
static const int32_t __sy0_ep___ip_m = 0x727220A9;
static const int32_t __sy0_ep___ip_l = 0x28;
static const union {
  uint32_t w;
  float f;
} __sy0_ep___cc4 = {0x3e6ce1b2u};
static const union {
  uint32_t w;
  float f;
} __sy0_ep___cc3 = {0xbfaae2beu};
static const union {
  uint32_t w;
  float f;
} __sy0_ep___cc2 = {0x4081e0eeu};
static const union {
  uint32_t w;
  float f;
} __sy0_ep___cc1 = {0xc09de9e6u};
static const union {
  uint32_t w;
  float f;
} __sy0_ep___cc1l = {0xb3e646a5u};
static const union {
  uint32_t w;
  float f;
} __sy0_ep___cc0 = {0x3f800000u};
static const union {
  uint32_t w;
  float f;
} __sy0_ep___cs3 = {0xbf16c981u};
static const union {
  uint32_t w;
  float f;
} __sy0_ep___cs2 = {0x40232f49u};
static const union {
  uint32_t w;
  float f;
} __sy0_ep___cs1 = {0xc0a55dddu};
static const union {
  uint32_t w;
  float f;
} __sy0_ep___cs0 = {0x40490fdbu};
static const union {
  uint32_t w;
  float f;
} __sy0_ep___cs0l = {0xb3d195e9u};
static const uint32_t __sy0_ep_invpi_tbl[] = {
    0,          0x28BE60DB, 0x9391054A, 0x7F09D5F4,
    0x7D4D3770, 0x36D8A566, 0x4F10E410, 0x7F9458EA};
static int __sy0_ep_sincos_kernel_fp32(float xf, int n, float *psin,
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
    ip_low = (((uint32_t)__sy0_ep_invpi_tbl[index]) * ((uint32_t)mx));
    IP = (((uint64_t)((uint32_t)(__sy0_ep_invpi_tbl[index + 1]))) *
          ((uint32_t)(mx))) +
         (((uint64_t)ip_low) << 32);
    // scaled by 2^(95-j)
    IP2 = (((uint64_t)((uint32_t)(__sy0_ep_invpi_tbl[index + 2]))) *
           ((uint32_t)(mx))) +
          ((((uint64_t)((uint32_t)(__sy0_ep_invpi_tbl[index + 3]))) *
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
    IP_s = (((int64_t)((int32_t)(mx))) * ((int32_t)(__sy0_ep___ip_h)));
    IP = (uint64_t)IP_s;
    IP2_s = (((int64_t)((int32_t)(mx))) * ((int32_t)(__sy0_ep___ip_m)));
    IP2 = (uint64_t)IP2_s;
    // scale (23-ex)*2^(28+32+7)
    ip_low_s = (((int32_t)mx) * ((int32_t)__sy0_ep___ip_l));
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
  cpoly.f = __fma(__sy0_ep___cc4.f, R2h, __sy0_ep___cc3.f);
  spoly.f = __fma(__sy0_ep___cs3.f, R2h, __sy0_ep___cs2.f);
  cpoly.f = __fma(cpoly.f, R2h, __sy0_ep___cc2.f);
  spoly.f = __fma(spoly.f, R2h, __sy0_ep___cs1.f);
  cpoly.f = __fma(cpoly.f, R2h, __sy0_ep___cc1.f);
  spoly.f = __fma(spoly.f, R2h, __sy0_ep___cs0.f);
  cpoly.f = __fma(cpoly.f, R2h, __sy0_ep___cc0.f);
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
static int __sy0_ep_ln_kernel_fp32(float x, float *r) {
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
/* polynomial minimax approximation Y0(x) = Q1(x^2)+2/pi*j0(x)*log(x) on
 * interval (0.0,0.6). */
static const float __sy0_ep_fQ1[] = {
    -0x1.2e4d6ap-4,  //    -0.073804297 [0xbd9726b5]
    0x1.6bbcb4p-3,   //      0.17760602 [0x3e35de5a]
    -0x1.075b18p-6,  //    -0.016073965 [0xbc83ad8c]
    0x1.1a5e14p-11,  //   0.00053857325 [0x3a0d2f0a]
    -0x1.3b7a08p-17, //  -9.4019379e-06 [0xb71dbd04]
};
/* polynomial minimax approximation of J0(x) = 1+y*Q2(y), y=x^2, |x|=[0..0.6],
 * max.err .88e-11 */
static const float __sy0_ep_fQ2[] = {
    -0x1p-2,         //           -0.25 [0xbe800000]
    0x1.fffffep-7,   //     0.015624999 [0x3c7fffff]
    -0x1.c71982p-12, //  -0.00043401684 [0xb9e38cc1]
    0x1.c3d898p-18,  //   6.7330311e-06 [0x36e1ec4c]
};
/* polynomial minimax approximation Y0(x) = P1(x-z1) on interval (0.6,s1). */
static const float __sy0_ep_fZ1_MP[] = {
    0x1.c982eap-1, 0x1.8d417ep-25, // HI + LO:      0.89357692 +   4.6246665e-08
                                   // [0x3f64c175 + 0x3346a0bf]
};
static const float __sy0_ep_fP1_MP[] = {
    -0x1.af74bep-56,
    -0x1.a0f13p-80, // HI + LO:  -2.3389278e-17 +  -1.3472127e-24 [0xa3d7ba5f +
                    // 0x97d07898]
    0x1.c2437p-1,
    0x1.8421e4p-25, // HI + LO:      0.87942076 +   4.5184557e-08 [0x3f6121b8 +
                    // 0x334210f2]
    -0x1.f7e38ap-2,
    -0x1.5e717cp-28, // HI + LO:     -0.49207893 +  -5.0996212e-09 [0xbefbf1c5 +
                     // 0xb1af38be]
    0x1.c3b134p-3,
    0x1.cb4394p-30, // HI + LO:      0.22055283 +   1.6707926e-09 [0x3e61d89a +
                    // 0x30e5a1ca]
    -0x1.cf18dap-3,
    -0x1.01f1a8p-27, // HI + LO:     -0.22612162 +  -7.5071576e-09 [0xbe678c6d +
                     // 0xb200f8d4]
    0x1.c067cap-3,
    0x1.2d87fcp-29, // HI + LO:      0.21894796 +   2.1939282e-09 [0x3e6033e5 +
                    // 0x3116c3fe]
    -0x1.a397e2p-3,
    -0x1.035c76p-27, // HI + LO:     -0.20487954 +  -7.5484037e-09 [0xbe51cbf1 +
                     // 0xb201ae3b]
    0x1.942de4p-3,
    0x1.d9866p-27, // HI + LO:      0.19735315 +   1.3781388e-08 [0x3e4a16f2 +
                   // 0x326cc330]
    -0x1.8d324cp-3,
    -0x1.1d89aap-29, // HI + LO:     -0.19394359 +  -2.0775606e-09 [0xbe469926 +
                     // 0xb10ec4d5]
    0x1.8b78ep-3,
    0x1.90dbdep-27, // HI + LO:      0.19310164 +   1.1666528e-08 [0x3e45bc70 +
                    // 0x32486def]
    -0x1.8e4fa2p-3,
    -0x1.c35a84p-28, // HI + LO:     -0.19448783 +   -6.568059e-09 [0xbe4727d1 +
                     // 0xb1e1ad42]
    0x1.99180ep-3,
    0x1.1cbb4cp-27, // HI + LO:      0.19975291 +    8.286781e-09 [0x3e4c8c07 +
                    // 0x320e5da6]
    -0x1.afbbaap-3,
    -0x1.fe4a22p-27, // HI + LO:     -0.21080716 +  -1.4851381e-08 [0xbe57ddd5 +
                     // 0xb27f2511]
    0x1.c5ed8ep-3,
    0x1.a7f108p-31, // HI + LO:      0.22164451 +   7.7114515e-10 [0x3e62f6c7 +
                    // 0x3053f884]
    -0x1.b6cbc2p-3,
    -0x1.210156p-30, // HI + LO:     -0.21425582 +  -1.0513949e-09 [0xbe5b65e1 +
                     // 0xb09080ab]
    0x1.6434cep-3,
    0x1.df2d3ap-27, // HI + LO:      0.17392884 +   1.3945876e-08 [0x3e321a67 +
                    // 0x326f969d]
    -0x1.c3594ap-4,
    -0x1.e466cap-29, // HI + LO:     -0.11019257 +  -3.5244849e-09 [0xbde1aca5 +
                     // 0xb1723365]
    0x1.a3a98cp-5,
    0x1.2ba6c6p-30, // HI + LO:     0.051228307 +   1.0901257e-09 [0x3d51d4c6 +
                    // 0x3095d363]
    -0x1.0ab356p-6,
    -0x1.22eb9p-30, // HI + LO:    -0.016278109 +  -1.0583614e-09 [0xbc8559ab +
                    // 0xb09175c8]
    0x1.9c12ecp-9,
    0x1.873c04p-34, // HI + LO:    0.0031438745 +   8.8956412e-11 [0x3b4e0976 +
                    // 0x2ec39e02]
    -0x1.23377p-12,
    -0x1.4cee56p-36, // HI + LO:  -0.00027772575 +  -1.8924936e-11 [0xb9919bb8 +
                     // 0xada6772b]
};
static const float __sy0_ep_fP1[] = {
    -0x1.af74cp-56,  //   -2.338928e-17 [0xa3d7ba60]
    0x1.c24372p-1,   //      0.87942082 [0x3f6121b9]
    -0x1.f7e38ap-2,  //     -0.49207893 [0xbefbf1c5]
    0x1.c3b134p-3,   //      0.22055283 [0x3e61d89a]
    -0x1.cf18dcp-3,  //     -0.22612163 [0xbe678c6e]
    0x1.c067cap-3,   //      0.21894796 [0x3e6033e5]
    -0x1.a397e4p-3,  //     -0.20487955 [0xbe51cbf2]
    0x1.942de6p-3,   //      0.19735317 [0x3e4a16f3]
    -0x1.8d324cp-3,  //     -0.19394359 [0xbe469926]
    0x1.8b78e2p-3,   //      0.19310166 [0x3e45bc71]
    -0x1.8e4fa2p-3,  //     -0.19448783 [0xbe4727d1]
    0x1.99181p-3,    //      0.19975293 [0x3e4c8c08]
    -0x1.afbbacp-3,  //     -0.21080717 [0xbe57ddd6]
    0x1.c5ed8ep-3,   //      0.22164451 [0x3e62f6c7]
    -0x1.b6cbc2p-3,  //     -0.21425582 [0xbe5b65e1]
    0x1.6434dp-3,    //      0.17392886 [0x3e321a68]
    -0x1.c3594ap-4,  //     -0.11019257 [0xbde1aca5]
    0x1.a3a98cp-5,   //     0.051228307 [0x3d51d4c6]
    -0x1.0ab358p-6,  //     -0.01627811 [0xbc8559ac]
    0x1.9c12ecp-9,   //    0.0031438745 [0x3b4e0976]
    -0x1.233772p-12, //  -0.00027772578 [0xb9919bb9]
};
/* polynomial minimax approximation Y0(x) = P2(x-z2)/Q2(x-z2) on interval
 * (s1,s2). */
static const float __sy0_ep_fZ2_MP[] = {
    0x1.fa9534p+1, 0x1.b30ad4p-24, // HI + LO:       3.9576783 +   1.0129118e-07
                                   // [0x407d4a9a + 0x33d9856a]
};
static const float __sy0_ep_fP2[] = {
    -0x1.8fa896p-55, //  -4.3331066e-17 [0xa447d44b]
    -0x1.9c3426p-2,  //     -0.40254268 [0xbece1a13]
    0x1.a09c92p-5,   //     0.050855909 [0x3d504e49]
    0x1.df6d5ap-5,   //     0.058523823 [0x3d6fb6ad]
    -0x1.c116fep-8,  //   -0.0068525667 [0xbbe08b7f]
    -0x1.1e32bap-9,  //   -0.0021835186 [0xbb0f195d]
    0x1.998266p-13,  //   0.00019526928 [0x394cc133]
    0x1.ab2b2cp-15,  //    5.092247e-05 [0x38559596]
    -0x1.486008p-18, //  -4.8931706e-06 [0xb6a43004]
    -0x1.3a6d62p-22, //  -2.9283322e-07 [0xb49d36b1]
    -0x1.7a5c5p-26,  //  -2.2023485e-08 [0xb2bd2e28]
    0x1.3b9fcep-26,  //   1.8371749e-08 [0x329dcfe7]
    -0x1.ed292ap-29, //  -3.5882171e-09 [0xb1769495]
    0x1.1e2174p-30,  //   1.0409373e-09 [0x308f10ba]
    -0x1.a70ee4p-33, //  -1.9238458e-10 [0xaf538772]
    -0x1.00258ep-35, //  -2.9120508e-11 [0xae0012c7]
    -0x1.eb0008p-37, //  -1.3955063e-11 [0xad758004]
    0x1.90e974p-37,  //   1.1394602e-11 [0x2d4874ba]
};
/* polynomial minimax approximation Y0(x) = P3(x-z3) on interval (s2,s3). */
static const float __sy0_ep_fZ3_MP[] = {
    0x1.c581dcp+2, 0x1.39c84p-24, // HI + LO:        7.086051 +   7.3058118e-08
                                  // [0x40e2c0ee + 0x339ce420]
};
static const float __sy0_ep_fP3[] = {
    0x1.e91b1ap-56,  //   2.6514482e-17 [0x23f48d8d]
    0x1.334ccap-2,   //      0.30009761 [0x3e99a665]
    -0x1.5aef62p-6,  //    -0.021175237 [0xbcad77b1]
    -0x1.8969c6p-5,  //     -0.04802407 [0xbd44b4e3]
    0x1.b2f14ap-9,   //    0.0033183482 [0x3b5978a5]
    0x1.1d35e8p-9,   //     0.002175984 [0x3b0e9af4]
    -0x1.26dd6cp-13, //  -0.00014060255 [0xb9136eb6]
    -0x1.8177f2p-15, //   -4.595143e-05 [0xb840bbf9]
    0x1.6a903p-19,   //   2.7013066e-06 [0x36354818]
    0x1.34acb6p-21,  //   5.7495134e-07 [0x351a565b]
    -0x1.09db12p-25, //  -3.0949682e-08 [0xb304ed89]
    -0x1.447abcp-28, //  -4.7217972e-09 [0xb1a23d5e]
    0x1.e2d8dep-33,  //   2.1957346e-10 [0x2f716c6f]
    0x1.f3106ap-36,  //   2.8368511e-11 [0x2df98835]
};
/* polynomial minimax approximation Y0(x) = P4(x-z4) on interval (s3,s4). */
static const float __sy0_ep_fZ4_MP[] = {
    0x1.471d72p+3, 0x1.5a47d6p-21, // HI + LO:       10.222344 +    6.449979e-07
                                   // [0x41238eb9 + 0x352d23eb]
};
static const float __sy0_ep_fP4[] = {
    -0x1.cabd7cp-53, //  -1.9894684e-16 [0xa5655ebe]
    -0x1.ff635cp-3,  //     -0.24970123 [0xbe7fb1ae]
    0x1.903646p-7,   //     0.012213501 [0x3c481b23]
    0x1.4e667ap-5,   //     0.040820349 [0x3d27333d]
    -0x1.0325eep-9,  //   -0.0019771436 [0xbb0192f7]
    -0x1.fe2392p-10, //   -0.0019460256 [0xbaff11c9]
    0x1.7f84ccp-14,  //   9.1437993e-05 [0x38bfc266]
    0x1.6afddap-15,  //   4.3271972e-05 [0x38357eed]
    -0x1.040398p-19, //  -1.9372555e-06 [0xb60201cc]
    -0x1.2aeb2ap-21, //   -5.567793e-07 [0xb5157595]
    0x1.936d56p-26,  //   2.3482547e-08 [0x32c9b6ab]
    0x1.428b84p-28,  //   4.6936472e-09 [0x31a145c2]
    -0x1.86ccap-33,  //  -1.7771495e-10 [0xaf436650]
    -0x1.e2e486p-36, //   -2.744927e-11 [0xadf17243]
};
/* polynomial minimax approximation Y0(x) = P5(x-z5) on interval (s4,s5). */
static const float __sy0_ep_fZ5_MP[] = {
    0x1.ab8e1cp+3, 0x1.2879d2p-23, // HI + LO:       13.361097 +   1.3805733e-07
                                   // [0x4155c70e + 0x34143ce9]
};
static const float __sy0_ep_fP5[] = {
    0x1.4d9fe4p-53,  //   1.4468659e-16 [0x2526cff2]
    0x1.bf32a2p-3,   //      0.21835829 [0x3e5f9951]
    -0x1.0bc2d8p-7,  //   -0.0081714205 [0xbc05e16c]
    -0x1.26cab4p-5,  //    -0.035985328 [0xbd13655a]
    0x1.5f03e2p-10,  //    0.0013390166 [0x3aaf81f1]
    0x1.caaa6cp-10,  //    0.0017496708 [0x3ae55536]
    -0x1.0c5ec6p-14, //  -6.3984444e-05 [0xb8862f63]
    -0x1.4f08eep-15, //   -3.993927e-05 [0xb8278477]
    0x1.7d0b6p-20,   //   1.4195011e-06 [0x35be85b0]
    0x1.1a779p-21,   //   5.2613586e-07 [0x350d3bc8]
    -0x1.340368p-26, //  -1.7928734e-08 [0xb29a01b4]
    -0x1.2879dep-28, //  -4.3142943e-09 [0xb1943cef]
    0x1.19366cp-33,  //   1.2788068e-10 [0x2f0c9b36]
};
/* polynomial minimax approximation Y0(x) = P6(x-z6) on interval (s5,s6). */
static const float __sy0_ep_fZ6_MP[] = {
    0x1.0803c6p+4, 0x1.400322p-20, // HI + LO:       16.500921 +   1.1921385e-06
                                   // [0x418401e3 + 0x35a00191]
};
static const float __sy0_ep_fP6[] = {
    0x1.cd828p-53,   //   2.0014796e-16 [0x2566c140]
    -0x1.925c36p-3,  //     -0.19646494 [0xbe492e1b]
    0x1.86254ap-8,   //    0.0059531503 [0x3bc312a5]
    0x1.0a4512p-5,   //     0.032503638 [0x3d052289]
    -0x1.013b36p-10, //  -0.00098125951 [0xba809d9b]
    -0x1.a24a06p-10, //   -0.0015956465 [0xbad12503]
    0x1.8f90cp-15,   //   4.7631911e-05 [0x3847c860]
    0x1.35cf82p-15,  //   3.6932299e-05 [0x381ae7c1]
    -0x1.2274ecp-20, //  -1.0820356e-06 [0xb5913a76]
    -0x1.09482ap-21, //  -4.9412603e-07 [0xb504a415]
    0x1.e2b94p-27,   //   1.4049107e-08 [0x32715ca0]
    0x1.1a5b64p-28,  //    4.108835e-09 [0x318d2db2]
    -0x1.c5f2cap-34, //  -1.0321591e-10 [0xaee2f965]
};
/* polynomial pade approximation P0(x) = PP(256/x^2) in point 256/x^2 = 0.5 */
static const float __sy0_ep_fPP[] = {
    0x1p+0,          //               1 [0x3f800000]
    -0x1.2p-12,      //   -0.0002746582 [0xb9900000]
    0x1.cb5f86p-20,  //   1.7112983e-06 [0x35e5afc3]
    -0x1.24f578p-25, //    -3.41049e-08 [0xb3127abc]
    0x1.7ca5eep-30,  //   1.3847899e-09 [0x30be52f7]
    -0x1.47a91p-34,  //   -7.450135e-11 [0xaea3d488]
};
/* polynomial pade approximation Q0(x) = QP(256/x^2)*(16/x)) in point 256/x^2 =
 * 0.5 */
static const float __sy0_ep_fQP[] = {
    -0x1p-7,         //      -0.0078125 [0xbc000000]
    0x1.2cp-16,      //   1.7881393e-05 [0x37960000]
    -0x1.d11ca8p-23, //  -2.1658462e-07 [0xb4688e54]
    0x1.b9d68ep-28,  //   6.4295906e-09 [0x31dceb47]
    -0x1.7a8362p-32, //  -3.4425576e-10 [0xafbd41b1]
    0x1.845fecp-36,  //   2.2076545e-11 [0x2dc22ff6]
};
inline int __devicelib_imf_internal_sy0(const float *a, float *r) {
  int nRet = 0;
  float xf = *a;
  uint32_t ix, iax;
  ix = (*(int *)&xf);
  iax = ix & (~0x80000000);
  if ((ix - 0x00000001) < 0x7f800000 - 0x00000001) /* finite positive x */
  {
    if (ix < 0x419058e3) /* 0 < x < 18.04340 */
    {
      if (ix < 0x413bfc8a) /* 0 < x < 11.74915 */
      {
        if (ix < 0x40adbff2) /* 0 < x < 5.4296808 */
        {
          if (ix < 0x400c9df7) /* 0 < x < 2.197141 */
          {
            if (ix < 0x3f19999a) /* 0 < x < .6 */
            {
              const float ptonpi =
                  0x1.45f306p-1; // 0.63661975 [0x3f22f983] 2/Pi
              float px = xf, plnx, ps, py, pp, pq, presult;
              __sy0_ep_ln_kernel_fp32(px, &plnx);
              ps = ptonpi * plnx;
              py = px * px;
              pp =
                  __sy0_ep_fQ1[0] +
                  py * (__sy0_ep_fQ1[1] +
                        py * (__sy0_ep_fQ1[2] +
                              py * (__sy0_ep_fQ1[3] + py * (__sy0_ep_fQ1[4]))));
              pq = ps *
                   (py *
                    (__sy0_ep_fQ2[0] +
                     py * (__sy0_ep_fQ2[1] +
                           py * (__sy0_ep_fQ2[2] + py * (__sy0_ep_fQ2[3])))));
              presult = pp + pq;
              presult += ps;
              *r = (float)presult;
              return nRet;
            } else /* .6 <= x < 2.197141408 */
            {
              float px, xh, xl, pl, ph, presult;
              xh = -__sy0_ep_fZ1_MP[0];
              xl = -__sy0_ep_fZ1_MP[1];
              {
                float __ph, __ahl, __ahh;
                __ph = __fma(xh, 1.0f, xf);
                __ahh = __fma(__ph, 1.0f, -xf);
                __ahl = __fma(xh, 1.0f, -__ahh);
                xl = xl + __ahl;
                xh = __ph;
              };
              px = xh + xl;
              ph = __sy0_ep_fP1[18] +
                   px * (__sy0_ep_fP1[19] + px * (__sy0_ep_fP1[20]));
              pl = 0.0f;
              {
                float __ph, __phl;
                __ph = __fma(ph, xh, 0.0f);
                __phl = __fma(ph, xh, -__ph);
                pl = __fma(pl, xh, __phl);
                pl = __fma(ph, xl, pl);
                ph = __ph;
              };
              {
                float __ph, __ahl, __ahh;
                float __ah, __bh;
                __bh = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[17 * 2]))
                           ? (__sy0_ep_fP1_MP[17 * 2])
                           : (ph);
                __ah = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[17 * 2]))
                           ? (ph)
                           : (__sy0_ep_fP1_MP[17 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __sy0_ep_fP1_MP[17 * 2 + 1]) + __ahl;
                ph = __ph;
              };
              {
                float __ph, __phl;
                __ph = __fma(ph, xh, 0.0f);
                __phl = __fma(ph, xh, -__ph);
                pl = __fma(pl, xh, __phl);
                pl = __fma(ph, xl, pl);
                ph = __ph;
              };
              {
                float __ph, __ahl, __ahh;
                float __ah, __bh;
                __bh = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[16 * 2]))
                           ? (__sy0_ep_fP1_MP[16 * 2])
                           : (ph);
                __ah = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[16 * 2]))
                           ? (ph)
                           : (__sy0_ep_fP1_MP[16 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __sy0_ep_fP1_MP[16 * 2 + 1]) + __ahl;
                ph = __ph;
              };
              {
                float __ph, __phl;
                __ph = __fma(ph, xh, 0.0f);
                __phl = __fma(ph, xh, -__ph);
                pl = __fma(pl, xh, __phl);
                pl = __fma(ph, xl, pl);
                ph = __ph;
              };
              {
                float __ph, __ahl, __ahh;
                float __ah, __bh;
                __bh = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[15 * 2]))
                           ? (__sy0_ep_fP1_MP[15 * 2])
                           : (ph);
                __ah = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[15 * 2]))
                           ? (ph)
                           : (__sy0_ep_fP1_MP[15 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __sy0_ep_fP1_MP[15 * 2 + 1]) + __ahl;
                ph = __ph;
              };
              {
                float __ph, __phl;
                __ph = __fma(ph, xh, 0.0f);
                __phl = __fma(ph, xh, -__ph);
                pl = __fma(pl, xh, __phl);
                pl = __fma(ph, xl, pl);
                ph = __ph;
              };
              {
                float __ph, __ahl, __ahh;
                float __ah, __bh;
                __bh = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[14 * 2]))
                           ? (__sy0_ep_fP1_MP[14 * 2])
                           : (ph);
                __ah = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[14 * 2]))
                           ? (ph)
                           : (__sy0_ep_fP1_MP[14 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __sy0_ep_fP1_MP[14 * 2 + 1]) + __ahl;
                ph = __ph;
              };
              {
                float __ph, __phl;
                __ph = __fma(ph, xh, 0.0f);
                __phl = __fma(ph, xh, -__ph);
                pl = __fma(pl, xh, __phl);
                pl = __fma(ph, xl, pl);
                ph = __ph;
              };
              {
                float __ph, __ahl, __ahh;
                float __ah, __bh;
                __bh = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[13 * 2]))
                           ? (__sy0_ep_fP1_MP[13 * 2])
                           : (ph);
                __ah = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[13 * 2]))
                           ? (ph)
                           : (__sy0_ep_fP1_MP[13 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __sy0_ep_fP1_MP[13 * 2 + 1]) + __ahl;
                ph = __ph;
              };
              {
                float __ph, __phl;
                __ph = __fma(ph, xh, 0.0f);
                __phl = __fma(ph, xh, -__ph);
                pl = __fma(pl, xh, __phl);
                pl = __fma(ph, xl, pl);
                ph = __ph;
              };
              {
                float __ph, __ahl, __ahh;
                float __ah, __bh;
                __bh = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[12 * 2]))
                           ? (__sy0_ep_fP1_MP[12 * 2])
                           : (ph);
                __ah = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[12 * 2]))
                           ? (ph)
                           : (__sy0_ep_fP1_MP[12 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __sy0_ep_fP1_MP[12 * 2 + 1]) + __ahl;
                ph = __ph;
              };
              {
                float __ph, __phl;
                __ph = __fma(ph, xh, 0.0f);
                __phl = __fma(ph, xh, -__ph);
                pl = __fma(pl, xh, __phl);
                pl = __fma(ph, xl, pl);
                ph = __ph;
              };
              {
                float __ph, __ahl, __ahh;
                float __ah, __bh;
                __bh = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[11 * 2]))
                           ? (__sy0_ep_fP1_MP[11 * 2])
                           : (ph);
                __ah = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[11 * 2]))
                           ? (ph)
                           : (__sy0_ep_fP1_MP[11 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __sy0_ep_fP1_MP[11 * 2 + 1]) + __ahl;
                ph = __ph;
              };
              {
                float __ph, __phl;
                __ph = __fma(ph, xh, 0.0f);
                __phl = __fma(ph, xh, -__ph);
                pl = __fma(pl, xh, __phl);
                pl = __fma(ph, xl, pl);
                ph = __ph;
              };
              {
                float __ph, __ahl, __ahh;
                float __ah, __bh;
                __bh = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[10 * 2]))
                           ? (__sy0_ep_fP1_MP[10 * 2])
                           : (ph);
                __ah = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[10 * 2]))
                           ? (ph)
                           : (__sy0_ep_fP1_MP[10 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __sy0_ep_fP1_MP[10 * 2 + 1]) + __ahl;
                ph = __ph;
              };
              {
                float __ph, __phl;
                __ph = __fma(ph, xh, 0.0f);
                __phl = __fma(ph, xh, -__ph);
                pl = __fma(pl, xh, __phl);
                pl = __fma(ph, xl, pl);
                ph = __ph;
              };
              {
                float __ph, __ahl, __ahh;
                float __ah, __bh;
                __bh = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[9 * 2]))
                           ? (__sy0_ep_fP1_MP[9 * 2])
                           : (ph);
                __ah = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[9 * 2]))
                           ? (ph)
                           : (__sy0_ep_fP1_MP[9 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __sy0_ep_fP1_MP[9 * 2 + 1]) + __ahl;
                ph = __ph;
              };
              {
                float __ph, __phl;
                __ph = __fma(ph, xh, 0.0f);
                __phl = __fma(ph, xh, -__ph);
                pl = __fma(pl, xh, __phl);
                pl = __fma(ph, xl, pl);
                ph = __ph;
              };
              {
                float __ph, __ahl, __ahh;
                float __ah, __bh;
                __bh = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[8 * 2]))
                           ? (__sy0_ep_fP1_MP[8 * 2])
                           : (ph);
                __ah = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[8 * 2]))
                           ? (ph)
                           : (__sy0_ep_fP1_MP[8 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __sy0_ep_fP1_MP[8 * 2 + 1]) + __ahl;
                ph = __ph;
              };
              {
                float __ph, __phl;
                __ph = __fma(ph, xh, 0.0f);
                __phl = __fma(ph, xh, -__ph);
                pl = __fma(pl, xh, __phl);
                pl = __fma(ph, xl, pl);
                ph = __ph;
              };
              {
                float __ph, __ahl, __ahh;
                float __ah, __bh;
                __bh = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[7 * 2]))
                           ? (__sy0_ep_fP1_MP[7 * 2])
                           : (ph);
                __ah = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[7 * 2]))
                           ? (ph)
                           : (__sy0_ep_fP1_MP[7 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __sy0_ep_fP1_MP[7 * 2 + 1]) + __ahl;
                ph = __ph;
              };
              {
                float __ph, __phl;
                __ph = __fma(ph, xh, 0.0f);
                __phl = __fma(ph, xh, -__ph);
                pl = __fma(pl, xh, __phl);
                pl = __fma(ph, xl, pl);
                ph = __ph;
              };
              {
                float __ph, __ahl, __ahh;
                float __ah, __bh;
                __bh = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[6 * 2]))
                           ? (__sy0_ep_fP1_MP[6 * 2])
                           : (ph);
                __ah = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[6 * 2]))
                           ? (ph)
                           : (__sy0_ep_fP1_MP[6 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __sy0_ep_fP1_MP[6 * 2 + 1]) + __ahl;
                ph = __ph;
              };
              {
                float __ph, __phl;
                __ph = __fma(ph, xh, 0.0f);
                __phl = __fma(ph, xh, -__ph);
                pl = __fma(pl, xh, __phl);
                pl = __fma(ph, xl, pl);
                ph = __ph;
              };
              {
                float __ph, __ahl, __ahh;
                float __ah, __bh;
                __bh = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[5 * 2]))
                           ? (__sy0_ep_fP1_MP[5 * 2])
                           : (ph);
                __ah = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[5 * 2]))
                           ? (ph)
                           : (__sy0_ep_fP1_MP[5 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __sy0_ep_fP1_MP[5 * 2 + 1]) + __ahl;
                ph = __ph;
              };
              {
                float __ph, __phl;
                __ph = __fma(ph, xh, 0.0f);
                __phl = __fma(ph, xh, -__ph);
                pl = __fma(pl, xh, __phl);
                pl = __fma(ph, xl, pl);
                ph = __ph;
              };
              {
                float __ph, __ahl, __ahh;
                float __ah, __bh;
                __bh = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[4 * 2]))
                           ? (__sy0_ep_fP1_MP[4 * 2])
                           : (ph);
                __ah = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[4 * 2]))
                           ? (ph)
                           : (__sy0_ep_fP1_MP[4 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __sy0_ep_fP1_MP[4 * 2 + 1]) + __ahl;
                ph = __ph;
              };
              {
                float __ph, __phl;
                __ph = __fma(ph, xh, 0.0f);
                __phl = __fma(ph, xh, -__ph);
                pl = __fma(pl, xh, __phl);
                pl = __fma(ph, xl, pl);
                ph = __ph;
              };
              {
                float __ph, __ahl, __ahh;
                float __ah, __bh;
                __bh = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[3 * 2]))
                           ? (__sy0_ep_fP1_MP[3 * 2])
                           : (ph);
                __ah = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[3 * 2]))
                           ? (ph)
                           : (__sy0_ep_fP1_MP[3 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __sy0_ep_fP1_MP[3 * 2 + 1]) + __ahl;
                ph = __ph;
              };
              {
                float __ph, __phl;
                __ph = __fma(ph, xh, 0.0f);
                __phl = __fma(ph, xh, -__ph);
                pl = __fma(pl, xh, __phl);
                pl = __fma(ph, xl, pl);
                ph = __ph;
              };
              {
                float __ph, __ahl, __ahh;
                float __ah, __bh;
                __bh = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[2 * 2]))
                           ? (__sy0_ep_fP1_MP[2 * 2])
                           : (ph);
                __ah = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[2 * 2]))
                           ? (ph)
                           : (__sy0_ep_fP1_MP[2 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __sy0_ep_fP1_MP[2 * 2 + 1]) + __ahl;
                ph = __ph;
              };
              {
                float __ph, __phl;
                __ph = __fma(ph, xh, 0.0f);
                __phl = __fma(ph, xh, -__ph);
                pl = __fma(pl, xh, __phl);
                pl = __fma(ph, xl, pl);
                ph = __ph;
              };
              {
                float __ph, __ahl, __ahh;
                float __ah, __bh;
                __bh = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[1 * 2]))
                           ? (__sy0_ep_fP1_MP[1 * 2])
                           : (ph);
                __ah = (__fabs(ph) <= __fabs(__sy0_ep_fP1_MP[1 * 2]))
                           ? (ph)
                           : (__sy0_ep_fP1_MP[1 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __sy0_ep_fP1_MP[1 * 2 + 1]) + __ahl;
                ph = __ph;
              };
              {
                float __ph, __phl;
                __ph = __fma(ph, xh, 0.0f);
                __phl = __fma(ph, xh, -__ph);
                pl = __fma(pl, xh, __phl);
                pl = __fma(ph, xl, pl);
                ph = __ph;
              };
              presult = ph + pl + __sy0_ep_fP1[0];
              *r = (float)presult;
              return nRet;
            }
          } else /* 2.1971414 <= x < 5.4296808 */
          {
            float px, presult;
            px = xf - __sy0_ep_fZ2_MP[0];
            px = px - __sy0_ep_fZ2_MP[1];
            presult =
                __sy0_ep_fP2[0] +
                px *
                    (__sy0_ep_fP2[1] +
                     px *
                         (__sy0_ep_fP2[2] +
                          px *
                              (__sy0_ep_fP2[3] +
                               px *
                                   (__sy0_ep_fP2[4] +
                                    px *
                                        (__sy0_ep_fP2[5] +
                                         px *
                                             (__sy0_ep_fP2[6] +
                                              px *
                                                  (__sy0_ep_fP2[7] +
                                                   px *
                                                       (__sy0_ep_fP2[8] +
                                                        px *
                                                            (__sy0_ep_fP2[9] +
                                                             px *
                                                                 (__sy0_ep_fP2
                                                                      [10] +
                                                                  px *
                                                                      (__sy0_ep_fP2
                                                                           [11] +
                                                                       px *
                                                                           (__sy0_ep_fP2
                                                                                [12] +
                                                                            px *
                                                                                (__sy0_ep_fP2
                                                                                     [13] +
                                                                                 px *
                                                                                     (__sy0_ep_fP2
                                                                                          [14] +
                                                                                      px *
                                                                                          (__sy0_ep_fP2
                                                                                               [15] +
                                                                                           px *
                                                                                               (__sy0_ep_fP2
                                                                                                    [16] +
                                                                                                px *
                                                                                                    (__sy0_ep_fP2
                                                                                                         [17])))))))))))))))));
            *r = (float)presult;
            return nRet;
          }
        } else /* 5.429680 <= x < 11.749155 */
        {
          const float *P;
          const float *Z;
          float px, presult;
          if (ix < 0x4109893d) /* 8.59600543 */
          {
            Z = __sy0_ep_fZ3_MP;
            P = __sy0_ep_fP3;
          } else {
            Z = __sy0_ep_fZ4_MP;
            P = __sy0_ep_fP4;
          }
          px = xf - Z[0];
          px = px - Z[1];
          presult =
              P[0] +
              px *
                  (P[1] +
                   px *
                       (P[2] +
                        px *
                            (P[3] +
                             px *
                                 (P[4] +
                                  px *
                                      (P[5] +
                                       px *
                                           (P[6] +
                                            px *
                                                (P[7] +
                                                 px *
                                                     (P[8] +
                                                      px *
                                                          (P[9] +
                                                           px *
                                                               (P[10] +
                                                                px *
                                                                    (P[11] +
                                                                     px *
                                                                         (P[12] +
                                                                          px *
                                                                              (P[13])))))))))))));
          *r = (float)presult;
          return nRet;
        }
      } else /* 11.7491550 <= x < 18.043401 */
      {
        const float *P;
        const float *Z;
        float px, presult;
        if (ix < 0x416e5bec) /* 14.89744 */
        {
          Z = __sy0_ep_fZ5_MP;
          P = __sy0_ep_fP5;
        } else {
          Z = __sy0_ep_fZ6_MP;
          P = __sy0_ep_fP6;
        }
        px = xf - Z[0];
        px = px - Z[1];
        presult =
            P[0] +
            px *
                (P[1] +
                 px *
                     (P[2] +
                      px *
                          (P[3] +
                           px *
                               (P[4] +
                                px *
                                    (P[5] +
                                     px *
                                         (P[6] +
                                          px *
                                              (P[7] +
                                               px *
                                                   (P[8] +
                                                    px *
                                                        (P[9] +
                                                         px *
                                                             (P[10] +
                                                              px *
                                                                  (P[11] +
                                                                   px *
                                                                       (P[12]))))))))))));
        *r = (float)presult;
        return nRet;
      }
    } else /* finite x >= 18.04340171 Hancels asymptotic forms */
    {
      const float ptonpi = 0x1.45f306p-1; // 0.63661975 [0x3f22f983] 2/Pi
      float px = xf, pxi = (1.0f / px), py = (16.0f * pxi), pz = py * py,
            pt = pz * pz, psq = __sqrt(ptonpi * pxi), psn, pcs, pp, pq, presult;
      __sy0_ep_sincos_kernel_fp32(px, -1, &psn, &pcs);
      pp = __sy0_ep_fPP[0] +
           pz * (__sy0_ep_fPP[1] +
                 pz * (__sy0_ep_fPP[2] +
                       pz * (__sy0_ep_fPP[3] +
                             pz * (__sy0_ep_fPP[4] + pz * (__sy0_ep_fPP[5])))));
      pq = py * (__sy0_ep_fQP[0] +
                 pz * (__sy0_ep_fQP[1] +
                       pz * (__sy0_ep_fQP[2] +
                             pz * (__sy0_ep_fQP[3] +
                                   pz * (__sy0_ep_fQP[4] +
                                         pz * (__sy0_ep_fQP[5]))))));
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
      const unsigned int _ones[] = {0x3f800000, 0xbf800000}; /* +1,-1 */
      *r = (((const float *)_ones)[(1)] /
            0.0f); /* raise div-by-zero, return -INF*/
      return nRet;
    } else if (ix & 0x80000000) /* negative x */
    {
      const unsigned int _infs[] = {0x7f800000, 0xff800000}; /* +INF,-INF */
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
} /* namespace __imf_impl_y0_s_ep */
DEVICE_EXTERN_C_INLINE float __devicelib_imf_y0f(float x) {
  using namespace __imf_impl_y0_s_ep;
  float r;
  __devicelib_imf_internal_sy0(&x, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
