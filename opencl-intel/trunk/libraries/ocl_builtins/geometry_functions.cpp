// Copyright (c) 2006-2007 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// lo PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
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

#ifdef __cplusplus
extern "C" {
#endif

#define ALIGN16 __attribute__((aligned(16)))
#include <intrin.h>
#include "cl_geom_declaration.h"

// helper function
const int ALIGN16 signMask[4]      = {0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF};

float4 __copysignf4(float4 p0, float4 p1)
{
	__m128i mask = _mm_load_si128((__m128i*)signMask);
	__m128i tmp0 = _mm_castps_si128(p0);
	__m128i tmp1 = _mm_castps_si128(p1);
	mask = _mm_andnot_si128(mask, tmp0);
	tmp1 = _mm_or_si128(tmp1, mask);
	return _mm_castsi128_ps(tmp1);
}

const double ALIGN16 exp600[2]       = {0x1.0p600, 0x1.0p600};
const double ALIGN16 expMinus600[2]	= {0x1.0p-600, 0x1.0p-600};
const double ALIGN16 exp700[2]       = {0x1.0p700, 0x1.0p700};
const double ALIGN16 expMinus700[2]  = {0x1.0p-700, 0x1.0p-700};
const double ALIGN16 expMinus512[2]	= {0x1.0p-512, 0x1.0p-512};
const double ALIGN16 expMinus512_2[2]= {0x1.0p-512 / 2, 0x1.0p-512 / 2};
const double ALIGN16 expMinus512_4[2]= {0x1.0p-512 / 4, 0x1.0p-512 / 4};

const float ALIGN16 oneStorage[4]      = {1.0f, 1.0f, 1.0f, 1.0f};
const float ALIGN16 infinityStorage[4] = {INFINITY, INFINITY, INFINITY, INFINITY};

float  __attribute__((overloadable)) dot(float x, float y)
{
	return x*y;
}

//recieves float 2 as float 4 (z and w are 0) for optimization purposes
float __dotf2as4(float4 x, float4 y)
{
	x = _mm_mul_ps(x, y);
	x = _mm_hadd_ps(x, x);
	return  _mm_cvtss_f32(x);
}

float  __attribute__((overloadable)) dot(float4 x, float4 y)
{
	x = _mm_mul_ps(x, y);
	x = _mm_hadd_ps(x, x);
	x = _mm_hadd_ps(x, x);
	return  _mm_cvtss_f32(x);
}

double  __attribute__((overloadable)) dot(double x, double y)
{
	return x*y;
}

double  __attribute__((overloadable)) dot(double2 x, double2 y)
{
	x = _mm_mul_pd(x, y);
	x = _mm_hadd_pd(x, x);
	double res;
	_mm_store_sd(&res, x);
	return res;
}

double  __attribute__((overloadable)) dot(double4 x, double4 y)
{
	__m128d resLo = _mm_mul_pd(x.lo, y.lo);
	__m128d resHi = _mm_mul_pd(x.hi, y.hi);
	resLo = _mm_add_pd(resLo, resHi);
	resLo = _mm_hadd_pd(resLo, resLo);
	double res;
	_mm_store_sd(&res, resLo);
	return res;
}

//Cross
//Returns the cross product of p0.xyz and p1.xyz. The
//w component of float4 result returned will be 0.0.
float4  __attribute__((overloadable)) cross(float4 p0, float4 p1)
{
	double4 t0, t1;
	double4 res;

	t0.lo = _mm_cvtps_pd(p0);
	p0 = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(p0), 8));
	t0.hi = _mm_cvtps_pd(p0);

	t1.lo = _mm_cvtps_pd(p1);
	p1 = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(p1), 8));
	t1.hi = _mm_cvtps_pd(p1);

	__m128d p0Lo = _mm_shuffle_pd(t0.lo, t0.hi, 0b01);
	__m128d p0Hi = _mm_shuffle_pd(t0.lo, t0.hi, 0b10);
	__m128d p1Lo = _mm_shuffle_pd(t1.lo, t1.hi, 0b01);
	__m128d p1Hi = _mm_shuffle_pd(t1.lo, t1.hi, 0b10);

	t0.lo = _mm_mul_pd(t0.lo, p1Lo);
	t0.hi = _mm_mul_pd(t0.hi, p1Hi);
	t1.lo = _mm_mul_pd(t1.lo, p0Lo);
	t1.hi = _mm_mul_pd(t1.hi, p0Hi);

	t0.lo = _mm_sub_pd(t0.lo, t1.lo);
	t0.hi = _mm_sub_pd(t0.hi, t1.hi);

	res.lo = _mm_shuffle_pd(t0.lo, t0.hi, 0b01);
	res.hi = _mm_shuffle_pd(t0.lo, t0.hi, 0b10);

	float4 resultLo;
	float4 resultHi;
	resultLo =  _mm_cvtpd_ps(res.lo);
	resultHi =  _mm_cvtpd_ps(res.hi);

	return _mm_movelh_ps (resultLo, resultHi);


	//__m128 p0Shuffed = _mm_shuffle_ps(p0, p0, 0b11001001);
	//__m128 p1Shuffed = _mm_shuffle_ps(p1, p1, 0b11001001);

	//p0 = _mm_mul_ps(p0, p1Shuffed);
	//p1 = _mm_mul_ps(p1, p0Shuffed);

	//__m128 res = _mm_sub_ps(p0, p1);

	//res = _mm_shuffle_ps(res, res, 0b11001001);

	//return res;
}

