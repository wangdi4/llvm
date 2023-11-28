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
namespace __imf_impl_yn_s_ep {
namespace {
static const int32_t __syn_sincosf_ep___ip_h = 0x0517CC1B;
static const int32_t __syn_sincosf_ep___ip_m = 0x727220A9;
static const int32_t __syn_sincosf_ep___ip_l = 0x28;
static const union {
  uint32_t w;
  float f;
} __syn_sincosf_ep___cc4 = {0x3e6ce1b2u};
static const union {
  uint32_t w;
  float f;
} __syn_sincosf_ep___cc3 = {0xbfaae2beu};
static const union {
  uint32_t w;
  float f;
} __syn_sincosf_ep___cc2 = {0x4081e0eeu};
static const union {
  uint32_t w;
  float f;
} __syn_sincosf_ep___cc1 = {0xc09de9e6u};
static const union {
  uint32_t w;
  float f;
} __syn_sincosf_ep___cc1l = {0xb3e646a5u};
static const union {
  uint32_t w;
  float f;
} __syn_sincosf_ep___cc0 = {0x3f800000u};
static const union {
  uint32_t w;
  float f;
} __syn_sincosf_ep___cs3 = {0xbf16c981u};
static const union {
  uint32_t w;
  float f;
} __syn_sincosf_ep___cs2 = {0x40232f49u};
static const union {
  uint32_t w;
  float f;
} __syn_sincosf_ep___cs1 = {0xc0a55dddu};
static const union {
  uint32_t w;
  float f;
} __syn_sincosf_ep___cs0 = {0x40490fdbu};
static const union {
  uint32_t w;
  float f;
} __syn_sincosf_ep___cs0l = {0xb3d195e9u};
static const uint32_t __syn_sincosf_ep_invpi_tbl[] = {
    0,          0x28BE60DB, 0x9391054A, 0x7F09D5F4,
    0x7D4D3770, 0x36D8A566, 0x4F10E410, 0x7F9458EA};
static inline int __syn_sincos_ep_kernel_fp32(float xf, int n, float *psin,
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
    ip_low = (((uint32_t)__syn_sincosf_ep_invpi_tbl[index]) * ((uint32_t)mx));
    IP = (((uint64_t)((uint32_t)(__syn_sincosf_ep_invpi_tbl[index + 1]))) *
          ((uint32_t)(mx))) +
         (((uint64_t)ip_low) << 32);
    // scaled by 2^(95-j)
    IP2 = (((uint64_t)((uint32_t)(__syn_sincosf_ep_invpi_tbl[index + 2]))) *
           ((uint32_t)(mx))) +
          ((((uint64_t)((uint32_t)(__syn_sincosf_ep_invpi_tbl[index + 3]))) *
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
    IP_s = (((int64_t)((int32_t)(mx))) * ((int32_t)(__syn_sincosf_ep___ip_h)));
    IP = (uint64_t)IP_s;
    IP2_s = (((int64_t)((int32_t)(mx))) * ((int32_t)(__syn_sincosf_ep___ip_m)));
    IP2 = (uint64_t)IP2_s;
    // scale (23-ex)*2^(28+32+7)
    ip_low_s = (((int32_t)mx) * ((int32_t)__syn_sincosf_ep___ip_l));
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
  cpoly.f = __fma(__syn_sincosf_ep___cc4.f, R2h,
                                   __syn_sincosf_ep___cc3.f);
  spoly.f = __fma(__syn_sincosf_ep___cs3.f, R2h,
                                   __syn_sincosf_ep___cs2.f);
  cpoly.f = __fma(cpoly.f, R2h, __syn_sincosf_ep___cc2.f);
  spoly.f = __fma(spoly.f, R2h, __syn_sincosf_ep___cs1.f);
  cpoly.f = __fma(cpoly.f, R2h, __syn_sincosf_ep___cc1.f);
  spoly.f = __fma(spoly.f, R2h, __syn_sincosf_ep___cs0.f);
  cpoly.f = __fma(cpoly.f, R2h, __syn_sincosf_ep___cc0.f);
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
static inline int __syn_ln_ep_kernel_fp32(float x, float *r) {
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
  } else if (x == 0.0f) {
    uint32_t ires = 0xff800000;
    *r = *(float *)&ires;
    return -1;
  } else if (x < 0.0f) {
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
static const float __syn_y0f_ep_fQ1[] = {
    -0x1.2e4d6ap-4f,  //    -0.073804297 [0xbd9726b5]
    0x1.6bbcb4p-3f,   //      0.17760602 [0x3e35de5a]
    -0x1.075b18p-6f,  //    -0.016073965 [0xbc83ad8c]
    0x1.1a5e14p-11f,  //   0.00053857325 [0x3a0d2f0a]
    -0x1.3b7a08p-17f, //  -9.4019379e-06 [0xb71dbd04]
};
/* polynomial minimax approximation of J0(x) = 1+y*Q2(y), y=x^2, |x|=[0..0.6],
 * max.err .88e-11 */
static const float __syn_y0f_ep_fQ2[] = {
    -0x1p-2f,         //           -0.25 [0xbe800000]
    0x1.fffffep-7f,   //     0.015624999 [0x3c7fffff]
    -0x1.c71982p-12f, //  -0.00043401684 [0xb9e38cc1]
    0x1.c3d898p-18f,  //   6.7330311e-06 [0x36e1ec4c]
};
/* polynomial minimax approximation Y0(x) = P1(x-z1) on interval (0.6,s1). */
static const float __syn_y0f_ep_fZ1_MP[] = {
    0x1.c982eap-1f,
    0x1.8d417ep-25f, // HI + LO:      0.89357692 +   4.6246665e-08 [0x3f64c175 +
                     // 0x3346a0bf]
};
static const float __syn_y0f_ep_fP1_MP[] = {
    -0x1.af74bep-56f,
    -0x1.a0f13p-80f, // HI + LO:  -2.3389278e-17 +  -1.3472127e-24 [0xa3d7ba5f +
                     // 0x97d07898]
    0x1.c2437p-1f,
    0x1.8421e4p-25f, // HI + LO:      0.87942076 +   4.5184557e-08 [0x3f6121b8 +
                     // 0x334210f2]
    -0x1.f7e38ap-2f,
    -0x1.5e717cp-28f, // HI + LO:     -0.49207893 +  -5.0996212e-09 [0xbefbf1c5
                      // + 0xb1af38be]
    0x1.c3b134p-3f,
    0x1.cb4394p-30f, // HI + LO:      0.22055283 +   1.6707926e-09 [0x3e61d89a +
                     // 0x30e5a1ca]
    -0x1.cf18dap-3f,
    -0x1.01f1a8p-27f, // HI + LO:     -0.22612162 +  -7.5071576e-09 [0xbe678c6d
                      // + 0xb200f8d4]
    0x1.c067cap-3f,
    0x1.2d87fcp-29f, // HI + LO:      0.21894796 +   2.1939282e-09 [0x3e6033e5 +
                     // 0x3116c3fe]
    -0x1.a397e2p-3f,
    -0x1.035c76p-27f, // HI + LO:     -0.20487954 +  -7.5484037e-09 [0xbe51cbf1
                      // + 0xb201ae3b]
    0x1.942de4p-3f,
    0x1.d9866p-27f, // HI + LO:      0.19735315 +   1.3781388e-08 [0x3e4a16f2 +
                    // 0x326cc330]
    -0x1.8d324cp-3f,
    -0x1.1d89aap-29f, // HI + LO:     -0.19394359 +  -2.0775606e-09 [0xbe469926
                      // + 0xb10ec4d5]
    0x1.8b78ep-3f,
    0x1.90dbdep-27f, // HI + LO:      0.19310164 +   1.1666528e-08 [0x3e45bc70 +
                     // 0x32486def]
    -0x1.8e4fa2p-3f,
    -0x1.c35a84p-28f, // HI + LO:     -0.19448783 +   -6.568059e-09 [0xbe4727d1
                      // + 0xb1e1ad42]
    0x1.99180ep-3f,
    0x1.1cbb4cp-27f, // HI + LO:      0.19975291 +    8.286781e-09 [0x3e4c8c07 +
                     // 0x320e5da6]
    -0x1.afbbaap-3f,
    -0x1.fe4a22p-27f, // HI + LO:     -0.21080716 +  -1.4851381e-08 [0xbe57ddd5
                      // + 0xb27f2511]
    0x1.c5ed8ep-3f,
    0x1.a7f108p-31f, // HI + LO:      0.22164451 +   7.7114515e-10 [0x3e62f6c7 +
                     // 0x3053f884]
    -0x1.b6cbc2p-3f,
    -0x1.210156p-30f, // HI + LO:     -0.21425582 +  -1.0513949e-09 [0xbe5b65e1
                      // + 0xb09080ab]
    0x1.6434cep-3f,
    0x1.df2d3ap-27f, // HI + LO:      0.17392884 +   1.3945876e-08 [0x3e321a67 +
                     // 0x326f969d]
    -0x1.c3594ap-4f,
    -0x1.e466cap-29f, // HI + LO:     -0.11019257 +  -3.5244849e-09 [0xbde1aca5
                      // + 0xb1723365]
    0x1.a3a98cp-5f,
    0x1.2ba6c6p-30f, // HI + LO:     0.051228307 +   1.0901257e-09 [0x3d51d4c6 +
                     // 0x3095d363]
    -0x1.0ab356p-6f,
    -0x1.22eb9p-30f, // HI + LO:    -0.016278109 +  -1.0583614e-09 [0xbc8559ab +
                     // 0xb09175c8]
    0x1.9c12ecp-9f,
    0x1.873c04p-34f, // HI + LO:    0.0031438745 +   8.8956412e-11 [0x3b4e0976 +
                     // 0x2ec39e02]
    -0x1.23377p-12f,
    -0x1.4cee56p-36f, // HI + LO:  -0.00027772575 +  -1.8924936e-11 [0xb9919bb8
                      // + 0xada6772b]
};
static const float __syn_y0f_ep_fP1[] = {
    -0x1.af74cp-56f,  //   -2.338928e-17 [0xa3d7ba60]
    0x1.c24372p-1f,   //      0.87942082 [0x3f6121b9]
    -0x1.f7e38ap-2f,  //     -0.49207893 [0xbefbf1c5]
    0x1.c3b134p-3f,   //      0.22055283 [0x3e61d89a]
    -0x1.cf18dcp-3f,  //     -0.22612163 [0xbe678c6e]
    0x1.c067cap-3f,   //      0.21894796 [0x3e6033e5]
    -0x1.a397e4p-3f,  //     -0.20487955 [0xbe51cbf2]
    0x1.942de6p-3f,   //      0.19735317 [0x3e4a16f3]
    -0x1.8d324cp-3f,  //     -0.19394359 [0xbe469926]
    0x1.8b78e2p-3f,   //      0.19310166 [0x3e45bc71]
    -0x1.8e4fa2p-3f,  //     -0.19448783 [0xbe4727d1]
    0x1.99181p-3f,    //      0.19975293 [0x3e4c8c08]
    -0x1.afbbacp-3f,  //     -0.21080717 [0xbe57ddd6]
    0x1.c5ed8ep-3f,   //      0.22164451 [0x3e62f6c7]
    -0x1.b6cbc2p-3f,  //     -0.21425582 [0xbe5b65e1]
    0x1.6434dp-3f,    //      0.17392886 [0x3e321a68]
    -0x1.c3594ap-4f,  //     -0.11019257 [0xbde1aca5]
    0x1.a3a98cp-5f,   //     0.051228307 [0x3d51d4c6]
    -0x1.0ab358p-6f,  //     -0.01627811 [0xbc8559ac]
    0x1.9c12ecp-9f,   //    0.0031438745 [0x3b4e0976]
    -0x1.233772p-12f, //  -0.00027772578 [0xb9919bb9]
};
/* polynomial minimax approximation Y0(x) = P2(x-z2)/Q2(x-z2) on interval
 * (s1,s2). */
static const float __syn_y0f_ep_fZ2_MP[] = {
    0x1.fa9534p+1f,
    0x1.b30ad4p-24f, // HI + LO:       3.9576783 +   1.0129118e-07 [0x407d4a9a +
                     // 0x33d9856a]
};
static const float __syn_y0f_ep_fP2[] = {
    -0x1.8fa896p-55f, //  -4.3331066e-17 [0xa447d44b]
    -0x1.9c3426p-2f,  //     -0.40254268 [0xbece1a13]
    0x1.a09c92p-5f,   //     0.050855909 [0x3d504e49]
    0x1.df6d5ap-5f,   //     0.058523823 [0x3d6fb6ad]
    -0x1.c116fep-8f,  //   -0.0068525667 [0xbbe08b7f]
    -0x1.1e32bap-9f,  //   -0.0021835186 [0xbb0f195d]
    0x1.998266p-13f,  //   0.00019526928 [0x394cc133]
    0x1.ab2b2cp-15f,  //    5.092247e-05 [0x38559596]
    -0x1.486008p-18f, //  -4.8931706e-06 [0xb6a43004]
    -0x1.3a6d62p-22f, //  -2.9283322e-07 [0xb49d36b1]
    -0x1.7a5c5p-26f,  //  -2.2023485e-08 [0xb2bd2e28]
    0x1.3b9fcep-26f,  //   1.8371749e-08 [0x329dcfe7]
    -0x1.ed292ap-29f, //  -3.5882171e-09 [0xb1769495]
    0x1.1e2174p-30f,  //   1.0409373e-09 [0x308f10ba]
    -0x1.a70ee4p-33f, //  -1.9238458e-10 [0xaf538772]
    -0x1.00258ep-35f, //  -2.9120508e-11 [0xae0012c7]
    -0x1.eb0008p-37f, //  -1.3955063e-11 [0xad758004]
    0x1.90e974p-37f,  //   1.1394602e-11 [0x2d4874ba]
};
/* polynomial minimax approximation Y0(x) = P3(x-z3) on interval (s2,s3). */
static const float __syn_y0f_ep_fZ3_MP[] = {
    0x1.c581dcp+2f,
    0x1.39c84p-24f, // HI + LO:        7.086051 +   7.3058118e-08 [0x40e2c0ee +
                    // 0x339ce420]
};
static const float __syn_y0f_ep_fP3[] = {
    0x1.e91b1ap-56f,  //   2.6514482e-17 [0x23f48d8d]
    0x1.334ccap-2f,   //      0.30009761 [0x3e99a665]
    -0x1.5aef62p-6f,  //    -0.021175237 [0xbcad77b1]
    -0x1.8969c6p-5f,  //     -0.04802407 [0xbd44b4e3]
    0x1.b2f14ap-9f,   //    0.0033183482 [0x3b5978a5]
    0x1.1d35e8p-9f,   //     0.002175984 [0x3b0e9af4]
    -0x1.26dd6cp-13f, //  -0.00014060255 [0xb9136eb6]
    -0x1.8177f2p-15f, //   -4.595143e-05 [0xb840bbf9]
    0x1.6a903p-19f,   //   2.7013066e-06 [0x36354818]
    0x1.34acb6p-21f,  //   5.7495134e-07 [0x351a565b]
    -0x1.09db12p-25f, //  -3.0949682e-08 [0xb304ed89]
    -0x1.447abcp-28f, //  -4.7217972e-09 [0xb1a23d5e]
    0x1.e2d8dep-33f,  //   2.1957346e-10 [0x2f716c6f]
    0x1.f3106ap-36f,  //   2.8368511e-11 [0x2df98835]
};
/* polynomial minimax approximation Y0(x) = P4(x-z4) on interval (s3,s4). */
static const float __syn_y0f_ep_fZ4_MP[] = {
    0x1.471d72p+3f,
    0x1.5a47d6p-21f, // HI + LO:       10.222344 +    6.449979e-07 [0x41238eb9 +
                     // 0x352d23eb]
};
static const float __syn_y0f_ep_fP4[] = {
    -0x1.cabd7cp-53f, //  -1.9894684e-16 [0xa5655ebe]
    -0x1.ff635cp-3f,  //     -0.24970123 [0xbe7fb1ae]
    0x1.903646p-7f,   //     0.012213501 [0x3c481b23]
    0x1.4e667ap-5f,   //     0.040820349 [0x3d27333d]
    -0x1.0325eep-9f,  //   -0.0019771436 [0xbb0192f7]
    -0x1.fe2392p-10f, //   -0.0019460256 [0xbaff11c9]
    0x1.7f84ccp-14f,  //   9.1437993e-05 [0x38bfc266]
    0x1.6afddap-15f,  //   4.3271972e-05 [0x38357eed]
    -0x1.040398p-19f, //  -1.9372555e-06 [0xb60201cc]
    -0x1.2aeb2ap-21f, //   -5.567793e-07 [0xb5157595]
    0x1.936d56p-26f,  //   2.3482547e-08 [0x32c9b6ab]
    0x1.428b84p-28f,  //   4.6936472e-09 [0x31a145c2]
    -0x1.86ccap-33f,  //  -1.7771495e-10 [0xaf436650]
    -0x1.e2e486p-36f, //   -2.744927e-11 [0xadf17243]
};
/* polynomial minimax approximation Y0(x) = P5(x-z5) on interval (s4,s5). */
static const float __syn_y0f_ep_fZ5_MP[] = {
    0x1.ab8e1cp+3f,
    0x1.2879d2p-23f, // HI + LO:       13.361097 +   1.3805733e-07 [0x4155c70e +
                     // 0x34143ce9]
};
static const float __syn_y0f_ep_fP5[] = {
    0x1.4d9fe4p-53f,  //   1.4468659e-16 [0x2526cff2]
    0x1.bf32a2p-3f,   //      0.21835829 [0x3e5f9951]
    -0x1.0bc2d8p-7f,  //   -0.0081714205 [0xbc05e16c]
    -0x1.26cab4p-5f,  //    -0.035985328 [0xbd13655a]
    0x1.5f03e2p-10f,  //    0.0013390166 [0x3aaf81f1]
    0x1.caaa6cp-10f,  //    0.0017496708 [0x3ae55536]
    -0x1.0c5ec6p-14f, //  -6.3984444e-05 [0xb8862f63]
    -0x1.4f08eep-15f, //   -3.993927e-05 [0xb8278477]
    0x1.7d0b6p-20f,   //   1.4195011e-06 [0x35be85b0]
    0x1.1a779p-21f,   //   5.2613586e-07 [0x350d3bc8]
    -0x1.340368p-26f, //  -1.7928734e-08 [0xb29a01b4]
    -0x1.2879dep-28f, //  -4.3142943e-09 [0xb1943cef]
    0x1.19366cp-33f,  //   1.2788068e-10 [0x2f0c9b36]
};
/* polynomial minimax approximation Y0(x) = P6(x-z6) on interval (s5,s6). */
static const float __syn_y0f_ep_fZ6_MP[] = {
    0x1.0803c6p+4f,
    0x1.400322p-20f, // HI + LO:       16.500921 +   1.1921385e-06 [0x418401e3 +
                     // 0x35a00191]
};
static const float __syn_y0f_ep_fP6[] = {
    0x1.cd828p-53f,   //   2.0014796e-16 [0x2566c140]
    -0x1.925c36p-3f,  //     -0.19646494 [0xbe492e1b]
    0x1.86254ap-8f,   //    0.0059531503 [0x3bc312a5]
    0x1.0a4512p-5f,   //     0.032503638 [0x3d052289]
    -0x1.013b36p-10f, //  -0.00098125951 [0xba809d9b]
    -0x1.a24a06p-10f, //   -0.0015956465 [0xbad12503]
    0x1.8f90cp-15f,   //   4.7631911e-05 [0x3847c860]
    0x1.35cf82p-15f,  //   3.6932299e-05 [0x381ae7c1]
    -0x1.2274ecp-20f, //  -1.0820356e-06 [0xb5913a76]
    -0x1.09482ap-21f, //  -4.9412603e-07 [0xb504a415]
    0x1.e2b94p-27f,   //   1.4049107e-08 [0x32715ca0]
    0x1.1a5b64p-28f,  //    4.108835e-09 [0x318d2db2]
    -0x1.c5f2cap-34f, //  -1.0321591e-10 [0xaee2f965]
};
/* polynomial pade approximation P0(x) = PP(256/x^2) in point 256/x^2 = 0.5 */
static const float __syn_y0f_ep_fPP[] = {
    0x1p+0f,          //               1 [0x3f800000]
    -0x1.2p-12f,      //   -0.0002746582 [0xb9900000]
    0x1.cb5f86p-20f,  //   1.7112983e-06 [0x35e5afc3]
    -0x1.24f578p-25f, //    -3.41049e-08 [0xb3127abc]
    0x1.7ca5eep-30f,  //   1.3847899e-09 [0x30be52f7]
    -0x1.47a91p-34f,  //   -7.450135e-11 [0xaea3d488]
};
/* polynomial pade approximation Q0(x) = QP(256/x^2)*(16/x)) in point 256/x^2 =
 * 0.5 */
static const float __syn_y0f_ep_fQP[] = {
    -0x1p-7f,         //      -0.0078125 [0xbc000000]
    0x1.2cp-16f,      //   1.7881393e-05 [0x37960000]
    -0x1.d11ca8p-23f, //  -2.1658462e-07 [0xb4688e54]
    0x1.b9d68ep-28f,  //   6.4295906e-09 [0x31dceb47]
    -0x1.7a8362p-32f, //  -3.4425576e-10 [0xafbd41b1]
    0x1.845fecp-36f,  //   2.2076545e-11 [0x2dc22ff6]
};
static inline int __syn_y0_ep_kernel_fp32(const float *a, float *r) {
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
                  0x1.45f306p-1f; // 0.63661975 [0x3f22f983] 2/Pi
              float px = xf, plnx, ps, py, pp, pq, presult;
              __syn_ln_ep_kernel_fp32(px, &plnx);
              ps = ptonpi * plnx;
              py = px * px;
              pp = __syn_y0f_ep_fQ1[0] +
                   py * (__syn_y0f_ep_fQ1[1] +
                         py * (__syn_y0f_ep_fQ1[2] +
                               py * (__syn_y0f_ep_fQ1[3] +
                                     py * (__syn_y0f_ep_fQ1[4]))));
              pq = ps * (py * (__syn_y0f_ep_fQ2[0] +
                               py * (__syn_y0f_ep_fQ2[1] +
                                     py * (__syn_y0f_ep_fQ2[2] +
                                           py * (__syn_y0f_ep_fQ2[3])))));
              presult = pp + pq;
              presult += ps;
              *r = (float)presult;
              return nRet;
            } else /* .6 <= x < 2.197141408 */
            {
              float px, xh, xl, pl, ph, presult;
              xh = -__syn_y0f_ep_fZ1_MP[0];
              xl = -__syn_y0f_ep_fZ1_MP[1];
              {
                float __ph, __ahl, __ahh;
                __ph = __fma(xh, 1.0f, xf);
                __ahh = __fma(__ph, 1.0f, -xf);
                __ahl = __fma(xh, 1.0f, -__ahh);
                xl = xl + __ahl;
                xh = __ph;
              };
              px = xh + xl;
              ph = __syn_y0f_ep_fP1[18] +
                   px * (__syn_y0f_ep_fP1[19] + px * (__syn_y0f_ep_fP1[20]));
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
                __bh = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[17 * 2]))
                           ? (__syn_y0f_ep_fP1_MP[17 * 2])
                           : (ph);
                __ah = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[17 * 2]))
                           ? (ph)
                           : (__syn_y0f_ep_fP1_MP[17 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __syn_y0f_ep_fP1_MP[17 * 2 + 1]) + __ahl;
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
                __bh = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[16 * 2]))
                           ? (__syn_y0f_ep_fP1_MP[16 * 2])
                           : (ph);
                __ah = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[16 * 2]))
                           ? (ph)
                           : (__syn_y0f_ep_fP1_MP[16 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __syn_y0f_ep_fP1_MP[16 * 2 + 1]) + __ahl;
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
                __bh = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[15 * 2]))
                           ? (__syn_y0f_ep_fP1_MP[15 * 2])
                           : (ph);
                __ah = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[15 * 2]))
                           ? (ph)
                           : (__syn_y0f_ep_fP1_MP[15 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __syn_y0f_ep_fP1_MP[15 * 2 + 1]) + __ahl;
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
                __bh = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[14 * 2]))
                           ? (__syn_y0f_ep_fP1_MP[14 * 2])
                           : (ph);
                __ah = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[14 * 2]))
                           ? (ph)
                           : (__syn_y0f_ep_fP1_MP[14 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __syn_y0f_ep_fP1_MP[14 * 2 + 1]) + __ahl;
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
                __bh = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[13 * 2]))
                           ? (__syn_y0f_ep_fP1_MP[13 * 2])
                           : (ph);
                __ah = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[13 * 2]))
                           ? (ph)
                           : (__syn_y0f_ep_fP1_MP[13 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __syn_y0f_ep_fP1_MP[13 * 2 + 1]) + __ahl;
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
                __bh = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[12 * 2]))
                           ? (__syn_y0f_ep_fP1_MP[12 * 2])
                           : (ph);
                __ah = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[12 * 2]))
                           ? (ph)
                           : (__syn_y0f_ep_fP1_MP[12 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __syn_y0f_ep_fP1_MP[12 * 2 + 1]) + __ahl;
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
                __bh = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[11 * 2]))
                           ? (__syn_y0f_ep_fP1_MP[11 * 2])
                           : (ph);
                __ah = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[11 * 2]))
                           ? (ph)
                           : (__syn_y0f_ep_fP1_MP[11 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __syn_y0f_ep_fP1_MP[11 * 2 + 1]) + __ahl;
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
                __bh = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[10 * 2]))
                           ? (__syn_y0f_ep_fP1_MP[10 * 2])
                           : (ph);
                __ah = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[10 * 2]))
                           ? (ph)
                           : (__syn_y0f_ep_fP1_MP[10 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __syn_y0f_ep_fP1_MP[10 * 2 + 1]) + __ahl;
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
                __bh = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[9 * 2]))
                           ? (__syn_y0f_ep_fP1_MP[9 * 2])
                           : (ph);
                __ah = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[9 * 2]))
                           ? (ph)
                           : (__syn_y0f_ep_fP1_MP[9 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __syn_y0f_ep_fP1_MP[9 * 2 + 1]) + __ahl;
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
                __bh = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[8 * 2]))
                           ? (__syn_y0f_ep_fP1_MP[8 * 2])
                           : (ph);
                __ah = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[8 * 2]))
                           ? (ph)
                           : (__syn_y0f_ep_fP1_MP[8 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __syn_y0f_ep_fP1_MP[8 * 2 + 1]) + __ahl;
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
                __bh = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[7 * 2]))
                           ? (__syn_y0f_ep_fP1_MP[7 * 2])
                           : (ph);
                __ah = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[7 * 2]))
                           ? (ph)
                           : (__syn_y0f_ep_fP1_MP[7 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __syn_y0f_ep_fP1_MP[7 * 2 + 1]) + __ahl;
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
                __bh = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[6 * 2]))
                           ? (__syn_y0f_ep_fP1_MP[6 * 2])
                           : (ph);
                __ah = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[6 * 2]))
                           ? (ph)
                           : (__syn_y0f_ep_fP1_MP[6 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __syn_y0f_ep_fP1_MP[6 * 2 + 1]) + __ahl;
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
                __bh = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[5 * 2]))
                           ? (__syn_y0f_ep_fP1_MP[5 * 2])
                           : (ph);
                __ah = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[5 * 2]))
                           ? (ph)
                           : (__syn_y0f_ep_fP1_MP[5 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __syn_y0f_ep_fP1_MP[5 * 2 + 1]) + __ahl;
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
                __bh = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[4 * 2]))
                           ? (__syn_y0f_ep_fP1_MP[4 * 2])
                           : (ph);
                __ah = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[4 * 2]))
                           ? (ph)
                           : (__syn_y0f_ep_fP1_MP[4 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __syn_y0f_ep_fP1_MP[4 * 2 + 1]) + __ahl;
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
                __bh = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[3 * 2]))
                           ? (__syn_y0f_ep_fP1_MP[3 * 2])
                           : (ph);
                __ah = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[3 * 2]))
                           ? (ph)
                           : (__syn_y0f_ep_fP1_MP[3 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __syn_y0f_ep_fP1_MP[3 * 2 + 1]) + __ahl;
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
                __bh = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[2 * 2]))
                           ? (__syn_y0f_ep_fP1_MP[2 * 2])
                           : (ph);
                __ah = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[2 * 2]))
                           ? (ph)
                           : (__syn_y0f_ep_fP1_MP[2 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __syn_y0f_ep_fP1_MP[2 * 2 + 1]) + __ahl;
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
                __bh = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[1 * 2]))
                           ? (__syn_y0f_ep_fP1_MP[1 * 2])
                           : (ph);
                __ah = (__fabs(ph) <=
                        __fabs(__syn_y0f_ep_fP1_MP[1 * 2]))
                           ? (ph)
                           : (__syn_y0f_ep_fP1_MP[1 * 2]);
                __ph = __fma(__ah, 1.0f, __bh);
                __ahh = __fma(__ph, 1.0f, -__bh);
                __ahl = __fma(__ah, 1.0f, -__ahh);
                pl = (pl + __syn_y0f_ep_fP1_MP[1 * 2 + 1]) + __ahl;
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
              presult = ph + pl + __syn_y0f_ep_fP1[0];
              *r = (float)presult;
              return nRet;
            }
          } else /* 2.1971414 <= x < 5.4296808 */
          {
            float px, presult;
            px = xf - __syn_y0f_ep_fZ2_MP[0];
            px = px - __syn_y0f_ep_fZ2_MP[1];
            presult =
                __syn_y0f_ep_fP2[0] +
                px *
                    (__syn_y0f_ep_fP2[1] +
                     px *
                         (__syn_y0f_ep_fP2[2] +
                          px *
                              (__syn_y0f_ep_fP2[3] +
                               px *
                                   (__syn_y0f_ep_fP2[4] +
                                    px *
                                        (__syn_y0f_ep_fP2[5] +
                                         px *
                                             (__syn_y0f_ep_fP2[6] +
                                              px *
                                                  (__syn_y0f_ep_fP2[7] +
                                                   px *
                                                       (__syn_y0f_ep_fP2[8] +
                                                        px *
                                                            (__syn_y0f_ep_fP2
                                                                 [9] +
                                                             px *
                                                                 (__syn_y0f_ep_fP2
                                                                      [10] +
                                                                  px *
                                                                      (__syn_y0f_ep_fP2
                                                                           [11] +
                                                                       px *
                                                                           (__syn_y0f_ep_fP2
                                                                                [12] +
                                                                            px *
                                                                                (__syn_y0f_ep_fP2
                                                                                     [13] +
                                                                                 px *
                                                                                     (__syn_y0f_ep_fP2
                                                                                          [14] +
                                                                                      px *
                                                                                          (__syn_y0f_ep_fP2
                                                                                               [15] +
                                                                                           px *
                                                                                               (__syn_y0f_ep_fP2
                                                                                                    [16] +
                                                                                                px *
                                                                                                    (__syn_y0f_ep_fP2
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
            Z = __syn_y0f_ep_fZ3_MP;
            P = __syn_y0f_ep_fP3;
          } else {
            Z = __syn_y0f_ep_fZ4_MP;
            P = __syn_y0f_ep_fP4;
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
          Z = __syn_y0f_ep_fZ5_MP;
          P = __syn_y0f_ep_fP5;
        } else {
          Z = __syn_y0f_ep_fZ6_MP;
          P = __syn_y0f_ep_fP6;
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
      const float ptonpi = 0x1.45f306p-1f; // 0.63661975 [0x3f22f983] 2/Pi
      float px = xf, pxi = (1.0f / px), py = (16.0f * pxi), pz = py * py,
            pt = pz * pz, psq = __sqrt(ptonpi * pxi), psn, pcs,
            pp, pq, presult;
      __syn_sincos_ep_kernel_fp32(px, -1, &psn, &pcs);
      pp = __syn_y0f_ep_fPP[0] +
           pz * (__syn_y0f_ep_fPP[1] +
                 pz * (__syn_y0f_ep_fPP[2] +
                       pz * (__syn_y0f_ep_fPP[3] +
                             pz * (__syn_y0f_ep_fPP[4] +
                                   pz * (__syn_y0f_ep_fPP[5])))));
      pq = py * (__syn_y0f_ep_fQP[0] +
                 pz * (__syn_y0f_ep_fQP[1] +
                       pz * (__syn_y0f_ep_fQP[2] +
                             pz * (__syn_y0f_ep_fQP[3] +
                                   pz * (__syn_y0f_ep_fQP[4] +
                                         pz * (__syn_y0f_ep_fQP[5]))))));
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
/* file: _vsy1_kernel_cout.i */
/* polynomial minimax approximation Y1(x) = x*Q1(x^2)+2/pi*(j0(x)*log(x)-1/x) on
 * interval (MinVal,1.77). */
static const float __syn_y1f_ep_fQ1[] = {
    -0x1.918662p-3f,  //      -0.1960571 [0xbe48c331]
    0x1.bd3974p-5f,   //     0.054348685 [0x3d5e9cba]
    -0x1.835b76p-9f,  //   -0.0029553014 [0xbb41adbb]
    0x1.2c795cp-14f,  //   7.1638598e-05 [0x38963cae]
    -0x1.09e396p-20f, //  -9.9051374e-07 [0xb584f1cb]
    0x1.1fc6b2p-27f,  //   8.3753884e-09 [0x320fe359]
};
static const float __syn_y1f_ep_fQ1_MP[] = {
    -0x1.91866p-3f,
    -0x1.3b5d12p-27f, // HI + LO:     -0.19605708 +  -9.1782875e-09 [0xbe48c330
                      // + 0xb21dae89]
    0x1.bd3974p-5f,
    0x1.c599f6p-30f, // HI + LO:     0.054348685 +   1.6501923e-09 [0x3d5e9cba +
                     // 0x30e2ccfb]
    -0x1.835b76p-9f,
    -0x1.825414p-35f, // HI + LO:   -0.0029553014 +  -4.3920458e-11 [0xbb41adbb
                      // + 0xae412a0a]
    0x1.2c795cp-14f,
    0x1.33037ap-40f, // HI + LO:   7.1638598e-05 +   1.0907313e-12 [0x38963cae +
                     // 0x2b9981bd]
    -0x1.09e394p-20f,
    -0x1.25761p-44f, // HI + LO:  -9.9051363e-07 +  -6.5161472e-14 [0xb584f1ca +
                     // 0xa992bb08]
    0x1.1fc6bp-27f,
    0x1.590678p-51f, // HI + LO:   8.3753875e-09 +   5.9852343e-16 [0x320fe358 +
                     // 0x262c833c]
};
/* polynomial minimax approximation of J0(x) = 1+y*Q2(y), y=x^2,
 * |x|=[0..1.7699999809265136718750], max.err .15e-10 */
static const float __syn_y1f_ep_fQ2[] = {
    0x1p-1f,          //             0.5 [0x3f000000]
    -0x1p-4f,         //         -0.0625 [0xbd800000]
    0x1.55554ap-9f,   //    0.0026041653 [0x3b2aaaa5]
    -0x1.c718b6p-15f, //  -5.4251734e-05 [0xb8638c5b]
    0x1.6b8304p-21f,  //   6.7709345e-07 [0x3535c182]
    -0x1.6f3516p-28f, //  -5.3435705e-09 [0xb1b79a8b]
};
static const float __syn_y1f_ep_fQ2_MP[] = {
    0x1.fffffep-2f,
    0x1.ffbef6p-26f, // HI + LO:      0.49999997 +   2.9787534e-08 [0x3effffff +
                     // 0x32ffdf7b]
    -0x1.fffffep-5f,
    -0x1.d12e0ep-29f, // HI + LO:    -0.062499996 +  -3.3846292e-09 [0xbd7fffff
                      // + 0xb1689707]
    0x1.55554ap-9f,
    0x1.a22f18p-35f, // HI + LO:    0.0026041653 +   4.7542012e-11 [0x3b2aaaa5 +
                     // 0x2e51178c]
    -0x1.c718b6p-15f,
    -0x1.ce322cp-41f, // HI + LO:  -5.4251734e-05 +    -8.21025e-13 [0xb8638c5b
                      // + 0xab671916]
    0x1.6b8302p-21f,
    0x1.12c5dp-45f, // HI + LO:    6.770934e-07 +   3.0505898e-14 [0x3535c181 +
                    // 0x290962e8]
    -0x1.6f3514p-28f,
    -0x1.dca096p-52f, // HI + LO:    -5.34357e-09 +  -4.1340827e-16 [0xb1b79a8a
                      // + 0xa5ee504b]
};
/* polynomial minimax approximation Y1(x) = P1(x-z1) on interval (1.77,s1). */
static const float __syn_y1f_ep_fZ1_MP[] = {
    0x1.193becp+1f,
    0x1.4dff24p-23f, // HI + LO:       2.1971412 +   1.5552931e-07 [0x400c9df6 +
                     // 0x3426ff92]
};
static const float __syn_y1f_ep_fP1[] = {
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
static const float __syn_y1f_ep_fZ2_MP[] = {
    0x1.5b7fe4p+2f,
    0x1.d0f606p-23f, // HI + LO:       5.4296808 +   2.1651435e-07 [0x40adbff2 +
                     // 0x34687b03]
};
static const float __syn_y1f_ep_fP2[] = {
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
static const float __syn_y1f_ep_fZ3_MP[] = {
    0x1.13127ap+3f,
    0x1.cc2d36p-22f, // HI + LO:       8.5960054 +   4.2857286e-07 [0x4109893d +
                     // 0x34e6169b]
};
static const float __syn_y1f_ep_fP3[] = {
    -0x1.63bc02p-54f, //  -7.7137603e-17 [0xa4b1de01]
    0x1.15f994p-2f,   //      0.27145988 [0x3e8afcca]
    -0x1.02b394p-6f,  //    -0.015789885 [0xbc8159ca]
    -0x1.6395ep-5f,   //    -0.043406427 [0xbd31caf0]
    0x1.3ced2cp-9f,   //    0.0024179569 [0x3b1e7696]
    0x1.07a674p-9f,   //    0.0020114915 [0x3b03d33a]
    -0x1.b50dd6p-14f, //   -0.0001042018 [0xb8da86eb]
    -0x1.6f79bcp-15f, //  -4.3806496e-05 [0xb837bcde]
    0x1.1777aep-19f,  //   2.0821951e-06 [0x360bbbd7]
    0x1.2b9662p-21f,  //   5.5802508e-07 [0x3515cb31]
    -0x1.a5371p-26f,  //  -2.4517945e-08 [0xb2d29b88]
    -0x1.335ab2p-28f, //  -4.4725934e-09 [0xb199ad59]
    0x1.b52aa6p-33f,  //   1.9880035e-10 [0x2f5a9553]
};
/* polynomial minimax approximation Y1(x) = P4(x-z4) on interval (s3,s4). */
static const float __syn_y1f_ep_fZ4_MP[] = {
    0x1.77f912p+3f,
    0x1.8d432p-21f, // HI + LO:       11.749154 +   7.3995852e-07 [0x413bfc89 +
                    // 0x3546a190]
};
static const float __syn_y1f_ep_fP4[] = {
    0x1.f96d32p-58f,  //    6.849807e-18 [0x22fcb699]
    -0x1.dc14eap-3f,  //     -0.23246177 [0xbe6e0a75]
    0x1.4429fep-7f,   //    0.0098927012 [0x3c2214ff]
    0x1.367d7ep-5f,   //     0.037901636 [0x3d1b3ebf]
    -0x1.9d6eb2p-10f, //   -0.0015771195 [0xbaceb759]
    -0x1.dc4f98p-10f, //   -0.0018169819 [0xbaee27cc]
    0x1.315eb8p-14f,  //    7.280588e-05 [0x3898af5c]
    0x1.571808p-15f,  //   4.0899977e-05 [0x382b8c04]
    -0x1.a294c6p-20f, //  -1.5593363e-06 [0xb5d14a63]
    -0x1.1e837ap-21f, //  -5.3367313e-07 [0xb50f41bd]
    0x1.4a11c4p-26f,  //   1.9212568e-08 [0x32a508e2]
    0x1.387092p-28f,  //   4.5465964e-09 [0x319c3849]
    -0x1.449c6ap-33f, //  -1.4761599e-10 [0xaf224e35]
    -0x1.c50aa6p-36f, //  -2.5752433e-11 [0xade28553]
};
/* polynomial minimax approximation Y1(x) = P5(x-z5) on interval (s4,s5). */
static const float __syn_y1f_ep_fZ5_MP[] = {
    0x1.dcb7d8p+3f,
    0x1.1bd092p-22f, // HI + LO:       14.897442 +   2.6432306e-07 [0x416e5bec +
                     // 0x348de849]
};
static const float __syn_y1f_ep_fP5[] = {
    0x1.213202p-53f,  //   1.2541849e-16 [0x25109901]
    0x1.a7022cp-3f,   //      0.20654711 [0x3e538116]
    -0x1.c650b6p-8f,  //    -0.006932301 [0xbbe3285b]
    -0x1.163192p-5f,  //    -0.033959184 [0xbd0b18c9]
    0x1.26b046p-10f,  //    0.0011241477 [0x3a935823]
    0x1.b175f8p-10f,  //    0.0016535218 [0x3ad8bafc]
    -0x1.c0aa2ep-15f, //  -5.3485008e-05 [0xb8605517]
    -0x1.3e3796p-15f, //  -3.7934438e-05 [0xb81f1bcb]
    0x1.3f3fa8p-20f,  //   1.1892939e-06 [0x359f9fd4]
    0x1.0e4578p-21f,  //   5.0341964e-07 [0x350722bc]
    -0x1.06481ap-26f, //  -1.5266801e-08 [0xb283240d]
    -0x1.1deb74p-28f, //  -4.1606798e-09 [0xb18ef5ba]
    0x1.1fa85cp-33f,  //   1.3081156e-10 [0x2f0fd42e]
};
/* polynomial minimax approximation Y1(x) = P6(x-z6) on interval (s5,s6). */
static const float __syn_y1f_ep_fZ6_MP[] = {
    0x1.20b1c6p+4f,
    0x1.2be3c8p-21f, // HI + LO:       18.043402 +   5.5858823e-07 [0x419058e3 +
                     // 0x3515f1e4]
};
static const float __syn_y1f_ep_fP6[] = {
    -0x1.39d4c4p-52f, //   -2.722051e-16 [0xa59cea62]
    -0x1.80781cp-3f,  //     -0.18772909 [0xbe403c0e]
    0x1.54eda6p-8f,   //    0.0052021532 [0x3baa76d3]
    0x1.fbe6ep-6f,    //     0.030999869 [0x3cfdf370]
    -0x1.be318ep-11f, //   -0.0008510467 [0xba5f18c7]
    -0x1.8efee4p-10f, //   -0.0015220477 [0xbac77f72]
    0x1.59144ep-15f,  //    4.113666e-05 [0x382c8a27]
    0x1.282d1ep-15f,  //   3.5306959e-05 [0x3814168f]
    -0x1.f56836p-21f, //  -9.3394345e-07 [0xb57ab41b]
    -0x1.fdcc5cp-22f, //  -4.7478665e-07 [0xb4fee62e]
    0x1.a3b264p-27f,  //   1.2214786e-08 [0x3251d932]
    0x1.1d892ap-28f,  //   4.1550927e-09 [0x318ec495]
    -0x1.b2a37p-34f,  //  -9.8825337e-11 [0xaed951b8]
    -0x1.ab2b06p-36f, //  -2.4281693e-11 [0xadd59583]
};
/* polynomial pade approximation P0(x) = PP(256/x^2) in point 256/x^2 = 0.5 */
static const float __syn_y1f_ep_fPP[] = {
    0x1p+0f,          //               1 [0x3f800000]
    0x1.ep-12f,       //   0.00045776367 [0x39f00000]
    -0x1.274fbep-19f, //  -2.2002421e-06 [0xb613a7df]
    0x1.5a3d1ep-25f,  //   4.0307494e-08 [0x332d1e8f]
    -0x1.afbe9cp-30f, //  -1.5706776e-09 [0xb0d7df4e]
    0x1.6be2b6p-34f,  //   8.2738004e-11 [0x2eb5f15b]
};
static const float __syn_y1f_ep_fPP_MP[] = {
    0x1p+0f,
    0x1.138p-43f, // HI + LO:               1 +   1.2234658e-13 [0x3f800000 +
                  // 0x2a09c000]
    0x1.dffffep-12f,
    0x1.e5e8a2p-36f, // HI + LO:   0.00045776364 +   2.7620713e-11 [0x39efffff +
                     // 0x2df2f451]
    -0x1.274fbcp-19f,
    -0x1.bfe60ep-43f, // HI + LO:  -2.2002419e-06 +  -1.9890696e-13 [0xb613a7de
                      // + 0xaa5ff307]
    0x1.5a3d1ep-25f,
    0x1.4dad62p-50f, // HI + LO:   4.0307494e-08 +   1.1576756e-15 [0x332d1e8f +
                     // 0x26a6d6b1]
    -0x1.afbe9cp-30f,
    -0x1.7a291cp-58f, // HI + LO:  -1.5706776e-09 +  -5.1250316e-18 [0xb0d7df4e
                      // + 0xa2bd148e]
    0x1.6be2b6p-34f,
    0x1.2b74cep-60f, // HI + LO:   8.2738004e-11 +   1.0145973e-18 [0x2eb5f15b +
                     // 0x2195ba67]
};
/* polynomial pade approximation Q0(x) = QP(256/x^2)*(16/x)) in point 256/x^2 =
 * 0.5 */
static const float __syn_y1f_ep_fQP[] = {
    0x1.8p-6f,        //       0.0234375 [0x3cc00000]
    -0x1.a4p-16f,     //  -2.5033951e-05 [0xb7d20000]
    0x1.1c3c46p-22f,  //   2.6471488e-07 [0x348e1e23]
    -0x1.fdd85cp-28f, //  -7.4192235e-09 [0xb1feec2e]
    0x1.a76f66p-32f,  //   3.8511203e-10 [0x2fd3b7b3]
    -0x1.ab6366p-36f, //  -2.4294211e-11 [0xadd5b1b3]
};
static const float __syn_y1f_ep_fQP_MP[] = {
    0x1.7ffffep-6f,
    0x1.fffcfcp-30f, // HI + LO:     0.023437498 +   1.8626023e-09 [0x3cbfffff +
                     // 0x30fffe7e]
    -0x1.a3fffep-16f,
    -0x1.6d894cp-40f, // HI + LO:  -2.5033949e-05 +  -1.2986459e-12 [0xb7d1ffff
                      // + 0xabb6c4a6]
    0x1.1c3c44p-22f,
    0x1.96a0fep-46f, // HI + LO:   2.6471486e-07 +   2.2572437e-14 [0x348e1e22 +
                     // 0x28cb507f]
    -0x1.fdd85cp-28f,
    -0x1.47f8dep-55f, // HI + LO:  -7.4192235e-09 +   -3.555881e-17 [0xb1feec2e
                      // + 0xa423fc6f]
    0x1.a76f64p-32f,
    0x1.8cb7ep-56f, // HI + LO:     3.85112e-10 +    2.150614e-17 [0x2fd3b7b2 +
                    // 0x23c65bf0]
    -0x1.ab6366p-36f,
    -0x1.769e9ap-61f, // HI + LO:  -2.4294211e-11 +  -6.3463018e-19 [0xadd5b1b3
                      // + 0xa13b4f4d]
};
static inline int __syn_y1_ep_kernel_fp32(const float *a, float *r) {
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
                      0x1.45f306p-1f; // 0.63661975 [0x3f22f983] 2/Pi
                  float px = xf, plnx, ps1, ps2, py, pp, pq, ps, presult;
                  __syn_ln_ep_kernel_fp32(px, &plnx);
                  ps1 = ptonpi * plnx;
                  ps2 = ptonpi / px;
                  py = px * px;
                  pp = px *
                       (__syn_y1f_ep_fQ1[0] +
                        py * (__syn_y1f_ep_fQ1[1] +
                              py * (__syn_y1f_ep_fQ1[2] +
                                    py * (__syn_y1f_ep_fQ1[3] +
                                          py * (__syn_y1f_ep_fQ1[4] +
                                                py * (__syn_y1f_ep_fQ1[5]))))));
                  pq = px *
                       (__syn_y1f_ep_fQ2[0] +
                        py * (__syn_y1f_ep_fQ2[1] +
                              py * (__syn_y1f_ep_fQ2[2] +
                                    py * (__syn_y1f_ep_fQ2[3] +
                                          py * (__syn_y1f_ep_fQ2[4] +
                                                py * (__syn_y1f_ep_fQ2[5]))))));
                  ps1 = ps1 * pq;
                  ps = ps1 - ps2;
                  presult = pp + ps;
                  *r = (float)presult;
                  return nRet;
                } else /* 1.76999998 <= x < 3.95767831 */
                {
                  float presult, px = xf - __syn_y1f_ep_fZ1_MP[0];
                  px = px - __syn_y1f_ep_fZ1_MP[1];
                  presult =
                      __syn_y1f_ep_fP1[0] +
                      px *
                          (__syn_y1f_ep_fP1[1] +
                           px *
                               (__syn_y1f_ep_fP1[2] +
                                px *
                                    (__syn_y1f_ep_fP1[3] +
                                     px *
                                         (__syn_y1f_ep_fP1[4] +
                                          px *
                                              (__syn_y1f_ep_fP1[5] +
                                               px *
                                                   (__syn_y1f_ep_fP1[6] +
                                                    px *
                                                        (__syn_y1f_ep_fP1[7] +
                                                         px *
                                                             (__syn_y1f_ep_fP1
                                                                  [8] +
                                                              px *
                                                                  (__syn_y1f_ep_fP1
                                                                       [9] +
                                                                   px *
                                                                       (__syn_y1f_ep_fP1
                                                                            [10] +
                                                                        px *
                                                                            (__syn_y1f_ep_fP1
                                                                                 [11] +
                                                                             px *
                                                                                 (__syn_y1f_ep_fP1
                                                                                      [12] +
                                                                                  px *
                                                                                      (__syn_y1f_ep_fP1
                                                                                           [13] +
                                                                                       px *
                                                                                           (__syn_y1f_ep_fP1
                                                                                                [14] +
                                                                                            px *
                                                                                                (__syn_y1f_ep_fP1
                                                                                                     [15])))))))))))))));
                  *r = (float)presult;
                  return nRet;
                }
              } else /* 3.9576783 <= x < 7.0860509 */
              {
                float presult, px = xf - __syn_y1f_ep_fZ2_MP[0];
                px = px - __syn_y1f_ep_fZ2_MP[1];
                presult =
                    __syn_y1f_ep_fP2[0] +
                    px *
                        (__syn_y1f_ep_fP2[1] +
                         px *
                             (__syn_y1f_ep_fP2[2] +
                              px *
                                  (__syn_y1f_ep_fP2[3] +
                                   px *
                                       (__syn_y1f_ep_fP2[4] +
                                        px *
                                            (__syn_y1f_ep_fP2[5] +
                                             px *
                                                 (__syn_y1f_ep_fP2[6] +
                                                  px *
                                                      (__syn_y1f_ep_fP2[7] +
                                                       px *
                                                           (__syn_y1f_ep_fP2
                                                                [8] +
                                                            px *
                                                                (__syn_y1f_ep_fP2
                                                                     [9] +
                                                                 px *
                                                                     (__syn_y1f_ep_fP2
                                                                          [10] +
                                                                      px *
                                                                          (__syn_y1f_ep_fP2
                                                                               [11] +
                                                                           px *
                                                                               (__syn_y1f_ep_fP2
                                                                                    [12] +
                                                                                px *
                                                                                    (__syn_y1f_ep_fP2
                                                                                         [13])))))))))))));
                *r = (float)presult;
                return nRet;
              }
            } else /* 7.0860509 <= x < 10.222345 */
            {
              float presult, px = xf - __syn_y1f_ep_fZ3_MP[0];
              px = px - __syn_y1f_ep_fZ3_MP[1];
              presult =
                  __syn_y1f_ep_fP3[0] +
                  px *
                      (__syn_y1f_ep_fP3[1] +
                       px *
                           (__syn_y1f_ep_fP3[2] +
                            px *
                                (__syn_y1f_ep_fP3[3] +
                                 px *
                                     (__syn_y1f_ep_fP3[4] +
                                      px *
                                          (__syn_y1f_ep_fP3[5] +
                                           px *
                                               (__syn_y1f_ep_fP3[6] +
                                                px *
                                                    (__syn_y1f_ep_fP3[7] +
                                                     px *
                                                         (__syn_y1f_ep_fP3[8] +
                                                          px *
                                                              (__syn_y1f_ep_fP3
                                                                   [9] +
                                                               px *
                                                                   (__syn_y1f_ep_fP3
                                                                        [10] +
                                                                    px *
                                                                        (__syn_y1f_ep_fP3
                                                                             [11] +
                                                                         px *
                                                                             __syn_y1f_ep_fP3
                                                                                 [12])))))))))));
              *r = (float)presult;
              return nRet;
            }
          } else /* 10.2223453 <= x < 13.36109733 */
          {
            float presult, px = xf - __syn_y1f_ep_fZ4_MP[0];
            px = px - __syn_y1f_ep_fZ4_MP[1];
            presult =
                __syn_y1f_ep_fP4[0] +
                px *
                    (__syn_y1f_ep_fP4[1] +
                     px *
                         (__syn_y1f_ep_fP4[2] +
                          px *
                              (__syn_y1f_ep_fP4[3] +
                               px *
                                   (__syn_y1f_ep_fP4[4] +
                                    px *
                                        (__syn_y1f_ep_fP4[5] +
                                         px *
                                             (__syn_y1f_ep_fP4[6] +
                                              px *
                                                  (__syn_y1f_ep_fP4[7] +
                                                   px *
                                                       (__syn_y1f_ep_fP4[8] +
                                                        px *
                                                            (__syn_y1f_ep_fP4
                                                                 [9] +
                                                             px *
                                                                 (__syn_y1f_ep_fP4
                                                                      [10] +
                                                                  px *
                                                                      (__syn_y1f_ep_fP4
                                                                           [11] +
                                                                       px *
                                                                           (__syn_y1f_ep_fP4
                                                                                [12] +
                                                                            px *
                                                                                (__syn_y1f_ep_fP4
                                                                                     [13])))))))))))));
            *r = (float)presult;
            return nRet;
          }
        } else /* 13.3610973 <= x < 16.500923 */
        {
          float presult, px = xf - __syn_y1f_ep_fZ5_MP[0];
          px = px - __syn_y1f_ep_fZ5_MP[1];
          presult =
              __syn_y1f_ep_fP5[0] +
              px *
                  (__syn_y1f_ep_fP5[1] +
                   px *
                       (__syn_y1f_ep_fP5[2] +
                        px *
                            (__syn_y1f_ep_fP5[3] +
                             px *
                                 (__syn_y1f_ep_fP5[4] +
                                  px *
                                      (__syn_y1f_ep_fP5[5] +
                                       px *
                                           (__syn_y1f_ep_fP5[6] +
                                            px *
                                                (__syn_y1f_ep_fP5[7] +
                                                 px *
                                                     (__syn_y1f_ep_fP5[8] +
                                                      px *
                                                          (__syn_y1f_ep_fP5[9] +
                                                           px *
                                                               (__syn_y1f_ep_fP5
                                                                    [10] +
                                                                px *
                                                                    (__syn_y1f_ep_fP5
                                                                         [11] +
                                                                     px *
                                                                         __syn_y1f_ep_fP5
                                                                             [12])))))))))));
          *r = (float)presult;
          return nRet;
        }
      } else /* 16.500923 <= x < 19.64130973 */
      {
        float presult, px = xf - __syn_y1f_ep_fZ6_MP[0];
        px = px - __syn_y1f_ep_fZ6_MP[1];
        presult =
            __syn_y1f_ep_fP6[0] +
            px *
                (__syn_y1f_ep_fP6[1] +
                 px *
                     (__syn_y1f_ep_fP6[2] +
                      px *
                          (__syn_y1f_ep_fP6[3] +
                           px *
                               (__syn_y1f_ep_fP6[4] +
                                px *
                                    (__syn_y1f_ep_fP6[5] +
                                     px *
                                         (__syn_y1f_ep_fP6[6] +
                                          px *
                                              (__syn_y1f_ep_fP6[7] +
                                               px *
                                                   (__syn_y1f_ep_fP6[8] +
                                                    px *
                                                        (__syn_y1f_ep_fP6[9] +
                                                         px *
                                                             (__syn_y1f_ep_fP6
                                                                  [10] +
                                                              px *
                                                                  (__syn_y1f_ep_fP6
                                                                       [11] +
                                                                   px *
                                                                       (__syn_y1f_ep_fP6
                                                                            [12] +
                                                                        px *
                                                                            (__syn_y1f_ep_fP6
                                                                                 [13])))))))))))));
        *r = (float)presult;
        return nRet;
      }
    } else /* finite x >= 19.6413097 Hancels asymptotic forms */
    {
      const float ptonpi = 0x1.45f306p-1f; // 0.63661975 [0x3f22f983] 2/Pi
      float px = xf, pxi = (1.0f / px), py = (16.0f * pxi), pz = py * py,
            pt = pz * pz, psq = __sqrt(ptonpi * pxi), psn, pcs,
            pp, pq, presult;
      __syn_sincos_ep_kernel_fp32(px, -3, &psn, &pcs);
      pp = __syn_y1f_ep_fPP[0] +
           pz * (__syn_y1f_ep_fPP[1] +
                 pz * (__syn_y1f_ep_fPP[2] +
                       pz * (__syn_y1f_ep_fPP[3] +
                             pz * (__syn_y1f_ep_fPP[4] +
                                   pz * (__syn_y1f_ep_fPP[5])))));
      pq = py * (__syn_y1f_ep_fQP[0] +
                 pz * (__syn_y1f_ep_fQP[1] +
                       pz * (__syn_y1f_ep_fQP[2] +
                             pz * (__syn_y1f_ep_fQP[3] +
                                   pz * (__syn_y1f_ep_fQP[4] +
                                         pz * (__syn_y1f_ep_fQP[5]))))));
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

/* file: _vssincos_hl_kernel_cout.i */
// unsigned 64-bit shift
// signed 64-bit shift
static const int32_t __syn_ep___ip_h = 0x0517CC1B;
static const int32_t __syn_ep___ip_m = 0x727220A9;
static const int32_t __syn_ep___ip_m2 = 0x4FE13ABE;
static const int32_t __syn_ep___ip_l = 0x48;
static const union {
  uint32_t w;
  float f;
} __syn_ep___cc5 = {0xbcd07725u};
static const union {
  uint32_t w;
  float f;
} __syn_ep___cc5l = {0x300699f8u};
static const union {
  uint32_t w;
  float f;
} __syn_ep___cc4 = {0x3e70f3e7u};
static const union {
  uint32_t w;
  float f;
} __syn_ep___cc4l = {0xb08f2818u};
static const union {
  uint32_t w;
  float f;
} __syn_ep___cc3 = {0xbfaae9ddu};
static const union {
  uint32_t w;
  float f;
} __syn_ep___cc3l = {0xb2479ff1u};
static const union {
  uint32_t w;
  float f;
} __syn_ep___cc2 = {0x4081e0f8u};
static const union {
  uint32_t w;
  float f;
} __syn_ep___cc2l = {0x33e9c2f2u};
static const union {
  uint32_t w;
  float f;
} __syn_ep___cc1 = {0xc09de9e6u};
static const union {
  uint32_t w;
  float f;
} __syn_ep___cc1l = {0xb41bd2c9u};
static const union {
  uint32_t w;
  float f;
} __syn_ep___cc0 = {0x3f800000u};
static const union {
  uint32_t w;
  float f;
} __syn_ep___cc0l = {0xa97a0000u};
static const union {
  uint32_t w;
  float f;
} __syn_ep___cs5 = {0xbbeea72au};
static const union {
  uint32_t w;
  float f;
} __syn_ep___cs5l = {0x2e34aadau};
static const union {
  uint32_t w;
  float f;
} __syn_ep___cs4 = {0x3da838dfu};
static const union {
  uint32_t w;
  float f;
} __syn_ep___cs4l = {0x3153b5efu};
static const union {
  uint32_t w;
  float f;
} __syn_ep___cs3 = {0xbf196963u};
static const union {
  uint32_t w;
  float f;
} __syn_ep___cs3l = {0xb1a62812u};
static const union {
  uint32_t w;
  float f;
} __syn_ep___cs2 = {0x402335e3u};
static const union {
  uint32_t w;
  float f;
} __syn_ep___cs2l = {0x3361fa23u};
static const union {
  uint32_t w;
  float f;
} __syn_ep___cs1 = {0xc0a55de7u};
static const union {
  uint32_t w;
  float f;
} __syn_ep___cs1l = {0xb3c4aef7u};
static const union {
  uint32_t w;
  float f;
} __syn_ep___cs0 = {0x40490fdbu};
static const union {
  uint32_t w;
  float f;
} __syn_ep___cs0l = {0xb3bbbd30u};
static const union {
  uint32_t w;
  float f;
} __syn_ep___cs_small = {0xBE2AAAABu};
static const union {
  uint32_t w;
  float f;
} __syn_ep___twom24 = {0x33800000u};
static const uint32_t invpi_tbl[] = {0,          0x28BE60DB, 0x9391054A,
                                     0x7F09D5F4, 0x7D4D3770, 0x36D8A566,
                                     0x4F10E410, 0x7F9458EA, 0xF7AEF158};
static inline int __syn_sincos_hl_ep_kernel_fp32(float xin, int n, float *psin,
                                                 float *pcos) {
  uint64_t IP, IP2, IP3;
  int64_t lN = (int64_t)n;
  int64_t IP_s, IP2_s, IP3_s;
  int32_t ip_low_s;
  uint32_t ip_low, ex;
  int_float x, Rh, Rl, res, scale, spoly, cpoly;
  int_float cpoly_h, spoly_h, cpoly_l, spoly_l, cres_h, sres_h, cres_l, sres_l;
  int mx, sgn_x, ex1, ip_h, shift, index, j, sgn_p, sgn_xp;
  float High, Low, R2h, R2l, Ph, Pl;
  x.f = xin;
  mx = (x.w & 0x007fffff) | 0x00800000;
  sgn_x = x.w & 0x80000000;
  ex = ((x.w ^ sgn_x) >> 23);
  // redirect large or very small inputs
  if (__builtin_expect(((unsigned)(ex - 0x7f + 11)) >= (16 + 11), (0 == 1))) {
    // small input (disabled)
    // if (__builtin_expect((ex < (0x7f - 12)), (1 == 1)))
    // if (__builtin_expect((ex < 0x7f - 11), (1 == 1)))
    //{
    //    psin[0] = xin;  psin[1] = _VSTATIC(__cs_small).f * xin*xin*xin;
    //    pcos[0] = 1.0f;  pcos[1] = -0.5f * xin*xin;
    //    return;
    //}
    // Inf/NaN
    if (ex == 0xff) // if (xa.w >= 0x7f800000)
    {
      x.w |= 0x00400000;
      psin[0] = pcos[0] = x.f;
      psin[1] = pcos[1] = x.f;
      return 0;
    }
    ex1 = ex - 0x7f - 23;
    index = 1 + (ex1 >> 5);
    j = ex1 & 0x1f; // expon % 32
    // x/Pi, scaled by 2^(63-j)
    ip_low = (((uint32_t)invpi_tbl[index]) * ((uint32_t)mx));
    IP = (((uint64_t)((uint32_t)(invpi_tbl[index + 1]))) * ((uint32_t)(mx))) +
         (((uint64_t)ip_low) << 32);
    // scaled by 2^(127-j)
    IP3 = (((uint64_t)((uint32_t)(invpi_tbl[index + 3]))) * ((uint32_t)(mx)));
    // scaled by 2^(95-j)
    IP2 = (((uint64_t)((uint32_t)(invpi_tbl[index + 2]))) * ((uint32_t)(mx))) +
          (IP3 >> 32);
    IP = IP + (IP2 >> 32);
    IP3 = (uint32_t)IP3;
    // scale 2^63
    IP <<= j;
    // shift low part by 32-j, j in [0,31]
    ip_low = (uint32_t)IP2;
    // IP3 scale:  2^127
    IP3 |= (((uint64_t)ip_low) << 32);
    IP3 <<= j;
    ip_low >>= (31 - j);
    ip_low >>= 1;
    IP |= (uint64_t)ip_low;
  } else // main path
  {
    // products are really unsigned; operands are small enough so that signed
    // MUL works as well x*2^(23-ex)*(1/Pi)*2^28 p[k] products fit in 31 bits
    // each
    IP_s = (((int64_t)((int32_t)(mx))) * ((int32_t)(__syn_ep___ip_h)));
    IP = (uint64_t)IP_s;
    IP2_s = (((int64_t)((int32_t)(mx))) * ((int32_t)(__syn_ep___ip_m)));
    IP2 = (uint64_t)IP2_s;
    IP3_s = (((int64_t)((int32_t)(mx))) * ((int32_t)(__syn_ep___ip_m2)));
    IP3 = (uint64_t)IP3_s;
    // (x/Pi)*2^63
    IP <<= (ex - 0x7f + 12);
    IP += (IP2 >> (20 + 0x7f - ex));
    IP += (IP3 >> (52 + 0x7f - ex));
    IP2 <<= (44 - 0x7f + ex);
    // additional  low bits
    IP3 <<= (12 - 0x7f + ex);
    IP3 += IP2;
    IP = (IP3 < IP2) ? (IP + 1) : IP;
  }
  // add correction by n
  IP += (uint64_t)(lN << 61);
  // return to 32-bit, scale 2^31
  ip_h = IP >> 32;
  // fix sign bit
  sgn_xp = ((ip_h + 0x20000000) & 0xc0000000);
  // reduced argument (signed, high-low), scale 2^32
  ip_h <<= 2;
  Rh.f = (float)ip_h;
  // reduced argument will need to be normalized
  shift = 2 + 30 + 0x7f - ((Rh.w >> 23) & 0xff);
  // correction for shift=0
  shift = (shift >= 2) ? shift : 2;
  // normalize
  IP = (IP << shift) | (IP3 >> (64 - shift));
  ip_h = IP >> 32;
  Rh.f = (float)ip_h;
  ip_h -= ((int)Rh.f);
  ip_low = (uint32_t)IP;
  ip_h = (ip_h << 24) | (ip_low >> 8);
  Rl.f = (float)ip_h;
  // adjust scale
  scale.w = (0x7f - 31 - shift) << 23;
  Rh.f = __fma(Rh.f, scale.f, 0.0f);
  Rl.f = __fma(Rl.f, scale.f, 0.0f);
  // scale Rl by 2^(-24)
  Rl.f = __fma(Rl.f, __syn_ep___twom24.f, 0.0f);
  //(Rh+Rl)^2
  R2h = Rh.f;
  R2l = Rl.f;
  {
    float __ph, __phl;
    __ph = __fma(R2h, Rh.f, 0.0f);
    __phl = __fma(R2h, Rh.f, -__ph);
    R2l = __fma(R2l, Rh.f, __phl);
    R2l = __fma(R2h, Rl.f, R2l);
    R2h = __ph;
  };
  // polynomial evaluation
  spoly_h.f = __syn_ep___cs5.f;
  spoly_l.f = __syn_ep___cs5l.f;
  cpoly_h.f = __syn_ep___cc5.f;
  cpoly_l.f = __syn_ep___cc5l.f;
  {
    float __ph, __phl;
    __ph = __fma(spoly_h.f, R2h, 0.0f);
    __phl = __fma(spoly_h.f, R2h, -__ph);
    spoly_l.f = __fma(spoly_l.f, R2h, __phl);
    spoly_l.f = __fma(spoly_h.f, R2l, spoly_l.f);
    spoly_h.f = __ph;
  };
  {
    float __ph, __ahl, __ahh;
    __ph = __fma(spoly_h.f, 1.0f, __syn_ep___cs4.f);
    __ahh = __fma(__ph, 1.0f, -__syn_ep___cs4.f);
    __ahl = __fma(spoly_h.f, 1.0f, -__ahh);
    spoly_l.f = (spoly_l.f + __syn_ep___cs4l.f) + __ahl;
    spoly_h.f = __ph;
  };
  {
    float __ph, __phl;
    __ph = __fma(cpoly_h.f, R2h, 0.0f);
    __phl = __fma(cpoly_h.f, R2h, -__ph);
    cpoly_l.f = __fma(cpoly_l.f, R2h, __phl);
    cpoly_l.f = __fma(cpoly_h.f, R2l, cpoly_l.f);
    cpoly_h.f = __ph;
  };
  {
    float __ph, __ahl, __ahh;
    __ph = __fma(cpoly_h.f, 1.0f, __syn_ep___cc4.f);
    __ahh = __fma(__ph, 1.0f, -__syn_ep___cc4.f);
    __ahl = __fma(cpoly_h.f, 1.0f, -__ahh);
    cpoly_l.f = (cpoly_l.f + __syn_ep___cc4l.f) + __ahl;
    cpoly_h.f = __ph;
  };
  {
    float __ph, __phl;
    __ph = __fma(spoly_h.f, R2h, 0.0f);
    __phl = __fma(spoly_h.f, R2h, -__ph);
    spoly_l.f = __fma(spoly_l.f, R2h, __phl);
    spoly_l.f = __fma(spoly_h.f, R2l, spoly_l.f);
    spoly_h.f = __ph;
  };
  {
    float __ph, __ahl, __ahh;
    __ph = __fma(spoly_h.f, 1.0f, __syn_ep___cs3.f);
    __ahh = __fma(__ph, 1.0f, -__syn_ep___cs3.f);
    __ahl = __fma(spoly_h.f, 1.0f, -__ahh);
    spoly_l.f = (spoly_l.f + __syn_ep___cs3l.f) + __ahl;
    spoly_h.f = __ph;
  };
  {
    float __ph, __phl;
    __ph = __fma(cpoly_h.f, R2h, 0.0f);
    __phl = __fma(cpoly_h.f, R2h, -__ph);
    cpoly_l.f = __fma(cpoly_l.f, R2h, __phl);
    cpoly_l.f = __fma(cpoly_h.f, R2l, cpoly_l.f);
    cpoly_h.f = __ph;
  };
  {
    float __ph, __ahl, __ahh;
    __ph = __fma(cpoly_h.f, 1.0f, __syn_ep___cc3.f);
    __ahh = __fma(__ph, 1.0f, -__syn_ep___cc3.f);
    __ahl = __fma(cpoly_h.f, 1.0f, -__ahh);
    cpoly_l.f = (cpoly_l.f + __syn_ep___cc3l.f) + __ahl;
    cpoly_h.f = __ph;
  };
  {
    float __ph, __phl;
    __ph = __fma(spoly_h.f, R2h, 0.0f);
    __phl = __fma(spoly_h.f, R2h, -__ph);
    spoly_l.f = __fma(spoly_l.f, R2h, __phl);
    spoly_l.f = __fma(spoly_h.f, R2l, spoly_l.f);
    spoly_h.f = __ph;
  };
  {
    float __ph, __ahl, __ahh;
    __ph = __fma(spoly_h.f, 1.0f, __syn_ep___cs2.f);
    __ahh = __fma(__ph, 1.0f, -__syn_ep___cs2.f);
    __ahl = __fma(spoly_h.f, 1.0f, -__ahh);
    spoly_l.f = (spoly_l.f + __syn_ep___cs2l.f) + __ahl;
    spoly_h.f = __ph;
  };
  {
    float __ph, __phl;
    __ph = __fma(cpoly_h.f, R2h, 0.0f);
    __phl = __fma(cpoly_h.f, R2h, -__ph);
    cpoly_l.f = __fma(cpoly_l.f, R2h, __phl);
    cpoly_l.f = __fma(cpoly_h.f, R2l, cpoly_l.f);
    cpoly_h.f = __ph;
  };
  {
    float __ph, __ahl, __ahh;
    __ph = __fma(cpoly_h.f, 1.0f, __syn_ep___cc2.f);
    __ahh = __fma(__ph, 1.0f, -__syn_ep___cc2.f);
    __ahl = __fma(cpoly_h.f, 1.0f, -__ahh);
    cpoly_l.f = (cpoly_l.f + __syn_ep___cc2l.f) + __ahl;
    cpoly_h.f = __ph;
  };
  {
    float __ph, __phl;
    __ph = __fma(spoly_h.f, R2h, 0.0f);
    __phl = __fma(spoly_h.f, R2h, -__ph);
    spoly_l.f = __fma(spoly_l.f, R2h, __phl);
    spoly_l.f = __fma(spoly_h.f, R2l, spoly_l.f);
    spoly_h.f = __ph;
  };
  {
    float __ph, __ahl, __ahh;
    __ph = __fma(spoly_h.f, 1.0f, __syn_ep___cs1.f);
    __ahh = __fma(__ph, 1.0f, -__syn_ep___cs1.f);
    __ahl = __fma(spoly_h.f, 1.0f, -__ahh);
    spoly_l.f = (spoly_l.f + __syn_ep___cs1l.f) + __ahl;
    spoly_h.f = __ph;
  };
  {
    float __ph, __phl;
    __ph = __fma(cpoly_h.f, R2h, 0.0f);
    __phl = __fma(cpoly_h.f, R2h, -__ph);
    cpoly_l.f = __fma(cpoly_l.f, R2h, __phl);
    cpoly_l.f = __fma(cpoly_h.f, R2l, cpoly_l.f);
    cpoly_h.f = __ph;
  };
  {
    float __ph, __ahl, __ahh;
    __ph = __fma(cpoly_h.f, 1.0f, __syn_ep___cc1.f);
    __ahh = __fma(__ph, 1.0f, -__syn_ep___cc1.f);
    __ahl = __fma(cpoly_h.f, 1.0f, -__ahh);
    cpoly_l.f = (cpoly_l.f + __syn_ep___cc1l.f) + __ahl;
    cpoly_h.f = __ph;
  };
  {
    float __ph, __phl;
    __ph = __fma(spoly_h.f, R2h, 0.0f);
    __phl = __fma(spoly_h.f, R2h, -__ph);
    spoly_l.f = __fma(spoly_l.f, R2h, __phl);
    spoly_l.f = __fma(spoly_h.f, R2l, spoly_l.f);
    spoly_h.f = __ph;
  };
  {
    float __ph, __ahl, __ahh;
    __ph = __fma(spoly_h.f, 1.0f, __syn_ep___cs0.f);
    __ahh = __fma(__ph, 1.0f, -__syn_ep___cs0.f);
    __ahl = __fma(spoly_h.f, 1.0f, -__ahh);
    spoly_l.f = (spoly_l.f + __syn_ep___cs0l.f) + __ahl;
    spoly_h.f = __ph;
  };
  {
    float __ph, __phl;
    __ph = __fma(cpoly_h.f, R2h, 0.0f);
    __phl = __fma(cpoly_h.f, R2h, -__ph);
    cpoly_l.f = __fma(cpoly_l.f, R2h, __phl);
    cpoly_l.f = __fma(cpoly_h.f, R2l, cpoly_l.f);
    cpoly_h.f = __ph;
  };
  {
    float __ph, __ahl, __ahh;
    __ph = __fma(cpoly_h.f, 1.0f, __syn_ep___cc0.f);
    __ahh = __fma(__ph, 1.0f, -__syn_ep___cc0.f);
    __ahl = __fma(cpoly_h.f, 1.0f, -__ahh);
    cpoly_l.f = (cpoly_l.f + __syn_ep___cc0l.f) + __ahl;
    cpoly_h.f = __ph;
  };
  {
    float __ph, __phl;
    __ph = __fma(spoly_h.f, Rh.f, 0.0f);
    __phl = __fma(spoly_h.f, Rh.f, -__ph);
    spoly_l.f = __fma(spoly_l.f, Rh.f, __phl);
    spoly_l.f = __fma(spoly_h.f, Rl.f, spoly_l.f);
    spoly_h.f = __ph;
  };
  sgn_p = sgn_xp & 0x80000000;
  // adjust sign
  cpoly_h.w ^= sgn_p;
  cpoly_l.w ^= sgn_p;
  sgn_p = sgn_p ^ (sgn_xp << 1);
  spoly_h.w ^= sgn_p;
  spoly_l.w ^= sgn_p;
  sres_h.w = (sgn_xp & 0x40000000) ? cpoly_h.w : spoly_h.w;
  cres_h.w = (sgn_xp & 0x40000000) ? spoly_h.w : cpoly_h.w;
  sres_l.w = (sgn_xp & 0x40000000) ? cpoly_l.w : spoly_l.w;
  cres_l.w = (sgn_xp & 0x40000000) ? spoly_l.w : cpoly_l.w;
  sres_h.w ^= sgn_x;
  sres_l.w ^= sgn_x;
  pcos[0] = cres_h.f;
  pcos[1] = cres_l.f;
  psin[0] = sres_h.f;
  psin[1] = sres_l.f;
  return 0;
}

/*
//
//  Inf points where all inputs on the left produce -INF results
//
*/
static const float function_infs[] = {
    0.0f,    0.0f,    0.0f,    0.4f,    1.3f,    3.0f,    5.3f,    8.1f,
    11.4f,   15.1f,   19.1f,   23.3f,   27.9f,   32.6f,   37.5f,   42.5f,
    47.8f,   53.1f,   58.5f,   64.1f,   69.8f,   75.5f,   81.3f,   87.2f,
    93.2f,   99.2f,   105.3f,  111.4f,  117.6f,  123.8f,  130.1f,  136.4f,
    142.8f,  149.2f,  155.6f,  162.1f,  168.6f,  175.1f,  181.7f,  188.2f,
    194.9f,  201.5f,  208.2f,  214.9f,  221.6f,  228.3f,  235.0f,  241.8f,
    248.6f,  255.4f,  262.2f,  269.1f,  275.9f,  282.8f,  289.7f,  296.6f,
    303.5f,  310.4f,  317.4f,  324.4f,  331.3f,  338.3f,  345.3f,  352.3f,
    359.3f,  366.4f,  373.4f,  380.5f,  387.5f,  394.6f,  401.7f,  408.8f,
    415.9f,  423.0f,  430.1f,  437.2f,  444.3f,  451.5f,  458.6f,  465.8f,
    473.0f,  480.1f,  487.3f,  494.5f,  501.7f,  508.9f,  516.1f,  523.3f,
    530.5f,  537.8f,  545.0f,  552.2f,  559.5f,  566.7f,  574.0f,  581.2f,
    588.5f,  595.8f,  603.1f,  610.4f,  617.6f,  624.9f,  632.2f,  639.5f,
    646.8f,  654.2f,  661.5f,  668.8f,  676.1f,  683.5f,  690.8f,  698.1f,
    705.5f,  712.8f,  720.2f,  727.5f,  734.9f,  742.2f,  749.6f,  757.0f,
    764.4f,  771.7f,  779.1f,  786.5f,  793.9f,  801.3f,  808.7f,  816.1f,
    823.5f,  830.9f,  838.3f,  845.7f,  853.1f,  860.5f,  868.0f,  875.4f,
    882.8f,  890.3f,  897.7f,  905.1f,  912.6f,  920.0f,  927.4f,  934.9f,
    942.3f,  949.8f,  957.3f,  964.7f,  972.2f,  979.6f,  987.1f,  994.6f,
    1002.0f, 1009.5f, 1017.0f, 1024.5f, 1031.9f, 1039.4f, 1046.9f, 1054.4f,
    1061.9f, 1069.4f, 1076.9f, 1084.4f, 1091.9f, 1099.4f, 1106.9f, 1114.4f,
    1121.9f, 1129.4f, 1136.9f, 1144.4f, 1151.9f, 1159.4f, 1167.0f, 1174.5f,
    1182.0f, 1189.5f, 1197.1f, 1204.6f, 1212.1f, 1219.6f, 1227.2f, 1234.7f,
    1242.3f, 1249.8f, 1257.3f, 1264.9f, 1272.4f, 1280.0f, 1287.5f, 1295.1f,
    1302.6f, 1310.2f, 1317.7f, 1325.3f, 1332.8f, 1340.4f, 1347.9f, 1355.5f,
    1363.1f, 1370.6f, 1378.2f, 1385.8f, 1393.3f, 1400.9f, 1408.5f, 1416.1f,
    1423.6f, 1431.2f, 1438.8f, 1446.4f, 1453.9f, 1461.5f, 1469.1f, 1476.7f,
    1484.3f, 1491.9f, 1499.4f, 1507.0f, 1514.6f, 1522.2f, 1529.8f, 1537.4f,
    1545.0f, 1552.6f, 1560.2f, 1567.8f, 1575.4f, 1583.0f, 1590.6f, 1598.2f,
    1605.8f, 1613.4f, 1621.0f, 1628.6f, 1636.3f, 1643.9f, 1651.5f, 1659.1f,
    1666.7f, 1674.3f, 1681.9f, 1689.6f, 1697.2f, 1704.8f, 1712.4f, 1720.0f,
    1727.7f, 1735.3f, 1742.9f, 1750.5f, 1758.2f, 1765.8f, 1773.4f, 1781.1f,
    1788.7f, 1796.3f, 1804.0f, 1811.6f, 1819.2f, 1826.9f, 1834.5f, 1842.1f,
    1849.8f, 1857.4f, 1865.1f, 1872.7f, 1880.3f, 1888.0f, 1895.6f, 1903.3f,
    1910.9f, 1918.6f, 1926.2f, 1933.9f, 1941.5f, 1949.2f, 1956.8f, 1964.5f,
    1972.1f, 1979.8f, 1987.4f, 1995.1f, 2002.8f, 2010.4f, 2018.1f, 2025.7f,
    2033.4f, 2041.0f, 2048.7f, 2056.4f, 2064.0f, 2071.0f, 2079.0f, 2087.0f,
    2094.0f, 2102.0f, 2110.0f, 2117.0f, 2125.0f, 2133.0f, 2140.0f, 2148.0f,
    2156.0f, 2163.0f, 2171.0f, 2179.0f, 2186.0f, 2194.0f, 2202.0f, 2209.0f,
    2217.0f, 2225.0f, 2232.0f, 2240.0f, 2248.0f, 2255.0f, 2263.0f, 2271.0f,
    2278.0f,
};
inline int __devicelib_imf_internal_syn(const int *n, const float *a,
                                        float *r) {
  int nRet = 0;
  int32_t in, idx, isign, iflgh, iflgs, imaxloopcnt;
  uint32_t ux, uax, un, uy;
  float fx, fy, flna, flnr, ft, fz, fp, fq, fpinv, fyj, fpn, fv, ft1, ft2, fx2,
      fs, fr, ftv, fn4, fx8, fkp1, fkp2, fkq1, fkq2, fmpy[2], fmpz[2];
  const float ftonpi = 0x1.45f306p-1f;        // 0.63661975 [0x3f22f983] = 2/Pi
  const float feulon2 = 0x1.c7f45cp-1;        // 0.89053620899509899899e-00
  const float foonpi = 0x1.45f306p-2;         // 0.31830988618379069121e-00
  const float fhighmask = 0x1.8p+9;           // 2^9 to clear low bits
  const float fzeros[] = {0x0p+0f, -0x0p+0f}; // Positive and negative zeros
  const float fones[] = {1.0f, -1.0f};        // 1.0f, -1.0f
  const uint32_t uinfs[] = {0x7f800000, 0xff800000};   // +INF, -INF
  const uint32_t ularges[] = {0x71800000, 0xf1800000}; //+2^100,-2^100
  const float *finfs = (const float *)uinfs;
  const float *flarges = (const float *)ularges;
  in = *n;
  fx = *a;
  ux = (*(uint32_t *)&fx);
  uax = ux & (~0x80000000);
  un = (in < 0) ? -in : in;
  // Branch for finite positive x
  if (ux - 0x00000001 < 0x7f800000 - 0x00000001) {
    // Branch to y0 for n=0
    if (un == 0) {
      return __syn_y0_ep_kernel_fp32(a, r);
    }
    // Branch to y1 for n=1
    else if (un == 1) {
      return __syn_y1_ep_kernel_fp32(a, r);
    }
    // For even n: isign=0, for odd n: isign=isign(n)*isign(x)
    isign = ((uint32_t)(ux ^ in) >> 31) & in;
    // Find where yn(n,x) = -INF
    idx = un >> 3;
    if (idx <= 320) {
      fy = function_infs[idx];
    } else // idx > 320
    {
      idx -= 320;
      fy = function_infs[320] + 7 * idx;
    }
    // Check if x is below left bound to return -INF
    if (fx < fy) {
      *r = flarges[1] * flarges[0]; // raise overflow, return -INF
      return nRet;
    }
    // PATH #1: Power series for small n & x
    if ((un < 6) && (fx <= 0.6f)) {
      imaxloopcnt = 100;
      fx2 = fx / 2.0f;
      ft = 2.0f / fx;
      fq = 1.0f;
      fp = 1.0f;
      fpn = foonpi;
      fr = 0.0f;
      for (idx = 1, fz = 1.0f; idx <= un; idx++, fz += 1.0f) {
        fp = fp * fx2;       // p = (x/2)^n
        fpn = fpn * ft;      // pn = (x/2)^(-n)
        fv = fq;             // v = (n-1)!
        fq = fq * fz;        // q = n!
        fr = fr + 1.0f / fz; // r = 1+1/2+...+1/n
      }
      idx = 0;
      fz = 1.0f;
      ft1 = fy = fr;
      fyj = 1.0f;
      fx2 *= -fx2;
      do {
        idx++;
        ft2 = ft1;
        ft = 1.0f / (idx * (un + idx));
        fr = fr + (un + 2 * idx) * ft;
        fz = fz * fx2 * ft;
        fyj += fz;
        fy += fz * fr;
        fs = fy * fhighmask;
        ftv = (fy + fs);
        ft1 = (ftv - fs); // t1 = y, clear low bits
      } while (ft1 != ft2 && imaxloopcnt--);
      fz = fp / fq;
      fyj = fyj * fz; // yj = jn(n,x)
      flna = feulon2 * fx;
      __syn_ln_ep_kernel_fp32(flna, &flnr);
      fy = ftonpi * fyj * flnr - foonpi * fy * fz;
      fx2 = -fx2;
      fz = fv;
      for (idx = 1; idx < un; idx++) {
        fv = fv * fx2 / (idx * (un - idx));
        fz += fv;
      }
      fy = fy - fz * fpn;
      *r = (float)(isign ? -fy : fy);
    }
    // PATH #2: n < 80 or x<(5*n+5000)
    else if ((un < 80) || (fx < (5 * un + 5000))) {
      // Call y0 and y1
      __syn_y0_ep_kernel_fp32(a, &fz);
      __syn_y1_ep_kernel_fp32(a, &fy);
      // Check if y1 result overflows into -Inf for small inputs
      // then set it to maximum negative number
      uy = *(uint32_t *)&fy;
      if (uy == 0xff800000) {
        uy = 0xff7fffff;
        fy = *(float *)&uy;
      }
      for (idx = 1; idx < un; idx++) {
        ft = fy;
        fy = (2.0f * idx / fx) * fy - fz;
        uy = *(uint32_t *)&fy;
        // Check for fy overflow to -INF
        // then return fy
        if (uy == 0xff800000) {
          isign = 0;
          break;
        }
        fz = ft;
      }
      *r = (float)(isign ? -fy : fy);
    }
    // PATH #3: Hancels asymptotic forms for big n or x
    else {
      iflgs = -1;
      imaxloopcnt = 100;
      fn4 = 4.0f * un * un;
      fx8 = fx * 8.0f;
      fv = fp = 1.0f;
      fq = (fn4 - 1.0f) / fx8;
      fz = fq;
      fkp1 = 2.0f;
      fkq1 = 3.0f;
      fkp2 = 3.0f;
      fkq2 = 5.0f;
      iflgh = 0;
      ft = 1.0f;
      fx2 = 1.0f;
      fpinv = (1.0f / fp);
      do {
        fy = fv;
        fz *= (fn4 - fkp2 * fkp2) / (fkp1 * fx8);
        fr = (fn4 - fkq2 * fkq2) / (fkq1 * fx8);
        fr *= fz;
        if (iflgs > 0.0f) {
          fp += fz;
          fq += fr;
        } else {
          fp -= fz;
          fq -= fr;
        }
        fz = fr;
        fkp1 += 2.0f;
        fkq1 += 2.0f;
        fkp2 += 4.0f;
        fkq2 += 4.0f;
        ft = __fabs(fz * fpinv);
        if (ft < fx2) {
          fx2 = ft;
          iflgh = 1;
        } else {
          if (iflgh != 0)
            break;
        }
        iflgs = -iflgs;
        fs = fp * fhighmask;
        ftv = (fp + fs);
        fv = (ftv - fs); // v = p, clear low bits
      } while (fv != fy && imaxloopcnt--);
      // Run sincos(x) kernel with octant correction to -(2*n + 1)
      __syn_sincos_hl_ep_kernel_fp32(fx, -(2 * un + 1), &fmpy[0], &fmpz[0]);
      // Nuklti-precision (p*y + q*z)
      {
        float __ph, __phl;
        __ph = __fma(fmpy[0], fp, 0.0f);
        __phl = __fma(fmpy[0], fp, -__ph);
        fmpy[1] = __fma(fmpy[1], fp, __phl);
        fmpy[0] = __ph;
      };
      {
        float __ph, __phl;
        __ph = __fma(fmpz[0], fq, 0.0f);
        __phl = __fma(fmpz[0], fq, -__ph);
        fmpz[1] = __fma(fmpz[1], fq, __phl);
        fmpz[0] = __ph;
      };
      {
        float __ph, __ahl, __ahh;
        float __ah, __bh;
        __bh = (__fabs(fmpy[0]) <=
                __fabs(fmpz[0]))
                   ? (fmpz[0])
                   : (fmpy[0]);
        __ah = (__fabs(fmpy[0]) <=
                __fabs(fmpz[0]))
                   ? (fmpy[0])
                   : (fmpz[0]);
        __ph = __fma(__ah, 1.0f, __bh);
        __ahh = __fma(__ph, 1.0f, -__bh);
        __ahl = __fma(__ah, 1.0f, -__ahh);
        fmpy[1] = (fmpy[1] + fmpz[1]) + __ahl;
        fmpy[0] = __ph;
      };
      fy = __sqrt(ftonpi / fx);
      fy = fy * (fmpy[0] + fmpy[1]);
      *r = (float)(isign ? -fy : fy);
    }
    return nRet;
  }
  // Special arguments path: NaN, INF, negative x, zero
  else {
    // NaN
    if (uax > 0x7f800000) {
      // Raise invalid on SNaN
      *r = fx * 1.0f;
      return nRet;
    }
    // x = +-0
    else if (uax == 0) {
      *r = fones[1] / fzeros[0];
      return nRet;
    }
    // Negative x
    else if (ux & 0x80000000) {
      // Raise invalid, return QNaN
      *r = fzeros[0] * finfs[0];
      return nRet;
    }
    // x = +INF
    else {
      // For even n:0, for odd n:isign(n)
      isign = ((uint32_t)in >> 31) & in;
      *r = fzeros[isign];
      return nRet;
    }
  }
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_yn_s_ep */
DEVICE_EXTERN_C_INLINE float __devicelib_imf_ynf(int32_t x, float y) {
  using namespace __imf_impl_yn_s_ep;
  float r;
  __devicelib_imf_internal_syn(&x, &y, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
