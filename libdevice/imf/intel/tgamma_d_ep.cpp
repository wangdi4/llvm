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
/*
// ALGORITHM DESCRIPTION:
//   ---------------------
//  The method consists of three cases.
//
//  If       2 <= x < OVERFLOW_BOUNDARY
//  else    if 0 < x < 2
//  else    if -(i+1) <  x < -i, i = 0...184
//
//  Case 2 <= x < OVERFLOW_BOUNDARY
//  -------------------------------
//    Here we use algorithm based on the recursive formula
//    GAMMA(x+1) = x*GAMMA(x). For that we subdivide interval
//    [2; OVERFLOW_BOUNDARY] into intervals [16*n; 16*(n+1)] and
//    approximate GAMMA(x) by polynomial of 22th degree on each
//    [16*n; 16*n+1], recursive formula is used to expand GAMMA(x)
//    to [16*n; 16*n+1]. In other words we need to find n, i and r
//    such that x = 16 * n + i + r where n and i are integer numbers
//    and r is fractional part of x. So GAMMA(x) = GAMMA(16*n+i+r) =
//    = (x-1)*(x-2)*...*(x-i)*GAMMA(x-i) =
//    = (x-1)*(x-2)*...*(x-i)*GAMMA(16*n+r) ~
//    ~ (x-1)*(x-2)*...*(x-i)*P22n(r).
//
//    Step 1: Reduction
//    -----------------
//     N = [x] with truncate
//     r = x - N, note 0 <= r < 1
//
//     n = N & ~0xF - index of table that contains coefficient of
//                    polynomial approximation
//     i = N & 0xF  - is used in recursive formula
//
//
//    Step 2: Approximation
//    ---------------------
//     We use factorized minimax approximation polynomials
//     P22n(r) = A22*(r^2+C01(n)*R+C00(n))*
//               *(r^2+C11(n)*R+C10(n))*...*(r^2+CA1(n)*R+CA0(n))
//
//    Step 3: Recursion
//    -----------------
//     In case when i > 0 we need to multiply P22n(r) by product
//     R(i)=(x-1)*(x-2)*...*(x-i). To reduce number of fp-instructions
//     we can calculate R as follow:
//     R(i) = ((x-1)*(x-2))*((x-3)*(x-4))*...*((x-(i-1))*(x-i)) if i is
//     even or R = ((x-1)*(x-2))*((x-3)*(x-4))*...*((x-(i-2))*(x-(i-1)))*
//     *(i-1) if i is odd. In both cases we need to calculate
//     R2(i) = (x^2-3*x+2)*(x^2-7*x+12)*...*(x^2+x+2*j*(2*j-1)) =
//     = (x^2-3*x+2)*(x^2-7*x+12)*...*((x^2+x)+2*j*(2*(j-1)+(1-2*x))) =
//     = (RA+2*(2-RB))*(RA+4*(4-RB))*...*(RA+2*j*(2*(j-1)+RB))
//     where j = 1..[i/2], RA = x^2+x, RB = 1-2*x.
//
//    Step 4: Reconstruction
//    ----------------------
//     Reconstruction is just simple multiplication i.e.
//     GAMMA(x) = P22n(r)*R(i)
//
//  Case 0 < x < 2
//  --------------
//     To calculate GAMMA(x) on this interval we do following
//         if 1 <= x < 1.25   than  GAMMA(x) = P15(x-1)
//         if 1.25 <= x < 1.5 than  GAMMA(x) = P15(x-x_min) where
//         x_min is point of local minimum on [1; 2] interval.
//         if 1.5  <= x < 2.0 than  GAMMA(x) = P15(x-1.5)
//     and
//         if 0 < x < 1 than GAMMA(x) = GAMMA(x+1)/x
//
//  Case -(i+1) <  x < -i, i = 0...184
//  ----------------------------------
//     Here we use the fact that GAMMA(-x) = PI/(x*GAMMA(x)*sin(PI*x)) and
//     so we need to calculate GAMMA(x), sin(PI*x)/PI. Calculation of
//     GAMMA(x) is described above.
//
//    Step 1: Reduction
//    -----------------
//     Note that period of sin(PI*x) is 2 and range reduction for
//     sin(PI*x) is like to range reduction for GAMMA(x)
//     i.e r = x - [x] with exception of cases
//     when r > 0.5 (in such cases r = 1 - (x - [x])).
//
//    Step 2: Approximation
//    ---------------------
//     To approximate sin(PI*x)/PI = sin(PI*(2*n+r))/PI =
//     = (-1)^n*sin(PI*r)/PI Taylor series is used.
//     sin(PI*r)/PI ~ S21(r).
//
//    Step 3: Division
//    ----------------
//     Calculate 1/(x*GAMMA(x)*S21(r)).
//
// --
//
*/
#include "_imf_include_fp64.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_tgamma_d_ep {
namespace {

typedef struct {
  double x[2];
} __dtgamma_ep_mp_double;

static inline __dtgamma_ep_mp_double __dtgamma_ep_mp_double_init_hl(double a,
                                                                    double b) {
  return {a, b};
}
static inline __dtgamma_ep_mp_double __dtgamma_ep_mp_double_init_h(double b) {
  return {b, 0.0};
}
static inline __dtgamma_ep_mp_double __dtgamma_ep_mp_double_init_lo(double b) {
  return {0.0, b};
}
static inline double __dtgamma_ep_mp_double_get_h(__dtgamma_ep_mp_double a) {
  return a.x[0];
}
static inline double __dtgamma_ep_mp_double_get_lo(__dtgamma_ep_mp_double a) {
  return a.x[1];
}
static inline double __dtgamma_ep_mp_double_reduce(__dtgamma_ep_mp_double a) {
  return (a.x[0] + a.x[1]);
}

static inline __dtgamma_ep_mp_double
__dtgamma_ep_mp_double_mul(__dtgamma_ep_mp_double a, __dtgamma_ep_mp_double b) {
  __dtgamma_ep_mp_double __mp_res;
  double __ph, __phl, Ah = a.x[0], Al = a.x[1], Bh = b.x[0], Bl = b.x[1];
  __ph = __fma(Ah, Bh, 0.0);
  __phl = __fma(Ah, Bh, -__ph);
  Al = __fma(Al, Bh, __phl);
  __mp_res.x[1] = __fma(Ah, Bl, Al);
  __mp_res.x[0] = __ph;
  return __mp_res;
}
static inline __dtgamma_ep_mp_double
__dtgamma_ep_mp_double_mul_h(__dtgamma_ep_mp_double a, double b) {
  __dtgamma_ep_mp_double __mp_res;
  double __ph, __phl, Ah = a.x[0], Al = a.x[1], B = b;
  __ph = __fma(Ah, B, 0.0);
  __phl = __fma(Ah, B, -__ph);
  __mp_res.x[1] = __fma(Al, B, __phl);
  __mp_res.x[0] = __ph;
  return __mp_res;
}
static inline __dtgamma_ep_mp_double
__dtgamma_ep_mp_double_add(__dtgamma_ep_mp_double a, __dtgamma_ep_mp_double b) {
  __dtgamma_ep_mp_double __mp_res;
  double __ph, __ahl, __ahh;
  double __ah, __bh, Ah = a.x[0], Al = a.x[1], Bh = b.x[0], Bl = b.x[1];
  __bh = (__fabs(Ah) <= __fabs(Bh)) ? (Bh)
                                                                      : (Ah);
  __ah = (__fabs(Ah) <= __fabs(Bh)) ? (Ah)
                                                                      : (Bh);
  __ph = __fma(__ah, 1.0, __bh);
  __ahh = __fma(__ph, 1.0, -__bh);
  __ahl = __fma(__ah, 1.0, -__ahh);
  __mp_res.x[1] = (Al + Bl) + __ahl;
  __mp_res.x[0] = __ph;
  return __mp_res;
}
static inline __dtgamma_ep_mp_double
__dtgamma_ep_mp_double_add_h(__dtgamma_ep_mp_double a, double b) {
  __dtgamma_ep_mp_double __mp_res;
  double __ph, __ahl, __ahh;
  double __ah, __bh, Ah = a.x[0], Al = a.x[1], Bh = b;
  __bh = (__fabs(Ah) <= __fabs(Bh)) ? (Bh)
                                                                      : (Ah);
  __ah = (__fabs(Ah) <= __fabs(Bh)) ? (Ah)
                                                                      : (Bh);
  __ph = __fma(__ah, 1.0, __bh);
  __ahh = __fma(__ph, 1.0, -__bh);
  __ahl = __fma(__ah, 1.0, -__ahh);
  __mp_res.x[1] = Al + __ahl;
  __mp_res.x[0] = __ph;
  return __mp_res;
}
static inline __dtgamma_ep_mp_double
__dtgamma_ep_mp_double_add_hl(__dtgamma_ep_mp_double a, double b, double c) {
  __dtgamma_ep_mp_double __mp_res;
  double __ph, __ahl, __ahh;
  double __ah, __bh, Ah = a.x[0], Al = a.x[1], Bh = b, Bl = c;
  __bh = (__fabs(Ah) <= __fabs(Bh)) ? (Bh)
                                                                      : (Ah);
  __ah = (__fabs(Ah) <= __fabs(Bh)) ? (Ah)
                                                                      : (Bh);
  __ph = __fma(__ah, 1.0, __bh);
  __ahh = __fma(__ph, 1.0, -__bh);
  __ahl = __fma(__ah, 1.0, -__ahh);
  __mp_res.x[1] = (Al + Bl) + __ahl;
  __mp_res.x[0] = __ph;
  return __mp_res;
}

static const double __dtgamma_ep_a_tab[] = {
    /* [2; 3] */
    0x1.de1d0b935c815p+1, // C01 = 3.735261389688285139     [0x400DE1D0B935C815]
    0x1.944093bb6812ep+1, // C11 = 3.158220736033533704     [0x400944093BB6812E]
    0x1.33440769b85a3p+1, // C21 = 2.400513579007296894     [0x40033440769B85A3]
    0x1.7ebb3b381852bp+0, // C31 = 1.495044423289366931     [0x3FF7EBB3B381852B]
    0x1.d1ba06fbd7067p-2, // C41 = 0.4548112002573873602    [0x3FDD1BA06FBD7067]
    -0x1.6d5c734546d54p-1, // C51 = -0.7135959646288179137 [0xBFE6D5C734546D54]
    -0x1.0092c1b850a05p+1, // C61 = -2.004478659627468407 [0xC000092C1B850A05]
    -0x1.b3e72e7083814p+1, // C71 = -3.405492596567464503 [0xC00B3E72E7083814]
    -0x1.383314cedf144p+2, // C81 = -4.878117754004339446 [0xC01383314CEDF144]
    -0x1.9316fb5c057c6p+2, // C91 = -6.298277702202488726 [0xC019316FB5C057C6]
    -0x1.d395e6358d26p+2, // CA1 = -7.306024124428034838    [0xC01D395E6358D260]
    0x1.f77bab0c1141ap+1, // C00 = 3.933461552517212745     [0x400F77BAB0C1141A]
    0x1.fba062da3c0fp+1,  // C10 = 3.965832096633782555     [0x400FBA062DA3C0F0]
    0x1.0c616ec08d183p+2, // C20 = 4.193446815527184235     [0x4010C616EC08D183]
    0x1.27dbc41c8fb23p+2, // C30 = 4.622788455860049162     [0x40127DBC41C8FB23]
    0x1.50de74718efadp+2, // C40 = 5.263577567002283963     [0x40150DE74718EFAD]
    0x1.889f68fd20b08p+2, // C50 = 6.134729621119511478     [0x401889F68FD20B08]
    0x1.d0c7dcefc388bp+2, // C60 = 7.26219867147859599      [0x401D0C7DCEFC388B]
    0x1.1582b7c47173bp+3, // C70 = 8.672206767728871668     [0x4021582B7C47173B]
    0x1.4bb296cb246a2p+3, // C80 = 10.36555041958064649     [0x4024BB296CB246A2]
    0x1.86f3b67c41429p+3, // C90 = 12.21725010174312054     [0x40286F3B67C41429]
    0x1.b594936e72e9fp+3, // CA0 = 13.67438670703933035     [0x402B594936E72E9F]
    0x1.021c92d435fd9p-30, // An  = 9.390045868056711888e-10
                           // [0x3E1021C92D435FD9]
    /* [16; 17] */
    0x1.f0feeafe6cbddp+1, // C01 = 3.882779478293271413     [0x400F0FEEAFE6CBDD]
    0x1.d9509f7524eb4p+1, // C11 = 3.69777291507656436      [0x400D9509F7524EB4]
    0x1.a9c14abc0f51dp+1, // C21 = 3.326211301637739925     [0x400A9C14ABC0F51D]
    0x1.61d68bd29914cp+1, // C31 = 2.76435992987732071      [0x40061D68BD29914C]
    0x1.00a1ad69ef90cp+1, // C41 = 2.004934002615419431     [0x40000A1AD69EF90C]
    0x1.08e3622354d24p+0, // C51 = 1.034719594598592529     [0x3FF08E3622354D24]
    -0x1.5b5e8c55f2cb1p-3, // C61 = -0.1696139301041959258 [0xBFC5B5E8C55F2CB1]
    -0x1.a64f8f4b0f518p+0, // C71 = -1.649651485276370622 [0xBFFA64F8F4B0F518]
    -0x1.bdea325699ebcp+1, // C81 = -3.483709614072578731 [0xC00BDEA325699EBC]
    -0x1.75bac97640b5ep+2, // C91 = -5.839525571328378462 [0xC0175BAC97640B5E]
    -0x1.266144954cfadp+3, // CA1 = -9.199373523333486347 [0xC02266144954CFAD]
    0x1.eafdd5e6db189p+1, // C00 = 3.835871446344928604     [0x400EAFDD5E6DB189]
    0x1.00e125c2544a3p+2, // C10 = 4.013741912626900721     [0x40100E125C2544A3]
    0x1.188b1f01bdcfep+2, // C20 = 4.383491279316556799     [0x401188B1F01BDCFE]
    0x1.3e618e63b8efep+2, // C30 = 4.974704358468214238     [0x4013E618E63B8EFE]
    0x1.7589cfc133874p+2, // C40 = 5.836536348984839861     [0x4017589CFC133874]
    0x1.c2f806af55e7fp+2, // C50 = 7.046388312555449396     [0x401C2F806AF55E7F]
    0x1.1751008ea1f7fp+3, // C60 = 8.728637960986587174     [0x4021751008EA1F7F]
    0x1.631d56796c97dp+3, // C70 = 11.09733127322828672     [0x402631D56796C97D]
    0x1.d2156e02b5e38p+3, // C80 = 14.56511593369860691     [0x402D2156E02B5E38]
    0x1.41572f9d3c4b2p+4, // C90 = 20.08378564281583323     [0x40341572F9D3C4B2]
    0x1.ebc1c1400185bp+4, // CA0 = 30.73480343820879668     [0x403EBC1C1400185B]
    0x1.b2fe1f0fd8f23p+6, // An  = 108.7481653667605741     [0x405B2FE1F0FD8F23]
    /* [32; 33] */
    0x1.860b3482be286p+1, // C01 = 3.047216953124293859     [0x400860B3482BE286]
    0x1.7383b5848cc8bp+1, // C11 = 2.902456941339158103     [0x4007383B5848CC8B]
    0x1.4e203b221326p+1,  // C21 = 2.610358611715398069     [0x4004E203B2213260]
    0x1.152a4571c9541p+1, // C31 = 2.16535251670816864      [0x400152A4571C9541]
    0x1.8ed44e3c9752cp+0, // C41 = 1.557927026551008787     [0x3FF8ED44E3C9752C]
    0x1.8badbc5471b7p-1,  // C51 = 0.7728098729211136941    [0x3FE8BADBC5471B70]
    -0x1.b748410d32f03p-3, // C61 = -0.2144932825234421025 [0xBFCB748410D32F03]
    -0x1.71dbb62849839p+0, // C71 = -1.444758782230122884 [0xBFF71DBB62849839]
    -0x1.7ee6e5f68aa49p+1, // C81 = -2.991421456700923631 [0xC007EE6E5F68AA49]
    -0x1.407bbceb2262bp+2, // C91 = -5.007552365897519842 [0xC01407BBCEB2262B]
    -0x1.fb4df8804584p+2, // CA1 = -7.926633954301053109    [0xC01FB4DF88045840]
    0x1.2eb357ef2f64ap+1, // C00 = 2.36484812907182107      [0x4002EB357EF2F64A]
    0x1.3f42c18d9e31ap+1, // C10 = 2.494224733495127744     [0x4003F42C18D9E31A]
    0x1.61dfe3c6a61c2p+1, // C20 = 2.764645072933689995     [0x40061DFE3C6A61C2]
    0x1.99ac1ab88e02dp+1, // C30 = 3.200564708819021664     [0x40099AC1AB88E02D]
    0x1.ebca7ec60f616p+1, // C40 = 3.842117163380645017     [0x400EBCA7EC60F616]
    0x1.3018527e6d2c1p+2, // C50 = 4.751484511817296585     [0x4013018527E6D2C1]
    0x1.81b7ff7729153p+2, // C60 = 6.026855341308265501     [0x40181B7FF7729153]
    0x1.f56a8c4bd34b1p+2, // C70 = 7.834628175781447545     [0x401F56A8C4BD34B1]
    0x1.4fc102edac0a2p+3, // C80 = 10.49231096668546925     [0x4024FC102EDAC0A2]
    0x1.d755d892a44e1p+3, // C90 = 14.72922924653033938     [0x402D755D892A44E1]
    0x1.6e8f39b5a40aap+4, // CA0 = 22.90996714547569724     [0x4036E8F39B5A40AA]
    0x1.559a8b26387ffp+85, // An  = 5.162168947230857006e+25
                           // [0x454559A8B26387FF]
    /* [48; 49] */
    0x1.557caeff79c7p+1,  // C01 = 2.667867541069021797     [0x400557CAEFF79C70]
    0x1.44e995dd6bbeep+1, // C11 = 2.538378460994400676     [0x40044E995DD6BBEE]
    0x1.237216cc8d837p+1, // C21 = 2.276919221762934153     [0x400237216CC8D837]
    0x1.e0ce30a33944bp+0, // C31 = 1.878146209573304359     [0x3FFE0CE30A33944B]
    0x1.553dc30381c6p+0,  // C41 = 1.332973659855063886     [0x3FF553DC30381C60]
    0x1.40f7b6a64b965p-1, // C51 = 0.6268899038312097938    [0x3FE40F7B6A64B965]
    -0x1.0d9170e6409f2p-2, // C61 = -0.2632501259360245927 [0xBFD0D9170E6409F2]
    -0x1.603258ba7c8f1p+0, // C71 = -1.375768228081650557 [0xBFF603258BA7C8F1]
    -0x1.63c14c4c7a983p+1, // C81 = -2.779336488097386759 [0xC0063C14C4C7A983]
    -0x1.2774654231d59p+2, // C91 = -4.61647922005007505 [0xC012774654231D59]
    -0x1.d2854739b37ddp+2, // CA1 = -7.289384657238739074 [0xC01D2854739B37DD]
    0x1.d071612af61a8p+0, // C00 = 1.81423003481732259      [0x3FFD071612AF61A8]
    0x1.ec1c1e000e9fap+0, // C10 = 1.922304034246393289     [0x3FFEC1C1E000E9FA]
    0x1.130d326d5c018p+1, // C20 = 2.148840239904540539     [0x400130D326D5C018]
    0x1.41fc9d154c224p+1, // C30 = 2.515521655463333062     [0x40041FC9D154C224]
    0x1.87624539fe3bbp+1, // C40 = 3.057686475102369794     [0x40087624539FE3BB]
    0x1.ea301e0000833p+1, // C50 = 3.829593420029618667     [0x400EA301E0000833]
    0x1.3aa0fa0f7f764p+2, // C60 = 4.916075244078317752     [0x4013AA0FA0F7F764]
    0x1.9d6a7ef42747p+2,  // C70 = 6.459624994689065147     [0x4019D6A7EF427470]
    0x1.175fdf3c14922p+3, // C80 = 8.730453126282160525     [0x402175FDF3C14922]
    0x1.8b1df225a73c4p+3, // C90 = 12.34740550378945301     [0x4028B1DF225A73C4]
    0x1.3510230845744p+4, // CA0 = 19.3164396594140868      [0x4033510230845744]
    0x1.7ad81c48efd82p+173, // An  = 1.771780453414861323e+52
                            // [0x4AC7AD81C48EFD82]
    /* [64; 65] */
    0x1.3801bbeeb9b8cp+1, // C01 = 2.437552920880483143     [0x4003801BBEEB9B8C]
    0x1.288d5d381ed5p+1,  // C11 = 2.31681409111266845      [0x400288D5D381ED50]
    0x1.09578a1066217p+1, // C21 = 2.072983987815188112     [0x40009578A1066217]
    0x1.b3747412ef63ep+0, // C31 = 1.700995688078719592     [0x3FFB3747412EF63E]
    0x1.313631932823ap+0, // C41 = 1.192233179499579965     [0x3FF313631932823A]
    0x1.10dccc804631ap-1, // C51 = 0.53293456140372375      [0x3FE10DCCC804631A]
    -0x1.32051848d1e32p-2, // C61 = -0.2988475603528711888 [0xBFD32051848D1E32]
    -0x1.56e5a2fb9a81ap+0, // C71 = -1.339441477228780375 [0xBFF56E5A2FB9A81A]
    -0x1.53b35937c8586p+1, // C81 = -2.653910782086941644 [0xC0053B35937C8586]
    -0x1.1823b307542efp+2, // C91 = -4.377178914219924444 [0xC011823B307542EF]
    -0x1.b8ef3d30383e2p+2, // CA1 = -6.889601990785168439 [0xC01B8EF3D30383E2]
    0x1.83fa9590b2cbdp+0, // C00 = 1.515542362047923364     [0x3FF83FA9590B2CBD]
    0x1.9c7a5b213a714p+0, // C10 = 1.611242004024863839     [0x3FF9C7A5B213A714]
    0x1.cfef33015a0dfp+0, // C20 = 1.812243640737555728     [0x3FFCFEF33015A0DF]
    0x1.11bb3ee790d01p+1, // C30 = 2.138526785916497364     [0x40011BB3EE790D01]
    0x1.4fad965965a8ep+1, // C40 = 2.622484964037830046     [0x4004FAD965965A8E]
    0x1.a82158917eee9p+1, // C50 = 3.313517638247379882     [0x400A82158917EEE9]
    0x1.127337215fd7bp+2, // C60 = 4.288282187072918639     [0x401127337215FD7B]
    0x1.6b2dbf1eefd71p+2, // C70 = 5.674667148792068794     [0x4016B2DBF1EEFD71]
    0x1.edb288b408ff7p+2, // C80 = 7.714021850410055059     [0x401EDB288B408FF7]
    0x1.5ea5e1baca442p+3, // C90 = 10.95774923784677313     [0x4025EA5E1BACA442]
    0x1.131368b7f9103p+4, // CA0 = 17.19223853935637791     [0x403131368B7F9103]
    0x1.6ff95d0dcdc22p+268, // An  = 6.817357929830277882e+80
                            // [0x50B6FF95D0DCDC22]
    /* [80; 81] */
    0x1.238ffe9d32686p+1, // C01 = 2.277831866031934105     [0x400238FFE9D32686]
    0x1.14dcf5707c8b5p+1, // C11 = 2.162993125844048681     [0x40014DCF5707C8B5]
    0x1.ee5ac705c84f8p+0, // C21 = 1.931072653686269902     [0x3FFEE5AC705C84F8]
    0x1.93c5c9f367648p+0, // C31 = 1.577236768650793763     [0x3FF93C5C9F367648]
    0x1.17dfdead5c0bbp+0, // C41 = 1.093259732555494024     [0x3FF17DFDEAD5C0BB]
    0x1.dd2ccd56d430bp-2, // C51 = 0.4659912189718375397    [0x3FDDD2CCD56D430B]
    -0x1.4d5f5eb9df5dcp-2, // C61 = -0.325559120268591462 [0xBFD4D5F5EB9DF5DC]
    -0x1.50efd5fde3f39p+0, // C71 = -1.316159605488748108 [0xBFF50EFD5FDE3F39]
    -0x1.48b7a866091d9p+1, // C81 = -2.568104791471551795 [0xC0048B7A866091D9]
    -0x1.0d7a2c6d6ee9ap+2, // C91 = -4.210581881397638782 [0xC010D7A2C6D6EE9A]
    -0x1.a6e437c00dc21p+2, // CA1 = -6.607679307510836431 [0xC01A6E437C00DC21]
    0x1.52fdf575230a8p+0, // C00 = 1.324187604043837396     [0x3FF52FDF575230A8]
    0x1.695e15d767f2fp+0, // C10 = 1.41159187802254471      [0x3FF695E15D767F2F]
    0x1.987048326d0a3p+0, // C20 = 1.595463287650127127     [0x3FF987048326D0A3]
    0x1.e5054c4c200ddp+0, // C30 = 1.89461209162259725      [0x3FFE5054C4C200DD]
    0x1.2b71d3b637a8bp+1, // C40 = 2.339411224339817519     [0x4002B71D3B637A8B]
    0x1.7ceb2e81c76b5p+1, // C50 = 2.975927174919741791     [0x4007CEB2E81C76B5]
    0x1.f006f6360fa4fp+1, // C60 = 3.875212456128145799     [0x400F006F6360FA4F]
    0x1.49eded8fe12fp+2,  // C70 = 5.155146971232838382     [0x40149EDED8FE12F0]
    0x1.c2632c04677ep+2,  // C80 = 7.037302974988023152     [0x401C2632C04677E0]
    0x1.40dbe87a72457p+3, // C90 = 10.02684425273234403     [0x40240DBE87A72457]
    0x1.f84cd564e4ab4p+3, // CA0 = 15.75937909799299774     [0x402F84CD564E4AB4]
    0x1.b3e96025db5fep+368, // An
                            // = 1.023757009764010667e+111[0x56FB3E96025DB5FE]
    /* [96; 97] */
    0x1.143f462e366b4p+1, // C01 = 2.158180973591550256     [0x400143F462E366B4]
    0x1.061ab71087122p+1, // C11 = 2.047690280027510745     [0x400061AB71087122]
    0x1.d3161e2c2ccefp+0, // C21 = 1.824556241784019539     [0x3FFD3161E2C2CCEF]
    0x1.7bf068febecfbp+0, // C31 = 1.484137117570581976     [0x3FF7BF068FEBECFB]
    0x1.04be2bbc69d81p+0, // C41 = 1.018526776790025901     [0x3FF04BE2BBC69D81]
    0x1.a908c30d71559p-2, // C51 = 0.4150724865870266123    [0x3FDA908C30D71559]
    -0x1.62c2127562923p-2, // C61 = -0.3464434513231415269 [0xBFD62C2127562923]
    -0x1.4caeb1dd49002p+0, // C71 = -1.299540630840966049 [0xBFF4CAEB1DD49002]
    -0x1.408d6cea6df46p+1, // C81 = -2.504315962283643948 [0xC00408D6CEA6DF46]
    -0x1.0577d837973d2p+2, // C91 = -4.08543973377978098 [0xC010577D837973D2]
    -0x1.993bc56477523p+2, // CA1 = -6.394273136245177902 [0xC01993BC56477523]
    0x1.30762c2915efp+0,  // C00 = 1.189303169279749994     [0x3FF30762C2915EF0]
    0x1.4549b1dd21fddp+0, // C10 = 1.27065574311017504      [0x3FF4549B1DD21FDD]
    0x1.71288df46ffe9p+0, // C20 = 1.442025062729948681     [0x3FF71288DF46FFE9]
    0x1.b8aae4c9090bfp+0, // C30 = 1.721357630806821026     [0x3FFB8AAE4C9090BF]
    0x1.119abafb72ec2p+1, // C40 = 2.137534497049132121     [0x400119ABAFB72EC2]
    0x1.5df9099b51c3dp+1, // C50 = 2.734162522162959963     [0x4005DF9099B51C3D]
    0x1.ca012221b6be8p+1, // C60 = 3.578159586393201863     [0x400CA012221B6BE8]
    0x1.31ebe90e71cf6p+2, // C70 = 4.78002382670023529      [0x40131EBE90E71CF6]
    0x1.a2fdcc9c43ffcp+2, // C80 = 6.546740677455087365     [0x401A2FDCC9C43FFC]
    0x1.2b2def4fc6c29p+3, // C90 = 9.349357276738446743     [0x4022B2DEF4FC6C29]
    0x1.d6c9b7ca1c24cp+3, // CA0 = 14.71212377047154263     [0x402D6C9B7CA1C24C]
    0x1.4120a61af3c66p+473, // An  = 3.059324388111218493e+142
                            // [0x5D84120A61AF3C60]
    /* [112; 113] */
    0x1.082e131fac559p+1, // C01 = 2.063906088319225507     [0x400082E131FAC559]
    0x1.f4f12c90d6ef4p+0, // C11 = 1.956805024491527156     [0x3FFF4F12C90D6EF4]
    0x1.bd9316295ebddp+0, // C21 = 1.7405256129265958       [0x3FFBD9316295EBDD]
    0x1.691c2a78d7e17p+0, // C31 = 1.410586027624850081     [0x3FF691C2A78D7E17]
    0x1.eb2f6071792afp-1, // C41 = 0.959345830775950037     [0x3FEEB2F6071792AF]
    0x1.7f8de1109482bp-2, // C51 = 0.3745646635408116931    [0x3FD7F8DE1109482B]
    -0x1.740eb427257b7p-2, // C61 = -0.3633373402493548787 [0xBFD740EB427257B7]
    -0x1.496f3c036899p+0, // C71 = -1.286853552658246969    [0xBFF496F3C0368990]
    -0x1.3a25f4ee237f5p+1, // C81 = -2.454283348349231009 [0xC003A25F4EE237F5]
    -0x1.fe4988f91c211p+1, // C91 = -3.986619111667331072 [0xC00FE4988F91C211]
    -0x1.8e64bd90cac4cp+2, // CA1 = -6.224898711584568645 [0xC018E64BD90CAC4C]
    0x1.168fb9b3c2c21p+0, // C00 = 1.088130575549037582     [0x3FF168FB9B3C2C21]
    0x1.2a3162236e1f1p+0, // C10 = 1.16481603016847779      [0x3FF2A3162236E1F1]
    0x1.539832e40ded1p+0, // C20 = 1.326541119267791169     [0x3FF539832E40DED1]
    0x1.9730529981af5p+0, // C30 = 1.590581095196907713     [0x3FF9730529981AF5]
    0x1.fc12927d522cap+0, // C40 = 1.984658389659829769     [0x3FFFC12927D522CA]
    0x1.4675d44542d65p+1, // C50 = 2.550470861262214672     [0x4004675D44542D65]
    0x1.ad057277afaf3p+1, // C60 = 3.351728733482894906     [0x400AD057277AFAF3]
    0x1.1f90a98dcdc62p+2, // C70 = 4.493204487304952366     [0x4011F90A98DCDC62]
    0x1.8aea9b0515ffcp+2, // C80 = 6.170569186178905596     [0x4018AEA9B0515FFC]
    0x1.1a8242d672e27p+3, // C90 = 8.828401011319398251     [0x4021A8242D672E27]
    0x1.bcf1ae8d09023p+3, // CA0 = 13.90450217767653918     [0x402BCF1AE8D09023]
    0x1.71452f45b80c6p+581, // An
                            // = 1.141646997748512783e+175[0x64471452F45B80C6]
    /* [128; 129] */
    0x1.fca7a068844e2p+0, // C01 = 1.986932778851319359     [0x3FFFCA7A068844E2]
    0x1.e1f0af75d8332p+0, // C11 = 1.882578817625653489     [0x3FFE1F0AF75D8332]
    0x1.abfedae1b7786p+0, // C21 = 1.671857528788196046     [0x3FFABFEDAE1B7786]
    0x1.59b54be931162p+0, // C31 = 1.35042261546285447      [0x3FF59B54BE931162]
    0x1.d25c0d3f15aebp-1, // C41 = 0.9108585490710913168    [0x3FED25C0D3F15AEB]
    0x1.5d76c8c8b7b28p-2, // C51 = 0.3412734386928826247    [0x3FD5D76C8C8B7B28]
    -0x1.826e8c493e737p-2, // C61 = -0.3773748321251448323 [0xBFD826E8C493E737]
    -0x1.46d6e9eb6c52fp+0, // C71 = -1.276716823555329006 [0xBFF46D6E9EB6C52F]
    -0x1.34ef0c1be301fp+1, // C81 = -2.413545144669128728 [0xC0034EF0C1BE301F]
    -0x1.f3f08afb08821p+1, // C91 = -3.905778286528417187 [0xC00F3F08AFB08821]
    -0x1.857e898c93d71p+2, // CA1 = -6.085848223955836467 [0xC01857E898C93D71]
    0x1.0244bce91379cp+0, // C00 = 1.008861357596720687     [0x3FF0244BCE91379C]
    0x1.14f114139c1dep+0, // C10 = 1.081803564822727548     [0x3FF14F114139C1DE]
    0x1.3c5cdf0da363bp+0, // C20 = 1.235792103604935077     [0x3FF3C5CDF0DA363B]
    0x1.7cd0cd6439edap+0, // C30 = 1.487561070412928732     [0x3FF7CD0CD6439EDA]
    0x1.dd2884dd8290dp+0, // C40 = 1.863899520949931388     [0x3FFDD2884DD8290D]
    0x1.33d601e23b2bcp+1, // C50 = 2.40496848627512172      [0x40033D601E23B2BC]
    0x1.9600b97223d6fp+1, // C60 = 3.1718971068691455       [0x4009600B97223D6F]
    0x1.10f3545fbfe74p+2, // C70 = 4.264851659303179332     [0x40110F3545FBFE74]
    0x1.77b4a715a8196p+2, // C80 = 5.870401164197025068     [0x40177B4A715A8196]
    0x1.0d2db093a6237p+3, // C90 = 8.411827362419996845     [0x4020D2DB093A6237]
    0x1.a83c380829023p+3, // CA0 = 13.257350936830397       [0x402A83C380829023]
    0x1.d743487a69dccp+692, // An
                            // = 3.782511143738250114e+208[0x6B3D743487A69DCC]
    /* [144; 145] */
    0x1.ec227c12e654dp+0, // C01 = 1.92240119420451383      [0x3FFEC227C12E654D]
    0x1.d201a8b26360ap+0, // C11 = 1.820337813903622592     [0x3FFD201A8B26360A]
    0x1.9d3fab1ee8bd7p+0, // C21 = 1.614252753301789189     [0x3FF9D3FAB1EE8BD7]
    0x1.4cc752c6d90cap+0, // C31 = 1.299916432902181729     [0x3FF4CC752C6D90CA]
    0x1.bd7ea62b12d9dp-1, // C41 = 0.870106880910885061     [0x3FEBD7EA62B12D9D]
    0x1.40bfd482bd7afp-2, // C51 = 0.3132317738339862045    [0x3FD40BFD482BD7AF]
    -0x1.8ea1e2257b388p-2, // C61 = -0.3892894111038960325 [0xBFD8EA1E2257B388]
    -0x1.44b2746b8a38ep+0, // C71 = -1.268348003630510146 [0xBFF44B2746B8A38E]
    -0x1.3091c3b4ff682p+1, // C81 = -2.379448378923201268 [0xC003091C3B4FF682]
    -0x1.eb3fdd0f4aeb3p+1, // C91 = -3.837886459796061356 [0xC00EB3FDD0F4AEB3]
    -0x1.7e00407213c8bp+2, // CA1 = -5.968765365031809544 [0xC017E00407213C8B]
    0x1.e3b1c2ae40025p-1, // C00 = 0.9447155797679483991    [0x3FEE3B1C2AE40025]
    0x1.03baa72f9611bp+0, // C10 = 1.014566849820829786     [0x3FF03BAA72F9611B]
    0x1.2983b84e3516fp+0, // C20 = 1.162166136830901086     [0x3FF2983B84E3516F]
    0x1.675f5f36baddcp+0, // C30 = 1.403799010144999748     [0x3FF675F5F36BADDC]
    0x1.c3f698af95038p+0, // C40 = 1.765481512896270644     [0x3FFC3F698AF95038]
    0x1.249f044af1554p+1, // C50 = 2.286102806656950648     [0x400249F044AF1554]
    0x1.8327f5f3a766dp+1, // C60 = 3.024657005283009514     [0x4008327F5F3A766D]
    0x1.04f5c74d6d1dap+2, // C70 = 4.077501130687290143     [0x40104F5C74D6D1DA]
    0x1.67ea544fdda48p+2, // C80 = 5.623677328103958928     [0x40167EA544FDDA48]
    0x1.02340f3f6792ap+3, // C90 = 8.068854926907608416     [0x40202340F3F6792A]
    0x1.972882a8e16dfp+3, // CA0 = 12.72369511589516655     [0x402972882A8E16DF]
    0x1.01404c1b967d9p+807, // An  = 8.57677228252869865e+242
                            // [0x72601404C1B967D9]
    /* [160; 161] */
    0x1.ddffca24558e6p+0, // C01 = 1.867184289808898168     [0x3FFDDFFCA24558E6]
    0x1.c45ee176a55abp+0, // C11 = 1.767072764841354227     [0x3FFC45EE176A55AB]
    0x1.909fe043fd328p+0, // C21 = 1.564939514731301884     [0x3FF909FE043FD328]
    0x1.41b44411d6b65p+0, // C31 = 1.256656889300495239     [0x3FF41B44411D6B65]
    0x1.ab9ba5f9f15a6p-1, // C41 = 0.8351718776327075044    [0x3FEAB9BA5F9F15A6]
    0x1.2817d8db9b023p-2, // C51 = 0.2891534694706498487    [0x3FD2817D8DB9B023]
    -0x1.992a9de9fa9d7p-2, // C61 = -0.3995766328868319284 [0xBFD992A9DE9FA9D7]
    -0x1.42e25c5351809p+0, // C71 = -1.261266489354513576 [0xBFF42E25C5351809]
    -0x1.2cd6abbf6486ap+1, // C81 = -2.350301235631424568 [0xC002CD6ABBF6486A]
    -0x1.e3cd45739dba2p+1, // C91 = -3.779701882794555878 [0xC00E3CD45739DBA2]
    -0x1.7790f79806202p+2, // CA1 = -5.868223093472353824 [0xC017790F79806202]
    0x1.c872e9cda5031p-1, // C00 = 0.8915017188578585605    [0x3FEC872E9CDA5031]
    0x1.eae054a2d6c99p-1, // C10 = 0.9587427567270366824    [0x3FEEAE054A2D6C99]
    0x1.19d7963665c33p+0, // C20 = 1.100945843010560976     [0x3FF19D7963665C33]
    0x1.55821e0b76efcp+0, // C30 = 1.33401668338677748      [0x3FF55821E0B76EFC]
    0x1.aeeddf95640bap+0, // C40 = 1.683317159626354087     [0x3FFAEEDDF95640BA]
    0x1.17e48123d1166p+1, // C50 = 2.186660902489324876     [0x40017E48123D1166]
    0x1.735bc0230fccap+1, // C60 = 2.901237504119852595     [0x400735BC0230FCCA]
    0x1.f5c8a11edcec5p+1, // C70 = 3.920185222692313065     [0x400F5C8A11EDCEC5]
    0x1.5aa2ccd15adaep+2, // C80 = 5.416186527679583307     [0x4015AA2CCD15ADAE]
    0x1.f1ebfe2e22a9fp+2, // C90 = 7.780028863004786466     [0x401F1EBFE2E22A9F]
    0x1.88c26424d1322p+3, // CA0 = 12.27372939291814546     [0x40288C26424D1322]
    0x1.876d27e68961dp+923, // An
                            // = 1.084166877864065408e+278[0x79A876D27E68961D]
    /* [176; 177] */
    0x1.d1b4c28233024p+0, // C01 = 1.819164425646712324     [0x3FFD1B4C28233024]
    0x1.b882bc0511644p+0, // C11 = 1.720744849431313561     [0x3FFB882BC0511644]
    0x1.85a4487c89d93p+0, // C21 = 1.52203801193704602      [0x3FF85A4487C89D93]
    0x1.3810ccc96a60fp+0, // C31 = 1.219006346868244384     [0x3FF3810CCC96A60F]
    0x1.9c07af41168c2p-1, // C41 = 0.8047461287453858692    [0x3FE9C07AF41168C2]
    0x1.1297c775556cdp-2, // C51 = 0.2681571164479066849    [0x3FD1297C775556CD]
    -0x1.a263dbae34e06p-2, // C61 = -0.4085840535237540427 [0xBFDA263DBAE34E06]
    -0x1.41520f9bb7c21p+0, // C71 = -1.255158401028673376 [0xBFF41520F9BB7C21]
    -0x1.299879b38ecb9p+1, // C81 = -2.324965679812148966 [0xC00299879B38ECB9]
    -0x1.dd50be46f02cp+1, // C91 = -3.729026589048174856    [0xC00DD50BE46F02C0]
    -0x1.71f4096047c88p+2, // CA1 = -5.780519813560324849 [0xC0171F4096047C88]
    0x1.b1658fb3f97bp-1,  // C00 = 0.8464779765315508797    [0x3FEB1658FB3F97B0]
    0x1.d2acdd2755cfcp-1, // C10 = 0.9114750967518427949    [0x3FED2ACDD2755CFC]
    0x1.0c8dcf1557dep+0,  // C20 = 1.049038832388639264     [0x3FF0C8DCF1557DE0]
    0x1.4655db6c5c608p+0, // C30 = 1.274747575717244175     [0x3FF4655DB6C5C608]
    0x1.9d07c80501ddp+0,  // C40 = 1.613399983618240441     [0x3FF9D07C80501DD0]
    0x1.0d0a8bd68544bp+1, // C50 = 2.101884345766426687     [0x4000D0A8BD68544B]
    0x1.65de0959d1742p+1, // C60 = 2.795838517052204431     [0x40065DE0959D1742]
    0x1.e48fa45845cd2p+1, // C70 = 3.785633605100522381     [0x400E48FA45845CD2]
    0x1.4f435727c7303p+2, // C80 = 5.238485134931354814     [0x4014F435727C7303]
    0x1.e212a2e67c58dp+2, // C90 = 7.532387471278878088     [0x401E212A2E67C58D]
    0x1.7c66b37f5a25p+3,  // CA0 = 11.8875367629381401      [0x4027C66B37F5A250]
    0x1p+0,               // An  = 1                        [0x3FF0000000000000]
};
/* sin(pi*x)/pi*/
static const double __dtgamma_ep_sinc_tab[] = {
    -0x1.a51a6625307d3p+0,  // S3 = -1.644934066848226406 [0xBFFA51A6625307D3]
    0x1.9f9cb402bc46cp-1,   // S5 = 0.811742425283353608 [0x3FE9F9CB402BC46C]
    -0x1.86a8e4720db67p-3,  // S7 = -0.1907518241220842181 [0xBFC86A8E4720DB67]
    0x1.ac6805cf350a6p-6,   // S9 = 0.02614784781765479987 [0x3F9AC6805CF350A6]
    -0x1.33816aa4607abp-9,  // S11 = -0.002346081035455823468
                            // [0xBF633816AA4607AB]
    0x1.374719fab3915p-13,  // S13 = 0.0001484287930310709965
                            // [0x3F2374719FAB3915]
    -0x1.d42498d1ce099p-18, // S15 = -6.975873661656380708e-06
                            // [0xBEDD42498D1CE099]
    0x1.0fc992ff39e13p-22,  // S17 = 2.531217404137027415e-07
                            // [0x3E90FC992FF39E13]
    -0x1.f5f9d970ca6dfp-28, // S19 = -7.304711822217774969e-09
                            // [0xBE3F5F9D970CA6DF]
    0x1.79788684225eap-33,  // S21 = 1.716538474982143217e-10
                            // [0x3DE79788684225EA]
};
/* [1.0;1.25]*/
static const double __dtgamma_ep_a100_tab[] = {
    0x1p+0,                // A0 = 1                        [0x3FF0000000000000]
    -0x1.2788cfc6fb619p-1, // A1 = -0.5772156649015328655   [0xBFE2788CFC6FB619]
    0x1.fa658c23b151p-1,   // A2 = 0.9890559953279609573    [0x3FEFA658C23B1510]
    -0x1.d0a118f32141bp-1, // A3 = -0.9074790760793151057   [0xBFED0A118F32141B]
    0x1.f6a5105412b63p-1,  // A4 = 0.9817280867222105689    [0x3FEF6A5105412B63]
    -0x1.f6c80e99da4cbp-1, // A5 = -0.9819950640495788141   [0xBFEF6C80E99DA4CB]
    0x1.fc7e05cbdb6dp-1,   // A6 = 0.9931489764546430621    [0x3FEFC7E05CBDB6D0]
    -0x1.fdf395dc596b7p-1, // A7 = -0.9959990340691636801   [0xBFEFDF395DC596B7]
    0x1.ff02a77a85ce3p-1,  // A8 = 0.9980671250404181682    [0x3FEFF02A77A85CE3]
    -0x1.ff4bde72b2114p-1, // A9 = -0.9986257090577361772   [0xBFEFF4BDE72B2114]
    0x1.fe2e76fccdfa4p-1,  // A10 = 0.9964482482528080887   [0x3FEFE2E76FCCDFA4]
    -0x1.f6e7a62f260d1p-1, // A11 = -0.9822360928110588363  [0xBFEF6E7A62F260D1]
    0x1.d9efbbbfa4f92p-1,  // A12 = 0.9256571456376028184   [0x3FED9EFBBBFA4F92]
    -0x1.89bdae38f05efp-1, // A13 = -0.7690252727990677384  [0xBFE89BDAE38F05EF]
    0x1.eb6a827f2bc38p-2,  // A14 = 0.4798984899634350931   [0x3FDEB6A827F2BC38]
    -0x1.480aa6160cfdap-3, // A15 = -0.1601765609244683586  [0xBFC480AA6160CFDA]
};
/* [1.25;1.5]*/
static const double __dtgamma_ep_a125_tab[] = {
    0x1.c56dc82a74aefp-1, // A0 = 0.8856031944108887499     [0x3FEC56DC82A74AEF]
    0x1.e9f1f78f8193ap-57, // A1 = 1.327999081482368164e-17 [0x3C6E9F1F78F8193A]
    0x1.b6c53f7377b84p-2, // A2 = 0.4284868158555854567     [0x3FDB6C53F7377B84]
    -0x1.0bae9f40c7d03p-3, // A3 = -0.1307041589397855696 [0xBFC0BAE9F40C7D03]
    0x1.4981175e14ab3p-3, // A4 = 0.160890753325096364      [0x3FC4981175E14AB3]
    -0x1.79f77aaf0ca7p-4, // A5 = -0.09227703021387312354   [0xBFB79F77AAF0CA70]
    0x1.1e97bd1106464p-4, // A6 = 0.06996892789938297108    [0x3FB1E97BD1106464]
    -0x1.8071cdc64c2adp-5, // A7 = -0.04692926588393695614 [0xBFA8071CDC64C2AD]
    0x1.0b44c58df1fdep-5, // A8 = 0.03262556633814005236    [0x3FA0B44C58DF1FDE]
    -0x1.6df2396b86e23p-6, // A9 = -0.02233558280395987403 [0xBF96DF2396B86E23]
    0x1.f628021f66d2fp-7, // A10 = 0.01532459357877388566   [0x3F8F628021F66D2F]
    -0x1.591fabc23edddp-7, // A11 = -0.01053233992251184116 [0xBF8591FABC23EDDD]
    0x1.c6cdb65f64f7p-8, // A12 = 0.006939751648491795932  [0x3F7C6CDB65F64F70]
    -0x1.8a7f06c8ba8b3p-8, // A13 = -0.006019534260057013246
                           // [0xBF78A7F06C8BA8B3]
    0x1.d4dbd24bcdd72p-12, // A14 = 0.000447138478864648266 [0x3F3D4DBD24BCDD72]
    -0x1.a070124800c33p-8, // A15 = -0.006354336226652756818
                           // [0xBF7A070124800C33]
};
/* [1.5;2.0]*/
static const double __dtgamma_ep_a150_tab[] = {
    0x1.ffffe2f03fb7ep-1,  // A0 = 0.9999991338989813183 [0x3FEFFFFE2F03FB7E0]
    -0x1.27861fb2f03b7p-1, // A1 = -0.5771951585703537502 [0xBFE27861FB2F03B70]
    0x1.fa479b2e8368cp-1,  // A2 = 0.9888275617865516232 [0x3FEFA479B2E8368C0]
    -0x1.cfd05a18c2752p-1, // A3 = -0.9058864741539116583 [0xBFECFD05A18C27520]
    0x1.f2a6a04f8363fp-1,  // A4 = 0.9739275071203293832 [0x3FEF2A6A04F8363F0]
    -0x1.e8262550ae5dap-1, // A5 = -0.9534160290706339413 [0xBFEE8262550AE5DA0]
    0x1.d2c4a94480f4cp-1,  // A6 = 0.911656655895492829 [0x3FED2C4A94480F4C0]
    -0x1.9eaf26255afd6p-1, // A7 = -0.809930030879963736 [0xBFE9EAF26255AFD60]
    0x1.4cc0be4e43ef1p-1,  // A8 = 0.6499080153025414974 [0x3FE4CC0BE4E43EF10]
    -0x1.cfe46dd7f73efp-2, // A9 = -0.4530198252739969234 [0xBFDCFE46DD7F73EF0]
    0x1.0e88a46be8d12p-2,  // A10 = 0.2641931238997169418 [0x3FD0E88A46BE8D120]
    -0x1.fb09e6b6e19b3p-4, // A11 = -0.1237887394218891018 [0xBFBFB09E6B6E19B30]
    0x1.6babd3ca209f5p-5,  // A12 = 0.04439345708034984211 [0x3FA6BABD3CA209F50]
    -0x1.74ab5343f9107p-7, // A13 = -0.01137296262454202782
                           // [0xBF874AB5343F91070]
    0x1.e3eeba78ac397p-10, // A14 = 0.001846056109723747665
                           // [0x3F5E3EEBA78AC3970]
    -0x1.2a67f3a6e4769p-13, // A15 = -0.0001422910983959705538
                            // [0xBF22A67F3A6E47690]
};

inline int __devicelib_imf_internal_dtgamma(const double *a, double *r)
{
  int nRet = 0;
  __dtgamma_ep_mp_double mp_p, mp_r, mp_a, mp_d, mp_s;
  double tv;
  double s, p, pr, r2, r1, curabsx, y, res = 0.0;
  double x = (*a), absx = x;
  double dix, diabsx_n, diabsx_t;
  int t, i, j, irsign, hx, lx;
  int ix, iabsx_n, iabsx_t, ixsign, ixexp;
  const double *dA;
  /* negative values underflow range */
  const double neg_underflow[2] = {185.0, -185.0};
  /* negative values "half" overflow range - multiply by 1/An */
  const double neg_half_overflow[2] = {176.0, -176.0};
  /* overflow boundary */
  const double overflow_bound =
      0x1.573fae561f648p+7; // 171.6243769563027399 [0x406573FAE561F648]
  /* point of local minimum */
  const double local_min =
      0x1.762d86356be4p+0; // 1.461632144968362468 [0x3FF762D86356BE40]
  /* right shifter */
  const double two_52h = 0x1p+52; // 4503599627370496.0 [0x4330000000000000]
  /*[176; 177] 1/An for negatives*/
  const double a176_inv =
      0x0.00000ba5d5869p-1022; // 1.544785004074392221e-314 [0x00000000BA5D5869]
  const uint64_t _infs[] = {0x7ff0000000000000, 0xfff0000000000000};
  const double zeros[] = {0x0p+0, -0x0p+0};
  const double ones[] = {1.0, -1.0};
  const double own_large_value_64[] = {
      0x1p+1000, // +2^1000 =  1.071508607186267321e+301 [0x7E70000000000000]
      -0x1p+1000 // -2^1000 = -1.071508607186267321e+301 [0xFE70000000000000]
  };
  const double own_small_value_64[] = {
      0x1p-1000, // +2^(-1000) =  9.33263618503218879e-302 [0x0170000000000000]
      -0x1p-1000 // -2^(-1000) = -9.33263618503218879e-302 [0x8170000000000000]
  };
  /* get arg sign */
  ixsign = ((as_ulong(x) >> 63) & 1);
  /* get arg exponent */
  ixexp = ((as_ulong(x) >> 52) & 0x7FF);
  if (ixexp != 0x7FF) /* normal values */
  {
    /* create absolute value */
    (absx = as_double(as_ulong(absx) & 0x7fffffffffffffff));
    /* high part of x */
    hx = ((uint32_t)(as_ulong(absx) >> 32));
    /* low part of x */
    lx = ((uint32_t)(as_ulong(absx)));
    /* if x == 0 - zero divide exception */
    if (x == zeros[0]) {
      res = (ones[(ixsign)] / zeros[0]);
      nRet = 2;
      (*r) = res;
      return nRet;
    }
    /* if |x| < denorm_overflow */
    if ((hx < 0x00040000) || ((hx == 0x00040000) && (lx == 0x0))) {
      res = (own_large_value_64[(ixsign)] *
             own_large_value_64[0]); /* raise overflow */
      nRet = 3;
      (*r) = res;
      return nRet;
    }
    /* singularity at negative integer points */
    if (ixsign) {
      /* if x >= 2^52 - only integer values */
      if (ixexp >= 0x00000433) {
        res = (zeros[(0)] / zeros[0]);
        nRet = 1;
        (*r) = res;
        return nRet;
      } else {
        /* get integer value of arg (truncated) */
        tv = absx + two_52h;
        diabsx_t = (tv - two_52h);
        iabsx_t = *((int *)(&tv));
        if (diabsx_t > absx) {
          diabsx_t -= 1.0;
          iabsx_t -= 1;
        }
      }
      /* if arg - integer then singularity */
      if (absx == diabsx_t) {
        res = (zeros[(0)] / zeros[0]);
        nRet = 1;
        (*r) = res;
        return nRet;
      }
      /* if arg < -185.0 then underflow (values rounded to zero) */
      if (x < neg_underflow[1]) {
        res = (own_small_value_64[((~iabsx_t) & 1)] *
               own_small_value_64[0]); /* raise underflow and inexact */
        (*r) = res;
        return nRet;
      }
    }
    /* big positive values overflow domain (res rounded to INF) */
    if (x >= overflow_bound) {
      res = (own_large_value_64[(0)] *
             own_large_value_64[0]); /* raise overflow and inexact */
      nRet = 3;
      (*r) = res;
      return nRet;
    }
    /* compute sin(Pi*x)/x for negative values */
    if (ixsign) {
      /* get rounded to nearest abs arg */
      tv = absx + two_52h;
      diabsx_n = (tv - two_52h);
      iabsx_n = *((int *)(&tv));
      r1 = absx - diabsx_n; /* reduced argument */
      (r1 = as_double(as_ulong(r1) & 0x7fffffffffffffff)); /* remove sign */
      r2 = (r1 * r1);                                      /* r1^2 */
      dA = __dtgamma_ep_sinc_tab;
      /* Tailor series */
      s = (((((((((((dA[9]) * r2 + dA[8]) * r2 + dA[7]) * r2 + dA[6]) * r2 +
                 dA[5]) *
                    r2 +
                dA[4]) *
                   r2 +
               dA[3]) *
                  r2 +
              dA[2]) *
                 r2 +
             dA[1]) *
                r2 +
            dA[0]) *
               r2 +
           1.0) *
          r1;
    }
    /* get truncated integer argument */
    tv = (absx + two_52h);
    diabsx_t = (tv - two_52h);
    iabsx_t = *((int *)(&tv));
    if (diabsx_t > absx) {
      diabsx_t -= 1.0;
      iabsx_t -= 1;
    }
    /* get result sign */
    irsign = ((iabsx_t + 1) & 1);
    /* if x > 2.0 - simple polynomials */
    if (absx >= 2.0) {
      t = iabsx_t & (~0xF); /* index of table of coefficient */
      i = iabsx_t & (0xF);  /* used in recursive formula */
      if (iabsx_t < 16)     /* for 2 <= x < 16 - shift index*/
        i = i - 2;
      r1 = (absx - diabsx_t); /* reduced argument */
      /* factorized polynomial */
      size_t a_idx = (t + (t >> 2) + (t >> 3) + (t >> 4));
      dA = &(__dtgamma_ep_a_tab[a_idx]);
      __dtgamma_ep_mp_double mp_p0 = __dtgamma_ep_mp_double_init_h(dA[22]);
      __dtgamma_ep_mp_double mp_p1 =
          __dtgamma_ep_mp_double_init_h((dA[0] + r1) * r1 + dA[11]);
      __dtgamma_ep_mp_double mp_p2 =
          __dtgamma_ep_mp_double_init_h((dA[1] + r1) * r1 + dA[12]);
      __dtgamma_ep_mp_double mp_p3 =
          __dtgamma_ep_mp_double_init_h((dA[2] + r1) * r1 + dA[13]);
      __dtgamma_ep_mp_double mp_p4 = __dtgamma_ep_mp_double_add_h(
          __dtgamma_ep_mp_double_init_h(dA[3]), r1);
      mp_p4 = __dtgamma_ep_mp_double_mul_h(mp_p4, r1);
      mp_p4 = __dtgamma_ep_mp_double_add_h(mp_p4, dA[14]);
      __dtgamma_ep_mp_double mp_p5 = __dtgamma_ep_mp_double_add_h(
          __dtgamma_ep_mp_double_init_h(dA[4]), r1);
      mp_p5 = __dtgamma_ep_mp_double_mul_h(mp_p5, r1);
      mp_p5 = __dtgamma_ep_mp_double_add_h(mp_p5, dA[15]);
      __dtgamma_ep_mp_double mp_p6 = __dtgamma_ep_mp_double_add_h(
          __dtgamma_ep_mp_double_init_h(dA[5]), r1);
      mp_p6 = __dtgamma_ep_mp_double_mul_h(mp_p6, r1);
      mp_p6 = __dtgamma_ep_mp_double_add_h(mp_p6, dA[16]);
      __dtgamma_ep_mp_double mp_p7 = __dtgamma_ep_mp_double_add_h(
          __dtgamma_ep_mp_double_init_h(dA[6]), r1);
      mp_p7 = __dtgamma_ep_mp_double_mul_h(mp_p7, r1);
      mp_p7 = __dtgamma_ep_mp_double_add_h(mp_p7, dA[17]);
      __dtgamma_ep_mp_double mp_p8 = __dtgamma_ep_mp_double_add_h(
          __dtgamma_ep_mp_double_init_h(dA[7]), r1);
      mp_p8 = __dtgamma_ep_mp_double_mul_h(mp_p8, r1);
      mp_p8 = __dtgamma_ep_mp_double_add_h(mp_p8, dA[18]);
      __dtgamma_ep_mp_double mp_p9 = __dtgamma_ep_mp_double_add_h(
          __dtgamma_ep_mp_double_init_h(dA[8]), r1);
      mp_p9 = __dtgamma_ep_mp_double_mul_h(mp_p9, r1);
      mp_p9 = __dtgamma_ep_mp_double_add_h(mp_p9, dA[19]);
      __dtgamma_ep_mp_double mp_p10 = __dtgamma_ep_mp_double_add_h(
          __dtgamma_ep_mp_double_init_h(dA[9]), r1);
      mp_p10 = __dtgamma_ep_mp_double_mul_h(mp_p10, r1);
      mp_p10 = __dtgamma_ep_mp_double_add_h(mp_p10, dA[20]);
      __dtgamma_ep_mp_double mp_p11 = __dtgamma_ep_mp_double_add_h(
          __dtgamma_ep_mp_double_init_h(dA[10]), r1);
      mp_p11 = __dtgamma_ep_mp_double_mul_h(mp_p11, r1);
      mp_p11 = __dtgamma_ep_mp_double_add_h(mp_p11, dA[21]);
      mp_p = __dtgamma_ep_mp_double_mul(mp_p1, mp_p2);
      mp_p = __dtgamma_ep_mp_double_mul(mp_p, mp_p3);
      mp_p = __dtgamma_ep_mp_double_mul(mp_p, mp_p4);
      mp_p = __dtgamma_ep_mp_double_mul(mp_p, mp_p5);
      mp_p = __dtgamma_ep_mp_double_mul(mp_p, mp_p6);
      mp_p = __dtgamma_ep_mp_double_mul(mp_p, mp_p7);
      mp_p = __dtgamma_ep_mp_double_mul(mp_p, mp_p8);
      mp_p = __dtgamma_ep_mp_double_mul(mp_p, mp_p9);
      mp_p = __dtgamma_ep_mp_double_mul(mp_p, mp_p10);
      mp_p = __dtgamma_ep_mp_double_mul(mp_p, mp_p11);
      mp_p = __dtgamma_ep_mp_double_mul(mp_p, mp_p0);
      p = __dtgamma_ep_mp_double_reduce(mp_p);
      /* if no recursion - p = 1.0 */
      pr = 1.0;
      mp_r = __dtgamma_ep_mp_double_init_h(pr);
      /* if i > 0 - recursies */
      if (i) {
        for (j = 1; j <= i; j++) {
          pr *= (absx - j);
        }
      }
      if (ixsign) {
        /* for negatives r1 = 1/(x*s*gamma*recursies)*/
        double inv_pr = 1.0 / pr;
        res = inv_pr / (p * s * absx);
        if (x < neg_half_overflow[1]) {
          res *= a176_inv;
        }
        /* set sign */
        if (irsign)
          res = -res;
      } else {
        /* for positives r1 = gamma*recursies */
        res = p * pr;
      }
      (*r) = res;
      return nRet;
    } else {
      /* if |x| < 1 - calculate gamma(x+1) */
      if (absx < 1.0) {
        curabsx = (absx + 1.0);
      } else {
        curabsx = absx;
      }
      /* splitted intervals: */
      if (curabsx >= 1.5) {
        /* x >= 1.5 */
        r1 = (curabsx - 1.0);
        dA = __dtgamma_ep_a150_tab;
      } else if (curabsx >= 1.25) {
        /* 1.5 > x >= 1.25 */
        r1 = (curabsx - local_min);
        dA = __dtgamma_ep_a125_tab;
      } else if (curabsx < 1.25) {
        /* 0 < x < 1.25 */
        r1 = (curabsx - 1.0);
        dA = __dtgamma_ep_a100_tab;
      }
      if (ixexp) {
        /* for normal values - compute whole polynomial */
        p = (((((((((((((((dA[15]) * r1 + dA[14]) * r1 + dA[13]) * r1 +
                        dA[12]) *
                           r1 +
                       dA[11]) *
                          r1 +
                      dA[10]) *
                         r1 +
                     dA[9]) *
                        r1 +
                    dA[8]) *
                       r1 +
                   dA[7]) *
                      r1 +
                  dA[6]) *
                     r1 +
                 dA[5]) *
                    r1 +
                dA[4]) *
                   r1 +
               dA[3]) *
                  r1 +
              dA[2]) *
                 r1 +
             dA[1]) *
                r1 +
            dA[0];
      } else {
        /* for denormal - return just A[0] */
        p = dA[0];
      }
      if (absx < 1.0) {
        /* |x| < 1.0 */
        if (ixsign) {
          /* if x < 0 then r1 = 1/(s*p) */
          res = 1.0 / (s * p);
          if (irsign)
            res = (-res);
        } else {
          /* if x > 0 then r1 = p/x */
          res = p / absx;
        }
      } else {
        /* |x| > 1.0 */
        if (ixsign) {
          /* r1 = 1/(x*s*p); */
          res = 1.0 / (absx * s * p);
        } else {
          /* r1 = p */
          res = p;
        }
      }
      (*r) = res;
      return nRet;
    }
  } else { /* INF or NAN */
    /* Singularity at negative INF */
    if (ixsign && ((as_ulong(x) & 0x000fffffffffffff) == 0)) {
      res = (zeros[(1)] / zeros[0]);
      nRet = 1;
      (*r) = res;
      return nRet;
    } else {
      res = x + x; /* raise invalid on SNaN */
      (*r) = res;
      return nRet;
    }
  }
}

} /* namespace */
} /* namespace __imf_impl_tgamma_d_ep */

DEVICE_EXTERN_C_INLINE double __devicelib_imf_tgamma(double x)
{
  using namespace __imf_impl_tgamma_d_ep;
  double r;
  __devicelib_imf_internal_dtgamma(&x, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