double4  __attribute__((overloadable)) cross(double4 p0, double4 p1)
{
	double4 res;

	__m128d p0Lo = _mm_shuffle_pd(p0.lo, p0.hi, 0b01);
	__m128d p0Hi = _mm_shuffle_pd(p0.lo, p0.hi, 0b10);
	__m128d p1Lo = _mm_shuffle_pd(p1.lo, p1.hi, 0b01);
	__m128d p1Hi = _mm_shuffle_pd(p1.lo, p1.hi, 0b10);

	p0.lo = _mm_mul_pd(p0.lo, p1Lo);
	p0.hi = _mm_mul_pd(p0.hi, p1Hi);
	p1.lo = _mm_mul_pd(p1.lo, p0Lo);
	p1.hi = _mm_mul_pd(p1.hi, p0Hi);

	p0.lo = _mm_sub_pd(p0.lo, p1.lo);
	p0.hi = _mm_sub_pd(p0.hi, p1.hi);

	res.lo = _mm_shuffle_pd(p0.lo, p0.hi, 0b01);
	res.hi = _mm_shuffle_pd(p0.lo, p0.hi, 0b10);

	return res;

}

//length
//Return the length of vector p, i.e.,
//sqrt( p.x2 + p.y 2 + …)
float  __attribute__((overloadable)) length(float p)
{
	return fabs(p);
}

//recieves float 2 as float 4 (z and w are 0) for optimization purposes
float __lengthf2as4 (float4 p)
{
	__m128d	temp = _mm_cvtps_pd(p);
	// Convert to double
	temp = _mm_mul_pd(temp, temp);	// ^2
	temp = _mm_hadd_pd(temp, temp);	// x^2+y^2
	temp = _mm_sqrt_pd(temp);

	__m128 res = _mm_cvtpd_ps(temp);

	return _mm_cvtss_f32(res);
}

float  __attribute__((overloadable)) length(float4 p)
{
	__m128d tmpl = _mm_cvtps_pd(p);
	p = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(p), 8));
	__m128d tmph = _mm_cvtps_pd(p);
	tmpl = _mm_mul_pd(tmpl, tmpl);
	tmph = _mm_mul_pd(tmph, tmph);
	tmpl = _mm_add_pd(tmph, tmpl);
	tmpl = _mm_hadd_pd(tmpl, tmpl);
	tmpl = _mm_sqrt_pd(tmpl);

	__m128 res = _mm_cvtpd_ps(tmpl);

	return _mm_cvtss_f32(res);
}

double  __attribute__((overloadable)) length(double p)
{
	_1i64 res = *(_1i64*)(&p);
	res = (res & 0x7FFFFFFFFFFFFFFF);
	return *(double*)(&res);
}

