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
//   ---------------------
//  The method consists of three cases.
// 
//  If       2 <= x < OVERFLOW_BOUNDARY
//  else if  0 < x < 2
//  else if  -(i+1) <  x < -i, i = 0...43
// 
//  Case 2 <= x < OVERFLOW_BOUNDARY
//  -------------------------------
//    Here we use algorithm based on the recursive formula
//    GAMMA(x+1) = x*GAMMA(x). For that we subdivide interval
//    [2; OVERFLOW_BOUNDARY] into intervals [8*n; 8*(n+1)] and
//    approximate GAMMA(x) by polynomial of 22th degree on each
//    [8*n; 8*n+1], recursive formula is used to expand GAMMA(x)
//    to [8*n; 8*n+1]. In other words we need to find n, i and r
//    such that x = 8 * n + i + r where n and i are integer numbers
//    and r is fractional part of x. So GAMMA(x) = GAMMA(8*n+i+r) =
//    = (x-1)*(x-2)*...*(x-i)*GAMMA(x-i) =
//    = (x-1)*(x-2)*...*(x-i)*GAMMA(8*n+r) ~
//    ~ (x-1)*(x-2)*...*(x-i)*P12n(r).
// 
//    Step 1: Reduction
//    -----------------
//     N = [x] with truncate
//     r = x - N, note 0 <= r < 1
// 
//     n = N & ~0x7 - index of table that contains coefficient of
//                    polynomial approximation
//     i = N & 0x7  - is used in recursive formula
// 
// 
//    Step 2: Approximation
//    ---------------------
//     We use factorized minimax approximation polynomials
//     P12n(r) = A12*(r^2+C01(n)*r+C00(n))*
//               *(r^2+C11(n)*r+C10(n))*...*(r^2+C51(n)*r+C50(n))
// 
//    Step 3: Recursion
//    -----------------
//     In case when i > 0 we need to multiply P12n(r) by product
//     R(i,x)=(x-1)*(x-2)*...*(x-i). To reduce number of fp-instructions
//     we can calculate R as follow:
//     R(i,x) = ((x-1)*(x-2))*((x-3)*(x-4))*...*((x-(i-1))*(x-i)) if i is
//     even or R = ((x-1)*(x-2))*((x-3)*(x-4))*...*((x-(i-2))*(x-(i-1)))*
//     *(i-1) if i is odd. In both cases we need to calculate
//     R2(i,x) = (x^2-3*x+2)*(x^2-7*x+12)*...*(x^2+x+2*j*(2*j-1)) =
//     = ((x^2-x)+2*(1-x))*((x^2-x)+6*(2-x))*...*((x^2-x)+2*(2*j-1)*(j-x)) =
//     = (RA+2*RB)*(RA+6*(1-RB))*...*(RA+2*(2*j-1)*(j-1+RB))
//     where j = 1..[i/2], RA = x^2-x, RB = 1-x.
// 
//    Step 4: Reconstruction
//    ----------------------
//     Reconstruction is just simple multiplication i.e.
//     GAMMA(x) = P12n(r)*R(i,x)
// 
//  Case 0 < x < 2
//  --------------
//     To calculate GAMMA(x) on this interval we do following
//         if 1.0  <= x < 1.25  than  GAMMA(x) = P7(x-1)
//         if 1.25 <= x < 1.5   than  GAMMA(x) = P7(x-x_min) where
//               x_min is point of local minimum on [1; 2] interval.
//         if 1.5  <= x < 1.75  than  GAMMA(x) = P7(x-1)
//         if 1.75 <= x < 2.0   than  GAMMA(x) = P7(x-1)
//     and
//         if 0 < x < 1 than GAMMA(x) = GAMMA(x+1)/x
// 
//  Case -(i+1) <  x < -i, i = 0...43
//  ----------------------------------
//     Here we use the fact that GAMMA(-x) = PI/(x*GAMMA(x)*sin(PI*x)) and
//     so we need to calculate GAMMA(x), sin(PI*x)/PI. Calculation of
//     GAMMA(x) is described above.
// 
//    Step 1: Reduction
//    -----------------
//     Note that period of sin(PI*x) is 2 and range reduction for
//     sin(PI*x) is like to range reduction for GAMMA(x)
//     i.e rs = x - round(x) and |rs| <= 0.5.
// 
//    Step 2: Approximation
//    ---------------------
//     To approximate sin(PI*x)/PI = sin(PI*(2*n+rs))/PI =
//     = (-1)^n*sin(PI*rs)/PI Taylor series is used.
//     sin(PI*rs)/PI ~ S17(rs).
// 
//    Step 3: Division
//    ----------------
//     1/x and 1/(GAMMA(x)*S12(rs))
// --
// 
*/
#include "_imf_include_fp32.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_tgamma_s_la {
namespace {
static const float __stgamma_la__neg_underflow[2] = {43.0, -43.0};
static const float __stgamma_la__neg_half_overflow[2] = {40.0, -40.0};
/* overflow boundary (35.04010009765625)*/
static const unsigned int __stgamma_la__overflow_boundary[] = {0x00000000,
                                                               0x40418522};
/* point of local minium (0.461632144968362356785)*/
static const unsigned int __stgamma_la__local_minimum[] = {0x8D5AF8FE,
                                                           0x3FDD8B61};
static const unsigned int __stgamma_la__tgamma_A_table[] = {
    /*[2; 8)*/
    0xBA0CB3B4,
    0x4009EFD1 /* C01*/
    ,
    0x78FF4822,
    0x3FFFB353 /* C11*/
    ,
    0xF695B198,
    0x3FD9CE68 /* C21*/
    ,
    0xC900DA03,
    0xBFF8C30A /* C31*/
    ,
    0x0413B896,
    0xC0103227 /* C41*/
    ,
    0x4C0D6827,
    0xC01F171A /* C51*/
    ,
    0xF0535C02,
    0x400E17D2 /* C00*/
    ,
    0x40F7FAC8,
    0x40106892 /* C10*/
    ,
    0x197396AC,
    0x40148F8E /* C20*/
    ,
    0x59F1249C,
    0x401C6019 /* C30*/
    ,
    0x7DDCCF8D,
    0x40256314 /* C40*/
    ,
    0x0480A21C,
    0x4033406D /* C50*/
    ,
    0x81741977,
    0x3EE21AD8 /* An*/
    /*[8; 16)*/
    ,
    0xAE0B793B,
    0x4006222B /* C01*/
    ,
    0x33473EDA,
    0x40024527 /* C11*/
    ,
    0x8F7B73C5,
    0x3FF54582 /* C21*/
    ,
    0x578764DF,
    0xBFBBD210 /* C31*/
    ,
    0x326FDDB3,
    0xC0010EF3 /* C41*/
    ,
    0x17F99C0F,
    0xC01492B8 /* C51*/
    ,
    0x98F53CFC,
    0x40005420 /* C00*/
    ,
    0x09AD6C81,
    0x40032C13 /* C10*/
    ,
    0x5A249B75,
    0x40099C90 /* C20*/
    ,
    0xAE0E533D,
    0x4012B972 /* C30*/
    ,
    0xE19BD2E1,
    0x401D7331 /* C40*/
    ,
    0x7295EF57,
    0x402A0680 /* C50*/
    ,
    0x91D0D4CC,
    0x3FE6F6DB /* An*/
    /*[16; 24)*/
    ,
    0x02867596,
    0x40001310 /* C01*/
    ,
    0xD5D1B6F2,
    0x3FFAA362 /* C11*/
    ,
    0x45BC2769,
    0x3FEEB1F3 /* C21*/
    ,
    0xE7F3316F,
    0xBFC3BBE6 /* C31*/
    ,
    0x5697DB6D,
    0xBFFCB698 /* C41*/
    ,
    0x3BFC3B3B,
    0xC0115BEE /* C51*/
    ,
    0xDA5E9983,
    0x3FF14E07 /* C00*/
    ,
    0xBF81E2C0,
    0x3FF53B76 /* C10*/
    ,
    0x83456F73,
    0x3FFE62FF /* C20*/
    ,
    0x78A114C4,
    0x4007E334 /* C30*/
    ,
    0x0269A3DC,
    0x4014051E /* C40*/
    ,
    0x27468EDB,
    0x40229D42 /* C50*/
    ,
    0x3795ED57,
    0x41E9B2B7 /* An*/
    /*[24; 32)*/
    ,
    0x498384DE,
    0x3FFAF7BD /* C01*/
    ,
    0xB4D1C3D2,
    0x3FF62AD8 /* C11*/
    ,
    0xBBB4B727,
    0x3FE8943C /* C21*/
    ,
    0x66E11756,
    0xBFCB39D4 /* C31*/
    ,
    0xCD004C32,
    0xBFFABCAD /* C41*/
    ,
    0x7C097EC9,
    0xC00FADE9 /* C51*/
    ,
    0x3243D8C1,
    0x3FE879AF /* C00*/
    ,
    0xBB14CE1E,
    0x3FEEC7DE /* C10*/
    ,
    0xD737707E,
    0x3FF6DA9E /* C20*/
    ,
    0x9E0C782C,
    0x4002A29E /* C30*/
    ,
    0x9BA80BCB,
    0x401017B7 /* C40*/
    ,
    0xC3C4DE80,
    0x401E941D /* C50*/
    ,
    0x5167C6C3,
    0x44329D5B /* An*/
    /*[32; 40)*/
    ,
    0xA0E8FE5C,
    0x3FF7ECB3 /* C01*/
    ,
    0x8516316B,
    0x3FF3815A /* C11*/
    ,
    0xDA1B5783,
    0x3FE4CE76 /* C21*/
    ,
    0xB460BC4E,
    0xBFD0524D /* C31*/
    ,
    0xFCC000C3,
    0xBFF9ABD8 /* C41*/
    ,
    0x69A4195B,
    0xC00DD899 /* C51*/
    ,
    0xDF14E200,
    0x3FE35852 /* C00*/
    ,
    0x0359F642,
    0x3FE8C761 /* C10*/
    ,
    0x39CBF563,
    0x3FF2E431 /* C20*/
    ,
    0x3474A606,
    0x3FFF96DC /* C30*/
    ,
    0x0EC16173,
    0x400BCF75 /* C40*/
    ,
    0x02EA701C,
    0x401AC14E /* C50*/
    ,
    0x9B0DDDF0,
    0x46AFF4CA /* An*/
    /*[40; 48)*/
    ,
    0xD8193097,
    0x3FF5DCE4 /* C01*/
    ,
    0xC4974FFA,
    0x3FF1B0D8 /* C11*/
    ,
    0xB11D847A,
    0x3FE231DE /* C21*/
    ,
    0xAFD7E935,
    0xBFD251EC /* C31*/
    ,
    0x0194CAEA,
    0xBFF8FB45 /* C41*/
    ,
    0xE030A6C4,
    0xC00C9658 /* C51*/
    ,
    0xE288F6BF,
    0x3FE0368A /* C00*/
    ,
    0x4215A70C,
    0x3FE513AE /* C10*/
    ,
    0x1118AB46,
    0x3FF06885 /* C20*/
    ,
    0xBB46BF7D,
    0x3FFBF7C7 /* C30*/
    ,
    0xF7141B8B,
    0x4008F960 /* C40*/
    ,
    0x8134397B,
    0x40183BA0 /* C50*/
    ,
    0x00000000,
    0x3FF00000 /* An*/
};
/* sin(pi*x)/pi*/
static const unsigned int __stgamma_la__tgamma_sin_table[] = {
    0x760626E2,
    0xBEC144B2 /* S31*/
    ,
    0x2FF39E13,
    0x3E90FC99 /* S32*/
    ,
    0xABECEFCA,
    0x405541D7 /* S00*/
    ,
    0x377656CC,
    0xC026FB0D /* S01*/
    ,
    0x4A41C6E7,
    0x406CE58F /* S10*/
    ,
    0x95A22324,
    0x3FFFB15F /* S11*/
    ,
    0x6302C61E,
    0x40445378 /* S20*/
    ,
    0x47DBFCD3,
    0xC023D59A /* S21*/
};
/* [1.0;1.25]*/
static const unsigned int __stgamma_la__tgamma_A100_table[] = {
    0xFFE9209E,
    0x3FEFFFFF /* A0*/
    ,
    0xCD545AA4,
    0xBFE2788C /* A1*/
    ,
    0xD144911C,
    0x3FEFA648 /* A2*/
    ,
    0xD93449B8,
    0xBFED0800 /* A3*/
    ,
    0xA010CA3B,
    0x3FEF4857 /* A4*/
    ,
    0xF7720B4D,
    0xBFEE3720 /* A5*/
    ,
    0xEEA8520F,
    0x3FE96FFE /* A6*/
    ,
    0x48921868,
    0xBFD99096 /* A7*/
};
/* [1.25;1.5]*/
static const unsigned int __stgamma_la__tgamma_A125_table[] = {
    0x82A39AA2,
    0x3FEC56DC /* A0*/
    ,
    0x9D8452EB,
    0xBDEE86A3 /* A1*/
    ,
    0x65BBEF1F,
    0x3FDB6C54 /* A2*/
    ,
    0x710A10B9,
    0xBFC0BADE /* A3*/
    ,
    0x1A545163,
    0x3FC496A0 /* A4*/
    ,
    0x3A546EBE,
    0xBFB7E7F8 /* A5*/
    ,
    0x14F36691,
    0x3FAF2375 /* A6*/
    ,
    0x6426936C,
    0xBFB42123 /* A7*/
};
/*[1.5; 1.75)*/
static const unsigned int __stgamma_la__tgamma_A150_table[] = {
    0x3AE7FBC8,
    0x3FEFFC61 /* A0*/
    ,
    0x1DF735B8,
    0xBFE24597 /* A1*/
    ,
    0x7AB26771,
    0x3FEE6555 /* A2*/
    ,
    0x7DBD23E4,
    0xBFE85B42 /* A3*/
    ,
    0xC8F09147,
    0x3FE3C90C /* A4*/
    ,
    0xBE3AB42A,
    0xBFD59D31 /* A5*/
    ,
    0xE3816C7B,
    0x3FBF4203 /* A6*/
    ,
    0x51795867,
    0xBF94730B /* A7*/
};
/*[1.75; 2.0)*/
static const unsigned int __stgamma_la__tgamma_A175_table[] = {
    0x644B2022,
    0x3FEFEE1E /* A0*/
    ,
    0xDFB25495,
    0xBFE1C1AE /* A1*/
    ,
    0x0A977CA5,
    0x3FEC56A8 /* A2*/
    ,
    0xC40AC0BB,
    0xBFE3C24A /* A3*/
    ,
    0x949175BE,
    0x3FDB262D /* A4*/
    ,
    0x07560916,
    0xBFC6F0E7 /* A5*/
    ,
    0xD09735F3,
    0x3FA96E37 /* A6*/
    ,
    0x5137617E,
    0xBF7746A8 /* A7*/
};
/* Right shifter */
static const unsigned __stgamma_la__two_23h[] = {0x4b000000}; /* 2^23 */
/* Special values */
static const unsigned int __stgamma_la__own_large_value_32[] = {
    0x71800000, 0xf1800000}; /* +2^100,-2^100 */
static const unsigned int __stgamma_la__own_small_value_32[] = {
    0x0d800000, 0x8d800000}; /* +2^(-100),-2^(-100) */
/* constants */
static const unsigned int __stgamma_la__zeros[] = {0x00000000, 0x80000000};
static const unsigned int __stgamma_la__ones[] = {0x3f800000, 0xbf800000};
static const unsigned int __stgamma_la__infs[] = {0x7f800000, 0xff800000};
static const float __stgamma_la_twos[] = {2.0, -2.0};
static const float __stgamma_la_ones_5[] = {1.5, -1.5};
static const float __stgamma_la_ones_25[] = {1.25, -1.25};
static const float __stgamma_la_ones_75[] = {1.75, -1.75};
extern float exp2f(float);
inline int __devicelib_imf_internal_stgamma(const float *a, float *r) {
  int nRet = 0;
  int t = 0, i = 0, j = 0, irsign = 0;
  int ix = 0, iabsx_n = 0, iabsx_t = 0, ixsign = 0, ixexp = 0;
  float dix = 0.0f, diabsx_n = 0.0f, diabsx_t = 0.0f;
  float tv = 0.0f;
  float x = 0.0f;
  float absx = 0.0f, res = 0.0f;
  double resf = 0.0;
  double s = 0.0, r2 = 0.0, r3 = 0.0, rrr = 0.0;
  double curabsx = 0.0;
  double p = 0.0, pr = 0.0;
  const double *A;
  x = *(a);
  absx = x;
  res = ((const float *)__stgamma_la__zeros)[0];
  /* get arg sign */
  ixsign = (((_iml_sp_union_t *)&x)->bits.sign);
  /* get arg exponent */
  ixexp = (((_iml_sp_union_t *)&x)->bits.exponent);
  /* normal values */
  if (ixexp != 0xFF) {
    /* create absolute value */
    (((_iml_sp_union_t *)&absx)->bits.sign = 0);
    ix = *((int *)(&absx));
    /* if x == 0 - zero divide exception */
    if (x == ((const float *)__stgamma_la__zeros)[0]) {
      *r = (((const float *)__stgamma_la__ones)[(ixsign)] /
            ((const float *)__stgamma_la__zeros)[0]);
      nRet = 2;
      return nRet;
    }
    if (ix <= 0x00200000) /* if |x| < denorm_overflow */
    {
      {
        float tz = ((const float *)__stgamma_la__own_large_value_32)[0];
        ((*r)) =
            (((const float *)__stgamma_la__own_large_value_32)[(ixsign)] * tz);
      }; /* raise overflow */
      nRet = 3;
      return nRet;
    }
    /* singularity at negative integer points */
    if (ixsign) {
      /* if |x| >= 2^23 - only integer values */
      if (ixexp >= 0x00000096) {
        {
          float tz = ((const float *)__stgamma_la__zeros)[0];
          ((*r)) = (((const float *)__stgamma_la__zeros)[(0)] / tz);
        };
        nRet = 1;
        return nRet;
      } // if(ixexp >= 0x00000096)
      else {
        /* get integer value of arg (truncated) */
        tv = absx + (*(const float *)__stgamma_la__two_23h);
        diabsx_t = tv - (*(const float *)__stgamma_la__two_23h);
        iabsx_t = (0x000fffff) & (*((int *)(&tv)));
        if (diabsx_t > absx) {
          diabsx_t -= ((const float *)__stgamma_la__ones)[0];
          iabsx_t -= 1;
        }
      } // else if(ixexp >= 0x00000096)
      /* if arg - integer then singularity */
      if (absx == diabsx_t) {
        {
          float tz = ((const float *)__stgamma_la__zeros)[0];
          ((*r)) = (((const float *)__stgamma_la__zeros)[(0)] / tz);
        };
        nRet = 1;
        return nRet;
      }
      /* if arg < -185.0 then underflow (values rounded to zero) */
      if (x < __stgamma_la__neg_underflow[1]) {
        (*r) = (((const float *)
                     __stgamma_la__own_small_value_32)[((~iabsx_t) & 1)] *
                ((const float *)
                     __stgamma_la__own_small_value_32)[0]); /* raise underflow
                                                               and inexact */
        nRet = 4;
        return nRet;
      }
    } // if(ixsign)
    /* big positive values overflow domain (res rounded to INF) */
    if (x >= (*((const double *)__stgamma_la__overflow_boundary))) {
      {
        float tz = ((const float *)__stgamma_la__own_large_value_32)[0];
        ((*r)) = (((const float *)__stgamma_la__own_large_value_32)[(0)] * tz);
      }; /* raise overflow and inexact */
      nRet = 3;
      return nRet;
    }
    /* compute sin(Pi*x)/x for negative values */
    if (ixsign) {
      /* get rounded to nearest abs arg */
      tv = absx + (*(const float *)__stgamma_la__two_23h);
      diabsx_n = tv - (*(const float *)__stgamma_la__two_23h);
      iabsx_n = (0x000fffff) & (*((int *)(&tv)));
      rrr = absx - diabsx_n; /* reduced argument */
      if (rrr < 0)
        rrr = (-rrr); // IML_ABS_DP(rrr);  /* remove sign */
      r2 = rrr * rrr; /* rrr^2 */
      /* Tailor series */
      s = rrr +
          rrr * ((r2 *
                  (((const double *)__stgamma_la__tgamma_sin_table)[0] +
                   r2 * ((const double *)__stgamma_la__tgamma_sin_table)[1])) *
                 (((const double *)__stgamma_la__tgamma_sin_table)[2] +
                  r2 * (r2 +
                        ((const double *)__stgamma_la__tgamma_sin_table)[3])) *
                 (((const double *)__stgamma_la__tgamma_sin_table)[4] +
                  r2 * (r2 +
                        ((const double *)__stgamma_la__tgamma_sin_table)[5])) *
                 (((const double *)__stgamma_la__tgamma_sin_table)[6] +
                  r2 * (r2 +
                        ((const double *)__stgamma_la__tgamma_sin_table)[7])));
    } // if(ixsign)
    /* get truncated integer argument */
    tv = absx + (*(const float *)__stgamma_la__two_23h);
    diabsx_t = tv - (*(const float *)__stgamma_la__two_23h);
    iabsx_t = (0x000fffff) & (*((int *)(&tv)));
    if (diabsx_t > absx) {
      diabsx_t -= ((const float *)__stgamma_la__ones)[0];
      iabsx_t -= 1;
    }
    /* get result sign */
    irsign = ((iabsx_t + 1) & 1);
    /* if x > 2.0 - simple polynomials */
    if (absx >= __stgamma_la_twos[0]) {
      t = iabsx_t & (~0x7); /* index of table of coefficient */
      i = iabsx_t & (0x7);  /* used in recursive formula */
                            /* for 2 <= x < 8 - shift index*/
      if (iabsx_t < 8)
        i = i - 2;
      rrr = absx - diabsx_t; /* reduced argument */
      A = &(((const double *)
                 __stgamma_la__tgamma_A_table)[t + (t >> 1) +
                                               (t >> 3)]); /* table address */
      r2 = rrr * rrr;                                      /* rrr^2 */
      /* factorized polynomial */
      p = A[12] * (r2 + A[0] * rrr + A[6 + 0]) * (r2 + A[1] * rrr + A[6 + 1]) *
          (r2 + A[2] * rrr + A[6 + 2]) * (r2 + A[3] * rrr + A[6 + 3]) *
          (r2 + A[4] * rrr + A[6 + 4]) * (r2 + A[5] * rrr + A[6 + 5]);
      /* if no recursion - p = 1.0 */
      pr = ((const float *)__stgamma_la__ones)[0];
      /* if i > 0 - recursies */
      if (i) {
        for (j = 1; j <= i; j++) {
          pr *= (absx - j);
        }
      }
      if (ixsign) /* for negatives rrr = 1/(x*s*gamma*recursies)*/
      {
        unsigned int __stgamma_la__tgamma_A40_inv[] = {
            0xEDBCC440, 0x368954EA /* 1/An*/
        };
        double resd = (double)((const float *)__stgamma_la__ones)[0] /
                      ((double)absx * s * p * pr);
        if (x < __stgamma_la__neg_half_overflow[1]) {
          resd *= (*((double *)__stgamma_la__tgamma_A40_inv));
        }
        /* set sign */
        if (irsign)
          resd = -resd;
        (*r) = (float)resd;
      } else /* for positives rrr = gamma*recursies */
      {
        (*r) = p * pr;
      }
      return nRet;
    } // if(absx >= _VSTATIC(twos)[0])
    else {
      /* if |x| < 1 - calculate gamma(x+1) */
      if (absx < ((const float *)__stgamma_la__ones)[0]) {
        curabsx = absx + ((double)((const float *)__stgamma_la__ones)[0]);
      } else {
        curabsx = absx;
      }
      /* split intervals: */
      /* x >= 1.75 */
      if (curabsx >= __stgamma_la_ones_75[0]) {
        rrr = curabsx - ((double)((const float *)__stgamma_la__ones)[0]);
        A = ((const double *)__stgamma_la__tgamma_A175_table);
      } else if (curabsx >= __stgamma_la_ones_5[0]) /* x >= 1.5 */
      {
        rrr = curabsx - ((double)((const float *)__stgamma_la__ones)[0]);
        A = ((const double *)__stgamma_la__tgamma_A150_table);
      } else if (curabsx >= __stgamma_la_ones_25[0]) /* 1.5 > x >= 1.25 */
      {
        rrr = curabsx - (((double)((const float *)__stgamma_la__ones)[0]) +
                         (*((const double *)__stgamma_la__local_minimum)));
        A = ((const double *)__stgamma_la__tgamma_A125_table);
      } else if (curabsx < __stgamma_la_ones_25[0]) /* 0 < x < 1.25 */
      {
        rrr = curabsx - ((double)((const float *)__stgamma_la__ones)[0]);
        A = ((const double *)__stgamma_la__tgamma_A100_table);
      }
      if (ixexp) /* for normal values - compute whole polynomial */
      {
        p = A[0] +
            rrr * (A[1] +
                   rrr * (A[2] +
                          rrr * (A[3] +
                                 rrr * (A[4] +
                                        rrr * (A[5] +
                                               rrr * (A[6] + rrr * (A[7])))))));
      } else /* for denormal - return just A[0] */
      {
        p = A[0];
      }
      if (absx < ((const float *)__stgamma_la__ones)[0]) /* |x| < 1.0 */
      {
        if (ixsign) /* if x < 0 then rrr = 1/(s*p) */
        {
          resf = ((double)((const float *)__stgamma_la__ones)[0]) / (s * p);
          if (irsign)
            resf = -resf;
        } else /* if x > 0 then rrr = p/x */
        {
          resf = p / ((double)absx);
        }
      }    // if(absx < ones[0]) /* |x| < 1.0 */
      else /* |x| > 1.0 */
      {
        if (ixsign) /* rrr = 1/(x*s*p); */
        {
          resf = ((double)((const float *)__stgamma_la__ones)[0]) /
                 (((double)absx) * s * p);
        } else /* rrr = p */
        {
          resf = p;
        }
      } // else if(absx < ones[0]) /* |x| < 1.0 */
      (*r) = (float)resf;
      return nRet;
    }  // else if(absx >= _VSTATIC(twos)[0])
  }    // if (ixexp != IML_EXPINF_32)
  else /* INF or NAN */
  {
    /* Singularity at negative INF */
    if (ixsign && (!((((_iml_sp_union_t *)&x)->bits.significand) != 0))) {
      {
        float tz = ((const float *)__stgamma_la__zeros)[0];
        ((*r)) = (((const float *)__stgamma_la__zeros)[(1)] / tz);
      };
      nRet = 1;
      return nRet;
    } else {
      (*r) = x + x; /* raise invalid on SNaN */
      return nRet;
    }
  } // else if (ixexp != IML_EXPINF_32)
}
} /* namespace */
} /* namespace __imf_impl_tgamma_s_la */

DEVICE_EXTERN_C_INLINE float __devicelib_imf_tgammaf(float a) {
  using namespace __imf_impl_tgamma_s_la;
  float r;
  __devicelib_imf_internal_stgamma(&a, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
