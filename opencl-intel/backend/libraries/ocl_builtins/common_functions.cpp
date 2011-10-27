// Copyright (c) 2006-2007 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////
//  svml_naive_functions.cpp
///////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

// Compiled with Clang as LLVM module
#define ALIGN16 __attribute__((aligned(16)))
#define ALIGN32 __attribute__((aligned(32)))
#include <intrin.h>
#include "cl_common_declaration.h"

//#define INTRL_180_PI 57.2957795130823208767981548141 //180/pi
#define INRNL_PI        3.14159265358979323846264338327950288   /* pi */

//#define INTRL_PI_180 0.017453292519943295769236907684883 //pi/180
#define INTRL_PI_180	0.017453292519943295769236907684883 //pi/180 this precision needed for conformance
#define INTRL_180_PI	57.295779513082320876798154814105 //180/pi this precision needed for conformance

#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#define MIN(a,b)	(((a) < (b)) ? (a) : (b))

const float ALIGN16 f4const_1_180divpi[4] = {INTRL_180_PI, INTRL_180_PI, INTRL_180_PI, INTRL_180_PI};
const float ALIGN16 f4const_1_pidiv180[4] = {INTRL_PI_180, INTRL_PI_180, INTRL_PI_180, INTRL_PI_180};

const double ALIGN32 d2const_1_180divpi[4] = {INTRL_180_PI, INTRL_180_PI, INTRL_180_PI, INTRL_180_PI};
const double ALIGN32 d2const_1_pidiv180[4] = {INTRL_PI_180, INTRL_PI_180, INTRL_PI_180, INTRL_PI_180};

const float ALIGN16 f4const_oneStorage[4]       = {1.0f, 1.0f, 1.0f, 1.0f};
const float ALIGN16 f4const_minusZeroStorage[4] = {-0.0f, -0.0f, -0.0f, -0.0f};
const float ALIGN16 f4const_minusOneStorage[4]  = {-1.0f, -1.0f, -1.0f, -1.0f};
const float ALIGN16 f4const_two[4] = {2, 2, 2, 2};
const float ALIGN16 f4const_three[4] = {3, 3, 3, 3};
const float ALIGN16 f4const_nanStorage[4] = {0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF};

const double ALIGN16 d2const_oneStorage[2]       = {1.0f, 1.0f};
const double ALIGN16 d2const_minusZeroStorage[2] = {-0.0f, -0.0f};
const double ALIGN16 d2const_minusOneStorage[2]  = {-1.0f, -1.0f};
const double ALIGN16 d2const_two[2] = {2, 2};
const double ALIGN16 d2const_three[2] = {3, 3};
const double ALIGN16 d2const_nanStorage[2] = {0x7FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF};

#if defined (__AVX__)
const float ALIGN32 f8const_minusZeroStorage[8] = {-0.0f, -0.0f, -0.0f, -0.0f, -0.0f, -0.0f, -0.0f, -0.0f};
const float ALIGN32 f8const_1_180divpi[8] = {INTRL_180_PI, INTRL_180_PI, INTRL_180_PI, INTRL_180_PI, INTRL_180_PI, INTRL_180_PI, INTRL_180_PI, INTRL_180_PI};
const float ALIGN32 f8const_1_pidiv180[8] = {INTRL_PI_180, INTRL_PI_180, INTRL_PI_180, INTRL_PI_180, INTRL_PI_180, INTRL_PI_180, INTRL_PI_180, INTRL_PI_180};
const float ALIGN32 f8const_nanStorage[8] = {0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF};
const float ALIGN32 f8const_oneStorage[8]       = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
const float ALIGN32 f8const_minusOneStorage[8]  = {-1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f};

const float ALIGN32 f8const_three[8] = {3, 3, 3, 3, 3, 3, 3, 3};

const double ALIGN32 d4const_two[4] = {2, 2, 2, 2};
const double ALIGN32 d4const_three[4] = {3, 3, 3, 3};
const double ALIGN32 d4const_oneStorage[4]       = {1.0f, 1.0f, 1.0f, 1.0f};
const double ALIGN32 d4const_nanStorage[4] = {0x7FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF};
const double ALIGN32 d4const_minusZeroStorage[4] = {-0.0f, -0.0f, -0.0f, -0.0f};
const double ALIGN32 d4const_minusOneStorage[4]  = {-1.0f, -1.0f, -1.0f, -1.0f};
#endif

