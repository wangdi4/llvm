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
//  relational_functions.cpp
///////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif

#include <intrin.h>
#if defined(__AVX__)
#include <avxintrin.h>
#endif
#include "cl_relational_declaration.h"

#define ALIGN16 __attribute__((aligned(16)))
#define ALIGN32 __attribute__((aligned(32)))

ALIGN16 const int fnan_min[] = {0x7F800000, 0x7F800000, 0x7F800000, 0x7F800000}; // or numeric_limits<float>::quiet_NaN();
ALIGN16 const int fnan_max[] = {0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF}; // or numeric_limits<float>::quiet_NaN();
ALIGN16 const _1i64 dnan_min[] = {0x7FF0000000000000, 0x7FF0000000000000}; // or numeric_limits<float>::quiet_NaN();
ALIGN16 const _1i64 dnan_max[] = {0x7FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF}; // or numeric_limits<float>::quiet_NaN();
ALIGN16 const int fexp_mask[] = {0x7F800000, 0x7F800000, 0x7F800000, 0x7F800000};
ALIGN16 const int fman_mask[] = {0x007FFFFF, 0x007FFFFF, 0x007FFFFF, 0x007FFFFF};
ALIGN16 const _1i64 dexp_mask[] = {0x7FF0000000000000, 0x7FF0000000000000};
ALIGN16 const _1i64 dman_mask[] = {0x000FFFFFFFFFFFFF, 0x000FFFFFFFFFFFFF};
ALIGN16 const float fltm[] = {FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN};
ALIGN16 const int fsign_mask[] = {0x80000000, 0x80000000, 0x80000000, 0x80000000};
ALIGN16 const _1i64 dsign_mask[] = {0x8000000000000000, 0x8000000000000000};
ALIGN16 const _1i64 FF[] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};

#if defined (__AVX__)
ALIGN32 const int fsign_mask8[] = {0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000};
ALIGN32 const int FFFMask8[] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
ALIGN32 const float fltm8[] = {FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN};
ALIGN32 const int fnan_min8[] = {0x7F800000, 0x7F800000, 0x7F800000, 0x7F800000, 0x7F800000, 0x7F800000, 0x7F800000, 0x7F800000}; // or numeric_limits<float>::quiet_NaN();
ALIGN32 const int fnan_max8[] = {0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF}; // or numeric_limits<float>::quiet_NaN();
ALIGN32 const _1i64 d4nan_min[] = {0x7FF0000000000000, 0x7FF0000000000000, 0x7FF0000000000000, 0x7FF0000000000000}; // or numeric_limits<float>::quiet_NaN();
ALIGN32 const _1i64 d4nan_max[] = {0x7FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF}; // or numeric_limits<float>::quiet_NaN();
ALIGN32 const _1i64 d4exp_mask[] = {0x7FF0000000000000, 0x7FF0000000000000, 0x7FF0000000000000, 0x7FF0000000000000};
ALIGN32 const _1i64 d4sign_mask[] = {0x8000000000000000, 0x8000000000000000, 0x8000000000000000, 0x8000000000000000};
ALIGN32 const _1i64 FF4[] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
#endif

float __fabsf(float x)
{
    __m128 fX = _mm_load_ss(&x);
    fX = _mm_and_ps(fX, *((__m128*)fnan_max));
    float res;
    _mm_store_ss(&res, fX);
    return res;
}

#define IMPL_REL_FUNC_vI_vX_vX(FUNC, OPERATOR, TO, TI) \
int		__attribute__((overloadable)) FUNC(TI x, TI y) { return (x OPERATOR y); } \
TO##2	__attribute__((overloadable)) FUNC(TI##2 x, TI##2 y) { return (x OPERATOR y); } \
TO##3	__attribute__((overloadable)) FUNC(TI##3 x, TI##3 y) { return (x OPERATOR y); } \
TO##4	__attribute__((overloadable)) FUNC(TI##4 x, TI##4 y) { return (x OPERATOR y); } \
TO##8	__attribute__((overloadable)) FUNC(TI##8 x, TI##8 y) { return (x OPERATOR y); } \
TO##16	__attribute__((overloadable)) FUNC(TI##16 x, TI##16 y) { return (x OPERATOR y); }

IMPL_REL_FUNC_vI_vX_vX(isequal, ==, int, float)
IMPL_REL_FUNC_vI_vX_vX(isequal, ==, long, double)
IMPL_REL_FUNC_vI_vX_vX(isnotequal, !=, int, float)
IMPL_REL_FUNC_vI_vX_vX(isnotequal, !=, long, double)
IMPL_REL_FUNC_vI_vX_vX(isgreater, >, int, float)
IMPL_REL_FUNC_vI_vX_vX(isgreater, >, long, double)
IMPL_REL_FUNC_vI_vX_vX(isgreaterequal, >=, int, float)
IMPL_REL_FUNC_vI_vX_vX(isgreaterequal, >=, long, double)
IMPL_REL_FUNC_vI_vX_vX(isless, <, int, float)
IMPL_REL_FUNC_vI_vX_vX(isless, <, long, double)
IMPL_REL_FUNC_vI_vX_vX(islessequal, <=, int, float)
IMPL_REL_FUNC_vI_vX_vX(islessequal, <=, long, double)

//Returns the component-wise compare of (x < y) || (x > y)
int  __attribute__((overloadable)) islessgreater(float x, float y)
{
	if( (x!=x) || (y!=y) )
	{
		return 0;
	}
	 return (x < y) || (x > y);
}

_4i32  __attribute__((overloadable)) islessgreater(float4 x, float4 y)
{
	float4 res = _mm_cmpgt_ps(x,y);
	float4 res1 = _mm_cmpgt_ps(y,x);
	res = _mm_or_ps(res, res1);
	//convert to inetegr
	return (_4i32) _mm_castps_si128(res);	
}
#if defined (__AVX__)
_8i32  __attribute__((overloadable)) islessgreater(float8 x, float8 y)
{
	float8 res = _mm256_cmp_ps(x,y, _CMP_GT_OS);
	float8 res1 = _mm256_cmp_ps(y,x, _CMP_GT_OS);
	res = _mm256_or_ps(res, res1);
	//convert to inetegr
	return (_8i32) _mm256_castps_si256(res);	
}
#else
_8i32  __attribute__((overloadable)) islessgreater(float8 x, float8 y)
{
	_8i32 res;
	res.lo = islessgreater(x.lo,y.lo);
	res.hi = islessgreater(x.hi,y.hi);
	//convert to inetegr
	return res;	
}
#endif

int __attribute__((overloadable)) islessgreater(double x, double y)
{
	if( (x!=x) || (y!=y) )
	{
		return 0;
	}
	 return (x < y) || (x > y);
}

_2i64 __attribute__((overloadable)) islessgreater(double2 x, double2 y)
{
	double2 res = _mm_cmpgt_pd(x,y);
	double2 res1 = _mm_cmpgt_pd(y,x);
	res = _mm_or_pd(res, res1);
	return  _mm_castpd_si128(res);
}

