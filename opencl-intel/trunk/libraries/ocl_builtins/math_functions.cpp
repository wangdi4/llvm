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
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////
//  svml_naive_functions.cpp
///////////////////////////////////////////////////////////

// Compiled with Clang as LLVM module
#define ALIGN16 __attribute__((aligned(16)))
#define ALIGN32 __attribute__((aligned(32)))
#include <intrin.h>
#if defined(__AVX__)
#include <avxintrin.h>
#endif

#include "cl_math_declaration.h"
//#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

const int   ALIGN16 mth_signMask[4]		    = {0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF};
const float ALIGN16 mth_oneStorage[4]	    = {1.0f, 1.0f, 1.0f, 1.0f};
const float ALIGN16 mth_halfStorage[4]	    = {0.5f, 0.5f, 0.5f, 0.5f};
const float ALIGN16 mth_StorageF127[4]	    = {127.0, 127.0, 127.0, 127.0};
const float ALIGN16 mth_StorageF64[4]	    = {64.0, 64.0, 64.0, 64.0};
const int   ALIGN16 mth_StorageI127[4]		= {127, 127, 127, 127};
const int   ALIGN16 mth_StorageI64[4]		= {64, 64, 64, 64};
const int   ALIGN16 mth_nanStorage[4]		= {0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF};
const int   ALIGN16 mth_expMask[4]			= {0x7f800000, 0x7f800000, 0x7f800000, 0x7f800000};
const int   ALIGN16 mth_minusInfStorage[4]	= {0xff800000, 0xff800000, 0xff800000, 0xff800000};
const int   ALIGN16 mth_p64Storage[4]		= {0x5f800000, 0x5f800000, 0x5f800000, 0x5f800000};
const int   ALIGN16 mth_intMinStorage[4]	= {INT_MIN, INT_MIN, INT_MIN, INT_MIN};
const int   ALIGN16 mth_intMaxStorage[4]	= {INT_MAX, INT_MAX, INT_MAX, INT_MAX};
const int   ALIGN16 mth_fractLimit[4]		= {0x3f7fffff, 0x3f7fffff, 0x3f7fffff, 0x3f7fffff};
const float ALIGN16 mth_pzero[4]			= { 0.0f, 0.0f, 0.0f, 0.0f };

const int   ALIGN32 mth_signMask8[8] = {    0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF,
                                            0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF};
const float ALIGN32 mth_oneStorage8[8]	    = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
const float ALIGN32 mth_halfStorage8[8]	    = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f };

const int   ALIGN32 mth_StorageI127_8[8]    = {127, 127, 127, 127, 127, 127, 127, 127};

const int   ALIGN32 mth_expMask8[8]     = {0x7f800000, 0x7f800000, 0x7f800000, 0x7f800000, 
                                           0x7f800000, 0x7f800000, 0x7f800000, 0x7f800000};
const int   ALIGN32 mth_StorageI64_8[8]     = {64, 64, 64, 64, 64, 64, 64, 64};
const int   ALIGN32 mth_p64Storage_8[8]     = {0x5f800000, 0x5f800000, 0x5f800000, 0x5f800000, 
                                               0x5f800000, 0x5f800000, 0x5f800000, 0x5f800000 };
const int   ALIGN32 mth_intMinStorage8[8]   = {INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN };
const int   ALIGN32 mth_intMaxStorage8[8]   = {INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX };

const int   ALIGN32 mth_nanStorage8[8]       = { 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 
                                                0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF};

const int   ALIGN32 mth_fractLimit8[8]      = { 0x3f7fffff, 0x3f7fffff, 0x3f7fffff, 0x3f7fffff, 
                                                0x3f7fffff, 0x3f7fffff, 0x3f7fffff, 0x3f7fffff};