//clamp 
//Returns fmin(fmax(x, minval), maxval) .
//Results are undefined if minval > maxval.
float  __attribute__((overloadable)) clamp(float x, float minval, float maxval)
{
	__m128 xmmX = _mm_load_ss(&x);
	__m128 xmmMin = _mm_load_ss(&minval);
	__m128 xmmMax = _mm_load_ss(&maxval);
	xmmX = _mm_max_ss(xmmX, xmmMin);
	xmmX = _mm_min_ps(xmmX, xmmMax);
	float res;
	_mm_store_ss(&res, xmmX);
	return res;
}
float4  __attribute__((overloadable)) clamp(float4 x, float4 minval, float4 maxval)
{
	float4 max =  _mm_max_ps(x,minval);
	return _mm_min_ps(max, maxval);
}

#if defined(__AVX__)
float8  __attribute__((overloadable)) clamp(float8 x, float8 minval, float8 maxval)
{
		float8 max =  _mm256_max_ps(x,minval);
		return _mm256_min_ps(max, maxval);
}
#else
float8  __attribute__((overloadable)) clamp(float8 x, float8 minval, float8 maxval)
{
		float8 res;
		res.lo = clamp(x.lo, minval.lo, maxval.lo);
		res.hi = clamp(x.hi, minval.hi, maxval.hi);
		return res;
}
#endif


double __attribute__((overloadable)) clamp(double x, double minval, double maxval)
{
	return  MIN(MAX(x, minval), maxval);
}

double2 __attribute__((overloadable)) clamp(double2 x, double2 minval, double2 maxval)
{
	double2 max =  _mm_max_pd(x,minval);
	return _mm_min_pd(max, maxval);
}
#if defined (__AVX__)
double4 __attribute__((overloadable)) clamp(double4 x, double4 minval, double4 maxval)
{
	double4 max =  _mm256_max_pd(x,minval);
	return _mm256_min_pd(max, maxval);
}
#else
double4 __attribute__((overloadable)) clamp(double4 x, double4 minval, double4 maxval)
{
	double4 res;
	res.lo = clamp(x.lo, minval.lo, maxval.lo);
	res.hi = clamp(x.hi, minval.hi, maxval.hi);
	return res;
}
#endif


//degrees
//Converts radians to degrees, i.e. (180 / pi) *
//radians
float  __attribute__((overloadable)) degrees(float radians)
{
	return radians * INTRL_180_PI;
}
float4   __attribute__((overloadable)) degrees(float4 radians)
{
	return _mm_mul_ps(radians, *( (__m128*)f4const_1_180divpi ) );
}


#if defined (__AVX__)
float8   __attribute__((overloadable)) degrees(float8 radians)
{
	return _mm256_mul_ps(radians, *( (__m256*)f8const_1_180divpi ) );
}
#else
float8   __attribute__((overloadable)) degrees(float8 radians)
{
	float8 res;
	res.lo = degrees(radians.lo);
	res.hi = degrees(radians.hi);
	return res;
}
#endif

double  __attribute__((overloadable)) degrees(double radians)
{
	return radians * INTRL_180_PI;
}
double2   __attribute__((overloadable)) degrees(double2 radians)
{
	return _mm_mul_pd(radians, *( (__m128d*)d2const_1_180divpi ) );
}

#if defined (__AVX__)
double4   __attribute__((overloadable)) degrees(double4 radians)
{
	return _mm256_mul_pd(radians, *( (__m256d*)d2const_1_180divpi ) );
}
#else
double4   __attribute__((overloadable)) degrees(double4 radians)
{
	double4 res;
	res.lo = degrees(radians.lo);
	res.hi = degrees(radians.hi);
	return res;
}
#endif

//max
//Returns y if x < y, otherwise it returns x. If x and y
//are infinite or NaN, the return values are undefined
float  __attribute__((overloadable)) max(float x, float y)
{
	__m128 xmmX = _mm_load_ss(&x);
	__m128 xmmY = _mm_load_ss(&y);
	xmmX = _mm_max_ss(xmmX, xmmY);
	float res;
	_mm_store_ss(&res, xmmX);
	return res;
}
float4  __attribute__((overloadable)) max(float4 x, float4 y)
{
	return _mm_max_ps(x,y);
}