#if defined (__AVX__)
_4i64 __attribute__((overloadable)) islessgreater(double4 x, double4 y)
{
	double4 res = _mm256_cmp_pd(x,y, _CMP_GT_OS);
	double4 res1 = _mm256_cmp_pd(y,x, _CMP_GT_OS);
	res = _mm256_or_pd(res, res1);
	return  _mm256_castpd_si256(res);
}
#else
_4i64 __attribute__((overloadable)) islessgreater(double4 x, double4 y)\
{
	_4i64 res;
	res.lo = islessgreater(x.lo, y.lo);
	res.hi = islessgreater(x.hi, y.hi);
	return res;
}
#endif

//Test for finite value.
int  __attribute__((overloadable)) isfinite(float x)
{
	if (__fabsf(x) < INFINITY)
	{
		return 1;
	}
	return 0;
}

_4i32  __attribute__((overloadable)) isfinite(float4 x)
{
	//calculate abs(x)
	// Remove sign bit of the float
	x = _mm_castsi128_ps(_mm_and_si128(_mm_castps_si128(x), *((__m128i*)fnan_max)));
    _4i32 res = (_4i32)isless(x, *((float4*)fnan_min));
	return res;
}

#if defined (__AVX__)
_8i32  __attribute__((overloadable)) isfinite(float8 x)
{
	//calculate abs(x)
	// Remove sign bit of the float
	x = _mm256_and_ps((__m256)x, *((__m256*)fnan_max8));
	return (_8i32)isless(x, *((float8*)fnan_min8));
}
#else
_8i32 __attribute__((overloadable)) isfinite(float8 x)
{
	_8i32 res;
	res.lo = isfinite(x.lo);
	res.hi = isfinite(x.hi);
	return res;
}
#endif


int __attribute__((overloadable)) isfinite(double x)
{
	_1i64 xAsLong = *((_1i64*)&x);
	if( ( xAsLong & dnan_min[0] ) != dnan_min[0] )
	{
		return 1;
	}
	return 0;
}

_2i64 __attribute__((overloadable)) isfinite(double2 x)
{
	x = _mm_and_pd(x, *((__m128d*)dnan_min));
	double2 dRes = _mm_cmpneq_pd(x, *((__m128d*)dnan_min));
	_2i64 res = _mm_castpd_si128(dRes);
	return res;
}

#if defined (__AVX__)
_4i64 __attribute__((overloadable)) isfinite(double4 x)
{
	x = _mm256_and_pd(x, *((__m256d*)d4nan_min));
	return (_4i64) _mm256_cmp_pd(x, *((__m256d*)d4nan_min), _CMP_NEQ_OS);
}
#else
_4i64 __attribute__((overloadable)) isfinite(double4 x)
{
	_4i64 res;
	res.lo = isfinite(x.lo);
	res.hi = isfinite(x.hi);
	return res;
}
#endif


//Test for infinity value (+ve or –ve) .
int  __attribute__((overloadable)) isinf(float x)
{
	if (INFINITY == __fabsf(x))
	{
		return 1;
	}
	return 0;
}

_4i32  __attribute__((overloadable)) isinf(float4 x)
{
	//calculate abs(x)
	// Remove sign bit of the float
	x = _mm_castsi128_ps(_mm_and_si128(_mm_castps_si128(x), *((__m128i*)fnan_max)));
    return isgreaterequal(x, *((float4*)fnan_min));
}
#if defined (__AVX__)
_8i32  __attribute__((overloadable)) isinf(float8 x)
{
	//calculate abs(x)
	// Remove sign bit of the float
	x = _mm256_and_ps(x , *((__m256*)fnan_max8));
    return isgreaterequal(x, *((float8*)fnan_min8));
}
#else
_8i32 __attribute__((overloadable)) isinf(float8 x)
{
	_8i32 res;
	res.lo = isinf(x.lo);
	res.hi = isinf(x.hi);
	return res;
}
#endif


int __attribute__((overloadable)) isinf(double x)
{
	_1i64 xAsLong = *((_1i64*)&x);
	if( ( xAsLong & dnan_max[0] ) == dnan_min[0] )
	{
		return 1;
	}
	return 0;
}

_2i64 __attribute__((overloadable)) isinf(double2 x)
{
	//calculate abs(x)
	// Remove sign bit of the float
	x = _mm_and_pd( x, *((__m128d*)dnan_max) );
	double2 dRes = _mm_cmpeq_pd(x, *((__m128d*)dnan_min));
	_2i64 res = _mm_castpd_si128(dRes);
	return res;
}

#if defined (__AVX__)
_4i64 __attribute__((overloadable)) isinf(double4 x)
{
	//calculate abs(x)
	// Remove sign bit of the float
	x = _mm256_and_pd( x, *((__m256d*)d4nan_max) );
	return (_4i64)_mm256_cmp_pd(x, *((__m256d*)d4nan_min), _CMP_EQ_OQ);
}
#else
_4i64 __attribute__((overloadable)) isinf(double4 x)
{
	_4i64 res;
	res.lo = isinf(x.lo);
	res.hi = isinf(x.hi);
	return res;
}
#endif

//Test for a NaN.
_1i32  __attribute__((overloadable)) isnan(float f)
{
	return f != f;
}

_4i32  __attribute__((overloadable)) isnan(float4 x)
{
	__m128i mask = _mm_cmpgt_epi32(_mm_and_si128(_mm_castps_si128(x), *((__m128i*)fnan_max)), *((__m128i*)fnan_min));
	mask = _mm_andnot_si128(_mm_cmpgt_epi32(_mm_castps_si128(x), *((__m128i*)fnan_max)), mask);
	return (_4i32)mask;
}
#if defined (__AVX__)
_8i32  __attribute__((overloadable)) isnan(float8 x)
{
	return (_8i32)_mm256_cmp_ps(x, x, _CMP_UNORD_Q);
}
#else
_8i32 __attribute__((overloadable)) isnan(float8 x)
{
	_8i32 res;
	res.lo = isnan(x.lo);
	res.hi = isnan(x.hi);
	return res;
}
#endif

_1i32 __attribute__((overloadable)) isnan(double f)
{
	return f != f;
}

_2i64 __attribute__((overloadable)) isnan(double2 x)
{
	double2 res = _mm_cmpneq_pd(x, x);
	return _mm_castpd_si128(res);
}

#if defined (__AVX__)
_4i64  __attribute__((overloadable)) isnan(double4 x)
{
	return (_4i64)_mm256_cmp_pd(x, x, _CMP_UNORD_Q);
}
#else
_4i64  __attribute__((overloadable)) isnan(double4 x)
{
	_4i64 res;
	res.lo = isnan(x.lo);
	res.hi = isnan(x.hi);
	return res;
}
#endif

//Test for a normal value
int  __attribute__((overloadable)) isnormal(float x)
{
	int iAsIntAbs = (*(int*)&x) & fnan_max[0];
	float fAbs = *((float*)&iAsIntAbs);
	if ((fAbs < INFINITY) && (fAbs >= FLT_MIN))
	{
		return 1;
	}
	return 0;
}

