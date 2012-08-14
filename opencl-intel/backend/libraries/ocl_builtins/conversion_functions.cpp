
#if !defined (__MIC__) && !defined(__MIC2__)

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__AVX2__)
 /*
  * AVX2 requires svml s9 for 32-bit dll
  * and                l9 for 64-bit dll
  *
  */
  #if defined(_WIN64) || defined(__x86_64__) || defined(__LP64__)
  #define CTYPE l9
  #else
  #define CTYPE s9
  #endif
#elif defined (__AVX__)
 /*
  * AVX requires svml g9 for 32-bit dll
  * and               e9 for 64-bit dll
  *
  */
  #if defined(_WIN64) || defined(__x86_64__) || defined(__LP64__)
  #define CTYPE e9
  #else
  #define CTYPE g9
  #endif
#elif defined (__SSE4_2__)
  #if defined(_WIN64) || defined(__x86_64__) || defined(__LP64__)
  #define CTYPE h8
  #else
  #define CTYPE n8
  #endif
#elif defined(__SSE4_1__)
  #if defined(_WIN64) || defined(__x86_64__) || defined(__LP64__)
  #define CTYPE y8
  #else
  #define CTYPE p8
  #endif
#elif defined(__SSSE3__)
  #if defined(_WIN64) || defined(__x86_64__) || defined(__LP64__)  
  #define CTYPE u8
  #else
  #define CTYPE v8
  #endif
#elif defined(__SSE3__)
  #if defined(_WIN64) || defined(__x86_64__) || defined(__LP64__)
  #define CTYPE e7
  #else
  #define CTYPE t7
  #endif
#else
  #error SSE artchitecture was not defined
#endif // __SSE4_2__


#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#include <intrin.h> 
#if defined(__AVX__)
#include <avxintrin.h>
#endif
#define ALIGN16 __attribute__((aligned(16)))
#define ALIGN32 __attribute__((aligned(16)))
#include "cl_types2.h"
#include "conversions_svml.inc"

// Load the declarations of our LL intrinsic library.
#include "ll_intrinsics.h"

