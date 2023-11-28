/* file: iml_exp2f.h */

/* cvs_id[] = "$Id:" */

/*
** Copyright (C) 1985 Intel Corporation.
**
** The information and source code contained herein is the exclusive property
** of Intel Corporation and may not be disclosed, examined, or reproduced in
** whole or in part without explicit written authorization from the Company.
*/

/*
 * This file was copied here from the libdev/libm_ct/real/exp2f_bwr.c to
 * remove external dependency from math library.
 */

//++
//
//  MODIFICATION HISTORY:
//      27-Jul-2010, fence built-in cleaned out for MS compatibility. NA
//
//--


/* float exp2f(float x)
 * Returns the exponential of x.
 *
 * Generic CPU version
 *
 * Description:
 *  Let K = 64 (table size).
 *        x    n
 *       2  = 2 * (1 + T[j]) * (1 + P(y))
 *  where
 *       x = m/K + y,  y in [0.0..1/K]
 *       m = n*K + j,  m,n,j - signed integer, j in [-K/2..K/2]
 *                  j/K
 *       values of 2   are tabulated as T[j].
 *
 *       P(y) is a minimax polynomial approximation of exp2f(x)-1
 *       on small interval [0.0..1/K] (were calculated by Maple V).
 *
 * Special cases:
 *  exp2f(NaN) = NaN
 *  exp2f(+INF) = +INF
 *  exp2f(-INF) = 0
 *  exp2f(x) = 1 for subnormals
 *  for finite argument, exp2f returns exact result for any integer x in [-149..127]
 *  For IEEE float
 *    if x >= 128.0 then exp2f(x) overflow
 *    if x < -151.0 then exp2f(x) underflow
 */

#include <cassert>