_4i32  __attribute__((overloadable)) isnormal(float4 x)
{
	// Remove sign bit of the float
	x = _mm_castsi128_ps(_mm_and_si128(_mm_castps_si128(x), *((__m128i*)fnan_max)));
    __m128i res =  (__m128i)isless(x, *((float4*)fnan_min)); //x<INFINITY
    __m128i res1 = (__m128i)isgreaterequal(x, *((float4*)fltm)); //x >= FLT_MIN 
	return (_4i32)_mm_and_si128(res,res1);
}
#if defined (__AVX__)
_8i32  __attribute__((overloadable)) isnormal(float8 x)
{
	// Remove sign bit of the float
	x = _mm256_and_ps(x, *((__m256*)fnan_max8));
    __m256 res =  (__m256)isless(x, *((float8*)fnan_min8)); //x<INFINITY
    __m256 res1 = (__m256)isgreaterequal(x, *((float8*)fltm8)); //x >= FLT_MIN 
	return (_8i32)_mm256_and_ps(res,res1);
}
#else
_8i32 __attribute__((overloadable)) isnormal(float8 x)
{
	_8i32 res;
	res.lo = isnormal(x.lo);
	res.hi = isnormal(x.hi);
	return res;
}
#endif

int __attribute__((overloadable)) isnormal(double x)
{
	_1i64 iAsLong = *((_1i64*)&x);
	bool NaNorINF = ( iAsLong & dexp_mask[0] ) == dexp_mask[0];									//NaN or INF
	bool denom = ( iAsLong & dexp_mask[0] ) == 0;	//denom
	if( NaNorINF || denom )
	{
		return 0;
	}
	return 1;
}

_2i64 __attribute__((overloadable)) isnormal(double2 x)
{
	double2 exp = _mm_and_pd(x, *(__m128d*)dexp_mask);
	_2i64 NaNorINF = _mm_castpd_si128(_mm_cmpeq_pd(exp, *(__m128d*)dexp_mask));
	_2i64 denom = _mm_castpd_si128(_mm_cmpeq_pd(exp, _mm_setzero_pd()));
	_2i64 res = _mm_or_si128(NaNorINF, denom);
	return _mm_xor_si128(res, *(__m128i*)FF);
}

#if defined (__AVX__)
_4i64  __attribute__((overloadable)) isnormal(double4 x)
{
	double4 exp = _mm256_and_pd(x, *(__m256d*)d4exp_mask);
	_4i64 NaNorINF = _mm256_castpd_si256(_mm256_cmp_pd(exp, *(__m256d*)d4exp_mask, _CMP_EQ_OQ));
	_4i64 denom = _mm256_castpd_si256(_mm256_cmp_pd(exp, _mm256_setzero_pd(), _CMP_EQ_OQ));
	_4i64 res = _mm256_or_pd(NaNorINF, denom);
	return (_4i64)_mm256_xor_pd(res, *(__m256d*)FF4);
}
#else
_4i64  __attribute__((overloadable)) isnormal(double4 x)
{
	_4i64 res;
	res.lo = isnormal(x.lo);
	res.hi = isnormal(x.hi);
	return res;
}
#endif

// Test for ordered
int  __attribute__((overloadable)) isordered(float x, float y)
{
	return x == x && y == y;
}

_4i32  __attribute__((overloadable)) isordered(float4 x, float4 y)
{
	float4 res = _mm_cmpord_ps(x, y);
	//convert to inetegr
	return (_4i32) _mm_castps_si128(res);	
}
#if defined (__AVX__)
_8i32  __attribute__((overloadable)) isordered(float8 x, float8 y)
{
	return (_8i32)_mm256_cmp_ps(x, y, _CMP_ORD_Q);
}
#else
_8i32  __attribute__((overloadable)) isordered(float8 x, float8 y)
{
	_8i32 res;
	res.lo = isordered(x.lo,y.lo);
	res.hi = isordered(x.hi,y.hi);
	//convert to inetegr
	return res;	
}
#endif

int __attribute__((overloadable)) isordered(double x, double y)
{
	return x == x && y == y;
}

_2i64 __attribute__((overloadable)) isordered (double2 x, double2 y)
{
	double2 res = _mm_cmpord_pd(x, y);
	//convert to inetegr
	return  _mm_castpd_si128(res);	
}

#if defined (__AVX__)
_4i64 __attribute__((overloadable)) isordered (double4 x, double4 y)
{
	return  (_4i64)_mm256_cmp_pd(x, y,_CMP_ORD_Q);
}
#else
_4i64  __attribute__((overloadable)) isordered(double4 x, double4 y)
{
	_4i64 res;
	res.lo = isordered(x.lo,y.lo);
	res.hi = isordered(x.hi,y.hi);
	//convert to inetegr
	return res;	
}
#endif


//Test if arguments are unordered. isunordered() takes
//arguments x and y, returning non-zero if x or y is NaN,
//and zero otherwise
int  __attribute__((overloadable)) isunordered(float x, float y)
{
	return ((x!=x) || (y!=y));
}

_4i32  __attribute__((overloadable)) isunordered(float4 x, float4 y)
{
	float4 res = _mm_cmpunord_ps(x,y);
	//convert to inetegr
	__m128i resi = _mm_cvtps_epi32(res);
	__m128i tmp = _mm_srai_epi32(resi, 35);
	return (_4i32)tmp;
}

#if defined (__AVX__)
_8i32  __attribute__((overloadable)) isunordered(float8 x, float8 y)
{
	return (_8i32)_mm256_cmp_ps(x,y, _CMP_UNORD_S);
}
#else
_8i32  __attribute__((overloadable)) isunordered(float8 x, float8 y)
{
	_8i32 res;
	res.lo = isunordered(x.lo,y.lo);
	res.hi = isunordered(x.hi,y.hi);
	//convert to inetegr
	return res;	
}
#endif

int __attribute__((overloadable)) isunordered(double x, double y)
{
	return ((x!=x) || (y!=y));
}

_2i64 __attribute__((overloadable)) isunordered(double2 x, double2 y)
{
	double2 res = _mm_cmpunord_pd(x,y);
	//convert to inetegr
	return  _mm_castpd_si128(res);	
}

#if defined (__AVX__)
_4i64 __attribute__((overloadable)) isunordered (double4 x, double4 y)
{
	return  (_4i64)_mm256_cmp_pd(x, y,_CMP_UNORD_S);
}
#else
_4i64  __attribute__((overloadable)) isunordered(double4 x, double4 y)
{
	_4i64 res;
	res.lo = isunordered(x.lo,y.lo);
	res.hi = isunordered(x.hi,y.hi);
	return res;	
}
#endif

//Test for sign bit. The scalar version of the function
//returns a 1 if the sign bit in the float is set else returns 0.
//The vector version of the function returns the following
//for each component in floatn: a -1 if the sign bit in the
//float is set else returns 0
int  __attribute__((overloadable)) signbit(float x)
{
	return (*((int*)&x) & 0x80000000) != 0;
}

_4i32  __attribute__((overloadable)) signbit(float4 x)
{
	__m128i mask = _mm_load_si128((__m128i*)fsign_mask);
	__m128i signs = _mm_and_si128(_mm_castps_si128(x), mask);
	return (_4i32) _mm_cmpeq_epi32(signs, mask);
}
#if defined (__AVX__)
_8i32  __attribute__((overloadable)) signbit(float8 x)
{
	x = _mm256_and_ps(x, *(__m256*)fsign_mask8);
	return (_8i32)_mm256_blendv_ps(x, *(__m256*)FFFMask8, x);
}
#else
_8i32  __attribute__((overloadable)) signbit(float8 x)
{
	_8i32 res;
	res.lo = signbit(x.lo);
	res.hi = signbit(x.hi);
	//convert to inetegr
	return res;	
}
#endif