#define _LONG_TO_INT   0x88
#define _ULLONG_MAX    0xFFFFFFFFFFFFFFFF       /* maximum unsigned long long int value */
#define _LLONG_MAX     0x7FFFFFFFFFFFFFFF       /* maximum signed long long int value */
#define _LLONG_MIN     0x8000000000000000       /* minimum signed long long int value */
#define _INT_MAX		  2147483647
#define _INT_MAX_AS_FLOAT		  2147483647.0f
#define _INT_MIN		  -2147483648
#define _UINT_MAX	  4294967295
#define _CHAR_MAX      127
#define _CHAR_MIN      -128
#define _UCHAR_MAX     255

	ALIGN16 char _8x16to8x8[]  = {0, 2, 4, 6, 8, 10, 12, 14, 0, 0, 0, 0, 0, 0, 0, 0};
	ALIGN16 char _4x32to4x16[] = {0,1, 4,5,  8,9, 12,13, 0, 0, 0, 0, 0,0,0,0};
	ALIGN16 char _4x32to4x8[]  = {0, 4, 8, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	ALIGN16 char _2x32to2x8[]  = {0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	ALIGN16 char _2x64to2x8[]  = {0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	ALIGN16 char _2x64to2x16[] = {0,1, 8,9,0,0, 0, 0, 0, 0, 0, 0,0,0,0,0};
	ALIGN16 _1i64 maxiChar64[] = {0x007F, 0x007F};
	ALIGN16 _1u64 maxuChar64[] = {0x00FF, 0x00FF};
	ALIGN16 _1i64 miniChar64[] = {0x080, 0x080};
	ALIGN16 _1u64 minuChar64[] = {0x0, 0x0};
	ALIGN16 char miniChar8[] = {0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80, 0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80};
	ALIGN16 unsigned char minuChar8[] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
	ALIGN16 char miniCharVal8[] = {0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F, 0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F};
	ALIGN16 unsigned char minuCharVal8[] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};

	ALIGN16 short maxiShort16[] = {0x7FFF,0x7FFF,0x7FFF,0x7FFF,0x7FFF,0x7FFF,0x7FFF,0x7FFF};
	ALIGN16 unsigned short maxuShort16[] = {0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF};
	ALIGN16 short miniShort16[] = {0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000};
	ALIGN16 unsigned short minuShort16[] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
	ALIGN16 int maxiInt32[] = {0x7FFFFFFF,0x7FFFFFFF,0x7FFFFFFF,0x7FFFFFFF};
	ALIGN16 unsigned int maxuInt32[] = {0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF};
	ALIGN16 int miniInt32[] = {0x80000000,0x80000000,0x80000000,0x80000000};
	ALIGN16 unsigned int minuInt32[] = {0x0,0x0,0x0,0x0};
	ALIGN16 _1i64 maxiLong64[] = {_LLONG_MAX,_LLONG_MAX};
	ALIGN16 _1u64 maxuLong64[] = {_ULLONG_MAX,_ULLONG_MAX};
	ALIGN16 _1i64 miniLong64[] = {_LLONG_MIN,_LLONG_MIN};
	ALIGN16 _1u64 minuLong64[] = {0x0,0x0};

	ALIGN16 _1i64 maxiShort64[] = {0x07FFF, 0x07FFF};
	ALIGN16 _1u64 maxuShort64[] = {0x0FFFF, 0x0FFFF};
	ALIGN16 _1i64 miniShort64[] = {0x08000, 0x08000};
	ALIGN16 _1u64 minuShort64[] = {0x0, 0x0};
	ALIGN16 _1i64 maxiInt64[] = {0x07FFFFFFF, 0x07FFFFFFF};
	ALIGN16 _1u64 maxuInt64[] = {0x0FFFFFFFF, 0x0FFFFFFFF};
	ALIGN16 _1i64 miniInt64[] = {0x080000000, 0x080000000};
	ALIGN16 _1u64 minuInt64[] = {0x0L, 0x0L};
	ALIGN16 short maxiChar16[] = {0x007F, 0x007F, 0x007F, 0x007F, 0x007F, 0x007F, 0x007F, 0x007F};
	ALIGN16 unsigned short maxuChar16[] = {0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF};
	ALIGN16 int maxiChar32[] = {0x007F, 0x007F, 0x007F, 0x007F};
	ALIGN16 unsigned int maxuChar32[] = {0x00FF, 0x00FF, 0x00FF, 0x00FF};
	ALIGN16 int maxiShort32[] = {0x7FFFL, 0x7FFF, 0x7FFF, 0x7FFF};
	ALIGN16 unsigned int maxuShort32[] = {0x0FFFF, 0x0FFFF, 0x0FFFF, 0x0FFFF};
	ALIGN16 int minInt32[] = {0xcf000000, 0xcf000000, 0xcf000000, 0xcf000000};
	ALIGN16 int maxInt32[] = {0x4f000000, 0x4f000000, 0x4f000000, 0x4f000000};
	ALIGN16 int minIntVal32[] = {0x80000000, 0x80000000, 0x80000000, 0x80000000};
	ALIGN16 int maxIntVal32[] = {0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF};

#ifndef __SSE4_1__
// This code is required for compilation on Atom processors
// which support SSSE3 only
__m128i _mm_cvtepi8_epi16(__m128i param)
{
	ALIGN16 short	a[8];
	ALIGN16 char	r[16];
	_mm_store_si128((__m128i*)a, param);
	for(int i=0; i<8; ++i)
	{
		r[2*i] = (char)a[i];
		r[2*i+1] = a[i] < 0 ? 0xFF : 0;
	}

	return _mm_load_si128((__m128i*)r);
}
__m128i _mm_cvtepu8_epi16(__m128i param)
{
	ALIGN16 short	a[8];
	ALIGN16 char	r[16];
	_mm_store_si128((__m128i*)a, param);
	for(int i=0; i<8; ++i)
	{
		r[2*i] = (char)a[i];
		r[2*i+1] = 0;
	}

	return _mm_load_si128((__m128i*)r);
}
__m128i _mm_cvtepi8_epi32(__m128i param)
{
	ALIGN16 int		a[4];
	ALIGN16 char	r[16];
	_mm_store_si128((__m128i*)a, param);
	for(int i=0; i<4; ++i)
	{
		r[4*i] = (char)a[i];
		r[4*i+1] = r[4*i+2] = r[4*i+3] = a[i] < 0 ? 0xFF : 0;
	}

	return _mm_load_si128((__m128i*)r);
}

__m128i _mm_cvtepi16_epi32(__m128i param)
{
	ALIGN16 int		a[4];
	ALIGN16 short	r[8];
	_mm_store_si128((__m128i*)a, param);
	for(int i=0; i<4; ++i)
	{
		r[2*i] = (short)a[i];
		r[2*i+1] = a[i] < 0 ? 0xFFFF : 0;
	}

	return _mm_load_si128((__m128i*)r);
}

__m128i _mm_cvtepu8_epi32(__m128i param)
{
	ALIGN16 int		a[4];
	ALIGN16 char	r[16];
	_mm_store_si128((__m128i*)a, param);
	for(int i=0; i<4; ++i)
	{
		r[4*i] = (char)a[i];
		r[4*i+1] = r[4*i+2] = r[4*i+3] = 0;
	}

	return _mm_load_si128((__m128i*)r);
}

__m128i _mm_cvtepu16_epi32(__m128i param)
{
	ALIGN16 int		a[4];
	ALIGN16 short	r[8];
	_mm_store_si128((__m128i*)a, param);
	for(int i=0; i<4; ++i)
	{
		r[2*i] = (short)a[i];
		r[2*i+1] = 0;
	}

	return _mm_load_si128((__m128i*)r);
}

__m128i _mm_cvtepi8_epi64(__m128i param)
{
	ALIGN16 long	a[2];
	ALIGN16 char	r[16];
	_mm_store_si128((__m128i*)a, param);
	for(int i=0; i<2; ++i)
	{
		r[8*i] = (char)a[i];
		r[8*i+1] = r[8*i+2] = r[8*i+3] = r[8*i+4] =
			r[8*i+5] = r[8*i+6] = r[8*i+7] = a[i] < 0 ? 0xFF : 0;
	}

	return _mm_load_si128((__m128i*)r);
}

__m128i _mm_cvtepi16_epi64(__m128i param)
{
	ALIGN16 long 	a[2];
	ALIGN16 short	r[8];
	_mm_store_si128((__m128i*)a, param);
	for(int i=0; i<2; ++i)
	{
		r[4*i] = (short)a[i];
		r[4*i+1] = r[4*i+2] = r[4*i+3] = a[i] < 0 ? 0xFFFF : 0;
	}

	return _mm_load_si128((__m128i*)r);
}

__m128i _mm_cvtepi32_epi64(__m128i param)
{
	ALIGN16 long	a[2];
	ALIGN16 int		r[4];
	_mm_store_si128((__m128i*)a, param);
	for(int i=0; i<2; ++i)
	{
		r[2*i] = (int)a[i];
		r[2*i+1] = a[i] < 0 ? 0xFFFFFFFF : 0;
	}

	return _mm_load_si128((__m128i*)r);
}

__m128i _mm_cvtepu8_epi64(__m128i param)
{
	ALIGN16 long	a[2];
	ALIGN16 char	r[16];
	_mm_store_si128((__m128i*)a, param);
	for(int i=0; i<2; ++i)
	{
		r[8*i] = (char)a[i];
		r[8*i+1] = r[8*i+2] = r[8*i+3] = r[8*i+4] =
			r[8*i+5] = r[8*i+6] = r[8*i+7] = 0;
	}

	return _mm_load_si128((__m128i*)r);
}

__m128i _mm_cvtepu16_epi64(__m128i param)
{
	ALIGN16 long 	a[2];
	ALIGN16 short	r[8];
	_mm_store_si128((__m128i*)a, param);
	for(int i=0; i<2; ++i)
	{
		r[4*i] = (short)a[i];
		r[4*i+1] = r[4*i+2] = r[4*i+3] = 0;
	}

	return _mm_load_si128((__m128i*)r);
}

__m128i _mm_cvtepu32_epi64(__m128i param)
{
	ALIGN16 long	a[2];
	ALIGN16 int		r[4];
	_mm_store_si128((__m128i*)a, param);
	for(int i=0; i<2; ++i)
	{
		r[2*i] = (int)a[i];
		r[2*i+1] = 0;
	}

	return _mm_load_si128((__m128i*)r);
}

__m128i _mm_packus_epi32( 
   __m128i a_x,
   __m128i b_x
   )
{
	ALIGN16 int			a[4];
	ALIGN16 int			b[4];
	ALIGN16 short		r[8];
	_mm_store_si128((__m128i*)a, a_x);
	_mm_store_si128((__m128i*)b, b_x);

	for(int i=0; i<4;++i)
	{
		r[i] = (a[i] < 0) ? 0 : ((a[i] > 0xffff) ? 0xffff : (short)a[i]);
		r[i+4] = (b[i] < 0) ? 0 : ((b[i] > 0xffff) ? 0xffff : (short)b[i]);
	}

	return _mm_load_si128((__m128i*)r);
}
#endif

#if defined(__AVX__)
	__m256i float8ToInt8(float8 param)
	{
		return _mm256_cvttps_epi32 (param);
	} 
#endif // defined(__AVX__)
        
    __m128i float4ToInt4(float4 param)
	{
		__m128i res = _mm_cvttps_epi32(param);
		return res;
	} 

	int floatToInt(float4 param)
	{
		int res = _mm_cvttss_si32(param);
		return res;
	}

	float doubleToFloatSat(double param)
	{
		if(param > 0x47efffffe0000000) return 0x7f7fffff;
		if(param < 0xc7efffffe0000000) return 0xff7fffff;
		if((param < 0x3810000000000000)&&(param > 0)) return 0x00800000;
		if((param > 0xb810000000000000)&&(param < 0)) return 0x80800000;
		return (float)param;
	}

    __m128i double2ToInt4(double2 x)
    {
        return _mm_cvttpd_epi32(x);
    }

#if defined(__AVX__)
	__m128i double4ToInt4(double4 param)
	{
		return _mm256_cvttpd_epi32 (param);
	} 
#endif // defined(__AVX__)

    void setRound(int newMode)
	{
		_mm_setcsr(newMode);
	}

	int getRound()
	{
		return _mm_getcsr();
	}
	_4i32 floatToIntSat(float4 param)
	{
		__m128i t1 = (__m128i)_mm_cmpge_ps(param, *((__m128 *)maxInt32) );
		__m128i t2 = (__m128i)_mm_cmpge_ps(*((__m128 *)minInt32), param );
		__m128i t = _mm_or_si128(t1, t2);
		t1 = _mm_and_si128(t1, *((__m128i *)maxIntVal32));
		t2 = _mm_and_si128(t2, *((__m128i *)minIntVal32));
		t = _mm_andnot_si128(t, float4ToInt4(param) );
		_4i32 res = as_int4(_mm_or_si128(t1, t2));
		res = as_int4(_mm_or_si128(__builtin_astype(res,__m128i), t));
		return res;
	}

	int doubleToIntSat(double param)
	{
		//__m128i t1 = (__m128i)_mm_cmpge_pd(param, *((__m128 *)maxInt64) );
		//__m128i t2 = (__m128i)_mm_cmpge_pd(*((__m128 *)minInt64), param );
		//__m128i t = _mm_or_si128(t1, t2);
		//t1 = _mm_and_si128(t1, *((__m128i *)maxIntVal64));
		//t2 = _mm_and_si128(t2, *((__m128i *)minIntVal64));
		//t = _mm_shuffle_epi32(t, _LONG_TO_INT);\
		//t = _mm_andnot_si128(t, double2ToInt4(param, rMode) );
		//_4i32 res = _mm_or_si128(t1, t2);
		//res = _mm_shuffle_epi32(res, _LONG_TO_INT);\
		//res = _mm_or_si128(res, t);

		if(param >= 2147483647.0) return 2147483647.0;
		if(param <= -2147483648.0) return -2147483648.0;
		double2 t;
		t.lo = param;
		_4i32 res = as_int4(double2ToInt4(t));
		return res.s0;
	}

	_1u32 floatToUint(float param)
    {
        if (param > 2147483647.0f)
            return (_1u32)param;
        float4 p;
        p.s0 = param;
        _1u32 res = _mm_cvttss_si32(p);
        return res;
    }

    // internal any() function. copy of OpenCL built in
    int  _any4(_4i32 x)
    {
	    int mask = _mm_movemask_epi8((__m128i)x);
	    return (mask & 0x8888) != 0;
    }
	
    _4u32 float4ToUint4(float4 param)
	{
        _4u32 res;
        const ALIGN16 float mth_INT_MAX_32f[4] = { 2147483647.0f, 2147483647.0f, 2147483647.0f, 2147483647.0f};
        _4i32 mask_gt= as_int4(_mm_cmpgt_ps(param,  *(__m128*)mth_INT_MAX_32f));
         
        if(_any4(mask_gt))
        {
             res.s0 = (_1u32)param.s0;
             res.s1 = (_1u32)param.s1;
             res.s2 = (_1u32)param.s2;
             res.s3 = (_1u32)param.s3;
        }
        else
        {
            res =  as_uint4(_mm_cvttps_epi32(param));
        }
        return res;
    }
#if defined(__AVX__)
    // internal any() function. copy of OpenCL built in
    int  _any8(_8i32 x)
    {
        int mask = _mm256_movemask_ps((__m256)x);
        return (mask & 0xFF) != 0;
    }

    _8u32 float8ToUint8(float8 param)
    {
        _8u32 res;
        const ALIGN32 float mth_INT_MAX_32f[8] = { 2147483647.0f, 2147483647.0f, 2147483647.0f, 2147483647.0f, 
                                                   2147483647.0f, 2147483647.0f, 2147483647.0f, 2147483647.0f};
        _8i32 mask_gt= (_8i32)_mm256_cmp_ps(param,  *(__m256*)mth_INT_MAX_32f, _CMP_GT_OS );
         
        if(_any8(mask_gt))
        {
             res.s0 = (_1u32)param.s0;
             res.s1 = (_1u32)param.s1;
             res.s2 = (_1u32)param.s2;
             res.s3 = (_1u32)param.s3;
             res.s4 = (_1u32)param.s4;
             res.s5 = (_1u32)param.s5;
             res.s6 = (_1u32)param.s6;
             res.s7 = (_1u32)param.s7;
        }
        else
        {
            res =   (_8u32)_mm256_cvttps_epi32(param);
        }
        return res;
    }
#endif // defined(__AVX__)

    _1u32 floatToUintSat(float param)
    {
        // maximum representable IEEE754 float value which is < UINT_MAX(4294967295) 
        // in Hex should be 0x4F7FFFFF
        const float maxUintAsFloat =  4294967040.0f;
        return (param < 0.0f) ? 0 : ((param > maxUintAsFloat) ? _UINT_MAX : (_1u32) param);
    }

	float4 intToFloat(_4i32 param)
	{
		return as_float4(_mm_cvtepi32_ps(__builtin_astype(param,__m128i)));
	}

#if defined(__AVX__)
    float8 intToFloat8(_8i32 param)
	{
        return (float8)_mm256_cvtepi32_ps((__m256i)param);
	}
#endif
/*
float <- uint
Bob Hanek's suggested method
The simplest approach would be to split the uint into two 16-bit (signed) integers
convert them both to floating point and combine with an fma. That would be ~5 instructions:

          lo <- in & 0xffff       hi <- in >> 16
          flo <- (float) lo        fhi <- (float) hi;
          res <- 2^16*fhi + flo
*/
	float uintToFloat(_1u32 x)
	{
		int tlo, thi;
		float flo, fhi;
		tlo = x & 0xFFFF;
		flo = convert_float(tlo);
		thi = x >> 16;
		fhi = convert_float(thi);
		return mad(65536.0f, fhi, flo);
	}
	
	float4 uintToFloat4(_4u32 x)
	{
		int4 tlo, thi;
		float4 flo, fhi;
		tlo = as_int4(x & 0xFFFF);
		flo = convert_float4(tlo);
		thi = as_int4(x >> 16);
		fhi = convert_float4(thi);
		return mad(65536.0f, fhi, flo);
	}

	float8 uintToFloat8(_8u32 x)
	{
		int8 tlo, thi;
		float8 flo, fhi;
		tlo = as_int8(x & 0xFFFF);
		flo = convert_float8(tlo);
		thi = as_int8(x >> 16);
		fhi = convert_float8(thi);
		return mad(65536.0f, fhi, flo);
	}



    float4 double2ToFloat4(double2 param, int dummy)
	{
        return _mm_cvtpd_ps(param);
	}

#if defined(__AVX__)
    float4 double4ToFloat4(double4 param, int dummy)
	{
        return _mm256_cvtpd_ps(param);
	}
#endif


    // all type to type including all rounding modes, NO SAT
#define DEF_INT_PROTO1_X_Y(TI, TO, TYPIN, TYPOUT, RMODE)\
	TO __attribute__((overloadable)) convert_##TYPOUT##RMODE(TI x)\
	{\
	return as_##TYPOUT(x);\
	}


	// 8 bits
#define DEF_INT_PROTO8_8(TI, TO, TINAME, TONAME, RMODE)\
	DEF_INT_PROTO1_X_Y(_1##TI##8, _1##TO##8, TINAME##char, TONAME##char, RMODE)\
	DEF_INT_PROTO1_X_Y(_2##TI##8, _2##TO##8, TINAME##char2, TONAME##char2, RMODE)\
	DEF_INT_PROTO1_X_Y(TINAME##char3, TONAME##char3, TINAME##char3, TONAME##char3, RMODE)\
	DEF_INT_PROTO1_X_Y(_4##TI##8, _4##TO##8, TINAME##char4, TONAME##char4, RMODE)\
	DEF_INT_PROTO1_X_Y(_8##TI##8, _8##TO##8, TINAME##char8, TONAME##char8, RMODE)\
	DEF_INT_PROTO1_X_Y(_16##TI##8, _16##TO##8, TINAME##char16, TONAME##char16, RMODE)

#define DEF_INT_PROTO8_16(TI, TO, TINAME, TONAME, RMODE)\
	_1##TO##8 __attribute__((overloadable)) convert_##TONAME##char##RMODE(_1##TI##16 x)\
	{\
	return (_1##TO##8)x;\
	}\
	_2##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char2##RMODE(_2##TI##16 x)\
	{\
	return __builtin_astype( trunc_v2i16_v2i8(__builtin_astype(x, _2i16)), _2##TO##8); \
	}\
	TONAME##char3 __attribute__((overloadable)) convert_##TONAME##char3##RMODE(TINAME##short3 x)\
	{\
	return __builtin_astype( trunc_v3i16_v3i8(__builtin_astype(x, _3i16)), _3##TO##8); \
	}\
	_4##TO##8 __attribute__((overloadable)) convert_##TONAME##char4##RMODE(_4##TI##16 x)\
	{\
	return __builtin_astype( trunc_v4i16_v4i8(__builtin_astype(x, _4i16)), _4##TO##8); \
	}\
	_8##TO##8 __attribute__((overloadable)) convert_##TONAME##char8##RMODE(_8##TI##16 x)\
	{\
	return __builtin_astype( trunc_v8i16_v8i8(__builtin_astype(x, _8i16)), _8##TO##8); \
	}\
	_16##TO##8 __attribute__((overloadable)) convert_##TONAME##char16##RMODE(_16##TI##16 x)\
	{\
	return __builtin_astype( trunc_v16i16_v16i8(__builtin_astype(x, _16i16)), _16##TO##8); \
	}\

#define DEF_INT_PROTO8_32(TI, TO, TINAME, TONAME, RMODE)\
	_1##TO##8 __attribute__((overloadable)) convert_##TONAME##char##RMODE(_1##TI##32 x)\
	{\
	return (_1##TO##8)x;\
	}\
	_2##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char2##RMODE(_2##TI##32 x)\
	{\
	return __builtin_astype( trunc_v2i32_v2i8(__builtin_astype(x, _2i32)), _2##TO##8); \
	}\
	TONAME##char3 __attribute__((overloadable)) convert_##TONAME##char3##RMODE(TINAME##int3 x)\
	{\
	return __builtin_astype( trunc_v3i32_v3i8(__builtin_astype(x, _3i32)), TONAME##char3); \
	}\
	_4##TO##8 __attribute__((overloadable)) convert_##TONAME##char4##RMODE(_4##TI##32 x)\
	{\
	return __builtin_astype( trunc_v4i32_v4i8(__builtin_astype(x, _4i32)), _4##TO##8); \
	}\
	_8##TO##8 __attribute__((overloadable)) convert_##TONAME##char8##RMODE(_8##TI##32 x)\
	{\
	return __builtin_astype( trunc_v8i32_v8i8(__builtin_astype(x, _8i32)), _8##TO##8); \
	}\
	_16##TO##8 __attribute__((overloadable)) convert_##TONAME##char16##RMODE(_16##TI##32 x)\
	{\
	return __builtin_astype( trunc_v16i32_v16i8(__builtin_astype(x, _16i32)), _16##TO##8); \
	}\

#define DEF_INT_PROTO8_64(TI, TO, TINAME, TONAME, RMODE)\
	_1##TO##8 __attribute__((overloadable)) convert_##TONAME##char##RMODE(_1##TI##64 x)\
	{\
	return (_1##TO##8)x;\
	}\
	_2##TO##8 __attribute__((overloadable)) convert_##TONAME##char2##RMODE(_2##TI##64 x)\
	{\
	return __builtin_astype( trunc_v2i64_v2i8(__builtin_astype(x, _2i64)), _2##TO##8);\
	}\
	TONAME##char3 __attribute__((overloadable)) convert_##TONAME##char3##RMODE(TINAME##long3 x)\
	{\
	return __builtin_astype( trunc_v3i64_v3i8(__builtin_astype(x, _3i64)), TONAME##char3); \
	}\
	_4##TO##8 __attribute__((overloadable)) convert_##TONAME##char4##RMODE(_4##TI##64 x)\
	{\
	return __builtin_astype( trunc_v4i64_v4i8(__builtin_astype(x, _4i64)), _4##TO##8); \
	}\
	_8##TO##8 __attribute__((overloadable)) convert_##TONAME##char8##RMODE(_8##TI##64 x)\
	{\
	return __builtin_astype( trunc_v8i64_v8i8(__builtin_astype(x, _8i64)), _8##TO##8); \
	}\
	_16##TO##8 __attribute__((overloadable)) convert_##TONAME##char16##RMODE(_16##TI##64 x)\
	{\
	return __builtin_astype( trunc_v16i64_v16i8(__builtin_astype(x, _16i64)), _16##TO##8); \
	}

#define DEF_INT_PROTO8_F(TO, TONAME, RMODE, RMODEVAL, FLAG)\
	_1##TO##8 __attribute__((overloadable)) convert_##TONAME##char##RMODE(float x)\
	{\
	return convert_##TONAME##int##RMODE(x);\
	}\
	_2##TO##8 __attribute__((overloadable)) convert_##TONAME##char2##RMODE(float2 x)\
	{\
	float4 param;\
	param.lo = x;\
	_16##TO##8 res = __builtin_astype(convert_##TONAME##int4##RMODE(param), _16##TO##8);\
	res = __builtin_astype(_mm_shuffle_epi8(__builtin_astype(res,__m128i), *((__m128i *)_4x32to4x8)), _16##TO##8);\
	return res.s01;\
	}\
	TONAME##char3 __attribute__((overloadable)) convert_##TONAME##char3##RMODE(float3 x)\
	{\
	_16##TO##8 res = __builtin_astype(convert_##TONAME##int4##RMODE(as_float4(x)),_16##TO##8);\
	res = __builtin_astype(_mm_shuffle_epi8(__builtin_astype(res,__m128i), *((__m128i *)_4x32to4x8)), _16##TO##8);\
	return as_##TONAME##char3(res.s0123);\
	}\
	_4##TO##8 __attribute__((overloadable)) convert_##TONAME##char4##RMODE(float4 x)\
	{\
	_16##TO##8 res = __builtin_astype(convert_##TONAME##int4##RMODE(x),_16##TO##8);\
	return __builtin_astype( trunc_v4i32_v4i8(__builtin_astype(res, _4i32)), _4##TO##8); \
	}\
	_8##TO##8 __attribute__((overloadable)) convert_##TONAME##char8##RMODE(float8 x)\
	{\
    _8##TO##32 y = convert_##TONAME##int8##RMODE(x);\
	return __builtin_astype( trunc_v8i32_v8i8(__builtin_astype(y, _8i32)), _8##TO##8); \
	}\
	_16##TO##8 __attribute__((overloadable)) convert_##TONAME##char16##RMODE(float16 x)\
	{\
	_16##TO##8 res;\
	res.lo = convert_##TONAME##char8##RMODE(x.lo);\
	res.hi = convert_##TONAME##char8##RMODE(x.hi);\
	return res;\
	}
#define DEF_INT_PROTO8_D(TO, TONAME, RMODE, RMODEVAL)\
	_1##TO##8  __attribute__((overloadable)) convert_##TONAME##char##RMODE(double x)\
	{\
	return convert_##TONAME##int##RMODE(x);\
	}\
	_2##TO##8 __attribute__((overloadable)) convert_##TONAME##char2##RMODE(double2 x)\
	{\
    double4 t;\
    t.lo = x;\
	_16##TO##8 res = __builtin_astype(convert_##TONAME##int4##RMODE(t),_16##TO##8);\
	res = __builtin_astype(_mm_shuffle_epi8(__builtin_astype(res,__m128i), *((__m128i *)_4x32to4x8)),_16##TO##8);\
	return res.s01;\
	}\
	TONAME##char3 __attribute__((overloadable)) convert_##TONAME##char3##RMODE(double3 x)\
	{\
	double4 y;\
	y.s012 = x;\
    _16##TO##8 res = __builtin_astype(convert_##TONAME##int4##RMODE(y),_16##TO##8);\
    res = __builtin_astype(_mm_shuffle_epi8(__builtin_astype(res,__m128i), *((__m128i *)_4x32to4x8)),_16##TO##8);\
	return res.s012;\
	}\
	_4##TO##8  __attribute__((overloadable)) convert_##TONAME##char4##RMODE(double4 x)\
	{\
    _16##TO##8 res = __builtin_astype(convert_##TONAME##int4##RMODE(x),_16##TO##8);\
    res = __builtin_astype(_mm_shuffle_epi8(__builtin_astype(res,__m128i), *((__m128i *)_4x32to4x8)),_16##TO##8);\
	return res.s0123;\
	}\
	_8##TO##8  __attribute__((overloadable)) convert_##TONAME##char8##RMODE(double8 x)\
	{\
	_8##TO##8 res;\
	res.lo = convert_##TONAME##char4##RMODE(x.lo);\
	res.hi = convert_##TONAME##char4##RMODE(x.hi);\
	return res;\
	}\
	_16##TO##8  __attribute__((overloadable)) convert_##TONAME##char16##RMODE(double16 x)\
	{\
	_16##TO##8 res;\
	res.lo = convert_##TONAME##char8##RMODE(x.lo);\
	res.hi = convert_##TONAME##char8##RMODE(x.hi);\
	return res;\
	}


	// 16 bits
#define DEF_INT_PROTO16_8(_CC, TI, TO, TINAME, TONAME, RMODE)\
	_1##TO##16 __attribute__((overloadable)) convert_##TONAME##short##RMODE(_1##TI##8 x)\
	{\
	return (_1##TO##16)x;\
	}\
	_2##TO##16 __attribute__((overloadable)) convert_##TONAME##short2##RMODE(_2##TI##8 x)\
	{\
	return __builtin_astype( _CC##ext_v2i8_v2i16(__builtin_astype(x, _2i8)), _2##TO##16); \
	}\
	TONAME##short3 __attribute__((overloadable)) convert_##TONAME##short3##RMODE(TINAME##char3 x)\
	{\
	return __builtin_astype( _CC##ext_v3i8_v3i16(__builtin_astype(x, _3i8)), _3##TO##16); \
	}\
	_4##TO##16 __attribute__((overloadable)) convert_##TONAME##short4##RMODE(_4##TI##8 x)\
	{\
	return __builtin_astype( _CC##ext_v4i8_v4i16(__builtin_astype(x, _4i8)), _4##TO##16); \
	}\
	_8##TO##16 __attribute__((overloadable)) convert_##TONAME##short8##RMODE(_8##TI##8 x)\
	{\
	return __builtin_astype( _CC##ext_v8i8_v8i16(__builtin_astype(x, _8i8)), _8##TO##16); \
	}\
	_16##TO##16 __attribute__((overloadable)) convert_##TONAME##short16##RMODE(_16##TI##8 x)\
	{\
	return __builtin_astype( _CC##ext_v16i8_v16i16(__builtin_astype(x, _16i8)), _16##TO##16); \
	}\

#define DEF_INT_PROTO16_16(TI, TO, TINAME, TONAME, RMODE)\
	DEF_INT_PROTO1_X_Y(_1##TI##16, _1##TO##16, TINAME##short, TONAME##short, RMODE)\
	DEF_INT_PROTO1_X_Y(_2##TI##16, _2##TO##16, TINAME##short2, TONAME##short2, RMODE)\
	DEF_INT_PROTO1_X_Y(TINAME##short3, TONAME##short3, TINAME##short3, TONAME##short3, RMODE)\
	DEF_INT_PROTO1_X_Y(_4##TI##16, _4##TO##16, TINAME##short4, TONAME##short4, RMODE)\
	DEF_INT_PROTO1_X_Y(_8##TI##16, _8##TO##16, TINAME##short8, TONAME##short8, RMODE)\
	DEF_INT_PROTO1_X_Y(_16##TI##16, _16##TO##16, TINAME##short16, TONAME##short16, RMODE)

#define DEF_INT_PROTO16_32(TI, TO, TINAME, TONAME, RMODE)\
	_1##TO##16 __attribute__((overloadable)) convert_##TONAME##short##RMODE(_1##TI##32 x)\
	{\
	return (_1##TO##16)x;\
	}\
	_2##TO##16 __attribute__((overloadable)) convert_##TONAME##short2##RMODE(_2##TI##32 x)\
	{\
	return __builtin_astype( trunc_v2i32_v2i16(__builtin_astype(x, _2i32)), _2##TO##16); \
	}\
	TONAME##short3 __attribute__((overloadable)) convert_##TONAME##short3##RMODE(TINAME##int3 x)\
	{\
	return __builtin_astype( trunc_v3i32_v3i16(__builtin_astype(x, _3i32)), _3##TO##16); \
	}\
	_4##TO##16 __attribute__((overloadable)) convert_##TONAME##short4##RMODE(_4##TI##32 x)\
	{\
	return __builtin_astype( trunc_v4i32_v4i16(__builtin_astype(x, _4i32)), _4##TO##16); \
	}\
	_8##TO##16 __attribute__((overloadable)) convert_##TONAME##short8##RMODE(_8##TI##32 x)\
	{\
	return __builtin_astype( trunc_v8i32_v8i16(__builtin_astype(x, _8i32)), _8##TO##16); \
	}\
	_16##TO##16 __attribute__((overloadable)) convert_##TONAME##short16##RMODE(_16##TI##32 x)\
	{\
	return __builtin_astype( trunc_v16i32_v16i16(__builtin_astype(x, _16i32)), _16##TO##16); \
	}\

#define DEF_INT_PROTO16_64(TI, TO, TINAME, TONAME, RMODE)\
	_1##TO##16 __attribute__((overloadable)) convert_##TONAME##short##RMODE(_1##TI##64 x)\
	{\
	return (_1##TO##16)x;\
	}\
	_2##TO##16 __attribute__((overloadable)) convert_##TONAME##short2##RMODE(_2##TI##64 x)\
	{\
	return __builtin_astype( trunc_v2i64_v2i16(__builtin_astype(x, _2i64)), _2##TO##16); \
	}\
	TONAME##short3 __attribute__((overloadable)) convert_##TONAME##short3##RMODE(TINAME##long3 x)\
	{\
	return __builtin_astype( trunc_v3i64_v3i16(__builtin_astype(x, _3i64)), _3##TO##16); \
	}\
	_4##TO##16 __attribute__((overloadable)) convert_##TONAME##short4##RMODE(_4##TI##64 x)\
	{\
	return __builtin_astype( trunc_v4i64_v4i16(__builtin_astype(x, _4i64)), _4##TO##16); \
	}\
	_8##TO##16 __attribute__((overloadable)) convert_##TONAME##short8##RMODE(_8##TI##64 x)\
	{\
	return __builtin_astype( trunc_v8i64_v8i16(__builtin_astype(x, _8i64)), _8##TO##16); \
	}\
	_16##TO##16 __attribute__((overloadable)) convert_##TONAME##short16##RMODE(_16##TI##64 x)\
	{\
	return __builtin_astype( trunc_v16i64_v16i16(__builtin_astype(x, _16i64)), _16##TO##16); \
	}\

#define DEF_INT_PROTO16_F(TO, TONAME, RMODE, RMODEVAL, FLAG)\
	_1##TO##16 __attribute__((overloadable)) convert_##TONAME##short##RMODE(float x)\
	{\
    return convert_##TONAME##int##RMODE(x);\
	}\
	_2##TO##16 __attribute__((overloadable)) convert_##TONAME##short2##RMODE(float2 x)\
	{\
	_8##TO##16 res;\
	float4 param;\
	param.lo = x;\
	res = __builtin_astype(convert_##TONAME##int4##RMODE(param),_8##TO##16);\
	res = __builtin_astype(_mm_shuffle_epi8(__builtin_astype(res,__m128i), *((__m128i *)_4x32to4x16)),_8##TO##16);\
	return res.s01;\
	}\
	TONAME##short3 __attribute__((overloadable)) convert_##TONAME##short3##RMODE(float3 x)\
	{\
	_8##TO##16 res;\
	res = __builtin_astype(convert_##TONAME##int4##RMODE(as_float4(x)),_8##TO##16);\
	res = __builtin_astype(_mm_shuffle_epi8(__builtin_astype(res,__m128i), *((__m128i *)_4x32to4x16)),_8##TO##16);\
	return as_##TONAME##short3(res.lo);\
	}\
	_4##TO##16 __attribute__((overloadable)) convert_##TONAME##short4##RMODE(float4 x)\
	{\
	_8##TO##16 res;\
	res = __builtin_astype(convert_##TONAME##int4##RMODE(x),_8##TO##16);\
	res = __builtin_astype(_mm_shuffle_epi8(__builtin_astype(res,__m128i), *((__m128i *)_4x32to4x16)),_8##TO##16);\
	return res.lo;\
	}\
	_8##TO##16 __attribute__((overloadable)) convert_##TONAME##short8##RMODE(float8 x)\
	{\
	_8##TO##16 res;\
	_8##TO##16 t1 = __builtin_astype(convert_##TONAME##int4##RMODE(x.lo),_8##TO##16);\
	_8##TO##16 t2 = __builtin_astype(convert_##TONAME##int4##RMODE(x.hi),_8##TO##16);\
	t1 = __builtin_astype(_mm_shuffle_epi8(__builtin_astype(t1,__m128i), *((__m128i *)_4x32to4x16)),_8##TO##16);\
	t2 = __builtin_astype(_mm_shuffle_epi8(__builtin_astype(t2,__m128i), *((__m128i *)_4x32to4x16)),_8##TO##16);\
	res = __builtin_astype(_mm_unpacklo_epi64(__builtin_astype(t1,__m128i), __builtin_astype(t2,__m128i)),_8##TO##16);\
	return res;\
	}\
	_16##TO##16 __attribute__((overloadable)) convert_##TONAME##short16##RMODE(float16 x)\
	{\
	_16##TO##16 res;\
	res.lo = convert_##TONAME##short8##RMODE(x.lo);\
	res.hi = convert_##TONAME##short8##RMODE(x.hi);\
	return res;\
	}\

#define DEF_INT_PROTO16_D(TO, TONAME, RMODE, RMODEVAL)\
	_1##TO##16 __attribute__((overloadable)) convert_##TONAME##short##RMODE(double x)\
	{\
	return convert_##TONAME##int##RMODE(x);\
	}\
	_2##TO##16 __attribute__((overloadable)) convert_##TONAME##short2##RMODE(double2 x)\
	{\
    double4 y;\
    y.lo = x;\
	_8##TO##16 res = __builtin_astype(convert_##TONAME##int4##RMODE(y),_8##TO##16);\
	res = __builtin_astype(_mm_shuffle_epi8(__builtin_astype(res,__m128i), *((__m128i *)_4x32to4x16)),_8##TO##16);\
	return res.s01;\
	}\
	_3##TO##16 __attribute__((overloadable)) convert_##TONAME##short3##RMODE(double3 x)\
	{\
    double4 y;\
    y.s012 = x;\
	_8##TO##16 res = __builtin_astype(convert_##TONAME##int4##RMODE(y),_8##TO##16);\
	res = __builtin_astype(_mm_shuffle_epi8(__builtin_astype(res,__m128i), *((__m128i *)_4x32to4x16)),_8##TO##16);\
	return res.s012;\
	}\
	_4##TO##16 __attribute__((overloadable)) convert_##TONAME##short4##RMODE(double4 x)\
	{\
	_8##TO##16 res = __builtin_astype(convert_##TONAME##int4##RMODE(x),_8##TO##16);\
	res = __builtin_astype(_mm_shuffle_epi8(__builtin_astype(res,__m128i), *((__m128i *)_4x32to4x16)),_8##TO##16);\
	return res.s0123;\
	}\
	_8##TO##16 __attribute__((overloadable)) convert_##TONAME##short8##RMODE(double8 x)\
	{\
	_8##TO##16 res;\
	res.lo = convert_##TONAME##short4##RMODE(x.lo);\
	res.hi = convert_##TONAME##short4##RMODE(x.hi);\
	return res;\
	}\
	_16##TO##16 __attribute__((overloadable)) convert_##TONAME##short16##RMODE(double16 x)\
	{\
	_16##TO##16 res;\
	res.lo = convert_##TONAME##short8##RMODE(x.lo);\
	res.hi = convert_##TONAME##short8##RMODE(x.hi);\
	return res;\
	}\

	// 32 bits
#define DEF_INT_PROTO32_8( _CC, TI, TO, TINAME, TONAME, RMODE)\
	_1##TO##32 __attribute__((overloadable)) convert_##TONAME##int##RMODE(_1##TI##8 x)\
	{\
	return (_1##TO##32)x;\
	}\
	_2##TO##32 __attribute__((overloadable)) convert_##TONAME##int2##RMODE(_2##TI##8 x)\
	{\
	return __builtin_astype( _CC##ext_v2i8_v2i32(__builtin_astype(x, _2i8)), _2##TO##32); \
	}\
	TONAME##int3 __attribute__ ((overloadable)) convert_##TONAME##int3##RMODE(TINAME##char3 x)\
	{\
	return __builtin_astype( _CC##ext_v3i8_v3i32(__builtin_astype(x, _3i8)), _3##TO##32); \
	}\
	_4##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int4##RMODE(_4##TI##8 x)\
	{\
	return __builtin_astype( _CC##ext_v4i8_v4i32(__builtin_astype(x, _4i8)), _4##TO##32); \
	}\
	_8##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int8##RMODE(_8##TI##8 x)\
	{\
	return __builtin_astype( _CC##ext_v8i8_v8i32(__builtin_astype(x, _8i8)), _8##TO##32); \
	}\
	_16##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int16##RMODE(_16##TI##8 x)\
	{\
	return __builtin_astype( _CC##ext_v16i8_v16i32(__builtin_astype(x, _16i8)), _16##TO##32); \
	}\

#define DEF_INT_PROTO32_16( _CC, TI, TO, TINAME, TONAME, RMODE)\
	_1##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int##RMODE(_1##TI##16 x)\
	{\
	return (_1##TO##32)x;\
	}\
	_2##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int2##RMODE(_2##TI##16 x)\
	{\
	return __builtin_astype( _CC##ext_v2i16_v2i32(__builtin_astype(x, _2i16)), _2##TO##32); \
	}\
	TONAME##int3 __attribute__ ((overloadable)) convert_##TONAME##int3##RMODE(TINAME##short3 x)\
	{\
	return __builtin_astype( _CC##ext_v3i16_v3i32(__builtin_astype(x, _3i16)), _3##TO##32); \
	}\
	_4##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int4##RMODE(_4##TI##16 x)\
	{\
	return __builtin_astype( _CC##ext_v4i16_v4i32(__builtin_astype(x, _4i16)), _4##TO##32); \
	}\
	_8##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int8##RMODE(_8##TI##16 x)\
	{\
	return __builtin_astype( _CC##ext_v8i16_v8i32(__builtin_astype(x, _8i16)), _8##TO##32); \
	}\
	_16##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int16##RMODE(_16##TI##16 x)\
	{\
	return __builtin_astype( _CC##ext_v16i16_v16i32(__builtin_astype(x, _16i16)), _16##TO##32); \
	}

#define DEF_INT_PROTO32_32(TI, TO, TINAME, TONAME, RMODE)\
	DEF_INT_PROTO1_X_Y(_1##TI##32, _1##TO##32, TINAME##int, TONAME##int, RMODE)\
	DEF_INT_PROTO1_X_Y(_2##TI##32, _2##TO##32, TINAME##int2, TONAME##int2, RMODE)\
	DEF_INT_PROTO1_X_Y(TINAME##int3, TONAME##int3, TINAME##int3, TONAME##int3, RMODE)\
	DEF_INT_PROTO1_X_Y(_4##TI##32, _4##TO##32, TINAME##int4, TONAME##int4, RMODE)\
	DEF_INT_PROTO1_X_Y(_8##TI##32, _8##TO##32, TINAME##int8, TONAME##int8, RMODE)\
	DEF_INT_PROTO1_X_Y(_16##TI##32, _16##TO##32, TINAME##int16, TONAME##int16, RMODE)

#define DEF_INT_PROTO32_64(TI, TO, TINAME, TONAME, RMODE)\
	_1##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int##RMODE(_1##TI##64 x)\
	{\
	return (_1##TO##32)x;\
	}\
	_2##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int2##RMODE(_2##TI##64 x)\
	{\
	return __builtin_astype( trunc_v2i64_v2i32(__builtin_astype(x, _2i64)), _2##TO##32); \
	}\
	TONAME##int3 __attribute__ ((overloadable)) convert_##TONAME##int3##RMODE(TINAME##long3 x)\
	{\
	return __builtin_astype( trunc_v3i64_v3i32(__builtin_astype(x, _3i64)), _3##TO##32); \
	}\
	_4##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int4##RMODE(_4##TI##64 x)\
	{\
	return __builtin_astype( trunc_v4i64_v4i32(__builtin_astype(x, _4i64)), _4##TO##32); \
	}\
	_8##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int8##RMODE(_8##TI##64 x)\
	{\
	return __builtin_astype( trunc_v8i64_v8i32(__builtin_astype(x, _8i64)), _8##TO##32); \
	}\
	_16##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int16##RMODE(_16##TI##64 x)\
	{\
	return __builtin_astype( trunc_v16i64_v16i32(__builtin_astype(x, _16i64)), _16##TO##32); \
	}\

#define DEF_INT_PROTOI32_F_F1234_AS_F4(RMODE)                       \
	_1i32 __attribute__ ((overloadable)) convert_int##RMODE(float x)\
	{\
	_1i32 res;\
	float4 param;\
	param.s0 = x;\
	res = floatToInt(param);\
	return res;\
	}\
	_2i32 __attribute__ ((overloadable)) convert_int2##RMODE(float2 x)\
	{\
	_4i32 res;\
	float4 param;\
	param.lo = x;\
	res = as_int4(float4ToInt4(param));\
	return res.lo;\
	}\
	int3 __attribute__ ((overloadable)) convert_int3##RMODE(float3 x)\
	{\
	int4 res;\
	res = as_int4(float4ToInt4(as_float4(x)));\
	return as_int3(res);\
	}\
	_4i32 __attribute__ ((overloadable)) convert_int4##RMODE(float4 x)\
	{\
	_4i32 res;\
	res = as_int4(float4ToInt4(x));\
	return res;\
	}\


#define DEF_INT_PROTOI32_F_F816_AS_F4(RMODE)                            \
	_8i32 __attribute__ ((overloadable)) convert_int8##RMODE(float8 x)\
	{\
	_8i32 res;\
	res.lo = as_int4(float4ToInt4(x.lo));\
	res.hi = as_int4(float4ToInt4(x.hi));\
	return res;\
	}\
	_16i32 __attribute__ ((overloadable)) convert_int16##RMODE(float16 x)\
	{\
	_16i32 res;\
	res.lo.lo = as_int4(float4ToInt4(x.lo.lo));\
    res.lo.hi = as_int4(float4ToInt4(x.lo.hi));\
	res.hi.lo = as_int4(float4ToInt4(x.hi.lo));\
    res.hi.hi = as_int4(float4ToInt4(x.hi.hi));\
	return res;\
	}

#define DEF_INT_PROTOI32_F_F816_AS_F8(RMODE)            \
	_8i32 __attribute__ ((overloadable)) convert_int8##RMODE(float8 x)\
	{\
	_8i32 res;\
	res = as_int8(float8ToInt8(x));\
	return res;\
	}\
	_16i32 __attribute__ ((overloadable)) convert_int16##RMODE(float16 x)\
	{\
	_16i32 res;\
	res.lo = as_int8(float8ToInt8(x.lo));\
	res.hi = as_int8(float8ToInt8(x.hi));\
	return res;\
	}

#if defined(__AVX__)
#define DEF_INT_PROTOI32_FNOSVML(RMODE, RMODEVAL, RSVML, CPUTYPE)       \
    DEF_INT_PROTOI32_F_F1234_AS_F4(RMODE)                               \
    DEF_INT_PROTOI32_F_F816_AS_F8(RMODE)
#else // defined(__AVX__)
#define DEF_INT_PROTOI32_FNOSVML(RMODE, RMODEVAL, RSVML, CPUTYPE)       \
    DEF_INT_PROTOI32_F_F1234_AS_F4(RMODE)                               \
    DEF_INT_PROTOI32_F_F816_AS_F4(RMODE)
#endif // defined(__AVX__)

// Oleg:
// convert_int(float)
// This is issue with ABI for SVML.
// So as a hack we call float4 version of conversions for float
// TODO: switch to DEF_INT_PROTOI32_FUSESVML_X when CSSD100012898:
// "ABI for passing float2 arguments to SVML conversions function is invalid" is fixed
#define DEF_INT_PROTOI32_FUSESVML_2(RMODE, RMODEVAL, RSVML, CPUTYPE, WIDTHOCL, WIDTHSVML)\
    _2i32 __attribute__ ((overloadable)) convert_int2##RMODE(float2  x)\
	{\
    float4 y;\
    y.lo = x;\
    y.hi = x;\
    return convert_int4##RMODE(y).lo;\
	}

#define DEF_INT_PROTOI32_FUSESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, WIDTHOCL, WIDTHSVML)\
    _##WIDTHSVML##i32 __attribute__ ((overloadable)) convert_int##WIDTHOCL##RMODE(float##WIDTHOCL  x)\
	{\
    _##WIDTHSVML##i32 res = __ocl_svml_##CPUTYPE##_cvtfptoi32##RSVML##nosatf##WIDTHSVML(x);\
	return res;\
	}

#define DEF_INT_PROTOI32_FUSESVML(RMODE, RMODEVAL, RSVML, CPUTYPE)\
    DEF_INT_PROTOI32_FUSESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, , 1)\
    DEF_INT_PROTOI32_FUSESVML_2(RMODE, RMODEVAL, RSVML, CPUTYPE, 2, 2)\
    DEF_INT_PROTOI32_FUSESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, 3, 3)\
    DEF_INT_PROTOI32_FUSESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, 4, 4)\
    DEF_INT_PROTOI32_FUSESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, 8, 8)\
    DEF_INT_PROTOI32_FUSESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, 16, 16)


#define DEF_INT_PROTOU32_F_F1234(RMODE)\
	_1u32 __attribute__ ((overloadable)) convert_uint##RMODE(float x)\
	{\
    _1u32 res = as_uint(floatToUint(x));\
	return res;\
	}\
	_2u32 __attribute__ ((overloadable)) convert_uint2##RMODE(float2 x)\
	{\
	_4u32 res;\
	res.s0 = as_uint(floatToUint(x.lo));\
	res.s1 = as_uint(floatToUint(x.hi));\
	return res.lo;\
	}\
	uint3 __attribute__ ((overloadable)) convert_uint3##RMODE(float3 x)\
	{\
	_4u32 res;\
	res.s0 = as_uint(floatToUint(x.s0));\
	res.s1 = as_uint(floatToUint(x.s1));\
	res.s2 = as_uint(floatToUint(x.s2));\
	return as_uint3(res);\
	}\
	_4u32 __attribute__ ((overloadable)) convert_uint4##RMODE(float4 x)\
	{\
	_4u32 res;\
	res = float4ToUint4(x);\
	return res;\
	}\

#define DEF_INT_PROTOU32_F_F816_AS_F4(RMODE)\
    _8u32 __attribute__ ((overloadable)) convert_uint8##RMODE(float8 x)\
	{\
	_8u32 res;\
	res.lo = convert_uint4##RMODE(x.lo);\
	res.hi = convert_uint4##RMODE(x.hi);\
	return res;\
	}\
	_16u32 __attribute__ ((overloadable)) convert_uint16##RMODE(float16 x)\
	{\
	_16u32 res;\
	res.lo.lo = convert_uint4##RMODE(x.lo.lo);\
	res.lo.hi = convert_uint4##RMODE(x.lo.hi);\
	res.hi.lo = convert_uint4##RMODE(x.hi.lo);\
	res.hi.hi = convert_uint4##RMODE(x.hi.hi);\
	return res;\
	}

#define DEF_INT_PROTOU32_F_F816_AS_F8(RMODE)\
    _8u32 __attribute__ ((overloadable)) convert_uint8##RMODE(float8 x)\
	{\
	_8u32 res;\
	res = (uint8)float8ToUint8(x);\
	return res;\
	}\
	_16u32 __attribute__ ((overloadable)) convert_uint16##RMODE(float16 x)\
	{\
	_16u32 res;\
	res.lo = convert_uint8##RMODE(x.lo);\
	res.hi = convert_uint8##RMODE(x.hi);\
	return res;\
	}

#if defined(__AVX__)
#define DEF_INT_PROTOU32_FNOSVML(RMODE, RMODEVAL, RSVML, CPUTYPE)\
    DEF_INT_PROTOU32_F_F1234(RMODE)\
    DEF_INT_PROTOU32_F_F816_AS_F8(RMODE)
#else // defined(__AVX__)
#define DEF_INT_PROTOU32_FNOSVML(RMODE, RMODEVAL, RSVML, CPUTYPE)\
    DEF_INT_PROTOU32_F_F1234(RMODE)\
    DEF_INT_PROTOU32_F_F816_AS_F4(RMODE)
#endif // defined(__AVX__)

// Oleg:
// convert_int(float)
// This is issue with ABI for SVML.
// So as a hack we call float4 version of conversions for float2
// TODO: switch to DEF_INT_PROTOU32_FUSESVML_X when CSSD100012898:
// "ABI for passing float2 arguments to SVML conversions function is invalid" is fixed
#define DEF_INT_PROTOU32_FUSESVML_2(RMODE, RMODEVAL, RSVML, CPUTYPE, WIDTHOCL, WIDTHSVML)\
    _2u32 __attribute__ ((overloadable)) convert_uint2##RMODE(float2  x)\
	{\
    float4 y;\
    y.lo = x;\
    y.hi = x;\
    return convert_uint4##RMODE(y).lo;\
	}
#define DEF_INT_PROTOU32_FUSESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, WIDTHOCL, WIDTHSVML)\
    _##WIDTHSVML##u32 __attribute__ ((overloadable)) convert_uint##WIDTHOCL##RMODE(float##WIDTHOCL  x)\
	{\
    _##WIDTHSVML##u32 res = __ocl_svml_##CPUTYPE##_cvtfptou32##RSVML##nosatf##WIDTHSVML(x);\
	return res;\
	}

#define DEF_INT_PROTOU32_FUSESVML(RMODE, RMODEVAL, RSVML, CPUTYPE)\
    DEF_INT_PROTOU32_FUSESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, , 1)\
    DEF_INT_PROTOU32_FUSESVML_2(RMODE, RMODEVAL, RSVML, CPUTYPE, 2, 2)\
    DEF_INT_PROTOU32_FUSESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, 3, 3)\
    DEF_INT_PROTOU32_FUSESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, 4, 4)\
    DEF_INT_PROTOU32_FUSESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, 8, 8)\
    DEF_INT_PROTOU32_FUSESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, 16, 16)


#define DEF_INT_PROTOI32_D_D12_AS_D2(RMODE)                       \
	_1i32 __attribute__ ((overloadable)) convert_int##RMODE(double x)\
	{\
	_4i32 res;\
	double2 param;\
	param.lo = x;\
	res = as_int4(double2ToInt4(param));\
	return res.s0;\
	}\
	_2i32 __attribute__ ((overloadable)) convert_int2##RMODE(double2 x)\
	{\
	return as_int4(double2ToInt4(x)).s01;\
	}

#define DEF_INT_PROTOI32_D_D34_AS_D2(RMODE)                            \
	int3 __attribute__((overloadable)) convert_int3##RMODE(double3 x)\
	{\
	_4i32 t1, t2, res;\
	t1 = as_int4(double2ToInt4(x.lo));\
	t2 = as_int4(double2ToInt4(x.hi));\
	res = as_int4(_mm_unpacklo_epi64(__builtin_astype(t1,__m128i), __builtin_astype(t2,__m128i)));\
	return as_int3(res);\
	}\
	_4i32 __attribute__((overloadable)) convert_int4##RMODE(double4 x)\
	{\
	_4i32 t1, t2, res;\
	t1 = as_int4(double2ToInt4(x.lo));\
	t2 = as_int4(double2ToInt4(x.hi));\
	res = as_int4(_mm_unpacklo_epi64(__builtin_astype(t1,__m128i), __builtin_astype(t2,__m128i)));\
	return res;\
	}

#define DEF_INT_PROTOI32_D_D34_AS_D4(RMODE)                            \
	int3 __attribute__((overloadable)) convert_int3##RMODE(double3 x)\
	{\
    double4 tmp = as_double4(x);\
	_4i32 t = as_int4(double4ToInt4(tmp));\
	return as_int3(t);\
	}\
	_4i32 __attribute__((overloadable)) convert_int4##RMODE(double4 x)\
	{\
	return as_int4(double4ToInt4(x));\
	}


#define DEF_INT_PROTOI32_D_D816_AS_D4(RMODE)                            \
	_8i32 __attribute__((overloadable)) convert_int8##RMODE(double8 x)\
	{\
	_8i32 res;\
	res.lo =  convert_int4##RMODE(x.lo);\
	res.hi =  convert_int4##RMODE(x.hi);\
	return res;\
	}\
	_16i32 __attribute__((overloadable)) convert_int16##RMODE(double16 x)\
	{\
	_16i32 res;\
	res.lo =  convert_int8##RMODE(x.lo);\
	res.hi =  convert_int8##RMODE(x.hi);\
	return res;\
	}

#if defined(__AVX__)
#define DEF_INT_PROTOI32_DNOSVML(RMODE, RMODEVAL, RSVML, CPUTYPE)       \
    DEF_INT_PROTOI32_D_D12_AS_D2(RMODE)                               \
    DEF_INT_PROTOI32_D_D34_AS_D4(RMODE)                               \
    DEF_INT_PROTOI32_D_D816_AS_D4(RMODE)                              
#else // defined(__AVX__)
#define DEF_INT_PROTOI32_DNOSVML(RMODE, RMODEVAL, RSVML, CPUTYPE)       \
    DEF_INT_PROTOI32_D_D12_AS_D2(RMODE)                               \
    DEF_INT_PROTOI32_D_D34_AS_D2(RMODE)                              \
    DEF_INT_PROTOI32_D_D816_AS_D4(RMODE)                              
#endif // defined(__AVX__)


// Oleg:
// convert_int(double)
// This is issue with ABI for SVML.
// So as a hack we call double and double2 version of conversions
// TODO: switch double2,3,4,8,16 to SVML call when CSSD100012907:
// "ABI for passing double3 arguments to SVML conversions function is invalid" is fixed

#define DEF_INT_PROTOF_DUSESVML_234816_AS_D1(RMODE, RMODEVAL, RSVML, CPUTYPE)\
	int2 __attribute__ ((overloadable)) convert_int2##RMODE(double2 x)\
    {\
	int2 res;\
	res.lo = convert_int##RMODE(x.lo);\
	res.hi = convert_int##RMODE(x.hi);\
	return res;\
	}\
    int3 __attribute__ ((overloadable)) convert_int3##RMODE(double3 x)\
	{\
    int3 res;\
    double2 y1,y2;\
    y1 = x.s01;\
    y2.lo = x.s2;\
    int2 r1 = convert_int2##RMODE(y1);\
    int2 r2 = convert_int2##RMODE(y2);\
    res.s01 = r1;\
    res.s2 = r2.lo;\
    return res;\
	}\
	int4 __attribute__ ((overloadable)) convert_int4##RMODE(double4 x)\
    {\
	int4 res;\
	res.lo = convert_int2##RMODE(x.lo);\
	res.hi = convert_int2##RMODE(x.hi);\
	return res;\
	}\
	int8 __attribute__ ((overloadable)) convert_int8##RMODE(double8 x)\
    {\
	int8 res;\
	res.lo = convert_int4##RMODE(x.lo);\
	res.hi = convert_int4##RMODE(x.hi);\
	return res;\
	}\
	int16 __attribute__ ((overloadable)) convert_int16##RMODE(double16 x)\
    {\
	int16 res;\
	res.lo = convert_int8##RMODE(x.lo);\
	res.hi = convert_int8##RMODE(x.hi);\
	return res;\
	}

#define DEF_INT_PROTOI32_DUSESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, WIDTHOCL, WIDTHSVML)\
    _##WIDTHSVML##i32 __attribute__ ((overloadable)) convert_int##WIDTHOCL##RMODE(double##WIDTHOCL  x)\
	{\
    _##WIDTHSVML##i32 res = __ocl_svml_##CPUTYPE##_cvtfptoi32##RSVML##nosat##WIDTHSVML(x);\
	return res;\
	}

#define DEF_INT_PROTOI32_DUSESVML(RMODE, RMODEVAL, RSVML, CPUTYPE)\
    DEF_INT_PROTOI32_DUSESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, , 1)\
    DEF_INT_PROTOF_DUSESVML_234816_AS_D1(RMODE, RMODEVAL, RSVML, CPUTYPE)

/*
Boaz Ouriel Note:
here we have an issue with the convert_uint(double2 x);
we have a workaround for this issue by resolving this by calling the convert_uint(double4 x)
we need to report this to Nikita and get a fix for this.
*/
#define DEF_INT_PROTOU32_D(RMODE, RSVML, CPUTYPE)\
	_1u32 __attribute__((overloadable)) convert_uint##RMODE(double x)\
	{\
	return __ocl_svml_##CPUTYPE##_cvtfptou32##RSVML##nosat1(x);\
	}\
	_2u32 __attribute__((overloadable)) convert_uint2##RMODE(double2 x)\
	{\
    double4 y = (double4)(1.0); /* Use a normal value, to avoid worst case(denormal, NaN,,,) */ \
	y.lo = x;\
	uint4 res = __ocl_svml_##CPUTYPE##_cvtfptou32##RSVML##nosat4(y);\
	return res.lo;\
	}\
	uint3 __attribute__((overloadable)) convert_uint3##RMODE(double3 x)\
	{\
	double4 y;\
	y.s012 = x;\
	uint4 res = __ocl_svml_##CPUTYPE##_cvtfptou32##RSVML##nosat4(y);\
	return res.s012;\
	}\
	_4u32 __attribute__((overloadable)) convert_uint4##RMODE(double4 x)\
	{\
	return __ocl_svml_##CPUTYPE##_cvtfptou32##RSVML##nosat4(x);\
	}\
	_8u32 __attribute__((overloadable)) convert_uint8##RMODE(double8 x)\
	{\
	return __ocl_svml_##CPUTYPE##_cvtfptou32##RSVML##nosat8(x);\
	}\
	_16u32 __attribute__((overloadable)) convert_uint16##RMODE(double16 x)\
	{\
	_16u32 res;\
	res.lo = __ocl_svml_##CPUTYPE##_cvtfptou32##RSVML##nosat8(x.lo);\
	res.hi = __ocl_svml_##CPUTYPE##_cvtfptou32##RSVML##nosat8(x.hi);\
	return res;\
	}

	// 64 bits
#define DEF_INT_PROTO64_8(TI, TO, TINAME, TONAME, RMODE)\
	_1##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long##RMODE(_1##TI##8 x)\
	{\
	return (_1##TO##64)x;\
	}\
	_2##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long2##RMODE(_2##TI##8 x)\
	{\
	_2##TO##64 res;\
	_16##TI##8 param;\
	param.s01 = x;\
	res = __builtin_astype(_mm_cvtep##TI##8_epi64(__builtin_astype(param,__m128i)),_2##TO##64);\
	return res;\
	}\
	TONAME##long3 __attribute__ ((overloadable)) convert_##TONAME##long3##RMODE(TINAME##char3 x)\
	{\
	_4##TO##64 res;\
	_16##TI##8 param;\
	param.s0123 = as_##TINAME##char4(x);\
	res.lo =  __builtin_astype(_mm_cvtep##TI##8_epi64(__builtin_astype(param,__m128i)),_2##TO##64);\
	res.hi =  __builtin_astype(_mm_cvtep##TI##8_epi64(_mm_srli_si128(__builtin_astype(param,__m128i), 2)),_2##TO##64);\
	return as_##TONAME##long3(res);\
	}\
	_4##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long4##RMODE(_4##TI##8 x)\
	{\
	_4##TO##64 res;\
	_16##TI##8 param;\
	param.s0123 = x;\
	res.lo =  __builtin_astype(_mm_cvtep##TI##8_epi64(__builtin_astype(param,__m128i)),_2##TO##64);\
	res.hi =  __builtin_astype(_mm_cvtep##TI##8_epi64(_mm_srli_si128(__builtin_astype(param,__m128i), 2)),_2##TO##64);\
	return res;\
	}\
	_8##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long8##RMODE(_8##TI##8 x)\
	{\
	_8##TO##64 res;\
	_16##TI##8 param;\
	param.lo = x;\
	res.lo.lo = __builtin_astype(_mm_cvtep##TI##8_epi64(__builtin_astype(param,__m128i)),_2##TO##64);\
	res.lo.hi = __builtin_astype(_mm_cvtep##TI##8_epi64(_mm_srli_si128(__builtin_astype(param,__m128i), 2)),_2##TO##64);\
	res.hi.lo = __builtin_astype(_mm_cvtep##TI##8_epi64(_mm_srli_si128(__builtin_astype(param,__m128i), 4)),_2##TO##64);\
	res.hi.hi = __builtin_astype(_mm_cvtep##TI##8_epi64(_mm_srli_si128(__builtin_astype(param,__m128i), 6)),_2##TO##64);\
	return res;\
	}\
	_16##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long16##RMODE(_16##TI##8 x)\
	{\
	_16##TO##64 res;\
	res.lo.lo.lo = __builtin_astype(_mm_cvtep##TI##8_epi64(__builtin_astype(x,__m128i)),_2##TO##64);\
	res.lo.lo.hi = __builtin_astype(_mm_cvtep##TI##8_epi64(_mm_srli_si128(__builtin_astype(x,__m128i), 2)),_2##TO##64);\
	res.lo.hi.lo = __builtin_astype(_mm_cvtep##TI##8_epi64(_mm_srli_si128(__builtin_astype(x,__m128i), 4)),_2##TO##64);\
	res.lo.hi.hi = __builtin_astype(_mm_cvtep##TI##8_epi64(_mm_srli_si128(__builtin_astype(x,__m128i), 6)),_2##TO##64);\
	res.hi.lo.lo = __builtin_astype(_mm_cvtep##TI##8_epi64(_mm_srli_si128(__builtin_astype(x,__m128i), 8)),_2##TO##64);\
	res.hi.lo.hi = __builtin_astype(_mm_cvtep##TI##8_epi64(_mm_srli_si128(__builtin_astype(x,__m128i), 10)),_2##TO##64);\
	res.hi.hi.lo = __builtin_astype(_mm_cvtep##TI##8_epi64(_mm_srli_si128(__builtin_astype(x,__m128i), 12)),_2##TO##64);\
	res.hi.hi.hi = __builtin_astype(_mm_cvtep##TI##8_epi64(_mm_srli_si128(__builtin_astype(x,__m128i), 14)),_2##TO##64);\
	return res;\
	}\

#define DEF_INT_PROTO64_16(TI, TO, TINAME, TONAME, RMODE)\
	_1##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long##RMODE(_1##TI##16 x)\
	{\
	return (_1##TO##64)x;\
	}\
	_2##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long2##RMODE(_2##TI##16 x)\
	{\
	_2##TO##64 res;\
	_8##TI##16 param;\
	param.s01 = x;\
	res = __builtin_astype(_mm_cvtep##TI##16_epi64(__builtin_astype(param,__m128i)),_2##TO##64);\
	return res;\
	}\
	TONAME##long3 __attribute__ ((overloadable)) convert_##TONAME##long3##RMODE(TINAME##short3 x)\
	{\
	_8##TI##16 param;\
	_4##TO##64 res;\
	param.lo =as_##TINAME##short4(x);\
	res.lo = __builtin_astype(_mm_cvtep##TI##16_epi64(__builtin_astype(param,__m128i)),_2##TO##64);\
	res.hi = __builtin_astype(_mm_cvtep##TI##16_epi64(_mm_srli_si128(__builtin_astype(param,__m128i), 4)),_2##TO##64);\
	return as_##TONAME##long3(res);\
	}\
	_4##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long4##RMODE(_4##TI##16 x)\
	{\
	_8##TI##16 param;\
	_4##TO##64 res;\
	param.lo = x;\
	res.lo = __builtin_astype(_mm_cvtep##TI##16_epi64(__builtin_astype(param,__m128i)),_2##TO##64);\
	res.hi = __builtin_astype(_mm_cvtep##TI##16_epi64(_mm_srli_si128(__builtin_astype(param,__m128i), 4)),_2##TO##64);\
	return res;\
	}\
	_8##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long8##RMODE(_8##TI##16 x)\
	{\
	_8##TO##64 res;\
	res.lo.lo = __builtin_astype(_mm_cvtep##TI##16_epi64(__builtin_astype(x,__m128i)),_2##TO##64);\
	res.lo.hi = __builtin_astype(_mm_cvtep##TI##16_epi64(_mm_srli_si128(__builtin_astype(x,__m128i), 4)),_2##TO##64);\
	res.hi.lo = __builtin_astype(_mm_cvtep##TI##16_epi64(_mm_srli_si128(__builtin_astype(x,__m128i), 8)),_2##TO##64);\
	res.hi.hi = __builtin_astype(_mm_cvtep##TI##16_epi64(_mm_srli_si128(__builtin_astype(x,__m128i), 12)),_2##TO##64);\
	return res;\
	}\
	_16##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long16##RMODE(_16##TI##16 x)\
	{\
	_16##TO##64 res;\
	res.lo.lo.lo = __builtin_astype(_mm_cvtep##TI##16_epi64(__builtin_astype(x.lo,__m128i)),_2##TO##64);\
	res.lo.lo.hi = __builtin_astype(_mm_cvtep##TI##16_epi64(_mm_srli_si128(__builtin_astype(x.lo,__m128i), 4)),_2##TO##64);\
	res.lo.hi.lo = __builtin_astype(_mm_cvtep##TI##16_epi64(_mm_srli_si128(__builtin_astype(x.lo,__m128i), 8)),_2##TO##64);\
	res.lo.hi.hi = __builtin_astype(_mm_cvtep##TI##16_epi64(_mm_srli_si128(__builtin_astype(x.lo,__m128i), 12)),_2##TO##64);\
	res.hi.lo.lo = __builtin_astype(_mm_cvtep##TI##16_epi64(__builtin_astype(x.hi,__m128i)),_2##TO##64);\
	res.hi.lo.hi = __builtin_astype(_mm_cvtep##TI##16_epi64(_mm_srli_si128(__builtin_astype(x.hi,__m128i), 4)),_2##TO##64);\
	res.hi.hi.lo = __builtin_astype(_mm_cvtep##TI##16_epi64(_mm_srli_si128(__builtin_astype(x.hi,__m128i), 8)),_2##TO##64);\
	res.hi.hi.hi = __builtin_astype(_mm_cvtep##TI##16_epi64(_mm_srli_si128(__builtin_astype(x.hi,__m128i), 12)),_2##TO##64);\
	return res;\
	}\

#define DEF_INT_PROTO64_32(TI, TO, TINAME, TONAME, RMODE)\
	_1##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long##RMODE(_1##TI##32 x)\
	{\
	return (_1##TO##64)x;\
	}\
	_2##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long2##RMODE(_2##TI##32 x)\
	{\
	_2##TO##64 res;\
	_4##TI##32 param;\
	param.lo = x;\
	res = __builtin_astype(_mm_cvtep##TI##32_epi64(__builtin_astype(param,__m128i)),_2##TO##64);\
	return res;\
	}\
	TONAME##long3 __attribute__ ((overloadable)) convert_##TONAME##long3##RMODE(TINAME##int3 x)\
	{\
	_4##TO##64 res;\
	res.lo = __builtin_astype(_mm_cvtep##TI##32_epi64(__builtin_astype(as_##TONAME##int4(x),__m128i)),_2##TO##64);\
	res.hi = __builtin_astype(_mm_cvtep##TI##32_epi64(_mm_srli_si128(__builtin_astype(as_##TONAME##int4(x),__m128i), 8)),_2##TO##64);\
	return as_##TONAME##long3(res);\
	}\
	_4##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long4##RMODE(_4##TI##32 x)\
	{\
	_4##TO##64 res;\
	res.lo = __builtin_astype(_mm_cvtep##TI##32_epi64(__builtin_astype(x,__m128i)),_2##TO##64);\
	res.hi = __builtin_astype(_mm_cvtep##TI##32_epi64(_mm_srli_si128(__builtin_astype(x,__m128i), 8)),_2##TO##64);\
	return res;\
	}\
	_8##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long8##RMODE(_8##TI##32 x)\
	{\
	_8##TO##64 res;\
	res.lo.lo = __builtin_astype(_mm_cvtep##TI##32_epi64(__builtin_astype(x.lo,__m128i)),_2##TO##64);\
	res.lo.hi = __builtin_astype(_mm_cvtep##TI##32_epi64(_mm_srli_si128(__builtin_astype(x.lo,__m128i), 8)),_2##TO##64);\
	res.hi.lo = __builtin_astype(_mm_cvtep##TI##32_epi64(__builtin_astype(x.hi,__m128i)),_2##TO##64);\
	res.hi.hi = __builtin_astype(_mm_cvtep##TI##32_epi64(_mm_srli_si128(__builtin_astype(x.hi,__m128i), 8)),_2##TO##64);\
	return res;\
	}\
	_16##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long16##RMODE(_16##TI##32 x)\
	{\
	_16##TO##64 res;\
	res.lo.lo.lo = __builtin_astype(_mm_cvtep##TI##32_epi64(__builtin_astype(x.lo.lo,__m128i)),_2##TO##64);\
	res.lo.lo.hi = __builtin_astype(_mm_cvtep##TI##32_epi64(_mm_srli_si128(__builtin_astype(x.lo.lo,__m128i), 8)),_2##TO##64);\
	res.lo.hi.lo = __builtin_astype(_mm_cvtep##TI##32_epi64(__builtin_astype(x.lo.hi,__m128i)),_2##TO##64);\
	res.lo.hi.hi = __builtin_astype(_mm_cvtep##TI##32_epi64(_mm_srli_si128(__builtin_astype(x.lo.hi,__m128i), 8)),_2##TO##64);\
	res.hi.lo.lo = __builtin_astype(_mm_cvtep##TI##32_epi64(__builtin_astype(x.hi.lo,__m128i)),_2##TO##64);\
	res.hi.lo.hi = __builtin_astype(_mm_cvtep##TI##32_epi64(_mm_srli_si128(__builtin_astype(x.hi.lo,__m128i), 8)),_2##TO##64);\
	res.hi.hi.lo = __builtin_astype(_mm_cvtep##TI##32_epi64(__builtin_astype(x.hi.hi,__m128i)),_2##TO##64);\
	res.hi.hi.hi = __builtin_astype(_mm_cvtep##TI##32_epi64(_mm_srli_si128(__builtin_astype(x.hi.hi,__m128i), 8)),_2##TO##64);\
	return res;\
	}\

#define DEF_INT_PROTO64_64(TI, TO, TINAME, TONAME, RMODE)\
	DEF_INT_PROTO1_X_Y(_1##TI##64, _1##TO##64, TINAME##long, TONAME##long, RMODE)\
	DEF_INT_PROTO1_X_Y(_2##TI##64, _2##TO##64, TINAME##long2, TONAME##long2, RMODE)\
	DEF_INT_PROTO1_X_Y(TINAME##long3, TONAME##long3, TINAME##long3, TONAME##long3, RMODE)\
	DEF_INT_PROTO1_X_Y(_4##TI##64, _4##TO##64, TINAME##long4, TONAME##long4, RMODE)\
	DEF_INT_PROTO1_X_Y(_8##TI##64, _8##TO##64, TINAME##long8, TONAME##long8, RMODE)\
	DEF_INT_PROTO1_X_Y(_16##TI##64, _16##TO##64, TINAME##long16, TONAME##long16, RMODE)

#define DEF_INT_PROTO64_F(TO, TONAME, RMODE, RMODEVAL, CPUTYPE)\
	_1##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long##RMODE(float x)\
	{\
	_1##TO##64 res = __ocl_svml_##CPUTYPE##_cvtfpto##TO##64##RMODEVAL##nosatf1(x);\
	return res;\
	}\
	_2##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long2##RMODE(float2 x)\
	{\
    _4##TO##64 res = (TONAME##long4)(0);\
    float4 ftmp = (float4)(1.0);\
    ftmp.lo = x;\
    res = __ocl_svml_##CPUTYPE##_cvtfpto##TO##64##RMODEVAL##nosatf4(ftmp);\
	return res.lo;\
	}\
	TONAME##long3 __attribute__ ((overloadable)) convert_##TONAME##long3##RMODE(float3 x)\
	{\
	_4##TO##64 res = __ocl_svml_##CPUTYPE##_cvtfpto##TO##64##RMODEVAL##nosatf4(as_float4(x));\
	return res.s012;\
	}\
	_4##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long4##RMODE(float4 x)\
	{\
	_4##TO##64 res = __ocl_svml_##CPUTYPE##_cvtfpto##TO##64##RMODEVAL##nosatf4(x);\
	return res;\
	}\
	_8##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long8##RMODE(float8 x)\
	{\
	_8##TO##64 res = __ocl_svml_##CPUTYPE##_cvtfpto##TO##64##RMODEVAL##nosatf8(x);\
	return res;\
	}\
	_16##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long16##RMODE(float16 x)\
	{\
	_16##TO##64 res;\
	res.lo = __ocl_svml_##CPUTYPE##_cvtfpto##TO##64##RMODEVAL##nosatf8(x.lo);\
	res.hi = __ocl_svml_##CPUTYPE##_cvtfpto##TO##64##RMODEVAL##nosatf8(x.hi);\
	return res;\
	}\

#define DEF_INT_PROTO64_D(TO, TONAME, RMODE, RSVML, CPUTYPE)\
	_1##TO##64 __attribute__((overloadable)) convert_##TONAME##long##RMODE(double x)\
	{\
	return __ocl_svml_##CPUTYPE##_cvtfpto##TO##64##RSVML##nosat1(x);\
	}\
	_2##TO##64 __attribute__((overloadable)) convert_##TONAME##long2##RMODE(double2 x)\
	{\
    _4##TO##64 res = (TONAME##long4)(0);\
    double4 ftmp = (double4)(1.0);\
    ftmp.lo = x;\
	res = __ocl_svml_##CPUTYPE##_cvtfpto##TO##64##RSVML##nosat4(ftmp);\
	return res.lo;\
	}\
	TONAME##long3 __attribute__((overloadable)) convert_##TONAME##long3##RMODE(double3 x)\
	{\
	TONAME##long4 res = __ocl_svml_##CPUTYPE##_cvtfpto##TO##64##RSVML##nosat4(as_double4(x));\
	return as_##TONAME##long3(res);\
	}\
	_4##TO##64 __attribute__((overloadable)) convert_##TONAME##long4##RMODE(double4 x)\
	{\
	return __ocl_svml_##CPUTYPE##_cvtfpto##TO##64##RSVML##nosat4(x);\
	}\
	_8##TO##64 __attribute__((overloadable)) convert_##TONAME##long8##RMODE(double8 x)\
	{\
	return __ocl_svml_##CPUTYPE##_cvtfpto##TO##64##RSVML##nosat8(x);\
	}\
    _16##TO##64 __attribute__((overloadable)) convert_##TONAME##long16##RMODE(double16 x)\
	{\
	_16##TO##64 res;\
	res.lo = __ocl_svml_##CPUTYPE##_cvtfpto##TO##64##RSVML##nosat8(x.lo);\
	res.hi = __ocl_svml_##CPUTYPE##_cvtfpto##TO##64##RSVML##nosat8(x.hi);\
    return res;\
	}\

	//float
#define DEF_INT_PROTOF_8(TI, TINAME, RMODE)\
	float __attribute__ ((overloadable)) convert_float##RMODE(_1##TI##8 x)\
	{\
	return ((float)x);\
	}\
	float2 __attribute__ ((overloadable)) convert_float2##RMODE(_2##TI##8 x)\
	{\
	_16##TI##8 param;\
	param.s01 = x;\
	_4i32 t = convert_int4##RMODE(param.s0123);\
	float4 res = as_float4(_mm_cvtepi32_ps(__builtin_astype(t,__m128i)));\
	return res.lo;\
	}\
	float3 __attribute__ ((overloadable)) convert_float3##RMODE(TINAME##char3 x)\
	{\
	TINAME##char4 y = as_##TINAME##char4(x);\
	_4i32 t = convert_int4##RMODE(y);\
	float4 res = as_float4(_mm_cvtepi32_ps(__builtin_astype(t,__m128i)));\
	return as_float3(res);\
	}\
	float4 __attribute__ ((overloadable)) convert_float4##RMODE(_4##TI##8 x)\
	{\
	_4i32 t = convert_int4##RMODE(x);\
	float4 res = as_float4(_mm_cvtepi32_ps(__builtin_astype(t,__m128i)));\
	return res;\
	}\
	float8 __attribute__ ((overloadable)) convert_float8##RMODE(_8##TI##8 x)\
	{\
	_8i32 t = convert_int8##RMODE(x);\
	return convert_float8(t);\
	}\
	float16 __attribute__ ((overloadable)) convert_float16##RMODE(_16##TI##8 x)\
	{\
	_16i32 t = convert_int16##RMODE(x);\
	return convert_float16(t);\
	}\

#define DEF_INT_PROTOF_16(TI, TINAME, RMODE)\
	float __attribute__ ((overloadable)) convert_float##RMODE(_1##TI##16 x)\
	{\
	return ((float)x);\
	}\
	float2 __attribute__ ((overloadable)) convert_float2##RMODE(_2##TI##16 x)\
	{\
	_8##TI##16 param;\
	param.s01 = x;\
	_4i32 t = convert_int4##RMODE(param.s0123);\
	float4 res = as_float4(_mm_cvtepi32_ps(__builtin_astype(t,__m128i)));\
	return res.lo;\
	}\
	float3 __attribute__ ((overloadable)) convert_float3##RMODE(TINAME##short3 x)\
	{\
	TINAME##short4 y = as_##TINAME##short4(x);\
	_4i32 t = convert_int4##RMODE(y);\
	float4 res = as_float4(_mm_cvtepi32_ps(__builtin_astype(t,__m128i)));\
	return as_float3(res);\
	}\
	float4 __attribute__ ((overloadable)) convert_float4##RMODE(_4##TI##16 x)\
	{\
	_4i32 t = convert_int4##RMODE(x);\
	float4 res = as_float4(_mm_cvtepi32_ps(__builtin_astype(t,__m128i)));\
	return res;\
	}\
	float8 __attribute__ ((overloadable)) convert_float8##RMODE(_8##TI##16 x)\
	{\
	_8i32 t = convert_int8##RMODE(x);\
	return convert_float8(t);\
	}\
	float16 __attribute__ ((overloadable)) convert_float16##RMODE(_16##TI##16 x)\
	{\
	_16i32 t = convert_int16##RMODE(x);\
	return convert_float16(t);\
	}\

#define DEF_INT_PROTOF_I32_F816_AS_F4(RMODE, FLAGSAT)\
	float8 __attribute__ ((overloadable)) convert_float8##FLAGSAT##RMODE(_8i32 x)\
	{\
	float8 res;\
	res.lo = intToFloat(x.lo);\
	res.hi = intToFloat(x.hi);\
	return res;\
	}\
	float16 __attribute__ ((overloadable)) convert_float16##FLAGSAT##RMODE(_16i32 x)\
	{\
	float16 res;\
	res.lo.lo = intToFloat(x.lo.lo);\
	res.lo.hi = intToFloat(x.lo.hi);\
	res.hi.lo = intToFloat(x.hi.lo);\
	res.hi.hi = intToFloat(x.hi.hi);\
	return res;\
	}

#define DEF_INT_PROTOF_I32_F816_AS_F8(RMODE, FLAGSAT)\
	float8 __attribute__ ((overloadable)) convert_float8##FLAGSAT##RMODE(_8i32 x)\
	{\
	float8 res;\
	res = (float8) intToFloat8(x);\
	return res;\
	}\
	float16 __attribute__ ((overloadable)) convert_float16##FLAGSAT##RMODE(_16i32 x)\
	{\
	float16 res;\
	res.lo = intToFloat8(x.lo);\
	res.hi = intToFloat8(x.hi);\
	return res;\
	}

#define DEF_INT_PROTOF_I32_F1234_AS_F4(RMODE, FLAGSAT)\
	float __attribute__ ((overloadable)) convert_float##FLAGSAT##RMODE(_1i32 x)\
	{\
	_4i32 param;\
	param.s0 = x;\
	float4 res =  intToFloat(param);\
	return res.s0;\
	}\
	float2 __attribute__ ((overloadable)) convert_float2##FLAGSAT##RMODE(_2i32 x)\
	{\
	_4i32 param;\
	param.lo = x;\
	float4 res = intToFloat(param);\
	return res.lo;\
	}\
	float3 __attribute__ ((overloadable)) convert_float3##FLAGSAT##RMODE(int3 x)\
	{\
	float4 res;\
	_4i32 param;\
	param.s012 = x;\
	res = intToFloat(param);\
	return res.s012;\
	}\
	float4 __attribute__ ((overloadable)) convert_float4##FLAGSAT##RMODE(_4i32 x)\
	{\
	return intToFloat(x);\
	}

#if defined(__AVX__)
#define DEF_INT_PROTOF_I32NOSVML(RMODE, RMODEVAL, RSVML, CPUTYPE, FLAGSAT)           \
    DEF_INT_PROTOF_I32_F1234_AS_F4(RMODE, FLAGSAT)   \
    DEF_INT_PROTOF_I32_F816_AS_F8(RMODE, FLAGSAT)
#else // #if defined(__AVX__)
#define DEF_INT_PROTOF_I32NOSVML(RMODE, RMODEVAL, RSVML, CPUTYPE, FLAGSAT)           \
    DEF_INT_PROTOF_I32_F1234_AS_F4(RMODE, FLAGSAT)   \
    DEF_INT_PROTOF_I32_F816_AS_F4(RMODE, FLAGSAT)
#endif // #if defined(__AVX__)

// Oleg:
// convert_float(int)
// This is issue with ABI for SVML.
// So as a hack we call int4 version of conversions for float
// TODO: switch to DEF_INT_PROTOI32_FUSESVML_X when CSSD100012898:
// "ABI for passing float2 arguments to SVML conversions function is invalid" is fixed
#define DEF_INT_PROTOF_I32USESVML_12_AS_F4(RMODE, RMODEVAL, RSVML, CPUTYPE, FLAGSAT)\
    float __attribute__ ((overloadable)) convert_float##FLAGSAT##RMODE(int x)\
	{\
    int4 y = x;\
	return convert_float4##FLAGSAT##RMODE(y).s0;\
	}\
    float2 __attribute__ ((overloadable)) convert_float2##FLAGSAT##RMODE(int2 x)\
	{\
    int4 y;\
    y.lo = x;\
    y.hi = 0;\
	return convert_float4##FLAGSAT##RMODE(y).lo;\
	}

#define DEF_INT_PROTOF_I32USESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, WIDTHOCL, WIDTHSVML, FLAGSAT)\
    float##WIDTHOCL __attribute__ ((overloadable)) convert_float##WIDTHOCL##FLAGSAT##RMODE(int##WIDTHOCL  x)\
	{\
    float##WIDTHOCL res = __ocl_svml_##CPUTYPE##_cvti32tofp##RSVML##f##WIDTHSVML(x);\
	return res;\
	}

#define DEF_INT_PROTOF_I32USESVML(RMODE, RMODEVAL, RSVML, CPUTYPE, FLAGSAT)\
    DEF_INT_PROTOF_I32USESVML_12_AS_F4(RMODE, RMODEVAL, RSVML, CPUTYPE, FLAGSAT)\
    DEF_INT_PROTOF_I32USESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, 3, 3, FLAGSAT)\
    DEF_INT_PROTOF_I32USESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, 4, 4, FLAGSAT)\
    DEF_INT_PROTOF_I32USESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, 8, 8, FLAGSAT)\
    DEF_INT_PROTOF_I32USESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, 16, 16, FLAGSAT)

#define DEF_INT_PROTOF_U32NOSVML(RMODE, RMODEVAL, RSVML, CPUTYPE, FLAGSAT)\
    float __attribute__ ((overloadable)) convert_float##FLAGSAT##RMODE(_1u32 x)\
	{\
	float res = uintToFloat(x);\
	return res;\
	}\
	float2 __attribute__ ((overloadable)) convert_float2##FLAGSAT##RMODE(_2u32 x)\
	{\
	uint4 t;\
	t.lo = x;\
	return uintToFloat4(t).lo;\
	}\
	float3 __attribute__ ((overloadable)) convert_float3##FLAGSAT##RMODE(uint3 x)\
	{\
	uint4 t;\
	t.s012 = x;\
	return uintToFloat4(t).s012;\
	}\
	float4 __attribute__ ((overloadable)) convert_float4##FLAGSAT##RMODE(_4u32 x)\
	{\
	return uintToFloat4(x);\
	}\
	float8 __attribute__ ((overloadable)) convert_float8##FLAGSAT##RMODE(_8u32 x)\
	{\
	return uintToFloat8(x);\
	}\
	float16 __attribute__ ((overloadable)) convert_float16##FLAGSAT##RMODE(_16u32 x)\
	{\
	float16 res;\
	res.lo.lo = convert_float4##RMODE(x.lo.lo);\
	res.lo.hi = convert_float4##RMODE(x.lo.hi);\
	res.hi.lo = convert_float4##RMODE(x.hi.lo);\
	res.hi.hi = convert_float4##RMODE(x.hi.hi);\
	return res;\
	}

#define DEF_INT_PROTOF_U32USESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, WIDTHOCL, WIDTHSVML, FLAGSAT)\
    float##WIDTHOCL __attribute__ ((overloadable)) convert_float##WIDTHOCL##FLAGSAT##RMODE(_##WIDTHSVML##u32 x)\
	{\
    float##WIDTHOCL res = __ocl_svml_##CPUTYPE##_cvtu32tofp##RSVML##f##WIDTHSVML(x);\
	return res;\
	}

// Oleg:
// convert_float(uint)
// This is issue with ABI for SVML.
// So as a hack we call uint4 version of conversions for uint and uint2
// TODO: switch to DEF_INT_PROTOF_U32USESVML_X when CSSD100012897:
// "ABI for passing Uint and uint2 arguments to SVML conversions function is invalid" is fixed
#define DEF_INT_PROTOF_U32USESVML_1(RMODE, RMODEVAL, RSVML, CPUTYPE, WIDTHOCL, WIDTHSVML, FLAGSAT)\
    float __attribute__ ((overloadable)) convert_float##FLAGSAT##RMODE(_1u32 x)\
	{\
    uint4 y = x;\
	return convert_float4##FLAGSAT##RMODE(y).s0;\
	}

#define DEF_INT_PROTOF_U32USESVML_2(RMODE, RMODEVAL, RSVML, CPUTYPE, WIDTHOCL, WIDTHSVML, FLAGSAT)\
    float2 __attribute__ ((overloadable)) convert_float2##FLAGSAT##RMODE(_2u32 x)\
	{\
    uint4 y;\
    y.lo = x;\
    y.hi = x;\
	return convert_float4##FLAGSAT##RMODE(y).lo;\
	}

#define DEF_INT_PROTOF_U32USESVML(RMODE, RMODEVAL, RSVML, CPUTYPE, FLAGSAT)\
    DEF_INT_PROTOF_U32USESVML_1(RMODE, RMODEVAL, RSVML, CPUTYPE, , 1, FLAGSAT)\
    DEF_INT_PROTOF_U32USESVML_2(RMODE, RMODEVAL, RSVML, CPUTYPE, 2, 2, FLAGSAT)\
    DEF_INT_PROTOF_U32USESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, 3, 3, FLAGSAT)\
    DEF_INT_PROTOF_U32USESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, 4, 4, FLAGSAT)\
    DEF_INT_PROTOF_U32USESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, 8, 8, FLAGSAT)\
    DEF_INT_PROTOF_U32USESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, 16, 16, FLAGSAT)


// Oleg:
// TODO: switch convert_float to SVML direct call with long1 when CSSD100012898 is fixed
#define DEF_INT_PROTOF_64(TI, TINAME, RMODE, RSVML, CPUTYPE)\
	float __attribute__((overloadable)) convert_float##RMODE(_1##TI##64 x)\
	{\
	_2##TI##64 y = x;\
	return convert_float2##RMODE(y).lo;\
	}\
	float2 __attribute__((overloadable)) convert_float2##RMODE(_2##TI##64 x)\
	{\
    float4 res = (float4)(0.f);\
    _4##TI##64 ftmp = (TINAME##long4)(0);\
    ftmp.lo = x;\
	res = __ocl_svml_##CPUTYPE##_cvt##TI##64tofp##RSVML##f4(ftmp);\
	return res.lo;\
	}\
	float3 __attribute__((overloadable)) convert_float3##RMODE(TINAME##long3 x)\
	{\
	float4 res = __ocl_svml_##CPUTYPE##_cvt##TI##64tofp##RSVML##f4(as_##TINAME##long4(x));\
	return as_float3(res);\
	}\
	float4 __attribute__((overloadable)) convert_float4##RMODE(_4##TI##64 x)\
	{\
	float4 res = __ocl_svml_##CPUTYPE##_cvt##TI##64tofp##RSVML##f4(x);\
	return res;\
	}\
	float8 __attribute__((overloadable)) convert_float8##RMODE(_8##TI##64 x)\
	{\
	float8 res = __ocl_svml_##CPUTYPE##_cvt##TI##64tofp##RSVML##f8(x);\
	return res;\
	}\
	float16 __attribute__((overloadable)) convert_float16##RMODE(_16##TI##64 x)\
	{\
	float16 res;\
    res.lo = __ocl_svml_##CPUTYPE##_cvt##TI##64tofp##RSVML##f8(x.lo);\
    res.hi = __ocl_svml_##CPUTYPE##_cvt##TI##64tofp##RSVML##f8(x.hi);\
	return res;\
	}

#define DEF_INT_PROTOF_F(TI, TO, TINAME, TONAME, RMODE)\
	DEF_INT_PROTO1_X_Y(float, float, float, float, RMODE)\
	DEF_INT_PROTO1_X_Y(float2, float2, float2, float2, RMODE)\
	DEF_INT_PROTO1_X_Y(float3, float3, float3, float3, RMODE)\
	DEF_INT_PROTO1_X_Y(float4, float4, float4, float4, RMODE)\
	DEF_INT_PROTO1_X_Y(float8, float8, float8, float8, RMODE)\
	DEF_INT_PROTO1_X_Y(float16, float16,float16, float16, RMODE)


#define DEF_SAT_PROTOF_D(RMODE, RMODEVAL)\
	float __attribute__((overloadable)) convert_float##RMODE(double x)\
	{\
	int rm = getRound();\
	setRound((rm& ~_MM_ROUND_MASK) | RMODEVAL);\
	float res = doubleToFloatSat(x);\
	setRound(rm);\
	return res;\
	}\
	float2 __attribute__((overloadable)) convert_float2##RMODE(double2 x)\
	{\
	float2 res;\
	int rm = getRound();\
	setRound((rm& ~_MM_ROUND_MASK) | RMODEVAL);\
	res.lo = doubleToFloatSat(x.lo);\
	res.hi = doubleToFloatSat(x.hi);\
	setRound(rm);\
	return res;\
	}\
	float3 __attribute__((overloadable)) convert_float3##RMODE(double3 x)\
	{\
	float4 res;\
	int rm = getRound();\
	setRound((rm& ~_MM_ROUND_MASK) | RMODEVAL);\
	double4 y;\
	y.s012 = x;\
	res.lo.lo = doubleToFloatSat(y.lo.lo);\
	res.lo.hi = doubleToFloatSat(y.lo.hi);\
	res.hi.lo = doubleToFloatSat(y.hi.lo);\
	setRound(rm);\
	return as_float3(res);\
	}\
	float4 __attribute__((overloadable)) convert_float4##RMODE(double4 x)\
	{\
	float4 res;\
	int rm = getRound();\
	setRound((rm& ~_MM_ROUND_MASK) | RMODEVAL);\
	res.lo.lo = doubleToFloatSat(x.lo.lo);\
	res.lo.hi = doubleToFloatSat(x.lo.hi);\
	res.hi.lo = doubleToFloatSat(x.hi.lo);\
	res.hi.hi = doubleToFloatSat(x.hi.hi);\
	setRound(rm);\
	return res;\
	}\
	float8 __attribute__((overloadable)) convert_float8##RMODE##_double8(double8 x)\
	{\
	float8 res;\
	res.lo = convert_float4##RMODE(x.lo);\
	res.hi = convert_float4##RMODE(x.hi);\
	return res;\
	}\
	float16 __attribute__((overloadable)) convert_float16##RMODE##_##TINAME##double16(double16 x)\
	{\
	float16 res;\
	res.lo.lo = convert_float4##RMODE(x.lo.lo);\
	res.lo.hi = convert_float4##RMODE(x.lo.hi);\
	res.hi.lo = convert_float4##RMODE(x.hi.lo);\
	res.hi.hi = convert_float4##RMODE(x.hi.hi);\
	return res;\
	}\

#define DEF_INT_PROTOF_D_D12_AS_D2(RMODE, RMODEVAL)\
	float __attribute__((overloadable)) convert_float##RMODE(double x)\
	{\
	double2 param;\
    param.lo = x;\
    float4 res = double2ToFloat4( param, RMODEVAL);\
	return res.s0;\
	}\
	float2 __attribute__((overloadable)) convert_float2##RMODE(double2 x)\
	{\
	float4 res;\
	res = double2ToFloat4( x, RMODEVAL);\
	return res.lo;\
	}
	
#define DEF_INT_PROTOF_D_D34816_AS_D2(RMODE, RMODEVAL)\
    float3 __attribute__((overloadable)) convert_float3##RMODE(double3 x)\
	{\
	float4 res, t;\
	double4 y = as_double4(x);\
    t = double2ToFloat4( y.lo, RMODEVAL);\
    res.lo = t.lo;\
    t = double2ToFloat4( y.hi, RMODEVAL);\
    res.hi = t.lo;\
    return as_float3(res);\
	}\
    float4 __attribute__((overloadable)) convert_float4##RMODE(double4 x)\
	{\
	float4 res;\
	res.lo = convert_float2##RMODE(x.lo);\
	res.hi = convert_float2##RMODE(x.hi);\
	return res;\
	}\
	float8 __attribute__((overloadable)) convert_float8##RMODE(double8 x)\
	{\
	float8 res;\
	res.lo = convert_float4##RMODE(x.lo);\
	res.hi = convert_float4##RMODE(x.hi);\
	return res;\
	}\
	float16 __attribute__((overloadable)) convert_float16##RMODE(double16 x)\
	{\
	float16 res;\
	res.lo.lo = convert_float4##RMODE(x.lo.lo);\
	res.lo.hi = convert_float4##RMODE(x.lo.hi);\
	res.hi.lo = convert_float4##RMODE(x.hi.lo);\
	res.hi.hi = convert_float4##RMODE(x.hi.hi);\
	return res;\
	}

#define DEF_INT_PROTOF_D_D34816_AS_D4(RMODE, RMODEVAL)\
    float3 __attribute__((overloadable)) convert_float3##RMODE(double3 x)\
	{\
	float4 res, t;\
	double4 y = as_double4(x);\
    res = double4ToFloat4( y, RMODEVAL);\
    return as_float3(res);\
	}\
    float4 __attribute__((overloadable)) convert_float4##RMODE(double4 x)\
	{\
	float4 res;\
    res = double4ToFloat4( x, RMODEVAL);\
    return res;\
	}\
	float8 __attribute__((overloadable)) convert_float8##RMODE(double8 x)\
	{\
	float8 res;\
	res.lo = convert_float4##RMODE(x.lo);\
	res.hi = convert_float4##RMODE(x.hi);\
	return res;\
	}\
	float16 __attribute__((overloadable)) convert_float16##RMODE(double16 x)\
	{\
	float16 res;\
	res.lo.lo = convert_float4##RMODE(x.lo.lo);\
	res.lo.hi = convert_float4##RMODE(x.lo.hi);\
	res.hi.lo = convert_float4##RMODE(x.hi.lo);\
	res.hi.hi = convert_float4##RMODE(x.hi.hi);\
	return res;\
	}

#if defined(__AVX__)
#define DEF_INT_PROTOF_DNOSVML(RMODE, RMODEVAL, RSVML, CPUTYPE)\
    DEF_INT_PROTOF_D_D12_AS_D2(RMODE, RMODEVAL)\
    DEF_INT_PROTOF_D_D34816_AS_D4(RMODE, RMODEVAL)
#else // defined(__AVX__)
#define DEF_INT_PROTOF_DNOSVML(RMODE, RMODEVAL, RSVML, CPUTYPE)\
    DEF_INT_PROTOF_D_D12_AS_D2(RMODE, RMODEVAL)\
    DEF_INT_PROTOF_D_D34816_AS_D2(RMODE, RMODEVAL)
#endif // defined(__AVX__)

// Oleg:
// convert_float(double)
// This is issue with ABI for SVML.
// So as a hack we call double and double2 version of conversions
// TODO: switch double3,4,8,16 to SVML call when CSSD100012907:
// "ABI for passing double3 arguments to SVML conversions function is invalid" is fixed

#define DEF_INT_PROTOF_DUSESVML(RMODE, RMODEVAL, RSVML, CPUTYPE)\
	float __attribute__ ((overloadable)) convert_float##RMODE(double x)\
	{\
    float res = __ocl_svml_##CPUTYPE##_cvtfp64tofp32##RSVML##1(x);\
	return res;\
	}\
	float2 __attribute__ ((overloadable)) convert_float2##RMODE(double2 x)\
	{\
    float2 res = __ocl_svml_##CPUTYPE##_cvtfp64tofp32##RSVML##2(x);\
	return res;\
	}\
	float3 __attribute__ ((overloadable)) convert_float3##RMODE(double3 x)\
	{\
    float3 res;\
    double2 y1,y2;\
    y1 = x.s01;\
    y2.lo = x.s2;\
    float2 r1 = convert_float2##RMODE(y1);\
    float2 r2 = convert_float2##RMODE(y2);\
    res.s01 = r1;\
    res.s2 = r2.lo;\
    return res;\
	}\
	float4 __attribute__ ((overloadable)) convert_float4##RMODE(double4 x)\
    {\
	float4 res;\
	res.lo = convert_float2##RMODE(x.lo);\
	res.hi = convert_float2##RMODE(x.hi);\
	return res;\
	}\
	float8 __attribute__ ((overloadable)) convert_float8##RMODE(double8 x)\
    {\
	float8 res;\
	res.lo = convert_float4##RMODE(x.lo);\
	res.hi = convert_float4##RMODE(x.hi);\
	return res;\
	}\
	float16 __attribute__ ((overloadable)) convert_float16##RMODE(double16 x)\
    {\
	float16 res;\
	res.lo = convert_float8##RMODE(x.lo);\
	res.hi = convert_float8##RMODE(x.hi);\
	return res;\
	}

		//double
#define DEF_INT_PROTOD_8(TI, TINAME, RMODE)\
	double __attribute__((overloadable)) convert_double##RMODE(_1##TI##8 x)\
	{\
	return ((double)x);\
	}\
	double2 __attribute__((overloadable)) convert_double2##RMODE(_2##TI##8 x)\
	{\
	_16##TI##8 param;\
	param.s01 = x;\
	_4i32 t = convert_int4##RMODE(param.s0123);\
	double2 res = as_double2(_mm_cvtepi32_pd(__builtin_astype(t,__m128i)));\
	return res;\
	}\
	double3 __attribute__((overloadable)) convert_double3##RMODE(TINAME##char3 x)\
	{\
	TINAME##char4 y = as_##TINAME##char4(x);\
	_4i32 t = convert_int4##RMODE(y);\
	double4 res;\
	res.lo = as_double2(_mm_cvtepi32_pd(__builtin_astype(t,__m128i)));\
	t = as_int4(_mm_srli_si128(__builtin_astype(t,__m128i), 8));\
	res.hi = as_double2(_mm_cvtepi32_pd(__builtin_astype(t,__m128i)));\
	return as_double3(res);\
	}\
	double4 __attribute__((overloadable)) convert_double4##RMODE(_4##TI##8 x)\
	{\
	_4i32 t = convert_int4##RMODE(x);\
	double4 res;\
	res.lo = as_double2(_mm_cvtepi32_pd(__builtin_astype(t,__m128i)));\
	t = as_int4(_mm_srli_si128(__builtin_astype(t,__m128i), 8));\
	res.hi = as_double2(_mm_cvtepi32_pd(__builtin_astype(t,__m128i)));\
	return res;\
	}\
	double8 __attribute__((overloadable)) convert_double8##RMODE(_8##TI##8 x)\
	{\
	double8 res;\
	res.lo = convert_double4##RMODE(x.lo);\
	res.hi = convert_double4##RMODE(x.hi);\
	return res;\
	}\
	double16 __attribute__((overloadable)) convert_double16##RMODE(_16##TI##8 x)\
	{\
	double16 res;\
	res.lo = convert_double8##RMODE(x.lo);\
	res.hi = convert_double8##RMODE(x.hi);\
	return res;\
	}\

#define DEF_INT_PROTOD_16(TI, TINAME, RMODE)\
	double __attribute__((overloadable)) convert_double##RMODE(_1##TI##16 x)\
	{\
	return ((double)x);\
	}\
	double2 __attribute__((overloadable)) convert_double2##RMODE(_2##TI##16 x)\
	{\
	_8##TI##16 param;\
	double2 res;\
	param.s01 = x;\
	_4i32 t = convert_int4##RMODE(param.s0123);\
	res = as_double2(_mm_cvtepi32_pd(__builtin_astype(t,__m128i)));\
	return res;\
	}\
	double3 __attribute__((overloadable)) convert_double3##RMODE(TINAME##short3 x)\
	{\
	double4 res;\
	TINAME##short4 y = as_##TINAME##short4(x);\
	res.lo = convert_double2##RMODE(y.lo);\
	res.hi = convert_double2##RMODE(y.hi);\
	return as_double3(res);\
	}\
	double4 __attribute__((overloadable)) convert_double4##RMODE(_4##TI##16 x)\
	{\
	double4 res;\
	res.lo = convert_double2##RMODE(x.lo);\
	res.hi = convert_double2##RMODE(x.hi);\
	return res;\
	}\
	double8 __attribute__((overloadable)) convert_double8##RMODE(_8##TI##16 x)\
	{\
	double8 res;\
	res.lo =  convert_double4##RMODE(x.lo);\
	res.hi =  convert_double4##RMODE(x.hi);\
	return res;\
	}\
	double16 __attribute__((overloadable)) convert_double16##RMODE(_16##TI##16 x)\
	{\
	double16 res;\
	res.lo =  convert_double8##RMODE(x.lo);\
	res.hi =  convert_double8##RMODE(x.hi);\
	return res;\
	}\

#define DEF_INT_PROTOD_I32(RMODE, RMODEVAL)\
	double __attribute__((overloadable)) convert_double##RMODE(_1i32 x)\
	{\
	double2 res;\
	int rm = getRound();\
	setRound((rm& ~_MM_ROUND_MASK) | RMODEVAL);\
	_4i32 param;\
	param.s0 = x;\
	res =  as_double2(_mm_cvtepi32_pd(__builtin_astype(param,__m128i)));\
	setRound(rm);\
	return res.lo;\
	}\
	double2 __attribute__((overloadable)) convert_double2##RMODE(_2i32 x)\
	{\
	_4i32 param;\
	double2 res;\
	param.lo = x;\
	int rm = getRound();\
	setRound((rm& ~_MM_ROUND_MASK) | RMODEVAL);\
	res = as_double2(_mm_cvtepi32_pd(__builtin_astype(param,__m128i)));\
	setRound(rm);\
	return res;\
	}\
	double3 __attribute__((overloadable)) convert_double3##RMODE(int3 x)\
	{\
	double4 res;\
	int rm = getRound();\
	int4 y = as_int4(x);\
	setRound((rm& ~_MM_ROUND_MASK) | RMODEVAL);\
	res.lo = as_double2(_mm_cvtepi32_pd(__builtin_astype(y,__m128i)));\
	y = as_int4(_mm_srli_si128(__builtin_astype(y,__m128i), 8));\
	res.hi = as_double2(_mm_cvtepi32_pd(__builtin_astype(y,__m128i)));\
	setRound(rm);\
	return as_double3(res);\
	}\
	double4 __attribute__((overloadable)) convert_double4##RMODE(_4i32 x)\
	{\
	double4 res;\
	int rm = getRound();\
	setRound((rm& ~_MM_ROUND_MASK) | RMODEVAL);\
	res.lo = as_double2(_mm_cvtepi32_pd(__builtin_astype(x,__m128i)));\
	x = as_int4(_mm_srli_si128(__builtin_astype(x,__m128i), 8));\
	res.hi = as_double2(_mm_cvtepi32_pd(__builtin_astype(x,__m128i)));\
	setRound(rm);\
	return res;\
	}\
	double8 __attribute__((overloadable)) convert_double8##RMODE(_8i32 x)\
	{\
	double8 res;\
	int rm = getRound();\
	setRound((rm& ~_MM_ROUND_MASK) | RMODEVAL);\
	res.lo.lo = as_double2(_mm_cvtepi32_pd(__builtin_astype(x.lo,__m128i)));\
	x.lo = as_int4(_mm_srli_si128(__builtin_astype(x.lo,__m128i), 8));\
	res.lo.hi = as_double2(_mm_cvtepi32_pd(__builtin_astype(x.lo,__m128i)));\
	res.hi.lo = as_double2(_mm_cvtepi32_pd(__builtin_astype(x.hi,__m128i)));\
	x.hi = as_int4(_mm_srli_si128(__builtin_astype(x.hi,__m128i), 8));\
	res.hi.hi = as_double2(_mm_cvtepi32_pd(__builtin_astype(x.hi,__m128i)));\
	setRound(rm);\
	return res;\
	}\
	double16 __attribute__((overloadable)) convert_double16##RMODE(_16i32 x)\
	{\
	double16 res;\
	res.lo = convert_double8##RMODE(x.lo);\
	res.hi = convert_double8##RMODE(x.hi);\
	return res;\
	}

// The scalar version calls a 'custom' implementation as a workaround.
// See comment before implementation of convert_double
#define DEF_INT_PROTOD_U32(RMODE, RSVML, CPUTYPE) \
	double __attribute__((overloadable)) convert_double##RMODE(_1u32 x)\
	{\
    _4u32 X;\
    X.s0 = x;\
    double4 res;\
    res = __ocl_svml_##CPUTYPE##_cvtu32tofp##RSVML##4(X);\
    return res.s0;\
	}\
	double2 __attribute__((overloadable)) convert_double2##RMODE(_2u32 x)\
	{\
    _4u32 X;\
    X.lo = x;\
    double4 res;\
    res = __ocl_svml_##CPUTYPE##_cvtu32tofp##RSVML##4(X);\
    return res.lo;\
	}\
	double3 __attribute__((overloadable)) convert_double3##RMODE(uint3 x)\
	{\
	double4 res = __ocl_svml_##CPUTYPE##_cvtu32tofp##RSVML##4(as_uint4(x));\
	return as_double3(res);\
	}\
	double4 __attribute__((overloadable)) convert_double4##RMODE(_4u32 x)\
	{\
	double4 res = __ocl_svml_##CPUTYPE##_cvtu32tofp##RSVML##4(x);\
	return res;\
	}\
	double8 __attribute__((overloadable)) convert_double8##RMODE(_8u32 x)\
	{\
	double8 res = __ocl_svml_##CPUTYPE##_cvtu32tofp##RSVML##8(x);\
	return res;\
	}\
	double16 __attribute__((overloadable)) convert_double16##RMODE(_16u32 x)\
	{\
	double16 res;\
    res.lo = __ocl_svml_##CPUTYPE##_cvtu32tofp##RSVML##8(x.lo);\
    res.hi = __ocl_svml_##CPUTYPE##_cvtu32tofp##RSVML##8(x.hi);\
	return res;\
	}
	

// Oleg:
// TODO: switch convert_double to SVML direct call with long1 when CSSD100012898 is fixed
#define DEF_INT_PROTOD_64(TI, TINAME, RMODE, RSVML, CPUTYPE)\
	double __attribute__((overloadable)) convert_double##RMODE(_1##TI##64 x)\
	{\
	_2##TI##64 y = x;\
	return convert_double2##RMODE(y).s0;\
	}\
	double2 __attribute__((overloadable)) convert_double2##RMODE(_2##TI##64 x)\
	{\
    _4##TI##64 param = (TINAME##long4)(0);\
	param.lo=x;\
	double4 res = __ocl_svml_##CPUTYPE##_cvt##TI##64tofp##RSVML##4(param);\
	return res.lo;\
	}\
	double3 __attribute__((overloadable)) convert_double3##RMODE(TINAME##long3 x)\
	{\
	double4 res = __ocl_svml_##CPUTYPE##_cvt##TI##64tofp##RSVML##4(as_##TINAME##long4(x));\
	return as_double3(res);\
	}\
	double4 __attribute__((overloadable)) convert_double4##RMODE(_4##TI##64 x)\
	{\
	double4 res = __ocl_svml_##CPUTYPE##_cvt##TI##64tofp##RSVML##4(x);\
	return res;\
	}\
	double8 __attribute__((overloadable)) convert_double8##RMODE(_8##TI##64 x)\
	{\
	double8 res = __ocl_svml_##CPUTYPE##_cvt##TI##64tofp##RSVML##8(x);\
	return res;\
	}\
	double16 __attribute__((overloadable)) convert_double16##RMODE(_16##TI##64 x)\
	{\
	double16 res;\
    res.lo = __ocl_svml_##CPUTYPE##_cvt##TI##64tofp##RSVML##8(x.lo);\
    res.hi = __ocl_svml_##CPUTYPE##_cvt##TI##64tofp##RSVML##8(x.hi);\
	return res;\
	}\

#define DEF_INT_PROTOD_F(RMODE)\
	double __attribute__((overloadable)) convert_double##RMODE(float x)\
	{\
	float4 param;\
	double2 res;\
	param.s0 = x;\
	res = _mm_cvtps_pd(param);\
	return res.lo;\
	}\
	double2 __attribute__((overloadable)) convert_double2##RMODE(float2 x)\
	{\
	float4 param;\
	double2 res;\
	param.lo = x;\
	res = _mm_cvtps_pd(param);\
	return res;\
	}\
	double3 __attribute__((overloadable)) convert_double3##RMODE(float3 x)\
	{\
	double4 res;\
	float4 y = as_float4(x);\
	res.lo = as_double2(_mm_cvtps_pd(__builtin_astype(y,__m128)));\
	y = as_float4(_mm_srli_si128(__builtin_astype(y,__m128i), 8));\
	res.hi = as_double2(_mm_cvtps_pd(__builtin_astype(y,__m128)));\
	return as_double3(res);\
	}\
	double4 __attribute__((overloadable)) convert_double4##RMODE(float4 x)\
	{\
	double4 res;\
	res.lo = as_double2(_mm_cvtps_pd(__builtin_astype(x,__m128)));\
	x = as_float4(_mm_srli_si128(__builtin_astype(x,__m128i), 8));\
	res.hi = as_double2(_mm_cvtps_pd(__builtin_astype(x,__m128)));\
	return res;\
	}\
	double8 __attribute__((overloadable)) convert_double8##RMODE(float8 x)\
	{\
	double8 res;\
	res.lo = convert_double4##RMODE(x.lo);\
	res.hi = convert_double4##RMODE(x.hi);\
	return res;\
	}\
	double16 __attribute__((overloadable)) convert_double16##RMODE(float16 x)\
	{\
	double16 res;\
	res.lo = convert_double8##RMODE(x.lo);\
	res.hi = convert_double8##RMODE(x.hi);\
	return res;\
	}\

#define DEF_INT_PROTOD_D(RMODE)\
	DEF_INT_PROTO1_X_Y(double, double, double, double, RMODE)\
	DEF_INT_PROTO1_X_Y(double2, double2, double2, double2, RMODE)\
	DEF_INT_PROTO1_X_Y(double3, double3, double3, double3, RMODE)\
	DEF_INT_PROTO1_X_Y(double4, double4, double4, double4, RMODE)\
	DEF_INT_PROTO1_X_Y(double8, double8, double8, double8, RMODE)\
	DEF_INT_PROTO1_X_Y(double16, double16, double16, double16, RMODE)
	/////////////////////////////  SAT  ///////////////////////////////
	// 8 bits
#define DEF_SAT_PROTO8_I16(TO, TONAME, RMODE, MAX, MIN)\
	_1##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char_sat##RMODE(_1i16 x)\
	{\
	if(x > MAX) return MAX;\
	if(x < MIN) return MIN;\
	return (_1##TO##8)x;\
	}\
	_2##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char2_sat##RMODE(_2i16 x)\
	{\
	_16##TO##8 res;\
	_8i16 param;\
	param.s01 = x;\
	res = as_##TONAME##char16(_mm_pack##TONAME##s_epi16(__builtin_astype(param,__m128i), __builtin_astype(param,__m128i)));\
	return res.s01;\
	}\
	TONAME##char3 __attribute__ ((overloadable)) convert_##TONAME##char3_sat##RMODE(short3 x)\
	{\
	_16##TO##8 res;\
	_8i16 param;\
	param.lo = as_short4(x);\
	res = as_##TONAME##char16(_mm_pack##TONAME##s_epi16(__builtin_astype(param,__m128i), __builtin_astype(param,__m128i)));\
	return as_##TONAME##char3(res.s0123);\
	}\
	_4##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char4_sat##RMODE(_4i16 x)\
	{\
	_16##TO##8 res;\
	_8i16 param;\
	param.lo = x;\
	res = as_##TONAME##char16(_mm_pack##TONAME##s_epi16(__builtin_astype(param,__m128i), __builtin_astype(param,__m128i)));\
	return res.s0123;\
	}\
	_8##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char8_sat##RMODE(_8i16 x)\
	{\
	_16##TO##8 res;\
	res = as_##TONAME##char16(_mm_pack##TONAME##s_epi16(__builtin_astype(x,__m128i), __builtin_astype(x,__m128i)));\
	return res.lo;\
	}\
	_16##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char16_sat##RMODE(_16i16 x)\
	{\
	_16##TO##8 res, temp1, temp2;\
	res = as_##TONAME##char16(_mm_pack##TONAME##s_epi16(__builtin_astype(x.lo,__m128i), __builtin_astype(x.hi,__m128i)));\
	return res;\
	}\

#define DEF_SAT_PROTO8_I32(TO, TONAME, RMODE, MAX, MIN)\
	_1##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char_sat##RMODE(_1i32 x)\
	{\
	if(x > MAX) x = MAX;\
	if(x < MIN) x = MIN;\
	return (_1##TO##8)x;\
	}\
	_2##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char2_sat##RMODE(_2i32 x)\
	{\
	_16##TO##8 res;\
	_4i32 param;\
	param.lo = x;\
	res = as_##TONAME##char16(_mm_packs_epi32(__builtin_astype(param,__m128i), __builtin_astype(param,__m128i)));\
	res = as_##TONAME##char16(_mm_pack##TONAME##s_epi16(__builtin_astype(res,__m128i), __builtin_astype(res,__m128i)));\
	return res.s01;\
	}\
	TONAME##char3 __attribute__ ((overloadable)) convert_##TONAME##char3_sat##RMODE(int3 x)\
	{\
	_16##TO##8 res;\
	int4 y = as_int4(x);\
	res = as_##TONAME##char16(_mm_packs_epi32(__builtin_astype(y,__m128i), __builtin_astype(y,__m128i)));\
	res = as_##TONAME##char16(_mm_pack##TONAME##s_epi16(__builtin_astype(res,__m128i), __builtin_astype(res,__m128i)));\
	return as_##TONAME##char3(res.s0123);\
	}\
	_4##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char4_sat##RMODE(_4i32 x)\
	{\
	_16##TO##8 res;\
	res = as_##TONAME##char16(_mm_packs_epi32(__builtin_astype(x,__m128i), __builtin_astype(x,__m128i)));\
	res = as_##TONAME##char16(_mm_pack##TONAME##s_epi16(__builtin_astype(res,__m128i), __builtin_astype(res,__m128i)));\
	return res.s0123;\
	}\
	_8##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char8_sat##RMODE(_8i32 x)\
	{\
	_16##TO##8 res;\
	res = as_##TONAME##char16(_mm_packs_epi32(__builtin_astype(x.lo,__m128i), __builtin_astype(x.hi,__m128i)));\
	res = as_##TONAME##char16(_mm_pack##TONAME##s_epi16(__builtin_astype(res,__m128i), __builtin_astype(res,__m128i)));\
	return res.lo;\
	}\
	_16##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char16_sat##RMODE(_16i32 x)\
	{\
	_16##TO##8 res, temp1, temp2;\
	temp1 = as_##TONAME##char16(_mm_packs_epi32(__builtin_astype(x.lo.lo,__m128i), __builtin_astype(x.lo.hi,__m128i)));\
	temp2 = as_##TONAME##char16(_mm_packs_epi32(__builtin_astype(x.hi.lo,__m128i), __builtin_astype(x.hi.hi,__m128i)));\
	res   = as_##TONAME##char16(_mm_pack##TONAME##s_epi16(__builtin_astype(temp1,__m128i), __builtin_astype(temp2,__m128i)));\
	return res;\
	}\

#define DEF_SAT_PROTO8_I64(TO, TONAME, RMODE, MAX, MIN)\
	_1##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char_sat##RMODE(_1i64 x)\
	{\
	if(x > MAX) return MAX;\
	if(x < MIN) return MIN;\
	return (_1##TO##8)x;\
	}\
	_2##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char2_sat##RMODE(_2i64 x)\
	{\
	_16##TO##8 res;\
	res.s0 = convert_##TONAME##char_sat##RMODE(x.lo);\
	res.s1 = convert_##TONAME##char_sat##RMODE(x.hi);\
	return res.s01;\
	}\
	TONAME##char3 __attribute__ ((overloadable)) convert_##TONAME##char3_sat##RMODE(long3 x)\
	{\
	_16##TO##8 res;\
	long4 y = as_long4(x);\
	res.s01 = convert_##TONAME##char2_sat##RMODE(y.lo);\
	res.s23 = convert_##TONAME##char2_sat##RMODE(y.hi);\
	return as_##TONAME##char3(res.s0123);\
	}\
	_4##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char4_sat##RMODE(_4i64 x)\
	{\
	_16##TO##8 res;\
	res.s01 = convert_##TONAME##char2_sat##RMODE(x.lo);\
	res.s23 = convert_##TONAME##char2_sat##RMODE(x.hi);\
	return res.s0123;\
	}\
	_8##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char8_sat##RMODE(_8i64 x)\
	{\
	_16##TO##8 res;\
	res.s0123 = convert_##TONAME##char4_sat##RMODE(x.lo);\
	res.s4567 = convert_##TONAME##char4_sat##RMODE(x.hi);\
	return res.lo;\
	}\
	_16##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char16_sat##RMODE(_16i64 x)\
	{\
	_16##TO##8 res;\
	res.lo = convert_##TONAME##char8_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##char8_sat##RMODE(x.hi);\
	return res;\
	}

#define DEF_SAT_PROTO8_F_F1234_AS_F4_F16_AS_F8(TO, TONAME, RMODE, RMODEVAL, MAX, MIN, FLAG)\
	_1##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char_sat##RMODE(float x)\
	{\
    float4 y;\
    y.s0 = x;\
	return convert_##TONAME##char4_sat##RMODE(y).s0;\
	}\
	_2##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char2_sat##RMODE(float2 x)\
	{\
	float4 y;\
    y.lo = x;\
	return convert_##TONAME##char4_sat##RMODE(y).lo;\
	}\
	TONAME##char3 __attribute__ ((overloadable)) convert_##TONAME##char3_sat##RMODE(float3 x)\
	{\
	float4 y;\
    y.s012= x;\
	return convert_##TONAME##char4_sat##RMODE(y).s012;\
	}\
	_4##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char4_sat##RMODE(float4 x)\
	{\
    float4 MIN4 = MIN;\
    float4 MAX4 = MAX;\
    float4 maxx =  _mm_max_ps(x,MIN4);\
    float4 y = _mm_min_ps(maxx, MAX4);\
    return __builtin_astype(convert_char4##RMODE(y), _4##TO##8);\
	}\
    _16##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char16_sat##RMODE(float16 x)\
	{\
	_16##TO##8 res;\
	res.lo = convert_##TONAME##char8_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##char8_sat##RMODE(x.hi);\
	return res;\
	}

#define DEF_SAT_PROTO8_F_F8_AS_F4(TO, TONAME, RMODE, RMODEVAL, MAX, MIN, FLAG)\
_8##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char8_sat##RMODE(float8 x)\
	{\
	_8##TO##8 res;\
	res.lo = convert_##TONAME##char4_sat##RMODE(x.lo);\
    res.hi = convert_##TONAME##char4_sat##RMODE(x.hi);\
	return res;\
	}

#define DEF_SAT_PROTO8_F_F8_AS_F8(TO, TONAME, RMODE, RMODEVAL, MAX, MIN, FLAG)\
_8##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char8_sat##RMODE(float8 x)\
    {\
    float8 MIN8 = MIN;\
    float8 MAX8 = MAX;\
    float8 maxx =  _mm256_max_ps(x,MIN8);\
    float8 y = _mm256_min_ps(maxx, MAX8);\
    return __builtin_astype(convert_char8##RMODE(y), _8##TO##8);\
    }

#if defined(__AVX__)
#define DEF_SAT_PROTO8_F(TO, TONAME, RMODE, RMODEVAL, MAX, MIN, FLAG)\
    DEF_SAT_PROTO8_F_F1234_AS_F4_F16_AS_F8(TO, TONAME, RMODE, RMODEVAL, MAX, MIN, FLAG)\
    DEF_SAT_PROTO8_F_F8_AS_F8(TO, TONAME, RMODE, RMODEVAL, MAX, MIN, FLAG)
#else
#define DEF_SAT_PROTO8_F(TO, TONAME, RMODE, RMODEVAL, MAX, MIN, FLAG)\
    DEF_SAT_PROTO8_F_F1234_AS_F4_F16_AS_F8(TO, TONAME, RMODE, RMODEVAL, MAX, MIN, FLAG)\
    DEF_SAT_PROTO8_F_F8_AS_F4(TO, TONAME, RMODE, RMODEVAL, MAX, MIN, FLAG)
#endif

#define DEF_SAT_PROTO8_D(TO, TONAME, RMODE, RMODEVAL)\
	_1##TO##8 __attribute__((overloadable)) convert_##TONAME##char_sat##RMODE(double x)\
	{\
    _1##TO##32 t = convert_##TONAME##int_sat##RMODE(x);\
	return convert_##TONAME##char_sat(t);\
	}\
	_2##TO##8 __attribute__((overloadable)) convert_##TONAME##char2_sat##RMODE(double2 x)\
	{\
		_2##TO##8 res;\
		res.lo = convert_##TONAME##char_sat##RMODE(x.lo);\
		res.hi = convert_##TONAME##char_sat##RMODE(x.hi);\
		return res;\
	}\
	TONAME##char3 __attribute__((overloadable)) convert_##TONAME##char3_sat##RMODE(double3 x)\
	{\
		_3##TO##8 res;\
		res.s01 = convert_##TONAME##char2_sat##RMODE(x.s01);\
		res.s2 = convert_##TONAME##char_sat##RMODE(x.s2);\
		return res;\
	}\
	_4##TO##8 __attribute__((overloadable)) convert_##TONAME##char4_sat##RMODE(double4 x)\
	{\
		_4##TO##8 res;\
		res.lo = convert_##TONAME##char2_sat##RMODE(x.lo);\
		res.hi = convert_##TONAME##char2_sat##RMODE(x.hi);\
		return res;\
	}\
	_8##TO##8 __attribute__((overloadable)) convert_##TONAME##char8_sat##RMODE(double8 x)\
	{\
	_16##TO##8 res;\
	res.s0123 = convert_##TONAME##char4_sat##RMODE(x.lo);\
	res.s4567 = convert_##TONAME##char4_sat##RMODE(x.hi);\
	return res.lo;\
	}\
	_16##TO##8 __attribute__((overloadable)) convert_##TONAME##char16_sat##RMODE(double16 x)\
	{\
	_16##TO##8 res;\
	res.lo = convert_##TONAME##char8_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##char8_sat##RMODE(x.hi);\
	return res;\
	}\

	// 16 bits
#define DEF_SAT_PROTO16_8(TI, TO, TINAME, TONAME, RMODE)\
	_1##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short##RMODE(_1##TI##8 x)\
	{\
	if (x < 0) return 0;\
	return (_1##TO##16)x;\
	}\
	_2##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short2##RMODE(_2##TI##8 x)\
	{\
	_8##TO##16 res;\
	_8##TI##16 tmp;\
	_16##TI##8 param;\
	param.s01 = x;\
	tmp = __builtin_astype(_mm_cvtep##TI##8_epi16(__builtin_astype(param,__m128i)),_8##TI##16);\
	res = convert_##TONAME##short8##RMODE(tmp);\
	return res.s01;\
	}\
	TONAME##short3 __attribute__ ((overloadable)) convert_##TONAME##short3##RMODE(TINAME##char3 x)\
	{\
	_8##TO##16 res;\
	_16##TI##8 param;\
	_8##TI##16 tmp;\
	param.s012 = x;\
	tmp = __builtin_astype(_mm_cvtep##TI##8_epi16(__builtin_astype(param,__m128i)),_8##TI##16);\
	res = convert_##TONAME##short8##RMODE(tmp);\
	return res.s012;\
	}\
	_4##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short4##RMODE(_4##TI##8 x)\
	{\
	_8##TO##16 res;\
	_16##TI##8 param;\
	_8##TI##16 tmp;\
	param.s0123 = x;\
	tmp = __builtin_astype(_mm_cvtep##TI##8_epi16(__builtin_astype(param,__m128i)),_8##TI##16);\
	res = convert_##TONAME##short8##RMODE(tmp);\
	return res.s0123;\
	}\
	_8##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short8##RMODE(_8##TI##8 x)\
	{\
	_8##TO##16 res;\
	_16##TI##8 param;\
	_8##TI##16 tmp;\
	param.lo = x;\
	tmp = __builtin_astype(_mm_cvtep##TI##8_epi16(__builtin_astype(param,__m128i)),_8##TI##16);\
	res = convert_##TONAME##short8##RMODE(tmp);\
	return res;\
	}\
	_16##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short16##RMODE(_16##TI##8 x)\
	{\
	_16##TO##16 res;\
	_8##TI##16 tmp;\
	tmp = __builtin_astype(_mm_cvtep##TI##8_epi16(__builtin_astype(x,__m128i)),_8##TI##16);\
	res.lo = convert_##TONAME##short8##RMODE(tmp);\
	tmp = __builtin_astype(_mm_cvtep##TI##8_epi16(_mm_srli_si128(__builtin_astype(x,__m128i), 8)),_8##TI##16);\
	res.hi = convert_##TONAME##short8##RMODE(tmp);\
	return res;\
	}\

#define DEF_SAT_PROTO16_I32(TO, TONAME, RMODE, MAX, MIN)\
	_1##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short_sat##RMODE(_1i32 x)\
	{\
	if(x > MAX) return MAX;\
	if(x < MIN) return MIN;\
	return (_1##TO##16)x;\
	}\
	_2##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short2_sat##RMODE(_2i32 x)\
	{\
	_8##TO##16 res;\
	_4i32 param;\
	param.lo = x;\
	res = __builtin_astype(_mm_pack##TONAME##s_epi32(__builtin_astype(param,__m128i), __builtin_astype(param,__m128i)),_8##TO##16);\
	return res.s01;\
	}\
	TONAME##short3 __attribute__ ((overloadable)) convert_##TONAME##short3_sat##RMODE(int3 x)\
	{\
	_8##TO##16 res;\
	int4 y = as_int4(x);\
	res = __builtin_astype(_mm_pack##TONAME##s_epi32(__builtin_astype(y,__m128i), __builtin_astype(y,__m128i)),_8##TO##16);\
	return as_##TONAME##short3(res.lo);\
	}\
	_4##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short4_sat##RMODE(_4i32 x)\
	{\
	_8##TO##16 res;\
	res = __builtin_astype(_mm_pack##TONAME##s_epi32(__builtin_astype(x,__m128i), __builtin_astype(x,__m128i)),_8##TO##16);\
	return res.lo;\
	}\
	_8##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short8_sat##RMODE(_8i32 x)\
	{\
	_8##TO##16 res;\
	res = __builtin_astype(_mm_pack##TONAME##s_epi32(__builtin_astype(x.lo,__m128i), __builtin_astype(x.hi,__m128i)),_8##TO##16);\
	return res;\
	}\
	_16##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short16_sat##RMODE(_16i32 x)\
	{\
	_16##TO##16 res;\
	res.lo = __builtin_astype(_mm_pack##TONAME##s_epi32(__builtin_astype(x.lo.lo,__m128i), __builtin_astype(x.lo.hi,__m128i)),_8##TO##16);\
	res.hi = __builtin_astype(_mm_pack##TONAME##s_epi32(__builtin_astype(x.hi.lo,__m128i), __builtin_astype(x.hi.hi,__m128i)),_8##TO##16);\
	return res;\
	}\

#define DEF_SAT_PROTO16_I64(TO, TONAME, RMODE, MAX, MIN)\
	_1##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short_sat##RMODE(_1i64 x)\
	{\
	if(x > MAX) return MAX;\
	if(x < MIN) return MIN;\
	return (_1##TO##16)x;\
	}\
	_2##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short2_sat##RMODE(_2i64 x)\
	{\
	_2##TO##16 res;\
	_2i64 t1, t2;\
	res.lo = convert_##TONAME##short_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##short_sat##RMODE(x.hi);\
	return res;\
	}\
	TONAME##short3 __attribute__ ((overloadable)) convert_##TONAME##short3_sat##RMODE(long3 x)\
	{\
	_8##TO##16 res;\
	long4 y = as_long4(x);\
	res.s01 = convert_##TONAME##short2_sat##RMODE(y.lo);\
	res.s23 = convert_##TONAME##short2_sat##RMODE(y.hi);\
	return res.s012;\
	}\
	_4##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short4_sat##RMODE(_4i64 x)\
	{\
	_8##TO##16 res;\
	res.s01 = convert_##TONAME##short2_sat##RMODE(x.lo);\
	res.s23 = convert_##TONAME##short2_sat##RMODE(x.hi);\
	return res.lo;\
	}\
	_8##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short8_sat##RMODE(_8i64 x)\
	{\
	_8##TO##16 res;\
	res.lo = convert_##TONAME##short4_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##short4_sat##RMODE(x.hi);\
	return res;\
	}\
	_16##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short16_sat##RMODE(_16i64 x)\
	{\
	_16##TO##16 res;\
	res.lo = convert_##TONAME##short8_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##short8_sat##RMODE(x.hi);\
	return res;\
	}\

#define DEF_SAT_PROTO16_F(TO, TONAME, RMODE, RMODEVAL, FLAG)\
	_1##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short_sat##RMODE(float x)\
	{\
	_8##TO##16 res;\
	float4 param;\
	param.s0 = x;\
    _4##TO##32 t = convert_##TONAME##int4_sat##RMODE(param);\
	res.lo = convert_##TONAME##short4_sat##RMODE(t);\
	return res.s0;\
	}\
	_2##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short2_sat##RMODE(float2 x)\
	{\
	_8##TO##16 res;\
	float4 param;\
	param.lo = x;\
    _4##TO##32 t = convert_##TONAME##int4_sat##RMODE(param);\
	res.lo = convert_##TONAME##short4_sat##RMODE(t);\
	return res.s01;\
	}\
	TONAME##short3 __attribute__ ((overloadable)) convert_##TONAME##short3_sat##RMODE(float3 x)\
	{\
	_8##TO##16 res;\
    _4##TO##32 t = convert_##TONAME##int4_sat##RMODE(as_float4(x));\
	res.lo = convert_##TONAME##short4_sat##RMODE(t);\
	return as_##TONAME##short3(res.lo);\
	}\
	_4##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short4_sat##RMODE(float4 x)\
	{\
	_8##TO##16 res;\
    _4##TO##32 t = convert_##TONAME##int4_sat##RMODE(x);\
	res.lo = convert_##TONAME##short4_sat##RMODE(t);\
	return res.lo;\
	}\
	_8##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short8_sat##RMODE(float8 x)\
	{\
	_8##TO##16 res;\
    _4##TO##32 t1 = convert_##TONAME##int4_sat##RMODE(x.lo);\
	_4##TO##32 t2 = convert_##TONAME##int4_sat##RMODE(x.hi);\
	res.lo = convert_##TONAME##short4_sat##RMODE(t1);\
	res.hi = convert_##TONAME##short4_sat##RMODE(t2);\
	return res;\
	}\
	_16##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short16_sat##RMODE(float16 x)\
	{\
	_16##TO##16 res;\
	res.lo = convert_##TONAME##short8_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##short8_sat##RMODE(x.hi);\
	return res;\
	}


#define DEF_SAT_PROTO16_D(TO, TONAME, RMODE, RMODEVAL)\
	_1##TO##16 __attribute__((overloadable)) convert_##TONAME##short_sat##RMODE(double x)\
	{\
	_1##TO##16 res;\
	_1##TO##32 t = convert_##TONAME##int_sat##RMODE(x);\
	res = convert_##TONAME##short_sat##RMODE(t);\
	return res;\
	}\
	_2##TO##16 __attribute__((overloadable)) convert_##TONAME##short2_sat##RMODE(double2 x)\
	{\
	_2##TO##16 res;\
	res.lo = convert_##TONAME##short_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##short_sat##RMODE(x.hi);\
	return res;\
	}\
	TONAME##short3 __attribute__((overloadable)) convert_##TONAME##short3_sat##RMODE(double3 x)\
	{\
	TONAME##short3 res;\
	res.s01 = convert_##TONAME##short2_sat##RMODE(x.s01);\
	res.s2 = convert_##TONAME##short_sat##RMODE(x.s2);\
	return res;\
	}\
	_4##TO##16 __attribute__((overloadable)) convert_##TONAME##short4_sat##RMODE(double4 x)\
	{\
	_4##TO##16 res;\
	res.lo = convert_##TONAME##short2_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##short2_sat##RMODE(x.hi);\
	return res;\
	}\
	_8##TO##16 __attribute__((overloadable)) convert_##TONAME##short8_sat##RMODE(double8 x)\
	{\
	_8##TO##16 res;\
	res.lo = convert_##TONAME##short4_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##short4_sat##RMODE(x.hi);\
	return res;\
	}\
	_16##TO##16 __attribute__((overloadable)) convert_##TONAME##short16_sat##RMODE(double16 x)\
	{\
	_16##TO##16 res;\
	res.lo = convert_##TONAME##short8_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##short8_sat##RMODE(x.hi);\
	return res;\
	}

#define DEF_SAT_PROTOI32_FNOSVML(RMODE, RMODEVAL, MAX, MIN, RSVML, CPUTYPE)\
	_1i32 __attribute__ ((overloadable)) convert_int_sat##RMODE(float x)\
	{\
	_1i32 res;\
	if(x >= MAX) return MAX;\
	if(x <= MIN) return MIN;\
	float4 p;\
	p.s0 = x;\
	res = floatToInt(p);\
	return res;\
	}\
	_2i32 __attribute__ ((overloadable)) convert_int2_sat##RMODE(float2 x)\
	{\
	_4i32 res;\
	float4 param;\
	param.lo = x;\
	res = floatToIntSat(param);\
	return res.lo;\
	}\
	int3 __attribute__ ((overloadable)) convert_int3_sat##RMODE(float3 x)\
	{\
	_4i32 res;\
	res = floatToIntSat(as_float4(x));\
	return as_int3(res);\
	}\
	_4i32 __attribute__ ((overloadable)) convert_int4_sat##RMODE(float4 x)\
	{\
	_4i32 res;\
	res = floatToIntSat(x);\
	return res;\
	}\
	_8i32 __attribute__ ((overloadable)) convert_int8_sat##RMODE(float8 x)\
	{\
	_8i32 res;\
	res.lo = floatToIntSat(x.lo);\
	res.hi = floatToIntSat(x.hi);\
	return res;\
	}\
	_16i32 __attribute__ ((overloadable)) convert_int16_sat##RMODE(float16 x)\
	{\
	_16i32 res;\
	res.lo.lo = floatToIntSat(x.lo.lo);\
	res.lo.hi = floatToIntSat(x.lo.hi);\
	res.hi.lo = floatToIntSat(x.hi.lo);\
	res.hi.hi = floatToIntSat(x.hi.hi);\
	return res;\
	}

// Oleg:
// convert_int(float)
// This is issue with ABI for SVML.
// So as a hack we call float4 version of conversions for float
// TODO: switch to DEF_INT_PROTOI32_FUSESVML_X when CSSD100012898:
// "ABI for passing float2 arguments to SVML conversions function is invalid" is fixed
#define DEF_SAT_PROTOI32_FUSESVML_2(RMODE, RMODEVAL, RSVML, CPUTYPE, WIDTHOCL, WIDTHSVML)\
    _2i32 __attribute__ ((overloadable)) convert_int2_sat##RMODE(float2  x)\
	{\
    float4 y;\
    y.lo = x;\
    y.hi = x;\
    return convert_int4_sat##RMODE(y).lo;\
	}

#define DEF_SAT_PROTOI32_FUSESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, WIDTHOCL, WIDTHSVML)\
    _##WIDTHSVML##i32 __attribute__ ((overloadable)) convert_int##WIDTHOCL##_sat##RMODE(float##WIDTHOCL  x)\
	{\
    _##WIDTHSVML##i32 res = __ocl_svml_##CPUTYPE##_cvtfptoi32##RSVML##satf##WIDTHSVML(x);\
	return res;\
	}

#define DEF_SAT_PROTOI32_FUSESVML(RMODE, RMODEVAL, _INT_MAX, _INT_MIN, RSVML, CPUTYPE)\
    DEF_SAT_PROTOI32_FUSESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, , 1)\
    DEF_SAT_PROTOI32_FUSESVML_2(RMODE, RMODEVAL, RSVML, CPUTYPE, 2, 2)\
    DEF_SAT_PROTOI32_FUSESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, 3, 3)\
    DEF_SAT_PROTOI32_FUSESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, 4, 4)\
    DEF_SAT_PROTOI32_FUSESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, 8, 8)\
    DEF_SAT_PROTOI32_FUSESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, 16, 16)

// oleg:
// convert_sat_int(double)
// This is issue with ABI for SVML.
// So as a hack we call double and double2 version of conversions
// TODO: switch double2,3,4,8,16 to SVML call when CSSD100012907:
// "ABI for passing double3 arguments to SVML conversions function is invalid" is fixed
#define DEF_SAT_PROTOI32_DUSESVML_234816_AS_D1(RMODE, RMODEVAL, RSVML, CPUTYPE)\
	int2 __attribute__ ((overloadable)) convert_int2_sat##RMODE(double2 x)\
    {\
	int2 res;\
	res.lo = convert_int_sat##RMODE(x.lo);\
	res.hi = convert_int_sat##RMODE(x.hi);\
	return res;\
	}\
    int3 __attribute__ ((overloadable)) convert_int3_sat##RMODE(double3 x)\
	{\
    int3 res;\
    double2 y1,y2;\
    y1 = x.s01;\
    y2.lo = x.s2;\
    int2 r1 = convert_int2_sat##RMODE(y1);\
    int2 r2 = convert_int2_sat##RMODE(y2);\
    res.s01 = r1;\
    res.s2 = r2.lo;\
    return res;\
	}\
	int4 __attribute__ ((overloadable)) convert_int4_sat##RMODE(double4 x)\
    {\
	int4 res;\
	res.lo = convert_int2_sat##RMODE(x.lo);\
	res.hi = convert_int2_sat##RMODE(x.hi);\
	return res;\
	}\
	int8 __attribute__ ((overloadable)) convert_int8_sat##RMODE(double8 x)\
    {\
	int8 res;\
	res.lo = convert_int4_sat##RMODE(x.lo);\
	res.hi = convert_int4_sat##RMODE(x.hi);\
	return res;\
	}\
	int16 __attribute__ ((overloadable)) convert_int16_sat##RMODE(double16 x)\
    {\
	int16 res;\
	res.lo = convert_int8_sat##RMODE(x.lo);\
	res.hi = convert_int8_sat##RMODE(x.hi);\
	return res;\
	}

#define DEF_SAT_PROTOI32_DUSESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, WIDTHOCL, WIDTHSVML)\
    _##WIDTHSVML##i32 __attribute__ ((overloadable)) convert_int##WIDTHOCL##_sat##RMODE(double##WIDTHOCL  x)\
	{\
    _##WIDTHSVML##i32 res = __ocl_svml_##CPUTYPE##_cvtfptoi32##RSVML##sat##WIDTHSVML(x);\
	return res;\
	}

#define DEF_SAT_PROTOI32_DUSESVML(RMODE, RMODEVAL, RSVML, CPUTYPE)\
    DEF_SAT_PROTOI32_DUSESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, , 1)\
    DEF_SAT_PROTOI32_DUSESVML_234816_AS_D1(RMODE, RMODEVAL, RSVML, CPUTYPE)

#define DEF_SAT_PROTOI32_D_D12_AS_D2(RMODE)                       \
	_1i32 __attribute__ ((overloadable)) convert_int_sat##RMODE(double x)\
	{\
		return doubleToIntSat(x);\
    }\
	_2i32 __attribute__ ((overloadable)) convert_int2_sat##RMODE(double2 x)\
	{\
	_2i32 res;\
	res.lo = doubleToIntSat(x.lo);\
	res.hi = doubleToIntSat(x.hi);\
	return res;\
	}

#define DEF_SAT_PROTOI32_D_D34_AS_D2(RMODE)                            \
	int3 __attribute__((overloadable)) convert_int3_sat##RMODE(double3 x)\
	{\
	int3 res;\
	res.s01 = convert_int2_sat##RMODE(x.s01);\
	res.s2 = convert_int_sat##RMODE(x.s2);\
	return res;\
	}\
	_4i32 __attribute__((overloadable)) convert_int4_sat##RMODE(double4 x)\
	{\
	_4i32 res;\
	res.lo = convert_int2_sat##RMODE(x.lo);\
	res.hi = convert_int2_sat##RMODE(x.hi);\
	return res;\
	}

#define DEF_SAT_PROTOI32_D_D816_AS_D4(RMODE)                            \
	_8i32 __attribute__((overloadable)) convert_int8_sat##RMODE(double8 x)\
	{\
	_8i32 res;\
	res.lo =  convert_int4_sat##RMODE(x.lo);\
	res.hi =  convert_int4_sat##RMODE(x.hi);\
	return res;\
	}\
	_16i32 __attribute__((overloadable)) convert_int16_sat##RMODE(double16 x)\
	{\
	_16i32 res;\
	res.lo =  convert_int8_sat##RMODE(x.lo);\
	res.hi =  convert_int8_sat##RMODE(x.hi);\
	return res;\
	}

#define DEF_SAT_PROTOI32_DNOSVML(RMODE, RMODEVAL, RSVML, CPUTYPE)       \
    DEF_SAT_PROTOI32_D_D12_AS_D2(RMODE)                               \
    DEF_SAT_PROTOI32_D_D34_AS_D2(RMODE)                              \
    DEF_SAT_PROTOI32_D_D816_AS_D4(RMODE)                              


#define DEF_SAT_PROTO32_I64(TO, TONAME, RMODE, MAX, MIN)\
	_1##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int_sat##RMODE(_1i64 x)\
	{\
	if(x > MAX) x = MAX;\
	if(x < MIN) x = MIN;\
	return (_1##TO##32)x;\
	}\
	_2##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int2_sat##RMODE(_2i64 x)\
	{\
	_2##TO##32 res;\
	res.lo = convert_##TONAME##int_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##int_sat##RMODE(x.hi);\
	return res;\
	}\
	TONAME##int3 __attribute__ ((overloadable)) convert_##TONAME##int3_sat##RMODE(long3 x)\
	{\
	TONAME##int3 res;\
	res.s01 = convert_##TONAME##int2_sat##RMODE(x.s01);\
	res.s2 = convert_##TONAME##int_sat##RMODE(x.s2);\
	return res;\
	}\
	_4##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int4_sat##RMODE(_4i64 x)\
	{\
	_4##TO##32 res;\
	res.lo = convert_##TONAME##int2_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##int2_sat##RMODE(x.hi);\
	return res;\
	}\
	_8##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int8_sat##RMODE(_8i64 x)\
	{\
	_8##TO##32 res;\
	res.lo = convert_##TONAME##int4_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##int4_sat##RMODE(x.hi);\
	return res;\
	}\
	_16##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int16_sat##RMODE(_16i64 x)\
	{\
	_16##TO##32 res;\
	res.lo = convert_##TONAME##int8_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##int8_sat##RMODE(x.hi);\
	return res;\
	}

	//unsigned SAT
#define DEF_SAT_PROTO8_U16(TO, TONAME, RMODE, MAX)\
	_1##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char_sat##RMODE(_1u16 x)\
	{\
	if(x > MAX) x = MAX;\
	return (_1##TO##8)x;\
	}\
	_2##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char2_sat##RMODE(_2u16 x)\
	{\
	_16##TO##8 res;\
	_8u16 param;\
	param.s01 = x;\
	_8##TO##16 t1 = as_##TONAME##short8(_mm_cmpgt_epi16(_mm_setzero_si128(), __builtin_astype(param,__m128i)));\
	_8##TO##16 t2 = as_##TONAME##short8(_mm_cmpgt_epi16(__builtin_astype(param,__m128i), *((__m128i*)max##TO##Char16)));\
	t1 = as_##TONAME##short8(_mm_or_si128(__builtin_astype(t1,__m128i), __builtin_astype(t2,__m128i))); \
	res = as_##TONAME##char16(_mm_and_si128(__builtin_astype(t1,__m128i), *((__m128i*)max##TO##Char16) ));\
	t1 = as_##TONAME##short8(_mm_andnot_si128(__builtin_astype(t1,__m128i), __builtin_astype(param,__m128i)));\
	res = as_##TONAME##char16(_mm_or_si128(__builtin_astype(t1,__m128i), __builtin_astype(res,__m128i)));\
	res = as_##TONAME##char16(_mm_shuffle_epi8(__builtin_astype(res,__m128i), *((__m128i *) _8x16to8x8)));\
	return res.s01;\
	}\
	TONAME##char3 __attribute__ ((overloadable)) convert_##TONAME##char3_sat##RMODE(ushort3 x)\
	{\
	_16##TO##8 res;\
	_8u16 param;\
	param.lo = as_ushort4(x);\
	_8##TO##16 t1 = as_##TONAME##short8(_mm_cmpgt_epi16(_mm_setzero_si128(), __builtin_astype(param,__m128i)));\
	_8##TO##16 t2 = as_##TONAME##short8(_mm_cmpgt_epi16(__builtin_astype(param,__m128i), *((__m128i*)max##TO##Char16)));\
	t1 = as_##TONAME##short8(_mm_or_si128(__builtin_astype(t1,__m128i), __builtin_astype(t2,__m128i))); \
	res = as_##TONAME##char16(_mm_and_si128(__builtin_astype(t1,__m128i), *((__m128i*)max##TO##Char16) ));\
	t1 = as_##TONAME##short8(_mm_andnot_si128(__builtin_astype(t1,__m128i), __builtin_astype(param,__m128i)));\
	res = as_##TONAME##char16(_mm_or_si128(__builtin_astype(t1,__m128i), __builtin_astype(res,__m128i)));\
	res = as_##TONAME##char16(_mm_shuffle_epi8(__builtin_astype(res,__m128i), *((__m128i *) _8x16to8x8)));\
	return res.s012;\
	}\
	_4##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char4_sat##RMODE(_4u16 x)\
	{\
	_16##TO##8 res;\
	_8u16 param;\
	param.lo = x;\
	_8##TO##16 t1 = as_##TONAME##short8(_mm_cmpgt_epi16(_mm_setzero_si128(), __builtin_astype(param,__m128i)));\
	_8##TO##16 t2 = as_##TONAME##short8(_mm_cmpgt_epi16(__builtin_astype(param,__m128i), *((__m128i*)max##TO##Char16)));\
	t1 = as_##TONAME##short8(_mm_or_si128(__builtin_astype(t1,__m128i), __builtin_astype(t2,__m128i))); \
	res = as_##TONAME##char16(_mm_and_si128(__builtin_astype(t1,__m128i), *((__m128i*)max##TO##Char16) ));\
	t1 = as_##TONAME##short8(_mm_andnot_si128(__builtin_astype(t1,__m128i), __builtin_astype(param,__m128i)));\
	res = as_##TONAME##char16(_mm_or_si128(__builtin_astype(t1,__m128i), __builtin_astype(res,__m128i)));\
	res = as_##TONAME##char16(_mm_shuffle_epi8(__builtin_astype(res,__m128i), *((__m128i *) _8x16to8x8)));\
	return res.s0123;\
	}\
	_8##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char8_sat##RMODE(_8u16 x)\
	{\
	_16##TO##8 res;\
	_8u16 t1 = as_ushort8(_mm_cmpgt_epi16(_mm_setzero_si128(), __builtin_astype(x,__m128i)));\
	_8u16 t2 = as_ushort8(_mm_cmpgt_epi16(__builtin_astype(x,__m128i), *((__m128i*)max##TO##Char16)));\
	t1 = as_ushort8(_mm_or_si128(__builtin_astype(t1,__m128i), __builtin_astype(t2,__m128i))); \
	res = as_##TONAME##char16(_mm_and_si128(__builtin_astype(t1,__m128i), *((__m128i*)max##TO##Char16) ));\
	t1 = as_ushort8(_mm_andnot_si128(__builtin_astype(t1,__m128i), __builtin_astype(x,__m128i)));\
	res = as_##TONAME##char16(_mm_or_si128(__builtin_astype(t1,__m128i), __builtin_astype(res,__m128i)));\
	res = as_##TONAME##char16(_mm_shuffle_epi8(__builtin_astype(res,__m128i), *((__m128i *) _8x16to8x8)));\
	return res.lo;\
	}\
	_16##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char16_sat##RMODE(_16u16 x)\
	{\
	_16##TO##8 res;\
	res.lo = convert_##TONAME##char8_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##char8_sat##RMODE(x.hi);\
	return res;\
	}\

#define DEF_SAT_PROTO8_U32(TO, TONAME, RMODE, MAX)\
	_1##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char_sat##RMODE(_1u32 x)\
	{\
	if(x > MAX) x = MAX;\
	return (_1##TO##8)x;\
	}\
	_2##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char2_sat##RMODE(_2u32 x)\
	{\
	_16##TO##8 res;\
	_4u32 param;\
	param.lo = x;\
	_4i32 t1 = as_int4(_mm_cmpgt_epi32(_mm_setzero_si128(), __builtin_astype(param,__m128i)));\
	_4i32 t2 = as_int4(_mm_cmpgt_epi32(__builtin_astype(param,__m128i), *((__m128i*)max##TO##Char32)));\
	t1 = as_int4(_mm_or_si128(__builtin_astype(t1,__m128i), __builtin_astype(t2,__m128i)));\
	res = __builtin_astype(_mm_and_si128(__builtin_astype(t1,__m128i), *((__m128i*)max##TO##Char32) ),_16##TO##8);\
	t1 = as_int4(_mm_andnot_si128(__builtin_astype(t1,__m128i), __builtin_astype(param,__m128i)));\
	res = __builtin_astype(_mm_or_si128(__builtin_astype(t1,__m128i), __builtin_astype(res,__m128i)),_16##TO##8);\
	res = __builtin_astype(_mm_shuffle_epi8(__builtin_astype(res,__m128i), *((__m128i *) _4x32to4x8)),_16##TO##8);\
	return res.s01;\
	}\
	TONAME##char3 __attribute__ ((overloadable)) convert_##TONAME##char3_sat##RMODE(uint3 x)\
	{\
	_16##TO##8 res;\
	uint4 y = as_uint4(x);\
	_4i32 t1 = as_int4(_mm_cmpgt_epi32(_mm_setzero_si128(), __builtin_astype(y,__m128i)));\
	_4i32 t2 = as_int4(_mm_cmpgt_epi32(__builtin_astype(y,__m128i), *((__m128i*)max##TO##Char32)));\
	t1 = as_int4(_mm_or_si128(__builtin_astype(t1,__m128i), __builtin_astype(t2,__m128i)));\
	res = __builtin_astype(_mm_and_si128(__builtin_astype(t1,__m128i), *((__m128i*)max##TO##Char32) ),_16##TO##8);\
	t1 = as_int4(_mm_andnot_si128(__builtin_astype(t1,__m128i), __builtin_astype(y,__m128i)));\
	res = __builtin_astype(_mm_or_si128(__builtin_astype(t1,__m128i), __builtin_astype(res,__m128i)),_16##TO##8);\
	res = __builtin_astype(_mm_shuffle_epi8(__builtin_astype(res,__m128i), *((__m128i *) _4x32to4x8)),_16##TO##8);\
	return res.s012;\
	}\
	_4##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char4_sat##RMODE(_4u32 x)\
	{\
	_16##TO##8 res;\
	_4i32 t1 = as_int4(_mm_cmpgt_epi32(_mm_setzero_si128(), __builtin_astype(x,__m128i)));\
	_4i32 t2 = as_int4(_mm_cmpgt_epi32(__builtin_astype(x,__m128i), *((__m128i*)max##TO##Char32)));\
	t1 = as_int4(_mm_or_si128(__builtin_astype(t1,__m128i), __builtin_astype(t2,__m128i)));\
	res = __builtin_astype(_mm_and_si128(__builtin_astype(t1,__m128i), *((__m128i*)max##TO##Char32) ),_16##TO##8);\
	t1 = as_int4(_mm_andnot_si128(__builtin_astype(t1,__m128i), __builtin_astype(x,__m128i)));\
	res = __builtin_astype(_mm_or_si128(__builtin_astype(t1,__m128i), __builtin_astype(res,__m128i)),_16##TO##8);\
	res = __builtin_astype(_mm_shuffle_epi8(__builtin_astype(res,__m128i), *((__m128i *) _4x32to4x8)),_16##TO##8);\
	return res.s0123;\
	}\
	_8##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char8_sat##RMODE(_8u32 x)\
	{\
	_16##TO##8 res;\
	res.s0123 = convert_##TONAME##char4_sat##RMODE(x.lo);\
	res.s4567 = convert_##TONAME##char4_sat##RMODE(x.hi);\
	return res.lo;\
	}\
	_16##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char16_sat##RMODE(_16u32 x)\
	{\
	_16##TO##8 res;\
	res.lo = convert_##TONAME##char8_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##char8_sat##RMODE(x.hi);\
	return res;\
	}

#define DEF_SAT_PROTO8_U64(TO, TONAME, RMODE, MAX)\
	_1##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char_sat##RMODE(_1u64 x)\
	{\
	if(x > MAX) x = MAX;\
	return (_1##TO##8)x;\
	}\
	_2##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char2_sat##RMODE(_2u64 x)\
	{\
	_2##TO##8 res;\
	res.lo = convert_##TONAME##char_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##char_sat##RMODE(x.hi);\
	return res.s01;\
	}\
	TONAME##char3 __attribute__ ((overloadable)) convert_##TONAME##char3_sat##RMODE(ulong3 x)\
	{\
	_16##TO##8 res;\
	res.s01 = convert_##TONAME##char2_sat##RMODE(x.s01);\
	res.s2 = convert_##TONAME##char_sat##RMODE(x.s2);\
	return res.s012;\
	}\
	_4##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char4_sat##RMODE(_4u64 x)\
	{\
	_16##TO##8 res;\
	res.s01 = convert_##TONAME##char2_sat##RMODE(x.lo);\
	res.s23 = convert_##TONAME##char2_sat##RMODE(x.hi);\
	return res.s0123;\
	}\
	_8##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char8_sat##RMODE(_8u64 x)\
	{\
	_16##TO##8 res;\
	res.s0123 = convert_##TONAME##char4_sat##RMODE(x.lo);\
	res.s4567 = convert_##TONAME##char4_sat##RMODE(x.hi);\
	return res.lo;\
	}\
	_16##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char16_sat##RMODE(_16u64 x)\
	{\
	_16##TO##8 res;\
	res.lo = convert_##TONAME##char8_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##char8_sat##RMODE(x.hi);\
	return res;\
	}

	// 16 bits
#define DEF_SAT_PROTO16_U32(TO, TONAME, RMODE, MAX)\
	_1##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short_sat##RMODE(_1u32 x)\
	{\
	if(x > MAX) x = MAX;\
	return (_1##TO##16)x;\
	}\
	_2##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short2_sat##RMODE(_2u32 x)\
	{\
	_8##TO##16 res;\
	_4u32 param;\
	param.lo = x;\
	_4##TO##32 t1 = __builtin_astype(_mm_cmpgt_epi32(_mm_setzero_si128(), __builtin_astype(param,__m128i)),_4##TO##32);\
	_4##TO##32 t2 = __builtin_astype(_mm_cmpgt_epi32(__builtin_astype(param,__m128i), *((__m128i*)max##TO##Short32)),_4##TO##32);\
	t1 = __builtin_astype(_mm_or_si128(__builtin_astype(t1,__m128i), __builtin_astype(t2,__m128i)),_4##TO##32); \
	res = __builtin_astype(_mm_and_si128(__builtin_astype(t1,__m128i), *((__m128i*)max##TO##Short32) ),_8##TO##16);\
	t1 = __builtin_astype(_mm_andnot_si128(__builtin_astype(t1,__m128i), __builtin_astype(param,__m128i)),_4##TO##32);\
	res = __builtin_astype(_mm_or_si128(__builtin_astype(t1,__m128i), __builtin_astype(res,__m128i)),_8##TO##16);\
	res = __builtin_astype(_mm_shuffle_epi8(__builtin_astype(res,__m128i), *((__m128i *) _4x32to4x16)),_8##TO##16);\
	return res.s01;\
	}\
	TONAME##short3 __attribute__ ((overloadable)) convert_##TONAME##short3_sat##RMODE(uint3 x)\
	{\
	_8##TO##16 res;\
	uint4 y = as_uint4(x);\
	_4##TO##32 t1 = __builtin_astype(_mm_cmpgt_epi32(_mm_setzero_si128(), __builtin_astype(y,__m128i)),_4##TO##32);\
	_4##TO##32 t2 = __builtin_astype(_mm_cmpgt_epi32(__builtin_astype(y,__m128i), *((__m128i*)max##TO##Short32)),_4##TO##32);\
	t1 = __builtin_astype(_mm_or_si128(__builtin_astype(t1,__m128i), __builtin_astype(t2,__m128i)),_4##TO##32); \
	res = __builtin_astype(_mm_and_si128(__builtin_astype(t1,__m128i), *((__m128i*)max##TO##Short32) ),_8##TO##16);\
	t1 = __builtin_astype(_mm_andnot_si128(__builtin_astype(t1,__m128i), __builtin_astype(y,__m128i)),_4##TO##32);\
	res = __builtin_astype(_mm_or_si128(__builtin_astype(t1,__m128i), __builtin_astype(res,__m128i)),_8##TO##16);\
	res = __builtin_astype(_mm_shuffle_epi8(__builtin_astype(res,__m128i), *((__m128i *) _4x32to4x16)),_8##TO##16);\
	return res.s012;\
	}\
	_4##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short4_sat##RMODE(_4u32 x)\
	{\
	_8##TO##16 res;\
	_4##TO##32 t1 = __builtin_astype(_mm_cmpgt_epi32(_mm_setzero_si128(), __builtin_astype(x,__m128i)),_4##TO##32);\
	_4##TO##32 t2 = __builtin_astype(_mm_cmpgt_epi32(__builtin_astype(x,__m128i), *((__m128i*)max##TO##Short32)),_4##TO##32);\
	t1 = __builtin_astype(_mm_or_si128(__builtin_astype(t1,__m128i), __builtin_astype(t2,__m128i)),_4##TO##32); \
	res = __builtin_astype(_mm_and_si128(__builtin_astype(t1,__m128i), *((__m128i*)max##TO##Short32) ),_8##TO##16);\
	t1 = __builtin_astype(_mm_andnot_si128(__builtin_astype(t1,__m128i), __builtin_astype(x,__m128i)),_4##TO##32);\
	res = __builtin_astype(_mm_or_si128(__builtin_astype(t1,__m128i), __builtin_astype(res,__m128i)),_8##TO##16);\
	res = __builtin_astype(_mm_shuffle_epi8(__builtin_astype(res,__m128i), *((__m128i *) _4x32to4x16)),_8##TO##16);\
	return res.s0123;\
	}\
	_8##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short8_sat##RMODE(_8u32 x)\
	{\
	_8##TO##16 res;\
	res.lo = convert_##TONAME##short4_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##short4_sat##RMODE(x.hi);\
	return res;\
	}\
	_16##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short16_sat##RMODE(_16u32 x)\
	{\
	_16##TO##16 res;\
	res.lo = convert_##TONAME##short8_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##short8_sat##RMODE(x.hi);\
	return res;\
	}\

#define DEF_SAT_PROTO16_U64(TO, TONAME, RMODE, MAX)\
	_1##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short_sat##RMODE(_1u64 x)\
	{\
	if(x > MAX) x = MAX;\
	return (_1##TO##16)x;\
	}\
	_2##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short2_sat##RMODE(_2u64 x)\
	{\
	_2##TO##16 res;\
	res.lo = convert_##TONAME##short_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##short_sat##RMODE(x.hi);\
	return res;\
	}\
	TONAME##short3 __attribute__ ((overloadable)) convert_##TONAME##short3_sat##RMODE(ulong3 x)\
	{\
	_8##TO##16 res;\
	res.s01 = convert_##TONAME##short2_sat##RMODE(x.s01);\
	res.s2 = convert_##TONAME##short_sat##RMODE(x.s2);\
	return res.s012;\
	}\
	_4##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short4_sat##RMODE(_4u64 x)\
	{\
	_8##TO##16 res;\
	res.s01 = convert_##TONAME##short2_sat##RMODE(x.lo);\
	res.s23 = convert_##TONAME##short2_sat##RMODE(x.hi);\
	return res.lo;\
	}\
	_8##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short8_sat##RMODE(_8u64 x)\
	{\
	_8##TO##16 res;\
	res.lo = convert_##TONAME##short4_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##short4_sat##RMODE(x.hi);\
	return res;\
	}\
	_16##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short16_sat##RMODE(_16u64 x)\
	{\
	_16##TO##16 res;\
	res.lo = convert_##TONAME##short8_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##short8_sat##RMODE(x.hi);\
	return res;\
	}

	// 32 bits
#define DEF_SAT_PROTOU32_I8(TI, TO, TINAME, TONAME, RMODE)\
	_1##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int##RMODE(_1##TI##8 x)\
	{\
	if(x < 0) return 0;\
	return (_1##TO##32)x;\
	}\
	_2##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int2##RMODE(_2##TI##8 x)\
	{\
	_4##TO##32 res;\
	_16##TI##8 param;\
	param.s01 = x;\
	res =  __builtin_astype(_mm_cvtep##TI##8_epi32(__builtin_astype(param,__m128i)),_4##TO##32);\
	res = convert_##TONAME##int4##RMODE((_4##TI##32)res);\
	return res.lo;\
	}\
	TONAME##int3 __attribute__ ((overloadable)) convert_##TONAME##int3##RMODE(TINAME##char3 x)\
	{\
	_4##TO##32 res;\
	_16##TI##8 param;\
	param.s012 = x;\
	res =  __builtin_astype(_mm_cvtep##TI##8_epi32(__builtin_astype(param,__m128i)),_4##TO##32);\
	res = convert_##TONAME##int4##RMODE((_4##TI##32)res);\
	return res.s012;\
	}\
	_4##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int4##RMODE(_4##TI##8 x)\
	{\
	_4##TO##32 res;\
	_16##TI##8 param;\
	param.s0123 = x;\
	res =  __builtin_astype(_mm_cvtep##TI##8_epi32(__builtin_astype(param,__m128i)),_4##TO##32);\
	res = convert_##TONAME##int4##RMODE((_4##TI##32)res);\
	return res;\
	}\
	_8##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int8##RMODE(_8##TI##8 x)\
	{\
	_8##TO##32 res;\
	_16##TI##8 param;\
	param.lo = x;\
	res.lo = __builtin_astype(_mm_cvtep##TI##8_epi32(__builtin_astype(param,__m128i)),_4##TO##32);\
	res.lo = convert_##TONAME##int4##RMODE((_4##TI##32)res.lo);\
	res.hi = __builtin_astype(_mm_cvtep##TI##8_epi32(_mm_srli_si128(__builtin_astype(param,__m128i), 4)),_4##TO##32);\
	res.hi = convert_##TONAME##int4##RMODE((_4##TI##32)res.hi);\
	return res;\
	}\
	_16##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int16##RMODE(_16##TI##8 x)\
	{\
	_16##TO##32 res;\
	res.lo.lo = __builtin_astype(_mm_cvtep##TI##8_epi32(__builtin_astype(x,__m128i)),_4##TO##32);\
	res.lo.lo = convert_##TONAME##int4##RMODE((_4##TI##32)res.lo.lo);\
	res.lo.hi = __builtin_astype(_mm_cvtep##TI##8_epi32(_mm_srli_si128(__builtin_astype(x,__m128i), 4)),_4##TO##32);\
	res.lo.hi = convert_##TONAME##int4##RMODE((_4##TI##32)res.lo.hi);\
	res.hi.lo = __builtin_astype(_mm_cvtep##TI##8_epi32(_mm_srli_si128(__builtin_astype(x,__m128i), 8)),_4##TO##32);\
	res.hi.lo = convert_##TONAME##int4##RMODE((_4##TI##32)res.hi.lo);\
	res.hi.hi = __builtin_astype(_mm_cvtep##TI##8_epi32(_mm_srli_si128(__builtin_astype(x,__m128i), 12)),_4##TO##32);\
	res.hi.hi = convert_##TONAME##int4##RMODE((_4##TI##32)res.hi.hi);\
	return res;\
	}\

#define DEF_SAT_PROTOU32_I16(TI, TO, TINAME, TONAME, RMODE)\
	_1##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int##RMODE(_1##TI##16 x)\
	{\
	if(x < 0) return 0;\
	return (_1##TO##32)x;\
	}\
	_2##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int2##RMODE(_2##TI##16 x)\
	{\
	_4##TO##32 res;\
	_8##TI##16 param;\
	param.s01 = x;\
	res = __builtin_astype(_mm_cvtep##TI##16_epi32(__builtin_astype(param,__m128i)),_4##TO##32);\
	res = convert_##TONAME##int4##RMODE((_4##TI##32)res);\
	return res.lo;\
	}\
	TONAME##int3 __attribute__ ((overloadable)) convert_##TONAME##int3##RMODE(TINAME##short3 x)\
	{\
	_8##TI##16 param;\
	param.s012 = x;\
	TONAME##int4 res = convert_##TONAME##int4##RMODE((_4##TI##32)_mm_cvtep##TI##16_epi32(__builtin_astype(param,__m128i)));\
	return res.s012;\
	}\
	_4##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int4##RMODE(_4##TI##16 x)\
	{\
	_8##TI##16 param;\
	param.lo = x;\
	return convert_##TONAME##int4##RMODE((_4##TI##32)_mm_cvtep##TI##16_epi32(__builtin_astype(param,__m128i)));\
	}\
	_8##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int8##RMODE(_8##TI##16 x)\
	{\
	_8##TO##32 res;\
	res.lo = __builtin_astype(_mm_cvtep##TI##16_epi32(__builtin_astype(x,__m128i)),_4##TO##32);\
	res.lo = convert_##TONAME##int4##RMODE((_4##TI##32)res.lo);\
	res.hi = __builtin_astype(_mm_cvtep##TI##16_epi32(_mm_srli_si128(__builtin_astype(x,__m128i), 8)),_4##TO##32);\
	res.hi = convert_##TONAME##int4##RMODE((_4##TI##32)res.hi);\
	return res;\
	}\
	_16##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int16##RMODE(_16##TI##16 x)\
	{\
	_16##TO##32 res;\
	res.lo.lo = __builtin_astype(_mm_cvtep##TI##16_epi32(__builtin_astype(x.lo,__m128i)),_4##TO##32);\
	res.lo.lo = convert_##TONAME##int4##RMODE((_4##TI##32)res.lo.lo);\
	res.lo.hi = __builtin_astype(_mm_cvtep##TI##16_epi32(_mm_srli_si128(__builtin_astype(x.lo,__m128i), 8)),_4##TO##32);\
	res.lo.hi = convert_##TONAME##int4##RMODE((_4##TI##32)res.lo.hi);\
	res.hi.lo = __builtin_astype(_mm_cvtep##TI##16_epi32(__builtin_astype(x.hi,__m128i)),_4##TO##32);\
	res.hi.lo = convert_##TONAME##int4##RMODE((_4##TI##32)res.hi.lo);\
	res.hi.hi = __builtin_astype(_mm_cvtep##TI##16_epi32(_mm_srli_si128(__builtin_astype(x.hi,__m128i), 8)),_4##TO##32);\
	res.hi.hi = convert_##TONAME##int4##RMODE((_4##TI##32)res.hi.hi);\
	return res;\
	}

#define DEF_SAT_PROTO32_U64(TO, TONAME, RMODE, MAX)\
	_1##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int_sat##RMODE(_1u64 x)\
	{\
	if(x > MAX) x = MAX;\
	return (_1##TO##32)x;\
	}\
	_2##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int2_sat##RMODE(_2u64 x)\
	{\
	_2##TO##32 res;\
	res.lo = convert_##TONAME##int_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##int_sat##RMODE(x.hi);\
	return res;\
	}\
	TONAME##int3 __attribute__ ((overloadable)) convert_##TONAME##int3_sat##RMODE(ulong3 x)\
	{\
	TONAME##int3 res;\
	res.s01 = convert_##TONAME##int2_sat##RMODE(x.s01);\
	res.s2= convert_##TONAME##int_sat##RMODE(x.s2);\
	return res;\
	}\
	_4##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int4_sat##RMODE(_4u64 x)\
	{\
	_4##TO##32 res;\
	res.lo = convert_##TONAME##int2_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##int2_sat##RMODE(x.hi);\
	return res;\
	}\
	_8##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int8_sat##RMODE(_8u64 x)\
	{\
	_8##TO##32 res;\
	res.lo = convert_##TONAME##int4_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##int4_sat##RMODE(x.hi);\
	return res;\
	}\
	_16##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int16_sat##RMODE(_16u64 x)\
	{\
	_16##TO##32 res;\
	res.lo = convert_##TONAME##int8_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##int8_sat##RMODE(x.hi);\
	return res;\
	}\

#define DEF_SAT_PROTOU32_F_F1234(RMODE)\
	_1u32 __attribute__ ((overloadable)) convert_uint_sat##RMODE(float x)\
	{\
    _1u32 res = as_uint(floatToUintSat(x));\
	return res;\
	}\
	_2u32 __attribute__ ((overloadable)) convert_uint2_sat##RMODE(float2 x)\
	{\
	_4u32 res;\
	res.s0 = as_uint(floatToUintSat(x.lo));\
	res.s1 = as_uint(floatToUintSat(x.hi));\
	return res.lo;\
	}\
	uint3 __attribute__ ((overloadable)) convert_uint3_sat##RMODE(float3 x)\
	{\
	_4u32 res;\
	res.s0 = as_uint(floatToUintSat(x.s0));\
	res.s1 = as_uint(floatToUintSat(x.s1));\
	res.s2 = as_uint(floatToUintSat(x.s2));\
	return as_uint3(res);\
	}\
    _4u32 __attribute__ ((overloadable)) convert_uint4_sat##RMODE(float4 x)\
    {\
    _4u32 res;\
    res.s0 = as_uint(floatToUintSat(x.s0));\
    res.s1 = as_uint(floatToUintSat(x.s1));\
    res.s2 = as_uint(floatToUintSat(x.s2));\
    res.s3 = as_uint(floatToUintSat(x.s3));\
    return res;\
    }

#define DEF_SAT_PROTOU32_F_F816_AS_F4(RMODE)\
    _8u32 __attribute__ ((overloadable)) convert_uint8_sat##RMODE(float8 x)\
	{\
	_8u32 res;\
	res.lo = convert_uint4_sat##RMODE(x.lo);\
	res.hi = convert_uint4_sat##RMODE(x.hi);\
	return res;\
	}\
	_16u32 __attribute__ ((overloadable)) convert_uint16_sat##RMODE(float16 x)\
	{\
	_16u32 res;\
	res.lo.lo = convert_uint4_sat##RMODE(x.lo.lo);\
	res.lo.hi = convert_uint4_sat##RMODE(x.lo.hi);\
	res.hi.lo = convert_uint4_sat##RMODE(x.hi.lo);\
	res.hi.hi = convert_uint4_sat##RMODE(x.hi.hi);\
	return res;\
	}

#define DEF_SAT_PROTOU32_FNOSVML(RMODE, RMODEVAL, RSVML, CPUTYPE)\
    DEF_SAT_PROTOU32_F_F1234(RMODE)\
    DEF_SAT_PROTOU32_F_F816_AS_F4(RMODE)

// Oleg:
// convert_int(float)
// This is issue with legal types in SSE and AVX
// Elena's comment
// We call v4 functions because v2i32 is not legal type for LLVM in SSE and AVX modes.
// Since the type is illegal it should be extended to a "nearest" legal. LLVM extends it to v2i64 and SVML expects v4i32 form. 
// In this case we just call v4 functions to avoid incompatibility.
#define DEF_SAT_PROTOU32_FUSESVML_2(RMODE, RMODEVAL, RSVML, CPUTYPE, WIDTHOCL, WIDTHSVML)\
    _2u32 __attribute__ ((overloadable)) convert_uint2_sat##RMODE(float2  x)\
	{\
    float4 y;\
    y.lo = x;\
    y.hi = x;\
    return convert_uint4_sat##RMODE(y).lo;\
	}

#define DEF_SAT_PROTOU32_FUSESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, WIDTHOCL, WIDTHSVML)\
    _##WIDTHSVML##u32 __attribute__ ((overloadable)) convert_uint##WIDTHOCL##_sat##RMODE(float##WIDTHOCL  x)\
	{\
    _##WIDTHSVML##u32 res = __ocl_svml_##CPUTYPE##_cvtfptou32##RSVML##satf##WIDTHSVML(x);\
	return res;\
	}

#define DEF_SAT_PROTOU32_FUSESVML(RMODE, RMODEVAL, RSVML, CPUTYPE)\
    DEF_SAT_PROTOU32_FUSESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, , 1)\
    DEF_SAT_PROTOU32_FUSESVML_2(RMODE, RMODEVAL, RSVML, CPUTYPE, 2, 2)\
    DEF_SAT_PROTOU32_FUSESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, 3, 3)\
    DEF_SAT_PROTOU32_FUSESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, 4, 4)\
    DEF_SAT_PROTOU32_FUSESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, 8, 8)\
    DEF_SAT_PROTOU32_FUSESVML_X(RMODE, RMODEVAL, RSVML, CPUTYPE, 16, 16)

// oleg:
// convert_sat_uint(double)
// This is issue with ABI for SVML.
// So as a hack we call double and double2 version of conversions
// TODO: switch double2,3,4,8,16 to SVML call when CSSD100012907:
// "ABI for passing double3 arguments to SVML conversions function is invalid" is fixed

#define DEF_SAT_PROTOU32_D(RMODE, RSVML, CPUTYPE)\
	_1u32 __attribute__((overloadable)) convert_uint_sat##RMODE(double x)\
	{\
	_1u32 res = __ocl_svml_##CPUTYPE##_cvtfptou32##RSVML##sat1(x);\
	return res;\
	}\
	_2u32 __attribute__ ((overloadable)) convert_uint2_sat##RMODE(double2 x)\
    {\
	uint2 res;\
	res.lo = convert_uint_sat##RMODE(x.lo);\
	res.hi = convert_uint_sat##RMODE(x.hi);\
	return res;\
	}\
    _3u32 __attribute__ ((overloadable)) convert_uint3_sat##RMODE(double3 x)\
	{\
    uint3 res;\
    double2 y1,y2;\
    y1 = x.s01;\
    y2.lo = x.s2;\
    uint2 r1 = convert_uint2_sat##RMODE(y1);\
    uint2 r2 = convert_uint2_sat##RMODE(y2);\
    res.s01 = r1;\
    res.s2 = r2.lo;\
    return res;\
	}\
	_4u32 __attribute__ ((overloadable)) convert_uint4_sat##RMODE(double4 x)\
    {\
	uint4 res;\
	res.lo = convert_uint2_sat##RMODE(x.lo);\
	res.hi = convert_uint2_sat##RMODE(x.hi);\
	return res;\
	}\
	_8u32 __attribute__ ((overloadable)) convert_uint8_sat##RMODE(double8 x)\
    {\
	uint8 res;\
	res.lo = convert_uint4_sat##RMODE(x.lo);\
	res.hi = convert_uint4_sat##RMODE(x.hi);\
	return res;\
	}\
	_16u32 __attribute__ ((overloadable)) convert_uint16_sat##RMODE(double16 x)\
    {\
	uint16 res;\
	res.lo = convert_uint8_sat##RMODE(x.lo);\
	res.hi = convert_uint8_sat##RMODE(x.hi);\
	return res;\
	}

	// 64 bits
#define DEF_SAT_PROTO64_8(RMODE)\
	_1u64 __attribute__ ((overloadable)) convert_ulong##RMODE(_1i8 x)\
	{\
	if(x < 0) return 0;\
	return (_1u64)x;\
	}\
	_2u64 __attribute__ ((overloadable)) convert_ulong2##RMODE(_2i8 x)\
	{\
	_2u64 res;\
	_16i8 param;\
	param.s01 = x;\
	res =  as_ulong2(_mm_cvtepi8_epi64(__builtin_astype(param,__m128i)));\
	res =  convert_ulong2##RMODE((_2i64)res);\
	return res;\
	}\
	ulong3 __attribute__ ((overloadable)) convert_ulong3##RMODE(char3 x)\
	{\
	_4u64 res;\
	_16i8 param;\
	param.s012 = x;\
	res.lo =  as_ulong2(_mm_cvtepi8_epi64(__builtin_astype(param,__m128i)));\
	res.hi =  as_ulong2(_mm_cvtepi8_epi64(_mm_srli_si128(__builtin_astype(param,__m128i), 2)));\
	res = convert_ulong4##RMODE((_4i64)res);\
	return as_ulong3(res);\
	}\
	_4u64 __attribute__ ((overloadable)) convert_ulong4##RMODE(_4i8 x)\
	{\
	_4u64 res;\
	_16i8 param;\
	param.s0123 = x;\
	res.lo =  as_ulong2(_mm_cvtepi8_epi64(__builtin_astype(param,__m128i)));\
	res.hi =  as_ulong2(_mm_cvtepi8_epi64(_mm_srli_si128(__builtin_astype(param,__m128i), 2)));\
	res = convert_ulong4##RMODE((_4i64)res);\
	return res;\
	}\
	_8u64 __attribute__ ((overloadable)) convert_ulong8##RMODE(_8i8 x)\
	{\
	_8u64 res;\
	_16i8 param;\
	param.lo = x;\
	res.lo.lo = as_ulong2(_mm_cvtepi8_epi64(__builtin_astype(param,__m128i)));\
	res.lo.hi = as_ulong2(_mm_cvtepi8_epi64(_mm_srli_si128(__builtin_astype(param,__m128i), 2)));\
	res.hi.lo = as_ulong2(_mm_cvtepi8_epi64(_mm_srli_si128(__builtin_astype(param,__m128i), 4)));\
	res.hi.hi = as_ulong2(_mm_cvtepi8_epi64(_mm_srli_si128(__builtin_astype(param,__m128i), 6)));\
	res = convert_ulong8##RMODE((_8i64)res);\
	return res;\
	}\
	_16u64 __attribute__ ((overloadable)) convert_ulong16##RMODE(_16i8 x)\
	{\
	_16u64 res;\
	res.lo.lo.lo = as_ulong2(_mm_cvtepi8_epi64(__builtin_astype(x,__m128i)));\
	res.lo.lo.hi = as_ulong2(_mm_cvtepi8_epi64(_mm_srli_si128(__builtin_astype(x,__m128i), 2)));\
	res.lo.hi.lo = as_ulong2(_mm_cvtepi8_epi64(_mm_srli_si128(__builtin_astype(x,__m128i), 4)));\
	res.lo.hi.hi = as_ulong2(_mm_cvtepi8_epi64(_mm_srli_si128(__builtin_astype(x,__m128i), 6)));\
	res.hi.lo.lo = as_ulong2(_mm_cvtepi8_epi64(_mm_srli_si128(__builtin_astype(x,__m128i), 8)));\
	res.hi.lo.hi = as_ulong2(_mm_cvtepi8_epi64(_mm_srli_si128(__builtin_astype(x,__m128i), 10)));\
	res.hi.hi.lo = as_ulong2(_mm_cvtepi8_epi64(_mm_srli_si128(__builtin_astype(x,__m128i), 12)));\
	res.hi.hi.hi = as_ulong2(_mm_cvtepi8_epi64(_mm_srli_si128(__builtin_astype(x,__m128i), 14)));\
	res = convert_ulong16##RMODE((_16i64)res);\
	return res;\
	}\

#define DEF_SAT_PROTO64_16(RMODE)\
	_1u64 __attribute__ ((overloadable)) convert_ulong##RMODE(_1i16 x)\
	{\
	if(x < 0) return 0;\
	return (_1u64)x;\
	}\
	_2u64 __attribute__ ((overloadable)) convert_ulong2##RMODE(_2i16 x)\
	{\
	_2u64 res;\
	_8i16 param;\
	param.s01 = x;\
	res =  as_ulong2(_mm_cvtepi16_epi64(__builtin_astype(param,__m128i)));\
	res =  convert_ulong2##RMODE((_2i64)res);\
	return res;\
	}\
	ulong3 __attribute__ ((overloadable)) convert_ulong3##RMODE(short3 x)\
	{\
	_4u64 res;\
	_8i16 param;\
	param.s012 = x;\
	res.lo =  as_ulong2(_mm_cvtepi16_epi64(__builtin_astype(param,__m128i)));\
	res.hi =  as_ulong2(_mm_cvtepi16_epi64(_mm_srli_si128(__builtin_astype(param,__m128i), 4)));\
	res = convert_ulong4##RMODE((_4i64)res);\
	return as_ulong3(res);\
	}\
	_4u64 __attribute__ ((overloadable)) convert_ulong4##RMODE(_4i16 x)\
	{\
	_4u64 res;\
	_8i16 param;\
	param.lo = x;\
	res.lo =  as_ulong2(_mm_cvtepi16_epi64(__builtin_astype(param,__m128i)));\
	res.hi =  as_ulong2(_mm_cvtepi16_epi64(_mm_srli_si128(__builtin_astype(param,__m128i), 4)));\
	res = convert_ulong4##RMODE((_4i64)res);\
	return res;\
	}\
	_8u64 __attribute__ ((overloadable)) convert_ulong8##RMODE(_8i16 x)\
	{\
	_8u64 res;\
	res.lo.lo = as_ulong2(_mm_cvtepi16_epi64(__builtin_astype(x,__m128i)));\
	res.lo.hi = as_ulong2(_mm_cvtepi16_epi64(_mm_srli_si128(__builtin_astype(x,__m128i), 4)));\
	res.hi.lo = as_ulong2(_mm_cvtepi16_epi64(_mm_srli_si128(__builtin_astype(x,__m128i), 8)));\
	res.hi.hi = as_ulong2(_mm_cvtepi16_epi64(_mm_srli_si128(__builtin_astype(x,__m128i), 12)));\
	res = convert_ulong8##RMODE((_8i64)res);\
	return res;\
	}\
	_16u64 __attribute__ ((overloadable)) convert_ulong16##RMODE(_16i16 x)\
	{\
	_16u64 res;\
	res.lo.lo.lo = as_ulong2(_mm_cvtepi16_epi64(__builtin_astype(x.lo,__m128i)));\
	res.lo.lo.hi = as_ulong2(_mm_cvtepi16_epi64(_mm_srli_si128(__builtin_astype(x.lo,__m128i), 4)));\
	res.lo.hi.lo = as_ulong2(_mm_cvtepi16_epi64(_mm_srli_si128(__builtin_astype(x.lo,__m128i), 8)));\
	res.lo.hi.hi = as_ulong2(_mm_cvtepi16_epi64(_mm_srli_si128(__builtin_astype(x.lo,__m128i), 12)));\
	res.hi.lo.lo = as_ulong2(_mm_cvtepi16_epi64(__builtin_astype(x.hi,__m128i)));\
	res.hi.lo.hi = as_ulong2(_mm_cvtepi16_epi64(_mm_srli_si128(__builtin_astype(x.hi,__m128i), 4)));\
	res.hi.hi.lo = as_ulong2(_mm_cvtepi16_epi64(_mm_srli_si128(__builtin_astype(x.hi,__m128i), 8)));\
	res.hi.hi.hi = as_ulong2(_mm_cvtepi16_epi64(_mm_srli_si128(__builtin_astype(x.hi,__m128i), 12)));\
	res = convert_ulong16##RMODE((_16i64)res);\
	return res;\
	}\

#define DEF_SAT_PROTO64_32(RMODE)\
	_1u64 __attribute__ ((overloadable)) convert_ulong##RMODE(_1i32 x)\
	{\
	if(x < 0) return 0;\
	return (_1u64)x;\
	}\
	_2u64 __attribute__ ((overloadable)) convert_ulong2##RMODE(_2i32 x)\
	{\
	_2u64 res;\
	_4i32 param;\
	param.lo = x;\
	res =  as_ulong2(_mm_cvtepi32_epi64(__builtin_astype(param,__m128i)));\
	res =  convert_ulong2##RMODE((_2i64)res);\
	return res;\
	}\
	ulong3 __attribute__ ((overloadable)) convert_ulong3##RMODE(int3 x)\
	{\
	_4u64 res;\
	int4 y = as_int4(x);\
	res.lo =  as_ulong2(_mm_cvtepi32_epi64(__builtin_astype(y,__m128i)));\
	res.hi =  as_ulong2(_mm_cvtepi32_epi64(_mm_srli_si128(__builtin_astype(y,__m128i), 8)));\
	res = convert_ulong4##RMODE((_4i64)res);\
	return as_ulong3(res);\
	}\
	_4u64 __attribute__ ((overloadable)) convert_ulong4##RMODE(_4i32 x)\
	{\
	_4u64 res;\
	res.lo =  as_ulong2(_mm_cvtepi32_epi64(__builtin_astype(x,__m128i)));\
	res.hi =  as_ulong2(_mm_cvtepi32_epi64(_mm_srli_si128(__builtin_astype(x,__m128i), 8)));\
	res = convert_ulong4##RMODE((_4i64)res);\
	return res;\
	}\
	_8u64 __attribute__ ((overloadable)) convert_ulong8##RMODE(_8i32 x)\
	{\
	_8u64 res;\
	res.lo= convert_ulong4##RMODE(x.lo);\
	res.hi= convert_ulong4##RMODE(x.hi);\
	return res;\
	}\
	_16u64 __attribute__ ((overloadable)) convert_ulong16##RMODE(_16i32 x)\
	{\
	_16u64 res;\
	res.lo= convert_ulong8##RMODE(x.lo);\
	res.hi= convert_ulong8##RMODE(x.hi);\
	return res;\
	}\

#define DEF_SAT_PROTO64_F(TO, TONAME, RMODE, RMODEVAL, CPUTYPE)\
	_1##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long_sat##RMODE(float x)\
	{\
	_1##TO##64 res = __ocl_svml_##CPUTYPE##_cvtfpto##TO##64##RMODEVAL##satf1(x);\
	return res;\
	}\
	_2##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long2_sat##RMODE(float2 x)\
	{\
    float4 param = (float4)(0.f);\
	param.lo=x;\
	_4##TO##64 res = __ocl_svml_##CPUTYPE##_cvtfpto##TO##64##RMODEVAL##satf4(param);\
	return res.lo;\
	}\
	TONAME##long3 __attribute__ ((overloadable)) convert_##TONAME##long3_sat##RMODE(float3 x)\
	{\
	float4 y = as_float4(x);\
	_4##TO##64 res = __ocl_svml_##CPUTYPE##_cvtfpto##TO##64##RMODEVAL##satf4(y);\
	return res.s012;\
	}\
	_4##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long4_sat##RMODE(float4 x)\
	{\
	_4##TO##64 res = __ocl_svml_##CPUTYPE##_cvtfpto##TO##64##RMODEVAL##satf4(x);\
	return res;\
	}\
	_8##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long8_sat##RMODE(float8 x)\
	{\
	_8##TO##64 res = __ocl_svml_##CPUTYPE##_cvtfpto##TO##64##RMODEVAL##satf8(x);\
	return res;\
	}\
	_16##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long16_sat##RMODE(float16 x)\
	{\
	_16##TO##64 res;\
	res.lo = __ocl_svml_##CPUTYPE##_cvtfpto##TO##64##RMODEVAL##satf8(x.lo);\
	res.hi = __ocl_svml_##CPUTYPE##_cvtfpto##TO##64##RMODEVAL##satf8(x.hi);\
	return res;\
	}


#define DEF_SAT_PROTO64_D(TO, TONAME, RMODE, RSVML, CPUTYPE)\
	_1##TO##64 __attribute__((overloadable)) convert_##TONAME##long_sat##RMODE(double x)\
	{\
	_1##TO##64 res = __ocl_svml_##CPUTYPE##_cvtfpto##TO##64##RSVML##sat1(x);\
	return res;\
	}\
	_2##TO##64 __attribute__((overloadable)) convert_##TONAME##long2_sat##RMODE(double2 x)\
	{\
    double4 param = (double4)(0.0);\
	param.lo=x;\
	_4##TO##64 res = __ocl_svml_##CPUTYPE##_cvtfpto##TO##64##RSVML##sat4(param);\
	return res.lo;\
	}\
	TONAME##long3 __attribute__((overloadable)) convert_##TONAME##long3_sat##RMODE(double3 x)\
	{\
	_4##TO##64 res = __ocl_svml_##CPUTYPE##_cvtfpto##TO##64##RSVML##sat4(as_double4(x));\
	return as_##TONAME##long3(res);\
	}\
	_4##TO##64 __attribute__((overloadable)) convert_##TONAME##long4_sat##RMODE(double4 x)\
	{\
	_4##TO##64 res = __ocl_svml_##CPUTYPE##_cvtfpto##TO##64##RSVML##sat4(x);\
	return res;\
	}\
	_8##TO##64 __attribute__((overloadable)) convert_##TONAME##long8_sat##RMODE(double8 x)\
	{\
	_8##TO##64 res = __ocl_svml_##CPUTYPE##_cvtfpto##TO##64##RSVML##sat8(x);\
	return res;\
	}\
	_16##TO##64 __attribute__((overloadable)) convert_##TONAME##long16_sat##RMODE(double16 x)\
	{\
	_16##TO##64 res;\
    res.lo = __ocl_svml_##CPUTYPE##_cvtfpto##TO##64##RSVML##sat8(x.lo);\
    res.hi = __ocl_svml_##CPUTYPE##_cvtfpto##TO##64##RSVML##sat8(x.hi);\
	return res;\
	}\

	// all type to type including all rounding modes, with SAT
#define DEF_SAT_PROTOU8(TI, TYPIN, RMODE, MIN, MAX)\
	_1u8 __attribute__ ((overloadable)) convert_uchar_sat##RMODE(_1##TI##8 x)\
	{\
	if(x < MIN) return MIN;\
	if(x > MAX) return MAX;\
	return (_1u8)x;\
	}\
	_16u8 __attribute__ ((overloadable)) convert_uchar16_sat##RMODE(_16##TI##8 x)\
	{\
	_16u8 res;\
	_16u8 t1 = as_uchar16(_mm_cmpgt_epi8(*((__m128i *)minuChar8), __builtin_astype(x,__m128i)));\
	res = as_uchar16(_mm_andnot_si128(__builtin_astype(t1,__m128i), __builtin_astype(x,__m128i)));\
	t1 = as_uchar16(_mm_and_si128(__builtin_astype(t1,__m128i), *((__m128i *)minuCharVal8)));\
	res = as_uchar16(_mm_or_si128(__builtin_astype(res,__m128i), __builtin_astype(t1,__m128i)));\
	return res;\
	}\
	_2u8 __attribute__ ((overloadable)) convert_uchar2_sat##RMODE(_2##TI##8 x)\
	{\
	_16u8 res;\
	_16##TI##8 param;\
	param.s01 = x;\
	res = convert_uchar16_sat##RMODE(param);\
	return res.s01;\
	}\
	uchar3 __attribute__ ((overloadable)) convert_uchar3_sat##RMODE(TYPIN##char3 x)\
	{\
	_16u8 res;\
	_16##TI##8 param;\
	param.s012 = x;\
	res = convert_uchar16_sat##RMODE(param);\
	return res.s012;\
	}\
	_4u8 __attribute__ ((overloadable)) convert_uchar4_sat##RMODE(_4##TI##8 x)\
	{\
	_16u8 res;\
	_16##TI##8 param;\
	param.s0123 = x;\
	res = convert_uchar16_sat##RMODE(param);\
	return res.s0123;\
	}\
	_8u8 __attribute__ ((overloadable)) convert_uchar8_sat##RMODE(_8##TI##8 x)\
	{\
	_16u8 res;\
	_16##TI##8 param;\
	param.lo = x;\
	res = convert_uchar16_sat##RMODE(param);\
	return res.lo;\
	}

#define DEF_SAT_PROTOI8(TI, TYPIN, RMODE, MAX)\
	_1i8 __attribute__ ((overloadable)) convert_char_sat##RMODE(_1##TI##8 x)\
	{\
	if(x > MAX) return MAX;\
	return (_1i8)x;\
	}\
	_16i8 __attribute__ ((overloadable)) convert_char16_sat##RMODE(_16##TI##8 x)\
	{\
	_16i8 res;\
	_16i8 t1 = as_char16(_mm_cmpgt_epi8(*((__m128i *)minuChar8), __builtin_astype(x,__m128i)));\
	res = as_char16(_mm_andnot_si128(__builtin_astype(t1,__m128i), __builtin_astype(x,__m128i)));\
	t1 = as_char16(_mm_and_si128(__builtin_astype(t1,__m128i), *((__m128i *)miniCharVal8)));\
	res = as_char16(_mm_or_si128(__builtin_astype(res,__m128i), __builtin_astype(t1,__m128i)));\
	return res;\
	}\
	_2i8 __attribute__ ((overloadable)) convert_char2_sat##RMODE(_2##TI##8 x)\
	{\
	_16i8 res;\
	_16##TI##8 param;\
	param.s01 = x;\
	res = convert_char16_sat##RMODE(param);\
	return res.s01;\
	}\
	char3 __attribute__ ((overloadable)) convert_char3_sat##RMODE(TYPIN##char3 x)\
	{\
	_16i8 res;\
	_16##TI##8 param;\
	param.s012 = x;\
	res = convert_char16_sat##RMODE(param);\
	return res.s012;\
	}\
	_4i8 __attribute__ ((overloadable)) convert_char4_sat##RMODE(_4##TI##8 x)\
	{\
	_16i8 res;\
	_16##TI##8 param;\
	param.s0123 = x;\
	res = convert_char16_sat##RMODE(param);\
	return res.s0123;\
	}\
	_8i8 __attribute__ ((overloadable)) convert_char8_sat##RMODE(_8##TI##8 x)\
	{\
	_16i8 res;\
	_16##TI##8 param;\
	param.lo = x;\
	res = convert_char16_sat##RMODE(param);\
	return res.lo;\
	}


#define DEF_SAT_PROTOU16(RMODE)\
	_1u16 __attribute__ ((overloadable)) convert_ushort_sat##RMODE(_1i16 x)\
	{\
	if(x < 0) return 0;\
	return (_1u16)x;\
	}\
	_8u16 __attribute__ ((overloadable)) convert_ushort8_sat##RMODE(_8i16 x)\
	{\
	_8u16 res;\
	_8i16 t1 = as_short8(_mm_cmpgt_epi16(*((__m128i *)minuShort16), __builtin_astype(x,__m128i)));\
	res = as_ushort8(_mm_andnot_si128(__builtin_astype(t1,__m128i), __builtin_astype(x,__m128i)));\
	t1 = as_short8(_mm_and_si128(__builtin_astype(t1,__m128i), *((__m128i *)minuShort16)));\
	res = as_ushort8(_mm_or_si128(__builtin_astype(res,__m128i), __builtin_astype(t1,__m128i)));\
	return res;\
	}\
	_2u16 __attribute__ ((overloadable)) convert_ushort2_sat##RMODE(_2i16 x)\
	{\
	_8u16 res;\
	_8i16 param;\
	param.s01 = x;\
	res = convert_ushort8_sat##RMODE(param);\
	return res.s01;\
	}\
	ushort3 __attribute__ ((overloadable)) convert_ushort3_sat##RMODE(short3 x)\
	{\
	_8u16 res;\
	_8i16 param;\
	param.s012 = x;\
	res = convert_ushort8_sat##RMODE(param);\
	return res.s012;\
	}\
	_4u16 __attribute__ ((overloadable)) convert_ushort4_sat##RMODE(_4i16 x)\
	{\
	_8u16 res;\
	_8i16 param;\
	param.s0123 = x;\
	res = convert_ushort8_sat##RMODE(param);\
	return res.lo;\
	}\
	_16u16 __attribute__ ((overloadable)) convert_ushort16_sat##RMODE(_16i16 x)\
	{\
	_16u16 res;\
	res.lo = convert_ushort8_sat##RMODE(x.lo);\
	res.hi = convert_ushort8_sat##RMODE(x.hi);\
	return res;\
	}

#define DEF_SAT_PROTOI16(RMODE)\
	_1i16 __attribute__ ((overloadable)) convert_short_sat##RMODE(_1u16 x)\
	{\
	if(x > 32767) return 32767;\
	return (_1i16)x;\
	}\
	_8i16 __attribute__ ((overloadable)) convert_short8_sat##RMODE(_8u16 x)\
	{\
	_8i16 res;\
	_8i16 t1 = as_short8(_mm_cmpgt_epi16(*((__m128i *)minuShort16), __builtin_astype(x,__m128i)));\
	res = as_short8(_mm_andnot_si128(__builtin_astype(t1,__m128i), __builtin_astype(x,__m128i)));\
	t1 = as_short8(_mm_and_si128(__builtin_astype(t1,__m128i), *((__m128i *)maxiShort16)));\
	res = as_short8(_mm_or_si128(__builtin_astype(res,__m128i), __builtin_astype(t1,__m128i)));\
	return res;\
	}\
	_2i16 __attribute__ ((overloadable)) convert_short2_sat##RMODE(_2u16 x)\
	{\
	_8i16 res;\
	_8u16 param;\
	param.s01 = x;\
	res = convert_short8_sat##RMODE(param);\
	return res.s01;\
	}\
	short3 __attribute__ ((overloadable)) convert_short3_sat##RMODE(ushort3 x)\
	{\
	_8i16 res;\
	_8u16 param;\
	param.s012 = x;\
	res = convert_short8_sat##RMODE(param);\
	return res.s012;\
	}\
	_4i16 __attribute__ ((overloadable)) convert_short4_sat##RMODE(_4u16 x)\
	{\
	_8i16 res;\
	_8u16 param;\
	param.s0123 = x;\
	res = convert_short8_sat##RMODE(param);\
	return res.lo;\
	}\
	_16i16 __attribute__ ((overloadable)) convert_short16_sat##RMODE(_16u16 x)\
	{\
	_16i16 res;\
	res.lo = convert_short8_sat##RMODE(x.lo);\
	res.hi = convert_short8_sat##RMODE(x.hi);\
	return res;\
	}

#define DEF_SAT_PROTOU32(RMODE)\
	_1u32 __attribute__ ((overloadable)) convert_uint_sat##RMODE(_1i32 x)\
	{\
	if(x < 0) return 0;\
	return (_1u32)x;\
	}\
	_4u32 __attribute__ ((overloadable)) convert_uint4_sat##RMODE(_4i32 x)\
	{\
	_4u32 res;\
	_4u32 t1 = as_uint4(_mm_cmpgt_epi32(*((__m128i *)minuInt32), __builtin_astype(x,__m128i)));\
	res = as_uint4(_mm_andnot_si128(__builtin_astype(t1,__m128i), __builtin_astype(x,__m128i)));\
	return res;\
	}\
	_2u32 __attribute__ ((overloadable)) convert_uint2_sat##RMODE(_2i32 x)\
	{\
	_4u32 res;\
	_4i32 param;\
	param.lo = x;\
	res = convert_uint4_sat##RMODE(param);\
	return res.lo;\
	}\
	uint3 __attribute__ ((overloadable)) convert_uint3_sat##RMODE(int3 x)\
	{\
	int4 y = as_int4(x);\
	_4u32 res = convert_uint4_sat##RMODE(y);\
	return as_uint3(res);\
	}\
	_8u32 __attribute__ ((overloadable)) convert_uint8_sat##RMODE(_8i32 x)\
	{\
	_8u32 res;\
	res.lo = convert_uint4_sat##RMODE(x.lo);\
	res.hi = convert_uint4_sat##RMODE(x.hi);\
	return res;\
	}\
	_16u32 __attribute__ ((overloadable)) convert_uint16_sat##RMODE(_16i32 x)\
	{\
	_16u32 res;\
	res.lo.lo = convert_uint4_sat##RMODE(x.lo.lo);\
	res.lo.hi = convert_uint4_sat##RMODE(x.lo.hi);\
	res.hi.lo = convert_uint4_sat##RMODE(x.hi.lo);\
	res.hi.hi = convert_uint4_sat##RMODE(x.hi.hi);\
	return res;\
	}\

#define DEF_SAT_PROTOI32(RMODE)\
	_1i32 __attribute__ ((overloadable)) convert_int_sat##RMODE(_1u32 x)\
	{\
	if(x > INT_MAX) return INT_MAX;\
	return (_1i32)x;\
	}\
	_4i32 __attribute__ ((overloadable)) convert_int4_sat##RMODE(_4u32 x)\
	{\
	_4i32 res;\
	_4i32 t1 = as_int4(_mm_cmpgt_epi32(*((__m128i *)minuInt32), __builtin_astype(x,__m128i)));\
	res = as_int4(_mm_andnot_si128(__builtin_astype(t1,__m128i), __builtin_astype(x,__m128i)));\
	t1 = as_int4(_mm_and_si128(__builtin_astype(t1,__m128i), *((__m128i *)maxiInt32)));\
	res = as_int4(_mm_or_si128(__builtin_astype(res,__m128i), __builtin_astype(t1,__m128i)));\
	return res;\
	}\
	_2i32 __attribute__ ((overloadable)) convert_int2_sat##RMODE(_2u32 x)\
	{\
	_4i32 res;\
	_4u32 param;\
	param.lo = x;\
	res = convert_int4_sat##RMODE(param);\
	return res.lo;\
	}\
	int3 __attribute__ ((overloadable)) convert_int3_sat##RMODE(uint3 x)\
	{\
	uint4 y = as_uint4(x);\
	_4i32 res = convert_int4_sat##RMODE(y);\
	return as_int3(res);\
	}\
	_8i32 __attribute__ ((overloadable)) convert_int8_sat##RMODE(_8u32 x)\
	{\
	_8i32 res;\
	res.lo = convert_int4_sat##RMODE(x.lo);\
	res.hi = convert_int4_sat##RMODE(x.hi);\
	return res;\
	}\
	_16i32 __attribute__ ((overloadable)) convert_int16_sat##RMODE(_16u32 x)\
	{\
	_16i32 res;\
	res.lo.lo = convert_int4_sat##RMODE(x.lo.lo);\
	res.lo.hi = convert_int4_sat##RMODE(x.lo.hi);\
	res.hi.lo = convert_int4_sat##RMODE(x.hi.lo);\
	res.hi.hi = convert_int4_sat##RMODE(x.hi.hi);\
	return res;\
	}\

#define DEF_SAT_PROTO64(TI, TO, TYPIN, TYPOUT, RMODE, MIN, MAX)\
	_1##TO##64 __attribute__ ((overloadable)) convert_##TYPOUT##long_sat##RMODE(_1##TI##64 x)\
	{\
	if(x < MIN) return MIN;\
	if(x > MAX) return MAX;\
	return (_1##TO##64)x;\
	}\
	_2##TO##64 __attribute__ ((overloadable)) convert_##TYPOUT##long2_sat##RMODE(_2##TI##64 x)\
	{\
	_2##TO##64 res;\
	res.lo = convert_##TYPOUT##long_sat##RMODE(x.lo);\
	res.hi = convert_##TYPOUT##long_sat##RMODE(x.hi);\
	return res;\
	}\
	TYPOUT##long3 __attribute__ ((overloadable)) convert_##TYPOUT##long3_sat##RMODE(TYPIN##long3 x)\
	{\
	TYPOUT##long3 res;\
	res.s01 = convert_##TYPOUT##long2_sat##RMODE(x.s01);\
	res.s2 = convert_##TYPOUT##long_sat##RMODE(x.s2);\
	return res;\
	}\
	_4##TO##64 __attribute__ ((overloadable)) convert_##TYPOUT##long4_sat##RMODE(_4##TI##64 x)\
	{\
	_4##TO##64 res;\
	res.lo = convert_##TYPOUT##long2_sat##RMODE(x.lo);\
	res.hi = convert_##TYPOUT##long2_sat##RMODE(x.hi);\
	return res;\
	}\
	_8##TO##64 __attribute__ ((overloadable)) convert_##TYPOUT##long8_sat##RMODE(_8##TI##64 x)\
	{\
	_8##TO##64 res;\
	res.lo.lo = convert_##TYPOUT##long2_sat##RMODE(x.lo.lo);\
	res.lo.hi = convert_##TYPOUT##long2_sat##RMODE(x.lo.hi);\
	res.hi.lo = convert_##TYPOUT##long2_sat##RMODE(x.hi.lo);\
	res.hi.hi = convert_##TYPOUT##long2_sat##RMODE(x.hi.hi);\
	return res;\
	}\
	_16##TO##64 __attribute__ ((overloadable)) convert_##TYPOUT##long16_sat##RMODE(_16##TI##64 x)\
	{\
	_16##TO##64 res;\
	res.lo.lo = convert_##TYPOUT##long4_sat##RMODE(x.lo.lo);\
	res.lo.hi = convert_##TYPOUT##long4_sat##RMODE(x.lo.hi);\
	res.hi.lo = convert_##TYPOUT##long4_sat##RMODE(x.hi.lo);\
	res.hi.hi = convert_##TYPOUT##long4_sat##RMODE(x.hi.hi);\
	return res;\
	}\


	//out char in all with RMODE
#define DEF_OUT_CHAR_RTX(RMODE, RMODEVAL, FLAG)\
	DEF_INT_PROTO8_8(u, u, u, u, RMODE)\
	DEF_INT_PROTO8_16(u, u, u, u, RMODE)\
	DEF_INT_PROTO8_32(u, u, u, u, RMODE)\
	DEF_INT_PROTO8_64(u, u, u, u, RMODE)\
	DEF_INT_PROTO8_8(u, i, u, , RMODE)\
	DEF_INT_PROTO8_16(u, i, u, , RMODE)\
	DEF_INT_PROTO8_32(u, i, u, , RMODE)\
	DEF_INT_PROTO8_64(u, i, u, , RMODE)\
	DEF_INT_PROTO8_16(i, u, , u, RMODE)\
	DEF_INT_PROTO8_8(i, u, , u, RMODE)\
	DEF_INT_PROTO8_32(i, u, ,u, RMODE)\
	DEF_INT_PROTO8_64(i, u, , u, RMODE)\
	DEF_INT_PROTO8_8(i, i, , , RMODE)\
	DEF_INT_PROTO8_16(i, i, , , RMODE)\
	DEF_INT_PROTO8_32(i, i, , , RMODE)\
	DEF_INT_PROTO8_64(i, i, , , RMODE)\
	DEF_INT_PROTO8_F(i, , RMODE, RMODEVAL, FLAG)\
	DEF_INT_PROTO8_F(u, u, RMODE, RMODEVAL, FLAG)\
	DEF_INT_PROTO8_D(i, , RMODE, RMODEVAL)\
	DEF_INT_PROTO8_D(u, u, RMODE, RMODEVAL)


#define DEF_OUT_CHAR()\
	DEF_OUT_CHAR_RTX(, 0x6000,)\
	DEF_OUT_CHAR_RTX(_rtz, 0x6000,)\
	DEF_OUT_CHAR_RTX(_rte, 0x0, Round)\
	DEF_OUT_CHAR_RTX(_rtn, 0x2000, Round)\
	DEF_OUT_CHAR_RTX(_rtp, 0x4000, Round)

	//out short in all with RMODE
#define DEF_OUT_SHORT_RTX(RMODE, RMODEVAL, FLAG)\
	DEF_INT_PROTO16_8(z, u, u, u, u, RMODE)\
	DEF_INT_PROTO16_32(u, u, u, u, RMODE)\
	DEF_INT_PROTO16_16(u, u, u, u, RMODE)\
	DEF_INT_PROTO16_64(u, u, u, u, RMODE)\
	DEF_INT_PROTO16_8(z, u, i, u, , RMODE)\
	DEF_INT_PROTO16_16(u, i, u, , RMODE)\
	DEF_INT_PROTO16_32(u, i, u, , RMODE)\
	DEF_INT_PROTO16_64(u, i, u, , RMODE)\
	DEF_INT_PROTO16_8(s, i, u, , u, RMODE)\
	DEF_INT_PROTO16_16(i, u, , u, RMODE)\
	DEF_INT_PROTO16_32(i, u, , u, RMODE)\
	DEF_INT_PROTO16_64(i, u, , u, RMODE)\
	DEF_INT_PROTO16_8(s, i, i, , , RMODE)\
	DEF_INT_PROTO16_16(i, i, , , RMODE)\
	DEF_INT_PROTO16_32(i, i, , , RMODE)\
	DEF_INT_PROTO16_64(i, i, , , RMODE)\
	DEF_INT_PROTO16_F(i, , RMODE, RMODEVAL, FLAG)\
	DEF_INT_PROTO16_F(u, u, RMODE, RMODEVAL, FLAG)\
	DEF_INT_PROTO16_D(i, , RMODE, RMODEVAL)\
	DEF_INT_PROTO16_D(u, u, RMODE, RMODEVAL)


#define DEF_OUT_SHORT()\
	DEF_OUT_SHORT_RTX(, 0x6000,)\
	DEF_OUT_SHORT_RTX(_rtz, 0x6000,)\
	DEF_OUT_SHORT_RTX(_rte, 0x0, Round)\
	DEF_OUT_SHORT_RTX(_rtn, 0x2000, Round)\
	DEF_OUT_SHORT_RTX(_rtp, 0x4000, Round)

	//out int in all with RMODE
#define DEF_OUT_INT_RTX(RMODE, RMODEVAL, RSVML, CPUTYPE, FLAGSVML)\
	DEF_INT_PROTO32_8(z, u, u, u, u, RMODE)\
	DEF_INT_PROTO32_8(z, u, i, u, , RMODE)\
	DEF_INT_PROTO32_8(s, i, u, , u, RMODE)\
	DEF_INT_PROTO32_8(s, i, i, , , RMODE)\
	DEF_INT_PROTO32_16(z, u, u, u, u, RMODE)\
	DEF_INT_PROTO32_16(z, u, i, u, , RMODE)\
	DEF_INT_PROTO32_16(s, i, u, , u, RMODE)\
	DEF_INT_PROTO32_16(s, i, i, , , RMODE)\
	DEF_INT_PROTO32_32(u, u, u, u, RMODE)\
	DEF_INT_PROTO32_32(u, i, u, , RMODE)\
	DEF_INT_PROTO32_32(i, u, , u, RMODE)\
	DEF_INT_PROTO32_32(i, i, , , RMODE)\
	DEF_INT_PROTO32_64(u, u, u, u, RMODE)\
	DEF_INT_PROTO32_64(u, i, u, , RMODE)\
	DEF_INT_PROTO32_64(i, u, , u, RMODE)\
	DEF_INT_PROTO32_64(i, i, , , RMODE)\
    DEF_INT_PROTOI32_F##FLAGSVML(RMODE, RMODEVAL, RSVML, CPUTYPE)\
	DEF_INT_PROTOU32_F##FLAGSVML(RMODE, RMODEVAL, RSVML, CPUTYPE)\
	DEF_INT_PROTOI32_D##FLAGSVML(RMODE, RMODEVAL, RSVML, CPUTYPE)\
	DEF_INT_PROTOU32_D(RMODE, RSVML, CPUTYPE)

#define DEF_OUT_INT()\
	DEF_OUT_INT_RTX(, 0x6000, rtz, CTYPE, NOSVML)\
	DEF_OUT_INT_RTX(_rtz, 0x6000, rtz, CTYPE, NOSVML)\
	DEF_OUT_INT_RTX(_rte, 0x0, rte, CTYPE, USESVML)\
	DEF_OUT_INT_RTX(_rtn, 0x2000, rtn, CTYPE, USESVML)\
	DEF_OUT_INT_RTX(_rtp, 0x4000, rtp, CTYPE, USESVML)

	//out long in all with RMODE
#define DEF_OUT_LONG_RTX(RMODE, RMODEVAL, RSVML, CPUTYPE)\
	DEF_INT_PROTO64_8(u, u, u, u,RMODE)\
	DEF_INT_PROTO64_8(u, i, u, , RMODE)\
	DEF_INT_PROTO64_8(i, u, , u, RMODE)\
	DEF_INT_PROTO64_8(i, i, , , RMODE)\
	DEF_INT_PROTO64_16(u, u, u, u, RMODE)\
	DEF_INT_PROTO64_16(u, i, u, , RMODE)\
	DEF_INT_PROTO64_16(i, u, , u, RMODE)\
	DEF_INT_PROTO64_16(i, i, , , RMODE)\
	DEF_INT_PROTO64_32(u, u, u, u, RMODE)\
	DEF_INT_PROTO64_32(u, i, u, , RMODE)\
	DEF_INT_PROTO64_32(i, u, , u, RMODE)\
	DEF_INT_PROTO64_32(i, i, , , RMODE)\
	DEF_INT_PROTO64_64(u, u, u, u, RMODE)\
	DEF_INT_PROTO64_64(u, i, u, , RMODE)\
	DEF_INT_PROTO64_64(i, u, , u, RMODE)\
	DEF_INT_PROTO64_64(i, i, , , RMODE) \
	DEF_INT_PROTO64_F(i, , RMODE, RSVML, CPUTYPE)\
	DEF_INT_PROTO64_F(u, u, RMODE, RSVML, CPUTYPE)\
	DEF_INT_PROTO64_D(i, , RMODE, RSVML, CPUTYPE)\
	DEF_INT_PROTO64_D(u, u, RMODE, RSVML, CPUTYPE)

#define DEF_OUT_LONG()\
	DEF_OUT_LONG_RTX(, 0x6000, rtz, CTYPE)\
	DEF_OUT_LONG_RTX(_rtz, 0x6000, rtz, CTYPE)\
	DEF_OUT_LONG_RTX(_rte, 0x0, rte, CTYPE)\
	DEF_OUT_LONG_RTX(_rtn, 0x2000, rtn, CTYPE)\
	DEF_OUT_LONG_RTX(_rtp, 0x4000, rtp, CTYPE)

	//out float in all with RMODE
#define DEF_OUT_FLOAT_RTX(RMODE, RMODEVAL, RSVML, RSTACK, FLAG, CPUTYPE, FLAGSVML)\
	DEF_INT_PROTOF_8(i, , RMODE)\
	DEF_INT_PROTOF_8(u, u, RMODE)\
	DEF_INT_PROTOF_16(i, , RMODE)\
	DEF_INT_PROTOF_16(u, u, RMODE)\
	DEF_INT_PROTOF_I32##FLAGSVML(RMODE, RMODEVAL, RSVML, CPUTYPE,)\
	DEF_INT_PROTOF_U32##FLAGSVML(RMODE, RMODEVAL, RSVML, CPUTYPE,)\
	DEF_INT_PROTOF_F(, , , , RMODE)\
    DEF_INT_PROTOF_D##FLAGSVML(RMODE, RMODEVAL, RSVML, CPUTYPE)\
	DEF_INT_PROTOF_64(i, , RMODE, RSVML, CPUTYPE)\
	DEF_INT_PROTOF_64(u, u, RMODE, RSVML, CPUTYPE)

#define DEF_OUT_FLOAT()\
	DEF_OUT_FLOAT_RTX(,0x0, rte, 0x0000, ,CTYPE, NOSVML)\
	DEF_OUT_FLOAT_RTX(_rtz, 0x6000, rtz, 0x0300, Round, CTYPE, USESVML)\
	DEF_OUT_FLOAT_RTX(_rte, 0x0, rte, 0x0000, , CTYPE, NOSVML)\
	DEF_OUT_FLOAT_RTX(_rtn, 0x2000, rtn, 0x0100, Round, CTYPE, USESVML)\
	DEF_OUT_FLOAT_RTX(_rtp, 0x4000, rtp, 0x0200, Round, CTYPE, USESVML)

	//out double in all with RMODE
#define DEF_OUT_DOUBLE_RTX(RMODE, RMODEVAL, RSVML, CPUTYPE)\
	DEF_INT_PROTOD_8(i, , RMODE)\
	DEF_INT_PROTOD_8(u, u, RMODE)\
	DEF_INT_PROTOD_16(i, , RMODE)\
	DEF_INT_PROTOD_16(u, u, RMODE)\
	DEF_INT_PROTOD_I32(RMODE, RMODEVAL)\
	DEF_INT_PROTOD_F(RMODE)\
	DEF_INT_PROTOD_D(RMODE) \
	DEF_INT_PROTOD_U32(RMODE, RSVML, CPUTYPE)\
	DEF_INT_PROTOD_64(i, , RMODE, RSVML, CPUTYPE)\
	DEF_INT_PROTOD_64(u, u, RMODE, RSVML, CPUTYPE)\


#define DEF_OUT_DOUBLE()\
	DEF_OUT_DOUBLE_RTX(,0x0, rte, CTYPE)\
	DEF_OUT_DOUBLE_RTX(_rtz, 0x6000, rtz, CTYPE)\
	DEF_OUT_DOUBLE_RTX(_rte, 0x0, rte, CTYPE)\
	DEF_OUT_DOUBLE_RTX(_rtn, 0x2000, rtn, CTYPE)\
	DEF_OUT_DOUBLE_RTX(_rtp, 0x4000, rtp, CTYPE)

	//SAT

	//out char in all with RMODE
#define DEF_OUT_CHAR_SAT(RMODE, RMODEVAL, FLAG)\
	DEF_SAT_PROTO8_I16(u, u, RMODE, _UCHAR_MAX, 0)\
	DEF_SAT_PROTO8_I32(u, u, RMODE, _UCHAR_MAX, 0)\
	DEF_SAT_PROTO8_I64(u, u, RMODE, _UCHAR_MAX, 0)\
	DEF_SAT_PROTO8_I16(i, , RMODE, _CHAR_MAX, _CHAR_MIN)\
	DEF_SAT_PROTO8_I32(i, , RMODE, _CHAR_MAX, _CHAR_MIN)\
	DEF_SAT_PROTO8_I64(i, , RMODE, _CHAR_MAX, _CHAR_MIN)\
	DEF_SAT_PROTO8_U16(u, u,RMODE, _UCHAR_MAX)\
	DEF_SAT_PROTO8_U32(u, u, RMODE, _UCHAR_MAX)\
	DEF_SAT_PROTO8_U64(u, u, RMODE, _UCHAR_MAX)\
	DEF_SAT_PROTO8_U16(i, , RMODE, _CHAR_MAX)\
	DEF_SAT_PROTO8_U32(i, , RMODE, _CHAR_MAX)\
	DEF_SAT_PROTO8_U64(i, , RMODE, _CHAR_MAX)\
	DEF_SAT_PROTO8_F(i, , RMODE, RMODEVAL, 127.0f, -128.0f, FLAG)\
	DEF_SAT_PROTO8_F(u, u, RMODE, RMODEVAL, 255.0f, 0, FLAG)\
	DEF_SAT_PROTO8_D(i, , RMODE, RMODEVAL)\
	DEF_SAT_PROTO8_D(u, u, RMODE, RMODEVAL)\
	DEF_SAT_PROTOU8(i, , RMODE, 0, _UCHAR_MAX)\
	DEF_SAT_PROTOI8(u, u, RMODE,_CHAR_MAX)\
	DEF_INT_PROTO8_8(u, u, u, u, _sat##RMODE)\
	DEF_INT_PROTO8_8(i, i, , , _sat##RMODE)

#define DEF_SAT_CHAR()\
	DEF_OUT_CHAR_SAT(, 0x6000,)\
	DEF_OUT_CHAR_SAT(_rtz, 0x6000,)\
	DEF_OUT_CHAR_SAT(_rte, 0x0, Round)\
	DEF_OUT_CHAR_SAT(_rtn, 0x2000, Round)\
	DEF_OUT_CHAR_SAT(_rtp, 0x4000, Round)

	//out short in all with RMODE
#define DEF_OUT_SHORT_SAT(RMODE, RMODEVAL, FLAG)\
	DEF_SAT_PROTOU16(RMODE)\
	DEF_SAT_PROTOI16(RMODE)\
	DEF_INT_PROTO16_16(u, u, u, u, _sat##RMODE)\
	DEF_INT_PROTO16_16(i, i, , , _sat##RMODE)\
	DEF_INT_PROTO16_8(z, u, u, u, u, _sat##RMODE)\
	DEF_SAT_PROTO16_I32(u, u, RMODE, 65535, 0)\
	DEF_SAT_PROTO16_I64(u, u, RMODE, 65535, 0)\
	DEF_INT_PROTO16_8(z, u, i, u, , _sat##RMODE)\
	DEF_SAT_PROTO16_I32(i, , RMODE, 32767, -32768)\
	DEF_SAT_PROTO16_I64(i, , RMODE, 32767, -32768)\
	DEF_SAT_PROTO16_8(i, u, , u, _sat##RMODE)\
	DEF_SAT_PROTO16_U32(u, u, RMODE, 65535)\
	DEF_SAT_PROTO16_U64(u, u, RMODE, 65535)\
	DEF_INT_PROTO16_8(s, i, i, , , _sat##RMODE)\
	DEF_SAT_PROTO16_U32(i, , RMODE, 32767)\
	DEF_SAT_PROTO16_U64(i, , RMODE, 32767)\
	DEF_SAT_PROTO16_F(i, , RMODE, RMODEVAL, FLAG)\
	DEF_SAT_PROTO16_F(u, u, RMODE, RMODEVAL, FLAG)\
	DEF_SAT_PROTO16_D(i, , RMODE, RMODEVAL)\
	DEF_SAT_PROTO16_D(u, u, RMODE, RMODEVAL)

#define DEF_SAT_SHORT()\
	DEF_OUT_SHORT_SAT( , 0x6000,)\
	DEF_OUT_SHORT_SAT(_rtz, 0x6000,)\
	DEF_OUT_SHORT_SAT(_rte, 0x0, Round)\
	DEF_OUT_SHORT_SAT(_rtn, 0x2000, Round)\
	DEF_OUT_SHORT_SAT(_rtp, 0x4000, Round)

	//out int in all with RMODE
#define DEF_OUT_INT_SAT(RMODE, RMODEVAL, RSVML, CPUTYPE, FLAGSVML)\
	DEF_SAT_PROTOU32(RMODE)\
	DEF_SAT_PROTOI32(RMODE)\
	DEF_INT_PROTO32_8( z, u, u, u, u, _sat##RMODE)\
	DEF_INT_PROTO32_8( z, u, i, u, , _sat##RMODE)\
	DEF_SAT_PROTOU32_I8(i, u, , u, _sat##RMODE)\
	DEF_INT_PROTO32_8( s, i, i, , , _sat##RMODE)\
	DEF_INT_PROTO32_16(z, u, u, u , u, _sat##RMODE)\
	DEF_INT_PROTO32_16(z, u, i, u, , _sat##RMODE)\
	DEF_SAT_PROTOU32_I16(i, u, , u, _sat##RMODE)\
	DEF_INT_PROTO32_16(s, i, i, , , _sat##RMODE)\
	DEF_SAT_PROTO32_I64(u, u, RMODE, _UINT_MAX, 0)\
	DEF_SAT_PROTO32_I64(i, , RMODE, _INT_MAX, _INT_MIN)\
	DEF_SAT_PROTO32_U64(u, u, RMODE, _UINT_MAX)\
	DEF_SAT_PROTO32_U64(i, , RMODE, _INT_MAX)\
	DEF_SAT_PROTOI32_F##FLAGSVML(RMODE, RMODEVAL, _INT_MAX, _INT_MIN, RSVML, CPUTYPE)\
	DEF_SAT_PROTOU32_F##FLAGSVML(RMODE, RMODEVAL, RSVML, CPUTYPE)\
	DEF_SAT_PROTOI32_D##FLAGSVML(RMODE, RMODEVAL, RSVML, CPUTYPE)\
	DEF_INT_PROTO32_32(u, u, u, u, _sat##RMODE)\
	DEF_INT_PROTO32_32(i, i, , , _sat##RMODE) \
	DEF_SAT_PROTOU32_D(RMODE, RSVML, CPUTYPE)

#define DEF_SAT_INT()\
	DEF_OUT_INT_SAT(    , 0x6000, rtz,  CTYPE, NOSVML)\
	DEF_OUT_INT_SAT(_rtz, 0x6000, rtz,  CTYPE, NOSVML)\
	DEF_OUT_INT_SAT(_rte, 0x0,    rte,  CTYPE, USESVML)\
	DEF_OUT_INT_SAT(_rtn, 0x2000, rtn, CTYPE, USESVML)\
	DEF_OUT_INT_SAT(_rtp, 0x4000, rtp,   CTYPE, USESVML)

	//out long in all with RMODE
#define DEF_OUT_LONG_SAT(RMODE, RMODEVAL, RSVML, CPUTYPE)\
	DEF_SAT_PROTO64(u, i, u, , RMODE, 0, 9223372036854775807)\
	DEF_SAT_PROTO64(i, u, , u, RMODE, 0, 9223372036854775807)\
	DEF_INT_PROTO64_8(u, u, u, u, _sat##RMODE)\
	DEF_INT_PROTO64_8(u, i, u, , _sat##RMODE)\
	DEF_SAT_PROTO64_8(_sat##RMODE)\
	DEF_INT_PROTO64_8(i, i, , , _sat##RMODE)\
	DEF_INT_PROTO64_16(u, u, u, u, _sat##RMODE)\
	DEF_INT_PROTO64_16(u, i, u, , _sat##RMODE)\
	DEF_SAT_PROTO64_16(_sat##RMODE)\
	DEF_INT_PROTO64_16(i, i, , , _sat##RMODE)\
	DEF_INT_PROTO64_32(u, u, u, u, _sat##RMODE)\
	DEF_INT_PROTO64_32(u, i, u, , _sat##RMODE)\
	DEF_SAT_PROTO64_32(_sat##RMODE)\
	DEF_INT_PROTO64_32(i, i, , , _sat##RMODE)\
	DEF_INT_PROTO64_64(u, u, u, u, _sat##RMODE)\
	DEF_INT_PROTO64_64(i, i, , , _sat##RMODE) \
	DEF_SAT_PROTO64_F(i, , RMODE, RSVML, CPUTYPE)\
	DEF_SAT_PROTO64_F(u, u, RMODE, RSVML, CPUTYPE)\
	DEF_SAT_PROTO64_D(i, , RMODE, RSVML, CPUTYPE)\
	DEF_SAT_PROTO64_D(u, u, RMODE, RSVML, CPUTYPE)

#define DEF_SAT_LONG()\
	DEF_OUT_LONG_SAT(, 0x6000, rtz, CTYPE)\
	DEF_OUT_LONG_SAT(_rtz, 0x6000, rtz, CTYPE)\
	DEF_OUT_LONG_SAT(_rte, 0x0, rte, CTYPE)\
	DEF_OUT_LONG_SAT(_rtn, 0x2000, rtn, CTYPE)\
	DEF_OUT_LONG_SAT(_rtp, 0x4000, rtp, CTYPE)

//#define DEF_OUT_FLOAT_RTX(RMODE, RMODEVAL, RSVML, RSTACK, FLAG, CPUTYPE)\

// create all conversion functions
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration" 
DEF_OUT_CHAR()  
DEF_OUT_SHORT() 
DEF_OUT_INT()   
DEF_OUT_LONG()  
#pragma GCC diagnostic warning "-Wimplicit-function-declaration" 
DEF_OUT_FLOAT()
DEF_OUT_DOUBLE()
DEF_SAT_CHAR()
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
DEF_SAT_SHORT()  
#pragma GCC diagnostic warning "-Wimplicit-function-declaration"
DEF_SAT_INT()
DEF_SAT_LONG()

#ifdef __cplusplus
}
#endif

#endif // defined (__MIC__) || defined(__MIC2__)
