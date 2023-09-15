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
#include "_imf_include_fp64.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_y0_d_ep {
namespace {
struct fp64 { /*/ sign:1 exponent:11 significand:52 (implied leading 1)*/
  unsigned lo_significand : 32;
  unsigned hi_significand : 20;
  unsigned exponent : 11;
  unsigned sign : 1;
};
static const uint32_t __dy0_ep__DP[] = {
    0x00000000, 0x00000000, /* 2^200 x 4/Pi, extended precision, split by 25
                               bits */
    0x60000000, 0x4c745f30, /* 2^150 x 1.43354022697369600000e+015 */
    0x00000000, 0x4aeb9391, /* 2^150 x 5.78319680000000000000e+007 */
    0x20000000, 0x494529fc, /* 2^150 x 6.61375105381011960000e-001 */
    0xc0000000, 0x47ad5f47, /* 2^150 x 1.36774911396742030000e-008 */
    0x00000000, 0x4624d377, /* 2^150 x 5.78038576612384040000e-016 */
    0x80000000, 0x447b6c52, /* 2^150 x 5.67097116406186190000e-024 */
    0x20000000, 0x430993c4, /* 2^150 x 6.30526941320619260000e-031 */
    0xe0000000, 0x4189041f, /* 2^150 x 3.67578719994527880000e-038 */
    0xe0000000, 0x3fe458ea, /* 2^150 x 4.45510746226535320000e-046 */
    0x50000000, 0x3e67aef1, /* 2^150 x 3.09082395413093270000e-053 */
    0x30000000, 0x3cd0db92, /* 2^150 x 6.55660755499121080000e-061 */
    0x80000000, 0x3b3c7484, /* 2^150 x 1.64915633578929980000e-068 */
    0x00000000, 0x39bba5c0, /* 2^150 x 9.55072531106557900000e-076 */
    0xa0000000, 0x382924bb, /* 2^150 x 2.58855104242604980000e-083 */
    0x90000000, 0x36904e8c, /* 2^150 x 5.00323524688082680000e-091 */
    0x00000000, 0x34ccfe1d, /* 2^150 x 1.65691052678466090000e-099 */
    0x20000000, 0x337d6396, /* 2^150 x 8.00878221345175640000e-106 */
    0x60000000, 0x31d4d39f, /* 2^150 x 8.45709247014436530000e-114 */
    0xf0000000, 0x3054411a, /* 2^150 x 4.90229408458623630000e-121 */
    0x40000000, 0x2ec52ebb, /* 2^150 x 1.52795298719984900000e-128 */
    0x60000000, 0x2d2213a6, /* 2^150 x 1.94299866957126560000e-136 */
    0xd0000000, 0x2ba1c09a, /* 2^150 x 1.13733453961144910000e-143 */
    0x00000000, 0x29e7df90, /* 2^150 x 5.69776444062822040000e-152 */
    0x60000000, 0x2883991d, /* 2^150 x 1.11518356990504860000e-158 */
    0x80000000, 0x26dcc1a9, /* 2^150 x 1.21915331962451730000e-166 */
    0x40000000, 0x255cfa4e, /* 2^150 x 7.32263308706998730000e-174 */
    0x80000000, 0x23a17e2e, /* 2^150 x 3.29347500466345590000e-182 */
    0x30000000, 0x224df928, /* 2^150 x 1.34544777683850980000e-188 */
    0x20000000, 0x20b63ff1, /* 2^150 x 2.97651736637168620000e-196 */
    0x10000000, 0x1f2fff78, /* 2^150 x 1.27571825625735480000e-203 */
    0xe0000000, 0x1d8980fe, /* 2^150 x 1.51516338890805680000e-211 */
    0xb0000000, 0x1c02f118, /* 2^150 x 6.70739706967294120000e-219 */
    0x40000000, 0x1a66829b, /* 2^150 x 1.18776977327670780000e-226 */
    0x40000000, 0x18cf6d36, /* 2^150 x 2.47098955256518700000e-234 */
    0xe0000000, 0x175f6793, /* 2^150 x 2.94358617354826070000e-241 */
    0x80000000, 0x15b6136e, /* 2^150 x 3.08334493652883650000e-249 */
    0xc0000000, 0x143e8c7e, /* 2^150 x 2.54318519592794750000e-256 */
    0xa0000000, 0x12aa797f, /* 2^150 x 6.56845574861513910000e-264 */
    0x30000000, 0x11116ba9, /* 2^150 x 1.28808876071917070000e-271 */
    0xb0000000, 0x0f8bac7e, /* 2^150 x 6.09821810754177020000e-279 */
    0x60000000, 0x0dfcbe2f, /* 2^150 x 1.88762133523511850000e-286 */
    0xe0000000, 0x0c5e839c, /* 2^150 x 2.98610443121133630000e-294 */
    0x40000000, 0x0adbc529, /* 2^150 x 1.61980503987574130000e-301 */
    0xf0000000, 0x0942ea6b, 0x30000000, 0x07b6bf62, 0x20000000,
    0x062e3574, 0x00000000, 0x046580cc, 0xc0000000, 0x02e1bf1e,
};
static const uint64_t __dy0_ep__DP3[] = {
    /* Pi/4, split by 24 bits */
    0x3fe921fb40000000, /* .785398125648498535156250000000000000e-00 */
    0x3e64442d00000000, /* .377489470793079817667603492736816406e-07 */
    0x3ce8469880000000, /* .269515126497888238277234052020503440e-14 */
    0x3b68cc51701b839a, /* .164100177143675023722023662896821847e-21 */
};
static const uint64_t __dy0_ep__DP2[] = {
    /* Pi/4, split by 36 bits */
    0x3fe921fb54440000, /* .785398163396166637539863586425781250e-00 */
    0x3d768c234c400000, /* .128167207563315921695079602216083003e-11 */
    0x3b68cc51701b839a, /* .164100177143675023722023662896821847e-21 */
};
/* approximation of (sin(x)/x-1)/(x^2) on |x|=[0..Pi/4], max.err .28e-13 */
static const uint64_t __dy0_ep__SP[] = {
    0xbfc555555555516d, /* -.1666666666666389001574092663227e-0 SP[0] */
    0x3f81111110fd4208, /*  .8333333331081340727248579433954e-2 SP[1] */
    0xbf2a019fd9bd0882, /* -.1984126691870238094119659272295e-3 SP[2] */
    0x3ec71d9aa585bfc4, /*  .2755599137475836194220161698382e-5 SP[3] */
    0xbe5aa2880297fc43, /* -.2480567232697144802048174630580e-7 SP[4] */
};
/* approximation of (cos(x)-1)/(x^2) on |x|=[0..Pi/4], max.err .36e-12 */
static const uint64_t __dy0_ep__CP[] = {
    0xbfdfffffffffe6a2, /* -.4999999999996395247842975513023e-0 CP[0] */
    0x3fa5555555150951, /*  .4166666663742780406412103193419e-1 CP[1] */
    0xbf56c16bae710ff8, /* -.1388888509397110798760860956647e-2 CP[2] */
    0x3efa01299942ab00, /*  .2479986285723167468258822162782e-4 CP[3] */
    0xbe9247507b5ee59e, /* -.2723719448833349736164120625340e-6 CP[4] */
};
/* |x| >= 2^30, use Payne and Hanek algorithm */
static int __dy0_ep_reduce_pi04d(double x, double *y, int n) {
  int bitpos, exp, i, j, k;
  double xl, yl, yh, t;
  struct fp64 *ptx = (struct fp64 *)&x;
  struct fp64 *ptt = (struct fp64 *)&t;
  const double PI04 =
      0.78539816339744830961566084581987572104929; /* 3FE921FB54442D18 Pi/4 */
  const double zero_none[2] = {0.0, -1.0};
  exp = ptx->exponent;
  ptx->exponent -= 200;
  /* divide x into two parts (low and high) */
  xl = x;
  ptx->lo_significand &= 0xf8000000;
  xl -= x;
  bitpos = exp - (0x03FF + 29);
  j = bitpos / 25;
  bitpos = bitpos - j * 25;
  if ((bitpos >= 17)) {
    yl = (xl * ((const double *)__dy0_ep__DP)[j + 0] +
          x * ((const double *)__dy0_ep__DP)[j + 1]);
    j++;
    t = yl;
    ptt->lo_significand &= 0xfff00000;
    yl -= t;
  } else {
    yl = zero_none[0]; /*0.0*/
  }
  yh = (xl * ((const double *)__dy0_ep__DP)[j + 0] +
        x * ((const double *)__dy0_ep__DP)[j + 1]);
  j++;
  yl = yl + yh;
  yh = (xl * ((const double *)__dy0_ep__DP)[j + 0] +
        x * ((const double *)__dy0_ep__DP)[j + 1]);
  j++;
  /* t = (int)yl */
  t = yl + yh;
  i = 0x03FF + 52 - ptt->exponent;
  (*(long long int *)&t) >>= i;
  k = ((int *)&t)[0];
  (*(long long int *)&t) <<= i;
  k += n;
  /* yl -= (int)yl; */
  yl -= t;
  *y = zero_none[k & 1];
  *y += yl;
  *y += yh;
  for (i = 0; i < 5; i++) {
    *y += (xl * ((const double *)__dy0_ep__DP)[j] +
           x * ((const double *)__dy0_ep__DP)[j + 1]);
    j++;
  }
  *y *= PI04;
  return k;
}
static int __dy0_ep_sincos_kernel_fp64(double x, int n, double *psn,
                                       double *pcs) {
  const int iones[2] = {1, -1};
  uint32_t ix;
  int j, k, ssign, csign, carry;
  double y, z, cs, sn, cs1, sn1, cs2, sn2;
  double tv;
  const uint64_t _INV_PI04 =
      0x3ff45f306dc9c883; /* 1.27323954473516276486577680 4/Pi */
  const uint64_t _TWO_52H = 0x4338000000000000; /* 2^52+2^51 */
  const double ones[2] = {1.0, -1.0};
  const double PI04 =
      0.78539816339744830961566084581987572104929; /* 3FE921FB54442D18 Pi/4 */
  ix = (((int *)&x)[1] & ~0x80000000);
  ssign = ((unsigned)((int *)&x)[1] >> 31);
  x = __fabs(x);
  n *= iones[ssign];
  if (ix < 0x41c00000) /* if |x| < 2^30 use Cody algorithm */
  {
    if (ix <= 0x3ff90000) {
      if ((k = n + 1) & 2)
        x -= PI04;
    } else {
      y = x * (*(const double *)&_INV_PI04);
      tv = (y + (*(const double *)&_TWO_52H));
      j = ((int *)&tv)[0];  /* integer part of x/PIO4 */
      j -= ((double)j > y); /* truncation */
      k = j + n;
      carry = (k & 1);
      k += carry;
      j += carry; /* map zeros to origin */
      /* Extended precision modular arithmetic */
      if (ix < 0x41000000) {
        x = x - j * ((const double *)__dy0_ep__DP2)[0] -
            j * ((const double *)__dy0_ep__DP2)[1] -
            j * ((const double *)__dy0_ep__DP2)[2];
      } else {
        x = x - j * ((const double *)__dy0_ep__DP3)[0] -
            j * ((const double *)__dy0_ep__DP3)[1] -
            j * ((const double *)__dy0_ep__DP3)[2] -
            j * ((const double *)__dy0_ep__DP3)[3];
      }
    }
  } else /* if |x| >= 2^30 use Payne and Hanek algorithm */
  {
    k = 1 + __dy0_ep_reduce_pi04d(x, &x, n);
  }
  csign = ((k + 2) >> 2) & 1;
  ssign ^= (k >> 2) & 1;
  y = x * x;
  z = y * y;
  cs1 = (((const double *)__dy0_ep__CP)[3] * z +
         ((const double *)__dy0_ep__CP)[1]) *
            z +
        ones[0];
  sn1 = (((const double *)__dy0_ep__SP)[3] * z +
         ((const double *)__dy0_ep__SP)[1]) *
            z * x +
        x;
  cs2 = ((((const double *)__dy0_ep__CP)[4] * z +
          ((const double *)__dy0_ep__CP)[2]) *
             z +
         ((const double *)__dy0_ep__CP)[0]) *
        y;
  sn2 = ((((const double *)__dy0_ep__SP)[4] * z +
          ((const double *)__dy0_ep__SP)[2]) *
             z +
         ((const double *)__dy0_ep__SP)[0]) *
        y * x;
  cs = (cs1 + cs2);
  sn = (sn1 + sn2);
  if (k & 2) {
    *psn = cs * ones[ssign];
    *pcs = sn * ones[csign];
  } else {
    *psn = sn * ones[ssign];
    *pcs = cs * ones[csign];
  }
  return k;
}
/* file: _vdln_kernel_cout.i */
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy0_ep___c19 = {0xbfb6e22682c05596UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy0_ep___c18 = {0x3fb6c694b21a9875UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy0_ep___c17 = {0xbfa68f0acee35e2dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy0_ep___c16 = {0x3fa9474ccd075ce5UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy0_ep___c15 = {0xbfb0750f4f9c34f9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy0_ep___c14 = {0x3fb16608748ab72dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy0_ep___c13 = {0xbfb23e2ec341eba0UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy0_ep___c12 = {0x3fb3aa521d980cd0UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy0_ep___c11 = {0xbfb555fa23866d76UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy0_ep___c10 = {0x3fb74629a554d880UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy0_ep___c9 = {0xbfb999938abcf213UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy0_ep___c8 = {0x3fbc71c472fb2195UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy0_ep___c7 = {0xbfc00000112830d9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy0_ep___c6 = {0x3fc24924982c2697UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy0_ep___c5 = {0xbfc55555551fbbdbUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy0_ep___c4 = {0x3fc99999998c68b5UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy0_ep___c3 = {0xbfd0000000002697UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy0_ep___c2 = {0x3fd5555555555b0eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy0_ep___c1 = {0xbfdffffffffffff0UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy0_ep___c0 = {0xbc8a30cfded694ffUL};
static int __dy0_ep_ln_kernel_fp64(double x, double *r) {
  int nRet = 0;
  union {
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
  } dx, expon, expon_r, one, l2;
  double R, d_expon;
  union {
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
  } denorm_scale;
  double poly, res;
  union {
    uint64_t w;
    uint32_t w32[2];
    int32_t s32[2];
    double f;
  } _res;
  int denorm_scale_exp;
  dx.f = x;
  // special branch for +/-0, negatives, +INFs, +NaNs
  if ((dx.w == 0x0uL) || (dx.w >= 0x7ff0000000000000uL)) {
    // dx = +/-0
    if ((dx.w & 0x7fffffffffffffff) == 0x0uL) {
      nRet = -1;
      _res.w = 0xfff0000000000000uL;
      *r = _res.f;
      return nRet;
    }
    // dx = any negative
    else if (dx.w > 0x8000000000000000uL) {
      nRet = -2;
      _res.w = dx.w | 0xfff8000000000000uL;
      *r = _res.f;
      return nRet;
    }
    // dx = +NaN or +INF
    else {
      // dx = +NaN
      if (dx.w > 0x7ff0000000000000uL) {
        _res.f = dx.f + dx.f;
      }
      // dx = +INF
      else {
        _res.w = dx.w;
      }
      *r = _res.f;
      return nRet;
    } // dx = +NaN or +INF
  }   // special branch for +/-0, negatives, +INFs, +NaNs
      // scale denormals
  denorm_scale.w = 0x43B0000000000000ull;
  denorm_scale_exp = (dx.w <= 0x000fffffffffffffuL) ? (60 + 0x3FF) : 0x3FF;
  dx.f = (dx.w <= 0x000fffffffffffffuL) ? (dx.f * denorm_scale.f) : dx.f;
  // argument reduction to (-1/3, 1/3)
  // reduced exponent
  expon.w = dx.w + 0x000AAAAAAAAAAAAAull;
  expon.w >>= 52;
  expon_r.w = expon.w << 52;
  // reduced mantissa
  one.w = 0x3FF0000000000000ull;
  dx.w = (dx.w + one.w) - expon_r.w;
  // reduced argument:  reduced_mantissa - 1.0
  R = dx.f - one.f;
  // polynomial
  poly = __fma(__dy0_ep___c19.f, R, __dy0_ep___c18.f);
  poly = __fma(poly, R, __dy0_ep___c17.f);
  poly = __fma(poly, R, __dy0_ep___c16.f);
  poly = __fma(poly, R, __dy0_ep___c15.f);
  poly = __fma(poly, R, __dy0_ep___c14.f);
  poly = __fma(poly, R, __dy0_ep___c13.f);
  poly = __fma(poly, R, __dy0_ep___c12.f);
  poly = __fma(poly, R, __dy0_ep___c11.f);
  poly = __fma(poly, R, __dy0_ep___c10.f);
  poly = __fma(poly, R, __dy0_ep___c9.f);
  poly = __fma(poly, R, __dy0_ep___c8.f);
  poly = __fma(poly, R, __dy0_ep___c7.f);
  poly = __fma(poly, R, __dy0_ep___c6.f);
  poly = __fma(poly, R, __dy0_ep___c5.f);
  poly = __fma(poly, R, __dy0_ep___c4.f);
  poly = __fma(poly, R, __dy0_ep___c3.f);
  poly = __fma(poly, R, __dy0_ep___c2.f);
  poly = __fma(poly, R, __dy0_ep___c1.f);
  poly = __fma(poly, R, __dy0_ep___c0.f);
  // prepare exponent
  // scale back denormals
  expon.s32[0] -= denorm_scale_exp;
  // exponent
  d_expon = (double)expon.s32[0];
  // full polynomial = log(1+R)
  poly = __fma(poly, R, R);
  // result:  reduced_exponent*log(2)+log(1+R)
  l2.w = 0x3FE62E42FEFA39EFull;
  res = __fma(d_expon, l2.f, poly);
  *r = res;
  return nRet;
}

/*
//
//  Interval centers - reduction constants x-Z[i]:
//
*/
static const double __dy0_ep_dJ_MP[] = {
    0x1.33d152e971b4p+1,
    -0x1.0f8p-53, // 2.40482555769577289 + -1.17744355931925782e-16
                  // [0x40033D152E971B40 + 0xBCA0F80000000000]
};
/*
//
//  Coefficients for polynomial minimax-approximation J0(x) = P1(x-Z[0]) on
interval (0,S[0]):
//
*/
static const double __dy0_ep_dJ1[] = {
    -0x1.8230006f896f4p-66, //  -2.04445963718088858e-20 [0xBBD8230006F896F4]
    0x1.55444ebabdbf7p-60,  //   1.15625698277889504e-18 [0x3C355444EBABDBF7]
    0x1.7260cf8da1afbp-56,  //   2.00782406657390732e-17 [0x3C77260CF8DA1AFB]
    -0x1.bba50f06efadep-52, //  -3.84800490555993181e-16 [0xBCBBBA50F06EFADE]
    -0x1.d23f740db037p-48,  //  -6.47048893056306539e-15 [0xBCFD23F740DB0370]
    0x1.f70e1f57fba0ep-44,  //   1.11700685538152342e-13 [0x3D3F70E1F57FBA0E]
    0x1.cd42362ea492fp-40,  //   1.63871987714522369e-12 [0x3D7CD42362EA492F]
    -0x1.bdc46cad37452p-36, //  -2.53389363975708676e-11 [0xBDBBDC46CAD37452]
    -0x1.5c2c39acab98bp-32, //  -3.16661276013226936e-10 [0xBDF5C2C39ACAB98B]
    0x1.2951bd472c072p-28,  //   4.32656516791486676e-09 [0x3E32951BD472C072]
    0x1.7ff991695aa69p-25,  //   4.47005586528694268e-08 [0x3E67FF991695AA69]
    -0x1.1cce302816eacp-21, //  -5.30491438342997002e-07 [0xBEA1CCE302816EAC]
    -0x1.232c77d22ca98p-18, //  -4.33882628862965924e-06 [0xBED232C77D22CA98]
    0x1.6ed3b9f07e84dp-15,  //   4.37291927290616944e-05 [0x3F06ED3B9F07E84D]
    0x1.15382ba06ccb6p-12,  //   0.000264377036752704172 [0x3F315382BA06CCB6]
    -0x1.1f992590d12b8p-9,  //   -0.00219420035901615393 [0xBF61F992590D12B8]
    -0x1.1bb1cbe1a4072p-7,  //    -0.0086576695933049154 [0xBF81BB1CBE1A4072]
    0x1.cfae864368d7p-5,    //     0.0566017744379462284 [0x3FACFAE864368D70]
    0x1.ba1deea029494p-4,   //      0.107938701754920097 [0x3FBBA1DEEA029494]
    -0x1.09cdb3655128p-1,   //     -0.519147497289466742 [0xBFE09CDB36551280]
    0x1.70ad177e48819p-65,  //   3.90351046521469379e-20 [0x3BE70AD177E48819]
};
// Internal J0 functions for small (less than 0.6) x
static inline double __dy0_ep_small_j0(double x) {
  uint32_t ixhi = 0;
  double xi = 0, y = 0, z = 0, p = 0, q = 0, rr = 0, r1 = 0, result = 0;
  double dy = 0, dz = 0;
  x = x - __dy0_ep_dJ_MP[0];
  x = x - __dy0_ep_dJ_MP[1];
  y = x * x;
  ixhi = ((uint32_t *)&x)[1];
  if (ixhi >= 0x3C600000) /* |x| >= 2^(-57) */
  {
    // Polynomial
    p = (((((((((__dy0_ep_dJ1[0] * y + __dy0_ep_dJ1[2]) * y + __dy0_ep_dJ1[4]) *
                   y +
               __dy0_ep_dJ1[6]) *
                  y +
              __dy0_ep_dJ1[8]) *
                 y +
             __dy0_ep_dJ1[10]) *
                y +
            __dy0_ep_dJ1[12]) *
               y +
           __dy0_ep_dJ1[14]) *
              y +
          __dy0_ep_dJ1[16]) *
             y +
         __dy0_ep_dJ1[18]) *
            y +
        __dy0_ep_dJ1[20];
    q = (((((((((__dy0_ep_dJ1[1] * y + __dy0_ep_dJ1[3]) * y + __dy0_ep_dJ1[5]) *
                   y +
               __dy0_ep_dJ1[7]) *
                  y +
              __dy0_ep_dJ1[9]) *
                 y +
             __dy0_ep_dJ1[11]) *
                y +
            __dy0_ep_dJ1[13]) *
               y +
           __dy0_ep_dJ1[15]) *
              y +
          __dy0_ep_dJ1[17]) *
             y +
         __dy0_ep_dJ1[19]) *
        x;
    result = p + q;
  } else {
    result = 1.0 - __fabs(x);
  }
  return result;
}
/*
//
//  Reduction values (center of intyervals)
//
*/
static const double __dy0_ep_dZ_MP[] = {
    0x1.c982eb8d417eap-1,
    0x1.ebp-56, // 0.893576966279167495 +      2.66171633345191339e-17
                // [0x3FEC982EB8D417EA + 0x3C7EB00000000000]
    0x1.fa9534d98569cp+1,
    -0x1.fp-54, //  3.95767841931485798 +      -1.0755285551056204e-16
                //  [0x400FA9534D98569C + 0xBC9F000000000000]
    0x1.c581dc4e72103p+2,
    -0x1.98p-54, //  7.08605106030177279 +     -8.84708972748171618e-17
                 //  [0x401C581DC4E72103 + 0xBC99800000000000]
    0x1.471d735a47d58p+3,
    -0x1.cb8p-51, //  10.2223450434964178 +      -7.9710543721134286e-16
                  //  [0x402471D735A47D58 + 0xBCCCB80000000000]
    0x1.ab8e1c4a1e74ap+3,
    -0x1.7ep-51, //  13.3610974738727641 +      -6.6266436782314031e-16
                 //  [0x402AB8E1C4A1E74A + 0xBCC7E00000000000]
    0x1.0803c74003214p+4,
    0x1.258p-50, //  16.5009224415280897 +      1.01828268039838576e-15
                 //  [0x4030803C74003214 + 0x3CD2580000000000]
    0x1.3a42cdf5febd7p+4,
    -0x1.8cp-50, //  19.6413097008879411 +     -1.37390099297363122e-15
                 //  [0x4033A42CDF5FEBD7 + 0xBCD8C00000000000]
    0x1.6c832fd77ac07p+4,
    0x1.ca8p-50, //  22.7820280472915577 +      1.59074142747073211e-15
                 //  [0x4036C832FD77AC07 + 0x3CDCA80000000000]
    0x1.9ec46f3e80146p+4,
    -0x1.04p-52, //  25.9229576531809229 +     -2.25514051876984922e-16
                 //  [0x4039EC46F3E80146 + 0xBCB0400000000000]
    0x1.d106449616c4fp+4,
    0x1.0a8p-50, //  29.0640302527283971 +      9.24607612695638181e-16
                 //  [0x403D106449616C4F + 0x3CD0A80000000000]
    0x1.01a4420e4abeep+5,
    0x1.d3p-49, //  32.2052041164932774 +      3.24046345312467565e-15
                //  [0x40401A4420E4ABEE + 0x3CED300000000000]
    0x1.1ac588c944279p+5,
    -0x1.ap-53, //  35.3464523052143207 +     -1.80411241501587938e-16
                //  [0x4041AC588C944279 + 0xBCAA000000000000]
};
/*
//
//  Interval bounds
//
*/
static const double __dy0_ep_dS[] = {
    0x1.193bed4dff243p+1, // 2.19714132603101708 [0x400193BED4DFF243]
    0x1.5b7fe4e87b02ep+2, // 5.42968104079413472 [0x4015B7FE4E87B02E]
    0x1.13127ae6169b4p+3, // 8.59600586833116864 [0x40213127AE6169B4]
    0x1.77f9138d43206p+3, // 11.7491548308398812 [0x40277F9138D43206]
    0x1.dcb7d88de848bp+3, //  14.897442128336726 [0x402DCB7D88DE848B]
    0x1.20b1c695f1e3bp+4, //  18.043402276727857 [0x40320B1C695F1E3B]
    0x1.53025492188cdp+4, // 21.1880689341422119 [0x40353025492188CD]
    0x1.854fa303820cap+4, // 24.3319425713569117 [0x403854FA303820CA]
    0x1.b79acee8cfb7dp+4, // 27.4752949804492239 [0x403B79ACEE8CFB7D]
    0x1.e9e480605283cp+4, // 30.6182864916411148 [0x403E9E480605283C]
    0x1.0e16907f8fb56p+5, // 33.7610177961093285 [0x4040E16907F8FB56]
    0x1.273a7b35a7affp+5, //  36.903555316142949 [0x404273A7B35A7AFF]
};
/*
//
//  Coefficients for polynomial minimax-approximation Y0(x) =
P0(x^2)+2/pi*j0(x)*log(x) on interval (0.0,0.6).
//
*/
static const double __dy0_ep_dP0[] = {
    0x1.21dc99f6a7cf6p-38,  //  4.11918285147607579e-12 [0x3D921DC99F6A7CF6]
    -0x1.a6ec762057d51p-31, // -7.69293687817313281e-10 [0xBE0A6EC762057D51]
    0x1.bce4a4c57ed71p-24,  //  1.03584755960165879e-07 [0x3E7BCE4A4C57ED71]
    -0x1.3e99794198952p-17, // -9.49500520447803614e-06 [0xBEE3E99794198952]
    0x1.1a6206b7b94c1p-11,  //   0.00053860266686158603 [0x3F41A6206B7B94C1]
    -0x1.075b1bbf41363p-6,  //   -0.0160739680259384225 [0xBF9075B1BBF41363]
    0x1.6bbcb41034286p-3,   //     0.177606016869067129 [0x3FC6BBCB41034286]
    -0x1.2e4d699cbd01fp-4,  //   -0.0738042951086872318 [0xBFB2E4D699CBD01F]
};
/*
//
//  Coefficients for rational minimax-approximation Y0(x) =
P11(x-Z[0])/Q11(x-Z[0]) on interval (0.6,1.2).
//
*/
static const double __dy0_ep_dP11[] = {
    -0x1.06072ede6eb0bp-21, // -4.88065292895399187e-07 [0xBEA06072EDE6EB0B]
    0x1.212caab38b774p-19,  //  2.15451776754739848e-06 [0x3EC212CAAB38B774]
    -0x1.8c635d634e53bp-18, // -5.90664362514733596e-06 [0xBED8C635D634E53B]
    0x1.bf68f1546ab01p-17,  //   1.3333855044835639e-05 [0x3EEBF68F1546AB01]
    -0x1.ca1fb322ae061p-16, // -2.73063079798411224e-05 [0xBEFCA1FB322AE061]
    0x1.bbe5fbc64827fp-15,  //  5.29168096810072043e-05 [0x3F0BBE5FBC64827F]
    -0x1.8a30bd5074d1ep-14, // -9.39823123709448884e-05 [0xBF18A30BD5074D1E]
    0x1.5e18593e741d1p-13,  //  0.000166938358189940131 [0x3F25E18593E741D1]
    -0x1.58875b0a05dap-11,  // -0.000657136407656123273 [0xBF458875B0A05DA0]
    0x1.3c981b9960cbcp-10,  //   0.00120771091891995388 [0x3F53C981B9960CBC]
    0x1.6bad68791617bp-7,   //    0.0110985527154958599 [0x3F86BAD68791617B]
    -0x1.de152ed86d7dcp-9,  //  -0.00364748188823236889 [0xBF6DE152ED86D7DC]
    -0x1.c3306601ccd4fp-3,  //    -0.220307156495986395 [0xBFCC3306601CCD4F]
    -0x1.4634a7174668cp-3,  //    -0.159280114553258634 [0xBFC4634A7174668C]
    0x1.10172857a74f3p+0,   //      1.06285335674266146 [0x3FF10172857A74F3]
    0x1.c9240472622aap+0,   //      1.78570583145680439 [0x3FFC9240472622AA]
    0x1.8a05847cd958cp-1,   //     0.769573345413549159 [0x3FE8A05847CD958C]
    0x1.30482cb085eb2p-66,  //  1.61085513487833563e-20 [0x3BD30482CB085EB2]
};
static const double __dy0_ep_dQ11[] = {
    0x1.823b9b9ca6c27p-1, // 0.754361021873937498 [0x3FE823B9B9CA6C27]
    0x1.331c0e45d1ec8p+1, //  2.39929369363003175 [0x400331C0E45D1EC8]
    0x1.42960aa97934ep+1, //  2.52020390772887648 [0x40042960AA97934E]
    0x1.c00bf1e079127p-1, // 0.875091131831628455 [0x3FEC00BF1E079127]
};
/*
//
//  Coefficients for rational minimax-approximation Y0(x) = P12(x-1.2) on
interval (1.2,S[0]).
//
*/
static const double __dy0_ep_dP12[] = {
    -0x1.8e7d0b04adefdp-25, // -4.63901610783762387e-08 [0xBE68E7D0B04ADEFD]
    0x1.6cf71392a5928p-21,  //  6.79800552144312527e-07 [0x3EA6CF71392A5928]
    -0x1.41340e5c62a81p-18, // -4.78630280687875225e-06 [0xBED41340E5C62A81]
    0x1.6b0968f7a29b7p-16,  //  2.16386769964855727e-05 [0x3EF6B0968F7A29B7]
    -0x1.298dfa9409e8ap-14, // -7.09425460746431586e-05 [0xBF1298DFA9409E8A]
    0x1.7b953559ceb45p-13,  //  0.000180999205264454271 [0x3F27B953559CEB45]
    -0x1.8c0ab4c3f462fp-12, // -0.000377694912683332101 [0xBF38C0AB4C3F462F]
    0x1.5ff9bd280e7cbp-11,  //  0.000671340069863684912 [0x3F45FF9BD280E7CB]
    -0x1.146f5da9293e6p-10, //  -0.00105451592598026491 [0xBF5146F5DA9293E6]
    0x1.8d4117594f1a5p-10,  //   0.00151540474901344061 [0x3F58D4117594F1A5]
    -0x1.0d8792b4a2a88p-9,  //  -0.00205634752120903777 [0xBF60D8792B4A2A88]
    0x1.6275c5171b098p-9,   //   0.00270431548010747688 [0x3F66275C5171B098]
    -0x1.cc255261191bp-9,   //  -0.00351063376051482384 [0xBF6CC255261191B0]
    0x1.2a08802a62e25p-8,   //   0.00454762581865567495 [0x3F72A08802A62E25]
    -0x1.83598352e749p-8,   //  -0.00591048675678708391 [0xBF783598352E7490]
    0x1.fa72d617a882p-8,    //   0.00772779204240417461 [0x3F7FA72D617A8820]
    -0x1.4da058319133p-7,   //   -0.0101814680703085914 [0xBF84DA0583191330]
    0x1.bba4832d3556fp-7,   //    0.0135388985167234892 [0x3F8BBA4832D3556F]
    -0x1.2a2b3948267c9p-6,  //   -0.0181987819092695215 [0xBF92A2B3948267C9]
    0x1.962c295813703p-6,   //    0.0247908023596243969 [0x3F9962C295813703]
    -0x1.1cfc4ac4a8b02p-5,  //    -0.034788270980490324 [0xBFA1CFC4AC4A8B02]
    0x1.993acec07357ap-5,   //    0.0499547994728386097 [0x3FA993ACEC07357A]
    -0x1.d324ee4f13d79p-5,  //   -0.0570244459601054257 [0xBFAD324EE4F13D79]
    0x1.26a78540a7349p-4,   //     0.071937103765276636 [0x3FB26A78540A7349]
    -0x1.7dcc0469dd49bp-2,  //    -0.372848576508951701 [0xBFD7DCC0469DD49B]
    0x1.3e05966ac1184p-1,   //     0.621136379748847833 [0x3FE3E05966AC1184]
    0x1.d31d7198f1808p-3,   //     0.228083503227196838 [0x3FCD31D7198F1808]
};
/*
//
//  Coefficients for polynomial minimax-approximation Y0(x) = P20(x-3.0) on
interval (S[0],3.0).
//
*/
static const double __dy0_ep_dP20[] = {
    0x1.f780c47a1f652p-29,  //  3.66346647207488884e-09 [0x3E2F780C47A1F652]
    0x1.0209ab9a60da9p-26,  //  1.50197752928707927e-08 [0x3E50209AB9A60DA9]
    0x1.612066e94208bp-25,  //  4.11093433270861082e-08 [0x3E6612066E94208B]
    0x1.a4930f89e2b15p-25,  //  4.89613106300185041e-08 [0x3E6A4930F89E2B15]
    0x1.87d13dbfe854cp-24,  //  9.12270854401201367e-08 [0x3E787D13DBFE854C]
    -0x1.a2b383eb81c0ep-25, // -4.87432386363704318e-08 [0xBE6A2B383EB81C0E]
    0x1.71f77d65febefp-22,  //  3.44558392817168101e-07 [0x3E971F77D65FEBEF]
    -0x1.134b6f189ccfp-20,  // -1.02555254080372772e-06 [0xBEB134B6F189CCF0]
    0x1.6810d3d373954p-19,  //  2.68269875800597296e-06 [0x3EC6810D3D373954]
    -0x1.328a165106414p-17, // -9.13558611341690809e-06 [0xBEE328A165106414]
    0x1.7b51f19857b6bp-14,  //  9.04369575258475513e-05 [0x3F17B51F19857B6B]
    -0x1.161ce60b86825p-12, // -0.000265229115684941571 [0xBF3161CE60B86825]
    -0x1.135aa20865cf4p-9,  //  -0.00210078456820960748 [0xBF6135AA20865CF4]
    0x1.2400005778fbp-8,    //   0.00445556648580584802 [0x3F72400005778FB0]
    0x1.0224f7ebcb622p-4,   //    0.0630235371033554259 [0x3FB0224F7EBCB622]
    -0x1.13127c21922b1p-3,  //    -0.134312600874428451 [0xBFC13127C21922B1]
    -0x1.4c7773d150462p-2,  //    -0.324674424791799976 [0xBFD4C7773D150462]
    0x1.81e4f8120242ap-2,   //     0.376850010012790393 [0x3FD81E4F8120242A]
};
/*
//
//  Coefficients for polynomial minimax-approximation Y0(x) = P21(x-Z[1]) on
interval (3.0,3.875]).
//
*/
static const double __dy0_ep_dP21[] = {
    -0x1.e04257f06ed2ep-34, // -1.09198289088809894e-10 [0xBDDE04257F06ED2E]
    -0x1.9524a74ad3a5bp-32, // -3.68475573557402739e-10 [0xBDF9524A74AD3A5B]
    -0x1.5d4964eb95ed7p-30, // -1.27069759826883632e-09 [0xBE15D4964EB95ED7]
    -0x1.1e8652d35171ap-31, // -5.21185395548392619e-10 [0xBE01E8652D35171A]
    -0x1.ab3e1d7b36225p-28, // -6.21719864064446566e-09 [0xBE3AB3E1D7B36225]
    0x1.766fc0bf69673p-26,  //  2.17950748539604374e-08 [0x3E5766FC0BF69673]
    -0x1.64ad523dfa409p-25, // -4.15226719534086106e-08 [0xBE664AD523DFA409]
    -0x1.1fde6326a2dc6p-22, // -2.68098619229172061e-07 [0xBE91FDE6326A2DC6]
    -0x1.3a5061a0aba31p-18, // -4.68364342569179194e-06 [0xBED3A5061A0ABA31]
    0x1.c5bb6242ace7p-15,   //  5.40890655619492107e-05 [0x3F0C5BB6242ACE70]
    0x1.59c48af8eec4ep-13,  //  0.000164874909186570778 [0x3F259C48AF8EEC4E]
    -0x1.29eaa4d4b135fp-9,  //  -0.00227292310805397814 [0xBF629EAA4D4B135F]
    -0x1.84b04c1370173p-8,  //   -0.0059309182865380115 [0xBF784B04C1370173]
    0x1.f0c08618b7ffp-5,    //    0.0606386775086774632 [0x3FAF0C08618B7FF0]
    0x1.277f1ee9ae1cap-5,   //    0.0360713580861929534 [0x3FA277F1EE9AE1CA]
    -0x1.a392128ee4f97p-2,  //    -0.409736909839233332 [0xBFDA392128EE4F97]
    0x1.133795e84af28p-5,   //    0.0335958412054807032 [0x3FA133795E84AF28]
};
/*
//
//  Coefficients for polynomial minimax-approximation Y0(x) = P22(x-Z[1]) on
interval (3.875,S[1]).
//
*/
static const double __dy0_ep_dP22[] = {
    -0x1.505043783c734p-45, //  -3.7338302394995717e-14 [0xBD2505043783C734]
    0x1.6c89386ede5f7p-41,  //  6.47546047628242199e-13 [0x3D66C89386EDE5F7]
    -0x1.8fb562827b6fap-38, // -5.68019991062373659e-12 [0xBD98FB562827B6FA]
    0x1.3188da0e51fd2p-35,  //  3.47352599043881753e-11 [0x3DC3188DA0E51FD2]
    -0x1.77d3bf76c0281p-33, // -1.70906396423014913e-10 [0xBDE77D3BF76C0281]
    0x1.9bcf4656e19cp-31,   //  7.49077420630351483e-10 [0x3E09BCF4656E19C0]
    -0x1.f46138a4244ffp-29, // -3.64074200134057791e-09 [0xBE2F46138A4244FF]
    0x1.44ff0da589133p-26,  //  1.89172745438373793e-08 [0x3E544FF0DA589133]
    -0x1.77585a6fd29e3p-26, // -2.18479620500100719e-08 [0xBE577585A6FD29E3]
    -0x1.3b21f475e460ep-22, // -2.93490138297386638e-07 [0xBE93B21F475E460E]
    -0x1.48636fb495d97p-18, // -4.89336882888244869e-06 [0xBED48636FB495D97]
    0x1.ab2c1fe22f9b1p-15,  //   5.0922913730644551e-05 [0x3F0AB2C1FE22F9B1]
    0x1.998276518c468p-13,  //  0.000195269402455208784 [0x3F2998276518C468]
    -0x1.1e32bc4ef852dp-9,  //  -0.00218351887404879272 [0xBF61E32BC4EF852D]
    -0x1.c116fdc598103p-8,  //   -0.0068525666771110964 [0xBF7C116FDC598103]
    0x1.df6d59bf50ebdp-5,   //    0.0585238221051702437 [0x3FADF6D59BF50EBD]
    0x1.a09c9290367efp-5,   //    0.0508559095921582369 [0x3FAA09C9290367EF]
    -0x1.9c34256a12a0cp-2,  //     -0.40254267177502423 [0xBFD9C34256A12A0C]
    -0x1.5844b224f16d1p-65, // -3.64508286804151021e-20 [0xBBE5844B224F16D1]
};
/*
//
//  Coefficients for polynomial minimax-approximation Y0(x) = P3(x-Z[2]) on
interval (S[1],S[2]).
//
*/
static const double __dy0_ep_dP3[] = {
    0x1.426b9c60c3f33p-64,  //  6.82751658640515547e-20 [0x3BF426B9C60C3F33]
    -0x1.813d0a5bc8fc1p-62, //  -3.2630965064330242e-19 [0xBC1813D0A5BC8FC1]
    0x1.b9eaffce7064ap-61,  //  7.48638154025514046e-19 [0x3C2B9EAFFCE7064A]
    -0x1.27eb0bf61e546p-55, // -3.20835102481016035e-17 [0xBC827EB0BF61E546]
    0x1.397dd5f56941cp-51,  //  5.43821144365969239e-16 [0x3CC397DD5F56941C]
    0x1.334da6aa81157p-48,  //  4.26469032826821466e-15 [0x3CF334DA6AA81157]
    -0x1.067bb0ef9b268p-43, // -1.16565942932970786e-13 [0xBD4067BB0EF9B268]
    -0x1.714bfaf47ac37p-40, // -1.31200578586068431e-12 [0xBD7714BFAF47AC37]
    0x1.f14e5c0e3de3fp-36,  //  2.82685784985385882e-11 [0x3DBF14E5C0E3DE3F]
    0x1.fa6c5023992cbp-33,  //  2.30294562163613272e-10 [0x3DEFA6C5023992CB]
    -0x1.442a3d3a923cap-28, // -4.71722155310636476e-09 [0xBE3442A3D3A923CA]
    -0x1.0a2814d751ab1p-25, // -3.09847025212127179e-08 [0xBE60A2814D751AB1]
    0x1.34aa7573f923ep-21,  //  5.74934956973161399e-07 [0x3EA34AA7573F923E]
    0x1.6a9227352ee0fp-19,  //  2.70136379273782968e-06 [0x3EC6A9227352EE0F]
    -0x1.8177e4fe52432p-15, // -4.59514067068861944e-05 [0xBF08177E4FE52432]
    -0x1.26dd71e391c7ep-13, // -0.000140602597740822151 [0xBF226DD71E391C7E]
    0x1.1d35e85fde2a4p-9,   //   0.00217598401644310442 [0x3F61D35E85FDE2A4]
    0x1.b2f14a95527cbp-9,   //    0.0033183482688956315 [0x3F6B2F14A95527CB]
    -0x1.8969c64cbf452p-5,  //   -0.0480240700762598755 [0xBFA8969C64CBF452]
    -0x1.5aef611fc4d57p-6,  //   -0.0211752365567695298 [0xBF95AEF611FC4D57]
    0x1.334cca0697a5bp-2,   //     0.300097614910475208 [0x3FD334CCA0697A5B]
    -0x1.4e90da8fc3bc1p-65, // -3.54235356572481522e-20 [0xBBE4E90DA8FC3BC1]
};
/*
//
//  Coefficients for polynomial minimax-approximation Y0(x) = P(x-Z[n]) on
intervals (S[n-1],S[n])
//                               for n from 3 to 11
//
*/
static const double __dy0_ep_dP[] = {
    /* interval S[2]-S[3] */
    0x1.5fb563cb6efe3p-60,  //  1.19163493226211291e-18 [0x3C35FB563CB6EFE3]
    0x1.fd7d9b26fc9d7p-57,  //   1.3809771704162901e-17 [0x3C6FD7D9B26FC9D7]
    -0x1.f10f62040782dp-52, // -4.31130902984584289e-16 [0xBCBF10F62040782D]
    -0x1.3ba068c00701cp-48, // -4.38019895853938444e-15 [0xBCF3BA068C00701C]
    0x1.18f3a34d8c3fdp-43,  //   1.2476762314631758e-13 [0x3D418F3A34D8C3FD]
    0x1.2612f0893e289p-40,  //  1.04476066123625247e-12 [0x3D72612F0893E289]
    -0x1.ec980898cf81ap-36, // -2.80007203161024076e-11 [0xBDBEC980898CF81A]
    -0x1.99d8c29d7230fp-33, // -1.86376710002875623e-10 [0xBDE99D8C29D7230F]
    0x1.428a3a43e94dep-28,  //  4.69357403282770526e-09 [0x3E3428A3A43E94DE]
    0x1.93eb9f1a81534p-26,  //  2.35112614741835609e-08 [0x3E593EB9F1A81534]
    -0x1.2aea9ec4984d5p-21, // -5.56775341009055868e-07 [0xBEA2AEA9EC4984D5]
    -0x1.04053abf3f276p-19, // -1.93730315311966031e-06 [0xBEC04053ABF3F276]
    0x1.6afdd57be1f88p-15,  //  4.32719634480541377e-05 [0x3F06AFDD57BE1F88]
    0x1.7f84d7c50e3f1p-14,  //  9.14380353415204149e-05 [0x3F17F84D7C50E3F1]
    -0x1.fe23914fb912bp-10, //  -0.00194602560434826532 [0xBF5FE23914FB912B]
    -0x1.0325ee41e910ap-9,  //  -0.00197714360634127385 [0xBF60325EE41E910A]
    0x1.4e667a71556afp-5,   //    0.0408203498324558531 [0x3FA4E667A71556AF]
    0x1.9036451ff57c5p-7,   //     0.012213500740397518 [0x3F89036451FF57C5]
    -0x1.ff635cc72b9f1p-3,  //    -0.249701237514684787 [0xBFCFF635CC72B9F1]
    0x1.af800c1b9f7acp-64,  //  9.13737183077997196e-20 [0x3BFAF800C1B9F7AC]
    /* interval S[3]-S[4] */
    -0x1.5cc3d3c010dacp-60, // -1.18166161332086662e-18 [0xBC35CC3D3C010DAC]
    -0x1.b6f2f3cd96816p-57, // -1.18977373699603262e-17 [0xBC6B6F2F3CD96816]
    0x1.efe55ddda3895p-52,  //  4.30121184787154293e-16 [0x3CBEFE55DDDA3895]
    0x1.0a20182b52e7bp-48,  //  3.69323139838895277e-15 [0x3CF0A20182B52E7B]
    -0x1.15f0c321e437ep-43, // -1.23430367031199437e-13 [0xBD415F0C321E437E]
    -0x1.e4080fb50ac8fp-41, // -8.59812647165348132e-13 [0xBD6E4080FB50AC8F]
    0x1.e235d52edb436p-36,  //  2.74104811617766677e-11 [0x3DBE235D52EDB436]
    0x1.46ce060dd438cp-33,  //  1.48613607906102728e-10 [0x3DE46CE060DD438C]
    -0x1.371ae6ca7a2f3p-28, // -4.52717481079265235e-09 [0xBE3371AE6CA7A2F3]
    -0x1.36021c89a40bp-26,  // -1.80448549770589635e-08 [0xBE536021C89A40B0]
    0x1.1ad073122e57bp-21,  //  5.26782601781963845e-07 [0x3EA1AD073122E57B]
    0x1.7d1e2809461a2p-20,  //  1.41977443696679998e-06 [0x3EB7D1E2809461A2]
    -0x1.4f0af7d46ce1ep-15, // -3.99402194121401634e-05 [0xBF04F0AF7D46CE1E]
    -0x1.0c5f18c46cf6ep-14, // -6.39847449466748517e-05 [0xBF10C5F18C46CF6E]
    0x1.caaa76e34992fp-10,  //   0.00174967146524236303 [0x3F5CAAA76E34992F]
    0x1.5f03e47165d7p-10,   //   0.00133901674084832209 [0x3F55F03E47165D70]
    -0x1.26cab38a8b368p-5,  //   -0.0359853274024775627 [0xBFA26CAB38A8B368]
    -0x1.0bc2d84e65214p-7,  //  -0.00817142068698565455 [0xBF80BC2D84E65214]
    0x1.bf32a27594007p-3,   //     0.218358296597671336 [0x3FCBF32A27594007]
    -0x1.b8aac9e758a6fp-67, // -1.16643621610169586e-20 [0xBBCB8AAC9E758A6F]
                            /* interval S[4]-S[5] */
    0x1.5758b68f047eep-60,  //  1.16330331199700894e-18 [0x3C35758B68F047EE]
    0x1.78ac3f4310e68p-57,  //   1.0209737794269999e-17 [0x3C678AC3F4310E68]
    -0x1.e7a5cfceb3d41p-52, // -4.22966958453995578e-16 [0xBCBE7A5CFCEB3D41]
    -0x1.c0cd66ba7e979p-49, // -3.11419188188682146e-15 [0xBCEC0CD66BA7E979]
    0x1.0f1682ab5a0c3p-43,  //  1.20387225235746619e-13 [0x3D40F1682AB5A0C3]
    0x1.8f866df2c7741p-41,  //  7.09699170859982161e-13 [0x3D68F866DF2C7741]
    -0x1.d1743d944ebd8p-36, // -2.64580003559986709e-11 [0xBDBD1743D944EBD8]
    -0x1.0730c1543c6c2p-33, // -1.19685159901176758e-10 [0xBDE0730C1543C6C2]
    0x1.288585504a439p-28,  //   4.3149566838780856e-09 [0x3E3288585504A439]
    0x1.e5de01dd711f4p-27,  //   1.4140597077719688e-08 [0x3E4E5DE01DD711F4]
    -0x1.099e713932af8p-21, // -4.94753783852662823e-07 [0xBEA099E713932AF8]
    -0x1.2283a9310cdd1p-20, // -1.08225010487800158e-06 [0xBEB2283A9310CDD1]
    0x1.35d17cec017aap-15,  //  3.69332209118466704e-05 [0x3F035D17CEC017AA]
    0x1.8f91421377e51p-15,  //  4.76321476090528433e-05 [0x3F08F91421377E51]
    -0x1.a24a1215f6685p-10, //  -0.00159564719569119215 [0xBF5A24A1215F6685]
    -0x1.013b38cfb929p-10,  // -0.000981259672635879238 [0xBF5013B38CFB9290]
    0x1.0a4512039d6a2p-5,   //    0.0325036384428305652 [0x3FA0A4512039D6A2]
    0x1.862549367591ep-8,   //     0.005953150152399082 [0x3F7862549367591E]
    -0x1.925c35988ee29p-3,  //    -0.196464937895016761 [0xBFC925C35988EE29]
    0x1.ae3fccfbcaa98p-64,  //  9.11088165460361642e-20 [0x3BFAE3FCCFBCAA98]
    /* interval S[5]-S[6] */
    -0x1.4fa9208b7434dp-60, //   -1.137262528220828e-18 [0xBC34FA9208B7434D]
    -0x1.43e8ef43daa8fp-57, // -8.77959545599829912e-18 [0xBC643E8EF43DAA8F]
    0x1.db5d770e7b25ep-52,  //  4.12313497501409152e-16 [0x3CBDB5D770E7B25E]
    0x1.7c4ab0e6a222bp-49,  //  2.63880418761639903e-15 [0x3CE7C4AB0E6A222B]
    -0x1.06492d902421ap-43, //  -1.1647831654170788e-13 [0xBD406492D902421A]
    -0x1.4cdefcc3a3b7dp-41, // -5.91297756315377492e-13 [0xBD64CDEFCC3A3B7D]
    0x1.be67c1d07d4d4p-36,  //  2.53752035134328517e-11 [0x3DBBE67C1D07D4D4]
    0x1.aeb1a60e1c26dp-34,  //  9.79284641402964461e-11 [0x3DDAEB1A60E1C26D]
    -0x1.199ec88bc81a4p-28, // -4.09811396951333338e-09 [0xBE3199EC88BC81A4]
    -0x1.8654affc2fb6bp-27, // -1.13601217255801199e-08 [0xBE48654AFFC2FB6B]
    0x1.f367117733b49p-22,  //  4.65104924751823185e-07 [0x3E9F367117733B49]
    0x1.caf84db864a56p-21,  //  8.54898124779473847e-07 [0x3EACAF84DB864A56]
    -0x1.206da232a2baap-15, // -3.43833275070129231e-05 [0xBF0206DA232A2BAA]
    -0x1.37612593854ebp-15, // -3.71193265459675951e-05 [0xBF037612593854EB]
    0x1.8247b02d6b0f7p-10,  //     0.001473541381851238 [0x3F58247B02D6B0F7]
    0x1.8d6c364d9207ep-11,  //  0.000758023650356612127 [0x3F48D6C364D9207E]
    -0x1.e924b85a17361p-6,  //   -0.0298549461661993289 [0xBF9E924B85A17361]
    -0x1.2c6731071e936p-8,  //  -0.00458378741233849384 [0xBF72C6731071E936]
    0x1.70c4f66cab47fp-3,   //     0.180063176337544179 [0x3FC70C4F66CAB47F]
    -0x1.3a8340991c9b2p-66, //   -1.665016393685051e-20 [0xBBD3A8340991C9B2]
                            /* interval S[6]-S[7] */
    0x1.46aa20efbeedfp-60,  //  1.10678259726325938e-18 [0x3C346AA20EFBEEDF]
    0x1.17fa752f497f3p-57,  //  7.58882839915035014e-18 [0x3C617FA752F497F3]
    -0x1.cd19ed804c9d5p-52, // -3.99941607809179342e-16 [0xBCBCD19ED804C9D5]
    -0x1.44d978a274a2fp-49, // -2.25409619433595067e-15 [0xBCE44D978A274A2F]
    0x1.f99d64fed83f2p-44,  //  1.12269043466025587e-13 [0x3D3F99D64FED83F2]
    0x1.18b4d730f528cp-41,  //  4.98634748705043685e-13 [0x3D618B4D730F528C]
    -0x1.ab498e3fec265p-36, // -2.42884724916690614e-11 [0xBDBAB498E3FEC265]
    -0x1.668085f00f1aep-34, // -8.15139273362494716e-11 [0xBDD668085F00F1AE]
    0x1.0b9cfae2bcaafp-28,  //  3.89428464713045064e-09 [0x3E30B9CFAE2BCAAF]
    0x1.40e6a7c6beb11p-27,  //  9.33944822649622693e-09 [0x3E440E6A7C6BEB11]
    -0x1.d735e8beba051p-22, // -4.38849053026415511e-07 [0xBE9D735E8BEBA051]
    -0x1.7541247ac2cc2p-21, //  -6.9524061471880918e-07 [0xBEA7541247AC2CC2]
    0x1.0e6afa01af57bp-15,  //  3.22363230344886045e-05 [0x3F00E6AFA01AF57B]
    0x1.f632a9396fb31p-16,  //   2.9933327118088208e-05 [0x3EFF632A9396FB31]
    -0x1.6853b2d047885p-10, //  -0.00137453822032499004 [0xBF56853B2D047885]
    -0x1.3ec49cb941f52p-11, // -0.000608001740290279322 [0xBF43EC49CB941F52]
    0x1.c6c415c971b4bp-6,   //    0.0277567112696275702 [0x3F9C6C415C971B4B]
    0x1.e0ee8ec84659ap-9,   //   0.00366921894977840796 [0x3F6E0EE8EC84659A]
    -0x1.5664d37c37d7bp-3,  //    -0.167184498051010738 [0xBFC5664D37C37D7B]
    -0x1.d57213607b311p-66, // -2.48522435663682539e-20 [0xBBDD57213607B311]
    /* interval S[7]-S[8] */
    -0x1.3d189fd596a01p-60, // -1.07436367771484024e-18 [0xBC33D189FD596A01]
    -0x1.e75971797d4d9p-58, //   -6.604815811765323e-18 [0xBC5E75971797D4D9]
    0x1.be316851c33cap-52,  //  3.87010734256058725e-16 [0x3CBBE316851C33CA]
    0x1.181cba6fa9a8ap-49,  //  1.94366897431340986e-15 [0x3CE181CBA6FA9A8A]
    -0x1.e6bfb03e29e4ep-44, // -1.08079941219444907e-13 [0xBD3E6BFB03E29E4E]
    -0x1.df6a85b4343f6p-42, // -4.25807036439304025e-13 [0xBD5DF6A85B4343F6]
    0x1.992136ca07e3p-36,   //  2.32563333080738567e-11 [0x3DB992136CA07E30]
    0x1.2f44ce5819dap-34,   //   6.8955335691901985e-11 [0x3DD2F44CE5819DA0]
    -0x1.fdbeb1072d26bp-29, // -3.70888220447093147e-09 [0xBE2FDBEB1072D26B]
    -0x1.0d1fded8af247p-27, // -7.83255364852240451e-09 [0xBE40D1FDED8AF247]
    0x1.be87e2cad253cp-22,  //  4.15864218329486414e-07 [0x3E9BE87E2CAD253C]
    0x1.36bd2d58f56c1p-21,  //   5.7879644110399921e-07 [0x3EA36BD2D58F56C1]
    -0x1.fe48825ed3c8fp-16, // -3.04152512129977197e-05 [0xBEFFE48825ED3C8F]
    -0x1.9fc04c675bf1p-16,  // -2.47807005541714393e-05 [0xBEF9FC04C675BF10]
    0x1.52d29a06802ep-10,   //   0.00129250588508982717 [0x3F552D29A06802E0]
    0x1.06f735cc0f6a3p-11,  //  0.000501567200906289832 [0x3F406F735CC0F6A3]
    -0x1.aab099314b209p-6,  //   -0.0260430809041043744 [0xBF9AAB099314B209]
    -0x1.8c37a29c4586fp-9,  //  -0.00302289829245255088 [0xBF68C37A29C4586F]
    0x1.40f8ffdf09a5fp-3,   //      0.15672492885024078 [0x3FC40F8FFDF09A5F]
    -0x1.3dbb13f38b65p-66,  // -1.68205270776814216e-20 [0xBBD3DBB13F38B650]
                            /* interval S[8]-S[9] */
    0x1.33737490fa56p-60,   //  1.04168449789286804e-18 [0x3C333737490FA560]
    0x1.ab60c99503e8bp-58,  //  5.79205297952426241e-18 [0x3C5AB60C99503E8B]
    -0x1.af6d1b0912158p-52, // -3.74202573248970553e-16 [0xBCBAF6D1B0912158]
    -0x1.e7b9e972cd23fp-50, // -1.69214024178293577e-15 [0xBCDE7B9E972CD23F]
    0x1.d4b25aebc356bp-44,  //  1.04071573546435473e-13 [0x3D3D4B25AEBC356B]
    0x1.9e4ec824bc1bp-42,   //  3.67979195068258872e-13 [0x3D59E4EC824BC1B0]
    -0x1.8850c8f878e43p-36, // -2.23005580760350095e-11 [0xBDB8850C8F878E43]
    -0x1.04444607dd04p-34,  // -5.91777947156347406e-11 [0xBDD04444607DD040]
    0x1.e6d78878b936fp-29,  //  3.54224121940928677e-09 [0x3E2E6D78878B936F]
    0x1.cb10b3a95e17cp-28,  //  6.68027847740937192e-09 [0x3E3CB10B3A95E17C]
    -0x1.a8ea97b66e824p-22, // -3.95734214643876645e-07 [0xBE9A8EA97B66E824]
    -0x1.07a7a07456ba7p-21, // -4.91095319559018674e-07 [0xBEA07A7A07456BA7]
    0x1.e4158391f2c08p-16,  //  2.88536571771240695e-05 [0x3EFE4158391F2C08]
    0x1.5f61b666127ffp-16,  //  2.09439807792657883e-05 [0x3EF5F61B666127FF]
    -0x1.40aaa5d94bd8dp-10, //    -0.001223245976074458 [0xBF540AAA5D94BD8D]
    -0x1.bb6aa3d4e9e73p-12, // -0.000422874987004529651 [0xBF3BB6AA3D4E9E73]
    0x1.9336443318ed1p-6,   //    0.0246101060878476975 [0x3F99336443318ED1]
    0x1.4dbf6a9fb80d7p-9,   //   0.00254629303513818515 [0x3F64DBF6A9FB80D7]
    -0x1.2f206e49909c7p-3,  //    -0.148011075611135662 [0xBFC2F206E49909C7]
    0x1.9836354209655p-64,  //  8.64422004587530529e-20 [0x3BF9836354209655]
    /* interval S[9]-S[10] */
    -0x1.2a07a7481e725p-60, // -1.00976456561767775e-18 [0xBC32A07A7481E725]
    -0x1.79a6b5c91c60bp-58, // -5.11812829704447218e-18 [0xBC579A6B5C91C60B]
    0x1.a138ba3ff01a2p-52,  //  3.61882045116364085e-16 [0x3CBA138BA3FF01A2]
    0x1.ac858d99227e1p-50,  //   1.4867332776932729e-15 [0x3CDAC858D99227E1]
    -0x1.c3c47396d619dp-44, // -1.00312511353292808e-13 [0xBD3C3C47396D619D]
    -0x1.69f50c705c067p-42, // -3.21482592593560702e-13 [0xBD569F50C705C067]
    0x1.78eafa534527dp-36,  //  2.14253010517809722e-11 [0x3DB78EAFA534527D]
    0x1.c462d71faaaebp-35,  //  5.14303445728651823e-11 [0x3DCC462D71FAAAEB]
    -0x1.d24fc943e9253p-29, // -3.39286390818885502e-09 [0xBE2D24FC943E9253]
    -0x1.8d22635b5a2f9p-28, // -5.77906508355427472e-09 [0xBE38D22635B5A2F9]
    0x1.95e4b1021714dp-22,  //  3.78017617324597561e-07 [0x3E995E4B1021714D]
    0x1.c672d4d6e7ebcp-22,  //  4.23238203090311516e-07 [0x3E9C672D4D6E7EBC]
    -0x1.cd5d4a9d789d9p-16, // -2.74994623533727428e-05 [0xBEFCD5D4A9D789D9]
    -0x1.2df9afa52111bp-16, // -1.79991326558918848e-05 [0xBEF2DF9AFA52111B]
    0x1.311781633515p-10,   //   0.00116383292406848152 [0x3F53117816335150]
    0x1.7c65c9302c536p-12,  //  0.000362775422227806939 [0x3F37C65C9302C536]
    -0x1.7f3506d4a1231p-6,  //   -0.0233891073899192316 [0xBF97F3506D4A1231]
    -0x1.1e2035324643cp-9,  //  -0.00218296670312814571 [0xBF61E2035324643C]
    0x1.1ff5ebddd3c3ap-3,   //     0.140605776507500668 [0x3FC1FF5EBDDD3C3A]
    0x1.04370896c67bp-64,   //   5.5102664257666096e-20 [0x3BF04370896C67B0]
                            /* interval S[10]-S[11] */
    0x1.20ffb5ca66bf8p-60,  //  9.79166250491478898e-19 [0x3C320FFB5CA66BF8]
    0x1.5028bb8dedcf6p-58,  //   4.5558054921501804e-18 [0x3C55028BB8DEDCF6]
    -0x1.93c7362d60b24p-52, // -3.50221735665363431e-16 [0xBCB93C7362D60B24]
    -0x1.7bb728f79beb6p-50, // -1.31740267604670205e-15 [0xBCD7BB728F79BEB6]
    0x1.b40cb7509e29ep-44,  //  9.68224771832549505e-14 [0x3D3B40CB7509E29E]
    0x1.3f5129847f0aap-42,  //  2.83610503755354973e-13 [0x3D53F5129847F0AA]
    -0x1.6adfbc9fc5e6cp-36, // -2.06269971798496824e-11 [0xBDB6ADFBC9FC5E6C]
    -0x1.8d7dbbd895c12p-35, //  -4.5189511587639728e-11 [0xBDC8D7DBBD895C12]
    0x1.bfe1396c9cc28p-29,  //  3.25875431353616787e-09 [0x3E2BFE1396C9CC28]
    0x1.5bba73dd3a714p-28,  //  5.06011318716592884e-09 [0x3E35BBA73DD3A714]
    -0x1.850ae878bfa22p-22, // -3.62324164934677983e-07 [0xBE9850AE878BFA22]
    -0x1.8cc82a70c9697p-22, // -3.69531938425969048e-07 [0xBE98CC82A70C9697]
    0x1.b974781a52668p-16,  //   2.6312765933489418e-05 [0x3EFB974781A52668]
    0x1.0719d13e00d02p-16,  //  1.56820326466912822e-05 [0x3EF0719D13E00D02]
    -0x1.238cfc13ac771p-10, //  -0.00111217773966784347 [0xBF5238CFC13AC771]
    -0x1.4afdf89fca616p-12, // -0.000315658640817591068 [0xBF34AFDF89FCA616]
    0x1.6de64242a831p-6,    //    0.0223327300142300289 [0x3F96DE64242A8310]
    0x1.f1aee31818d18p-10,  //   0.00189851056168139319 [0x3F5F1AEE31818D18]
    -0x1.12dd55d4be2b3p-3,  //    -0.134211226038834036 [0xBFC12DD55D4BE2B3]
    0x1.2be9adc4ecf3ep-63,  //  1.27018014737365197e-19 [0x3C02BE9ADC4ECF3E]
};
/*
//
//  Coefficients for polynomial pade-approximation P0(x) = PP(1296/x^2) in point
1296/x^2 = 0.5
//
*/
static const double __dy0_ep_dPP[] = {
    -0x1.1d33e22820ce2p-65, // -3.01970072274910112e-20 [0xBBE1D33E22820CE2]
    0x1.c21171d1b716bp-61,  //   7.6244509124953826e-19 [0x3C2C21171D1B716B]
    -0x1.646d4878d7441p-56, // -1.93219402655167118e-17 [0xBC7646D4878D7441]
    0x1.7197abb91691bp-51,  //  6.41140723905704152e-16 [0x3CC7197ABB91691B]
    -0x1.0f08e21634915p-45, // -3.00908964201284839e-14 [0xBD20F08E21634915]
    0x1.2f0474e0576bdp-39,  //  2.15306818341308315e-12 [0x3D82F0474E0576BD]
    -0x1.212ce9049c4b5p-32, //  -2.6300352198873972e-10 [0xBDF212CE9049C4B5]
    0x1.1ec919de85abdp-24,  //  6.67724644260202629e-08 [0x3E71EC919DE85ABD]
    -0x1.c71c71c71c71cp-15, // -5.42534722222222192e-05 [0xBF0C71C71C71C71C]
    0x1p+0,                 //                        1 [0x3FF0000000000000]
};
/*
//
//  Coefficients for polynomial pade-approximation Q0(x) = QP(1296/x^2)*(36/x))
in point 1296/x^2 = 0.5
//
*/
static const double __dy0_ep_dQP[] = {
    0x1.13ff08ba744efp-67,  //  7.30555929783910137e-21 [0x3BC13FF08BA744EF]
    -0x1.8dc7d0c1febd4p-63, // -1.68466606979394302e-19 [0xBC08DC7D0C1FEBD4]
    0x1.1562ae18f1c51p-58,  //  3.75927410153957425e-18 [0x3C51562AE18F1C51]
    -0x1.ed914f42e626p-54,  // -1.07025416238145711e-16 [0xBC9ED914F42E6260]
    0x1.2dd57e26be263p-48,  //    4.188787596874229e-15 [0x3CF2DD57E26BE263]
    -0x1.0e48b730d814fp-42, // -2.40060456197827542e-13 [0xBD50E48B730D814F]
    0x1.83dcbea2882f2p-36,  //  2.20474182616261576e-11 [0x3DB83DCBEA2882F2]
    -0x1.021b641511e22p-28, // -3.75595112396356956e-09 [0xBE3021B641511E22]
    0x1.a5663075fde4ap-20,  //  1.56983426568930047e-06 [0x3EBA5663075FDE4A]
    -0x1.c71c71c71c71cp-9,  //  -0.00347222222222222203 [0xBF6C71C71C71C71C]
};
inline int __devicelib_imf_internal_dy0(const double *a, double *r) {
  int nRet = 0;
  int i = 0;
  int32_t ixhi = 0, ixlo = 0, iabsxhi = 0, isignxhi = 0;
  int64_t lx = 0;
  double x = *a, lnx = 0, ps = 0, pc = 0, xi = 0, y = 0, z = 0, p = 0, q = 0,
         dr = 0, r1 = 0, result = 0;
  const double dtonpi = 0x1.45f306dc9c883p-1; // 6.366197723675813430755e-01
  const uint64_t linf = 0x7ff0000000000000;   // +Inf
  lx = *(int64_t *)&x;
  ixhi = ((int32_t *)&x)[1];
  ixlo = ((int32_t *)&x)[0];
  iabsxhi = ixhi & ~0x80000000;
  isignxhi = ixhi & 0x80000000;
  /* x is not INF or NaN */
  if (iabsxhi < 0x7FF00000) {
    /* x > 0 */
    if (lx > 0) {
      /* Rational or polynomial approximation of y0(x) */
      if (x <= __dy0_ep_dS[11]) {
        if (x < 0.6) {
          /* y0(x)=P0(x^2)+2/pi*j0(x)*log(x) */
          y = x * x;
          z = y * y;
          p = (((__dy0_ep_dP0[0] * z + __dy0_ep_dP0[2]) * z + __dy0_ep_dP0[4]) *
                   z +
               __dy0_ep_dP0[6]) *
              y;
          q = ((__dy0_ep_dP0[1] * z + __dy0_ep_dP0[3]) * z + __dy0_ep_dP0[5]) *
                  z +
              __dy0_ep_dP0[7];
          result = p + q;
          __dy0_ep_ln_kernel_fp64(x, &lnx);
          result += dtonpi * (__dy0_ep_small_j0(x)) * lnx;
        } else if (x < 1.2) {
          x = x - __dy0_ep_dZ_MP[0];
          x = x - __dy0_ep_dZ_MP[1];
          y = x * x;
          p = ((((((((__dy0_ep_dP11[0] * y + __dy0_ep_dP11[2]) * y +
                     __dy0_ep_dP11[4]) *
                        y +
                    __dy0_ep_dP11[6]) *
                       y +
                   __dy0_ep_dP11[8]) *
                      y +
                  __dy0_ep_dP11[10]) *
                     y +
                 __dy0_ep_dP11[12]) *
                    y +
                __dy0_ep_dP11[14]) *
                   y +
               __dy0_ep_dP11[16]) *
              x;
          q = (((((((__dy0_ep_dP11[1] * y + __dy0_ep_dP11[3]) * y +
                    __dy0_ep_dP11[5]) *
                       y +
                   __dy0_ep_dP11[7]) *
                      y +
                  __dy0_ep_dP11[9]) *
                     y +
                 __dy0_ep_dP11[11]) *
                    y +
                __dy0_ep_dP11[13]) *
                   y +
               __dy0_ep_dP11[15]) *
                  y +
              __dy0_ep_dP11[17];
          dr = ((__dy0_ep_dQ11[0] * x + __dy0_ep_dQ11[1]) * x +
                __dy0_ep_dQ11[2]) *
                   x +
               __dy0_ep_dQ11[3];
          result = (p + q) / dr;
        } else if (x < __dy0_ep_dS[0]) {
          x -= 1.2;
          y = x * x;
          p = ((((((((((((__dy0_ep_dP12[0] * y + __dy0_ep_dP12[2]) * y +
                         __dy0_ep_dP12[4]) *
                            y +
                        __dy0_ep_dP12[6]) *
                           y +
                       __dy0_ep_dP12[8]) *
                          y +
                      __dy0_ep_dP12[10]) *
                         y +
                     __dy0_ep_dP12[12]) *
                        y +
                    __dy0_ep_dP12[14]) *
                       y +
                   __dy0_ep_dP12[16]) *
                      y +
                  __dy0_ep_dP12[18]) *
                     y +
                 __dy0_ep_dP12[20]) *
                    y +
                __dy0_ep_dP12[22]) *
                   y +
               __dy0_ep_dP12[24]) *
                  y +
              __dy0_ep_dP12[26];
          q = ((((((((((((__dy0_ep_dP12[1] * y + __dy0_ep_dP12[3]) * y +
                         __dy0_ep_dP12[5]) *
                            y +
                        __dy0_ep_dP12[7]) *
                           y +
                       __dy0_ep_dP12[9]) *
                          y +
                      __dy0_ep_dP12[11]) *
                         y +
                     __dy0_ep_dP12[13]) *
                        y +
                    __dy0_ep_dP12[15]) *
                       y +
                   __dy0_ep_dP12[17]) *
                      y +
                  __dy0_ep_dP12[19]) *
                     y +
                 __dy0_ep_dP12[21]) *
                    y +
                __dy0_ep_dP12[23]) *
                   y +
               __dy0_ep_dP12[25]) *
              x;
          result = p + q;
        } else if (x < 3.0) {
          x -= 3.0;
          y = x * x;
          p = ((((((((__dy0_ep_dP20[0] * y + __dy0_ep_dP20[2]) * y +
                     __dy0_ep_dP20[4]) *
                        y +
                    __dy0_ep_dP20[6]) *
                       y +
                   __dy0_ep_dP20[8]) *
                      y +
                  __dy0_ep_dP20[10]) *
                     y +
                 __dy0_ep_dP20[12]) *
                    y +
                __dy0_ep_dP20[14]) *
                   y +
               __dy0_ep_dP20[16]) *
              x;
          q = (((((((__dy0_ep_dP20[1] * y + __dy0_ep_dP20[3]) * y +
                    __dy0_ep_dP20[5]) *
                       y +
                   __dy0_ep_dP20[7]) *
                      y +
                  __dy0_ep_dP20[9]) *
                     y +
                 __dy0_ep_dP20[11]) *
                    y +
                __dy0_ep_dP20[13]) *
                   y +
               __dy0_ep_dP20[15]) *
                  y +
              __dy0_ep_dP20[17];
          result = p + q;
        } else if (x < 3.875) {
          x -= 3.875;
          y = x * x;
          p = (((((((__dy0_ep_dP21[0] * y + __dy0_ep_dP21[2]) * y +
                    __dy0_ep_dP21[4]) *
                       y +
                   __dy0_ep_dP21[6]) *
                      y +
                  __dy0_ep_dP21[8]) *
                     y +
                 __dy0_ep_dP21[10]) *
                    y +
                __dy0_ep_dP21[12]) *
                   y +
               __dy0_ep_dP21[14]) *
                  y +
              __dy0_ep_dP21[16];
          q = (((((((__dy0_ep_dP21[1] * y + __dy0_ep_dP21[3]) * y +
                    __dy0_ep_dP21[5]) *
                       y +
                   __dy0_ep_dP21[7]) *
                      y +
                  __dy0_ep_dP21[9]) *
                     y +
                 __dy0_ep_dP21[11]) *
                    y +
                __dy0_ep_dP21[13]) *
                   y +
               __dy0_ep_dP21[15]) *
              x;
          result = p + q;
        } else if (x <= __dy0_ep_dS[1]) {
          x = x - __dy0_ep_dZ_MP[2];
          x = x - __dy0_ep_dZ_MP[3];
          y = x * x;
          p = ((((((((__dy0_ep_dP22[0] * y + __dy0_ep_dP22[2]) * y +
                     __dy0_ep_dP22[4]) *
                        y +
                    __dy0_ep_dP22[6]) *
                       y +
                   __dy0_ep_dP22[8]) *
                      y +
                  __dy0_ep_dP22[10]) *
                     y +
                 __dy0_ep_dP22[12]) *
                    y +
                __dy0_ep_dP22[14]) *
                   y +
               __dy0_ep_dP22[16]) *
                  y +
              __dy0_ep_dP22[18];
          q = ((((((((__dy0_ep_dP22[1] * y + __dy0_ep_dP22[3]) * y +
                     __dy0_ep_dP22[5]) *
                        y +
                    __dy0_ep_dP22[7]) *
                       y +
                   __dy0_ep_dP22[9]) *
                      y +
                  __dy0_ep_dP22[11]) *
                     y +
                 __dy0_ep_dP22[13]) *
                    y +
                __dy0_ep_dP22[15]) *
                   y +
               __dy0_ep_dP22[17]) *
              x;
          result = p + q;
        } else if (x < __dy0_ep_dS[2]) {
          x = x - __dy0_ep_dZ_MP[4];
          x = x - __dy0_ep_dZ_MP[5];
          y = x * x;
          p = ((((((((((__dy0_ep_dP3[0] * y + __dy0_ep_dP3[2]) * y +
                       __dy0_ep_dP3[4]) *
                          y +
                      __dy0_ep_dP3[6]) *
                         y +
                     __dy0_ep_dP3[8]) *
                        y +
                    __dy0_ep_dP3[10]) *
                       y +
                   __dy0_ep_dP3[12]) *
                      y +
                  __dy0_ep_dP3[14]) *
                     y +
                 __dy0_ep_dP3[16]) *
                    y +
                __dy0_ep_dP3[18]) *
                   y +
               __dy0_ep_dP3[20]) *
              x;
          q = (((((((((__dy0_ep_dP3[1] * y + __dy0_ep_dP3[3]) * y +
                      __dy0_ep_dP3[5]) *
                         y +
                     __dy0_ep_dP3[7]) *
                        y +
                    __dy0_ep_dP3[9]) *
                       y +
                   __dy0_ep_dP3[11]) *
                      y +
                  __dy0_ep_dP3[13]) *
                     y +
                 __dy0_ep_dP3[15]) *
                    y +
                __dy0_ep_dP3[17]) *
                   y +
               __dy0_ep_dP3[19]) *
                  y +
              __dy0_ep_dP3[21];
          result = p + q;
        } else {
          for (i = 3; (i < 11) && (x >= __dy0_ep_dS[i]); i++)
            ;
          x = x - __dy0_ep_dZ_MP[i * 2 + 0];
          x = x - __dy0_ep_dZ_MP[i * 2 + 1];
          i = (i - 3) * 20;
          y = x * x;
          p = (((((((((__dy0_ep_dP[i + 0] * y + __dy0_ep_dP[i + 2]) * y +
                      __dy0_ep_dP[i + 4]) *
                         y +
                     __dy0_ep_dP[i + 6]) *
                        y +
                    __dy0_ep_dP[i + 8]) *
                       y +
                   __dy0_ep_dP[i + 10]) *
                      y +
                  __dy0_ep_dP[i + 12]) *
                     y +
                 __dy0_ep_dP[i + 14]) *
                    y +
                __dy0_ep_dP[i + 16]) *
                   y +
               __dy0_ep_dP[i + 18]) *
              x;
          q = ((((((((__dy0_ep_dP[i + 1] * y + __dy0_ep_dP[i + 3]) * y +
                     __dy0_ep_dP[i + 5]) *
                        y +
                    __dy0_ep_dP[i + 7]) *
                       y +
                   __dy0_ep_dP[i + 9]) *
                      y +
                  __dy0_ep_dP[i + 11]) *
                     y +
                 __dy0_ep_dP[i + 13]) *
                    y +
                __dy0_ep_dP[i + 15]) *
                   y +
               __dy0_ep_dP[i + 17]) *
                  y +
              __dy0_ep_dP[i + 19];
          result = p + q;
        }
      }
      /* Hancels asymptotic forms */
      else {
        xi = (1.0 / x);
        z = (36.0 * xi);
        y = z * z;
        p = ((((((((__dy0_ep_dPP[0] * y + __dy0_ep_dPP[1]) * y +
                   __dy0_ep_dPP[2]) *
                      y +
                  __dy0_ep_dPP[3]) *
                     y +
                 __dy0_ep_dPP[4]) *
                    y +
                __dy0_ep_dPP[5]) *
                   y +
               __dy0_ep_dPP[6]) *
                  y +
              __dy0_ep_dPP[7]) *
                 y +
             __dy0_ep_dPP[8]) *
                y +
            __dy0_ep_dPP[9];
        q = (((((((((__dy0_ep_dQP[0] * y + __dy0_ep_dQP[1]) * y +
                    __dy0_ep_dQP[2]) *
                       y +
                   __dy0_ep_dQP[3]) *
                      y +
                  __dy0_ep_dQP[4]) *
                     y +
                 __dy0_ep_dQP[5]) *
                    y +
                __dy0_ep_dQP[6]) *
                   y +
               __dy0_ep_dQP[7]) *
                  y +
              __dy0_ep_dQP[8]) *
                 y +
             __dy0_ep_dQP[9]) *
            z;
        __dy0_ep_sincos_kernel_fp64(x, -1, &ps, &pc);
        r1 = p * ps;
        dr = r1 + (q * pc);
        result = __sqrt(dtonpi * xi) * dr;
      }
    } else {
      /* x <= 0 */
      if ((iabsxhi | ixlo) != 0) {
        result = (*(double *)&linf) * 0.0; // NaN
      } else {
        result = -1.0 / 0.0; // -Inf
      }
    }
  } else /* x is INF or NaN */
  {
    /* SIGNIFICAND_NONZERO_64(ptx) */
    if (((iabsxhi & ~0x7FF00000) | ixlo) != 0) {
      /* y0(NaN) = QNaN */
      result = x * 1.0;
    } else if (isignxhi) {
      result = (*(double *)&linf) * 0.0; // NaN
    } else {
      result = 0.0;
    }
  }
  *r = result;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_y0_d_ep */
DEVICE_EXTERN_C_INLINE double __devicelib_imf_y0(double x) {
  using namespace __imf_impl_y0_d_ep;
  double r;
  __devicelib_imf_internal_dy0(&x, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