int __attribute__((overloadable)) signbit(double x)
{
	return (*((_1i64*)&x) & 0x8000000000000000) != 0;
}

_2i64 __attribute__((overloadable)) signbit(double2 x)
{
	__m128d dsign = _mm_and_pd(x, *(__m128d*)dsign_mask);
	__m128i sign = _mm_castpd_si128(_mm_and_pd(x, *(__m128d*)dsign_mask));
	__m128i res = _mm_cmpeq_epi32(sign, *(__m128i*)dsign_mask);
	res = _mm_shuffle_epi32(res, 0b11110101);
	return res;
}

#if defined (__AVX__)
_4i64  __attribute__((overloadable)) signbit(double4 x)
{
	x = _mm256_and_pd(x, *(__m256d*)d4sign_mask);
	return (_4i64)_mm256_blendv_pd(x, *(__m256d*)FF4, x);
}
#else
_4i64  __attribute__((overloadable)) signbit(double4 x)
{
	_4i64 res;
	res.lo = signbit(x.lo);
	res.hi = signbit(x.hi);
	//convert to inetegr
	return res;	
}
#endif


//////////////////////////////////////////////////////////////
// int any(igentype)
//Returns 1 if the most significant bit in any component of
//x is set; otherwise returns 0.
// char/uchar functions


int  __attribute__((overloadable)) any(_16i8 x)
{
	int mask = _mm_movemask_epi8((__m128i)x);
	return (mask & 0xFFFF) != 0;
}
int  __attribute__((overloadable)) any(_1i8 x)
{
	return (x & 0x80) != 0;
}

// short/ushort functions
int  __attribute__((overloadable)) any(_8i16 x)
{
	int mask = _mm_movemask_epi8((__m128i)x);
	return (mask & 0xAAAA) != 0;
}

int  __attribute__((overloadable)) any(_1i16 x)
{
	return (x & 0x8000) != 0;
}

int  __attribute__((overloadable)) any(_4i32 x)
{
	int mask = _mm_movemask_epi8((__m128i)x);
	return (mask & 0x8888) != 0;
}

#if defined (__AVX__)
_1i32 __attribute__((overloadable)) any(_8i32 x)
{
	int mask = _mm256_movemask_ps((__m256)x);
	return (mask & 0xFF) != 0;
}
#else
_1i32 __attribute__((overloadable)) any(_8i32 x)
{
	_1i32 res1, res2;
	res1 = any(x.lo);
	res2 = any(x.hi);
	return res1 || res2;
}
#endif


int  __attribute__((overloadable)) any(int x)
{
	return (x & 0x80000000L) != 0;
}

int  __attribute__((overloadable)) any(_2i64 x)
{
	int mask = _mm_movemask_epi8((__m128i)x);
	return (mask & 0x8080) != 0;
}

int  __attribute__((overloadable)) any(_1i64 x)
{	 
	return (x & 0x8000000000000000L) != 0;
}

#if defined (__AVX__)
_1i32 __attribute__((overloadable)) any(_4i64 x)
{
	int mask = _mm256_movemask_ps((__m256)x);
	return (mask & 0xAA) != 0;
}
#else
_1i32 __attribute__((overloadable)) any(_4i64 x)
{
	_1i32 res1, res2;
	res1 = any(x.lo);
	res2 = any(x.hi);
	return res1 || res2;
}
#endif


///////////////////////////////////////////////////////////
// int all(igentype)
//Returns 1 if the most significant bit in all components of
//x is set; otherwise returns 0.
// char/uchar functions
int  __attribute__((overloadable)) all(_16i8 x)
{
	int mask = _mm_movemask_epi8((__m128i)x);
	return mask == 0xFFFF;
}

int  __attribute__((overloadable)) all(_1i8 x)
{
	return (x & 0x80) != 0;
}

int  __attribute__((overloadable)) all(_8i16 x)
{
	int mask = _mm_movemask_epi8((__m128i)x);
	return ((mask & 0xAAAA) == 0xAAAA);

}

int  __attribute__((overloadable)) all(_1i16 x)
{
	return (x & 0x8000) != 0;
}

int  __attribute__((overloadable)) all(_4i32 x)
{
	int mask = _mm_movemask_epi8((__m128i)x);
	return ((mask & 0x8888) == 0x8888);
}
#if defined (__AVX__)
_1i32 __attribute__((overloadable)) all(_8i32 x)
{
	int mask = _mm256_movemask_ps((__m256)x);
	return (mask & 0xFF) == 0xFF;
}
#else
_1i32 __attribute__((overloadable)) all(_8i32 x)
{
	_1i32 res1, res2;
	res1 = all(x.lo);
	res2 = all(x.hi);
	return res1 && res2;
}
#endif

int  __attribute__((overloadable)) all(int x)
{
	return (x & 0x80000000L) != 0;
}

int  __attribute__((overloadable)) all(_2i64 x)
{
	int mask = _mm_movemask_epi8((__m128i)x);
	return ((mask & 0x8080) == 0x8080);
}
int  __attribute__((overloadable)) all(_1i64 x)
{	
	return (x & 0x8000000000000000L) != 0;
}

#if defined (__AVX__)
_1i32 __attribute__((overloadable)) all(_4i64 x)
{
	int mask = _mm256_movemask_ps((__m256)x);
	return (mask & 0xAA) == 0xAA;
}
#else
_1i32 __attribute__((overloadable)) all(_4i64 x)
{
	_1i32 res1, res2;
	res1 = all(x.lo);
	res2 = all(x.hi);
	return res1 && res2;
}
#endif

//Each bit of the result is the corresponding bit of a if the
//corresponding bit of c is 0. Otherwise it is the
//corresponding bit of b.

float4  __attribute__((overloadable)) bitselect(float4 a, float4 b, float4 c)
{
	float4 res1 = _mm_andnot_ps( c, a); //res1 is a if c is 0
	float4 res2 = _mm_and_ps( c, b); //res2 bit is b if c is 1
	return _mm_or_ps(res1, res2);
}

#if defined (__AVX__)
float8  __attribute__((overloadable)) bitselect(float8 a, float8 b, float8 c)
{
	a = _mm256_andnot_ps( c, a); //res1 is a if c is 0
	b = _mm256_and_ps( c, b); //res2 bit is b if c is 1
	return _mm256_or_ps(a, b);
}
#else
float8  __attribute__((overloadable)) bitselect(float8 a, float8 b, float8 c)
{
	float8 res;
	res.lo = bitselect(a.lo, b.lo, c.lo);
	res.hi = bitselect(a.hi, b.hi, c.hi);
	return res;
}
#endif

float   __attribute__((overloadable)) bitselect(float a, float b, float c)
{
	__m128 inA = _mm_load_ss(&a);
	__m128 inB = _mm_load_ss(&b);
	__m128 inC = _mm_load_ss(&c);

	inB = _mm_and_ps(inB, inC);
	inC = _mm_andnot_ps(inC, inA);
	inC = _mm_or_ps(inC, inB);

	float res;
	_mm_store_ss(&res, inC);
	return res;
}