#if defined (__AVX__)
float8  __attribute__((overloadable)) max(float8 x, float8 y)
{
	return _mm256_max_ps(x,y);
}
#else 
float8  __attribute__((overloadable)) max(float8 x, float8 y)
{
	float8 res;
	res.lo = max(x.lo, y.lo);
	res.hi = max(x.hi, y.hi);
	return res;
}
#endif

double  __attribute__((overloadable)) max(double x, double y)
{
	return MAX(x,y);
}
double2  __attribute__((overloadable)) max(double2 x, double2 y)
{
	return _mm_max_pd(x,y);
}
#if defined (__AVX__)
double4  __attribute__((overloadable)) max(double4 x, double4 y)
{
	return _mm256_max_pd(x,y);
}
#else
double4  __attribute__((overloadable)) max(double4 x, double4 y)
{
	double4  res;
	res.lo = max(x.lo, y.lo);
	res.hi = max(x.hi, y.hi);
	return res;
}
#endif
//min
//Returns y if x > y, otherwise it returns x. If x and y
//are infinite or NaN, the return values are undefined
float  __attribute__((overloadable)) min(float x, float y)
{
	__m128 xmmX = _mm_load_ss(&x);
	__m128 xmmY = _mm_load_ss(&y);
	xmmX = _mm_min_ss(xmmX, xmmY);
	float res;
	_mm_store_ss(&res, xmmX);
	return res;
}
float4  __attribute__((overloadable)) min(float4 x, float4 y)
{
	return _mm_min_ps(x,y);
}
#if defined (__AVX__)
float8  __attribute__((overloadable)) min(float8 x, float8 y)
{
	return _mm256_min_ps(x,y);
}
#else
float8  __attribute__((overloadable)) min(float8 x, float8 y)
{
	float8 res;
	res.lo = min(x.lo, y.lo);
	res.hi = min(x.hi, y.hi);
	return res;
}
#endif

double  __attribute__((overloadable)) min(double x, double y)
{
	return MIN(x,y);
}
double2  __attribute__((overloadable)) min(double2 x, double2 y)
{
	return _mm_min_pd(x,y);
}

#if defined (__AVX__)
double4  __attribute__((overloadable)) min(double4 x, double4 y)
{
	return _mm256_min_pd(x,y);
}
#else
double4  __attribute__((overloadable)) min(double4 x, double4 y)
{
	double4  res;
	res.lo = min(x.lo, y.lo);
	res.hi = min(x.hi, y.hi);
	return res;
}
#endif

//mix
//Returns the linear blend of x & y implemented as:
//x + (y – x) * a
//a must be a value in the range 0.0 … 1.0. If a is not
//in the range 0.0 … 1.0, the return values are
//undefined.
float   __attribute__((overloadable)) mix(float x,float y, float a)
{
	float tmp = y-x;
	tmp = tmp * a;
	return x + tmp;
}

float4   __attribute__((overloadable)) mix(float4 x,float4 y, float4 a)
{
	float4 tmp = _mm_sub_ps(y, x);
	tmp = _mm_mul_ps(tmp, a);
	return _mm_add_ps(x, tmp);
}
#if defined(__AVX__)
float8   __attribute__((overloadable)) mix(float8 x,float8 y, float8 a)
{
	float8 tmp = _mm256_sub_ps(y, x);
	tmp = _mm256_mul_ps(tmp, a);
	return _mm256_add_ps(x, tmp);
}
#else
float8   __attribute__((overloadable)) mix(float8 x,float8 y, float8 a)
{
	float8 res;
	res.lo = mix(x.lo, y.lo, a.lo);
	res.hi = mix(x.hi, y.hi, a.hi);
	return res;
}
#endif


double   __attribute__((overloadable)) mix(double x,double y, double a)
{
	double tmp = y-x;
	tmp = tmp * a;
	return x + tmp;
}

