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

#if defined(_WIN32) && defined (_MSC_VER)

#include <Windows.h>
#include <Winbase.h>
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
#include <float.h>
#include <xmmintrin.h>
#include "llvm/Support/DataTypes.h"

namespace Conformance
{

#define MAKE_HEX_FLOAT(x,y,z)  ((float)ldexp( (float)(y), z))
#define MAKE_HEX_DOUBLE(x,y,z) ldexp( (double)(y), z)
#define MAKE_HEX_LONG(x,y,z)   ((long double) ldexp( (long double)(y), z))

#define isfinite(x) _finite(x)

#if !defined(__cplusplus)
typedef char bool;
#define inline

#else
extern "C" {
#endif

//typedef unsigned char       uint8_t;
//typedef char                int8_t;
//typedef unsigned short      uint16_t;
//typedef short               int16_t;
//typedef unsigned int        uint32_t;
//typedef int                 int32_t;
//typedef unsigned long long  uint64_t;
//typedef long long           int64_t;

#define MAXPATHLEN MAX_PATH

typedef unsigned short ushort;
typedef unsigned int   uint;
typedef unsigned long  ulong;


//#define INFINITY    (FLT_MAX + FLT_MAX)
static unsigned int __infinity[] = {0x7f800000};
#define INFINITY (*(float *) Conformance::__infinity)
//#define NAN (INFINITY | 1)
//const static int PINFBITPATT_SP32  = INFINITY;

#ifndef M_PI
    #define M_PI    3.14159265358979323846264338327950288
#endif


#define    isnan( x )       ((x) != (x))
#define     isinf( _x)      ((_x) == INFINITY || (_x) == -INFINITY)

double rint( double x);
float  rintf( float x);
long double rintl( long double x);

float cbrtf( float );
double cbrt( double );

int    ilogb( double x);
int    ilogbf (float x);
int    ilogbl(long double x);

double fmax(double x, double y);
double fmin(double x, double y);
float  fmaxf( float x, float y );
float  fminf(float x, float y);

double      log2(double x);
long double log2l(long double x);

double      exp2(double x);
long double exp2l(long double x);

double      fdim(double x, double y);
float       fdimf(float x, float y);
long double fdiml(long double x, long double y);

double      remquo( double x, double y, int *quo);
float       remquof( float x, float y, int *quo);
long double remquol( long double x, long double y, int *quo);

long double scalblnl(long double x, long n);

// Already defined in VC10
#if _MSC_VER < 1600
inline long long
llabs(long long __x) { return __x >= 0 ? __x : -__x; }
#endif


// end of math functions

//uint64_t ReadTime( void );
//double SubtractTime( uint64_t endTime, uint64_t startTime );

#define sleep(X)   Sleep(1000*X)
#define snprintf   sprintf_s
//#define hypotl     _hypot

float   make_nan();
float nanf( const char* str);
double  nan( const char* str);
long double nanl( const char* str);

//#if defined USE_BOOST
//#include <boost/math/tr1.hpp>
//double hypot(double x, double y);
float hypotf(float x, float y);
long double hypotl(long double x, long double y) ;
double lgamma(double x);
float  lgammaf(float x);

double trunc(double x);
float  truncf(float x);

double log1p(double x);
float  log1pf(float x);
long double log1pl(long double x);

double copysign(double x, double y);
float  copysignf(float x, float y);
long double copysignl(long double x, long double y);

long lround(double x);
long lroundf(float x);
//long lroundl(long double x)

double round(double x);
float  roundf(float x);
long double roundl(long double x);

int signbit(double x);
int signbitf(float x);

//bool signbitl(long double x)         { return boost::math::tr1::signbit<long double>(x); }
//#endif // USE_BOOST

long int lrint (double flt);
long int lrintf (float flt);


float   int2float (int32_t ix);
int32_t float2int (float   fx);

/** Returns the number of leading 0-bits in x,
    starting at the most significant bit position.
    If x is 0, the result is undefined.
*/
int __builtin_clz(unsigned int pattern);


static const double zero=  0.00000000000000000000e+00;
#define NAN  (INFINITY - INFINITY)
#define HUGE_VALF (float)HUGE_VAL

int usleep(int usec);

// reimplement fenv.h because windows doesn't have it
#define FE_INEXACT          0x0020
#define FE_UNDERFLOW        0x0010
#define FE_OVERFLOW         0x0008
#define FE_DIVBYZERO        0x0004
#define FE_INVALID          0x0001
#define FE_ALL_EXCEPT       0x003D

int fetestexcept(int excepts);
int feclearexcept(int excepts);

#ifdef __cplusplus
}
#endif

#else // !((defined(_WIN32) && defined(_MSC_VER)
#if defined(__MINGW32__)
#include <windows.h>
#define sleep(X)   Sleep(1000*X)

#endif
#include<math.h>
namespace Conformance
{
/// Force gcc to use the same implementation of MAKE_HEX_* to
/// suppress warnings caused by x
#define MAKE_HEX_FLOAT(x,y,z)  ((float)ldexp( (float)(y), z))
#define MAKE_HEX_DOUBLE(x,y,z) ldexp( (double)(y), z)
#define MAKE_HEX_LONG(x,y,z)   ((long double) ldexp( (long double)(y), z))

using ::copysign;
using ::copysignf;
using ::copysignl;
using ::ilogb;
using ::ilogbf;
using ::ilogbl;

#endif // !((defined(_WIN32) && defined(_MSC_VER)

}

#endif // _COMPAT_H_