const float ALIGN32 mth_pzero8[8]			= { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

#ifdef __SSE4_1__
	#define BLEND_PS(x, y, mask)\
		_mm_blendv_ps(x, y, mask);
	#define BLEND_PI(x, y, mask)\
		_mm_blendv_epi8(x, y, mask);
#else
	#define BLEND_PS(x, y, mask)\
		_mm_or_ps(_mm_andnot_ps(mask, x), _mm_and_ps(mask, y));
	#define BLEND_PI(x, y, mask)\
		_mm_or_si128(_mm_andnot_si128(mask, x), _mm_and_si128(mask, y));
#endif

#if defined(__AVX__)
float8 _isnan8(float8 x)
{ 
    return _mm256_cmp_ps(x, x, _CMP_UNORD_Q);
}
#endif // defined(__AVX__)

float  __attribute__((overloadable)) minmag(float x, float y)
{	
	float xabs = fabs(x);
	float yabs = fabs(y);
	if (xabs < yabs)
		return x;
	else if (yabs < xabs)
		return y;
	else
		return fmin(x,y);
}

float4  __attribute__((overloadable)) minmag(float4 x, float4 y)
{   
    float4 res;

    float4 xabs = fabs(x);
    float4 yabs = fabs(y);
    float4 xymin = fmin(x,y);
    
    float4 xGTy = _mm_cmpgt_ps(xabs, yabs);
    float4 yGTx = _mm_cmpgt_ps(yabs, xabs);

    res = BLEND_PS(xymin, y, xGTy);
    res = BLEND_PS(res,   x, yGTx);
    return res;
}

#if defined(__AVX__)
float8  __attribute__((overloadable)) minmag(float8 x, float8 y)
{   
    float8 res;

    float8 xabs = fabs(x);
    float8 yabs = fabs(y);
    float8 xymin = fmin(x,y);
    
    float8 xGTy = _mm256_cmp_ps(xabs, yabs, _CMP_GT_OS);
    float8 yGTx = _mm256_cmp_ps(yabs, xabs, _CMP_GT_OS);

    res = _mm256_blendv_ps(xymin, y, xGTy);
    res = _mm256_blendv_ps(res,   x, yGTx);
    return res;
}
#endif // defined(__AVX__)

float  __attribute__((overloadable)) maxmag(float x, float y)
{	
    float xabs = fabs(x);
	float yabs = fabs(y);
	if (xabs > yabs)
		return x;
	else if (yabs > xabs)
		return y;
	else
		return fmax(x,y);
}

float4  __attribute__((overloadable)) maxmag(float4 x, float4 y)
{	
    float4 res;
    
    float4 xabs = fabs(x);
    float4 yabs = fabs(y);
    float4 xymax = fmax(x,y);

    float4 xLTy = _mm_cmplt_ps(xabs, yabs);
    float4 yLTx = _mm_cmplt_ps(yabs, xabs);

    res = BLEND_PS(xymax, y, xLTy);
    res = BLEND_PS(res,   x, yLTx);
    return res;
}

#if defined(__AVX__)
float8  __attribute__((overloadable)) maxmag(float8 x, float8 y)
{
    float8 res;
    
    float8 xabs = fabs(x);
    float8 yabs = fabs(y);
    float8 xymax = fmax(x,y);

    float8 xLTy = _mm256_cmp_ps(xabs, yabs, _CMP_LT_OS);
    float8 yLTx = _mm256_cmp_ps(yabs, xabs, _CMP_LT_OS);

    res = _mm256_blendv_ps(xymax, y, xLTy);
    res = _mm256_blendv_ps(res,   x, yLTx);
    return res;
}

#endif // defined(__AVX__)

// double

double  __attribute__((overloadable)) minmag(double x, double y)
{	

	double xabs = fabs(x);
	double yabs = fabs(y);
	if (xabs < yabs)
		return x;
	else if (yabs < xabs)
		return y;
	else
		return fmin(x,y);
}

double  __attribute__((overloadable)) maxmag(double x, double y)
{	

	double xabs = fabs(x);
	double yabs = fabs(y);
	if (xabs > yabs)
		return x;
	else if (yabs > xabs)
		return y;
	else
		return fmax(x,y);
}

float  __attribute__((overloadable)) fabs(float x)
{
	float res;
	__m128 tmp = _mm_load_ss(&x);
	tmp = _mm_and_ps(tmp, *(__m128*)mth_signMask);
	_mm_store_ss(&res, tmp);
    return res;
}

float4  __attribute__((overloadable)) fabs(float4 p)
{
	p = _mm_and_ps(p, *(__m128*)mth_signMask);
	return p;
}

#if defined(__AVX__)
float8  __attribute__((overloadable)) fabs(float8 p)
{
    p = _mm256_and_ps(p, _mm256_broadcast_ss((float const *) mth_signMask));
    return p;
}
#endif // __AVX__

#ifdef __SSE4_1__

float  __attribute__((overloadable)) ceil(float x)
{
	float res;
	__m128 tmp = _mm_load_ss(&x);
	tmp = _mm_ceil_ss(tmp, tmp);
	_mm_store_ss(&res, tmp);
	return res;
}

float4  __attribute__((overloadable)) ceil(float4 x)
{
	return _mm_ceil_ps(x);
}

#if defined(__AVX__)
float8  __attribute__((overloadable)) ceil(float8 x)
{
    return _mm256_ceil_ps(x);
}
#endif // __AVX__

float  __attribute__((overloadable)) floor(float x)
{
	float res;
	__m128 tmp = _mm_load_ss(&x);
	tmp = _mm_floor_ss(tmp, tmp);
	_mm_store_ss(&res, tmp);
	return res;
}

float4  __attribute__((overloadable)) floor(float4 x)
{
	return _mm_floor_ps(x);
}

#if defined(__AVX__)
float8  __attribute__((overloadable)) floor(float8 x)
{
    return _mm256_floor_ps(x);
}
#endif // __AVX__

float  __attribute__((overloadable)) trunc(float x)
{
	float res;
	__m128 tmp = _mm_load_ss(&x);
	tmp = _mm_round_ss(tmp, tmp, 3);
	_mm_store_ss(&res, tmp);
	return res;
}

float4  __attribute__((overloadable)) trunc(float4 x)
{
	return _mm_round_ps(x, 3);
}

#if defined(__AVX__)
float8  __attribute__((overloadable)) trunc(float8 x)
{
    return _mm256_round_ps(x, 3);
}

#endif
float  __attribute__((overloadable)) round(float x)
{
	float res;
	__m128 tmp = _mm_set_ss(x);

	__m128 xAbs = _mm_and_ps(*(__m128*)mth_signMask, tmp);
	__m128 xSign = _mm_andnot_ps(*(__m128*)mth_signMask, tmp);
	__m128 xAbsRounded = _mm_round_ss(xAbs, xAbs, 0);
	__m128 xDiff = _mm_sub_ss(xAbs, xAbsRounded);
	xDiff = _mm_cmpeq_ss(xDiff, *(__m128*)mth_halfStorage);
	xDiff = _mm_and_ps(xDiff, *(__m128*)mth_oneStorage);
	tmp = _mm_add_ss(xAbsRounded, xDiff);
	tmp = _mm_or_ps(tmp, xSign);

	_mm_store_ss(&res, tmp);
	return res;
}

float4  __attribute__((overloadable)) round(float4 x)
{
	__m128 xAbs = _mm_and_ps(*(__m128*)mth_signMask, x);
	__m128 xSign = _mm_andnot_ps(*(__m128*)mth_signMask, x);
	__m128 xAbsRounded = _mm_round_ps(xAbs, 0);
	__m128 xDiff = _mm_sub_ps(xAbs, xAbsRounded);
	xDiff = _mm_cmpeq_ps(xDiff, *(__m128*)mth_halfStorage);
	xDiff = _mm_and_ps(xDiff, *(__m128*)mth_oneStorage);
	x = _mm_add_ps(xAbsRounded, xDiff);
	x = _mm_or_ps(x, xSign);
	return x;
}

#if defined(__AVX__)
float8  __attribute__((overloadable)) round(float8 x)
{
    __m256 xAbs = _mm256_and_ps(*(__m256*)mth_signMask8, x);
    __m256 xSign = _mm256_andnot_ps(*(__m256*)mth_signMask8, x);
    __m256 xAbsRounded = _mm256_round_ps(xAbs, 0);
    __m256 xDiff = _mm256_sub_ps(xAbs, xAbsRounded);
    xDiff = _mm256_cmp_ps(xDiff, *(__m256*)mth_halfStorage8, _CMP_EQ_OQ);
    xDiff = _mm256_and_ps(xDiff, *(__m256*)mth_oneStorage8);
    x = _mm256_add_ps(xAbsRounded, xDiff);
    x = _mm256_or_ps(x, xSign);
    return x;
}
#endif // defined(__AVX__)

float  __attribute__((overloadable)) rint(float x)
{
	float res;
	__m128 tmp = _mm_load_ss(&x);
	tmp = _mm_round_ss(tmp, tmp, 0);
	_mm_store_ss(&res, tmp);
	return res;
}

float4  __attribute__((overloadable)) rint(float4 x)
{
	return _mm_round_ps(x, 0);
}

#if defined(__AVX__)
float8  __attribute__((overloadable)) rint(float8 x)
{
    return _mm256_round_ps(x, 0);
}
#endif // defined(__AVX__)

#endif

float  __attribute__((overloadable)) fmax(float x, float y)
{
	float res;
	__m128 tmpX = _mm_load_ss(&x);
	__m128 tmpY = _mm_load_ss(&y);

	//if( isnan(y) )
	//    return x;	
	float4 nan = _mm_cmpneq_ss(tmpY, tmpY);
	float4 res1 = _mm_max_ss(tmpX, tmpY);

	res1 = BLEND_PS(res1, tmpX, nan);

	_mm_store_ss(&res, res1);
	return res;
}

float4  __attribute__((overloadable)) fmax(float4 x, float4 y)
{
	//if( isnan(y) )
	//    return x;	
	float4 nan = _mm_cmpneq_ps(y, y);
	float4 res = _mm_max_ps(x,y);

	res = BLEND_PS(res, x, nan);
	
	return res;
}

#if defined(__AVX__)
float8  __attribute__((overloadable)) fmax(float8 x, float8 y)
{
    float8 nan = _isnan8(y);
    float8 res = _mm256_max_ps(x,y);

    res = _mm256_blendv_ps(res, x, nan);

    return res;
}
#endif // __AVX__

float  __attribute__((overloadable)) fmin(float x, float y)
{
	float res_t;
	__m128 tmpX = _mm_load_ss(&x);
	__m128 tmpY = _mm_load_ss(&y);

	//if( isnan(y) )
	//    return x;	
	float4 nan = _mm_cmpneq_ss(tmpY, tmpY);
	float4 res = _mm_min_ss(tmpX,tmpY);
	
	res = BLEND_PS(res, tmpX, nan);

	_mm_store_ss(&res_t, res);
	return res_t;
}

float4  __attribute__((overloadable)) fmin(float4 x, float4 y)
{
	//if( isnan(y) )
	//    return x;	
	float4 nan = _mm_cmpneq_ps(y, y);
	float4 res = _mm_min_ps(x,y);

	res = BLEND_PS(res, x, nan);

	return res;
}

#if defined(__AVX__)
float8  __attribute__((overloadable)) fmin(float8 x, float8 y)
{
    float8 nan = _isnan8(y);
    float8 res = _mm256_min_ps(x,y);

    res = _mm256_blendv_ps(res, x, nan);

    return res;
}
#endif // __AVX__

float  __attribute__((overloadable)) rsqrt(float x)
{
    float4 tempX;
    float4 tempRes;
    float4 res;

    tempX.s0 = x;
    tempRes = _mm_sqrt_ss(tempX); //SQRTPS
    res = 1.0f / tempRes;
    return res.s0;
}

float4  __attribute__((overloadable)) rsqrt(float4 x)
{
    float4 tempRes;
    float4 res;

    tempRes = _mm_sqrt_ps(x); //SQRTPS
    res = 1.0f / tempRes;
    return res;
}
#if defined(__AVX__)
float8  __attribute__((overloadable)) rsqrt(float8 x)
{
    float8 tempRes;
    float8 res;

    tempRes = _mm256_sqrt_ps(x); //SQRTPS
    res = 1.0f / tempRes;
    return res;
}
#endif // __AVX___
float  __attribute__((overloadable)) copysign(float x, float y)
{
    int ux = as_int(x);
    int uy = as_int(y);
    ux &= mth_signMask[0];
    ux |= uy & ~mth_signMask[0];
    return as_float(ux);
}
float4  __attribute__((overloadable)) copysign(float4 x, float4 y)
{
    y = _mm_andnot_ps(*(__m128*)mth_signMask, y);	//get the sign of y
	x = _mm_and_ps(*(__m128*)mth_signMask, x);		//get x without the sign
	x = _mm_or_ps(x, y);					//merge them
	return x;
}
#if defined(__AVX__)
float8  __attribute__((overloadable)) copysign(float8 x, float8 y)
{
    y = _mm256_andnot_ps(*(__m256*)mth_signMask8, y);   //get the sign of y
    x = _mm256_and_ps(*(__m256*)mth_signMask8, x);      //get x without the sign
    x = _mm256_or_ps(x, y);                             //merge them
    return x;
}
#endif // defined(__AVX__)

float  __attribute__((overloadable)) fdim(float x, float y)
{
	float res;
	__m128 tmpX = _mm_load_ss(&x);
	__m128 tmpY = _mm_load_ss(&y);

	float4 xMinusY = _mm_sub_ss(tmpX, tmpY);
	float4 xNaN = _mm_cmpneq_ss(tmpX, tmpX);
	float4 yNaN = _mm_cmpneq_ss(tmpY, tmpY);
	float4 NaN = _mm_or_ps(xNaN, yNaN);
#ifdef __SSE4_1__
	float4 res1 = _mm_blendv_ps(xMinusY, _mm_setzero_ps(), xMinusY);
	res1 = _mm_blendv_ps(res1, *(__m128*)mth_nanStorage, NaN);
#else
	float4 xGTy = _mm_cmpgt_ss(tmpX, tmpY);
	float4 res1 = _mm_setzero_ps();
	xMinusY = _mm_and_ps(xMinusY, xGTy);
	res1 = _mm_or_ps(res1, xMinusY);
	res1 = _mm_or_ps(_mm_andnot_ps(NaN, res1), _mm_and_ps(NaN, *(__m128*)mth_nanStorage));
#endif

	_mm_store_ss(&res, res1);
	return res;
}

float4  __attribute__((overloadable)) fdim(float4 x, float4 y)
{
	float4 xMinusY = _mm_sub_ps(x, y);
	float4 xNaN = _mm_cmpneq_ps(x, x);
	float4 yNaN = _mm_cmpneq_ps(y, y);
	float4 NaN = _mm_or_ps(xNaN, yNaN);

#ifdef __SSE4_1__
	float4 res = _mm_blendv_ps(xMinusY, _mm_setzero_ps(), xMinusY);
	res = _mm_blendv_ps(res, *(__m128*)mth_nanStorage, NaN);
#else
	float4 xGTy = _mm_cmpgt_ps(x, y);
	float4 res = _mm_setzero_ps();
	xMinusY = _mm_and_ps(xMinusY, xGTy);
	res = _mm_or_ps(res, xMinusY);
	res = _mm_or_ps(_mm_andnot_ps(NaN, res), _mm_and_ps(NaN, *(__m128*)mth_nanStorage));
#endif

	return res;
}

#if defined(__AVX__)
float8  __attribute__((overloadable)) fdim(float8 x, float8 y)
{
    float8 xMinusY = x - y;
    float8 xNaN = _isnan8(x);
    float8 yNaN = _isnan8(y);
    float8 NaN = _mm256_or_ps(xNaN, yNaN);

    float8 res = _mm256_blendv_ps(xMinusY, _mm256_setzero_ps(), xMinusY);
    res = _mm256_blendv_ps(res, *(__m256*)mth_nanStorage8, NaN);

    return res;
}
#endif
double  __attribute__((overloadable)) mad(double x, double y, double z)
{
	return (x*y+z);
}

double2  __attribute__((overloadable)) mad(double2 x, double2 y, double2 z)
{
	return (x*y+z);
}

double3  __attribute__((overloadable)) mad(double3 x, double3 y, double3 z)
{
	return (x*y+z);
}

double4  __attribute__((overloadable)) mad(double4 x, double4 y, double4 z)
{
	return (x*y+z);
}

double8  __attribute__((overloadable)) mad(double8 x, double8 y, double8 z)
{
	return (x*y+z);
}

double16  __attribute__((overloadable)) mad(double16 x, double16 y, double16 z)
{
	return (x*y+z);
}


float  __attribute__((overloadable)) mad(float x, float y, float z)
{
	return (x*y+z);
}

float2  __attribute__((overloadable)) mad(float2 x, float2 y, float2 z)
{
	return (x*y+z);
}
float3  __attribute__((overloadable)) mad(float3 x, float3 y, float3 z)
{
	return (x*y+z);
}
float4  __attribute__((overloadable)) mad(float4 x, float4 y, float4 z)
{
	return (x*y+z);
}
float8  __attribute__((overloadable)) mad(float8 x, float8 y, float8 z)
{
	return (x*y+z);
}
float16  __attribute__((overloadable)) mad(float16 x, float16 y, float16 z)
{
	return (x*y+z);
}

float  __attribute__((overloadable)) nan(_1u32 x)
{
	float res;
	__m128 tmpX = _mm_load_ss((float*)&x);
	tmpX = _mm_or_ps(tmpX, *(__m128*)mth_nanStorage);
	_mm_store_ss(&res, tmpX);
	return res;
}

float4  __attribute__((overloadable)) nan(_4u32 x)
{
	return _mm_or_ps(_mm_castsi128_ps((__m128i)x), *(__m128*)mth_nanStorage);
}
#if defined(__AVX__)
float8  __attribute__((overloadable)) nan(_8u32 x)
{
    return _mm256_or_ps(x, *(__m256*)mth_nanStorage8);
}
#endif // defined(__AVX__)
/*
float  __attribute__((overloadable)) logb(float x)
{
	float res;
	__m128 xVec = _mm_load_ss((float*)&x);
	xVec = _mm_and_ps(xVec, *(__m128*)mth_signMask);
	__m128 zero = _mm_cmpeq_ss(xVec, _mm_setzero_ps());		//case zero

	__m128 factor = *(__m128*)mth_StorageF127;

	__m128 res1 = _mm_and_ps(xVec, *(__m128*)mth_expMask);		//normal case

	__m128 res2 = _mm_cmpeq_ss(res1, *(__m128*)mth_expMask);	//case NaN or inf
	__m128 NaNorINF = res2;
	res2 = _mm_and_ps(res2, xVec);

	__m128 res3 = _mm_cmpeq_ss(res1, _mm_setzero_ps());		//case denom

	factor = _mm_add_ss(_mm_and_ps(res3, *(__m128*)mth_StorageF64), factor);

	xVec = _mm_and_ps(res3, xVec);
	res3 = _mm_and_ps(res3, *(__m128*)mth_p64Storage);
	res3 = _mm_mul_ss(res3, xVec);
	res3 = _mm_and_ps(res3, *(__m128*)mth_expMask);

	__m128 res4 = _mm_and_ps(zero, *(__m128*)mth_minusInfStorage);		//case zero

	res1 = _mm_cvtepi32_ps(_mm_srli_epi32(_mm_castps_si128(res1), 23));
	res3 = _mm_cvtepi32_ps(_mm_srli_epi32(_mm_castps_si128(res3), 23));

	res2 = BLEND_PS(res2, res2, NaNorINF);
	res2 = BLEND_PS(res2, res4, zero);

	//now decide whether to take the regular or iregular case
	__m128 normal = _mm_cmpeq_ss(res2, _mm_setzero_ps());
	res1 = BLEND_PS(res2, res1, normal);

	//if res is NaN inf or -inf the factor won't matter
	res1 = _mm_sub_ss(res1, factor);

	_mm_store_ss(&res, res1);
	return res;
}


float4  __attribute__((overloadable)) logb(float4 x)
{
	__m128 factor = *(__m128*)mth_StorageF127;

	x = _mm_and_ps(x, *(__m128*)mth_signMask);
	__m128 zero = _mm_cmpeq_ps(x, _mm_setzero_ps());		//case zero

	__m128 res1 = _mm_and_ps(x, *(__m128*)mth_expMask);		//normal case

	__m128 res2 = _mm_cmpeq_ps(res1, *(__m128*)mth_expMask);	//case Nan or inf
	__m128 NaNorINF = res2;
	res2 = _mm_and_ps(res2, x);

	__m128 res3 = _mm_cmpeq_ps(res1, _mm_setzero_ps());		//case denom

	factor = _mm_add_ps(_mm_and_ps(res3, *(__m128*)mth_StorageF64), factor);

	x    = _mm_and_ps(res3, x);
	res3 = _mm_and_ps(res3, *(__m128*)mth_p64Storage);
	res3 = _mm_mul_ps(res3, x);
	res3 = _mm_and_ps(res3, *(__m128*)mth_expMask);

	__m128 res4 = _mm_and_ps(zero, *(__m128*)mth_minusInfStorage);

	res1 = _mm_cvtepi32_ps(_mm_srli_epi32(_mm_castps_si128(res1), 23));
	res3 = _mm_cvtepi32_ps(_mm_srli_epi32(_mm_castps_si128(res3), 23));

	//res2-4 are mutually exclusive sor we can "or" between them
	res2 = BLEND_PS(res3, res2, NaNorINF);
	res2 = BLEND_PS(res2, res4, zero);

	//now decide whether to take the regular or iregular case
	__m128 normal = _mm_cmpeq_ps(res2, _mm_setzero_ps());
	res1 = BLEND_PS(res2, res1, normal);

	//if res is NaN inf or -inf the factor won't matter
	res1 = _mm_sub_ps(res1, factor);

	return res1;
}
*/
_1i32  __attribute__((overloadable)) ilogb(float x)
{
	int ALIGN16 res[4];
	//int t = 0x00800000;
	//__m128 xVec = _mm_load_ss((float*)&t);
	__m128 xVec = _mm_load_ss((float*)&x);
	xVec = _mm_and_ps(xVec, *(__m128*)mth_signMask);
	__m128i zero = _mm_castps_si128(_mm_cmpeq_ss(xVec, _mm_setzero_ps()));		//case zero

	__m128i factor = *(__m128i*)mth_StorageI127;

	__m128i res1 = _mm_and_si128(_mm_castps_si128(xVec), *(__m128i*)mth_expMask);		//normal case

	__m128i NaNorINF = _mm_cmpeq_epi32(res1, *(__m128i*)mth_expMask);	//case NaN or inf

	__m128 denom = _mm_cmpeq_ss(_mm_castsi128_ps(res1), _mm_setzero_ps());		//case denom

	factor = _mm_add_epi32(_mm_and_si128(_mm_castps_si128(denom), *(__m128i*)mth_StorageI64), factor);

	xVec = _mm_and_ps(denom, xVec);
	denom = _mm_and_ps(denom, *(__m128*)mth_p64Storage);
	denom = _mm_mul_ss(denom, xVec);
	denom = _mm_and_ps(denom, *(__m128*)mth_expMask);

	res1 = _mm_srli_epi32(res1, 23);
	__m128i res3 = _mm_srli_epi32(_mm_castps_si128(denom), 23);
	__m128i notDenom = _mm_cmpeq_epi32(res3, _mm_setzero_si128());

	res1 = _mm_sub_epi32(res1, factor);
	res3 = _mm_sub_epi32(res3, factor);

	//res2-4 are mutually exclusive sor we can "or" between them
	res1 = BLEND_PI(res3, res1, notDenom);
	res1 = BLEND_PI(res1, *(__m128i*)mth_intMaxStorage, NaNorINF);
	res1 = BLEND_PI(res1, *(__m128i*)mth_intMinStorage, zero);

	_mm_store_si128 ((__m128i*)res, res1);
	return res[0];
}

_4i32  __attribute__((overloadable)) ilogb(float4 x)
{
	x = _mm_and_ps(x, *(__m128*)mth_signMask);
	__m128i zero = _mm_castps_si128(_mm_cmpeq_ps(x, _mm_setzero_ps()));		//case zero

	__m128i factor = *(__m128i*)mth_StorageI127;

	__m128i res1 = _mm_and_si128(_mm_castps_si128(x), *(__m128i*)mth_expMask);		//normal case

	__m128i NaNorINF = _mm_cmpeq_epi32(res1, *(__m128i*)mth_expMask);	//case NaN or inf

	__m128 denom = _mm_cmpeq_ps(_mm_castsi128_ps(res1), _mm_setzero_ps());		//case denom

	factor = _mm_add_epi32(_mm_and_si128(_mm_castps_si128(denom), *(__m128i*)mth_StorageI64), factor);

	x = _mm_and_ps(denom, x);
	denom = _mm_and_ps(denom, *(__m128*)mth_p64Storage);
	denom = _mm_mul_ps(denom, x);
	denom = _mm_and_ps(denom, *(__m128*)mth_expMask);

	res1 = _mm_srli_epi32(res1, 23);
	__m128i res3 = _mm_srli_epi32(_mm_castps_si128(denom), 23);
	__m128i notDenom = _mm_cmpeq_epi32(res3, _mm_setzero_si128());

	res1 = _mm_sub_epi32(res1, factor);
	res3 = _mm_sub_epi32(res3, factor);

	//res2-4 are mutually exclusive sor we can "or" between them
	res1 = BLEND_PI(res3, res1, notDenom);
	res1 = BLEND_PI(res1, *(__m128i*)mth_intMaxStorage, NaNorINF);
	res1 = BLEND_PI(res1, *(__m128i*)mth_intMinStorage, zero);

	//now decide whether to take the regular or iregular case
	//__m128i normal = _mm_cmpeq_epi32(res3, _mm_setzero_si128());
	//res1 = _mm_blendv_epi8(res3, res1, normal);

	return (_4i32)res1;
}

#if defined(__AVX__)
_8i32  __attribute__((overloadable)) ilogb(float8 x)
{
    x = _mm256_and_ps(x, *(__m256*)mth_signMask8);
    __m256 zero = _mm256_cmp_ps(x, _mm256_setzero_ps(), _CMP_EQ_OQ);     //case zero

    __m256 res1 = _mm256_and_ps(x, *(__m256*)mth_expMask8);//normal case
    __m256 NaNorINF = _mm256_cmp_ps(res1, *(__m256*)mth_expMask8, _CMP_EQ_OQ);	//case NaN or inf

    __m256 denom = _mm256_cmp_ps(res1, _mm256_setzero_ps(), _CMP_EQ_OQ);//case denom

    __m256 res_and = _mm256_and_ps(denom, *(__m256*)mth_StorageI64_8);
    // convert m256 to 2 m128 to perform integer addition since AVX256 does not have it
    __m128i res_and0 =  _mm256_extractf128_si256 (res_and, 0);
    __m128i res_and1 =  _mm256_extractf128_si256 (res_and, 1);
    __m128i factor0 = _mm_add_epi32(res_and0, *(__m128i*)mth_StorageI127);
    __m128i factor1 = _mm_add_epi32(res_and1, *(__m128i*)mth_StorageI127);

    x = _mm256_and_ps(denom, x);
    denom = _mm256_and_ps(denom, *(__m256*)mth_p64Storage_8);
    denom = _mm256_mul_ps(denom, x);
    denom = _mm256_and_ps(denom, *(__m256*)mth_expMask8);

    __m128i res1_0 = _mm256_extractf128_si256(res1, 0);
    __m128i res1_1 = _mm256_extractf128_si256(res1, 1);

    res1_0 = _mm_srli_epi32(res1_0, 23);
    res1_1 = _mm_srli_epi32(res1_1, 23);

    __m128i denom_0 = _mm256_extractf128_si256(denom, 0);
    __m128i denom_1 = _mm256_extractf128_si256(denom, 1);
    
    __m128i res3_0 = _mm_srli_epi32(_mm_castps_si128(denom_0), 23);
    __m128i res3_1 = _mm_srli_epi32(_mm_castps_si128(denom_1), 23);

    __m256i res3;
    res3 = _mm256_insertf128_ps(res3, res3_0, 0);
    res3 = _mm256_insertf128_ps(res3, res3_1, 1);

    __m256i notDenom = _mm256_cmp_ps( res3, _mm256_setzero_ps(), _CMP_EQ_OQ );

    res1_0 = _mm_sub_epi32(res1_0, factor0);
    res1_1 = _mm_sub_epi32(res1_1, factor1);
    
    res3_0 = _mm_sub_epi32(res3_0, factor0);
    res3_1 = _mm_sub_epi32(res3_1, factor1);

    res1 = _mm256_insertf128_ps(res1, res1_0, 0);
    res1 = _mm256_insertf128_ps(res1, res1_1, 1);

    res3 = _mm256_insertf128_ps(res3, res3_0, 0);
    res3 = _mm256_insertf128_ps(res3, res3_1, 1);

    //res2-4 are mutually exclusive sor we can "or" between them
    res1 = _mm256_blendv_ps(res3, res1, notDenom);
    res1 = _mm256_blendv_ps(res1, *(__m256*)mth_intMaxStorage8, NaNorINF);
    res1 = _mm256_blendv_ps(res1, *(__m256*)mth_intMinStorage8, zero);

    return (_8i32)res1;
}

#endif 

#ifdef __SSE4_1__
float  __attribute__((overloadable)) fract(float x, float *iptr)
{
    float4 i4;
    float4 f4x = _mm_set_ss(x);
    // call fract(float4) 
    float4 f4y = fract(f4x, &i4);
     _mm_store_ss(iptr, i4);
    return _mm_cvtss_f32(f4y);
}
/*
    According to spec OpenCL1.1 rev40 6.11.2
    gentype fract (gentype x,
    __global gentype *iptr)43
    gentype fract (gentype x,
    __local gentype *iptr)
    gentype fract (gentype x,
    __private gentype *iptr)

    Returns fmin( x – floor (x), 0x1.fffffep-1f ).
    floor(x) is returned in iptr.

    Special cases:
    fract ( x, iptr) shall not return a value greater than or equal to 1.0, and shall not return a
    value less than 0.
    fract ( +0, iptr ) returns +0 and +0 in iptr.
    fract ( -0, iptr ) returns -0 and -0 in iptr.
    fract ( +inf, iptr ) returns +0 and +inf in iptr.
    fract ( -inf, iptr ) returns -0 and -inf in iptr.
    fract ( NaN, iptr ) returns the NaN and NaN in iptr.

*/
float4 __attribute__((overloadable)) fract(float4 f4x, float4 *iptr)
{
    // check for NaN
    int4 isNaN = _mm_cmpneq_ps(f4x, f4x);
    // get absolute value
    float4 xAbs = _mm_and_ps(*(__m128*)mth_signMask, f4x);
    // check for +- Infinity
    int4 isInf = _mm_cmpeq_epi32(xAbs, *((__m128i*)mth_expMask));
    // check for zero
    int4 isZero = _mm_cmpeq_ps(xAbs, *((__m128*)mth_pzero));
    // obtain mask with elements zero or infinity. we need to keep sign
    int4 mask =  _mm_or_si128(isNaN,  _mm_or_si128( isInf, isZero));
    // floor(x)
    *iptr = _mm_floor_ps(f4x);
    // x - floor(x)
    f4x = _mm_sub_ps(f4x, *iptr);
    // fmin( x – floor (x), 0x1.fffffep-1f )
    float4 res = _mm_min_ps(f4x, *(__m128*)mth_fractLimit);

    // approach with if() instead of unconditional blend
    // works better on SSE then on AVX. 
    // if any of elements of mask is 0 or infinity
    if( _mm_movemask_ps(mask))
    {
        // obtain sign
        int4 xSign = _mm_andnot_ps(*(__m128*)mth_signMask, f4x);
        // make zero with sign
        float4 xSignedZero = _mm_or_ps(*(__m128*)mth_pzero, xSign);

        // NaN. Put NaN from f4x to res
        res = _mm_blendv_ps(res, f4x, isNaN);
        // +/- inf and zero. Put zero with proper sign
        res = _mm_blendv_ps(res, xSignedZero, _mm_or_si128( isInf, isZero));
    }

    return res;
}
#endif // __SSE4_1__
#if defined(__AVX__)
float8 __attribute__((overloadable)) fract(float8 x, float8 *iptr)
{
    // check for NaN
    float8 isNaN = _isnan8(x);
    // get absolute value
    float8 xAbs = _mm256_and_ps(*(__m256*)mth_signMask8, x);
    // check for +- Infinity
    int8 isInf =  _mm256_cmp_ps(xAbs, *((float8*)mth_expMask8), _CMP_EQ_OQ);
    // check for zero
    int8 isZero = _mm256_cmp_ps(xAbs,  *((__m256*)mth_pzero8), _CMP_EQ_OQ);
    // obtain sign
    int8 xSign = _mm256_andnot_ps(*(__m256*)mth_signMask8, x);
    // make zero with sign
    float8 xSignedZero = _mm256_or_ps(*(__m256*)mth_pzero8, xSign);
    // floor(x)
    *iptr = _mm256_floor_ps(x);
    // x - floor(x)
    x = _mm256_sub_ps(x, *iptr);
    // fmin( x – floor (x), 0x1.fffffep-1f )
    float8 res = _mm256_min_ps(x, *(__m256*)mth_fractLimit8);

    // approach with unconditional blend instead of if() 
    // works better on AVX then on SSE
    // NaN. Put NaN from f4x to res
    res = _mm256_blendv_ps(res, x, isNaN);
    // +/- inf and zero. Put zero with proper sign
    res = _mm256_blendv_ps(res, xSignedZero, _mm256_or_ps(isInf, isZero));

    return res;
}
#endif // defined(__AVX__)

/* ****************************
** RELAXED EXTENSION
** MATH FUNCTION DECLARED IN
** OPENCL SPEC rev 34 
** PAGE 204 TABLE 6.8
** ***************************/

//native_fdim
float __attribute__((overloadable)) native_fdim(float x, float y)
{
	float res;
	__m128 tmpX = _mm_load_ss(&x);
	__m128 tmpY = _mm_load_ss(&y);

	float4 xMinusY = _mm_sub_ss(tmpX, tmpY);
#ifdef __SSE4_1__
	float4 res1 = _mm_blendv_ps(xMinusY, _mm_setzero_ps(), xMinusY);
#else
	float4 xGTy = _mm_cmpgt_ss(tmpX, tmpY);
	float4 res1 = _mm_setzero_ps();
	xMinusY = _mm_and_ps(xMinusY, xGTy);
	res1 = _mm_or_ps(res1, xMinusY);
#endif
	_mm_store_ss(&res, res1);
	return res;
}

float4 __attribute__((overloadable)) native_fdim(float4 x, float4 y)
{
	float4 xMinusY = _mm_sub_ps(x, y);
#ifdef __SSE4_1__
	float4 res = _mm_blendv_ps(xMinusY, _mm_setzero_ps(), xMinusY);
#else
	float4 xGTy = _mm_cmpgt_ps(x, y);
	float4 res = _mm_setzero_ps();
	xMinusY = _mm_and_ps(xMinusY, xGTy);
	res = _mm_or_ps(res, xMinusY);
#endif
	return res;
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) native_fdim(float8 x, float8 y)
{
    float8 xMinusY = _mm256_sub_ps(x, y);
    return _mm256_blendv_ps(xMinusY, _mm256_setzero_ps(), xMinusY);
}
#endif

//native_fmax
float __attribute__((overloadable)) native_fmax(float x, float y)
{
	float res;
	__m128 tmpX = _mm_load_ss(&x);
	__m128 tmpY = _mm_load_ss(&y);

	float4 res1 = _mm_max_ss(tmpX, tmpY);

	_mm_store_ss(&res, res1);
	return res;
}

float4 __attribute__((overloadable)) native_fmax(float4 x, float4 y)
{
	return _mm_max_ps(x,y);
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) native_fmax(float8 x, float8 y)
{
	return _mm256_max_ps(x,y);
}
#endif // defined(__AVX__)
//native_fmin
float __attribute__((overloadable)) native_fmin(float x, float y)
{
	float res;
	__m128 tmpX = _mm_load_ss(&x);
	__m128 tmpY = _mm_load_ss(&y);

	float4 res1 = _mm_min_ss(tmpX,tmpY);

	_mm_store_ss(&res, res1);
	return res;
}

float4 __attribute__((overloadable)) native_fmin(float4 x, float4 y)
{
	return _mm_min_ps(x,y);
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) native_fmin(float8 x, float8 y)
{
	return _mm256_min_ps(x,y);
}
#endif // defined(__AVX__)

//native_fmod
float __attribute__((overloadable)) native_fmod(float x, float y)
{
	float res;
	__m128 xVec = _mm_load_ss(&x);
	__m128 yVec = _mm_load_ss(&y);

	__m128 m_trunc = _mm_div_ss(xVec, yVec);
	m_trunc = trunc((float4)m_trunc);

	yVec = _mm_mul_ss(yVec, m_trunc);
	__m128 tmp = _mm_sub_ss(xVec, yVec);

	_mm_store_ss(&res, tmp);
	return res;
}

float4 __attribute__((overloadable)) native_fmod(float4 x, float4 y)
{
	__m128 m_trunc = _mm_div_ps(x, y);
	m_trunc = trunc((float4)m_trunc);
	y = _mm_mul_ps(y, m_trunc);
	return _mm_sub_ps(x, y);
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) native_fmod(float8 x, float8 y)
{
    __m256 m_trunc = _mm256_div_ps(x, y);
    m_trunc = trunc((float8)m_trunc);
    y = _mm256_mul_ps(y, m_trunc);
    return _mm256_sub_ps(x, y);
}
#endif

//native_fract
float __attribute__((overloadable)) native_fract(float x, float *iptr)
{
	__m128 f4Floor;
	__m128 f4x;

	f4x = _mm_set_ss(x);
	
	f4Floor = floor((float4)f4x);
	f4x = _mm_sub_ss(f4x, f4Floor);
	__m128 res = _mm_min_ss(f4x, *(__m128*)mth_fractLimit);

	_mm_store_ss(iptr, f4Floor);
	return _mm_cvtss_f32(res);
}

float4 __attribute__((overloadable)) native_fract(float4 f4x, float4 *iptr)
{
#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
	*iptr = __floorf4(f4x);
#else
	*iptr = floor((float4)f4x);
#endif
	f4x = _mm_sub_ps(f4x, *iptr);
	__m128 res = _mm_min_ps(f4x, *(__m128*)mth_fractLimit);
	return res;
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) native_fract(float8 f8x, float8 *iptr)
{
    *iptr = floor(f8x);
    f8x = _mm256_sub_ps(f8x, *iptr);
    __m256 res = _mm256_min_ps(f8x, *(__m256*)mth_fractLimit8);
    return res;
}
#endif // defined(__AVX__)

//native_ilgob
_1i32 __attribute__((overloadable)) native_ilogb(float x)
{
	int ALIGN16 res[4];
	__m128 xVec = _mm_load_ss((float*)&x);

	__m128 res1 = _mm_and_ps(xVec, *(__m128*)mth_expMask);
	__m128i tmp = _mm_srli_epi32(_mm_castps_si128(res1), 23);
	tmp = _mm_sub_epi32(tmp, *(__m128i*)mth_StorageI127);

	_mm_store_si128 ((__m128i*)res, tmp);
	return res[0];
}

_4i32 __attribute__((overloadable)) native_ilogb(float4 x)
{
	__m128 res = _mm_and_ps(x, *(__m128*)mth_expMask);
	__m128i tmp = _mm_srli_epi32(_mm_castps_si128(res), 23);
	tmp = _mm_sub_epi32(tmp, *(__m128i*)mth_StorageI127);

	return (_4i32)tmp;
}

//native_logb
float __attribute__((overloadable)) native_logb(float x)
{
	float res;
	__m128 xVec = _mm_load_ss((float*)&x);

	__m128 res1 = _mm_and_ps(xVec, *(__m128*)mth_expMask);
	__m128i tmp = _mm_srli_epi32(_mm_castps_si128(res1), 23);
	tmp = _mm_sub_epi32(tmp, *(__m128i*)mth_StorageI127);
	res1 = _mm_cvtepi32_ps(tmp);

	_mm_store_ss(&res, res1);
	return res;
}

float4 __attribute__((overloadable)) native_logb(float4 x)
{
	__m128 res = _mm_and_ps(x, *(__m128*)mth_expMask);
	__m128i tmp = _mm_srli_epi32(_mm_castps_si128(res), 23);
	tmp = _mm_sub_epi32(tmp, *(__m128i*)mth_StorageI127);
	res = _mm_cvtepi32_ps(tmp);

	return res;
}

//native_hypot
float __attribute__((overloadable)) native_hypot(float x, float y)
{
      float4 a, b, res;
      a.s0 = x;
      b.s0 = y;
      a = _mm_mul_ss(a,a);
      b = _mm_mul_ss(b,b);
      res = _mm_add_ss(a,b);
      res = _mm_sqrt_ss(res);
      return res.s0;
}

float4 __attribute__((overloadable)) native_hypot(float4 x, float4 y)
{
      float4 res;
      x = _mm_mul_ps(x,x);
      y = _mm_mul_ps(y,y);
      res = _mm_add_ps(x,y);
      res = _mm_sqrt_ps(res);
      return res;
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) native_hypot(float8 x, float8 y)
{
      float8 res;
      x = _mm256_mul_ps(x,x);
      y = _mm256_mul_ps(y,y);
      res = _mm256_add_ps(x,y);
      res = _mm256_sqrt_ps(res);
      return res;
}
#endif // defined(__AVX__)
double __attribute__((overloadable)) native_hypot(double x, double y)
{
      double2 a, b, res;
      a.lo = x;
      b.lo = y;
      a = _mm_mul_sd(a,a);
      b = _mm_mul_sd(b,b);
      res = _mm_add_sd(a,b);
      res = _mm_sqrt_sd(res, res);
      return res.lo;
}

double2 __attribute__((overloadable)) native_hypot(double2 x, double2 y)
{
      double2 res;
      x = _mm_mul_pd(x,x);
      y = _mm_mul_pd(y,y);
      res = _mm_add_pd(x,y);
      res = _mm_sqrt_pd(res);
      return res;
}

#if defined(__AVX__)
double4 __attribute__((overloadable)) native_hypot(double4 x, double4 y)
{
      double4 res;
      x = _mm256_mul_pd(x,x);
      y = _mm256_mul_pd(y,y);
      res = _mm256_add_pd(x,y);
      res = _mm256_sqrt_pd(res);
      return res;
}
#endif // defined(__AVX__)
/* ****************************
** HALF FUNCTION DECLARED IN
** OPENCL SPEC rev 34 
** PAGE 208 TABLE 6.9
** ***************************/
//half_cos
float __attribute__((overloadable)) half_cos(float x)
{
	return native_cos(x);
}

float4 __attribute__((overloadable)) half_cos(float4 x)
{
	return native_cos(x);
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) half_cos(float8 x)
{
    return native_cos(x);
}
#endif
//half_divide
//float  __attribute__((overloadable)) half_divide(float x, float y)
//{
//	return native_divide(x,y);
//}

//float4  __attribute__((overloadable)) half_divide(float4 x, float4 y)
//{
//	return native_divide(x,y);
//}

//half_exp
float __attribute__((overloadable)) half_exp(float x)
{
	return native_exp(x);
}

float4 __attribute__((overloadable)) half_exp(float4 x)
{
	return native_exp(x);
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) half_exp(float8 x)
{
    return native_exp(x);
}
#endif

//half_exp2
float __attribute__((overloadable)) half_exp2(float x)
{
	return native_exp2(x);
}

float4 __attribute__((overloadable)) half_exp2(float4 x)
{
	return native_exp2(x);
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) half_exp2(float8 x)
{
    return native_exp2(x);
}
#endif // defined(__AVX__)
//half_exp10
float __attribute__((overloadable)) half_exp10(float x)
{
	return native_exp10(x);
}

float4 __attribute__((overloadable)) half_exp10(float4 x)
{
	return native_exp10(x);
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) half_exp10(float8 x)
{
    return native_exp10(x);
}
#endif // defined(__AVX__)

//half_log
float __attribute__((overloadable)) half_log(float x)
{
	return native_log(x);
}

float4 __attribute__((overloadable)) half_log(float4 x)
{
	return native_log(x);
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) half_log(float8 x)
{
    return native_log(x);
}
#endif // defined(__AVX__)

//half_log2
float __attribute__((overloadable)) half_log2(float x)
{
	return native_log2(x);
}

float4 __attribute__((overloadable)) half_log2(float4 x)
{
	return native_log2(x);
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) half_log2(float8 x)
{
    return native_log2(x);
}
#endif // defined(__AVX__)

//half_log10
float __attribute__((overloadable)) half_log10(float x)
{
	return native_log10(x);
}

float4 __attribute__((overloadable)) half_log10(float4 x)
{
	return native_log10(x);
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) half_log10(float8 x)
{
    return native_log10(x);
}
#endif // defined(__AVX__)

//half_powr
float  __attribute__((overloadable)) half_powr(float x, float y)
{
	return native_powr(x,y);
}

float4  __attribute__((overloadable)) half_powr(float4 x, float4 y)
{
	return native_powr(x,y);
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) half_powr(float8 x, float8 y)
{
    return native_powr(x, y);
}
#endif // defined(__AVX__)

//half_recip
float __attribute__((overloadable)) half_recip(float x)
{
	return native_recip(x);
}

float4 __attribute__((overloadable)) half_recip(float4 x)
{
	return native_recip(x);
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) half_recip(float8 x)
{
    return native_recip(x);
}
#endif // defined(__AVX__)

//half_rsqrt
float __attribute__((overloadable)) half_rsqrt(float x)
{
	return native_rsqrt(x);
}

float4 __attribute__((overloadable)) half_rsqrt(float4 x)
{
	return native_rsqrt(x);
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) half_rsqrt(float8 x)
{
    return native_rsqrt(x);
}
#endif // defined(__AVX__)

//half_sin
float __attribute__((overloadable)) half_sin(float x)
{
	return native_sin(x);
}

float4 __attribute__((overloadable)) half_sin(float4 x)
{
	return native_sin(x);
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) half_sin(float8 x)
{
    return native_sin(x);
}
#endif // defined(__AVX__)

//half_sqrt
float __attribute__((overloadable)) half_sqrt(float x)
{
	return native_sqrt(x);
}

float4 __attribute__((overloadable)) half_sqrt(float4 x)
{
	return native_sqrt(x);
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) half_sqrt(float8 x)
{
    return native_sqrt(x);
}
#endif // defined(__AVX__)

//half_tan
float __attribute__((overloadable)) half_tan(float x)
{
	return native_tan(x);
}

float4 __attribute__((overloadable)) half_tan(float4 x)
{
	return native_tan(x);
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) half_tan(float8 x)
{
    return native_tan(x);
}
#endif // defined(__AVX__)

/* ****************************
** NATIVE FUNCTION DECLARED IN
** OPENCL SPEC rev 34 
** PAGE 208 TABLE 6.9
** + double versions (extension)
** ***************************/

//native_cos
float __attribute__((overloadable)) native_cos(float x)
{
	float res;
	res = OCL_SVML_FUNCTION(_cosf1)(x);
	return res;
}

float4 __attribute__((overloadable)) native_cos(float4 x)
{
	__m128 res = OCL_SVML_FUNCTION(_cosf4)(x);
	return res;
}
#if defined(__AVX__)
float8 __attribute__((overloadable)) native_cos(float8 x)
{
	__m256 res = OCL_SVML_FUNCTION(_cosf8)(x);
	return res;
}
#endif // defined(__AVX__)

double __attribute__((overloadable)) native_cos(double x)
{
	double res;
	res = OCL_SVML_FUNCTION(_cos1)(x);
	return res;
}

double2 __attribute__((overloadable)) native_cos(double2 x)
{
	double2 res;
	res = OCL_SVML_FUNCTION(_cos2)(x);
	return res;
}
#if defined(__AVX__)
double4 __attribute__((overloadable)) native_cos(double4 x)
{
	double4 res;
	res = OCL_SVML_FUNCTION(_cos4)(x);
	return res;
}
#endif // defined(__AVX__)
//native_divide
float __attribute__((overloadable)) native_divide(float x, float y)
{
	float res;
	__m128 xVec = _mm_load_ss((float*)&x);
	__m128 yVec = _mm_load_ss((float*)&y);

	__m128 res1 = _mm_rcp_ss( yVec );
	res1 = _mm_mul_ss( res1, xVec );

	_mm_store_ss(&res, res1);
	return res;
}

float4 __attribute__((overloadable)) native_divide(float4 x, float4 y)
{
	__m128 res = _mm_rcp_ps( y );
	return _mm_mul_ps( res, x );
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) native_divide(float8 x, float8 y)
{
    __m256 res = _mm256_rcp_ps( y );
    return _mm256_mul_ps( res, x );
}
#endif // defined(__AVX__)

double __attribute__((overloadable)) native_divide(double x, double y)
{
	double res;
	__m128d xVec = _mm_load_sd((double*)&x);
	__m128d yVec = _mm_load_sd((double*)&y);

	__m128d res1 = _mm_div_sd( xVec,yVec );

	_mm_store_sd(&res, res1);
	return res;
}

double2 __attribute__((overloadable)) native_divide(double2 x, double2 y)
{
	
	return _mm_div_pd(  x , y );
}
#if defined(__AVX__)
double4 __attribute__((overloadable)) native_divide(double4 x, double4 y)
{
	
	return _mm256_div_pd(  x , y );
}
#endif

//native_exp
float __attribute__((overloadable)) native_exp(float x)
{
	float res = OCL_SVML_FUNCTION(_expf1)(x);
	return res;
}

float4 __attribute__((overloadable)) native_exp(float4 x)
{
	__m128 res = OCL_SVML_FUNCTION(_expf4)(x);
	return res;
}
#if defined(__AVX__)
float8 __attribute__((overloadable)) native_exp(float8 x)
{
    __m256 res = OCL_SVML_FUNCTION(_expf8)(x);
    return res;
}
#endif // defined(__AVX__)

double __attribute__((overloadable)) native_exp(double x)
{
	double res = OCL_SVML_FUNCTION(_exp1)(x);
	return res;
}

double2 __attribute__((overloadable)) native_exp(double2 x)
{
	double2 res = OCL_SVML_FUNCTION(_exp2)(x);
	return res;
}

double3 __attribute__((overloadable)) native_exp(double3 x)
{
    double4 valx; valx.s012 = x;
	double4 res = OCL_SVML_FUNCTION(_exp4)(valx);
	return res.s012;
}

double4 __attribute__((overloadable)) native_exp(double4 x)
{
	double4 res = OCL_SVML_FUNCTION(_exp4)(x);
	return res;
}

double8 __attribute__((overloadable)) native_exp(double8 x)
{
	double8 res = OCL_SVML_FUNCTION(_exp8)(x);
	return res;
}

double16 __attribute__((overloadable)) native_exp(double16 x)
{    
	double16 res; 
    res.lo = OCL_SVML_FUNCTION(_exp8)(x.lo);
    res.hi = OCL_SVML_FUNCTION(_exp8)(x.hi);
	return res;
}

//native_exp2
float __attribute__((overloadable)) native_exp2(float x)
{
	float res = OCL_SVML_FUNCTION(_exp2f1)(x);
	return res;
}

float4 __attribute__((overloadable)) native_exp2(float4 x)
{
	__m128 res = OCL_SVML_FUNCTION(_exp2f4)(x);
	return res;
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) native_exp2(float8 x)
{
	__m256 res = OCL_SVML_FUNCTION(_exp2f8)(x);
	return res;
}
#endif

double __attribute__((overloadable)) native_exp2(double x)
{
	double res = OCL_SVML_FUNCTION(_exp21)(x);
	return res;
}

double2 __attribute__((overloadable)) native_exp2(double2 x)
{
	double2 res = OCL_SVML_FUNCTION(_exp22)(x);
	return res;
}

#if defined(__AVX__)
double4 __attribute__((overloadable)) native_exp2(double4 x)
{
	double4 res = OCL_SVML_FUNCTION(_exp24)(x);
	return res;
}
#endif
//native_exp10
float __attribute__((overloadable)) native_exp10(float x)
{
	float res = OCL_SVML_FUNCTION(_exp10f1)(x);
	return res;
}

float4 __attribute__((overloadable)) native_exp10(float4 x)
{
	__m128 res = OCL_SVML_FUNCTION(_exp10f4)(x);
	return res;
}
#if defined(__AVX__)
float8 __attribute__((overloadable)) native_exp10(float8 x)
{
	__m256 res = OCL_SVML_FUNCTION(_exp10f8)(x);
	return res;
}
#endif // defined(__AVX__)

double __attribute__((overloadable)) native_exp10(double x)
{
	double res = OCL_SVML_FUNCTION(_exp101)(x);
	return res;
}

double2 __attribute__((overloadable)) native_exp10(double2 x)
{
	double2 res = OCL_SVML_FUNCTION(_exp102)(x);
	return res;
}

#if defined(__AVX__)
double4 __attribute__((overloadable)) native_exp10(double4 x)
{
	double4 res = OCL_SVML_FUNCTION(_exp104)(x);
	return res;
}
#endif // defined(__AVX__)
//native_log
float __attribute__((overloadable)) native_log(float x)
{
	float res = OCL_SVML_FUNCTION(_logf1)(x);
	return res;
}

float4 __attribute__((overloadable)) native_log(float4 x)
{
	__m128 res = OCL_SVML_FUNCTION(_logf4)(x);
	return res;
}
#if defined(__AVX__)
float8 __attribute__((overloadable)) native_log(float8 x)
{
	__m256 res = OCL_SVML_FUNCTION(_logf8)(x);
	return res;
}
#endif // defined(__AVX__)

double __attribute__((overloadable)) native_log(double x)
{
	double res = OCL_SVML_FUNCTION(_log1)(x);
	return res;
}

double2 __attribute__((overloadable)) native_log(double2 x)
{
	double2 res = OCL_SVML_FUNCTION(_log2)(x);
	return res;
}
#if defined(__AVX__)
double4 __attribute__((overloadable)) native_log(double4 x)
{
	double4 res = OCL_SVML_FUNCTION(_log4)(x);
	return res;
}
#endif // defined(__AVX__)

//native_log2
float __attribute__((overloadable)) native_log2(float x)
{
	float res = OCL_SVML_FUNCTION(_log2f1)(x);
	return res;
}

float4 __attribute__((overloadable)) native_log2(float4 x)
{
	__m128 res = OCL_SVML_FUNCTION(_log2f4)(x);
	return res;
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) native_log2(float8 x)
{
	__m256 res = OCL_SVML_FUNCTION(_log2f8)(x);
	return res;
}
#endif

double __attribute__((overloadable)) native_log2(double x)
{
	double res = OCL_SVML_FUNCTION(_log21)(x);
	return res;
}

double2 __attribute__((overloadable)) native_log2(double2 x)
{
	double2 res = OCL_SVML_FUNCTION(_log22)(x);
	return res;
}

#if defined(__AVX__)
double4 __attribute__((overloadable)) native_log2(double4 x)
{
	double4 res = OCL_SVML_FUNCTION(_log24)(x);
	return res;
}
#endif // defined(__AVX__)
//native_log10
float __attribute__((overloadable)) native_log10(float x)
{
	float res = OCL_SVML_FUNCTION(_log10f1)(x);
	return res;
}

float4 __attribute__((overloadable)) native_log10(float4 x)
{
	__m128 res = OCL_SVML_FUNCTION(_log10f4)(x);
	return res;
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) native_log10(float8 x)
{
	__m256 res = OCL_SVML_FUNCTION(_log10f8)(x);
	return res;
}
#endif // defined(__AVX__)

double __attribute__((overloadable)) native_log10(double x)
{
	double res = OCL_SVML_FUNCTION(_log101)(x);
	return res;
}

double2 __attribute__((overloadable)) native_log10(double2 x)
{
	double2 res = OCL_SVML_FUNCTION(_log102)(x);
	return res;
}

#if defined(__AVX__)
double4 __attribute__((overloadable)) native_log10(double4 x)
{
	double4 res = OCL_SVML_FUNCTION(_log104)(x);
	return res;
}
#endif // defined(__AVX__)

//native_powr
float __attribute__((overloadable)) native_powr(float x,float y)
{
	float res = OCL_SVML_FUNCTION(_powrf1)(x,y);
	return res;
}

float4 __attribute__((overloadable)) native_powr(float4 x,float4 y)
{
	__m128 res = OCL_SVML_FUNCTION(_powrf4)(x,y);
	return res;
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) native_powr(float8 x,float8 y)
{
	__m256 res = OCL_SVML_FUNCTION(_powrf8)(x,y);
	return res;
}
#endif // defined(__AVX__)

double __attribute__((overloadable)) native_powr(double x,double y)
{
	double res = OCL_SVML_FUNCTION(_powr1)(x,y);
	return res;
}

double2 __attribute__((overloadable)) native_powr(double2 x,double2 y)
{
	double2 res = OCL_SVML_FUNCTION(_powr2)(x,y);
	return res;
}

#if defined(__AVX__)
double4 __attribute__((overloadable)) native_powr(double4 x,double4 y)
{
	double4 res = OCL_SVML_FUNCTION(_powr4)(x,y);
	return res;
}
#endif 
//native recip
float __attribute__((overloadable)) native_recip(float x)
{
	float res;
	__m128 xVec = _mm_load_ss((float*)&x);

	__m128 res1 = _mm_rcp_ss( xVec );

	_mm_store_ss(&res, res1);
	return res;
}

float4 __attribute__((overloadable)) native_recip(float4 x)
{
	return _mm_rcp_ps(x);
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) native_recip(float8 x)
{
	return _mm256_rcp_ps(x);
}
#endif // defined(__AVX__)

double __attribute__((overloadable)) native_recip(double x)
{
	double res;
	__m128d one=_mm_set_sd(1.0);
	__m128d xVec = _mm_load_sd((double*)&x);

	__m128d res1 = _mm_div_sd( one ,xVec );

	_mm_store_sd(&res, res1);
	return res;
}

double2 __attribute__((overloadable)) native_recip(double2 x)
{
	__m128d one=_mm_set1_pd(1.0);
	return _mm_div_pd(one , x);
}

#if defined(__AVX__)
double4 __attribute__((overloadable)) native_recip(double4 x)
{
    return _mm256_div_pd(_mm256_set1_pd(1.0), x);
}
#endif // defined(__AVX__)

//native rsqrt
float __attribute__((overloadable)) native_rsqrt(float x)
{
	float res;
	__m128 xVec = _mm_load_ss((float*)&x);

	__m128 res1 = _mm_rsqrt_ss( xVec );

	_mm_store_ss(&res, res1);
	return res;
}

float4 __attribute__((overloadable)) native_rsqrt(float4 x)
{
	return _mm_rsqrt_ps( x );
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) native_rsqrt(float8 x)
{
	return _mm256_rsqrt_ps( x );
}
#endif

double __attribute__((overloadable)) native_rsqrt(double x)
{
	double res;
	__m128 xVec = _mm_load_sd((double*)&x);

	__m128 res1 = _mm_sqrt_sd( xVec ,xVec);

	res1=_mm_div_sd(_mm_set_sd(1.0),res1);

	_mm_store_sd(&res, res1);
	return res;
}

double2 __attribute__((overloadable)) native_rsqrt(double2 x)
{
	
	return _mm_div_pd( _mm_set1_pd(1.0),_mm_sqrt_pd(x));
}

#if defined(__AVX__)
double4 __attribute__((overloadable)) native_rsqrt(double4 x)
{
	
	return _mm256_div_pd( _mm256_set1_pd(1.0),_mm256_sqrt_pd(x));
}
#endif // defined(__AVX__)
//native_sin
float __attribute__((overloadable)) native_sin(float x)
{
	float res;
	res = OCL_SVML_FUNCTION(_sinf1)(x);
	return res;
}

float4 __attribute__((overloadable)) native_sin(float4 x)
{
	__m128 res = OCL_SVML_FUNCTION(_sinf4)(x);
	return res;
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) native_sin(float8 x)
{
    __m256 res = OCL_SVML_FUNCTION(_sinf8)(x);
    return res;
}
#endif

double __attribute__((overloadable)) native_sin(double x)
{
	double res;
	res = OCL_SVML_FUNCTION(_sin1)(x);
	return res;
}

double2 __attribute__((overloadable)) native_sin(double2 x)
{
	double2 res;
	res = OCL_SVML_FUNCTION(_sin2)(x);
	return res;
}

#if defined(__AVX__)
double4 __attribute__((overloadable)) native_sin(double4 x)
{
	double4 res;
	res = OCL_SVML_FUNCTION(_sin4)(x);
	return res;
}
#endif // defined(__AVX__)
//native_sqrt
float __attribute__((overloadable)) native_sqrt(float x)
{
	float res;
	__m128 xVec = _mm_load_ss((float*)&x);

	__m128 res1 = _mm_sqrt_ss( xVec );

	_mm_store_ss(&res, res1);
	return res;
}

float4 __attribute__((overloadable)) native_sqrt(float4 x)
{
	return _mm_sqrt_ps( x );
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) native_sqrt(float8 x)
{
	return _mm256_sqrt_ps( x );
}
#endif // defined(__AVX__)

double __attribute__((overloadable)) native_sqrt(double x)
{
	double res;
	__m128 xVec = _mm_load_sd((double*)&x);

	__m128 res1 = _mm_sqrt_sd( xVec,xVec );

	_mm_store_sd(&res, res1);
	return res;
}

double2 __attribute__((overloadable)) native_sqrt(double2 x)
{
	return _mm_sqrt_pd( x );
}

#if defined(__AVX__)
double4 __attribute__((overloadable)) native_sqrt(double4 x)
{
	return _mm256_sqrt_pd( x );
}
#endif // defined(__AVX__)

//native_tan
float __attribute__((overloadable)) native_tan(float x)
{
	float res;
	res = OCL_SVML_FUNCTION(_tanf1)(x);
	return res;
}

float4 __attribute__((overloadable)) native_tan(float4 x)
{
	__m128 res = OCL_SVML_FUNCTION(_tanf4)(x);
	return res;
}

#if defined(__AVX__)
float8 __attribute__((overloadable)) native_tan(float8 x)
{
	__m256 res = OCL_SVML_FUNCTION(_tanf8)(x);
	return res;
}
#endif // defined(__AVX__)

double __attribute__((overloadable)) native_tan(double x)
{
	double res;
	res = OCL_SVML_FUNCTION(_tan1)(x);
	return res;
}

double2 __attribute__((overloadable)) native_tan(double2 x)
{
	double2 res;
	res = OCL_SVML_FUNCTION(_tan2)(x);
	return res;
}

#if defined(__AVX__)
double4 __attribute__((overloadable)) native_tan(double4 x)
{
	double4 res;
	res = OCL_SVML_FUNCTION(_tan4)(x);
	return res;
}
#endif // defined(__AVX__)

#if defined(__AVX__)
/*
BIs used with OCL_INTR_P1_F8_F8 macro.  
OCL_INTR_P1_F8_F8 was overloaded for AVX and expects to have float8 function.
Macro OCL_INTR_P1_F8_F8_PROXY declares float8 BI proxy function using 2
calls to float4 BI.It is intended to be used when native float8 datatype
AVX implementation of built-in is not available.

If AVX native float8 function is implemented it should be removed from this list
*/

OCL_INTR_P1_F8_F8_PROXY(native_logb)
#endif // defined(__AVX__)

#ifdef __cplusplus
}
#endif