double2   __attribute__((overloadable)) mix(double2 x,double2 y, double2 a)
{
	double2 tmp = _mm_sub_pd(y, x);
	tmp = _mm_mul_pd(tmp, a);
	return _mm_add_pd(x, tmp);
}

#if defined (__AVX__)
double4   __attribute__((overloadable)) mix(double4 x,double4 y, double4 a)
{
	double4 tmp = _mm256_sub_pd(y, x);
	tmp = _mm256_mul_pd(tmp, a);
	return _mm256_add_pd(x, tmp);
}
#else 
double4 __attribute__((overloadable)) mix(double4 x, double4 y, double4 z)
{
	double4 res;
	res.lo = mix(x.lo, y.lo, z.lo);
	res.hi = mix(x.hi, y.hi, z.hi);
	return res;
}
#endif

//radians
//Converts  __degrees to radians, i.e. (pi / 180) *
// __degrees.
float  __attribute__((overloadable)) radians(float radians)
{
	return radians * INTRL_PI_180;
}
float4  __attribute__((overloadable)) radians(float4 radians)
{
	return _mm_mul_ps(radians, *((__m128*)f4const_1_pidiv180) );
}
#if defined (__AVX__)
float8  __attribute__((overloadable)) radians(float8 _radians)
{
	return _mm256_mul_ps(_radians, *((__m256*)f8const_1_pidiv180) );
}
#else
float8  __attribute__((overloadable)) radians(float8 _radians)
{
	float8 res;
	res.lo = radians(_radians.lo);
	res.hi = radians(_radians.hi);
	return res;
}
#endif


double  __attribute__((overloadable)) radians(double radians)
{
	return radians * INTRL_PI_180;
}

double2  __attribute__((overloadable)) radians(double2 radians)
{
	return _mm_mul_pd(radians, *( (__m128d*)d2const_1_pidiv180 ) );
}

#if defined (__AVX__)
double4  __attribute__((overloadable)) radians(double4 radians)
{
	return _mm256_mul_pd(radians, *( (__m256d*)d2const_1_pidiv180 ) );
}
#else
double4 __attribute__((overloadable)) radians(double4 _radians)
{
	double4 res;
	res.lo = radians(_radians.lo);
	res.hi = radians(_radians.hi);
	return res;
}
#endif

//step
//Returns 0.0 if x < edge, otherwise it returns 1.0
float  __attribute__((overloadable)) step(float edge, float x)
{
	if(x < edge)
	{
		return 0.0;
	}
	else 
	{
		return 1.0;
	}
}
float4  __attribute__((overloadable)) step(float4 edge, float4 x)
{
//if x>=edge return -1 else (x<edge) return 0
	float4 res = _mm_cmpge_ps(x, edge);
//Convert -1 to 1
	res =  _mm_and_ps(res, *((float4 *)f4const_oneStorage));
	return res;
}
#if defined (__AVX__)
float8  __attribute__((overloadable)) step(float8 edge, float8 x)
{	
	//if x>=edge return -1 else (x<edge) return 0
		float8 res = _mm256_cmp_ps(x, edge, _CMP_GE_OS);
	//Convert -1 to 1
		res =  _mm256_and_ps(res, *((float8 *)f8const_oneStorage));
		return res;
}
#else
float8  __attribute__((overloadable)) step(float8 edge, float8 x)
{
	float8 res;
	res.lo = step(edge.lo, x.lo);
	res.hi = step(edge.hi, x.hi);
	return res;
}
#endif


double  __attribute__((overloadable)) step(double edge, double x)
{
	if(x < edge)
	{
		return 0.0;
	}
	else 
	{
		return 1.0;
	}
}
double2  __attribute__((overloadable)) step(double2 edge, double2 x)
{
	//if x>=edge return -1 else (x<edge) return 0
	double2 res = _mm_cmpge_pd(x, edge);
	//Convert -1 to 1
	res =  _mm_and_pd(res, *((double2 *)d2const_oneStorage));
	return res;
}

#if defined (__AVX__)
double4  __attribute__((overloadable)) step(double4 edge, double4 x)
{
	//if x>=edge return -1 else (x<edge) return 0
	double4 res = _mm256_cmp_pd(x, edge, _CMP_GE_OS);
	//Convert -1 to 1
	res =  _mm256_and_pd(res, *((double4 *)d4const_oneStorage));
	return res;
}
#else
double4  __attribute__((overloadable)) step(double4 edge, double4 x)
{
	double4 res;
	res.lo = step(edge.lo, x.lo);
	res.hi = step(edge.hi, x.hi);
	return res;
}
#endif