double  __attribute__((overloadable)) length(double2 p)
{
	double length;
	__m128d temp = _mm_mul_pd(p, p);
	temp = _mm_hadd_pd(temp, temp);
	
	_mm_store_sd(&length, temp);

	if( length == INFINITY )
	{
		temp = _mm_mul_pd(p, *((__m128d *)expMinus600));
		temp = _mm_mul_pd(temp, temp);
		temp = _mm_hadd_pd(temp, temp);
		temp = _mm_sqrt_pd(temp);
		temp = _mm_mul_pd(temp, *((__m128d *)exp600));
	}
	else if( length < 2 * DBL_MIN / DBL_EPSILON )
	{
		temp = _mm_mul_pd(p, *((__m128d *)exp700));
		temp = _mm_mul_pd(temp, temp);
		temp = _mm_hadd_pd(temp, temp);
		temp = _mm_sqrt_pd(temp);
		temp = _mm_mul_pd(temp, *((__m128d *)expMinus700));
	}
	else
	{
		temp = _mm_sqrt_pd(temp);
	}

	_mm_store_sd(&length, temp);

	return length;
}

double  __attribute__((overloadable)) length(double4 p)
{
	double length;

	__m128d tempLo = _mm_mul_pd(p.lo, p.lo);
	__m128d tempHi = _mm_mul_pd(p.hi, p.hi);
	tempLo = _mm_add_pd(tempLo, tempHi);
	tempLo = _mm_hadd_pd(tempLo, tempLo);

	_mm_store_sd(&length, tempLo);

	if( length == INFINITY )
	{
		tempLo = _mm_mul_pd(p.lo, *((__m128d *)expMinus600));
		tempHi = _mm_mul_pd(p.hi, *((__m128d *)expMinus600));

		tempLo = _mm_mul_pd(tempLo, tempLo);
		tempHi = _mm_mul_pd(tempHi, tempHi);
		tempLo = _mm_add_pd(tempLo, tempHi);
		tempLo = _mm_hadd_pd(tempLo, tempLo);
		tempLo = _mm_sqrt_pd(tempLo);

		tempLo = _mm_mul_pd(tempLo, *((__m128d *)exp600));
	}
	else if( length < 4 * DBL_MIN / DBL_EPSILON )
	{
		tempLo = _mm_mul_pd(p.lo, *((__m128d *)exp700));
		tempHi = _mm_mul_pd(p.hi, *((__m128d *)exp700));

		tempLo = _mm_mul_pd(tempLo, tempLo);
		tempHi = _mm_mul_pd(tempHi, tempHi);
		tempLo = _mm_add_pd(tempLo, tempHi);
		tempLo = _mm_hadd_pd(tempLo, tempLo);
		tempLo = _mm_sqrt_pd(tempLo);

		tempLo = _mm_mul_pd(tempLo, *((__m128d *)expMinus700));
	}
	else
	{
		tempLo = _mm_sqrt_pd(tempLo);
	}

	_mm_store_sd(&length, tempLo);

	return length;
}

//Returns the distance between p0 and p1. This is
//calculated as length(p0 – p1).
float  __attribute__((overloadable)) distance(float p0, float p1)
{
	float p = p0 - p1;
	return fabs(p);
}

//recieves float 2 as float 4 (z and w are 0) for optimization purposes
float __distancef2as4 (float4 p0, float4 p1)
{
	__m128 p = _mm_sub_ps(p0, p1);
	__m128d	temp = _mm_cvtps_pd(p);
	// Convert to double
	temp = _mm_mul_pd(temp, temp);	// ^2
	temp = _mm_hadd_pd(temp, temp);	// x^2+y^2
	temp = _mm_sqrt_pd(temp);

	__m128 res = _mm_cvtpd_ps(temp);

	return _mm_cvtss_f32(res);
}

float  __attribute__((overloadable)) distance(float4 p0, float4 p1)
{
	__m128 p = _mm_sub_ps(p0, p1);
	__m128d tmpl = _mm_cvtps_pd(p);
	p = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(p), 8));
	__m128d tmph = _mm_cvtps_pd(p);
	tmpl = _mm_mul_pd(tmpl, tmpl);
	tmph = _mm_mul_pd(tmph, tmph);
	tmpl = _mm_add_pd(tmph, tmpl);
	tmpl = _mm_hadd_pd(tmpl, tmpl);
	tmpl = _mm_sqrt_sd(tmpl, tmpl);

	__m128 res = _mm_cvtpd_ps(tmpl);

	return _mm_cvtss_f32(res);
}

double  __attribute__((overloadable)) distance(double p0, double p1)
{
	double p = p0 - p1;
	_1i64 res = *(_1i64*)(&p);
	res = (res & 0x7FFFFFFFFFFFFFFF);
	return *(double*)(&res);
}