static float IML_ATTR_exp2f (float x)
{
    /* macros to sign-expand low 'num' bits of 'val' to native integer */
    # define SIGN_EXPAND(val,num)  ((int)(val) << (32-(num))) >> (32-(num)) /* sign expand of 'num' LSBs */

    #define EXACT_RESULTS   /* raise inexact more accurately, but slow down the routine */

    static const unsigned range[] = { 0x42ffffff,0x431e0000 };  /* 128,151 */

    static const double P[] = {
         .69314718055214481968005225839151665254000291772723e-0 /* 3FE62E42 FEFA39EF */
        ,.24022651095133014901564508704543890813313513082204e-0 /* 3FCEBFBD FF82C58F */
        ,.55503393667531251585018864474520942624622996868437e-1 /* 3FAC6B08 D704A135 */
        ,.96703711395723542012735846658159878679459918215073e-2 /* 3F83B2AB 6FB88304 */
    };

    static const double KONE = 64.0;    /* 40500000 00000000 K */

    static const double NONEK = -.156250e-01; /* 3F900000 00000000 1/K */

    #define inf_zero ((const float *)_inf_zero)
    static const unsigned _inf_zero[] = { 0x7f800000,0x00000000 };  /* +INF,+0 */

    /* miscellaneous data */

    union uTWO_23H {
      unsigned u;
      float f;
    } tTWO_23H;
    #define TWO_23H (tTWO_23H.f)
    tTWO_23H.u = 0x4b400000 /* 2^23+2^22 */;

    union uI32x2F32x2 {
      unsigned u[2];
      float f[2];
    };

    union uI32x2F32x2 t_large_value_32, t_small_value_32;

    #define OVERFLOW_32(S) (large_value_32[(S)] * large_value_32[0])
    #define large_value_32 (t_large_value_32.f)
    t_large_value_32.u[0] = 0x71800000; /* +2^100 */
    t_large_value_32.u[1] = 0xf1800000; /* -2^100 */

    #define UNDERFLOW_32(S) (small_value_32[(S)] * small_value_32[0])
    #define small_value_32 (t_small_value_32.f)
    t_small_value_32.u[0] = 0x0d800000; /* +2^(-100) */
    t_small_value_32.u[1] = 0x8d800000; /* -2^(-100) */

    static const float ones[2] = { 1.0f,-1.0f };

    #define BIAS_32      127
    #define BIAS_64     1023

    #define EXPO_32(i) (((i) >> 23) - BIAS_32)
    #define FRACTION_32(i) ((i) << (32-23 + EXPO_32(i)))

    union uI32F32 {
      int i;
      float f;
    };

    union uI32F32 tI32_tv, tI32_resultf, tI32_scf, tI32_x;

    union uI64_scale {
      long long int i;
      double d;
    } tI64_scale;

    /* exp2f() data */
    #define L 6
    #define K 64 /* 2^L */

    #define T __libm_expf_table_64
    #include "iml_expf_table.h"


    unsigned ix;
    int sign,m,n,j;
    float resultf;
    float volatile tv;
    double y,z,t,p;

    tI32_x.f = x;
    ix = (tI32_x.i & ~0x80000000);
#ifdef EXACT_RESULTS
    /* check for argument values for which result must be exact.
    * for exp2f, this is all integers in the inclusive range [-149..127].
    */
    if ((ix-0x3f800000 <= 0x42fe0000-0x3f800000) && (FRACTION_32(ix) == 0) &&   /* |x| >= 1.0, x is integer */
        (tI32_tv.f = (x + TWO_23H), m = SIGN_EXPAND (tI32_tv.i, 23-1), m >= -149)) {     /* m = (int)x */
            if ((m += BIAS_32) > 0) {   /* exact normal result */
                tI32_resultf.i = ((int)m << 23);
            } else {                    /* exact subnormal resultf */
                tI32_resultf.i = ((int)1 << (m+22));
            }
            return tI32_resultf.f;
    }
#endif  /*EXACT_RESULTS*/
    if (ix < 0x42fa0000 /*125.0*/) {    /* 0 <= |x| < 125.0 */
        if (ix < 0x31800000) {  /* 0 <= |x| < 2^(-28) */
            return tv = (ones[0] + x);   /* value of 1.0 with inexact raised, except for x==0 */
        }
        tv = (x*KONE + TWO_23H); t = (tv - TWO_23H); tI32_tv.f = tv; m = SIGN_EXPAND (tI32_tv.i, 23-1);
        y = (x + t*NONEK); j = SIGN_EXPAND (m, L); n = (m - j) >> L;
        tI32_scf.i = (int)( n + BIAS_32 ) << 23;
        z = y*y; p = (P[3]*z + P[1])*z + (P[2]*z + P[0])*y;
        resultf = (p*T[K/2+j] + T[K/2+j])*tI32_scf.f;
        return resultf;
    }
    assert(tI32_x.f == x && "Expected x not to be redefined in this function");
    sign = ((unsigned)tI32_x.i >> 31);
    if (ix <= range[sign]) {    /* 125.0 <= |x| <= range */
        tv = (x*KONE + TWO_23H); t = (tv - TWO_23H); tI32_tv.f = tv; m = SIGN_EXPAND (tI32_tv.i, 23-1);
        y = (x + t*NONEK); j = SIGN_EXPAND (m, L); n = (m - j) >> L;
        tI64_scale.i = (long long int)( BIAS_64 + n ) << 52;
        z = y*y; p = (P[3]*z + P[1])*z + (P[2]*z + P[0])*y;
        tI32_resultf.i = (p*T[K/2+j] + T[K/2+j])*tI64_scale.d;
        if (tI32_resultf.i < 0x00800000) {
            //__libm_error_support (&x, &x, &resultf, exp2f_underflow);
        }
        return tI32_resultf.f;
    }
    /* NaN, INF, or range error */
    if (ix < 0x7f800000) {  /* range error */
        if (sign) {
            resultf = UNDERFLOW_32(0);  /* raise underflow+inexact, return +0/+MinSub */
            //__libm_error_support (&x, &x, &resultf, exp2f_underflow);
            return resultf;
        } else {
            resultf = OVERFLOW_32(0);   /* raise overflow+inexact, return +INF/+MaxNorm */
            //__libm_error_support (&x, &x, &resultf, exp2f_overflow);
            return resultf;
        }
    } else {                /* NaN or INF */
        if (ix == 0x7f800000) { /* INF */
            return inf_zero[sign];
        } else {            /* NaN */
            return x + x;   /* raise invalid on SNaN */
        }
    }
}