//Returns 0.0 if x <= edge0 and 1.0 if x >= edge1 and
//performs smooth Hermite interpolation between 0
//and 1when edge0 < x < edge1. This is useful in
//cases where you would want a threshold function
//with a smooth transition.
//This is equivalent to:
//gentype t;
//t = clamp ((x – edge0) / (edge1 – edge0), 0, 1);
//return t * t * (3 – 2 * t);
//Results are undefined if edge0 >= edge1 or if x,
//edge0 or edge1 is a NaN.
float  __attribute__((overloadable)) smoothstep(float edge0, float edge1, float x)
{
	float t = (x - edge0) / (edge1 - edge0);
	if (t < 0.0f)
     return 0.0f;
    else if (t > 1.0f)
      return 1.0f;
    return t * t * (3.0f - 2.0f * t);
	
}
float4  __attribute__((overloadable)) smoothstep(float4 edge0, float4 edge1, float4 x)
{
	float4 tmp1, tmp2;
	tmp1 = _mm_sub_ps(x, edge0); //tmp1  = (x – edge0)
	tmp2 = _mm_sub_ps(edge1, edge0); //tmp2 = (edge1 – edge0)
	tmp1 = _mm_div_ps(tmp1, tmp2); //tmp1  = (x – edge0)/(edge1 – edge0)
	tmp1 = clamp(tmp1, (float4)_mm_setzero_ps(), (float4)*((__m128*)f4const_oneStorage)); //tmp1 = clamp ((x – edge0) / (edge1 – edge0), 0, 1);
	tmp2 = _mm_add_ps(tmp1, tmp1); //tmp2 = 2 * t
	tmp2 = _mm_sub_ps(*((__m128*)f4const_three), tmp2); //tmp2 = (3 – 2 * t)
	tmp1 = _mm_mul_ps(tmp1, tmp1); //tmp 1 = tmp1 * tmp1
	float4 res = _mm_mul_ps(tmp2, tmp1); //res2 = t * t * (3 – 2 * t)
	return res;//_mm_or_ps(tmp3, tmp4);
}

#if defined(__AVX__)
float8  __attribute__((overloadable)) smoothstep(float8 edge0, float8 edge1, float8 x)
{
	float8 tmp1, tmp2;
	tmp1 = _mm256_sub_ps(x, edge0); //tmp1  = (x – edge0)
	tmp2 = _mm256_sub_ps(edge1, edge0); //tmp2 = (edge1 – edge0)
	tmp1 = _mm256_div_ps(tmp1, tmp2); //tmp1  = (x – edge0)/(edge1 – edge0)
	tmp1 = clamp(tmp1, (float8)_mm256_setzero_ps() , (float8)*((__m256*)f8const_oneStorage)); //tmp1 = clamp ((x – edge0) / (edge1 – edge0), 0, 1);
	tmp2 = _mm256_add_ps(tmp1, tmp1); //tmp2 = 2 * t
	tmp2 = _mm256_sub_ps(*((__m256*)f8const_three), tmp2); //tmp2 = (3 – 2 * t)
	tmp1 = _mm256_mul_ps(tmp1, tmp1); //tmp 1 = tmp1 * tmp1
	float8 res = _mm256_mul_ps(tmp2, tmp1); //res2 = t * t * (3 – 2 * t)
	return res;//_mm_or_ps(tmp3, tmp4);
}
#else
float8  __attribute__((overloadable)) smoothstep(float8 edge0, float8 edge1, float8 x)
{
	float8 res;
	res.lo = smoothstep(edge0.lo, edge1.lo, x.lo);
	res.hi = smoothstep(edge0.hi, edge1.hi, x.hi);
	return res;
}
#endif


