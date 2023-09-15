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
namespace __imf_impl_y1_d_ep {
namespace {
struct fp64 { /*/ sign:1 exponent:11 significand:52 (implied leading 1)*/
  unsigned lo_significand : 32;
  unsigned hi_significand : 20;
  unsigned exponent : 11;
  unsigned sign : 1;
};
static const uint32_t __dy1_ep__DP[] = {
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
static const uint64_t __dy1_ep__DP3[] = {
    /* Pi/4, split by 24 bits */
    0x3fe921fb40000000, /* .785398125648498535156250000000000000e-00 */
    0x3e64442d00000000, /* .377489470793079817667603492736816406e-07 */
    0x3ce8469880000000, /* .269515126497888238277234052020503440e-14 */
    0x3b68cc51701b839a, /* .164100177143675023722023662896821847e-21 */
};
static const uint64_t __dy1_ep__DP2[] = {
    /* Pi/4, split by 36 bits */
    0x3fe921fb54440000, /* .785398163396166637539863586425781250e-00 */
    0x3d768c234c400000, /* .128167207563315921695079602216083003e-11 */
    0x3b68cc51701b839a, /* .164100177143675023722023662896821847e-21 */
};
/* approximation of (sin(x)/x-1)/(x^2) on |x|=[0..Pi/4], max.err .28e-13 */
static const uint64_t __dy1_ep__SP[] = {
    0xbfc555555555516d, /* -.1666666666666389001574092663227e-0 SP[0] */
    0x3f81111110fd4208, /*  .8333333331081340727248579433954e-2 SP[1] */
    0xbf2a019fd9bd0882, /* -.1984126691870238094119659272295e-3 SP[2] */
    0x3ec71d9aa585bfc4, /*  .2755599137475836194220161698382e-5 SP[3] */
    0xbe5aa2880297fc43, /* -.2480567232697144802048174630580e-7 SP[4] */
};
/* approximation of (cos(x)-1)/(x^2) on |x|=[0..Pi/4], max.err .36e-12 */
static const uint64_t __dy1_ep__CP[] = {
    0xbfdfffffffffe6a2, /* -.4999999999996395247842975513023e-0 CP[0] */
    0x3fa5555555150951, /*  .4166666663742780406412103193419e-1 CP[1] */
    0xbf56c16bae710ff8, /* -.1388888509397110798760860956647e-2 CP[2] */
    0x3efa01299942ab00, /*  .2479986285723167468258822162782e-4 CP[3] */
    0xbe9247507b5ee59e, /* -.2723719448833349736164120625340e-6 CP[4] */
};
/* |x| >= 2^30, use Payne and Hanek algorithm */
static int __dy1_ep_reduce_pi04d(double x, double *y, int n) {
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
    yl = (xl * ((const double *)__dy1_ep__DP)[j + 0] +
          x * ((const double *)__dy1_ep__DP)[j + 1]);
    j++;
    t = yl;
    ptt->lo_significand &= 0xfff00000;
    yl -= t;
  } else {
    yl = zero_none[0]; /*0.0*/
  }
  yh = (xl * ((const double *)__dy1_ep__DP)[j + 0] +
        x * ((const double *)__dy1_ep__DP)[j + 1]);
  j++;
  yl = yl + yh;
  yh = (xl * ((const double *)__dy1_ep__DP)[j + 0] +
        x * ((const double *)__dy1_ep__DP)[j + 1]);
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
    *y += (xl * ((const double *)__dy1_ep__DP)[j] +
           x * ((const double *)__dy1_ep__DP)[j + 1]);
    j++;
  }
  *y *= PI04;
  return k;
}
static int __dy1_ep_sincos_kernel_fp64(double x, int n, double *psn,
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
        x = x - j * ((const double *)__dy1_ep__DP2)[0] -
            j * ((const double *)__dy1_ep__DP2)[1] -
            j * ((const double *)__dy1_ep__DP2)[2];
      } else {
        x = x - j * ((const double *)__dy1_ep__DP3)[0] -
            j * ((const double *)__dy1_ep__DP3)[1] -
            j * ((const double *)__dy1_ep__DP3)[2] -
            j * ((const double *)__dy1_ep__DP3)[3];
      }
    }
  } else /* if |x| >= 2^30 use Payne and Hanek algorithm */
  {
    k = 1 + __dy1_ep_reduce_pi04d(x, &x, n);
  }
  csign = ((k + 2) >> 2) & 1;
  ssign ^= (k >> 2) & 1;
  y = x * x;
  z = y * y;
  cs1 = (((const double *)__dy1_ep__CP)[3] * z +
         ((const double *)__dy1_ep__CP)[1]) *
            z +
        ones[0];
  sn1 = (((const double *)__dy1_ep__SP)[3] * z +
         ((const double *)__dy1_ep__SP)[1]) *
            z * x +
        x;
  cs2 = ((((const double *)__dy1_ep__CP)[4] * z +
          ((const double *)__dy1_ep__CP)[2]) *
             z +
         ((const double *)__dy1_ep__CP)[0]) *
        y;
  sn2 = ((((const double *)__dy1_ep__SP)[4] * z +
          ((const double *)__dy1_ep__SP)[2]) *
             z +
         ((const double *)__dy1_ep__SP)[0]) *
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
} __dy1_ep___c19 = {0xbfb6e22682c05596UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy1_ep___c18 = {0x3fb6c694b21a9875UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy1_ep___c17 = {0xbfa68f0acee35e2dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy1_ep___c16 = {0x3fa9474ccd075ce5UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy1_ep___c15 = {0xbfb0750f4f9c34f9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy1_ep___c14 = {0x3fb16608748ab72dUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy1_ep___c13 = {0xbfb23e2ec341eba0UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy1_ep___c12 = {0x3fb3aa521d980cd0UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy1_ep___c11 = {0xbfb555fa23866d76UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy1_ep___c10 = {0x3fb74629a554d880UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy1_ep___c9 = {0xbfb999938abcf213UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy1_ep___c8 = {0x3fbc71c472fb2195UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy1_ep___c7 = {0xbfc00000112830d9UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy1_ep___c6 = {0x3fc24924982c2697UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy1_ep___c5 = {0xbfc55555551fbbdbUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy1_ep___c4 = {0x3fc99999998c68b5UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy1_ep___c3 = {0xbfd0000000002697UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy1_ep___c2 = {0x3fd5555555555b0eUL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy1_ep___c1 = {0xbfdffffffffffff0UL};
static const union {
  uint64_t w;
  uint32_t w32[2];
  int32_t s32[2];
  double f;
} __dy1_ep___c0 = {0xbc8a30cfded694ffUL};
static int __dy1_ep_ln_kernel_fp64(double x, double *r) {
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
  poly = __fma(__dy1_ep___c19.f, R, __dy1_ep___c18.f);
  poly = __fma(poly, R, __dy1_ep___c17.f);
  poly = __fma(poly, R, __dy1_ep___c16.f);
  poly = __fma(poly, R, __dy1_ep___c15.f);
  poly = __fma(poly, R, __dy1_ep___c14.f);
  poly = __fma(poly, R, __dy1_ep___c13.f);
  poly = __fma(poly, R, __dy1_ep___c12.f);
  poly = __fma(poly, R, __dy1_ep___c11.f);
  poly = __fma(poly, R, __dy1_ep___c10.f);
  poly = __fma(poly, R, __dy1_ep___c9.f);
  poly = __fma(poly, R, __dy1_ep___c8.f);
  poly = __fma(poly, R, __dy1_ep___c7.f);
  poly = __fma(poly, R, __dy1_ep___c6.f);
  poly = __fma(poly, R, __dy1_ep___c5.f);
  poly = __fma(poly, R, __dy1_ep___c4.f);
  poly = __fma(poly, R, __dy1_ep___c3.f);
  poly = __fma(poly, R, __dy1_ep___c2.f);
  poly = __fma(poly, R, __dy1_ep___c1.f);
  poly = __fma(poly, R, __dy1_ep___c0.f);
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
//  Coefficients for polynomial minimax-approximation J1(x) = P1(x) on interval
(0,s[0]).
//
*/
static const double __dy1_ep_dJ1[] = {
    -0x1.442f0fe360f4dp-61, // -5.49188780431403671e-19 [0xBC2442F0FE360F4D]
    -0x1.deb930b84dcap-57,  // -1.29758237482006856e-17 [0xBC6DEB930B84DCA0]
    0x1.64f9909afcfe5p-51,  //  6.19252674752346552e-16 [0x3CC64F9909AFCFE5]
    -0x1.161028cdf8a9ap-51, // -4.82362626626922211e-16 [0xBCC161028CDF8A9A]
    -0x1.4e58bf2a524f1p-43, // -1.48479747142391555e-13 [0xBD44E58BF2A524F1]
    -0x1.3fd3cb961ebc1p-48, // -4.43849575471100773e-15 [0xBCF3FD3CB961EBC1]
    0x1.27f8f0466f708p-35,  //   3.3648168062842007e-11 [0x3DC27F8F0466F708]
    -0x1.eccd22ab9bad3p-47, //  -1.3677984030887287e-14 [0xBD0ECCD22AB9BAD3]
    -0x1.845c4025d7598p-28, //  -5.6513869468182652e-09 [0xBE3845C4025D7598]
    -0x1.1395f52ff2d85p-46, // -1.52980834950567006e-14 [0xBD11395F52FF2D85]
    0x1.6c16c1cf5627bp-21,  //   6.7816841379648086e-07 [0x3EA6C16C1CF5627B]
    -0x1.b4d5998901168p-48, // -6.06229474968156533e-15 [0xBCFB4D5998901168]
    -0x1.c71c71c6c247ap-15, // -5.42534722197196502e-05 [0xBF0C71C71C6C247A]
    -0x1.b3462f0c51592p-51, // -7.55080295853641947e-16 [0xBCCB3462F0C51592]
    0x1.55555555556c7p-9,   //   0.00260416666666682698 [0x3F655555555556C7]
    -0x1.a20841fbadcfcp-56, // -2.26615740507999183e-17 [0xBC7A20841FBADCFC]
    -0x1p-4,                //                  -0.0625 [0xBFB0000000000000]
    -0x1.a612f6dd351ep-64,  // -8.93776628472555558e-20 [0xBBFA612F6DD351E0]
    0x1p-1,                 //                      0.5 [0x3FE0000000000000]
};
double __dy1_ep_small_j1(double x) {
  int i = 0, j = 0;
  uint32_t iaxhi = 0, isxhi = 0, sign = 0;
  uint64_t lsx = 0, lax = 0;
  double ax = __fabs(x);
  double result = 0;
  double xi = 0, y = 0, z = 0, p = 0, q = 0, rr = 0, r1 = 0;
  double dy = 0, dz = 0;
  iaxhi = ((uint32_t *)&ax)[1];
  isxhi = ((uint32_t *)&x)[1];
  lsx = *(uint64_t *)&x;
  lax = *(uint64_t *)&ax;
  sign = isxhi >> 31;
  if (iaxhi < 0x3C600000) /* small input |x| < 2^(-57) */
  {
    result = x * 0.5; /* return x/2 */
  } else {
    y = ax * ax;
    p = (((((((((__dy1_ep_dJ1[0] * y + __dy1_ep_dJ1[2]) * y + __dy1_ep_dJ1[4]) *
                   y +
               __dy1_ep_dJ1[6]) *
                  y +
              __dy1_ep_dJ1[8]) *
                 y +
             __dy1_ep_dJ1[10]) *
                y +
            __dy1_ep_dJ1[12]) *
               y +
           __dy1_ep_dJ1[14]) *
              y +
          __dy1_ep_dJ1[16]) *
             y +
         __dy1_ep_dJ1[18]) *
        ax;
    q = ((((((((__dy1_ep_dJ1[1] * y + __dy1_ep_dJ1[3]) * y + __dy1_ep_dJ1[5]) *
                  y +
              __dy1_ep_dJ1[7]) *
                 y +
             __dy1_ep_dJ1[9]) *
                y +
            __dy1_ep_dJ1[11]) *
               y +
           __dy1_ep_dJ1[13]) *
              y +
          __dy1_ep_dJ1[15]) *
             y +
         __dy1_ep_dJ1[17]) *
        y;
    result = p + q;
    result = (sign ? -result : result);
  }
  return result;
}
/*
//
//  Reduction values (center of intyervals)
//
*/
static const double __dy1_ep_dZ_MP[] = {
    0x1.193bed4dff243p+1,
    -0x1.bep-55, // 2.19714132603101708 + -4.83554168928534978e-17
                 // [0x400193BED4DFF243 + 0xBC8BE00000000000]
    0x1.5b7fe4e87b02ep+2,
    0x1.ep-52, // 5.42968104079413472 +  4.16333634234433703e-16
               // [0x4015B7FE4E87B02E + 0x3CBE000000000000]
    0x1.13127ae6169b4p+3,
    0x1.48p-52, // 8.59600586833116864 +  2.84494650060196363e-16
                // [0x40213127AE6169B4 + 0x3CB4800000000000]
    0x1.77f9138d43206p+3,
    0x1.1p-55, // 11.7491548308398812 +  2.94902990916057206e-17
               // [0x40277F9138D43206 + 0x3C81000000000000]
    0x1.dcb7d88de848bp+3,
    -0x1.5ep-51, //  14.897442128336726 + -6.07153216591882483e-16
                 //  [0x402DCB7D88DE848B + 0xBCC5E00000000000]
    0x1.20b1c695f1e3bp+4,
    -0x1.a2p-50, //  18.043402276727857 + -1.45022882591661073e-15
                 //  [0x40320B1C695F1E3B + 0xBCDA200000000000]
    0x1.53025492188cdp+4,
    0x1.39p-50, // 21.1880689341422119 +  1.08593689596148124e-15
                // [0x40353025492188CD + 0x3CD3900000000000]
    0x1.854fa303820cap+4,
    0x1.52p-52, // 24.3319425713569117 +  2.93168267440080399e-16
                // [0x403854FA303820CA + 0x3CB5200000000000]
    0x1.b79acee8cfb7dp+4,
    -0x1.dp-52, // 27.4752949804492239 + -4.02455846426619246e-16
                // [0x403B79ACEE8CFB7D + 0xBCBD000000000000]
    0x1.e9e480605283cp+4,
    -0x1.e8p-54, // 30.6182864916411148 + -1.05818132034585233e-16
                 // [0x403E9E480605283C + 0xBC9E800000000000]
    0x1.0e16907f8fb56p+5,
    -0x1.968p-49, // 33.7610177961093285 + -2.82066037193828834e-15
                  // [0x4040E16907F8FB56 + 0xBCE9680000000000]
    0x1.273a7b35a7affp+5,
    0x1.25p-50, //  36.903555316142949 +  1.01654795692240896e-15
                //  [0x404273A7B35A7AFF + 0x3CD2500000000000]
};
/*
//
//  Interval bounds
//
*/
static const double __dy1_ep_dS[] = {
    0x1.fa9534d98569cp+1, // 3.95767841931485798 [0x400FA9534D98569C]
    0x1.c581dc4e72103p+2, // 7.08605106030177279 [0x401C581DC4E72103]
    0x1.471d735a47d58p+3, // 10.2223450434964178 [0x402471D735A47D58]
    0x1.ab8e1c4a1e74ap+3, // 13.3610974738727641 [0x402AB8E1C4A1E74A]
    0x1.0803c74003214p+4, // 16.5009224415280897 [0x4030803C74003214]
    0x1.3a42cdf5febd7p+4, // 19.6413097008879411 [0x4033A42CDF5FEBD7]
    0x1.6c832fd77ac07p+4, // 22.7820280472915577 [0x4036C832FD77AC07]
    0x1.9ec46f3e80146p+4, // 25.9229576531809229 [0x4039EC46F3E80146]
    0x1.d106449616c4fp+4, // 29.0640302527283971 [0x403D106449616C4F]
    0x1.01a4420e4abeep+5, // 32.2052041164932774 [0x40401A4420E4ABEE]
    0x1.1ac588c944279p+5, // 35.3464523052143207 [0x4041AC588C944279]
    0x1.33e6ecf5cb221p+5, //   38.48775665308154 [0x40433E6ECF5CB221]
};
/*
//
//  Coefficients for polynomial minimax-approximation Y1(x) =
x*P0(x^2)+2/pi*(j1(x)*log(x)-1/x)
//                                   on interval (0.0,1.77).
//
*/
static const double __dy1_ep_dP0[] = {
    0x1.87ae1f96163e5p-59,  //  2.65412806660663179e-18 [0x3C487AE1F96163E5]
    -0x1.143b19fc52364p-50, // -9.58368333509174507e-16 [0xBCD143B19FC52364]
    0x1.2a4d2dce10646p-42,  //  2.64944937258598771e-13 [0x3D52A4D2DCE10646]
    -0x1.f0ce3011340fdp-35, // -5.64802372704520549e-11 [0xBDCF0CE3011340FD]
    0x1.32e5a4d60e789p-27,  //   8.9318796075580616e-09 [0x3E432E5A4D60E789]
    -0x1.0a780ac766012p-20, //  -9.9267406192781286e-07 [0xBEB0A780AC766012]
    0x1.2c7dbffcde69dp-14,  //  7.16426874997296476e-05 [0x3F12C7DBFFCDE69D]
    -0x1.835b97894be51p-9,  //  -0.00295530533607982942 [0xBF6835B97894BE51]
    0x1.bd3975c75b4a7p-5,   //    0.0543486881605102432 [0x3FABD3975C75B4A7]
    -0x1.91866143cbc8ap-3,  //    -0.196057090646238941 [0xBFC91866143CBC8A]
};
/*
//
//  Coefficients for rational minimax-approximation Y1(x) =
P1(x-Z[0])/Q1(x-Z[0]) on interval (1.77,S[0]).
//
*/
static const double __dy1_ep_dP1[] = {
    0x1.14656e67e994bp-39,  //  1.96391323406757619e-12 [0x3D814656E67E994B]
    -0x1.7723473ab6c8ep-39, // -2.66551442694869469e-12 [0xBD87723473AB6C8E]
    -0x1.7168e58448e9ep-31, // -6.71952424722776299e-10 [0xBE07168E58448E9E]
    0x1.515b11dcc751dp-30,  //  1.22729303733228742e-09 [0x3E1515B11DCC751D]
    0x1.9e6e4e0ac5d8dp-24,  //  9.64922081510671762e-08 [0x3E79E6E4E0AC5D8D]
    -0x1.5d242ac8a48d7p-25, // -4.06453942214584027e-08 [0xBE65D242AC8A48D7]
    -0x1.0fc290580b17ep-17, // -8.09907957951475953e-06 [0xBEE0FC290580B17E]
    -0x1.2b473f2fee979p-17, // -8.91918861599805297e-06 [0xBEE2B473F2FEE979]
    0x1.7be62c2fb248bp-12,  //  0.000362300025682112711 [0x3F37BE62C2FB248B]
    0x1.e7c9a7ca07f5cp-11,  //  0.000930381234187631617 [0x3F4E7C9A7CA07F5C]
    -0x1.bfc1ff1a75769p-8,  //  -0.00683224180325745726 [0xBF7BFC1FF1A75769]
    -0x1.cc521ca1975fap-6,  //   -0.0280957488633841526 [0xBF9CC521CA1975FA]
    0x1.19159bfd41968p-6,   //    0.0171560309727508964 [0x3F919159BFD41968]
    0x1.9f6c6a359c047p-3,   //     0.202843503724127155 [0x3FC9F6C6A359C047]
    0x1.e6bb7d16c3df5p-3,   //     0.237662293679462927 [0x3FCE6BB7D16C3DF5]
    -0x1.ad17da6040304p-66, // -2.27160035894894737e-20 [0xBBDAD17DA6040304]
};
static const double __dy1_ep_dQ1[] = {
    0x1.ca8400983465fp-14, // 0.000109318645971323469 [0x3F1CA8400983465F]
    0x1.6187bae8826e4p-11, // 0.000674305335509131371 [0x3F46187BAE8826E4]
    0x1.57bae814a2da4p-6,  //   0.0209796205913740513 [0x3F957BAE814A2DA4]
    0x1.645c92d4758b6p-3,  //    0.174004695042361213 [0x3FC645C92D4758B6]
    0x1.f92fb76f46505p-2,  //      0.4933460866610207 [0x3FDF92FB76F46505]
    0x1.d34e2058210ecp-2,  //    0.456352715853667545 [0x3FDD34E2058210EC]
};
/*
//
//  Coefficients for rational minimax-approximation Y1(x) =
P2(x-Z[1])/Q2(x-Z[1]) on interval (S[0],S[1]).
//
*/
static const double __dy1_ep_dP2[] = {
    0x1.d525d1ff30b55p-44,  //  1.04171723591104216e-13 [0x3D3D525D1FF30B55]
    -0x1.29bafa06f1364p-40, // -1.05775078513323357e-12 [0xBD729BAFA06F1364]
    -0x1.18276dd8e58bfp-35, // -3.18498245958445038e-11 [0xBDC18276DD8E58BF]
    0x1.3831e10dec74cp-32,  //  2.83939553180474522e-10 [0x3DF3831E10DEC74C]
    0x1.b74c86440bf6bp-28,  //  6.39264069808466182e-09 [0x3E3B74C86440BF6B]
    -0x1.84a5e393bc431p-25, //  -4.5244582439869773e-08 [0xBE684A5E393BC431]
    -0x1.aeca92dd42963p-21, // -8.02411331741759624e-07 [0xBEAAECA92DD42963]
    0x1.17ab105a16f11p-18,  //  4.16738120109415872e-06 [0x3ED17AB105A16F11]
    0x1.04bb6d955f79dp-14,  //  6.21633865502666433e-05 [0x3F104BB6D955F79D]
    -0x1.b857295f95753p-13, // -0.000209970700768275816 [0xBF2B857295F95753]
    -0x1.601e044516c31p-9,  //  -0.00268644144175091875 [0xBF6601E044516C31]
    0x1.328c95dd189edp-8,   //    0.0046775689962452573 [0x3F7328C95DD189ED]
    0x1.be4053c6844c6p-5,   //    0.0544740329970721388 [0x3FABE4053C6844C6]
    -0x1.d85d92fd79dcfp-6,  //   -0.0288309035730927553 [0xBF9D85D92FD79DCF]
    -0x1.55950feffff73p-2,  //    -0.333576439879826825 [0xBFD55950FEFFFF73]
    -0x1.0312e97a7d97ap-65, // -2.74305134976974801e-20 [0xBBE0312E97A7D97A]
};
static const double __dy1_ep_dQ2[] = {
    0x1.90e0dffe4156cp-23, // 1.86673560116790981e-07 [0x3E890E0DFFE4156C]
    0x1.6a1322d2b9a6bp-22, // 3.37208388474796711e-07 [0x3E96A1322D2B9A6B]
    0x1.7c61a8d772a2cp-11, // 0.000725520100723746949 [0x3F47C61A8D772A2C]
    0x1.6d397c7bce601p-9,  //  0.00278644222808277541 [0x3F66D397C7BCE601]
    0x1.665bca13fd7ebp-3,  //    0.174979761824658325 [0x3FC665BCA13FD7EB]
    0x1.f5db800d88ccfp-1,  //    0.980190278675218241 [0x3FEF5DB800D88CCF]
};
/*
//
//  Coefficients for polynomial minimax-approximation Y1(x) = P(x-Z[n]) on
intervals (S[n-1],S[n])
//                               for n from 2 to 11
//
*/
static const double __dy1_ep_dP[] = {
    /* interval S[1]-S[2] */
    -0x1.23c4bb7553e3ep-60, // -9.88550080372520585e-19 [0xBC323C4BB7553E3E]
    -0x1.0f7cb29ebbb8fp-56, // -1.47173452095874271e-17 [0xBC70F7CB29EBBB8F]
    0x1.fa151696070a1p-52,  //  4.38956489113516994e-16 [0x3CBFA151696070A1]
    0x1.3171cdb0ee64cp-48,  //  4.23889458045057812e-15 [0x3CF3171CDB0EE64C]
    -0x1.16f653b9e1ef1p-43, // -1.23884109663554916e-13 [0xBD416F653B9E1EF1]
    -0x1.268654326d457p-40, // -1.04636200946101774e-12 [0xBD7268654326D457]
    0x1.ec40b00a20e6dp-36,  //  2.79813256242153984e-11 [0x3DBEC40B00A20E6D]
    0x1.9e32843fae514p-33,  //  1.88355138769036466e-10 [0x3DE9E32843FAE514]
    -0x1.41e7a91b4f17dp-28, //  -4.6843331672875829e-09 [0xBE341E7A91B4F17D]
    -0x1.a384eebe6a606p-26, // -2.44192352950018297e-08 [0xBE5A384EEBE6A606]
    0x1.2becb2b6ad58cp-21,  //   5.5865310496251824e-07 [0x3EA2BECB2B6AD58C]
    0x1.176e72bf92df6p-19,  //  2.08192645326731138e-06 [0x3EC176E72BF92DF6]
    -0x1.6f7bab104f181p-15, // -4.38073967665457141e-05 [0xBF06F7BAB104F181]
    -0x1.b50d7e1d31fc9p-14, // -0.000104201485061072106 [0xBF1B50D7E1D31FC9]
    0x1.07a678d6000b9p-9,   //   0.00201149201438973997 [0x3F607A678D6000B9]
    0x1.3ced2a2e6918p-9,    //   0.00241795673282946266 [0x3F63CED2A2E69180]
    -0x1.6395dfe49fcd4p-5,  //   -0.0434064267074007193 [0xBFA6395DFE49FCD4]
    -0x1.02b3933cf21b1p-6,  //   -0.0157898843642969057 [0xBF902B3933CF21B1]
    0x1.15f993fceab5cp-2,   //     0.271459877311533537 [0x3FD15F993FCEAB5C]
    0x1.af12431b1228bp-64,  //  9.12829061480359733e-20 [0x3BFAF12431B1228B]
                            /* interval S[2]-S[3] */
    0x1.4d7880d44d429p-60,  //  1.12984273283773674e-18 [0x3C34D7880D44D429]
    0x1.b7f6ead01cfb1p-57,  //  1.19252622225131675e-17 [0x3C6B7F6EAD01CFB1]
    -0x1.eea793e6d058cp-52, // -4.29044474036634532e-16 [0xBCBEEA793E6D058C]
    -0x1.0d4440dff0ad2p-48, // -3.73682494545463922e-15 [0xBCF0D4440DFF0AD2]
    0x1.15e48ce887637p-43,  //  1.23409182912878684e-13 [0x3D415E48CE887637]
    0x1.eff4c3caa680fp-41,  //  8.80995032557714429e-13 [0x3D6EFF4C3CAA680F]
    -0x1.e32cf1d510801p-36, // -2.74653510284562777e-11 [0xBDBE32CF1D510801]
    -0x1.549e89e5e2cf8p-33, // -1.54895720541559814e-10 [0xBDE549E89E5E2CF8]
    0x1.39066099a7d85p-28,  //  4.55511197654908906e-09 [0x3E339066099A7D85]
    0x1.4a7b82d1421edp-26,  //  1.92366112527288339e-08 [0x3E54A7B82D1421ED]
    -0x1.1e86423078152p-21, // -5.33693372214324706e-07 [0xBEA1E86423078152]
    -0x1.a2977fa427b1fp-20, // -1.55937593952050399e-06 [0xBEBA2977FA427B1F]
    0x1.571814a1aa12fp-15,  //   4.0899999715295731e-05 [0x3F0571814A1AA12F]
    0x1.315ec04d6e60ap-14,  //   7.2805910540284371e-05 [0x3F1315EC04D6E60A]
    -0x1.dc4f991b3db82p-10, //  -0.00181698200213784066 [0xBF5DC4F991B3DB82]
    -0x1.9d6eb2bc49e33p-10, //  -0.00157711950220996706 [0xBF59D6EB2BC49E33]
    0x1.367d7d608e4bap-5,   //    0.0379016350529552543 [0x3FA367D7D608E4BA]
    0x1.4429fef5b5fbdp-7,   //   0.00989270161828403362 [0x3F84429FEF5B5FBD]
    -0x1.dc14ea14e89f9p-3,  //    -0.232461766017038746 [0xBFCDC14EA14E89F9]
    -0x1.a417d82054693p-68, // -5.55988643484316615e-21 [0xBBBA417D82054693]
    /* interval S[3]-S[4] */
    -0x1.4cf7dd90ea184p-60, // -1.12814022643196789e-18 [0xBC34CF7DD90EA184]
    -0x1.7fbfbadff5fb8p-57, //  -1.0401536002789667e-17 [0xBC67FBFBADFF5FB8]
    0x1.e7f3d0faa0bc6p-52,  //  4.23231248239319699e-16 [0x3CBE7F3D0FAA0BC6]
    0x1.cd81a41eeb45ep-49,  //  3.20234401867072989e-15 [0x3CECD81A41EEB45E]
    -0x1.101b3c3cbe706p-43, // -1.20839510796760791e-13 [0xBD4101B3C3CBE706]
    -0x1.9fdce8ca13dafp-41, // -7.38720954778357796e-13 [0xBD69FDCE8CA13DAF]
    0x1.d4ca9d6f9295cp-36,  //  2.66477095908685358e-11 [0x3DBD4CA9D6F9295C]
    0x1.161b696b87b8ep-33,  //  1.26468456679571697e-10 [0x3DE161B696B87B8E]
    -0x1.2c1a8c894c566p-28, // -4.36708370273232534e-09 [0xBE32C1A8C894C566]
    -0x1.056babcaebd9ep-26, // -1.52166810642218502e-08 [0xBE5056BABCAEBD9E]
    0x1.0e9b612db24aep-21,  //  5.04044725701697227e-07 [0x3EA0E9B612DB24AE]
    0x1.3f35db1feac1p-20,   //  1.18915131251218385e-06 [0x3EB3F35DB1FEAC10]
    -0x1.3e398cbc47183p-15, // -3.79353527668350866e-05 [0xBF03E398CBC47183]
    -0x1.c0a9cee3c82dbp-15, // -5.34848348066102171e-05 [0xBF0C0A9CEE3C82DB]
    0x1.b17602840abf1p-10,  //   0.00165352239947469067 [0x3F5B17602840ABF1]
    0x1.26b045287ddc8p-10,  //   0.00112414762599920455 [0x3F526B045287DDC8]
    -0x1.163191c30aa62p-5,  //   -0.0339591833984129871 [0xBFA163191C30AA62]
    -0x1.c650b6b83109ap-8,  //  -0.00693230114865541792 [0xBF7C650B6B83109A]
    0x1.a7022be084d99p-3,   //     0.206547110356592595 [0x3FCA7022BE084D99]
    0x1.e151f1e87dce3p-67,  //  1.27404368980499142e-20 [0x3BCE151F1E87DCE3]
                            /* interval S[4]-S[5] */
    0x1.48eede9471c7ap-60,  //  1.11446863767936155e-18 [0x3C348EEDE9471C7A]
    0x1.4d57aaba3cf39p-57,  //  9.03526518047914635e-18 [0x3C64D57AABA3CF39]
    -0x1.dd398875a030ap-52, // -4.13926478558563718e-16 [0xBCBDD398875A030A]
    -0x1.8ac1c027a0fdap-49, // -2.73917581880254113e-15 [0xBCE8AC1C027A0FDA]
    0x1.08331cf64a87bp-43,  //  1.17328218552343539e-13 [0x3D408331CF64A87B]
    0x1.5d3809162514fp-41,  //  6.20337361299472299e-13 [0x3D65D3809162514F]
    -0x1.c3439b9960ddap-36, // -2.56513938554899509e-11 [0xBDBC3439B9960DDA]
    -0x1.c96e9ab628898p-34, // -1.04008006067260625e-10 [0xBDDC96E9AB628898]
    0x1.1df6532d8d629p-28,  //  4.16129779033682957e-09 [0x3E31DF6532D8D629]
    0x1.a44a79032df68p-27,  //  1.22320753581295741e-08 [0x3E4A44A79032DF68]
    -0x1.fdd03174e759cp-22, //  -4.7480059289249439e-07 [0xBE9FDD03174E759C]
    -0x1.f56c29d9a0f3ep-21, // -9.33972212638769643e-07 [0xBEAF56C29D9A0F3E]
    0x1.282d26a74c26dp-15,  //  3.53069747752774335e-05 [0x3F0282D26A74C26D]
    0x1.59145b4f0e3adp-15,  //  4.11366842105251245e-05 [0x3F059145B4F0E3AD]
    -0x1.8efee4094379ap-10, //  -0.00152204768011496858 [0xBF58EFEE4094379A]
    -0x1.be318d61276ddp-11, //   -0.0008510466833688519 [0xBF4BE318D61276DD]
    0x1.fbe6df840847fp-6,   //    0.0309998686572083813 [0x3F9FBE6DF840847F]
    0x1.54eda697a0098p-8,   //   0.00520215336985088012 [0x3F754EDA697A0098]
    -0x1.80781c32422e7p-3,  //     -0.18772909191490969 [0xBFC80781C32422E7]
    0x1.a95cf5f9be762p-65,  //  4.50370730260161083e-20 [0x3BEA95CF5F9BE762]
    /* interval S[5]-S[6] */
    -0x1.42902ba810806p-60, // -1.09288651718310442e-18 [0xBC342902BA810806]
    -0x1.21de544fbbf0fp-57, // -7.85690073089726266e-18 [0xBC621DE544FBBF0F]
    0x1.d0215d40c39c7p-52,  //  4.02568888968634086e-16 [0x3CBD0215D40C39C7]
    0x1.52df4ba743e23p-49,  //  2.35139857674538332e-15 [0x3CE52DF4BA743E23]
    -0x1.fe918e736ac7ap-44, // -1.13368998606021039e-13 [0xBD3FE918E736AC7A]
    -0x1.276c2e1db4588p-41, // -5.24775918142277894e-13 [0xBD6276C2E1DB4588]
    0x1.b0e00eb7f8feap-36,  //  2.46061077057601351e-11 [0x3DBB0E00EB7F8FEA]
    0x1.7cfa786d5f503p-34,  //  8.66244590892587489e-11 [0x3DD7CFA786D5F503]
    -0x1.1021cdd567b6cp-28, // -3.96004247918164643e-09 [0xBE31021CDD567B6C]
    -0x1.588c7589af927p-27, // -1.00276860316818112e-08 [0xBE4588C7589AF927]
    0x1.e12725852bde3p-22,  //  4.48108572757641658e-07 [0x3E9E12725852BDE3]
    0x1.94f64f476314bp-21,  //  7.54300779102993692e-07 [0x3EA94F64F476314B]
    -0x1.154ed4598d1ffp-15, // -3.30576810480299953e-05 [0xBF0154ED4598D1FF]
    -0x1.132c0aa83cfcbp-15, // -3.28030631084049468e-05 [0xBF0132C0AA83CFCB]
    0x1.7307b03e248f6p-10,  //   0.00141536725235319244 [0x3F57307B03E248F6]
    0x1.6081b0b7fe56fp-11,  //  0.000672352986838804334 [0x3F46081B0B7FE56F]
    -0x1.d5f857a2a6107p-6,  //   -0.0286846977056436601 [0xBF9D5F857A2A6107]
    -0x1.0bf614807033cp-8,  //  -0.00408876419572906311 [0xBF70BF614807033C]
    0x1.62d94d97e859cp-3,   //     0.173266035269119878 [0x3FC62D94D97E859C]
    -0x1.2c47f24b48519p-64, // -6.35869837619352631e-20 [0xBBF2C47F24B48519]
                            /* interval S[6]-S[7] */
    0x1.3ad885d1d7785p-60,  //  1.06673903626441605e-18 [0x3C33AD885D1D7785]
    0x1.fa35e806c6375p-58,  //  6.86043251955634451e-18 [0x3C5FA35E806C6375]
    -0x1.c1f7535a31d2dp-52, //  -3.9028339206748663e-16 [0xBCBC1F7535A31D2D]
    -0x1.24d2cb6829464p-49, // -2.03187061788757043e-15 [0xBCE24D2CB6829464]
    0x1.ec45a826e4008p-44,  //  1.09306363303890395e-13 [0x3D3EC45A826E4008]
    0x1.f8ce22d12fb43p-42,  //  4.48357101461121668e-13 [0x3D5F8CE22D12FB43]
    -0x1.9eed9f5fe3ac6p-36, // -2.35859382151438423e-11 [0xBDB9EED9F5FE3AC6]
    -0x1.41c85ff6200cap-34, // -7.31649184342963088e-11 [0xBDD41C85FF6200CA]
    0x1.03466438e4e89p-28,  //    3.772947337275593e-09 [0x3E303466438E4E89]
    0x1.1fce14f0b14cep-27,  //  8.37622812897249059e-09 [0x3E41FCE14F0B14CE]
    -0x1.c7b3d81b534edp-22, // -4.24406040717996221e-07 [0xBE9C7B3D81B534ED]
    -0x1.4ee5e4e7c107dp-21, // -6.23796180016403297e-07 [0xBEA4EE5E4E7C107D]
    0x1.05375a588a65dp-15,  //  3.11394002817210665e-05 [0x3F005375A588A65D]
    0x1.c3625d7a64ea2p-16,  //  2.69045972146180555e-05 [0x3EFC3625D7A64EA2]
    -0x1.5beee6fd51c88p-10, //   -0.0013272598728961988 [0xBF55BEEE6FD51C88]
    -0x1.1f6911725a953p-11, // -0.000548191876345508595 [0xBF41F6911725A953]
    0x1.b750d89a9b35fp-6,   //    0.0268137088082086548 [0x3F9B750D89A9B35F]
    0x1.b3878aadeb34dp-9,   //   0.00332282607901960301 [0x3F6B3878AADEB34D]
    -0x1.4b2a38f1ab9b4p-3,  //    -0.161701626658624087 [0xBFC4B2A38F1AB9B4]
    0x1.4000a4a8848acp-63,  //  1.35526335639833764e-19 [0x3C04000A4A8848AC]
    /* interval S[7]-S[8] */
    -0x1.327bdffc19753p-60, // -1.03840779862563278e-18 [0xBC3327BDFFC19753]
    -0x1.bc9190ce6547fp-58, // -6.02502823860615491e-18 [0xBC5BC9190CE6547F]
    0x1.b39ad6736d9f4p-52,  //  3.77826966554380227e-16 [0x3CBB39AD6736D9F4]
    0x1.fe14719717f8cp-50,  //  1.76969500945508689e-15 [0x3CDFE14719717F8C]
    -0x1.da69c601b6551p-44, // -1.05340886589709095e-13 [0xBD3DA69C601B6551]
    -0x1.b3dbb60f8bdfap-42, // -3.87119888834993279e-13 [0xBD5B3DBB60F8BDFA]
    0x1.8e087a1380d19p-36,  //  2.26255629476524288e-11 [0x3DB8E087A1380D19]
    0x1.137a1a61b6d28p-34,  //  6.26362100439866467e-11 [0x3DD137A1A61B6D28]
    -0x1.ef28e31d55157p-29, // -3.60276110218155364e-09 [0xBE2EF28E31D55157]
    -0x1.e8e0000b60ca7p-28, // -7.11406756713676368e-09 [0xBE3E8E0000B60CA7]
    0x1.b143d39c85c07p-22,  //   4.0350942656766462e-07 [0x3E9B143D39C85C07]
    0x1.1a743e05aaa41p-21,  //  5.26111705941475038e-07 [0x3EA1A743E05AAA41]
    -0x1.eeceb341ad6ccp-16, //  -2.9492820664520425e-05 [0xBEFEECEB341AD6CC]
    -0x1.7a8e14711bf43p-16, // -2.25636362682351246e-05 [0xBEF7A8E14711BF43]
    0x1.48843c426abdep-10,  //   0.00125319116396371095 [0x3F548843C426ABDE]
    0x1.e024f567ac482p-12,  //  0.000457901353438101952 [0x3F3E024F567AC482]
    -0x1.9de7a33bc3a97p-6,  //   -0.0252627462460233952 [0xBF99DE7A33BC3A97]
    -0x1.6afe4fe0bc0f7p-9,  //  -0.00276941990920686774 [0xBF66AFE4FE0BC0F7]
    0x1.37aaceac987b9p-3,   //     0.152181257860375202 [0x3FC37AACEAC987B9]
    -0x1.2075f63b9cde1p-63, //  -1.2216789591053412e-19 [0xBC02075F63B9CDE1]
                            /* interval S[8]-S[9] */
    0x1.29f108151f739p-60,  //  1.00946516763614657e-18 [0x3C329F108151F739]
    0x1.88f8f0a0f6041p-58,  //  5.32576941672142648e-18 [0x3C588F8F0A0F6041]
    -0x1.a591cd6ee9c07p-52, // -3.65653289689007054e-16 [0xBCBA591CD6EE9C07]
    -0x1.bfebb8c357de2p-50, // -1.55403741267986894e-15 [0xBCDBFEBB8C357DE2]
    0x1.c9729f4b9c4bcp-44,  //  1.01573803402521483e-13 [0x3D3C9729F4B9C4BC]
    0x1.7c2d94eb287ap-42,   //  3.37665942822071369e-13 [0x3D57C2D94EB287A0]
    -0x1.7e68014e9abfcp-36, // -2.17372797774249687e-11 [0xBDB7E68014E9ABFC]
    -0x1.dd7f8ebc658aep-35, // -5.42852685302207526e-11 [0xBDCDD7F8EBC658AE]
    0x1.da0e1c8421a22p-29,  //  3.44920497898229022e-09 [0x3E2DA0E1C8421A22]
    0x1.a53958c2c02b5p-28,  //  6.12961609486201578e-09 [0x3E3A53958C2C02B5]
    -0x1.9d764ee072969p-22, // -3.85066625721442452e-07 [0xBE99D764EE072969]
    -0x1.e448fbc891ebbp-22, // -4.51025638646407465e-07 [0xBE9E448FBC891EBB]
    0x1.d6dfcdb025ee9p-16,  //  2.80662913501862882e-05 [0x3EFD6DFCDB025EE9]
    0x1.43394c95b2ba7p-16,  //  1.92656412625714684e-05 [0x3EF43394C95B2BA7]
    -0x1.37eef9aadeee1p-10, //  -0.00118993185853260458 [0xBF537EEF9AADEEE1]
    -0x1.98cd1bebe144p-12,  // -0.000389863211914868307 [0xBF398CD1BEBE1440]
    0x1.8868d7401bf2ep-6,   //    0.0239507772498079238 [0x3F98868D7401BF2E]
    0x1.349369dc780bbp-9,   //   0.00235424678088244415 [0x3F6349369DC780BB]
    -0x1.2740819f1caaap-3,  //     -0.14416600481816505 [0xBFC2740819F1CAAA]
    0x1.988fb7f736c57p-67,  //  1.08145302340879313e-20 [0x3BC988FB7F736C57]
    /* interval S[9]-S[10] */
    -0x1.21817b556268fp-60, // -9.80883764004484672e-19 [0xBC321817B556268F]
    -0x1.5dabb073706b2p-58, // -4.73892113128973606e-18 [0xBC55DABB073706B2]
    0x1.98245f5f9a472p-52,  //  3.54006824100745394e-16 [0x3CB98245F5F9A472]
    0x1.8c7a72600e22bp-50,  //  1.37556045626017407e-15 [0x3CD8C7A72600E22B]
    -0x1.b990686b9b228p-44, // -9.80469246520686829e-14 [0xBD3B990686B9B228]
    -0x1.4ec0aa96471cdp-42, // -2.97320037879873064e-13 [0xBD54EC0AA96471CD]
    0x1.701128aacc46cp-36,  //  2.09221881722165739e-11 [0x3DB701128AACC46C]
    0x1.a262167d7525ep-35,  //  4.75646579242511057e-11 [0x3DCA262167D7525E]
    -0x1.c70ab1536c516p-29, // -3.31086461867430843e-09 [0xBE2C70AB1536C516]
    -0x1.6f737958af3bap-28, //  -5.3471168262983686e-09 [0xBE36F737958AF3BA]
    0x1.8be81ad444941p-22,  //  3.68716809321816511e-07 [0x3E98BE81AD444941]
    0x1.a4e0bc0917f1fp-22,  //  3.91973060736768598e-07 [0x3E9A4E0BC0917F1F]
    -0x1.c1f05a2d85045p-16, //  -2.6818446874868574e-05 [0xBEFC1F05A2D85045]
    -0x1.18123e875172ep-16, // -1.66935483581133294e-05 [0xBEF18123E875172E]
    0x1.29934b7a84482p-10,  //    0.0011351599520177673 [0x3F529934B7A84482]
    0x1.617c581be35adp-12,  //  0.000337110251842210825 [0x3F3617C581BE35AD]
    -0x1.75eceaabf7f86p-6,  //   -0.0228225986210186735 [0xBF975ECEAABF7F86]
    -0x1.0a846a83fecf2p-9,  //  -0.00203336525191632426 [0xBF60A846A83FECF2]
    0x1.192f2627a74e3p-3,   //     0.137296960911874683 [0x3FC192F2627A74E3]
    0x1.1359132991e55p-62,  //  2.33228784460989702e-19 [0x3C11359132991E55]
                            /* interval S[10]-S[11] */
    0x1.1957c5e64dec1p-60,  //  9.53226699297399268e-19 [0x3C31957C5E64DEC1]
    0x1.3925bc11bacdap-58,  //  4.24393865535078934e-18 [0x3C53925BC11BACDA]
    -0x1.8b73fe0dc1f17p-52, // -3.43000884034416543e-16 [0xBCB8B73FE0DC1F17]
    -0x1.61929f3b687c8p-50, // -1.22670187267964261e-15 [0xBCD61929F3B687C8]
    0x1.aacdb50836329p-44,  //  9.47694242148858361e-14 [0x3D3AACDB50836329]
    0x1.294a0966390f6p-42,  //  2.64045857109760948e-13 [0x3D5294A0966390F6]
    -0x1.62f2157345f84p-36, //   -2.017632367627559e-11 [0xBDB62F2157345F84]
    -0x1.722395a06c18bp-35, //  -4.2079932640204065e-11 [0xBDC722395A06C18B]
    0x1.b5e23ab9c8ab6p-29,  //  3.18602330359387926e-09 [0x3E2B5E23AB9C8AB6]
    0x1.43fa0ea0f3e19p-28,  //  4.71448272170634349e-09 [0x3E343FA0EA0F3E19]
    -0x1.7c412616fd00cp-22, // -3.54139588264928567e-07 [0xBE97C412616FD00C]
    -0x1.720522bb12d1ep-22, // -3.44608036055489005e-07 [0xBE9720522BB12D1E]
    0x1.af7544eeac65ep-16,  //  2.57169057770789404e-05 [0x3EFAF7544EEAC65E]
    0x1.eb59416878e92p-17,  //  1.46433310001589007e-05 [0x3EEEB59416878E92]
    -0x1.1cff175d05c29p-10, //  -0.00108717517946899989 [0xBF51CFF175D05C29]
    -0x1.35959b8482e3dp-12, // -0.000295242695103257156 [0xBF335959B8482E3D]
    0x1.65d05948a946ap-6,   //    0.0218392249977522948 [0x3F965D05948A946A]
    0x1.d26e7af251f79p-10,  //    0.0017792952099444355 [0x3F5D26E7AF251F79]
    -0x1.0cf3ee98f769bp-3,  //    -0.131324638407865318 [0xBFC0CF3EE98F769B]
    -0x1.08f18a6c5db9fp-62, //  -2.2441588926492306e-19 [0xBC108F18A6C5DB9F]
};
/*
//
//  Coefficients for polynomial pade-approximation P1(x) = PP(1296/x^2) in point
1296/x^2 = 0.5
//
*/
static const double __dy1_ep_dPP[] = {
    0x1.2e041332b59b9p-65,  //  3.19771791362518245e-20 [0x3BE2E041332B59B9]
    -0x1.df44c525bf7dep-61, // -8.11912645865070006e-19 [0xBC2DF44C525BF7DE]
    0x1.7ed733e2b4011p-56,  //  2.07538324341166498e-17 [0x3C77ED733E2B4011]
    -0x1.91bb3fce3335bp-51, // -6.96892964072491198e-16 [0xBCC91BB3FCE3335B]
    0x1.2b908e904c525p-45,  //  3.32583599936692969e-14 [0x3D22B908E904C525]
    -0x1.576b7365d6b96p-39, // -2.44014394170339461e-12 [0xBD8576B7365D6B96]
    0x1.55c0b64b456cdp-32,  //   3.1082234416873872e-10 [0x3DF55C0B64B456CD]
    -0x1.70b96a673e266p-24, // -8.58503114048832652e-08 [0xBE770B96A673E266]
    0x1.7b425ed097b42p-14,  //  9.04224537037036987e-05 [0x3F17B425ED097B42]
    0x1p+0,                 //                        1 [0x3FF0000000000000]
};
/*
//
//  Coefficients for polynomial pade-approximation Q1(x) = QP(1296/x^2)*(36/x))
in point 1296/x^2 = 0.5
//
*/
static const double __dy1_ep_dQP[] = {
    -0x1.23696090354ep-67,  // -7.71360161124522182e-21 [0xBBC23696090354E0]
    0x1.a60b89719da94p-63,  //  1.78743038053950533e-19 [0x3C0A60B89719DA94]
    -0x1.2886d1ecf082ap-58, // -4.01868535071482541e-18 [0xBC52886D1ECF082A]
    0x1.0a86e8652da7bp-53,  //  1.15587653844937511e-16 [0x3CA0A86E8652DA7B]
    -0x1.4a947ed633352p-48, // -4.58771993137511348e-15 [0xBCF4A947ED633352]
    0x1.2e1508fc7bc09p-42,  //  2.68302862922006368e-13 [0x3D52E1508FC7BC09]
    -0x1.bf888d31b489fp-36, // -2.54393287634639123e-11 [0xBDBBF888D31B489F]
    0x1.3b76cfa7f96aep-28,  //  4.59060692928882268e-09 [0x3E33B76CFA7F96AE]
    -0x1.26fabb85cb534p-19, // -2.19776797196502074e-06 [0xBEC26FABB85CB534]
    0x1.5555555555555p-7,   //    0.0104166666666666661 [0x3F85555555555555]
};
inline int __devicelib_imf_internal_dy1(const double *a, double *r) {
  int nRet = 0;
  int i = 0;
  int32_t ixhi = 0, ixlo = 0, iabsxhi = 0, isignxhi = 0, isign = 0;
  int64_t lx = 0;
  double x = *a, result = 0, xi = 0, y = 0, z = 0, p = 0, q = 0, dr = 0, r1 = 0,
         lnx = 0, p1 = 0, p2 = 0, ps = 0, pc = 0;
  const double dtonpi = 0x1.45f306dc9c883p-1; // 6.366197723675813430755e-01
  const uint64_t linf = 0x7ff0000000000000;   // +Inf
  const double zeros[] = {0x0.0p+0, -0x0.0p+0};
  lx = *(int64_t *)&x;
  ixhi = ((int32_t *)&x)[1];
  ixlo = ((int32_t *)&x)[0];
  iabsxhi = ixhi & ~0x80000000;
  isignxhi = ixhi & 0x80000000;
  /* x is not INF or NaN */
  if (iabsxhi < 0x7FF00000) {
    /* x > 0 */
    if (lx > 0) {
      /* Rational or polynomial approximation of y1(x) */
      if (x <= __dy1_ep_dS[11]) {
        if (x <
            1.77) { /* y1(x) = x*_VSTATIC(dP0)(x^2)+2/pi*(j1(x)*log(x)-1/x) */
          y = x * x;
          z = y * y;
          p = ((((((((__dy1_ep_dP0[0] * y + __dy1_ep_dP0[1]) * y +
                     __dy1_ep_dP0[2]) *
                        y +
                    __dy1_ep_dP0[3]) *
                       y +
                   __dy1_ep_dP0[4]) *
                      y +
                  __dy1_ep_dP0[5]) *
                     y +
                 __dy1_ep_dP0[6]) *
                    y +
                __dy1_ep_dP0[7]) *
                   y +
               __dy1_ep_dP0[8]) *
                  y +
              __dy1_ep_dP0[9];
          result = p * x;
          __dy1_ep_ln_kernel_fp64(x, &lnx);
          p1 = dtonpi * __dy1_ep_small_j1(x) * lnx;
          p2 = dtonpi / x;
          result += p1 - p2;
        } else if (x < __dy1_ep_dS[0]) {
          x = x - __dy1_ep_dZ_MP[0];
          x = x - __dy1_ep_dZ_MP[1];
          p = ((((((((((((((__dy1_ep_dP1[0] * x + __dy1_ep_dP1[1]) * x +
                           __dy1_ep_dP1[2]) *
                              x +
                          __dy1_ep_dP1[3]) *
                             x +
                         __dy1_ep_dP1[4]) *
                            x +
                        __dy1_ep_dP1[5]) *
                           x +
                       __dy1_ep_dP1[6]) *
                          x +
                      __dy1_ep_dP1[7]) *
                         x +
                     __dy1_ep_dP1[8]) *
                        x +
                    __dy1_ep_dP1[9]) *
                       x +
                   __dy1_ep_dP1[10]) *
                      x +
                  __dy1_ep_dP1[11]) *
                     x +
                 __dy1_ep_dP1[12]) *
                    x +
                __dy1_ep_dP1[13]) *
                   x +
               __dy1_ep_dP1[14]) *
                  x +
              __dy1_ep_dP1[15];
          dr = ((((__dy1_ep_dQ1[0] * x + __dy1_ep_dQ1[1]) * x +
                  __dy1_ep_dQ1[2]) *
                     x +
                 __dy1_ep_dQ1[3]) *
                    x +
                __dy1_ep_dQ1[4]) *
                   x +
               __dy1_ep_dQ1[5];
          result = p / dr;
        } else if (x < __dy1_ep_dS[1]) {
          x = x - __dy1_ep_dZ_MP[2];
          x = x - __dy1_ep_dZ_MP[3];
          p = ((((((((((((((__dy1_ep_dP2[0] * x + __dy1_ep_dP2[1]) * x +
                           __dy1_ep_dP2[2]) *
                              x +
                          __dy1_ep_dP2[3]) *
                             x +
                         __dy1_ep_dP2[4]) *
                            x +
                        __dy1_ep_dP2[5]) *
                           x +
                       __dy1_ep_dP2[6]) *
                          x +
                      __dy1_ep_dP2[7]) *
                         x +
                     __dy1_ep_dP2[8]) *
                        x +
                    __dy1_ep_dP2[9]) *
                       x +
                   __dy1_ep_dP2[10]) *
                      x +
                  __dy1_ep_dP2[11]) *
                     x +
                 __dy1_ep_dP2[12]) *
                    x +
                __dy1_ep_dP2[13]) *
                   x +
               __dy1_ep_dP2[14]) *
                  x +
              __dy1_ep_dP2[15];
          dr = ((((__dy1_ep_dQ2[0] * x + __dy1_ep_dQ2[1]) * x +
                  __dy1_ep_dQ2[2]) *
                     x +
                 __dy1_ep_dQ2[3]) *
                    x +
                __dy1_ep_dQ2[4]) *
                   x +
               __dy1_ep_dQ2[5];
          result = p / dr;
        } else {
          for (i = 2; (i < 11) && (x >= __dy1_ep_dS[i]); i++)
            ;
          x = x - __dy1_ep_dZ_MP[i * 2 + 0];
          x = x - __dy1_ep_dZ_MP[i * 2 + 1];
          i = (i - 2) * 20;
          result =
              ((((((((((((((((((__dy1_ep_dP[i + 0] * x + __dy1_ep_dP[i + 1]) *
                                   x +
                               __dy1_ep_dP[i + 2]) *
                                  x +
                              __dy1_ep_dP[i + 3]) *
                                 x +
                             __dy1_ep_dP[i + 4]) *
                                x +
                            __dy1_ep_dP[i + 5]) *
                               x +
                           __dy1_ep_dP[i + 6]) *
                              x +
                          __dy1_ep_dP[i + 7]) *
                             x +
                         __dy1_ep_dP[i + 8]) *
                            x +
                        __dy1_ep_dP[i + 9]) *
                           x +
                       __dy1_ep_dP[i + 10]) *
                          x +
                      __dy1_ep_dP[i + 11]) *
                         x +
                     __dy1_ep_dP[i + 12]) *
                        x +
                    __dy1_ep_dP[i + 13]) *
                       x +
                   __dy1_ep_dP[i + 14]) *
                      x +
                  __dy1_ep_dP[i + 15]) *
                     x +
                 __dy1_ep_dP[i + 16]) *
                    x +
                __dy1_ep_dP[i + 17]) *
                   x +
               __dy1_ep_dP[i + 18]) *
                  x +
              __dy1_ep_dP[i + 19];
        }
      }
      /* Hancels asymptotic forms */
      else {
        xi = (1.0 / x);
        y = (36.0 * xi);
        z = y * y;
        p = ((((((((__dy1_ep_dPP[0] * z + __dy1_ep_dPP[1]) * z +
                   __dy1_ep_dPP[2]) *
                      z +
                  __dy1_ep_dPP[3]) *
                     z +
                 __dy1_ep_dPP[4]) *
                    z +
                __dy1_ep_dPP[5]) *
                   z +
               __dy1_ep_dPP[6]) *
                  z +
              __dy1_ep_dPP[7]) *
                 z +
             __dy1_ep_dPP[8]) *
                z +
            __dy1_ep_dPP[9];
        q = (((((((((__dy1_ep_dQP[0] * z + __dy1_ep_dQP[1]) * z +
                    __dy1_ep_dQP[2]) *
                       z +
                   __dy1_ep_dQP[3]) *
                      z +
                  __dy1_ep_dQP[4]) *
                     z +
                 __dy1_ep_dQP[5]) *
                    z +
                __dy1_ep_dQP[6]) *
                   z +
               __dy1_ep_dQP[7]) *
                  z +
              __dy1_ep_dQP[8]) *
                 z +
             __dy1_ep_dQP[9]) *
            y;
        __dy1_ep_sincos_kernel_fp64(x, -3, &ps, &pc);
        r1 = p * ps;
        dr = r1 + q * pc;
        result = __sqrt(dtonpi * xi) * dr;
      }
    } else {
      /* x <= 0 */
      if ((iabsxhi | ixlo) != 0) {
        result = (*(double *)&linf) * 0.0; // NaN;
      } else {
        result = -1.0 / 0.0; // -Inf;
      }
    }
  }
  /* x is INF or NaN */
  else {
    /* SIGNIFICAND_NONZERO_64(ptx) */
    if (((iabsxhi & ~0x7FF00000) | ixlo) != 0) {
      result = x * 1.0; /* y1(NaN) = QNaN */
    } else if (isignxhi) {
      result = (*(double *)&linf) * 0.0; // NaN;
    } else {
      isign = ixhi >> 31;
      result = zeros[isign];
    }
  }
  *r = result;
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_y1_d_ep */
DEVICE_EXTERN_C_INLINE double __devicelib_imf_y1(double x) {
  using namespace __imf_impl_y1_d_ep;
  double r;
  __devicelib_imf_internal_dy1(&x, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