double  __attribute__((overloadable)) distance(double2 p0, double2 p1)
{
	double length;
	__m128d p = _mm_sub_pd(p0, p1);
	__m128d temp = _mm_mul_pd(p, p);
	temp = _mm_hadd_pd(temp, temp);
	
	_mm_store_sd(&length, temp);

	if( length == INFINITY )
	{
		temp = _mm_mul_pd(p, *((__m128d *)expMinus600));
		temp = _mm_mul_pd(temp, temp);
		temp = _mm_hadd_pd(temp, temp);
		temp = _mm_sqrt_pd(temp);
		temp = _mm_mul_pd(temp, *((__m128d *)exp600));
	}
	else if( length < 4 * DBL_MIN / DBL_EPSILON )
	{
		temp = _mm_mul_pd(p, *((__m128d *)exp700));
		temp = _mm_mul_pd(temp, temp);
		temp = _mm_hadd_pd(temp, temp);
		temp = _mm_sqrt_pd(temp);
		temp = _mm_mul_pd(temp, *((__m128d *)expMinus700));
	}
	else
	{
		temp = _mm_sqrt_pd(temp);
	}

	_mm_store_sd(&length, temp);

	return length;
}

double  __attribute__((overloadable)) distance(double4 p0, double4 p1)
{
	double length;
	double4 p;

	p.lo = _mm_sub_pd(p0.lo, p1.lo);
	p.hi = _mm_sub_pd(p0.hi, p1.hi);
	__m128d tempLo = _mm_mul_pd(p.lo, p.lo);
	__m128d tempHi = _mm_mul_pd(p.hi, p.hi);
	tempLo = _mm_add_pd(tempLo, tempHi);
	tempLo = _mm_hadd_pd(tempLo, tempLo);

	_mm_store_sd(&length, tempLo);

	if( length == INFINITY )
	{
		tempLo = _mm_mul_pd(p.lo, *((__m128d *)expMinus600));
		tempHi = _mm_mul_pd(p.hi, *((__m128d *)expMinus600));

		tempLo = _mm_mul_pd(tempLo, tempLo);
		tempHi = _mm_mul_pd(tempHi, tempHi);
		tempLo = _mm_add_pd(tempLo, tempHi);
		tempLo = _mm_hadd_pd(tempLo, tempLo);
		tempLo = _mm_sqrt_pd(tempLo);

		tempLo = _mm_mul_pd(tempLo, *((__m128d *)exp600));
	}
	else if( length < 4 * DBL_MIN / DBL_EPSILON )
	{
		tempLo = _mm_mul_pd(p.lo, *((__m128d *)exp700));
		tempHi = _mm_mul_pd(p.hi, *((__m128d *)exp700));

		tempLo = _mm_mul_pd(tempLo, tempLo);
		tempHi = _mm_mul_pd(tempHi, tempHi);
		tempLo = _mm_add_pd(tempLo, tempHi);
		tempLo = _mm_hadd_pd(tempLo, tempLo);
		tempLo = _mm_sqrt_pd(tempLo);

		tempLo = _mm_mul_pd(tempLo, *((__m128d *)expMinus700));
	}
	else
	{
		tempLo = _mm_sqrt_pd(tempLo);
	}

	_mm_store_sd(&length, tempLo);

	return length;
}

//normalize
//Returns lo vector in the same direction as p but with
//lo length of 1
float  __attribute__((overloadable)) normalize(float p)
{
	if( p == 0.f )
	{
		// Special edge case: copy vector over without change
		return p;
	}

	if( p != p)
	{
		//p is Nan: return p
		return p;
	}

	return (p > 0.f) ? 1.f : -1.f;
}

