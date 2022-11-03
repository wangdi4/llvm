/******************************************************************
 //
 //  OpenCL Conformance Tests
 //
 //  Copyright: (c) 2008-2009 by Apple Inc. All Rights Reserved.
 //
 ******************************************************************/

// File includes functions and macros needed for compilation by
// Microsoft Visual Studio

#ifndef _COMPAT_H_
#define _COMPAT_H_

/*
    ------------------------------------------------------------------------------------------------
    HEX_FLT, HEXT_DBL, HEX_LDBL -- Create hex floating point literal of type
   float, double, long double respectively. Arguments:

        sm    -- sign of number,
        int   -- integer part of mantissa (without `0x' prefix),
        fract -- fractional part of mantissa (without decimal point and `L' or
   `LL' suffixes), se    -- sign of exponent, exp   -- absolute value of
   (binary) exponent.

    Example:

        double yhi = HEX_DBL( +, 1, 5555555555555, -, 2 ); // ==
   0x1.5555555555555p-2

    Note:

        We have to pass signs as separate arguments because gcc pass negative
   integer values (e. g. `-2') into a macro as two separate tokens, so `HEX_FLT(
   1, 0, -2 )' produces result `0x1.0p- 2' (note a space between minus and two)
   which is not a correct floating point literal.
    ------------------------------------------------------------------------------------------------
*/