double  __attribute__((overloadable)) bitselect(double a, double b, double c)
{
	__m128d inA = _mm_load_sd(&a);
	__m128d inB = _mm_load_sd(&b);
	__m128d inC = _mm_load_sd(&c);

	inB = _mm_and_pd(inB, inC);
	inC = _mm_andnot_pd(inC, inA);
	inC = _mm_or_pd(inC, inB);

	double res;
	_mm_store_sd(&res, inC);
	return res;
}

double2 __attribute__((overloadable)) bitselect(double2 a, double2 b, double2 c)
{
	double2 res1 = _mm_andnot_pd( c, a); //res1 is a if c is 0
	double2 res2 = _mm_and_pd( c, b); //res2 bit is b if c is 1
	return _mm_or_pd(res1, res2);
}
#if defined (__AVX__)
double4 __attribute__((overloadable)) bitselect(double4 a, double4 b, double4 c)
{
	a = _mm256_andnot_pd( c, a); //res1 is a if c is 0
	b = _mm256_and_pd( c, b); //res2 bit is b if c is 1
	return (double4) _mm256_or_pd(a, b);
}
#else
double4 __attribute__((overloadable)) bitselect(double4 a, double4 b, double4 c)
{
	double4 res;
	res.lo = bitselect(a.lo, b.lo, c.lo);
	res.hi = bitselect(a.hi, b.hi, c.hi);
	return res;
}
#endif

_1i32   __attribute__((overloadable)) bitselect(_1i32 a, _1i32 b, _1i32 c)
{
	return a & ~c | b & c;
}

_4i32  __attribute__((overloadable)) bitselect(_4i32 a, _4i32 b, _4i32 c)
{
	__m128i res1 = _mm_andnot_si128((__m128i) c,(__m128i) a);
	__m128i res2 = _mm_and_si128((__m128i) c,(__m128i) b);
	return (_4i32)_mm_or_si128(res1, res2);
}

#if defined (__AVX__)
_8i32  __attribute__((overloadable)) bitselect(_8i32 a, _8i32 b, _8i32 c)
{
	__m256 res1 = _mm256_andnot_ps((__m256) c,(__m256) a);
	__m256 res2 = _mm256_and_ps((__m256) c,(__m256) b);
	return (_8i32)_mm256_or_ps(res1,res2);
}
#else
_8i32  __attribute__((overloadable)) bitselect(_8i32 a, _8i32 b, _8i32 c)
{
	_8i32 res;
	res.lo = bitselect(a.lo, b.lo, c.lo);
	res.hi = bitselect(a.hi, b.hi, c.hi);
	return res;
}
#endif

_1i8   __attribute__((overloadable)) bitselect(_1i8 a, _1i8 b, _1i8 c)
{
	return a & ~c | b & c;
}

_16i8   __attribute__((overloadable)) bitselect(_16i8 a, _16i8 b, _16i8 c)
{
	a = (_16i8) _mm_andnot_si128((__m128i) c,(__m128i) a);
	b = (_16i8) _mm_and_si128((__m128i) c,(__m128i) b);
	return (_16i8) _mm_or_si128((__m128i)a,(__m128i) b);
}

_1i16   __attribute__((overloadable)) bitselect(_1i16 a, _1i16 b, _1i16 c)
{
	return a & ~c | b & c;
}

_8i16   __attribute__((overloadable)) bitselect(_8i16 a, _8i16 b, _8i16 c)
{
	a = (_8i16) _mm_andnot_si128((__m128i) c,(__m128i) a);
	b = (_8i16) _mm_and_si128((__m128i) c,(__m128i) b);
	return (_8i16) _mm_or_si128((__m128i)a,(__m128i) b);
}


#if defined (__AVX__)
_16i16   __attribute__((overloadable)) bitselect(_16i16 a, _16i16 b, _16i16 c)
{
	__m256 res1 = _mm256_andnot_ps((__m256) c,(__m256) a);
	__m256 res2 = _mm256_and_ps((__m256) c,(__m256) b);
	__m256i res3 = _mm256_castps_si256(_mm256_or_ps(res1,res2));
	return (_16i16)res3 ;
}
#else
_16i16 __attribute__((overloadable)) bitselect(_16i16 a, _16i16 b, _16i16 c)
{
	_16i16 res;
	res.lo = bitselect(a.lo, b.lo, c.lo);
	res.hi = bitselect(a.hi, b.hi, c.hi);
	return res;
}
#endif

_1i64   __attribute__((overloadable)) bitselect(_1i64 a, _1i64 b, _1i64 c)
{
	return a & ~c | b & c;
}
_2i64   __attribute__((overloadable)) bitselect(_2i64 a, _2i64 b, _2i64 c)
{
	return (_2i64)bitselect((_4i32)a, (_4i32)b, (_4i32)c);
}


#if defined (__AVX__)
_4i64   __attribute__((overloadable)) bitselect(_4i64 a, _4i64 b, _4i64 c)
{
	return (_4i64)bitselect((_8i32)a, (_8i32)b, (_8i32)c);
}
#else
_4i64   __attribute__((overloadable)) bitselect(_4i64 a, _4i64 b, _4i64 c)
{
	_4i64 res;
	res.lo = bitselect(a.lo, b.lo, c.lo);
	res.hi = bitselect(a.hi, b.hi, c.hi);
	return res;
}
#endif

_1u32   __attribute__((overloadable)) bitselect(_1u32 a, _1u32 b, _1u32 c)
{
	return a & ~c | b & c;
}

_4u32  __attribute__((overloadable)) bitselect(_4u32 a, _4u32 b, _4u32 c)
{
	__m128i res1 = _mm_andnot_si128((__m128i) c,(__m128i) a);
	__m128i res2 = _mm_and_si128((__m128i) c,(__m128i) b);
	return (_4u32)_mm_or_si128(res1, res2);
}

_1u8   __attribute__((overloadable)) bitselect(_1u8 a, _1u8 b, _1u8 c)
{
	return a & ~c | b & c;
}

_16u8   __attribute__((overloadable)) bitselect(_16u8 a, _16u8 b, _16u8 c)
{
	a = (_16u8) _mm_andnot_si128((__m128i) c,(__m128i) a);
	b = (_16u8) _mm_and_si128((__m128i) c,(__m128i) b);
	return (_16u8) _mm_or_si128((__m128i)a,(__m128i) b);
}

#if defined (__AVX__)
_16u16   __attribute__((overloadable)) bitselect(_16u16 a, _16u16 b, _16u16 c)
{
	__m256 res1 = _mm256_andnot_ps((__m256) c,(__m256) a);
	__m256 res2 = _mm256_and_ps((__m256) c,(__m256) b);
	return (_16u16)_mm256_or_ps(res1,res2);
}
#else
_16u16 __attribute__((overloadable)) bitselect(_16u16 a, _16u16 b, _16u16 c)
{
	_16u16 res;
	res.lo = bitselect(a.lo, b.lo, c.lo);
	res.hi = bitselect(a.hi, b.hi, c.hi);
	return res;
}
#endif


_1u16   __attribute__((overloadable)) bitselect(_1u16 a, _1u16 b, _1u16 c)
{
	return a & ~c | b & c;
}