double  __attribute__((overloadable)) smoothstep(double edge0, double edge1, double x)
{
	float t = (x - edge0) / (edge1 - edge0);
	if (t < 0.0f)
      t = 0.0f;
    else if (t > 1.0f)
      t = 1.0f;
    return t * t * (3.0f - 2.0f * t);
}
double2  __attribute__((overloadable)) smoothstep(double2 edge0, double2 edge1, double2 x)
{
	double2 tmp1, tmp2;
	tmp1 = _mm_sub_pd(x, edge0); //tmp1  = (x – edge0)
	tmp2 = _mm_sub_pd(edge1, edge0); //tmp2 = (edge1 – edge0)
	tmp1 = _mm_div_pd(tmp1, tmp2); //tmp1  = (x – edge0)/(edge1 – edge0)
	tmp1 = clamp(tmp1, 0.0, 1.0); //tmp1 = clamp ((x – edge0) / (edge1 – edge0), 0, 1);
	tmp2 = _mm_mul_pd(tmp1, *((double2*)d2const_two)); //tmp2 = 2 * t
	tmp2 = _mm_sub_pd(*((double2*)d2const_three), tmp2); //tmp2 = (3 – 2 * t)
	tmp1 = _mm_mul_pd(tmp1, tmp1); //tmp 1 = tmp1 * tmp1
	double2 res = _mm_mul_pd(tmp2, tmp1); //res2 = t * t * (3 – 2 * t)
	return res;//_mm_or_ps(tmp3, tmp4);
}

#if defined (__AVX__)
double4  __attribute__((overloadable)) smoothstep(double4 edge0, double4 edge1, double4 x)
{
	double4 tmp1, tmp2;
	tmp1 = _mm256_sub_pd(x, edge0); //tmp1  = (x – edge0)
	tmp2 = _mm256_sub_pd(edge1, edge0); //tmp2 = (edge1 – edge0)
	tmp1 = _mm256_div_pd(tmp1, tmp2); //tmp1  = (x – edge0)/(edge1 – edge0)
	tmp1 = clamp(tmp1, 0.0, 1.0); //tmp1 = clamp ((x – edge0) / (edge1 – edge0), 0, 1);
	tmp2 = _mm256_mul_pd(tmp1, *((double4*)d4const_two)); //tmp2 = 2 * t
	tmp2 = _mm256_sub_pd(*((double4*)d4const_three), tmp2); //tmp2 = (3 – 2 * t)
	tmp1 = _mm256_mul_pd(tmp1, tmp1); //tmp 1 = tmp1 * tmp1
	double4 res = _mm256_mul_pd(tmp2, tmp1); //res2 = t * t * (3 – 2 * t)
	return res;//_mm_or_ps(tmp3, tmp4);
}
#else
double4 __attribute__((overloadable)) smoothstep(double4 x, double4 y, double4 z)
{
	double4 res;
	res.lo = smoothstep(x.lo, y.lo, z.lo);
	res.hi = smoothstep(x.hi, y.hi, z.hi);
	return res;
}
#endif


//gentype sign (gentype x) Returns 1.0 if x > 0, -0.0 if x = -0.0, +0.0 if x =
//+0.0, or –1.0 if x < 0. Returns 0.0 if x is a NaN.
float  __attribute__((overloadable)) sign(float x)
{
	if(x != x)
	{
		return 0.0;
	}
	if(x >0)
	{
		return 1.0;
	}
	if(x <0 )
	{
		return -1.0;
	}

	return x;
}
float4  __attribute__((overloadable)) sign(float4 x)
{
//if X=nan -> x=0	
//	__m128 nan = _mm_set1_ps(0x7FFFFFFF); // or numeric_limits<float>::quiet_NaN();
	 float4 f4res = _mm_cmpneq_ps(x, *((float4 *)f4const_nanStorage));//if x=nan i4res=0
	 x = _mm_and_ps(f4res, x);//if i4res=0 (x=nan) x=0

//if x>0 return res=1
	float4 res = _mm_cmpgt_ps(x, _mm_setzero_ps());
	res =  _mm_and_ps(res, *((float4 *)f4const_oneStorage)); //res = 1 if x>0

//if x<-0 return res1=-1
	float4 res1 = _mm_cmplt_ps(x, *((float4 *)f4const_minusZeroStorage)); ////res1 = -1 if x<-0
	res1 =  _mm_and_ps(res1, *((float4 *)f4const_minusOneStorage));
	res = _mm_or_ps(res, res1);
//if x=0 res=0 (if x=0 res0=res1=0) - no need to do anything
	
//Convert integer to float
	return res;
}