//recieves float 2 as float 4 (z and w are 0) for optimization purposes
float4 __normalizef2as4 (float4 p)
{
	double length;

	// Calculate vector length
	__m128d pl = _mm_cvtps_pd(p);
	__m128d tmpl = _mm_mul_pd(pl, pl);
	tmpl = _mm_hadd_pd(tmpl, tmpl);

	_mm_store_sd(&length, tmpl);

	if( length == 0.f )
	{
		// Special edge case: copy vector over without change
		return p;
	}

	// Deal with infinities
	if( length == INFINITY )
	{
		__m128 f4abs = fabs(p);
		// Compare with INFINITY
		__m128 f4InfMask = _mm_cmpeq_ps(f4abs, *((__m128 *)infinityStorage));		
		//__m128 f4cpsign = __svml_copysignf4(*((__m128 *)oneStorage), p);		
		__m128 f4cpsign = __copysignf4(*((__m128 *)oneStorage), p);		


		// Summarize infinities
		f4cpsign = _mm_and_ps(f4cpsign, f4InfMask);
		__m128d pl = _mm_cvtps_pd(f4cpsign);
		__m128d tmpl = _mm_mul_pd(pl, pl);
		tmpl = _mm_hadd_pd(tmpl, tmpl);
	}

	tmpl = _mm_sqrt_pd(tmpl);

	pl = _mm_div_pd(pl, tmpl);
	p = _mm_cvtpd_ps(pl);

	return p;
}

float4  __attribute__((overloadable)) normalize(float4 p)
{
	const float ALIGN16 oneStorage[4]      = {1.0f, 1.0f, 1.0f, 1.0f};
	const float ALIGN16 infinityStorage[4] = {(float)INFINITY, (float)INFINITY, (float)INFINITY, (float)INFINITY};

	double length;

	// Calculate vector length
	__m128d pl = _mm_cvtps_pd(p);
	__m128d ph = _mm_cvtps_pd(_mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(p), 8)));
	__m128d tmpl = _mm_mul_pd(pl, pl);
	__m128d tmph = _mm_mul_pd(ph, ph);
	tmpl = _mm_add_pd(tmph, tmpl);
	tmpl = _mm_hadd_pd(tmpl, tmpl);

	_mm_store_sd(&length, tmpl);

	if( length == 0.f )
	{
		// Special edge case: copy vector over without change
		return p;
	}

	   // Deal with infinities
	if( length == INFINITY )
	{
		__m128 f4abs = fabs(p);
		// Compare with INFINITY
		__m128 f4InfMask = _mm_cmpeq_ps(f4abs, *((__m128 *)infinityStorage));		
		//__m128 f4cpsign = __svml_copysignf4(*((__m128 *)oneStorage), p);		
		__m128 f4cpsign = __copysignf4(*((__m128 *)oneStorage), p);		


		// Summarize infinities
		f4cpsign = _mm_and_ps(f4cpsign, f4InfMask);
		tmpl = _mm_cvtps_pd(f4cpsign);
		tmph = _mm_cvtps_pd(_mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(f4cpsign), 8)));
		tmpl = _mm_mul_pd(tmpl, tmpl);
		tmph = _mm_mul_pd(tmph, tmph);
		tmpl = _mm_add_pd(tmph, tmpl);
		tmpl = _mm_hadd_pd(tmpl, tmpl);
	}

	tmpl = _mm_sqrt_pd(tmpl);

	pl = _mm_div_pd(pl, tmpl);
	ph = _mm_div_pd(ph, tmpl);
	p =  (__m128)_mm_unpacklo_epi64((__m128i)_mm_cvtpd_ps(pl), (__m128i)_mm_cvtpd_ps(ph));

	return p;
}

double  __attribute__((overloadable)) normalize(double p)
{
	if( p == (double)0.0 )
	{
		// Special edge case: copy vector over without change
		return p;
	}

	if( p != p)
	{
		//p is Nan: return p
		return p;
	}

	return (p > 0.f) ? 1.f : -1.f;
}

double2  __attribute__((overloadable)) normalize(double2 p)
{
	double length;

	double2 temp = _mm_mul_pd(p, p);
	temp = _mm_hadd_pd(temp, temp);
	_mm_store_sd(&length, temp);

	if( length == INFINITY )
	{
		p = _mm_mul_pd(p, *((__m128d *)expMinus512_2));

		temp = _mm_mul_pd(p, p);
		temp = _mm_hadd_pd(temp, temp);
	}
	else if( length < 2 * DBL_MIN / DBL_EPSILON )
	{
		p = _mm_mul_pd(p, *((__m128d *)exp700));
		temp = _mm_mul_pd(p, p);
		temp = _mm_hadd_pd(temp, temp);

		_mm_store_sd(&length, temp);

		if( length == 0 )
		{
			return p;
		}
	}

	temp = _mm_sqrt_pd(temp);

	p = _mm_div_pd(p, temp);

	return p;

}