_8u16   __attribute__((overloadable)) bitselect(_8u16 a, _8u16 b, _8u16 c)
{
	a = (_8u16) _mm_andnot_si128((__m128i) c,(__m128i) a);
	b = (_8u16) _mm_and_si128((__m128i) c,(__m128i) b);
	return (_8u16) _mm_or_si128((__m128i)a,(__m128i) b);
}

#if defined (__AVX__)
_8u32  __attribute__((overloadable)) bitselect(_8u32 a, _8u32 b, _8u32 c)
{
	a = _mm256_andnot_ps((__m256) c,(__m256) a);
	b = _mm256_and_ps((__m256) c,(__m256) b);
	return (_8u32)_mm256_or_ps(a, b);
}
#else
_8u32  __attribute__((overloadable)) bitselect(_8u32 a, _8u32 b, _8u32 c)
{
	_8u32 res;
	res.lo = bitselect(a.lo, b.lo, c.lo);
	res.hi = bitselect(a.hi, b.hi, c.hi);
	return res;
}
#endif


_1u64   __attribute__((overloadable)) bitselect(_1u64 a, _1u64 b, _1u64 c)
{
	return a & ~c | b & c;
}
_2u64   __attribute__((overloadable)) bitselect(_2u64 a, _2u64 b, _2u64 c)
{
	return (_2u64)bitselect((_4u32)a, (_4u32)b, (_4u32)c);
}

#if defined (__AVX__)
_4u64   __attribute__((overloadable)) bitselect(_4u64 a, _4u64 b, _4u64 c)
{
	return (_4u64)bitselect((_8u32)a, (_8u32)b, (_8u32)c);
}
#else
_4u64   __attribute__((overloadable)) bitselect(_4u64 a, _4u64 b, _4u64 c)
{
	_4u64 res;
	res.lo = bitselect(a.lo, b.lo, c.lo);
	res.hi = bitselect(a.hi, b.hi, c.hi);
	return res;
}
#endif

//For each component of a vector type,
//result[i] = if MSB of c[i] is set ? b[i] : a[i].
//For a scalar type, result = c ? b : a.
_1i8  __attribute__((overloadable)) select(_1i8 a, _1i8 b, _1i8 c)
{
	return c ? b : a;
}

_16i8  __attribute__((overloadable)) select(_16i8 a, _16i8 b, _16i8 c)
{
#ifdef __SSE4_1__
	return (_16i8) _mm_blendv_epi8((__m128i)a,(__m128i)b,(__m128i)c);
#else
	__m128i zero = _mm_setzero_si128();
	__m128i mask = _mm_cmpgt_epi8(zero, c);
	__m128i res = _mm_and_si128(mask, b);
	return _mm_or_si128(_mm_andnot_si128(mask, a), res);
#endif
}

_1i16  __attribute__((overloadable)) select(_1i16 a, _1i16 b, _1i16 c)
{
	return c ? b : a;
}

_8i16  __attribute__((overloadable)) select(_8i16  a, _8i16  b, _8i16 c)
{
	__m128i zero = _mm_setzero_si128();
	__m128i mask = _mm_cmpgt_epi16((__m128i)zero,(__m128i) c);
	__m128i res = _mm_and_si128((__m128i)mask,(__m128i) b);
	return (_8i16) _mm_or_si128(_mm_andnot_si128((__m128i)mask, (__m128i)a),(__m128i) res);
}

_1i32  __attribute__((overloadable)) select(_1i32 a, _1i32 b, _1i32 c)
{
	return c ? b : a;
}

_4i32    __attribute__((overloadable)) select(_4i32 a, _4i32 b, _4i32 c)
{
	__m128i zero = _mm_setzero_si128();
	__m128i mask = _mm_cmpgt_epi32((__m128i)zero,(__m128i) c);
	__m128i res = _mm_and_si128((__m128i)mask,(__m128i) b);
	return (_4i32) _mm_or_si128(_mm_andnot_si128((__m128i)mask, (__m128i)a),(__m128i)(__m128i) res);
}

#if defined (__AVX__)
_8i32    __attribute__((overloadable)) select(_8i32 a, _8i32 b, _8i32 c)
{
	return (_8i32)_mm256_blendv_ps((__m256)a, (__m256)b, (__m256)c); 
}
#else
_8i32    __attribute__((overloadable)) select(_8i32 a, _8i32 b, _8i32 c)
{
	_8i32 res;
	res.lo = select(a.lo, b.lo, c.lo);
	res.hi = select(a.hi, b.hi, c.hi);
	return res;
}
#endif

_1i64  __attribute__((overloadable)) select(_1i64 a, _1i64 b, _1i64 c)
{
	return c ? b : a;
}
_2i64   __attribute__((overloadable)) select(_2i64 a, _2i64 b, _2i64 c)
{
	__m128i zero = _mm_setzero_si128();
	__m128i mask = _mm_cmpgt_epi32((__m128i)zero,(__m128i) c);
	mask = _mm_shuffle_epi32 (mask, 0xF5);
	__m128i res = _mm_and_si128((__m128i)mask,(__m128i) b);
	return (_2i64) _mm_or_si128(_mm_andnot_si128((__m128i)mask, (__m128i)a),(__m128i)(__m128i) res);

}

#if defined (__AVX__)
_4i64   __attribute__((overloadable)) select(_4i64 a, _4i64 b, _4i64 c)
{
	return (_4i64)_mm256_blendv_pd((__m256d)a, (__m256d)b, (__m256d)c); 
}
#else
_4i64   __attribute__((overloadable)) select(_4i64 a, _4i64 b, _4i64 c)
{
	_4i64 res;
	res.lo = select(a.lo, b.lo, c.lo);
	res.hi = select(a.hi, b.hi, c.hi);
	return res;
}
#endif

_1i8  __attribute__((overloadable)) select(_1i8 a, _1i8 b, _1u8 c)
{
	return c ? b : a;
}

_16i8  __attribute__((overloadable)) select(_16i8 a, _16i8 b, _16u8 c)
{
#ifdef __SSE4_1__
	return (_16i8) _mm_blendv_epi8((__m128i)a,(__m128i)b,(__m128i)c);
#else
	__m128i zero = _mm_setzero_si128();
	__m128i mask = _mm_cmpgt_epi8(zero, c);
	__m128i res = _mm_and_si128(mask, b);
	return _mm_or_si128(_mm_andnot_si128(mask, a), res);
#endif
}
_1i16  __attribute__((overloadable)) select(_1i16 a, _1i16 b, _1u16 c)
{
	return c ? b : a;
}

_8i16  __attribute__((overloadable)) select(_8i16  a, _8i16  b, _8u16 c)
{
	__m128i zero = _mm_setzero_si128();
	__m128i mask = _mm_cmpgt_epi16((__m128i)zero,(__m128i) c);
	__m128i res = _mm_and_si128((__m128i)mask,(__m128i) b);
	return (_8i16) _mm_or_si128(_mm_andnot_si128((__m128i)mask, (__m128i)a),(__m128i)(__m128i) res);
}

_1i32  __attribute__((overloadable)) select(_1i32 a, _1i32 b, _1u32 c)
{
	return c ? b : a;
}