#if !defined(__INTEL_COMPILER)
// If compiler does not support hex floating point literals:
#define HEX_FLT(sm, int, fract, se, exp)                                       \
  sm ::ldexpf((float)(0x##int##fract##UL),                                     \
              se exp + ::ilogbf((float)0x##int) -                              \
                  ::ilogbf((float)(0x##int##fract##UL)))
#define HEX_DBL(sm, int, fract, se, exp)                                       \
  sm ::ldexp((double)(0x##int##fract##ULL),                                    \
             se exp + ::ilogb((double)0x##int) -                               \
                 ::ilogb((double)(0x##int##fract##ULL)))
#define HEX_LDBL(sm, int, fract, se, exp)                                      \
  sm ::ldexpl((long double)(0x##int##fract##ULL),                              \
              se exp + ::ilogbl((long double)0x##int) -                        \
                  ::ilogbl((long double)(0x##int##fract##ULL)))
#else
// If compiler supports hex floating point literals: just concatenate all the
// parts into a literal.
#define HEX_FLT(sm, int, fract, se, exp) sm 0x##int##.##fract##p##se##exp##F
#define HEX_DBL(sm, int, fract, se, exp) sm 0x##int##.##fract##p##se##exp
#define HEX_LDBL(sm, int, fract, se, exp) sm 0x##int##.##fract##p##se##exp##L
#endif

#if defined(_WIN32) && defined(_MSC_VER)

#include <Winbase.h>
#include <Windows.h>
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#ifdef TRUE
#undef TRUE
#endif
#ifdef FALSE
#undef FALSE
#endif
#include "CL/cl.h"
#include "llvm/Support/DataTypes.h"
#include <float.h>
#include <xmmintrin.h>

namespace Conformance {

/*
    ------------------------------------------------------------------------------------------------
    WARNING: DO NOT USE THESE MACROS: MAKE_HEX_FLOAT, MAKE_HEX_DOUBLE,
   MAKE_HEX_LONG.

    This is a typical usage of the macros:

        double yhi =
   MAKE_HEX_DOUBLE(0x1.5555555555555p-2,0x15555555555555LL,-2);

     (taken from math_brute_force/reference_math.c). There are two problems:

        1.  There is an error here. On Windows it will produce incorrect result
            `0x1.5555555555555p+50'. To have a correct result it should be
   written as `MAKE_HEX_DOUBLE(0x1.5555555555555p-2,0x15555555555555LL,-54)'. A
   proper value of the third argument is not obvious -- sometimes it should be
   the same as exponent of the first argument, but sometimes not.

        2.  Information is duplicated. It is easy to make a mistake.

    Use HEX_FLT, HEX_DBL, HEX_LDBL macros instead.
    ------------------------------------------------------------------------------------------------
*/
#define MAKE_HEX_FLOAT(x, y, z) ((float)ldexp((float)(y), z))
#define MAKE_HEX_DOUBLE(x, y, z) ldexp((double)(y), z)
#define MAKE_HEX_LONG(x, y, z) ((long double)ldexp((long double)(y), z))

#define isfinite(x) _finite(x)

#if !defined(__cplusplus)
typedef char bool;
#define inline

// #else
//  extern "C" {
#endif

// typedef unsigned char       uint8_t;
// typedef char                int8_t;
// typedef unsigned short      uint16_t;
// typedef short               int16_t;
// typedef unsigned int        uint32_t;
// typedef int                 int32_t;
// typedef unsigned long long  uint64_t;
// typedef long long           int64_t;

#define MAXPATHLEN MAX_PATH

typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

// #define INFINITY    (FLT_MAX + FLT_MAX)
//  static unsigned int __infinity[] = {0x7f800000};
// #define INFINITY (*(float *) Conformance::__infinity)
// #define NAN (INFINITY | 1)
//  const static int PINFBITPATT_SP32  = INFINITY;

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#define isnan(x) ((x) != (x))
#define isinf(_x) ((_x) == INFINITY || (_x) == -INFINITY)

double rint(double x);
float rintf(float x);
long double rintl(long double x);

float cbrtf(float);
double cbrt(double);

int ilogb(double x);
int ilogbf(float x);
int ilogbl(long double x);

double fmax(double x, double y);
double fmin(double x, double y);
float fmaxf(float x, float y);
float fminf(float x, float y);

double log2(double x);
long double log2l(long double x);

double exp2(double x);
long double exp2l(long double x);

double fdim(double x, double y);
float fdimf(float x, float y);
long double fdiml(long double x, long double y);

double remquo(double x, double y, int *quo);
float remquof(float x, float y, int *quo);
long double remquol(long double x, long double y, int *quo);

long double scalblnl(long double x, long n);

// Already defined in VC10
#if _MSC_VER < 1600
inline long long llabs(long long __x) { return __x >= 0 ? __x : -__x; }
#endif

// end of math functions

// uint64_t ReadTime( void );
// double SubtractTime( uint64_t endTime, uint64_t startTime );

#define sleep(X) Sleep(1000 * X)
#define snprintf sprintf_s
// #define hypotl     _hypot

float make_nan();
float nanf(const char *str);
double nan(const char *str);
long double nanl(const char *str);

// #if defined USE_BOOST
// #include <boost/math/tr1.hpp>
//  double hypot(double x, double y);
float hypotf(float x, float y);
long double hypotl(long double x, long double y);
double lgamma(double x);
float lgammaf(float x);

double trunc(double x);
float truncf(float x);

double log1p(double x);
float log1pf(float x);
long double log1pl(long double x);

double copysign(double x, double y);
float copysignf(float x, float y);
long double copysignl(long double x, long double y);

long lround(double x);
long lroundf(float x);
// long lroundl(long double x)

double round(double x);
float roundf(float x);
long double roundl(long double x);

int signbit(double x);
int signbitf(float x);

// bool signbitl(long double x)         { return boost::math::tr1::signbit<long
// double>(x); } #endif // USE_BOOST

long int lrint(double flt);
long int lrintf(float flt);

float int2float(int32_t ix);
int32_t float2int(float fx);

/** Returns the number of leading 0-bits in x,
    starting at the most significant bit position.
    If x is 0, the result is undefined.
*/
int __builtin_clz(unsigned int pattern);

static const double zero = 0.00000000000000000000e+00;
// #define NAN  (INFINITY - INFINITY)
// #define HUGE_VALF (float)HUGE_VAL

int usleep(int usec);

// reimplement fenv.h because windows doesn't have it
#define FE_INEXACT 0x0020
#define FE_UNDERFLOW 0x0010
#define FE_OVERFLOW 0x0008
#define FE_DIVBYZERO 0x0004
#define FE_INVALID 0x0001
#define FE_ALL_EXCEPT 0x003D

int fetestexcept(int excepts);
int feclearexcept(int excepts);

#ifdef __cplusplus
//}
#endif

#else // !((defined(_WIN32) && defined(_MSC_VER)
#if defined(__MINGW32__)
#include <windows.h>
#define sleep(X) Sleep(1000 * X)

#endif
#include <math.h>
namespace Conformance {
/// Force gcc to use the same implementation of MAKE_HEX_* to
/// suppress warnings caused by x
#define MAKE_HEX_FLOAT(x, y, z) ((float)ldexp((float)(y), z))
#define MAKE_HEX_DOUBLE(x, y, z) ldexp((double)(y), z)
#define MAKE_HEX_LONG(x, y, z) ((long double)ldexp((long double)(y), z))

using ::copysign;
using ::copysignf;
using ::copysignl;
using ::ilogb;
using ::ilogbf;
using ::ilogbl;

#endif // !((defined(_WIN32) && defined(_MSC_VER)
}

#endif // _COMPAT_H_