double4  __attribute__((overloadable)) normalize(double4 p)
{
	double length;
	double4 temp;

	temp.lo = _mm_mul_pd(p.lo, p.lo);
	temp.hi = _mm_mul_pd(p.hi, p.hi);

	temp.lo = _mm_add_pd(temp.lo, temp.hi);
	temp.lo = _mm_hadd_pd(temp.lo, temp.lo);
	_mm_store_sd(&length, temp.lo);

	if( length == INFINITY )
	{
		p.lo = _mm_mul_pd(p.lo, *((__m128d *)expMinus512_4));
		p.hi = _mm_mul_pd(p.hi, *((__m128d *)expMinus512_4));
		temp.lo = _mm_mul_pd(p.lo, p.lo);
		temp.hi = _mm_mul_pd(p.hi, p.hi);
		temp.lo = _mm_add_pd(temp.lo, temp.hi);
		temp.lo = _mm_hadd_pd(temp.lo, temp.lo);
		temp.lo = _mm_sqrt_pd(temp.lo);
		_mm_store_sd(&length, temp.lo);
		temp.hi = _mm_div_pd(p.hi, temp.lo);
		temp.lo = _mm_div_pd(p.lo, temp.lo);
		return temp;
	}
	else if( length < 4 * DBL_MIN / DBL_EPSILON )
	{
		p.lo = _mm_mul_pd(p.lo, *((__m128d *)exp700));
		p.hi = _mm_mul_pd(p.hi, *((__m128d *)exp700));
		temp.lo = _mm_mul_pd(p.lo, p.lo);
		temp.hi = _mm_mul_pd(p.hi, p.hi);
		temp.lo = _mm_add_pd(temp.lo, temp.hi);
		temp.lo = _mm_hadd_pd(temp.lo, temp.lo);

		_mm_store_sd(&length, temp.lo);

		if( length == 0 )
		{
			return p;
		}

		temp.lo = _mm_sqrt_pd(temp.lo);
		temp.hi = _mm_div_pd(p.hi, temp.lo);
		temp.lo = _mm_div_pd(p.lo, temp.lo);
		return temp;
	}

	temp.lo = _mm_sqrt_pd(temp.lo);
	temp.hi = _mm_div_pd(p.hi, temp.lo);
	temp.lo = _mm_div_pd(p.lo, temp.lo);

	return temp;
}

//Returns the length of vector p computed as:
//half_sqrt(p.x^2 + p.y^2 + ….)
float  __attribute__((overloadable)) fast_length(float p)
{
	int res = *(int*)(&p);
	res = (res & 0x7FFFFFFF);
	return *(float*)(&res);
}

//recieves float 2 as float 4 (z and w are 0) for optimization purposes
float __fast_lengthf2as4 (float4 p)
{
	p = _mm_mul_ps(p, p);	// ^2
	p = _mm_hadd_ps(p, p);	// x^2+y^2
	p = _mm_sqrt_ps(p);

	return _mm_cvtss_f32(p);
}

float  __attribute__((overloadable)) fast_length(float4 p)
{
	p = _mm_mul_ps(p, p);	// ^2
	p = _mm_hadd_ps(p, p);	// x^2+y^2
	p = _mm_hadd_ps(p, p);	// x^2+y^2
	p = _mm_sqrt_ss(p);

	return _mm_cvtss_f32(p);
}

//__fast_distance
//Returns the distance between p0 and p1. This is
//calculated as length(p0 – p1).
float  __attribute__((overloadable)) fast_distance(float p0, float p1)
{
	float p = p0 - p1;
	return fabs(p);
}

//recieves float 2 as float 4 (z and w are 0) for optimization purposes
float __fast_distancef2as4 (float4 p0, float4 p1)
{
	__m128 p = _mm_sub_ps(p0, p1);
	p = _mm_mul_ps(p, p);	// ^2
	p = _mm_hadd_ps(p, p);	// x^2+y^2
	p = _mm_sqrt_ps(p);

	return _mm_cvtss_f32(p);
}