_4i32    __attribute__((overloadable)) select(_4i32 a, _4i32 b, _4u32 c)
{
	__m128i zero = _mm_setzero_si128();
	__m128i mask = _mm_cmpgt_epi32((__m128i)zero,(__m128i) c);
	__m128i res = _mm_and_si128((__m128i)mask,(__m128i) b);
	return (_4i32) _mm_or_si128(_mm_andnot_si128((__m128i)mask, (__m128i)a),(__m128i)(__m128i) res);
}

#if defined (__AVX__)
_8i32    __attribute__((overloadable)) select(_8i32 a, _8i32 b, _8u32 c)
{
	return (_8i32)_mm256_blendv_ps((__m256)a, (__m256)b, (__m256)c); 
}
#else
_8i32    __attribute__((overloadable)) select(_8i32 a, _8i32 b, _8u32 c)
{
	_8i32 res;
	res.lo = select(a.lo, b.lo, c.lo);
	res.hi = select(a.hi, b.hi, c.hi);
	return res;
}
#endif

_1i64  __attribute__((overloadable)) select(_1i64 a, _1i64 b, _1u64 c)
{
	return c ? b : a;
}
_2i64   __attribute__((overloadable)) select(_2i64 a, _2i64 b, _2u64 c)
{
	__m128i zero = _mm_setzero_si128();
	__m128i mask = _mm_cmpgt_epi32((__m128i)zero,(__m128i) c);
	mask = _mm_shuffle_epi32 (mask, 0xF5);
	__m128i res = _mm_and_si128((__m128i)mask,(__m128i) b);
	return (_2i64) _mm_or_si128(_mm_andnot_si128((__m128i)mask, (__m128i)a),(__m128i)(__m128i) res);
}

#if defined (__AVX__)
_4i64   __attribute__((overloadable)) select(_4i64 a, _4i64 b, _4u64 c)
{
	return (_4i64)_mm256_blendv_pd((__m256d)a, (__m256d)b, (__m256d)c); 
}
#else
_4i64   __attribute__((overloadable)) select(_4i64 a, _4i64 b, _4u64 c)
{
	_4i64 res;
	res.lo = select(a.lo, b.lo, c.lo);
	res.hi = select(a.hi, b.hi, c.hi);
	return res;
}
#endif

_1u8  __attribute__((overloadable)) select(_1u8 a, _1u8 b, _1i8 c)
{
	return c ? b : a;
}

_16u8  __attribute__((overloadable)) select(_16u8 a, _16u8 b, _16i8 c)
{
#ifdef __SSE4_1__
	return (_16u8) _mm_blendv_epi8((__m128i)a,(__m128i)b,(__m128i)c);
#else
	__m128i zero = _mm_setzero_si128();
	__m128i mask = _mm_cmpgt_epi8(zero, c);
	__m128i res = _mm_and_si128(mask, b);
	return _mm_or_si128(_mm_andnot_si128(mask, a), res);
#endif
}
_1u16  __attribute__((overloadable)) select(_1u16 a, _1u16 b, _1i16 c)
{
	return c ? b : a;
}

_8u16  __attribute__((overloadable)) select(_8u16  a, _8u16  b, _8i16 c)
{
	__m128i zero = _mm_setzero_si128();
	__m128i mask = _mm_cmpgt_epi16((__m128i)zero,(__m128i) c);
	__m128i res = _mm_and_si128((__m128i)mask,(__m128i) b);
	return (_8u16) _mm_or_si128(_mm_andnot_si128((__m128i)mask, (__m128i)a),(__m128i)(__m128i) res);
}

_1u32  __attribute__((overloadable)) select(_1u32 a, _1u32 b, _1i32 c)
{
	return c ? b : a;
}

_4u32    __attribute__((overloadable)) select(_4u32 a, _4u32 b, _4i32 c)
{
	__m128i zero = _mm_setzero_si128();
	__m128i mask = _mm_cmpgt_epi32((__m128i)zero,(__m128i) c);
	__m128i res = _mm_and_si128((__m128i)mask,(__m128i) b);
	return (_4u32) _mm_or_si128(_mm_andnot_si128((__m128i)mask, (__m128i)a),(__m128i)(__m128i) res);
}

#if defined (__AVX__)
_8u32    __attribute__((overloadable)) select(_8u32 a, _8u32 b, _8i32 c)
{
	return (_8u32)_mm256_blendv_ps((__m256)a, (__m256)b, (__m256)c); 
}
#else
_8u32    __attribute__((overloadable)) select(_8u32 a, _8u32 b, _8i32 c)
{
	_8u32 res;
	res.lo = select(a.lo, b.lo, c.lo);
	res.hi = select(a.hi, b.hi, c.hi);
	return res;
}
#endif

_1u64  __attribute__((overloadable)) select(_1u64 a, _1u64 b, _1i64 c)
{
	return c ? b : a;
}
_2u64   __attribute__((overloadable)) select(_2u64 a, _2u64 b, _2i64 c)
{
	__m128i zero = _mm_setzero_si128();
	__m128i mask = _mm_cmpgt_epi32((__m128i)zero,(__m128i) c);
	mask = _mm_shuffle_epi32 (mask, 0xF5);
	__m128i res = _mm_and_si128((__m128i)mask,(__m128i) b);
	return (_2u64) _mm_or_si128(_mm_andnot_si128((__m128i)mask, (__m128i)a),(__m128i)(__m128i) res);
}

#if defined (__AVX__)
_4u64   __attribute__((overloadable)) select(_4u64 a, _4u64 b, _4i64 c)
{
	return (_4u64)_mm256_blendv_pd((__m256d)a, (__m256d)b, (__m256d)c); 
}
#else
_4u64   __attribute__((overloadable)) select(_4u64 a, _4u64 b, _4i64 c)
{
	_4u64 res;
	res.lo = select(a.lo, b.lo, c.lo);
	res.hi = select(a.hi, b.hi, c.hi);
	return res;
}
#endif

_1u8  __attribute__((overloadable)) select(_1u8 a, _1u8 b, _1u8 c)
{
	return c ? b : a;
}

_16u8  __attribute__((overloadable)) select(_16u8 a, _16u8 b, _16u8 c)
{
#ifdef __SSE4_1__
	return (_16u8) _mm_blendv_epi8((__m128i)a,(__m128i)b,(__m128i)c);
#else
	__m128i zero = _mm_setzero_si128();
	__m128i mask = _mm_cmpgt_epi8(zero, c);
	__m128i res = _mm_and_si128(mask, b);
	return _mm_or_si128(_mm_andnot_si128(mask, a), res);
#endif
}
_1u16  __attribute__((overloadable)) select(_1u16 a, _1u16 b, _1u16 c)
{
	return c ? b : a;
}

_8u16  __attribute__((overloadable)) select(_8u16  a, _8u16  b, _8u16 c)
{
	__m128i zero = _mm_setzero_si128();
	__m128i mask = _mm_cmpgt_epi16((__m128i)zero,(__m128i) c);
	__m128i res = _mm_and_si128((__m128i)mask,(__m128i) b);
	return (_8u16) _mm_or_si128(_mm_andnot_si128((__m128i)mask, (__m128i)a),(__m128i)(__m128i) res);
}

_1u32  __attribute__((overloadable)) select(_1u32 a, _1u32 b, _1u32 c)
{
	return c ? b : a;
}