#if defined (__AVX__)
float8  __attribute__((overloadable)) sign(float8 x)
{
	//if X=nan -> x=0	
	//	__m128 nan = _mm_set1_ps(0x7FFFFFFF); // or numeric_limits<float>::quiet_NaN();
	 float8 f8res = _mm256_cmp_ps(x, *((float8 *)f8const_nanStorage), _CMP_NEQ_OS);//if x=nan i4res=0
	 x = _mm256_and_ps(f8res, x);//if i4res=0 (x=nan) x=0

	//if x>0 return res=1
	float8 res = _mm256_cmp_ps(x, _mm256_setzero_ps(), _CMP_GT_OS);
	res =  _mm256_and_ps(res, *((float8 *)f8const_oneStorage)); //res = 1 if x>0

	//if x<-0 return res1=-1
	float8 res1 = _mm256_cmp_ps(x, *((float8 *)f8const_minusZeroStorage), _CMP_LT_OS); ////res1 = -1 if x<-0
	res1 =  _mm256_and_ps(res1, *((float8 *)f8const_minusOneStorage));
	res = _mm256_or_ps(res, res1);
	//if x=0 res=0 (if x=0 res0=res1=0) - no need to do anything
	
	//Convert integer to float
	return res;
}

#else
float8  __attribute__((overloadable)) sign(float8 x)
{
	float8 res;
	res.lo = sign(x.lo);
	res.hi = sign(x.hi);
	return res;
}
#endif


double  __attribute__((overloadable)) sign(double x)
{
	if(x != x)
	{
		return 0.0;
	}
	if(x >0)
	{
		return 1.0;
	}
	if(x <0 )
	{
		return -1.0;
	}

	return x;
}
double2  __attribute__((overloadable)) sign(double2 x)
{
	//if X=nan -> x=0	
	 double2 d2res = _mm_cmpneq_pd(x, *((double2 *)d2const_nanStorage));//if x=nan d2res=0
	 x = _mm_and_pd(d2res, x);//if d2res=0 (x=nan) x=0

//if x>0 return res=1
	double2 res = _mm_cmpgt_pd(x, _mm_setzero_pd());
	res =  _mm_and_pd(res, *((double2 *)d2const_oneStorage)); //res = 1 if x>0

//if x<-0 return res1=-1
	double2 res1 = _mm_cmplt_pd(x, *((double2 *)d2const_minusZeroStorage)); ////res1 = -1 if x<-0
	res1 =  _mm_and_pd(res1, *((double2 *)d2const_minusOneStorage));
	res = _mm_or_pd(res, res1);
//if x=0 res=0 (if x=0 res0=res1=0) - no need to do anything
	
//Convert integer to float
	return res;
}

#if defined (__AVX__)
double4  __attribute__((overloadable)) sign(double4 x)
{
	//if X=nan -> x=0	
	double4 d4res = _mm256_cmp_pd(x, *((double4 *)d4const_nanStorage), _CMP_NEQ_OS);//if x=nan d4res=0
	x = _mm256_and_pd(d4res, x);//if d2res=0 (x=nan) x=0

//if x>0 return res=1
	double4 res = _mm256_cmp_pd(x, _mm256_setzero_ps(), _CMP_GT_OS);
	res =  _mm256_and_pd(res, *((double4 *)d4const_oneStorage)); //res = 1 if x>0

//if x<-0 return res1=-1
	double4 res1 = _mm256_cmp_pd(x, *((double4 *)d4const_minusZeroStorage), _CMP_LT_OS); ////res1 = -1 if x<-0
	res1 =  _mm256_and_pd(res1, *((double4 *)d4const_minusOneStorage));
	res = _mm256_or_pd(res, res1);
//if x=0 res=0 (if x=0 res0=res1=0) - no need to do anything
	
//Convert integer to float
	return res;
}
#else
double4  __attribute__((overloadable)) sign(double4 x)
{
	double4 res;
	res.lo = sign(x.lo);
	res.hi = sign(x.hi);
	return res;
}
#endif

						
#ifdef __cplusplus
}
#endif