float  __attribute__((overloadable)) fast_distance(float4 p0, float4 p1)
{
	__m128 p = _mm_sub_ps(p0, p1);
	p = _mm_mul_ps(p, p);	// ^2
	p = _mm_hadd_ps(p, p);	// x^2+y^2
	p = _mm_hadd_ps(p, p);	// x^2+y^2
	p = _mm_sqrt_ss(p);

	return _mm_cvtss_f32(p);
}

//fast_normalize
//Returns lo vector in the same direction as p but with
//lo length of 1. fast_normalize is computed as:
//p * half_rsqrt (p.x2 + p.y2 + … )
//The result shall be within 8192 ulps error from the
//infinitely precise result of
//if (all(p == 0.0f))
//result = p;
//else
//result = p / sqrt(p.x2 + p.y2 + ... );
//with the following exceptions:
//1) If the sum of squares is greater than FLT_MAX
//then the value of the floating-point values in the
//result vector are undefined.
float  __attribute__((overloadable)) fast_normalize(float p)
{
	if( p == 0.f )
	{
		// Special edge case: copy vector over without change
		return p;
	}

	return (p > 0.f) ? 1.f : -1.f;
}

//recieves float 2 as float 4 (z and w are 0) for optimization purposes
float4 __fast_normalizef2as4 (float4 p)
{
	const float ALIGN16 oneStorage[4]      = {1.0f, 1.0f, 1.0f, 1.0f};
	const float ALIGN16 infinityStorage[4] = {(float)INFINITY, (float)INFINITY, (float)INFINITY, (float)INFINITY};

	float length;

	// Calculate vector length
	__m128 tmp = _mm_mul_ps(p, p);	// ^2
	tmp = _mm_hadd_ps(tmp, tmp);	// x^2+y^2
	tmp = _mm_moveldup_ps(tmp);

	length = _mm_cvtss_f32(tmp);

	if( length == 0.f )
	{
		// Special edge case: copy vector over without change
		return p;
	}

	// Deal with infinities
	if( length == INFINITY )
	{
		__m128 f4abs = fabs(p);
		// Compare with INFINITY
		__m128 f4InfMask = _mm_cmpeq_ps(f4abs, *((__m128 *)infinityStorage));		
		//__m128 f4cpsign = __svml_copysignf4(*((__m128 *)oneStorage), p);		
		__m128 f4cpsign = __copysignf4(*((__m128 *)oneStorage), p);


		// Summarize infinities
		f4cpsign = _mm_and_ps(f4cpsign, f4InfMask);
		tmp = _mm_mul_ps(f4cpsign, f4cpsign);
		tmp = _mm_hadd_ps(tmp, tmp);
	}

	tmp = _mm_sqrt_ps(tmp);

	p = _mm_div_ps(p, tmp);

	return p;
}

float4  __attribute__((overloadable)) fast_normalize(float4 p)
{
	const float ALIGN16 oneStorage[4]      = {1.0f, 1.0f, 1.0f, 1.0f};
	const float ALIGN16 infinityStorage[4] = {(float)INFINITY, (float)INFINITY, (float)INFINITY, (float)INFINITY};

	float length;

	// Calculate vector length
	__m128 tmp = _mm_mul_ps(p, p);	// ^2
	tmp = _mm_hadd_ps(tmp, tmp);	// x^2+y^2
	tmp = _mm_hadd_ps(tmp, tmp);	// x^2+y^2

	length = _mm_cvtss_f32(p);

	if( length == 0.f )
	{
		// Special edge case: copy vector over without change
		return p;
	}

	// Deal with infinities
	if( length == INFINITY )
	{
		__m128 f4abs = fabs(p);
		// Compare with INFINITY
		__m128 f4InfMask = _mm_cmpeq_ps(f4abs, *((__m128 *)infinityStorage));		
		//__m128 f4cpsign = __svml_copysignf4(*((__m128 *)oneStorage), p);		
		__m128 f4cpsign = __copysignf4(*((__m128 *)oneStorage), p);


		// Summarize infinities
		f4cpsign = _mm_and_ps(f4cpsign, f4InfMask);
		tmp = _mm_mul_ps(f4cpsign, f4cpsign);
		tmp = _mm_hadd_ps(tmp, tmp);
		tmp = _mm_hadd_ps(tmp, tmp);
	}

	tmp = _mm_sqrt_ps(tmp);

	p = _mm_div_ps(p, tmp);

	return p;
}

#ifdef __cplusplus
}
#endif