_4u32    __attribute__((overloadable)) select(_4u32 a, _4u32 b, _4u32 c)
{
	__m128i zero = _mm_setzero_si128();
	__m128i mask = _mm_cmpgt_epi32((__m128i)zero,(__m128i) c);
	__m128i res = _mm_and_si128((__m128i)mask,(__m128i) b);
	return (_4u32) _mm_or_si128(_mm_andnot_si128((__m128i)mask, (__m128i)a),(__m128i)(__m128i) res);
}

#if defined (__AVX__)
_8u32    __attribute__((overloadable)) select(_8u32 a, _8u32 b, _8u32 c)
{
	return (_8u32)_mm256_blendv_ps((__m256)a, (__m256)b, (__m256)c); 
}
#else
_8u32    __attribute__((overloadable)) select(_8u32 a, _8u32 b, _8u32 c)
{
	_8u32 res;
	res.lo = select(a.lo, b.lo, c.lo);
	res.hi = select(a.hi, b.hi, c.hi);
	return res;
}
#endif

_1u64  __attribute__((overloadable)) select(_1u64 a, _1u64 b, _1u64 c)
{
	return c ? b : a;
}
_2u64   __attribute__((overloadable)) select(_2u64 a, _2u64 b, _2u64 c)
{
	__m128i zero = _mm_setzero_si128();
	__m128i mask = _mm_cmpgt_epi32((__m128i)zero,(__m128i) c);
	mask = _mm_shuffle_epi32 (mask, 0xF5);
	__m128i res = _mm_and_si128((__m128i)mask,(__m128i) b);
	return (_2u64) _mm_or_si128(_mm_andnot_si128((__m128i)mask, (__m128i)a),(__m128i)(__m128i) res);
}

#if defined (__AVX__)
_4u64   __attribute__((overloadable)) select(_4u64 a, _4u64 b, _4u64 c)
{
	return (_4u64)_mm256_blendv_pd((__m256d)a, (__m256d)b, (__m256d)c); 
}
#else
_4u64   __attribute__((overloadable)) select(_4u64 a, _4u64 b, _4u64 c)
{
	_4u64 res;
	res.lo = select(a.lo, b.lo, c.lo);
	res.hi = select(a.hi, b.hi, c.hi);
	return res;
}
#endif

float4  __attribute__((overloadable)) select(float4 a, float4 b, _4i32 c)
{
#ifdef __SSE4_1__
	return _mm_blendv_ps(a, b, _mm_castsi128_ps((__m128i)c));
#else
	__m128i mask = _mm_setzero_si128();
	mask = _mm_cmpgt_epi32(mask, c);
	b = _mm_and_ps(b, _mm_castsi128_ps(mask));
	mask = _mm_castps_si128(_mm_andnot_ps(_mm_castsi128_ps(mask), a));
	return _mm_or_ps(_mm_castsi128_ps(mask), b);
#endif
}
#if defined (__AVX__)
float8  __attribute__((overloadable)) select(float8 a, float8 b, _8i32 c)
{
	return (float8)_mm256_blendv_ps((__m256)a, (__m256)b, (__m256)c); 
}
#else
float8  __attribute__((overloadable)) select(float8 a, float8 b, _8i32 c)
{
	float8 res;
	res.lo = select(a.lo, b.lo, c.lo);
	res.hi = select(a.hi, b.hi, c.hi);
	return res;
}
#endif

float  __attribute__((overloadable)) select(float a, float b, int c)
{
	return c ? b : a;
}

float4  __attribute__((overloadable)) select(float4 a, float4 b, _4u32 c)
{
#ifdef __SSE4_1__
	return _mm_blendv_ps(a, b, _mm_castsi128_ps((__m128i)c));
#else
	__m128i mask = _mm_setzero_si128();
	mask = _mm_cmpgt_epi32(mask, c);
	b = _mm_and_ps(b, _mm_castsi128_ps(mask));
	mask = _mm_castps_si128(_mm_andnot_ps(_mm_castsi128_ps(mask), a));
	return _mm_or_ps(_mm_castsi128_ps(mask), b);
#endif
}

#if defined (__AVX__)
float8  __attribute__((overloadable)) select(float8 a, float8 b, _8u32 c)
{
	return (float8)_mm256_blendv_ps((__m256)a, (__m256)b, (__m256)c); 
}
#else
float8  __attribute__((overloadable)) select(float8 a, float8 b, _8u32 c)
{
	float8 res;
	res.lo = select(a.lo, b.lo, c.lo);
	res.hi = select(a.hi, b.hi, c.hi);
	return res;
}
#endif

float  __attribute__((overloadable)) select(float a, float b, unsigned int c)
{
	return c ? b : a;
}

#if defined (__AVX__)
double4 __attribute__((overloadable)) select(double4 a, double4 b, _4i64 c)
{
	return (double4)_mm256_blendv_pd((__m256d)a, (__m256d)b, (__m256d)c); 
}
#else
double4 __attribute__((overloadable)) select(double4 a, double4 b, _4i64 c)
{
	double4 res;
	res.lo = select(a.lo, b.lo, c.lo);
	res.hi = select(a.hi, b.hi, c.hi);
	return res;
}
#endif

double2 __attribute__((overloadable)) select(double2 a, double2 b, _2i64 c)
{
#ifdef __SSE4_1__
	return _mm_blendv_pd(a, b, _mm_castsi128_pd(c));
#else
	__m128i mask = _mm_setzero_si128();
	mask = _mm_cmpgt_epi32(mask, c);
	__m128 fMask = _mm_castsi128_ps(mask);
	fMask = _mm_movehdup_ps(fMask);
	__m128d dMask = _mm_castps_pd(fMask);
	b = _mm_and_pd(dMask, b);
	a = _mm_andnot_pd(dMask, a);
	return _mm_or_pd(a, b);
#endif
}

double __attribute__((overloadable)) select(double a, double b, _1i64 c)
{
	return c ? b : a;
}

#if defined (__AVX__)
double4 __attribute__((overloadable)) select(double4 a, double4 b, _4u64 c)
{
	return (double4)_mm256_blendv_pd((__m256d)a, (__m256d)b, (__m256d)c); 
}
#else
double4 __attribute__((overloadable)) select(double4 a, double4 b, _4u64 c)
{
	double4 res;
	res.lo = select(a.lo, b.lo, c.lo);
	res.hi = select(a.hi, b.hi, c.hi);
	return res;
}
#endif


double2 __attribute__((overloadable)) select(double2 a, double2 b, _2u64 c)
{
#ifdef __SSE4_1__
	return _mm_blendv_pd(a, b, _mm_castsi128_pd(c));
#else
	__m128i mask = _mm_setzero_si128();
	mask = _mm_cmpgt_epi32(mask, c);
	__m128 fMask = _mm_castsi128_ps(mask);
	fMask = _mm_movehdup_ps(fMask);
	__m128d dMask = _mm_castps_pd(fMask);
	b = _mm_and_pd(dMask, b);
	a = _mm_andnot_pd(dMask, a);
	return _mm_or_pd(a, b);
#endif
}

double __attribute__((overloadable)) select(double a, double b, _1u64 c)
{
	return c ? b : a;
}

#ifdef __cplusplus
}
#endif