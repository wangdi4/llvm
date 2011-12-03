
#ifdef __cplusplus
extern "C" {
#endif

#if defined(__AVX__)
 /*
  * AVX requires svml g9 for 32-bit dll
  * and               e9 for 64-bit dll
  *
  */
  #if defined(_WIN64) || defined(__x86_64__) || defined(__LP64__)
  #define CTYPE e9
  #define OCL_SVML_FUNCTION(oclfunc) __ocl_svml_##e9##_##oclfunc
  #else
  #define CTYPE g9
  #define OCL_SVML_FUNCTION(oclfunc) __ocl_svml_##g9##_##oclfunc
  #endif
#elif defined (__SSE4_2__)
  #if defined(_WIN64) || defined(__x86_64__) || defined(__LP64__)
  #define CTYPE h8
  #define OCL_SVML_FUNCTION(oclfunc) __ocl_svml_##h8##_##oclfunc
  #else
  #define CTYPE n8
  #define OCL_SVML_FUNCTION(oclfunc) __ocl_svml_##n8##_##oclfunc
  #endif
#elif defined(__SSE4_1__)
  #if defined(_WIN64) || defined(__x86_64__) || defined(__LP64__)
  #define CTYPE y8
  #define OCL_SVML_FUNCTION(oclfunc) __ocl_svml_##y8##_##oclfunc
  #else
  #define CTYPE p8
  #define OCL_SVML_FUNCTION(oclfunc) __ocl_svml_##p8##_##oclfunc
  #endif
#elif defined(__SSSE3__)
  #if defined(_WIN64) || defined(__x86_64__) || defined(__LP64__)  
  #define CTYPE u8
  #define OCL_SVML_FUNCTION(oclfunc) __ocl_svml_##u8##_##oclfunc
  #else
  #define CTYPE v8
  #define OCL_SVML_FUNCTION(oclfunc) __ocl_svml_##v8##_##oclfunc
  #endif
#elif defined(__SSE3__)
  #if defined(_WIN64) || defined(__x86_64__) || defined(__LP64__)
  #define CTYPE e7
  #define OCL_SVML_FUNCTION(oclfunc) __ocl_svml_##e7##_##oclfunc
  #else
  #define CTYPE t7
  #define OCL_SVML_FUNCTION(oclfunc) __ocl_svml_##t7##_##oclfunc
  #endif
#else
  #error SSE artchitecture was not defined
#endif // __SSE4_2__

#ifdef _WIN32
#ifdef CL_BUILTIN_FUNCTIONS_EXPORTS
#define CONVERSIONS_FUNC_DECL __declspec(dllexport)
#else
#define CONVERSIONS_FUNC_DECL __declspec(dllimport)
#endif
#else
#define CONVERSIONS_FUNC_DECL 
#endif

#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#if defined(_MSC_VER)
#include <smmintrin.h>
#include <xmmintrin.h>
#if defined(__AVX__)
#include <immintrin.h>
#endif
#include <float.h>
#include <stdio.h>
#define ALIGN16 __declspec(align(16))
#define ALIGN32 __declspec(align(32))
#define __attribute__(X) 
#include "cl_types2.h"
#else
#include <smmintrin.h>
#include <xmmintrin.h>
#if defined(__AVX__)
#include <immintrin.h>
#endif

//#include <float.h>
#include <stdio.h>
#define ALIGN16 __attribute__((aligned(16)))
#define ALIGN32 __attribute__((aligned(32)))
#include "cl_types2.h"
#endif
#else
#include <intrin.h> 
#if defined(__AVX__)
#include <avxintrin.h>
#endif
#define ALIGN16 __attribute__((aligned(16)))
#define ALIGN32 __attribute__((aligned(16)))
#include "cl_types2.h"
#include "conversions_svml.inc"
#endif

#include "conversions_workaround.h"

#if defined(_MSC_VER) || defined(__INTEL_COMPILER)

	#ifdef __SSSE3__
		#define SHUFFLE_EPI8(x, mask)\
			_mm_shuffle_epi8(x, mask)
	#else
		__m128i shuffle_epi8(__m128i x, __m128i mask)
		{
			ALIGN16 _1i8 tempX[16];											
			ALIGN16 _1i8 tampMask[16];										
			ALIGN16 _1i8 result[16];										
																			
			_mm_store_si128((__m128i *)tempX, x);							
			_mm_store_si128((__m128i *)tampMask, mask);					
			for(int i=0; i<16; ++i)											
			{																
				_1i8 index = tampMask[i] & 15;								
				result[i] = (tampMask[i] < 0) ? 0 : tempX[index];			
			}																
			return _mm_load_si128((const __m128i *)result);					
		}																	
																			
		#define SHUFFLE_EPI8(x, mask)\
			shuffle_epi8(x, mask)											
	#endif

	ALIGN16 int x7bff[] = {0x7bff, 0x7bff, 0x7bff, 0x7bff};
	ALIGN16 int x8000[] = {0x8000, 0x8000, 0x8000, 0x8000};
	ALIGN16 int x7fff[] = {0x7fff, 0x7fff, 0x7fff, 0x7fff};
	ALIGN16 int x0200[] = {0x0200, 0x0200, 0x0200, 0x0200};
	ALIGN16 int x7c00[] = {0x7c00, 0x7c00, 0x7c00, 0x7c00};
	ALIGN16 int xfbff[] = {0xfbff, 0xfbff, 0xfbff, 0xfbff};
	ALIGN16 int xfc00[] = {0xfc00, 0xfc00, 0xfc00, 0xfc00};
	ALIGN16 int x8001[] = {0x8001, 0x8001, 0x8001, 0x8001};
			
	ALIGN16 int x7fffffff[] = {0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff};
	ALIGN16 int x7f800000[] = {0x7f800000, 0x7f800000, 0x7f800000, 0x7f800000};
	ALIGN16 int x47800000[] = {0x47800000, 0x47800000, 0x47800000, 0x47800000};
	ALIGN16 int x33800000[] = {0x33800000, 0x33800000, 0x33800000, 0x33800000};
	ALIGN16 int x38800000[] = {0x38800000, 0x38800000, 0x38800000, 0x38800000};
	ALIGN16 int x4b800000[] = {0x4b800000, 0x4b800000, 0x4b800000, 0x4b800000};
	ALIGN16 int xffffe000[] = {0xffffe000, 0xffffe000, 0xffffe000, 0xffffe000};
	ALIGN16 int x38000000[] = {0x38000000, 0x38000000, 0x38000000, 0x38000000};
	ALIGN16 int x477ff000[] = {0x477ff000, 0x477ff000, 0x477ff000, 0x477ff000};
	ALIGN16 int xc7800000[] = {0xc7800000, 0xc7800000, 0xc7800000, 0xc7800000};
	ALIGN16 int xff800000[] = {0xff800000, 0xff800000, 0xff800000, 0xff800000};
	ALIGN16 int xc77fe000[] = {0xc77fe000, 0xc77fe000, 0xc77fe000, 0xc77fe000};
	ALIGN16 int x00002000[] = {0x00002000, 0x00002000, 0x00002000, 0x00002000};
	ALIGN16 int x33000000[] = {0x33000000, 0x33000000, 0x33000000, 0x33000000};
	ALIGN16 int x33c00000[] = {0x33c00000, 0x33c00000, 0x33c00000, 0x33c00000};
	ALIGN16 int x01000000[] = {0x01000000, 0x01000000, 0x01000000, 0x01000000};
	ALIGN16 int x46000000[] = {0x46000000, 0x46000000, 0x46000000, 0x46000000};
	ALIGN16 int x07800000[] = {0x07800000, 0x07800000, 0x07800000, 0x07800000};
	
	ALIGN16 __int64 x7fffffffffffffff[] = {0x7fffffffffffffff, 0x7fffffffffffffff};
	ALIGN16 __int64 x7ff0000000000000[] = {0x7ff0000000000000, 0x7ff0000000000000};
	ALIGN16 __int64 x40f0000000000000[] = {0x40f0000000000000, 0x40f0000000000000};
	ALIGN16 __int64 x3e70000000000000[] = {0x3e70000000000000, 0x3e70000000000000};
	ALIGN16 __int64 x3f10000000000000[] = {0x3f10000000000000, 0x3f10000000000000};
	ALIGN16 __int64 x4170000000000000[] = {0x4170000000000000, 0x4170000000000000};
	ALIGN16 __int64 xFFFFFC0000000000[] = {0xFFFFFC0000000000, 0xFFFFFC0000000000};
	ALIGN16 __int64 x3F00000000000000[] = {0x3F00000000000000, 0x3F00000000000000};
	ALIGN16 __int64 x40effe0000000000[] = {0x40effe0000000000, 0x40effe0000000000};
	ALIGN16 __int64 x40effc0000000000[] = {0x40effc0000000000, 0x40effc0000000000};
	ALIGN16 __int64 x00f0000000000000[] = {0x00f0000000000000, 0x00f0000000000000};
	ALIGN16 __int64 x4290000000000000[] = {0x4290000000000000, 0x4290000000000000};
	ALIGN16 __int64 x0000000001000000[] = {0x0000000001000000, 0x0000000001000000};
	ALIGN16 __int64 x3e78000000000000[] = {0x3e78000000000000, 0x3e78000000000000};
	ALIGN16 __int64 x3e60000000000000[] = {0x3e60000000000000, 0x3e60000000000000};
	ALIGN16 __int64 x0000040000000000[] = {0x0000040000000000, 0x0000040000000000};
	ALIGN16 __int64 xc0effc0000000000[] = {0xc0effc0000000000, 0xc0effc0000000000};
	ALIGN16 __int64 xfff0000000000000[] = {0xfff0000000000000, 0xfff0000000000000};
	ALIGN16 __int64 xc0f0000000000000[] = {0xc0f0000000000000, 0xc0f0000000000000};
		
	ALIGN16 int ones[] = {1, 1, 1, 1};
	ALIGN16 __int64 dones[] = {1, 1};
	ALIGN16 char _4x32to4x16[] = {0, 1, 4, 5, 8, 9, 12, 13, -1, -1, -1, -1, -1, -1, -1, -1};
	ALIGN16 char _2x64to2x16[] = {0, 1, 8, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};


//Klocwork warning Buffer overflow - this is known workaround to existing issue in Intel Compiler for win32”
#if defined(WIN32) && !defined(_DEBUG) && !defined(_M_X64)
#define ROUND_MODE(x) (*(&x - 4))
#else
#define ROUND_MODE(x) x
#endif

CONVERSIONS_FUNC_DECL __m128 double2ToFloat4Round(double2 x, int rMode) 
{

  int rm = _mm_getcsr();
  _mm_setcsr((rm& ~_MM_ROUND_MASK) | ROUND_MODE(rMode));
  __m128 res = _mm_cvtpd_ps(x);
  _mm_setcsr(rm);
  return res;
} 

#if defined(__AVX__)
CONVERSIONS_FUNC_DECL __m128 double4ToFloat4Round(__m256d x, int rMode) 
{
  int rm = _mm_getcsr();
  _mm_setcsr((rm& ~_MM_ROUND_MASK) | rMode);
  __m128 res = _mm256_cvtpd_ps(x);
  _mm_setcsr(rm);
  return res;
} 
#endif // defined(__AVX__)

#if defined(__AVX__)
CONVERSIONS_FUNC_DECL __m256i float8ToInt8Round(__m256 param, int rMode)
{
	int rm = _mm_getcsr();
	_mm_setcsr((rm& ~_MM_ROUND_MASK) | rMode);
	__m256i res = _mm256_cvtps_epi32(param);
	_mm_setcsr(rm);
	return res;
}
#endif // defined(__AVX__)

CONVERSIONS_FUNC_DECL __m128i float4ToInt4Round(float4 param, int rMode)
{
	int rm = _mm_getcsr();
	_mm_setcsr((rm& ~_MM_ROUND_MASK) | ROUND_MODE(rMode));
	__m128i res = _mm_cvtps_epi32(param);
	_mm_setcsr(rm);
	return res;
}

CONVERSIONS_FUNC_DECL int floatToIntRound(float4 param, int rMode)
{

	int rm = _mm_getcsr();
	_mm_setcsr((rm& ~_MM_ROUND_MASK) | ROUND_MODE(rMode));
	int res = _mm_cvtss_si32(param);
	_mm_setcsr(rm);
	return res;
}

	CONVERSIONS_FUNC_DECL __m128i double2ToInt4(double2 param, int rMode)
	{
		int rm = _mm_getcsr();
		_mm_setcsr((rm& ~_MM_ROUND_MASK) | ROUND_MODE(rMode));
		__m128i res = _mm_cvtpd_epi32(param);
		_mm_setcsr(rm);
		return res;
	} 

	CONVERSIONS_FUNC_DECL float ulongToFloat(_1u64 param, int rm)
	{
		unsigned int oldRound, tmp;
		if (param == 0) return 0.0f;
#if defined(_MSC_VER)
		_controlfp_s(&oldRound, _PC_64, _MCW_PC);
		_controlfp_s(&oldRound, rm, _MCW_RC);
		float res = (float)param;
		_controlfp_s(&tmp,oldRound, _MCW_RC);
#else
		float res = (float)param;
#endif
		return res;
	}

CONVERSIONS_FUNC_DECL float uintToFloatRound(_1u32 x, int rm)
{
	unsigned int oldRound, tmp;
	if(x == 0) return 0.0f;
  oldRound = _mm_getcsr();
  tmp = oldRound | rm;
  _mm_setcsr(tmp);
  float res = ((float)x);
  _mm_setcsr(oldRound);
	return res;
}

	CONVERSIONS_FUNC_DECL float4 intToFloatRound(_4i32 param, int rMode)
	{

		int crm = _mm_getcsr();
		_mm_setcsr((crm& ~_MM_ROUND_MASK) | ROUND_MODE(rMode));
		float4 res = _mm_cvtepi32_ps(param);
		_mm_setcsr(crm);
		return res;
	}
#if defined(__AVX__)
    CONVERSIONS_FUNC_DECL __m256 intToFloat8Round(__m256i param, int rMode)
	{
        int crm = _mm_getcsr();
        // ROUND_MODE is not needed here since Intel Compiler correctly obtains argument from stack
        _mm_setcsr((crm& ~_MM_ROUND_MASK) | rMode);
        __m256 res = _mm256_cvtepi32_ps(param);
        _mm_setcsr(crm);
        return res;
	}
#endif // defined(__AVX__)

    CONVERSIONS_FUNC_DECL  _1u32 floatToUintRound(float param, int rMode)
    {
        int crm = _mm_getcsr();
        _mm_setcsr((crm& ~_MM_ROUND_MASK) | (rMode));
        _1u32 res;
    
        if (param > 2147483647.0f){
            res = (_1u32)param;
        }
        else
        {
            __m128 p = _mm_load_ss(&param);
            res =  _mm_cvtss_si32(p);
        }
        _mm_setcsr(crm);
        return res;
    }


    // internal any() function. copy of OpenCL built in
    int _any4(__m128 x)
    {
	    int mask = _mm_movemask_epi8((__m128i)x);
	    return (mask & 0x8888) != 0;
    }
	
    CONVERSIONS_FUNC_DECL _4u32 float4ToUint4Round(float4 param, int rMode)
	{
        _4u32 res;
        const ALIGN16 float mth_INT_MAX_32f[4] = { 2147483647.0f, 2147483647.0f, 2147483647.0f, 2147483647.0f };

        int crm = _mm_getcsr();
        _mm_setcsr((crm& ~_MM_ROUND_MASK) | ROUND_MODE(rMode));

        __m128 mask_gt= _mm_cmpgt_ps(param,  *(__m128*)mth_INT_MAX_32f);
         
        if(_any4(mask_gt))
        {
            ALIGN16 float t[4];
            ALIGN16 int d[4];
            _mm_store_ps(t, param);
            d[0] = (_1u32)t[0];
            d[1] = (_1u32)t[1];
            d[2] = (_1u32)t[2];
            d[3] = (_1u32)t[3];
            res = _mm_load_si128((const __m128i*)d);
        }
        else
        {
            res =  _mm_cvtps_epi32(param);
        }

        _mm_setcsr(crm);
        return res;
    }

    
#if defined(__AVX__)
    // internal any() function. copy of OpenCL built in
    int _any8(__m256 x)
    {
        int mask = _mm256_movemask_ps((__m256)x);
        return (mask & 0xFF) != 0;
    }

    CONVERSIONS_FUNC_DECL __m256i float8ToUint8Round(__m256 param, int rMode)
    {
        int crm = _mm_getcsr();
        // ROUND_MODE is not needed here since Intel Compiler correctly obtains argument from stack
        _mm_setcsr((crm& ~_MM_ROUND_MASK) | rMode);

        __m256i res;
        const ALIGN32 float mth_INT_MAX_32f[8] = { 2147483647.0f, 2147483647.0f, 2147483647.0f, 2147483647.0f, 
                                                   2147483647.0f, 2147483647.0f, 2147483647.0f, 2147483647.0f};
        __m256 mask_gt= _mm256_cmp_ps(param,  *(__m256*)mth_INT_MAX_32f, _CMP_GT_OS );
         
        if(_any8(mask_gt))
        {
            ALIGN32 float t[8];
            ALIGN32 int d[8];
            _mm256_store_ps(t, param);
            d[0] = (_1u32)t[0];
            d[1] = (_1u32)t[1];
            d[2] = (_1u32)t[2];
            d[3] = (_1u32)t[3];
            d[4] = (_1u32)t[4];
            d[5] = (_1u32)t[5];
            d[6] = (_1u32)t[6];
            d[7] = (_1u32)t[7];
            res = _mm256_load_si256((const __m256i*)d);
        }
        else
        {
            res =   _mm256_cvtps_epi32(param);
        }

        _mm_setcsr(crm);
        return res;
    }
#endif // defined(__AVX__)

    CONVERSIONS_FUNC_DECL float longToFloat(_1i64 param, int rm)
	{
		unsigned int oldRound, tmp;
		if (param == 0) return 0.0f;
#if defined(_MSC_VER)
		_controlfp_s(&oldRound, _PC_64, _MCW_PC);
		_controlfp_s(&oldRound, rm, _MCW_RC);
		float res = (float)param;
		_controlfp_s(&tmp, oldRound, _MCW_RC);
#else
		float res = (float)param;
#endif
		return res;
	}

	/// !!! This function is copy-pasted to images module.
	/// In case of any changes they should also be applied to image_callback_functions.cpp
	CONVERSIONS_FUNC_DECL float4 float2half_rte(float4 param)
	{
		int rm = _MM_GET_ROUNDING_MODE();
		_MM_SET_ROUNDING_MODE(_MM_ROUND_NEAREST);

		//cl_uint sign = (u.u >> 16) & 0x8000;
		_8i16 temp = (_8i16)_mm_srli_epi32((__m128i)param, 0x10);
		_8i16 signs = (_8i16)_mm_and_si128((__m128i)temp,(__m128i) *((_4i32 *)x8000));
		float4 absParam = (float4)_mm_and_si128((__m128i)param,(__m128i) *((_4i32 *)x7fffffff));

		//Nan
		//if( x != x ) 
		_8i16 eq0 = (_8i16)_mm_cmpneq_ps(absParam, absParam);
		_8i16 eq = (_8i16)_mm_and_si128((__m128i)absParam,(__m128i) eq0);
		//u.u >>= (24-11);
		eq = (_8i16) _mm_srli_epi32((__m128i)eq, 0x0d);
		//u.u &= 0x7fff;
		eq = (_8i16) _mm_and_si128((__m128i)eq,(__m128i) *((_4i32 *)x7fff));
		//u.u |= 0x0200;   -- silence the NaN
		eq = (_8i16) _mm_or_si128((__m128i)eq,(__m128i) *((_4i32 *)x0200));
		//return u.u | sign;
		eq = (_8i16) _mm_or_si128((__m128i)eq,(__m128i) signs);
		eq = (_8i16) _mm_and_si128((__m128i)eq,(__m128i) eq0);
		_8i16 dflt = eq0;

		// overflow
		//if( x >= MAKE_HEX_FLOAT(0x1.ffcp15f, 0x1ffcL, 3) )
		//return 0x7c00 | sign;

		float4 eq1 = _mm_cmpge_ps(absParam, *((float4 *)x477ff000));
		eq0 = (_8i16) _mm_and_si128((__m128i)eq1,(__m128i) *((_4i32 *)x7c00));
		eq0 = (_8i16) _mm_or_si128((__m128i)signs,(__m128i) eq0);
		eq0 = (_8i16) _mm_andnot_si128((__m128i)dflt,(__m128i) eq0);
		eq = (_8i16) _mm_or_si128((__m128i)eq,(__m128i) eq0);
		dflt = (_8i16) _mm_or_si128((__m128i)eq1,(__m128i) dflt);

		// underflow
		//	if( x <= MAKE_HEX_FLOAT(0x1.0p-25f, 0x1L, -25) )
		// return sign
		eq1 = _mm_cmple_ps(absParam, *((float4 *)x33000000));
		eq0 = (_8i16) _mm_and_si128((__m128i)eq1,(__m128i) signs);
		eq0 = (_8i16) _mm_andnot_si128((__m128i)dflt,(__m128i) eq0);
		eq = (_8i16) _mm_or_si128((__m128i)eq,(__m128i) eq0);
		dflt = (_8i16) _mm_or_si128((__m128i)eq1,(__m128i) dflt);


		// very small
		//	if( x < MAKE_HEX_FLOAT(0x1.8p-24f, 0x18L, -28) )
		// return sign | 1;
		eq1 = _mm_cmplt_ps(absParam, *((float4 *)x33c00000));
		eq0 = (_8i16) _mm_and_si128((__m128i)eq1, _mm_or_si128((__m128i)signs,(__m128i) *((_4i32 *)ones)));
		eq0 = (_8i16) _mm_andnot_si128((__m128i)dflt,(__m128i) eq0);
		eq = (_8i16) _mm_or_si128((__m128i)eq,(__m128i) eq0);
		dflt = (_8i16) _mm_or_si128((__m128i)eq1,(__m128i) dflt);

		// half denormal         
		//  if( x < MAKE_HEX_FLOAT(0x1.0p-14f, 0x1L, -14) )
		//	x *= MAKE_HEX_FLOAT(0x1.0p-125f, 0x1L, -125);
		//  return sign | x;
		eq1 = _mm_cmplt_ps(absParam, *((float4 *)x38800000));
		float4 eq2 = _mm_mul_ps(absParam, *((float4 *)x01000000));  //x
		eq0 = (_8i16) _mm_and_si128((__m128i)eq1, _mm_or_si128((__m128i)signs,(__m128i) (_8i16)eq2));
		eq0 = (_8i16) _mm_andnot_si128((__m128i)dflt,(__m128i) eq0);
		eq = (_8i16) _mm_or_si128((__m128i)eq,(__m128i) eq0);
		dflt = (_8i16) _mm_or_si128((__m128i)eq1,(__m128i) dflt);

		// u.f *= MAKE_HEX_FLOAT(0x1.0p13f, 0x1L, 13);
		// u.u &= 0x7f800000;
		// x += u.f;
		// u.f = x - u.f;
		// u.f *= MAKE_HEX_FLOAT(0x1.0p-112f, 0x1L, -112);
		// return (u.u >> (24-11)) | sign;
		//int rm = _MM_GET_ROUNDING_MODE();
		//_MM_SET_ROUNDING_MODE(0x6000);
		
		eq1 = _mm_mul_ps(param, *((float4 *)x46000000));  
		eq0 = (_8i16) _mm_and_si128((__m128i)eq1,(__m128i) *((_4i32 *)x7f800000)); //u
		eq1 = _mm_add_ps( (float4)eq0 ,absParam); //x
		eq1 = _mm_sub_ps(eq1, (float4)eq0); //u
		eq1 = _mm_mul_ps(eq1, *((float4 *)x07800000));  
		eq0 = (_8i16) _mm_srli_epi32((__m128i)eq1, 0x0d);
		eq0 = (_8i16) _mm_or_si128((__m128i)eq0,(__m128i) signs);
		eq0 = (_8i16) _mm_andnot_si128((__m128i)dflt,(__m128i) eq0);
		eq = (_8i16) _mm_or_si128((__m128i)eq,(__m128i) eq0);

		eq1 = _mm_castsi128_ps(SHUFFLE_EPI8(eq, *((__m128i *)_4x32to4x16)));
		
		_MM_SET_ROUNDING_MODE(rm);

		return eq1;
	}

	CONVERSIONS_FUNC_DECL float4 float2half_rtz(float4 param)
	{
		int rm = _MM_GET_ROUNDING_MODE();
		_MM_SET_ROUNDING_MODE(_MM_ROUND_TOWARD_ZERO);

		//cl_uint sign = (u.u >> 16) & 0x8000;
		_8i16 temp = _mm_srli_epi32((_8i16)param, 0x10);
		_8i16 signs = _mm_and_si128(temp, *(_4i32*)x8000);
		param = (float4)_mm_and_si128((_4i32)param, *((_4i32*)x7fffffff));

		//Nan
		//if( x != x ) 
		_8i16 eq0 = (_8i16)_mm_cmpneq_ps(param, param);
		_8i16 eq = _mm_and_si128((_8i16)param, eq0);
		//u.u >>= (24-11);
		eq = _mm_srli_epi32(eq, 0x0d);
		//u.u &= 0x7fff;
		eq = _mm_and_si128(eq, *((_4i32*)x7fff));
		//u.u |= 0x0200;   -- silence the NaN
		eq = _mm_or_si128(eq, *((_4i32*)x0200));
		//return u.u | sign;
		eq = _mm_or_si128(eq, signs);
		eq = _mm_and_si128(eq, eq0);
		_8i16 dflt = eq0;

		// overflow
		//if( x >= MAKE_HEX_FLOAT(0x1.0p16f, 0x1L, 16) )

		float4 eq1 = _mm_cmpge_ps(param, *((float4*)x47800000));

		//if( x == INFINITY )
		//return 0x7c00 | sign;
		eq0 = _mm_cmpeq_epi32((_8i16)param, *((_4i32*)x7f800000));
		eq0 = _mm_and_si128(eq0, (_8i16)eq1);
		_8i16 eq2 = _mm_and_si128(eq0, *((_4i32*)x7c00));
		eq2 = _mm_or_si128(eq2, signs);
		eq2 = _mm_and_si128(eq0, eq2);
		eq2 = _mm_andnot_si128(dflt, eq2);
		eq = _mm_or_si128(eq, eq2);
		dflt = _mm_or_si128(eq0, dflt);


		//else return 0x7bff | sign;
		eq0 = _mm_xor_si128((_8i16)eq1, eq0);
		eq2 = _mm_and_si128(eq0, *((_4i32*)x7bff));
		eq2 = _mm_or_si128(eq2, signs);
		eq2 = _mm_and_si128(eq2, eq0);
		eq2 = _mm_andnot_si128(dflt, eq2);
		eq = _mm_or_si128(eq, eq2);
		dflt = _mm_or_si128(eq0, dflt);


		// underflow
		//	if( x < MAKE_HEX_FLOAT(0x1.0p-24f, 0x1L, -24) )
		//  return sign;    -- The halfway case can return 0x0001 or 0. 0 is even.
		eq1 = _mm_cmplt_ps(param, *((float4*)x33800000));
		eq0 = _mm_and_si128((_8i16)eq1, signs);
		eq0 = _mm_andnot_si128(dflt, eq0);
		eq = _mm_or_si128(eq, eq0);
		dflt = _mm_or_si128((_8i16)eq1, dflt);


		// half denormal
		//  if( x < MAKE_HEX_FLOAT(0x1.0p-14f, 0x1L, -14) )
		//	x *= MAKE_HEX_FLOAT(0x1.0p24f, 0x1L, 24);
		//  return (short)( (int)x | sign);
		eq1 = _mm_cmplt_ps(param, *((float4*)x38800000));
		eq2 = (_8i16)eq1;
		eq1 = _mm_and_ps(eq1, param);
		eq1 = _mm_mul_ps(eq1, *((float4*)x4b800000));
		eq0 = _mm_cvtps_epi32(eq1);
		eq0 = _mm_or_si128(eq0, signs);
		eq0 = _mm_and_si128(eq2, eq0);
		eq0 = _mm_andnot_si128(dflt, eq0);
		eq  = _mm_or_si128(eq, eq0); 
		dflt = _mm_or_si128(eq2, dflt);


		//u.u &= 0xFFFFE000U;
		eq0 = _mm_and_si128((_8i16)param, *((_4i32*)xffffe000));
		//u.u -= 0x38000000U;
		eq0 = _mm_sub_epi32(eq0, *((_4i32*)x38000000));
		eq0 = _mm_srli_epi32(eq0, 13);
		eq0 = _mm_or_si128(eq0, signs);
		eq0 = _mm_andnot_si128(dflt, eq0);
		eq  = _mm_or_si128(eq, eq0);

		eq1 = _mm_castsi128_ps(SHUFFLE_EPI8(eq, *((__m128i *)_4x32to4x16)));

		_MM_SET_ROUNDING_MODE(rm);

		return eq1;
	}

	CONVERSIONS_FUNC_DECL float4 float2half_rtp(float4 param)
	{
		int rm = _MM_GET_ROUNDING_MODE();
		_MM_SET_ROUNDING_MODE(_MM_ROUND_TOWARD_ZERO);

		float4 zeros = _mm_setzero_ps();

		//cl_uint sign = (u.u >> 16) & 0x8000;
		_8i16 temp = _mm_srli_epi32((_8i16)param, 0x10);
		_8i16 signs = _mm_and_si128(temp, *((_4i32 *)x8000));
		float4 absParam = (float4)_mm_and_si128((_8i16)param, *((_4i32 *)x7fffffff));

		//Nan
		//if( x != x ) 
		_8i16 eq0 = (_8i16)_mm_cmpneq_ps(absParam, absParam);
		_8i16 eq = _mm_and_si128((_4i32 )absParam, eq0);
		//u.u >>= (24-11);
		eq = _mm_srli_epi32(eq, 0x0d);
		//u.u &= 0x7fff;
		eq = _mm_and_si128(eq, *((_4i32 *)x7fff));
		//u.u |= 0x0200;   -- silence the NaN
		eq = _mm_or_si128(eq, *((_4i32 *)x0200));
		//return u.u | sign;
		eq = _mm_or_si128(eq, signs);
		eq = _mm_and_si128(eq, eq0);
		_8i16 dflt = eq0;

		// overflow
		//if( f > MAKE_HEX_FLOAT(0x1.ffcp15f, 0x1ffcL, 3) )
		//return 0x7c00;

		float4 eq1 = _mm_cmpgt_ps(param, *((float4 *)x477ff000));
		eq0 = _mm_and_si128((_8i16)eq1, *((_4i32 *)x7c00));
		eq0 = _mm_andnot_si128(dflt, eq0);
		eq = _mm_or_si128(eq, eq0);
		dflt = _mm_or_si128((_8i16)eq1, dflt);

		//	if( f <= MAKE_HEX_FLOAT(-0x1.0p16f, -0x1L, 16) )
		eq1 = _mm_cmple_ps(param, *((float4 *)xc7800000));

		//if( f == -INFINITY )
		//return 0xfc00;
		eq0 = _mm_cmpeq_epi32((_8i16)param, *((_4i32 *)xff800000));
		eq0 = _mm_and_si128(eq0, (_8i16)eq1);
		_8i16 eq2 = _mm_and_si128(eq0, *((_4i32 *)xfc00));
		eq2 = _mm_andnot_si128(dflt, eq2);
		eq = _mm_or_si128(eq, eq2);
		dflt = _mm_or_si128(eq0, dflt);

		//else return 0xfbff;
		eq0 = _mm_xor_si128((_8i16)eq1, eq0);
		eq2 = _mm_and_si128(eq0, *((_4i32 *)xfbff));
		eq2 = _mm_andnot_si128(dflt, eq2);
		eq = _mm_or_si128(eq, eq2);
		dflt = _mm_or_si128(eq0, dflt);

		// underflow
		//	if( x < MAKE_HEX_FLOAT(0x1.0p-24f, 0x1L, -24) )
		eq1 = _mm_cmplt_ps(absParam, *((float4 *)x33800000));

		// if (f > 0) return 1;
		eq0 = (_8i16)_mm_cmpgt_ps(param, zeros);
		eq0 = _mm_and_si128(eq0, (_8i16)eq1);
		eq2 = _mm_and_si128(eq0, *((_4i32 *)ones));
		eq2 = _mm_andnot_si128(dflt, eq2);
		eq = _mm_or_si128(eq, eq2);
		dflt = _mm_or_si128(eq0, dflt);
		// else return sign
		eq0 = _mm_xor_si128((_8i16)eq1, eq0);
		eq2 = _mm_and_si128(eq0, signs);
		eq2 = _mm_andnot_si128(dflt, eq2);
		eq = _mm_or_si128(eq, eq2);
		dflt = _mm_or_si128(eq0, dflt);

		// half denormal         
		//  if( x < MAKE_HEX_FLOAT(0x1.0p-14f, 0x1L, -14) )
		//	x *= MAKE_HEX_FLOAT(0x1.0p24f, 0x1L, 24);
		//  int r = (int)x;
		eq1 = _mm_cmplt_ps(absParam, *((float4 *)x38800000));
		eq2 = (_8i16)eq1;
		eq1 = _mm_and_ps(eq1, absParam);
		eq1 = _mm_mul_ps(eq1, *((float4 *)x4b800000));  //x
		eq0 = _mm_cvtps_epi32(eq1); //r
		// r += (float)r != x && f > 0.0f;
		float4 eq3 = _mm_cvtepi32_ps(eq0); //(float)r
		eq1 = _mm_cmpneq_ps(eq1, eq3); // (float)r != x
		eq3 = _mm_cmpgt_ps(param, zeros); //f > 0.0f
		eq1 = _mm_and_ps(eq1, eq3); //(float)r != x && f > 0.0f
		_8i16 eq4 = _mm_and_si128((_8i16)eq1, *((_4i32 *)ones));
		eq0 = _mm_add_epi32(eq0, eq4);
		// return (short)(r | sign)
		eq0 = _mm_or_si128(eq0, signs); 
		eq0 = _mm_and_si128(eq2, eq0);
		eq0 = _mm_andnot_si128(dflt, eq0);
		eq  = _mm_or_si128(eq, eq0); 
		dflt = _mm_or_si128(eq2, dflt);

		//u.u &= 0xFFFFE000U;
		eq0 = _mm_and_si128((_8i16)param, *((_4i32 *)xffffe000));
		//if (f > u.f)
		//u.u += 0x00002000U;
		eq1 = _mm_cmpgt_ps(param, (float4)eq0);
		eq2 = _mm_add_epi32(eq0, _mm_and_si128((_8i16)eq1, *((_4i32 *)x00002000)));
		//u.u -= 0x38000000U;
		//return ((u.u >> 13) | sign);
		eq2 = _mm_sub_epi32(eq2, *((_4i32 *)x38000000));
		eq2 = _mm_srli_epi32(eq2, 13);
		eq2 = _mm_or_si128(eq2, signs);
		eq2 = _mm_andnot_si128(dflt, eq2);
		eq  = _mm_or_si128(eq, eq2); 

		eq1 = _mm_castsi128_ps(SHUFFLE_EPI8(eq, *((__m128i *)_4x32to4x16)));

		_MM_SET_ROUNDING_MODE(rm);

		return eq1;
	}

	CONVERSIONS_FUNC_DECL float4 float2half_rtn(float4 param)
	{
		int rm = _MM_GET_ROUNDING_MODE();
		_MM_SET_ROUNDING_MODE(_MM_ROUND_TOWARD_ZERO);

		float4 zeros = _mm_setzero_ps();

		//cl_uint sign = (u.u >> 16) & 0x8000;
		_8i16 temp = _mm_srli_epi32((_8i16)param, 0x10);
		_8i16 signs = _mm_and_si128(temp, *((__m128i*)x8000));
		float4 absParam = (float4)_mm_and_si128((_8i16)param, *((_4i32 *)x7fffffff));

		//Nan
		//if( x != x ) 
		_8i16 eq0 = (_8i16)_mm_cmpneq_ps(absParam, absParam);
		_8i16 eq = _mm_and_si128((_8i16)absParam, eq0);
		//u.u >>= (24-11);
		eq = _mm_srli_epi32(eq, 0x0d);
		//u.u &= 0x7fff;
		eq = _mm_and_si128(eq, *((_4i32 *)x7fff));
		//u.u |= 0x0200;   -- silence the NaN
		eq = _mm_or_si128(eq, *((_4i32 *)x0200));
		//return u.u | sign;
		eq = _mm_or_si128(eq, signs);
		eq = _mm_and_si128(eq, eq0);
		_8i16 dflt = eq0;


		// overflow
		//if( f >= MAKE_HEX_FLOAT(0x1.0p16f, 0x1L, 16) )
		float4 eq1 = _mm_cmpge_ps(param, *((float4 *)x47800000));

		//if( f == INFINITY )
		//return 0x7c00;
		eq0 = _mm_cmpeq_epi32((_8i16)param, *((_4i32 *)x7f800000));
		eq0 = _mm_and_si128(eq0, (_8i16)eq1);
		_8i16 eq2 = _mm_and_si128(eq0, *((_4i32 *)x7c00));
		eq2 = _mm_andnot_si128(dflt, eq2);
		eq = _mm_or_si128(eq, eq2);
		dflt = _mm_or_si128(eq0, dflt);

		//else return 0x7bff;
		eq0 = _mm_xor_si128((_8i16)eq1, eq0);
		eq2 = _mm_and_si128(eq0, *((_4i32 *)x7bff));
		eq2 = _mm_andnot_si128(dflt, eq2);
		eq = _mm_or_si128(eq, eq2);
		dflt = _mm_or_si128(eq0, dflt);

		//if( f < MAKE_HEX_FLOAT(-0x1.ffcp15f, -0x1ffcL, 3) )
		//return 0xfc00;
		eq1 = _mm_cmplt_ps(param, *((float4 *)xc77fe000));
		eq0 = _mm_and_si128((_8i16)eq1, *((_4i32 *)xfc00));
		eq0 = _mm_andnot_si128(dflt, eq0);
		eq = _mm_or_si128(eq, eq0);
		dflt = _mm_or_si128((_8i16)eq1, dflt);

		// underflow
		//	if( x < MAKE_HEX_FLOAT(0x1.0p-24f, 0x1L, -24) )
		eq1 = _mm_cmple_ps(absParam, *((float4 *)x33800000));

		// if (f < 0) return 0x8001;
		eq0 = (_8i16)_mm_cmplt_ps(param, zeros);
		eq0 = _mm_and_si128(eq0, (_8i16)eq1);
		eq2 = _mm_and_si128(eq0, *((_4i32 *)x8001));
		eq2 = _mm_andnot_si128(dflt, eq2);
		eq = _mm_or_si128(eq, eq2);
		dflt = _mm_or_si128(eq0, dflt);
		// else return sign
		eq0 = _mm_xor_si128((_8i16)eq1, eq0);
		eq2 = _mm_and_si128(eq0, signs);
		eq2 = _mm_andnot_si128(dflt, eq2);
		eq = _mm_or_si128(eq, eq2);
		dflt = _mm_or_si128(eq0, dflt);

		// half denormal         
		//  if( x < MAKE_HEX_FLOAT(0x1.0p-14f, 0x1L, -14) )
		//	x *= MAKE_HEX_FLOAT(0x1.0p24f, 0x1L, 24);
		//  int r = (int)x;
		eq1 = _mm_cmplt_ps(absParam, *((float4 *)x38800000));
		eq2 = (_8i16)eq1;
		eq1 = _mm_and_ps(eq1, absParam);
		eq1 = _mm_mul_ps(eq1, *((float4 *)x4b800000));  //x
		eq0 = _mm_cvtps_epi32(eq1); //r

		// r += (float)r != x && f < 0.0f;
		float4 eq3 = _mm_cvtepi32_ps(eq0); //(float)r
		eq1 = _mm_cmpneq_ps(eq1, eq3); // (float)r != x
		eq3 = _mm_cmplt_ps(param, zeros); //f < 0.0f
		eq1 = _mm_and_ps(eq1, eq3); //(float)r != x && f < 0.0f
		_8i16 eq4 = _mm_and_si128((_8i16)eq1, *((_4i32 *)ones));
		eq0 = _mm_add_epi32(eq0, eq4);
		// return (short)(r | sign)
		eq0 = _mm_or_si128(eq0, signs); 
		eq0 = _mm_and_si128(eq2, eq0);
		eq0 = _mm_andnot_si128(dflt, eq0);
		eq  = _mm_or_si128(eq, eq0); 
		dflt = _mm_or_si128(eq2, dflt);

		//u.u &= 0xFFFFE000U;
		eq0 = _mm_and_si128((_8i16)param, *((_4i32 *)xffffe000));
		//if (u.f > f)
		//u.u += 0x00002000U;
		eq1 = _mm_cmpgt_ps((float4)eq0, param);
		eq2 = _mm_add_epi32(eq0, _mm_and_si128((_8i16)eq1, *((_4i32 *)x00002000)));
		//u.u -= 0x38000000U;
		//return ((u.u >> 13) | sign);
		eq2 = _mm_sub_epi32(eq2, *((_4i32 *)x38000000));
		eq2 = _mm_srli_epi32(eq2, 13);
		eq2 = _mm_or_si128(eq2, signs);
		eq2 = _mm_andnot_si128(dflt, eq2);
		eq  = _mm_or_si128(eq, eq2); 

		eq1 = _mm_castsi128_ps(SHUFFLE_EPI8(eq, *((__m128i *)_4x32to4x16)));

		_MM_SET_ROUNDING_MODE(rm);
		return eq1;
	}

	CONVERSIONS_FUNC_DECL float4 double2ToHalf2_rte(double2 param)
	{
		int rm = _MM_GET_ROUNDING_MODE();
		_MM_SET_ROUNDING_MODE(_MM_ROUND_NEAREST);

		//cl_ulong sign = (u.u >> 48) & 0x8000;
		//double x = fabs(f);
		__m128i temp = _mm_srli_epi64(_mm_castpd_si128(param), 48);
		__m128i signs = _mm_and_si128(temp, *((__m128i *)x8000));
		__m128d absParam = _mm_and_pd(param, *((__m128d *)x7fffffffffffffff));

		//Nan
		//if( x != x ) 
		//	u.u >>= (53-11);
		//	u.u &= 0x7fff;
		//	u.u |= 0x0200;   -- silence the NaN
		//	return u.u | sign;

		__m128i eq0 = _mm_castpd_si128( _mm_cmpneq_pd(absParam, absParam) );
		__m128i eq = _mm_and_si128(_mm_castpd_si128( absParam ), eq0);
		eq = _mm_srli_epi64(eq, 42);
		eq = _mm_and_si128(eq, *((__m128i *)x7fff));
		eq = _mm_or_si128(eq, *((__m128i *)x0200));
		eq = _mm_or_si128(eq, signs);
		eq = _mm_and_si128(eq, eq0);
		__m128i dflt = eq0;

		//// overflow
		////if( x >= MAKE_HEX_DOUBLE(0x1.ffep15, 0x1ffeL, 3) )
		////         0x40effe0000000000
		////return 0x7c00 | sign;

		__m128i eq1 = _mm_castpd_si128( _mm_cmpge_pd(absParam, *((__m128d *)x40effe0000000000)) );
		eq0 = _mm_and_si128(eq1, *((__m128i *)x7c00));
		eq0 = _mm_or_si128(signs, eq0);
		eq0 = _mm_andnot_si128(dflt, eq0);
		eq = _mm_or_si128(eq, eq0);
		dflt = _mm_or_si128(eq1, dflt);

		//// underflow
		////	if( x <= MAKE_HEX_DOUBLE(0x1.0p-25, 0x1L, -25) )
		////			 0x3e60000000000000
		//// return sign
		eq1 = _mm_castpd_si128( _mm_cmple_pd(absParam, *((__m128d *)x3e60000000000000)) );
		eq0 = _mm_and_si128(eq1, signs);
		eq0 = _mm_andnot_si128(dflt, eq0);
		eq = _mm_or_si128(eq, eq0);
		dflt = _mm_or_si128(eq1, dflt);


		//// very small
		////	if( x < MAKE_HEX_DOUBLE(0x1.8p-24, 0x18L, -28) )
		////			0x3e78000000000000
		//// return sign | 1;
		eq1 = _mm_castpd_si128( _mm_cmplt_pd(absParam, *((__m128d *)x3e78000000000000)) );
		eq0 = _mm_and_si128(eq1, _mm_or_si128(signs, *((__m128i *)dones)));
		eq0 = _mm_andnot_si128(dflt, eq0);
		eq = _mm_or_si128( eq, eq0);
		dflt = _mm_or_si128(eq1, dflt);

		//// half denormal         
		////  if( x < MAKE_HEX_DOUBLE(0x1.0p-14, 0x1L, -14) )
		////			0x3f10000000000000
		////	u.f = x * MAKE_HEX_DOUBLE(0x1.0p-1050, 0x1L, -1050);
		////			  0x0000000001000000
		////  return sign | x;
		eq1 = _mm_castpd_si128( _mm_cmplt_pd(absParam, *((__m128d *)x3f10000000000000)) );
		__m128i eq2 = _mm_castpd_si128( _mm_mul_pd(absParam, *((__m128d *)x0000000001000000)) );  //x
		eq0 = _mm_and_si128(eq1, _mm_or_si128(signs, eq2));
		eq0 = _mm_andnot_si128(dflt, eq0);
		eq = _mm_or_si128(eq, eq0);
		dflt = _mm_or_si128(eq1, dflt);

		//// u.f *= MAKE_HEX_DOUBLE(0x1.0p42, 0x1L, 42);
		////		  0x4290000000000000
		//// u.u &= 0x7ff0000000000000UL;
		//// x += u.f;
		//// u.f = x - u.f;
		//// u.f *= MAKE_HEX_DOUBLE(0x1.0p-1008, 0x1L, -1008);
		////		  0x00f0000000000000
		//// return (u.u >> (53-11)) | sign;
		//
		__m128d res = _mm_mul_pd(param, *((__m128d *)x4290000000000000));  
		__m128d tmp = _mm_and_pd(res, *((__m128d *)x7ff0000000000000)); //u
		res = _mm_add_pd( tmp ,absParam); //x
		res = _mm_sub_pd( res, tmp); //u
		res = _mm_mul_pd(res, *((__m128d *)x00f0000000000000));  
		res = _mm_castsi128_pd(_mm_srli_epi64(_mm_castpd_si128(res), 42));
		res = _mm_or_pd(res, _mm_castsi128_pd(signs));
		res = _mm_andnot_pd(_mm_castsi128_pd(dflt), res);
		eq = _mm_or_si128(eq, _mm_castpd_si128(res));

		eq1 = SHUFFLE_EPI8(eq, *((__m128i *)_2x64to2x16));

		_MM_SET_ROUNDING_MODE(rm);

		return _mm_castsi128_ps(eq1);
	}

	CONVERSIONS_FUNC_DECL float4 double2ToHalf2_rtz(double2 param)
	{
		int rm = _MM_GET_ROUNDING_MODE();
		_MM_SET_ROUNDING_MODE(_MM_ROUND_TOWARD_ZERO);

		//cl_ulong sign = (u.u >> 48) & 0x8000;
		//double x = fabs(f);
		__m128i temp = _mm_srli_epi64(_mm_castpd_si128(param), 48);
		__m128i signs = _mm_and_si128(temp, *((__m128i *)x8000));
		__m128d absParam = _mm_and_pd(param, *((__m128d *)x7fffffffffffffff));

		//Nan
		//if( x != x ) 
		//	u.u >>= (53-11);
		//	u.u &= 0x7fff;
		//	u.u |= 0x0200;   -- silence the NaN
		//	return u.u | sign;
		__m128i eq0 = _mm_castpd_si128( _mm_cmpneq_pd(absParam, absParam) );
		__m128i eq = _mm_and_si128(_mm_castpd_si128( absParam ), eq0);
		eq = _mm_srli_epi64(eq, 42);
		eq = _mm_and_si128(eq, *((__m128i *)x7fff));
		eq = _mm_or_si128(eq, *((__m128i *)x0200));
		eq = _mm_or_si128(eq, signs);
		eq = _mm_and_si128(eq, eq0);
		__m128i dflt = eq0;

		//if( x == INFINITY )
		//return 0x7c00 | sign;
		eq0 = _mm_castpd_si128( _mm_cmpeq_pd(absParam, *((__m128d*)x7ff0000000000000)) );
		__m128i eq1 = _mm_and_si128(eq0, *((__m128i*)x7c00));
		eq1 = _mm_or_si128(eq1, signs);
		eq1 = _mm_and_si128(eq0, eq1);
		eq1 = _mm_andnot_si128(dflt, eq1);
		eq = _mm_or_si128(eq, eq1);
		dflt = _mm_or_si128(eq0, dflt);

		// overflow
		//if( x >= MAKE_HEX_DOUBLE(0x1.0p16, 0x1L, 16) )
		//    return 0x7bff | sign;
		eq0 = _mm_castpd_si128( _mm_cmpge_pd(absParam, *((__m128d*)x40f0000000000000)) );
		eq1 = _mm_and_si128(eq0, *((__m128i*)x7bff));
		eq1 = _mm_or_si128(eq1, signs);
		eq1 = _mm_and_si128(eq0, eq1);
		eq1 = _mm_andnot_si128(dflt, eq1);
		eq = _mm_or_si128(eq, eq1);
		dflt = _mm_or_si128(eq0, dflt);

		// underflow
		//if( x < MAKE_HEX_DOUBLE(0x1.0p-24, 0x1L, -24) )
		//    return sign;    // The halfway case can return 0x0001 or 0. 0 is even.
		eq1 = _mm_castpd_si128( _mm_cmplt_pd(absParam, *((__m128d*)x3e70000000000000)) );
		eq0 = _mm_and_si128(eq1, signs);
		eq0 = _mm_andnot_si128(dflt, eq0);
		eq = _mm_or_si128(eq, eq0);
		dflt = _mm_or_si128(eq1, dflt);

		// half denormal
		//if( x < MAKE_HEX_DOUBLE(0x1.0p-14, 0x1L, -14) )
		//    x *= MAKE_HEX_FLOAT(0x1.0p24f, 0x1L, 24);
		//    return (cl_ushort)((int) x | sign);
		eq0 = _mm_castpd_si128( _mm_cmplt_pd(absParam, *((__m128d*)x3f10000000000000)) );
		eq1 = _mm_and_si128(eq0, _mm_castpd_si128( absParam ));
		eq1 = _mm_castpd_si128( _mm_mul_pd(_mm_castsi128_pd( eq1 ), *((__m128d*)x4170000000000000)) );
		eq1 = _mm_cvtpd_epi32(_mm_castsi128_pd( eq1 ));
		eq1 = _mm_unpacklo_epi32(eq1, _mm_setzero_si128());
		eq1 = _mm_or_si128(eq1, signs);
		eq1 = _mm_and_si128(eq1, eq0);
		eq1 = _mm_andnot_si128(dflt, eq1);
		eq  = _mm_or_si128(eq, eq1); 
		dflt = _mm_or_si128(eq0, dflt);

		//u.u &= 0xFFFFFC0000000000UL;
		eq0 = _mm_and_si128(_mm_castpd_si128( param ), *((__m128i*)xFFFFFC0000000000));
		//u.u -= 0x3F00000000000000UL;
		eq0 = _mm_sub_epi64(eq0, *((__m128i*)x3F00000000000000));
		//return (u.u >> (53-11)) | sign;
		eq0 = _mm_srli_epi64(eq0, 42);
		eq0 = _mm_or_si128(eq0, signs);
		eq0 = _mm_andnot_si128(dflt, eq0);
		eq  = _mm_or_si128(eq, eq0);
	    
		eq1 = SHUFFLE_EPI8(eq, *((__m128i *)_2x64to2x16));

		_MM_SET_ROUNDING_MODE(rm);

		return _mm_castsi128_ps(eq1);
	}

	CONVERSIONS_FUNC_DECL float4 double2ToHalf2_rtp(double2 param)
	{
		int rm = _MM_GET_ROUNDING_MODE();
		_MM_SET_ROUNDING_MODE(_MM_ROUND_TOWARD_ZERO);

		__m128d zeros = _mm_setzero_pd();

		//cl_ulong sign = (u.u >> 48) & 0x8000;
		//double x = fabs(f);
		__m128i temp = _mm_srli_epi64(_mm_castpd_si128(param), 48);
		__m128i signs = _mm_and_si128(temp, *((__m128i *)x8000));
		__m128d absParam = _mm_and_pd(param, *((__m128d *)x7fffffffffffffff));

		//Nan
		//if( x != x ) 
		//	u.u >>= (53-11);
		//	u.u &= 0x7fff;
		//	u.u |= 0x0200;   -- silence the NaN
		//	return u.u | sign;

		__m128i eq0 = _mm_castpd_si128( _mm_cmpneq_pd(absParam, absParam) );
		__m128i eq = _mm_and_si128(_mm_castpd_si128( absParam ), eq0);
		eq = _mm_srli_epi64(eq, 42);
		eq = _mm_and_si128(eq, *((__m128i *)x7fff));
		eq = _mm_or_si128(eq, *((__m128i *)x0200));
		eq = _mm_or_si128(eq, signs);
		eq = _mm_and_si128(eq, eq0);
		__m128i dflt = eq0;

		// overflow
		//if( f > MAKE_HEX_DOUBLE(0x1.ffcp15, 0x1ffcL, 3) )
		//		  0x40effc0000000000
		//	return 0x7c00;
		__m128i eq1 = _mm_castpd_si128( _mm_cmpgt_pd(param, *((__m128d *)x40effc0000000000)) );
		eq0 = _mm_and_si128(eq1, *((__m128i *)x7c00));
		eq0 = _mm_andnot_si128(dflt, eq0);
		eq = _mm_or_si128(eq, eq0);
		dflt = _mm_or_si128(eq1, dflt);

		//if( f <= MAKE_HEX_DOUBLE(-0x1.0p16, -0x1L, 16) )
		//		   0xc0f0000000000000
		eq1 = _mm_castpd_si128( _mm_cmple_pd(param, *((__m128d *)xc0f0000000000000)) );

		//if( f == -INFINITY )
		//return 0xfc00;
		eq0 = _mm_castpd_si128( _mm_cmpeq_pd(param, *((__m128d *)xfff0000000000000)) );
		eq0 = _mm_and_si128(eq0, eq1);
		__m128i eq2 = _mm_and_si128(eq0, *((__m128i *)xfc00));
		eq2 = _mm_andnot_si128(dflt, eq2);
		eq = _mm_or_si128(eq, eq2);
		dflt = _mm_or_si128(eq0, dflt);

		//else return 0xfbff;
		eq0 = _mm_xor_si128(eq1, eq0);
		eq2 = _mm_and_si128(eq0, *((__m128i *)xfbff));
		eq2 = _mm_andnot_si128(dflt, eq2);
		eq = _mm_or_si128(eq, eq2);
		dflt = _mm_or_si128(eq0, dflt);


		// underflow
		//	if( x < MAKE_HEX_DOUBLE(0x1.0p-24, 0x1L, -24) )
		//			0x3e70000000000000
		eq1 = _mm_castpd_si128( _mm_cmplt_pd(absParam, *((__m128d *)x3e70000000000000)) );

		// if (f > 0) return 1;
		eq0 = _mm_castpd_si128( _mm_cmpgt_pd(param, zeros) );
		eq0 = _mm_and_si128(eq0, eq1);
		eq2 = _mm_and_si128(eq0, *((__m128i *)dones));
		eq2 = _mm_andnot_si128(dflt, eq2);
		eq = _mm_or_si128(eq, eq2);
		dflt = _mm_or_si128(eq0, dflt);
		// else return sign
		eq0 = _mm_xor_si128(eq1, eq0);
		eq2 = _mm_and_si128(eq0, signs);
		eq2 = _mm_andnot_si128(dflt, eq2);
		eq = _mm_or_si128(eq, eq2);
		dflt = _mm_or_si128(eq0, dflt);

		// half denormal         
		//  if( x < MAKE_HEX_DOUBLE(0x1.0p-14, 0x1L, -14) )
		//			0x3f10000000000000
		//		x *= MAKE_HEX_DOUBLE(0x1.0p24, 0x1L, 24);
		//			 0x4170000000000000
		//		int r = (int)x;
		eq1 = _mm_castpd_si128( _mm_cmplt_pd(absParam, *((__m128d *)x3f10000000000000)) );
		eq2 = eq1;
		eq1 = _mm_and_si128(eq1, _mm_castpd_si128(absParam));
		eq1 = _mm_castpd_si128( _mm_mul_pd(_mm_castsi128_pd(eq1), *((__m128d *)x4170000000000000)) );  //x
		eq0 = _mm_cvtpd_epi32( _mm_castsi128_pd(eq1) ); //r

		// if( sign )
		//     r += (double) r != x;
		__m128d eq3 = _mm_cvtepi32_pd(eq0); //(double)r
		eq1 = _mm_castpd_si128( _mm_cmpneq_pd( _mm_castsi128_pd( eq1 ), eq3) ); // (double)r != x
		eq3 = _mm_cmpgt_pd(param, zeros); //f > 0.0f
		eq1 = _mm_and_si128(eq1, _mm_castpd_si128(eq3)); //(double)r != x && f < 0.0f
		__m128i eq4 = _mm_and_si128(eq1, *((__m128i *)dones));
		eq0 = _mm_unpacklo_epi32(eq0, _mm_setzero_si128());
		eq0 = _mm_add_epi64(eq0, eq4);

		// return (short)(r | sign)
		eq0 = _mm_or_si128(eq0, signs); 
		eq0 = _mm_and_si128(eq2, eq0);
		eq0 = _mm_andnot_si128(dflt, eq0);
		eq  = _mm_or_si128(eq, eq0); 
		dflt = _mm_or_si128(eq2, dflt);

		//u.u &= 0xFFFFFC0000000000UL;
		eq0 = _mm_and_si128(_mm_castpd_si128(param), *((__m128i *)xFFFFFC0000000000));
		//if (u.f < f)
		//u.u += 0x0000040000000000UL;
		eq1 = _mm_castpd_si128( _mm_cmpgt_pd(param, _mm_castsi128_pd( eq0 )) );
		eq2 = _mm_add_epi64(eq0, _mm_and_si128(eq1, *((__m128i *)x0000040000000000)));
		//u.u -= 0x3F00000000000000UL;
		//return ((u.u >> 42) | sign);
		eq2 = _mm_sub_epi64(eq2, *((__m128i *)x3F00000000000000));
		eq2 = _mm_srli_epi64(eq2, 42);
		eq2 = _mm_or_si128(eq2, signs);
		eq2 = _mm_andnot_si128(dflt, eq2);
		eq  = _mm_or_si128(eq, eq2); 

		eq1 = SHUFFLE_EPI8(eq, *((__m128i *)_2x64to2x16));

		_MM_SET_ROUNDING_MODE(rm);

		return _mm_castsi128_ps(eq1);
	}

	CONVERSIONS_FUNC_DECL float4 double2ToHalf2_rtn(double2 param)
	{
		int rm = _MM_GET_ROUNDING_MODE();
		_MM_SET_ROUNDING_MODE(_MM_ROUND_TOWARD_ZERO);

		__m128d zeros = _mm_setzero_pd();

		//cl_ulong sign = (u.u >> 48) & 0x8000;
		//double x = fabs(f);
		__m128i temp = _mm_srli_epi64(_mm_castpd_si128(param), 48);
		__m128i signs = _mm_and_si128(temp, *((__m128i *)x8000));
		__m128d absParam = _mm_and_pd(param, *((__m128d *)x7fffffffffffffff));

		//Nan
		//if( x != x ) 
		//	u.u >>= (53-11);
		//	u.u &= 0x7fff;
		//	u.u |= 0x0200;   -- silence the NaN
		//	return u.u | sign;

		__m128i eq0 = _mm_castpd_si128( _mm_cmpneq_pd(absParam, absParam) );
		__m128i eq = _mm_and_si128(_mm_castpd_si128( absParam ), eq0);
		eq = _mm_srli_epi64(eq, 42);
		eq = _mm_and_si128(eq, *((__m128i *)x7fff));
		eq = _mm_or_si128(eq, *((__m128i *)x0200));
		eq = _mm_or_si128(eq, signs);
		eq = _mm_and_si128(eq, eq0);
		__m128i dflt = eq0;

		
		// overflow
		//if( f >= MAKE_HEX_DOUBLE(0x1.0p16, 0x1L, 16) )
		//         0x40f0000000000000
		__m128i eq1 = _mm_castpd_si128( _mm_cmpge_pd(param, *((__m128d *)x40f0000000000000)) );

		//if( f == INFINITY )
		//return 0x7c00;
		eq0 = _mm_castpd_si128( _mm_cmpeq_pd(param, *((__m128d *)x7ff0000000000000)) );
		eq0 = _mm_and_si128(eq0, eq1);
		__m128i eq2 = _mm_and_si128(eq0, *((__m128i *)x7c00));
		eq2 = _mm_andnot_si128(dflt, eq2);
		eq = _mm_or_si128(eq, eq2);
		dflt = _mm_or_si128(eq0, dflt);

		//else return 0x7bff;
		eq0 = _mm_xor_si128(eq1, eq0);
		eq2 = _mm_and_si128(eq0, *((__m128i *)x7bff));
		eq2 = _mm_andnot_si128(dflt, eq2);
		eq = _mm_or_si128(eq, eq2);
		dflt = _mm_or_si128(eq0, dflt);

		//if( f < MAKE_HEX_DOUBLE(-0x1.ffcp15, -0x1ffcL, 3) )
		//        0xc0effc0000000000
		//return 0xfc00;
		eq1 = _mm_castpd_si128( _mm_cmplt_pd(param, *((__m128d *)xc0effc0000000000)) );
		eq0 = _mm_and_si128(eq1, *((__m128i *)xfc00));
		eq0 = _mm_andnot_si128(dflt, eq0);
		eq = _mm_or_si128(eq, eq0);
		dflt = _mm_or_si128(eq1, dflt);

		// underflow
		//	if( x < MAKE_HEX_DOUBLE(0x1.0p-24, 0x1L, -24) )
		//			0x3e70000000000000
		eq1 = _mm_castpd_si128( _mm_cmple_pd(absParam, *((__m128d *)x3e70000000000000)) );

		// if (f < 0) return 0x8001;
		eq0 = _mm_castpd_si128( _mm_cmplt_pd(param, zeros) );
		eq0 = _mm_and_si128(eq0, eq1);
		eq2 = _mm_and_si128(eq0, *((__m128i *)x8001));
		eq2 = _mm_andnot_si128(dflt, eq2);
		eq = _mm_or_si128(eq, eq2);
		dflt = _mm_or_si128(eq0, dflt);
		// else return sign
		eq0 = _mm_xor_si128(eq1, eq0);
		eq2 = _mm_and_si128(eq0, signs);
		eq2 = _mm_andnot_si128(dflt, eq2);
		eq = _mm_or_si128(eq, eq2);
		dflt = _mm_or_si128(eq0, dflt);

		// half denormal         
		//  if( x < MAKE_HEX_DOUBLE(0x1.0p-14, 0x1L, -14) )
		//			0x3f10000000000000
		//		x *= MAKE_HEX_DOUBLE(0x1.0p24, 0x1L, 24);
		//			 0x4170000000000000
		//		int r = (int)x;
		eq1 = _mm_castpd_si128( _mm_cmplt_pd(absParam, *((__m128d *)x3f10000000000000)) );
		eq2 = eq1;
		eq1 = _mm_and_si128(eq1, _mm_castpd_si128(absParam));
		eq1 = _mm_castpd_si128( _mm_mul_pd(_mm_castsi128_pd(eq1), *((__m128d *)x4170000000000000)) );  //x
		eq0 = _mm_cvtpd_epi32( _mm_castsi128_pd(eq1) ); //r
		
		// if( sign )
		//     r += (double) r != x;
		__m128d eq3 = _mm_cvtepi32_pd(eq0); //(double)r
		eq1 = _mm_castpd_si128( _mm_cmpneq_pd( _mm_castsi128_pd( eq1 ), eq3) ); // (double)r != x
		eq3 = _mm_cmplt_pd(param, zeros); //f < 0.0f
		eq1 = _mm_and_si128(eq1, _mm_castpd_si128(eq3)); //(double)r != x && f < 0.0f
		__m128i eq4 = _mm_and_si128(eq1, *((__m128i *)dones));
		eq0 = _mm_unpacklo_epi32(eq0, _mm_setzero_si128());
		eq0 = _mm_add_epi64(eq0, eq4);

		// return (short)(r | sign)
		eq0 = _mm_or_si128(eq0, signs); 
		eq0 = _mm_and_si128(eq2, eq0);
		eq0 = _mm_andnot_si128(dflt, eq0);
		eq  = _mm_or_si128(eq, eq0); 
		dflt = _mm_or_si128(eq2, dflt);

		//u.u &= 0xFFFFFC0000000000UL;
		eq0 = _mm_and_si128(_mm_castpd_si128(param), *((__m128i *)xFFFFFC0000000000));
		//if (u.f > f)
		//u.u += 0x0000040000000000UL;
		eq1 = _mm_castpd_si128( _mm_cmpgt_pd(_mm_castsi128_pd( eq0 ), param) );
		eq2 = _mm_add_epi64(eq0, _mm_and_si128(eq1, *((__m128i *)x0000040000000000)));
		//u.u -= 0x3F00000000000000UL;
		//return ((u.u >> 42) | sign);
		eq2 = _mm_sub_epi64(eq2, *((__m128i *)x3F00000000000000));
		eq2 = _mm_srli_epi64(eq2, 42);
		eq2 = _mm_or_si128(eq2, signs);
		eq2 = _mm_andnot_si128(dflt, eq2);
		eq  = _mm_or_si128(eq, eq2); 

		eq1 = SHUFFLE_EPI8(eq, *((__m128i *)_2x64to2x16));

		_MM_SET_ROUNDING_MODE(rm);

		return _mm_castsi128_ps(eq1);
	}
/*
#pragma linkage _lnk_l2f_ = ( result (xmm0) parameters(memory ,memory) )
#pragma linkage _lnk_f2i_ = ( result (eax) parameters(xmm0 ,memory) )
#pragma linkage _lnk_f2i4_ = ( result (xmm0) parameters(xmm0 ,memory) )


#pragma use_linkage _lnk_l2f_ ( longToFloat )
#pragma use_linkage _lnk_l2f_ ( ulongToFloat )
#pragma use_linkage _lnk_f2i_ ( floatToIntRound )
#pragma use_linkage _lnk_f2i4_ ( float4ToInt4Round )
#pragma use_linkage _lnk_f2i4_ ( double2ToInt4 )
#pragma use_linkage _lnk_f2i4_ ( intToFloatRound )
*/




#else

	// Declare external functions
    float4 double2ToFloat4Round(double2 param, int rm);
    float ulongToFloat(_1u64 param, int rm);
	float longToFloat(_1i64 param, int rm);
	__m128i float4ToInt4Round(float4 param, int rMode);
	__m128i double2ToInt4(double2 param, int rMode);
	int floatToIntRound(float4 param, int rMode);
	float uintToFloatRound(_1u32 param, int rMode);
	float4 intToFloatRound(_4i32, int rMode);
    _1u32 floatToUintRound(float param, int rMode);
    _4u32 float4ToUint4Round(float4 param, int rMode);

#if defined(__AVX__)
    __m256 intToFloat8Round(__m256i param, int rMode);
    __m256i float8ToInt8Round(__m256 param, int rMode);
    __m128 double4ToFloat4Round(__m256d param, int rm);
    __m256i float8ToUint8Round(__m256 param, int rMode);
#endif

#define _LONG_TO_INT   0x88
#define _ULLONG_MAX    0xFFFFFFFFFFFFFFFF       /* maximum unsigned long long int value */
#define _LLONG_MAX     0x7FFFFFFFFFFFFFFF       /* maximum signed long long int value */
#define _LLONG_MIN     0x8000000000000000       /* minimum signed long long int value */
#define _INT_MAX		  2147483647
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

    float __attribute__((const)) OCL_SVML_FUNCTION(cvtu64tofprtnf1)(_1u64);
    float ulongToFloat(_1u64 param, int rm)
	{
        return OCL_SVML_FUNCTION(cvtu64tofprtnf1)(param);
    }

    float3 __attribute__((const)) OCL_SVML_FUNCTION(cvtu64tofprtnf3)(_3u64);
    float3 ulong3ToFloat3(_3u64 param, int rm)
	{
        return OCL_SVML_FUNCTION(cvtu64tofprtnf3)(param);
    }

    float4 __attribute__((const)) OCL_SVML_FUNCTION(cvtu64tofprtnf4)(_4u64);
    float4 ulong4ToFloat4(_4u64 param, int rm)
	{
        return OCL_SVML_FUNCTION(cvtu64tofprtnf4)(param);
    }

    float8 __attribute__((const)) OCL_SVML_FUNCTION(cvtu64tofprtnf8)(_8u64);
    float8 ulong8ToFloat8(_8u64 param, int rm)
	{
        return OCL_SVML_FUNCTION(cvtu64tofprtnf8)(param);
    }

    float16 __attribute__((const)) OCL_SVML_FUNCTION(cvtu64tofprtnf16)(_16u64);
    float16 ulong16ToFloat16(_16u64 param, int rm)
	{
        return OCL_SVML_FUNCTION(cvtu64tofprtnf16)(param);
    }

#if defined(__AVX__)
	__m256i float8ToInt8(float8 param, int dummy)
	{
		__m256i res =  _mm256_cvttps_epi32 (param);
		return res;
	} 
#endif // defined(__AVX__)
        
    __m128i float4ToInt4(float4 param, int dummy)
	{
		__m128i res = _mm_cvttps_epi32(param);
		return res;
	} 

	int floatToInt(float4 param, int dummy)
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

	void setRound(int newMode)
	{
		_mm_setcsr(newMode);
	}

	int getRound()
	{
		return _mm_getcsr();
	}

	_4i32 floatToIntSat(float4 param, int dummy)
	{
		__m128i t1 = (__m128i)_mm_cmpge_ps(param, *((__m128 *)maxInt32) );
		__m128i t2 = (__m128i)_mm_cmpge_ps(*((__m128 *)minInt32), param );
		__m128i t = _mm_or_si128(t1, t2);
		t1 = _mm_and_si128(t1, *((__m128i *)maxIntVal32));
		t2 = _mm_and_si128(t2, *((__m128i *)minIntVal32));
		t = _mm_andnot_si128(t, float4ToInt4(param, dummy) );
		_4i32 res = _mm_or_si128(t1, t2);
		res = _mm_or_si128(res, t);
		return res;
	}

	_4i32 floatToIntSatRound(float4 param, int rMode)
	{
		__m128i t1 = (__m128i)_mm_cmpge_ps(param, *((__m128 *)maxInt32) );
		__m128i t2 = (__m128i)_mm_cmpge_ps(*((__m128 *)minInt32), param );
		__m128i t = _mm_or_si128(t1, t2);
		t1 = _mm_and_si128(t1, *((__m128i *)maxIntVal32));
		t2 = _mm_and_si128(t2, *((__m128i *)minIntVal32));
		t = _mm_andnot_si128(t, float4ToInt4Round(param, rMode) );
		_4i32 res = _mm_or_si128(t1, t2);
		res = _mm_or_si128(res, t);
		return res;
	}

	int doubleToIntSat(double param, int rMode)
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
		_4i32 res = double2ToInt4(t, rMode);
		return res.s0;
	}

	_1u32 floatToUint(float param, int dummy)
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
	
    _4u32 float4ToUint4(float4 param, int dummy)
	{
        _4u32 res;
        const ALIGN16 float mth_INT_MAX_32f[4] = { 2147483647.0f, 2147483647.0f, 2147483647.0f, 2147483647.0f};
        _4i32 mask_gt= _mm_cmpgt_ps(param,  *(__m128*)mth_INT_MAX_32f);
         
        if(_any4(mask_gt))
        {
             res.s0 = (_1u32)param.s0;
             res.s1 = (_1u32)param.s1;
             res.s2 = (_1u32)param.s2;
             res.s3 = (_1u32)param.s3;
        }
        else
        {
            res =  _mm_cvttps_epi32(param);
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

    _8u32 float8ToUint8(float8 param, int dummy)
    {
        _8u32 res;
        const ALIGN32 float mth_INT_MAX_32f[8] = { 2147483647.0f, 2147483647.0f, 2147483647.0f, 2147483647.0f, 
                                                   2147483647.0f, 2147483647.0f, 2147483647.0f, 2147483647.0f};
        _8i32 mask_gt= _mm256_cmp_ps(param,  *(__m256*)mth_INT_MAX_32f, _CMP_GT_OS );
         
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
            res =   _mm256_cvttps_epi32(param);
        }
        return res;
    }
#endif // defined(__AVX__)

    _1u32 floatToUintSat(float param)
	{
		float4 p;
		if(param >= 4294967295.0f) return _UINT_MAX;

		if(param <= 0.0f) return 0;
		if(param > 2147483647.0f)
		{
			param -= 2147483647.0f;
			p.s0 = param;
			_1u32 res = _mm_cvtss_si32(p);
			res += ((float)_INT_MAX);
			return res;
		}
		p.s0 = param;
		_1u32 res = _mm_cvtss_si32(p);
		return res;
	}

	float4 intToFloat(_4i32 param, int dummy)
	{
		return _mm_cvtepi32_ps(param);
	}

#if defined(__AVX__)
    float8 intToFloat8(_8i32 param, int dummy)
	{
        return _mm256_cvtepi32_ps(param);
	}
#endif

    float uintToFloat(_1u32 x, int dummy)
	{
		if(x == 0) return 0.0;
		float res =  ((float)x);
		return res;
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
	_16##TO##8 res;\
	_8##TI##16 param;\
	param.s01 = x;\
	res = _mm_shuffle_epi8(param, *((_16i8 *)_8x16to8x8));\
	return res.s01;\
	}\
	TONAME##char3 __attribute__((overloadable)) convert_##TONAME##char3##RMODE(TINAME##short3 x)\
	{\
	_16##TO##8 res;\
	_8##TI##16 param;\
	param.lo = as_##TINAME##short4(x);\
	res = _mm_shuffle_epi8(param, *((_16i8 *)_8x16to8x8));\
	return as_##TONAME##char3(res.s0123);\
	}\
	_4##TO##8 __attribute__((overloadable)) convert_##TONAME##char4##RMODE(_4##TI##16 x)\
	{\
	_16##TO##8 res;\
	_8##TI##16 param;\
	param.lo = x;\
	res = _mm_shuffle_epi8(param, *((_16i8 *)_8x16to8x8));\
	return res.s0123;\
	}\
	_8##TO##8 __attribute__((overloadable)) convert_##TONAME##char8##RMODE(_8##TI##16 x)\
	{\
	_16##TO##8 res;\
	res = _mm_shuffle_epi8(x, *((_16i8 *)_8x16to8x8));\
	return res.lo;\
	}\
	_16##TO##8 __attribute__((overloadable)) convert_##TONAME##char16##RMODE(_16##TI##16 x)\
	{\
	_16##TO##8 res, temp1, temp2;\
	temp1 = _mm_shuffle_epi8(x.lo, *((_16i8 *)_8x16to8x8));\
	temp2 = _mm_shuffle_epi8(x.hi, *((_16i8 *)_8x16to8x8));\
	res = _mm_unpacklo_epi64(temp1, temp2);\
	return res;\
	}\

#define DEF_INT_PROTO8_32(TI, TO, TINAME, TONAME, RMODE)\
	_1##TO##8 __attribute__((overloadable)) convert_##TONAME##char##RMODE(_1##TI##32 x)\
	{\
	return (_1##TO##8)x;\
	}\
	_2##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char2##RMODE(_2##TI##32 x)\
	{\
	_16##TO##8 res;\
	_4##TI##32 param;\
	param.lo = x;\
	res = _mm_shuffle_epi8(param, *((_16i8 *)_4x32to4x8));\
	return res.s01;\
	}\
	TONAME##char3 __attribute__((overloadable)) convert_##TONAME##char3##RMODE(TINAME##int3 x)\
	{\
	_16##TO##8 res;\
	res = _mm_shuffle_epi8(as_##TINAME##int4(x), *((_16i8 *)_4x32to4x8));\
	return as_##TONAME##char3(res.s0123);\
	}\
	_4##TO##8 __attribute__((overloadable)) convert_##TONAME##char4##RMODE(_4##TI##32 x)\
	{\
	_16##TO##8 res;\
	res = _mm_shuffle_epi8(x, *((_16i8 *)_4x32to4x8));\
	return res.s0123;\
	}\
	_8##TO##8 __attribute__((overloadable)) convert_##TONAME##char8##RMODE(_8##TI##32 x)\
	{\
	_16##TO##8 res, temp1, temp2;\
	temp1 = _mm_shuffle_epi8(x.lo, *((_16i8 *)_4x32to4x8));\
	temp2 = _mm_shuffle_epi8(x.hi, *((_16i8 *)_4x32to4x8));\
	res = _mm_unpacklo_epi32(temp1, temp2);\
	return res.lo;\
	}\
	_16##TO##8 __attribute__((overloadable)) convert_##TONAME##char16##RMODE(_16##TI##32 x)\
	{\
	_16##TO##8 res, temp1, temp2;\
	temp1 = _mm_shuffle_epi8(x.lo.lo, *((_16i8 *)_4x32to4x8));\
	temp2 = _mm_shuffle_epi8(x.lo.hi, *((_16i8 *)_4x32to4x8));\
	res = _mm_unpacklo_epi32(temp1, temp2);\
	temp1 = _mm_shuffle_epi8(x.hi.lo, *((_16i8 *)_4x32to4x8));\
	temp2 = _mm_shuffle_epi8(x.hi.hi, *((_16i8 *)_4x32to4x8));\
	temp1 = _mm_unpacklo_epi32(temp1, temp2);\
	res = _mm_unpacklo_epi64(res, temp1);\
	return res;\
	}\

#define DEF_INT_PROTO8_64(TI, TO, TINAME, TONAME, RMODE)\
	_1##TO##8 __attribute__((overloadable)) convert_##TONAME##char##RMODE(_1##TI##64 x)\
	{\
	return (_1##TO##8)x;\
	}\
	_2##TO##8 __attribute__((overloadable)) convert_##TONAME##char2##RMODE(_2##TI##64 x)\
	{\
	_16##TO##8 res;\
	res = _mm_shuffle_epi8(x, *((_16i8 *)_2x64to2x8));\
	return res.s01;\
	}\
	TONAME##char3 __attribute__((overloadable)) convert_##TONAME##char3##RMODE(TINAME##long3 x)\
	{\
	_16##TO##8 res, temp1, temp2;\
	TINAME##long4 y = as_##TINAME##long4(x);\
	temp1 = _mm_shuffle_epi8(y.lo, *((_16i8 *)_2x64to2x8));\
	temp2 = _mm_shuffle_epi8(y.hi, *((_16i8 *)_2x64to2x8));\
	res = _mm_unpacklo_epi16(temp1, temp2);\
	return as_##TONAME##char3(res.s0123);\
	}\
	_4##TO##8 __attribute__((overloadable)) convert_##TONAME##char4##RMODE(_4##TI##64 x)\
	{\
	_16##TO##8 res, temp1, temp2;\
	temp1 = _mm_shuffle_epi8(x.lo, *((_16i8 *)_2x64to2x8));\
	temp2 = _mm_shuffle_epi8(x.hi, *((_16i8 *)_2x64to2x8));\
	res = _mm_unpacklo_epi16(temp1, temp2);\
	return res.s0123;\
	}\
	_8##TO##8 __attribute__((overloadable)) convert_##TONAME##char8##RMODE(_8##TI##64 x)\
	{\
	_16##TO##8 res, temp1, temp2;\
	temp1 = _mm_shuffle_epi8(x.lo.lo, *((_16i8 *)_2x64to2x8));\
	temp2 = _mm_shuffle_epi8(x.lo.hi, *((_16i8 *)_2x64to2x8));\
	res = _mm_unpacklo_epi16(temp1, temp2);\
	temp1 = _mm_shuffle_epi8(x.hi.lo, *((_16i8 *)_2x64to2x8));\
	temp2 = _mm_shuffle_epi8(x.hi.hi, *((_16i8 *)_2x64to2x8));\
	temp1 = _mm_unpacklo_epi16(temp1, temp2);\
	res = _mm_unpacklo_epi32(res, temp1);\
	return res.lo;\
	}\
	_16##TO##8 __attribute__((overloadable)) convert_##TONAME##char16##RMODE(_16##TI##64 x)\
	{\
	_16##TO##8 res, temp1, temp2, temp3;\
	temp1 = _mm_shuffle_epi8(x.lo.lo.lo, *((_16i8 *)_2x64to2x8));\
	temp2 = _mm_shuffle_epi8(x.lo.lo.hi, *((_16i8 *)_2x64to2x8));\
	res = _mm_unpacklo_epi16(temp1, temp2);\
	temp1 = _mm_shuffle_epi8(x.lo.hi.lo, *((_16i8 *)_2x64to2x8));\
	temp2 = _mm_shuffle_epi8(x.lo.hi.hi, *((_16i8 *)_2x64to2x8));\
	temp1 = _mm_unpacklo_epi16(temp1, temp2);\
	res = _mm_unpacklo_epi32(res, temp1);\
	temp1 = _mm_shuffle_epi8(x.hi.lo.lo, *((_16i8 *)_2x64to2x8));\
	temp2 = _mm_shuffle_epi8(x.hi.lo.hi, *((_16i8 *)_2x64to2x8));\
	temp3 = _mm_unpacklo_epi16(temp1, temp2);\
	temp1 = _mm_shuffle_epi8(x.hi.hi.lo, *((_16i8 *)_2x64to2x8));\
	temp2 = _mm_shuffle_epi8(x.hi.hi.hi, *((_16i8 *)_2x64to2x8));\
	temp1 = _mm_unpacklo_epi16(temp1, temp2);\
	temp3 = _mm_unpacklo_epi32(temp3, temp1);\
	res = _mm_unpacklo_epi64(res, temp3);\
	return res;\
	}

#define DEF_INT_PROTO8_F(TO, TONAME, RMODE, RMODEVAL, FLAG)\
	_1##TO##8 __attribute__((overloadable)) convert_##TONAME##char##RMODE(float x)\
	{\
	float4 param;\
	param.s01 = x;\
	return floatToInt##FLAG(param, RMODEVAL);\
	}\
	_2##TO##8 __attribute__((overloadable)) convert_##TONAME##char2##RMODE(float2 x)\
	{\
	float4 param;\
	param.lo = x;\
	_16##TO##8 res = float4ToInt4##FLAG(param, RMODEVAL);\
	res = _mm_shuffle_epi8(res, *((__m128i *)_4x32to4x8));\
	return res.s01;\
	}\
	TONAME##char3 __attribute__((overloadable)) convert_##TONAME##char3##RMODE(float3 x)\
	{\
	_16##TO##8 res = float4ToInt4##FLAG(as_float4(x), RMODEVAL);\
	res = _mm_shuffle_epi8(res, *((__m128i *)_4x32to4x8));\
	return as_##TONAME##char3(res.s0123);\
	}\
	_4##TO##8 __attribute__((overloadable)) convert_##TONAME##char4##RMODE(float4 x)\
	{\
	_16##TO##8 res = float4ToInt4##FLAG(x, RMODEVAL);\
	res = _mm_shuffle_epi8(res, *((__m128i *)_4x32to4x8));\
	return res.s0123;\
	}\
	_8##TO##8 __attribute__((overloadable)) convert_##TONAME##char8##RMODE(float8 x)\
	{\
	_16##TO##8 res;\
	_16##TO##8 t1 =  float4ToInt4##FLAG(x.lo, RMODEVAL);\
	_16##TO##8 t2 =  float4ToInt4##FLAG(x.hi, RMODEVAL);\
	t1 = _mm_shuffle_epi8(t1, *((__m128i *)_4x32to4x8));\
	t2 = _mm_shuffle_epi8(t2, *((__m128i *)_4x32to4x8));\
	res = _mm_unpacklo_epi32(t1, t2);\
	return res.lo;\
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
	double2 param;\
	param.lo = x;\
	_4i32 res = double2ToInt4(param, RMODEVAL);\
	return res.s0;\
	}\
	_2##TO##8 __attribute__((overloadable)) convert_##TONAME##char2##RMODE(double2 x)\
	{\
	_16##TO##8 res = double2ToInt4(x, RMODEVAL);\
	res = _mm_shuffle_epi8(res, *((__m128i *)_4x32to4x8));\
	return res.s01;\
	}\
	TONAME##char3 __attribute__((overloadable)) convert_##TONAME##char3##RMODE(double3 x)\
	{\
	double4 y;\
	y.s012 = x;\
	_16##TO##8 t1 = double2ToInt4(y.lo, RMODEVAL);\
	_16##TO##8 t2 = double2ToInt4(y.hi, RMODEVAL);\
	t1 = _mm_shuffle_epi8(t1, *((__m128i *)_4x32to4x8));\
	t2 = _mm_shuffle_epi8(t2, *((__m128i *)_4x32to4x8));\
	_16##TO##8 res = _mm_unpacklo_epi16(t1, t2);\
	return res.s012;\
	}\
	_4##TO##8  __attribute__((overloadable)) convert_##TONAME##char4##RMODE(double4 x)\
	{\
	_16##TO##8 t1 = double2ToInt4(x.lo, RMODEVAL);\
	_16##TO##8 t2 = double2ToInt4(x.hi, RMODEVAL);\
	t1 = _mm_shuffle_epi8(t1, *((__m128i *)_4x32to4x8));\
	t2 = _mm_shuffle_epi8(t2, *((__m128i *)_4x32to4x8));\
	_16##TO##8 res = _mm_unpacklo_epi16(t1, t2);\
	return res.s0123;\
	}\
	_8##TO##8  __attribute__((overloadable)) convert_##TONAME##char8##RMODE(double8 x)\
	{\
	_16##TO##8 res;\
	_16##TO##8 t1 =  double2ToInt4(x.lo.lo, RMODEVAL);\
	_16##TO##8 t2 =  double2ToInt4(x.lo.hi, RMODEVAL);\
	t1 = _mm_shuffle_epi8(t1, *((__m128i *)_4x32to4x8));\
	t2 = _mm_shuffle_epi8(t2, *((__m128i *)_4x32to4x8));\
	res = _mm_unpacklo_epi16(t1, t2);\
	t1 =  double2ToInt4(x.hi.lo, RMODEVAL);\
	t2 =  double2ToInt4(x.hi.hi, RMODEVAL);\
	t1 = _mm_shuffle_epi8(t1, *((__m128i *)_4x32to4x8));\
	t2 = _mm_shuffle_epi8(t2, *((__m128i *)_4x32to4x8));\
	t1 = _mm_unpacklo_epi16(t1, t2);\
	res = _mm_unpacklo_epi32(res, t1);\
	return res.lo;\
	}\
	_16##TO##8  __attribute__((overloadable)) convert_##TONAME##char16##RMODE(double16 x)\
	{\
	_16##TO##8 res;\
	res.lo = convert_##TONAME##char8##RMODE(x.lo);\
	res.hi = convert_##TONAME##char8##RMODE(x.hi);\
	return res;\
	}


	// 16 bits
#define DEF_INT_PROTO16_8(TI, TO, TINAME, TONAME, RMODE)\
	_1##TO##16 __attribute__((overloadable)) convert_##TONAME##short##RMODE(_1##TI##8 x)\
	{\
	return (_1##TO##16)x;\
	}\
	_2##TO##16 __attribute__((overloadable)) convert_##TONAME##short2##RMODE(_2##TI##8 x)\
	{\
	_8##TO##16 res;\
	_16##TI##8 param;\
	param.s01 = x;\
	res = _mm_cvtep##TI##8_epi16(param);\
	return res.s01;\
	}\
	TONAME##short3 __attribute__((overloadable)) convert_##TONAME##short3##RMODE(TINAME##char3 x)\
	{\
	_8##TO##16 res;\
	_16##TI##8 param;\
	param.s0123 = as_##TINAME##char4(x);\
	res = _mm_cvtep##TI##8_epi16(param);\
	return as_##TONAME##short3(res.s0123);\
	}\
	_4##TO##16 __attribute__((overloadable)) convert_##TONAME##short4##RMODE(_4##TI##8 x)\
	{\
	_8##TO##16 res;\
	_16##TI##8 param;\
	param.s0123 = x;\
	res = _mm_cvtep##TI##8_epi16(param);\
	return res.s0123;\
	}\
	_8##TO##16 __attribute__((overloadable)) convert_##TONAME##short8##RMODE(_8##TI##8 x)\
	{\
	_8##TO##16 res;\
	_16##TI##8 param;\
	param.lo = x;\
	res = _mm_cvtep##TI##8_epi16(param);\
	return res;\
	}\
	_16##TO##16 __attribute__((overloadable)) convert_##TONAME##short16##RMODE(_16##TI##8 x)\
	{\
	_16##TO##16 res;\
	res.lo = _mm_cvtep##TI##8_epi16(x);\
	res.hi = _mm_cvtep##TI##8_epi16(_mm_srli_si128(x, 8));\
	return res;\
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
	_8##TO##16 res;\
	_4##TI##32 param;\
	param.lo = x;\
	res = _mm_shuffle_epi8(param, *((__m128i *)_4x32to4x16));\
	return res.s01;\
	}\
	TONAME##short3 __attribute__((overloadable)) convert_##TONAME##short3##RMODE(TINAME##int3 x)\
	{\
	_8##TO##16 res;\
	res = _mm_shuffle_epi8(as_##TINAME##int4(x), *((__m128i *)_4x32to4x16));\
	return as_##TONAME##short3(res.s0123);\
	}\
	_4##TO##16 __attribute__((overloadable)) convert_##TONAME##short4##RMODE(_4##TI##32 x)\
	{\
	_8##TO##16 res;\
	res = _mm_shuffle_epi8(x, *((__m128i *)_4x32to4x16));\
	return res.s0123;\
	}\
	_8##TO##16 __attribute__((overloadable)) convert_##TONAME##short8##RMODE(_8##TI##32 x)\
	{\
	_8##TO##16 res, temp1, temp2;\
	temp1 = _mm_shuffle_epi8(x.lo, *((__m128i *)_4x32to4x16));\
	temp2 = _mm_shuffle_epi8(x.hi, *((__m128i *)_4x32to4x16));\
	res = _mm_unpacklo_epi64(temp1, temp2);\
	return res;\
	}\
	_16##TO##16 __attribute__((overloadable)) convert_##TONAME##short16##RMODE(_16##TI##32 x)\
	{\
	_16##TO##16 res;\
	_8##TO##16 temp1, temp2;\
	temp1 = _mm_shuffle_epi8(x.lo.lo, *((__m128i *)_4x32to4x16));\
	temp2 = _mm_shuffle_epi8(x.lo.hi, *((__m128i *)_4x32to4x16));\
	res.lo = _mm_unpacklo_epi64(temp1, temp2);\
	temp1 = _mm_shuffle_epi8(x.hi.lo, *((__m128i *)_4x32to4x16));\
	temp2 = _mm_shuffle_epi8(x.hi.hi, *((__m128i *)_4x32to4x16));\
	res.hi = _mm_unpacklo_epi64(temp1, temp2);\
	return res;\
	}\

#define DEF_INT_PROTO16_64(TI, TO, TINAME, TONAME, RMODE)\
	_1##TO##16 __attribute__((overloadable)) convert_##TONAME##short##RMODE(_1##TI##64 x)\
	{\
	return (_1##TO##16)x;\
	}\
	_2##TO##16 __attribute__((overloadable)) convert_##TONAME##short2##RMODE(_2##TI##64 x)\
	{\
	_8##TO##16 res;\
	res = _mm_shuffle_epi8(x, *((__m128i *)_2x64to2x16));\
	return res.s01;\
	}\
	TONAME##short3 __attribute__((overloadable)) convert_##TONAME##short3##RMODE(TINAME##long3 x)\
	{\
	_8##TO##16 res, temp1, temp2;\
	TINAME##long4 y = as_##TINAME##long4(x);\
	temp1 = _mm_shuffle_epi8(y.lo, *((__m128i *)_2x64to2x16));\
	temp2 = _mm_shuffle_epi8(y.hi, *((__m128i *)_2x64to2x16));\
	res = _mm_unpacklo_epi32(temp1, temp2);\
	return as_##TONAME##short3(res.lo);\
	}\
	_4##TO##16 __attribute__((overloadable)) convert_##TONAME##short4##RMODE(_4##TI##64 x)\
	{\
	_8##TO##16 res, temp1, temp2;\
	temp1 = _mm_shuffle_epi8(x.lo, *((__m128i *)_2x64to2x16));\
	temp2 = _mm_shuffle_epi8(x.hi, *((__m128i *)_2x64to2x16));\
	res = _mm_unpacklo_epi32(temp1, temp2);\
	return res.lo;\
	}\
	_8##TO##16 __attribute__((overloadable)) convert_##TONAME##short8##RMODE(_8##TI##64 x)\
	{\
	_8##TO##16 res, temp1, temp2;\
	temp1 = _mm_shuffle_epi8(x.lo.lo, *((__m128i *)_2x64to2x16));\
	temp2 = _mm_shuffle_epi8(x.lo.hi, *((__m128i *)_2x64to2x16));\
	res = _mm_unpacklo_epi32(temp1, temp2);\
	temp1 = _mm_shuffle_epi8(x.hi.lo, *((__m128i *)_2x64to2x16));\
	temp2 = _mm_shuffle_epi8(x.hi.hi, *((__m128i *)_2x64to2x16));\
	temp1 = _mm_unpacklo_epi32(temp1, temp2);\
	res = _mm_unpacklo_epi64(res, temp1);\
	return res;\
	}\
	_16##TO##16 __attribute__((overloadable)) convert_##TONAME##short16##RMODE(_16##TI##64 x)\
	{\
	_16##TO##16 res;\
	_8##TO##16 temp1, temp2;\
	temp1 = _mm_shuffle_epi8(x.lo.lo.lo, *((__m128i *)_2x64to2x16));\
	temp2 = _mm_shuffle_epi8(x.lo.lo.hi, *((__m128i *)_2x64to2x16));\
	res.lo = _mm_unpacklo_epi32(temp1, temp2);\
	temp1 = _mm_shuffle_epi8(x.lo.hi.lo, *((__m128i *)_2x64to2x16));\
	temp2 = _mm_shuffle_epi8(x.lo.hi.hi, *((__m128i *)_2x64to2x16));\
	temp1 = _mm_unpacklo_epi32(temp1, temp2);\
	res.lo = _mm_unpacklo_epi64(res.lo, temp1);\
	temp1 = _mm_shuffle_epi8(x.hi.lo.lo, *((__m128i *)_2x64to2x16));\
	temp2 = _mm_shuffle_epi8(x.hi.lo.hi, *((__m128i *)_2x64to2x16));\
	res.hi = _mm_unpacklo_epi32(temp1, temp2);\
	temp1 = _mm_shuffle_epi8(x.hi.hi.lo, *((__m128i *)_2x64to2x16));\
	temp2 = _mm_shuffle_epi8(x.hi.hi.hi, *((__m128i *)_2x64to2x16));\
	temp1 = _mm_unpacklo_epi32(temp1, temp2);\
	res.hi = _mm_unpacklo_epi64(res.hi, temp1);\
	return res;\
	}\

#define DEF_INT_PROTO16_F(TO, TONAME, RMODE, RMODEVAL, FLAG)\
	_1##TO##16 __attribute__((overloadable)) convert_##TONAME##short##RMODE(float x)\
	{\
	_1##TO##16 res;\
	float4 param;\
	param.s0 = x;\
	res = floatToInt##FLAG(param , RMODEVAL);\
	return res;\
	}\
	_2##TO##16 __attribute__((overloadable)) convert_##TONAME##short2##RMODE(float2 x)\
	{\
	_8##TO##16 res;\
	float4 param;\
	param.lo = x;\
	res = float4ToInt4##FLAG(param, RMODEVAL);\
	res = _mm_shuffle_epi8(res, *((__m128i *)_4x32to4x16));\
	return res.s01;\
	}\
	TONAME##short3 __attribute__((overloadable)) convert_##TONAME##short3##RMODE(float3 x)\
	{\
	_8##TO##16 res;\
	res = float4ToInt4##FLAG(as_float4(x), RMODEVAL);\
	res = _mm_shuffle_epi8(res, *((__m128i *)_4x32to4x16));\
	return as_##TONAME##short3(res.lo);\
	}\
	_4##TO##16 __attribute__((overloadable)) convert_##TONAME##short4##RMODE(float4 x)\
	{\
	_8##TO##16 res;\
	res = float4ToInt4##FLAG(x, RMODEVAL);\
	res = _mm_shuffle_epi8(res, *((__m128i *)_4x32to4x16));\
	return res.lo;\
	}\
	_8##TO##16 __attribute__((overloadable)) convert_##TONAME##short8##RMODE(float8 x)\
	{\
	_8##TO##16 res;\
	_8##TO##16 t1 = float4ToInt4##FLAG(x.lo, RMODEVAL);\
	_8##TO##16 t2 = float4ToInt4##FLAG(x.hi, RMODEVAL);\
	t1 = _mm_shuffle_epi8(t1, *((__m128i *)_4x32to4x16));\
	t2 = _mm_shuffle_epi8(t2, *((__m128i *)_4x32to4x16));\
	res = _mm_unpacklo_epi64(t1, t2);\
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
	double2 param;\
	param.s0 = x;\
	_4i32 res = double2ToInt4(param, RMODEVAL);\
	return res.s0;\
	}\
	_2##TO##16 __attribute__((overloadable)) convert_##TONAME##short2##RMODE(double2 x)\
	{\
	_8##TO##16 res;\
	res = double2ToInt4(x, RMODEVAL);\
	res = _mm_shuffle_epi8(res, *((__m128i *)_4x32to4x16));\
	return res.s01;\
	}\
	_3##TO##16 __attribute__((overloadable)) convert_##TONAME##short3##RMODE(double3 x)\
	{\
	_8##TO##16 res, t1, t2;\
	double4 y = as_double4(x);\
	t1 = double2ToInt4(y.lo, RMODEVAL);\
	t2 = double2ToInt4(y.hi, RMODEVAL);\
	res = _mm_unpacklo_epi64(t1, t2);\
	res = _mm_shuffle_epi8(res, *((__m128i *)_4x32to4x16));\
	return as_##TONAME##short3(res.lo);\
	}\
	_4##TO##16 __attribute__((overloadable)) convert_##TONAME##short4##RMODE(double4 x)\
	{\
	_8##TO##16 res, t1, t2;\
	t1 = double2ToInt4(x.lo, RMODEVAL);\
	t2 = double2ToInt4(x.hi, RMODEVAL);\
	res = _mm_unpacklo_epi64(t1, t2);\
	res = _mm_shuffle_epi8(res, *((__m128i *)_4x32to4x16));\
	return res.lo;\
	}\
	_8##TO##16 __attribute__((overloadable)) convert_##TONAME##short8##RMODE(double8 x)\
	{\
	_8##TO##16 res;\
	_8##TO##16 t1 = double2ToInt4(x.lo.lo, RMODEVAL);\
	_8##TO##16 t2 = double2ToInt4(x.lo.hi, RMODEVAL);\
	t1 = _mm_shuffle_epi8(t1, *((__m128i *)_4x32to4x16));\
	t2 = _mm_shuffle_epi8(t2, *((__m128i *)_4x32to4x16));\
	res = _mm_unpacklo_epi32(t1, t2);\
	t1 = double2ToInt4(x.hi.lo, RMODEVAL);\
	t2 = double2ToInt4(x.hi.hi, RMODEVAL);\
	t1 = _mm_shuffle_epi8(t1, *((__m128i *)_4x32to4x16));\
	t2 = _mm_shuffle_epi8(t2, *((__m128i *)_4x32to4x16));\
	t1 = _mm_unpacklo_epi32(t1, t2);\
	res = _mm_unpacklo_epi64(res, t1);\
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
#define DEF_INT_PROTO32_8(TI, TO, TINAME, TONAME, RMODE)\
	_1##TO##32 __attribute__((overloadable)) convert_##TONAME##int##RMODE(_1##TI##8 x)\
	{\
	return (_1##TO##32)x;\
	}\
	_2##TO##32 __attribute__((overloadable)) convert_##TONAME##int2##RMODE(_2##TI##8 x)\
	{\
	_4##TO##32 res;\
	_16##TI##8 param;\
	param.s01 = x;\
	res =  _mm_cvtep##TI##8_epi32(param);\
	return res.lo;\
	}\
	TONAME##int3 __attribute__ ((overloadable)) convert_##TONAME##int3##RMODE(TINAME##char3 x)\
	{\
	_4##TO##32 res;\
	_16##TI##8 param;\
	param.s0123 = as_##TINAME##char4(x);\
	res =  _mm_cvtep##TI##8_epi32(param);\
	return as_##TONAME##int3(res);\
	}\
	_4##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int4##RMODE(_4##TI##8 x)\
	{\
	_4##TO##32 res;\
	_16##TI##8 param;\
	param.s0123 = x;\
	res =  _mm_cvtep##TI##8_epi32(param);\
	return res;\
	}\
	_8##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int8##RMODE(_8##TI##8 x)\
	{\
	_8##TO##32 res;\
	_16##TI##8 param;\
	param.lo = x;\
	res.lo = _mm_cvtep##TI##8_epi32(param);\
	res.hi = _mm_cvtep##TI##8_epi32(_mm_srli_si128(param, 4));\
	return res;\
	}\
	_16##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int16##RMODE(_16##TI##8 x)\
	{\
	_16##TO##32 res;\
	res.lo.lo = _mm_cvtep##TI##8_epi32(x);\
	res.lo.hi = _mm_cvtep##TI##8_epi32(_mm_srli_si128(x, 4));\
	res.hi.lo = _mm_cvtep##TI##8_epi32(_mm_srli_si128(x, 8));\
	res.hi.hi = _mm_cvtep##TI##8_epi32(_mm_srli_si128(x, 12));\
	return res;\
	}\

#define DEF_INT_PROTO32_16(TI, TO, TINAME, TONAME, RMODE)\
	_1##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int##RMODE(_1##TI##16 x)\
	{\
	return (_1##TO##32)x;\
	}\
	_2##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int2##RMODE(_2##TI##16 x)\
	{\
	_4##TO##32 res;\
	_8##TI##16 param;\
	param.s01 = x;\
	res = _mm_cvtep##TI##16_epi32(param);\
	return res.lo;\
	}\
	TONAME##int3 __attribute__ ((overloadable)) convert_##TONAME##int3##RMODE(TINAME##short3 x)\
	{\
	_8##TI##16 param;\
	param.lo = as_##TINAME##short4(x);\
	TONAME##int4 res = _mm_cvtep##TI##16_epi32(param);\
	return as_##TONAME##int3(res);\
	}\
	_4##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int4##RMODE(_4##TI##16 x)\
	{\
	_8##TI##16 param;\
	param.lo = x;\
	return _mm_cvtep##TI##16_epi32(param);\
	}\
	_8##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int8##RMODE(_8##TI##16 x)\
	{\
	_8##TO##32 res;\
	res.lo = _mm_cvtep##TI##16_epi32(x);\
	res.hi = _mm_cvtep##TI##16_epi32(_mm_srli_si128(x, 8));\
	return res;\
	}\
	_16##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int16##RMODE(_16##TI##16 x)\
	{\
	_16##TO##32 res;\
	res.lo.lo = _mm_cvtep##TI##16_epi32(x.lo);\
	res.lo.hi = _mm_cvtep##TI##16_epi32(_mm_srli_si128(x.lo, 8));\
	res.hi.lo = _mm_cvtep##TI##16_epi32(x.hi);\
	res.hi.hi = _mm_cvtep##TI##16_epi32(_mm_srli_si128(x.hi, 8));\
	return res;\
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
	_4##TO##32 res;\
	res = _mm_shuffle_epi32(x, _LONG_TO_INT);\
	return res.lo;\
	}\
	TONAME##int3 __attribute__ ((overloadable)) convert_##TONAME##int3##RMODE(TINAME##long3 x)\
	{\
	_4##TO##32 res, temp1, temp2;\
	TINAME##long4 y = as_##TINAME##long4(x);\
	temp1 = _mm_shuffle_epi32(y.lo, _LONG_TO_INT);\
	temp2 = _mm_shuffle_epi32(y.hi, _LONG_TO_INT);\
	res = _mm_unpacklo_epi64(temp1, temp2);\
	return as_##TONAME##int3(res);\
	}\
	_4##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int4##RMODE(_4##TI##64 x)\
	{\
	_4##TO##32 res, temp1, temp2;\
	temp1 = _mm_shuffle_epi32(x.lo, _LONG_TO_INT);\
	temp2 = _mm_shuffle_epi32(x.hi, _LONG_TO_INT);\
	res = _mm_unpacklo_epi64(temp1, temp2);\
	return res;\
	}\
	_8##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int8##RMODE(_8##TI##64 x)\
	{\
	_8##TO##32 res;\
	_4##TO##32 temp1, temp2;\
	temp1 = _mm_shuffle_epi32(x.lo.lo, _LONG_TO_INT);\
	temp2 = _mm_shuffle_epi32(x.lo.hi, _LONG_TO_INT);\
	res.lo = _mm_unpacklo_epi64(temp1, temp2);\
	temp1 = _mm_shuffle_epi32(x.hi.lo, _LONG_TO_INT);\
	temp2 = _mm_shuffle_epi32(x.hi.hi, _LONG_TO_INT);\
	res.hi = _mm_unpacklo_epi64(temp1, temp2);\
	return res;\
	}\
	_16##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int16##RMODE(_16##TI##64 x)\
	{\
	_16##TO##32 res;\
	_4##TO##32 temp1, temp2;\
	temp1 = _mm_shuffle_epi32(x.lo.lo.lo, _LONG_TO_INT);\
	temp2 = _mm_shuffle_epi32(x.lo.lo.hi, _LONG_TO_INT);\
	res.lo.lo = _mm_unpacklo_epi64(temp1, temp2);\
	temp1 = _mm_shuffle_epi32(x.lo.hi.lo, _LONG_TO_INT);\
	temp2 = _mm_shuffle_epi32(x.lo.hi.hi, _LONG_TO_INT);\
	res.lo.hi = _mm_unpacklo_epi64(temp1, temp2);\
	temp1 = _mm_shuffle_epi32(x.hi.lo.lo, _LONG_TO_INT);\
	temp2 = _mm_shuffle_epi32(x.hi.lo.hi, _LONG_TO_INT);\
	res.hi.lo = _mm_unpacklo_epi64(temp1, temp2);\
	temp1 = _mm_shuffle_epi32(x.hi.hi.lo, _LONG_TO_INT);\
	temp2 = _mm_shuffle_epi32(x.hi.hi.hi, _LONG_TO_INT);\
	res.hi.hi = _mm_unpacklo_epi64(temp1, temp2);\
	return res;\
	}\

#define DEF_INT_PROTOI32_F_F1234_AS_F4(RMODE, RMODEVAL, FLAG)       \
	_1i32 __attribute__ ((overloadable)) convert_int##RMODE(float x)\
	{\
	_1i32 res;\
	float4 param;\
	param.s0 = x;\
	res = floatToInt##FLAG(param, RMODEVAL);\
	return res;\
	}\
	_2i32 __attribute__ ((overloadable)) convert_int2##RMODE(float2 x)\
	{\
	_4i32 res;\
	float4 param;\
	param.lo = x;\
	res = float4ToInt4##FLAG(param, RMODEVAL);\
	return res.lo;\
	}\
	int3 __attribute__ ((overloadable)) convert_int3##RMODE(float3 x)\
	{\
	int4 res;\
	res = float4ToInt4##FLAG(as_float4(x), RMODEVAL);\
	return as_int3(res);\
	}\
	_4i32 __attribute__ ((overloadable)) convert_int4##RMODE(float4 x)\
	{\
	_4i32 res;\
	res = float4ToInt4##FLAG(x, RMODEVAL);\
	return res;\
	}\


#define DEF_INT_PROTOI32_F_F816_AS_F4(RMODE, RMODEVAL, FLAG)            \
	_8i32 __attribute__ ((overloadable)) convert_int8##RMODE(float8 x)\
	{\
	_8i32 res;\
	res.lo = float4ToInt4##FLAG(x.lo, RMODEVAL);\
	res.hi = float4ToInt4##FLAG(x.hi, RMODEVAL);\
	return res;\
	}\
	_16i32 __attribute__ ((overloadable)) convert_int16##RMODE(float16 x)\
	{\
	_16i32 res;\
	res.lo.lo = float4ToInt4##FLAG(x.lo.lo, RMODEVAL);\
    res.lo.hi = float4ToInt4##FLAG(x.lo.hi, RMODEVAL);\
	res.hi.lo = float4ToInt4##FLAG(x.hi.lo, RMODEVAL);\
    res.hi.hi = float4ToInt4##FLAG(x.hi.hi, RMODEVAL);\
	return res;\
	}

#define DEF_INT_PROTOI32_F_F816_AS_F8(RMODE, RMODEVAL, FLAG)            \
	_8i32 __attribute__ ((overloadable)) convert_int8##RMODE(float8 x)\
	{\
	_8i32 res;\
	res = float8ToInt8##FLAG(x, RMODEVAL);\
	return res;\
	}\
	_16i32 __attribute__ ((overloadable)) convert_int16##RMODE(float16 x)\
	{\
	_16i32 res;\
	res.lo = float8ToInt8##FLAG(x.lo, RMODEVAL);\
	res.hi = float8ToInt8##FLAG(x.hi, RMODEVAL);\
	return res;\
	}

#if defined(__AVX__)
#define DEF_INT_PROTOI32_F(RMODE, RMODEVAL, FLAG)                       \
    DEF_INT_PROTOI32_F_F1234_AS_F4(RMODE, RMODEVAL, FLAG)               \
    DEF_INT_PROTOI32_F_F816_AS_F8(RMODE, RMODEVAL, FLAG)
#else // defined(__AVX__)
#define DEF_INT_PROTOI32_F(RMODE, RMODEVAL, FLAG)                       \
    DEF_INT_PROTOI32_F_F1234_AS_F4(RMODE, RMODEVAL, FLAG)               \
    DEF_INT_PROTOI32_F_F816_AS_F4(RMODE, RMODEVAL, FLAG)
#endif // defined(__AVX__)

#define DEF_INT_PROTOU32_F_F1234(RMODE, RMODEVAL, FLAG)\
	_1u32 __attribute__ ((overloadable)) convert_uint##RMODE(float x)\
	{\
    _1u32 res = floatToUint##FLAG(x, RMODEVAL);\
	return res;\
	}\
	_2u32 __attribute__ ((overloadable)) convert_uint2##RMODE(float2 x)\
	{\
	_4u32 res;\
	res.s0 = floatToUint##FLAG(x.lo, RMODEVAL);\
	res.s1 = floatToUint##FLAG(x.hi, RMODEVAL);\
	return res.lo;\
	}\
	uint3 __attribute__ ((overloadable)) convert_uint3##RMODE(float3 x)\
	{\
	_4u32 res;\
	res.s0 = floatToUint##FLAG(x.s0, RMODEVAL);\
	res.s1 = floatToUint##FLAG(x.s1, RMODEVAL);\
	res.s2 = floatToUint##FLAG(x.s2, RMODEVAL);\
	return as_uint3(res);\
	}\
	_4u32 __attribute__ ((overloadable)) convert_uint4##RMODE(float4 x)\
	{\
	_4u32 res;\
	res = float4ToUint4##FLAG(x, RMODEVAL);\
	return res;\
	}\

#define DEF_INT_PROTOU32_F_F816_AS_F4(RMODE, RMODEVAL, FLAG)\
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

#define DEF_INT_PROTOU32_F_F816_AS_F8(RMODE, RMODEVAL, FLAG)\
    _8u32 __attribute__ ((overloadable)) convert_uint8##RMODE(float8 x)\
	{\
	_8u32 res;\
	res = float8ToUint8##FLAG(x, RMODEVAL);\
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
#define DEF_INT_PROTOU32_F(RMODE, RMODEVAL, FLAG)\
    DEF_INT_PROTOU32_F_F1234(RMODE, RMODEVAL, FLAG)\
    DEF_INT_PROTOU32_F_F816_AS_F8(RMODE, RMODEVAL, FLAG)
#else // defined(__AVX__)
#define DEF_INT_PROTOU32_F(RMODE, RMODEVAL, FLAG)\
    DEF_INT_PROTOU32_F_F1234(RMODE, RMODEVAL, FLAG)\
    DEF_INT_PROTOU32_F_F816_AS_F4(RMODE, RMODEVAL, FLAG)
#endif // defined(__AVX__)

#define DEF_INT_PROTOI32_D(RMODE, RMODEVAL)\
	_1i32 __attribute__((overloadable)) convert_int##RMODE(double x)\
	{\
	_4i32 res;\
	double2 param;\
	param.lo = x;\
	res = double2ToInt4(param, RMODEVAL);\
	return res.s0;\
	}\
	_2i32 __attribute__((overloadable)) convert_int2##RMODE(double2 x)\
	{\
	_4i32 res;\
	res = double2ToInt4(x, RMODEVAL);\
	return res.lo;\
	}\
	int3 __attribute__((overloadable)) convert_int3##RMODE(double3 x)\
	{\
	_4i32 t1, t2, res;\
	t1 = double2ToInt4(x.lo, RMODEVAL);\
	t2 = double2ToInt4(x.hi, RMODEVAL);\
	res = _mm_unpacklo_epi64(t1, t2);\
	return as_int3(res);\
	}\
	_4i32 __attribute__((overloadable)) convert_int4##RMODE(double4 x)\
	{\
	_4i32 t1, t2, res;\
	t1 = double2ToInt4(x.lo, RMODEVAL);\
	t2 = double2ToInt4(x.hi, RMODEVAL);\
	res = _mm_unpacklo_epi64(t1, t2);\
	return res;\
	}\
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
    double4 y = {1.0}; /* Use a normal value, to avoid worst case(denormal, NaN,,,) */ \
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
	res =  _mm_cvtep##TI##8_epi64(param);\
	return res;\
	}\
	TONAME##long3 __attribute__ ((overloadable)) convert_##TONAME##long3##RMODE(TINAME##char3 x)\
	{\
	_4##TO##64 res;\
	_16##TI##8 param;\
	param.s0123 = as_##TINAME##char4(x);\
	res.lo =  _mm_cvtep##TI##8_epi64(param);\
	res.hi =  _mm_cvtep##TI##8_epi64(_mm_srli_si128(param, 2));\
	return as_##TONAME##long3(res);\
	}\
	_4##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long4##RMODE(_4##TI##8 x)\
	{\
	_4##TO##64 res;\
	_16##TI##8 param;\
	param.s0123 = x;\
	res.lo =  _mm_cvtep##TI##8_epi64(param);\
	res.hi =  _mm_cvtep##TI##8_epi64(_mm_srli_si128(param, 2));\
	return res;\
	}\
	_8##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long8##RMODE(_8##TI##8 x)\
	{\
	_8##TO##64 res;\
	_16##TI##8 param;\
	param.lo = x;\
	res.lo.lo = _mm_cvtep##TI##8_epi64(param);\
	res.lo.hi = _mm_cvtep##TI##8_epi64(_mm_srli_si128(param, 2));\
	res.hi.lo = _mm_cvtep##TI##8_epi64(_mm_srli_si128(param, 4));\
	res.hi.hi = _mm_cvtep##TI##8_epi64(_mm_srli_si128(param, 6));\
	return res;\
	}\
	_16##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long16##RMODE(_16##TI##8 x)\
	{\
	_16##TO##64 res;\
	res.lo.lo.lo = _mm_cvtep##TI##8_epi64(x);\
	res.lo.lo.hi = _mm_cvtep##TI##8_epi64(_mm_srli_si128(x, 2));\
	res.lo.hi.lo = _mm_cvtep##TI##8_epi64(_mm_srli_si128(x, 4));\
	res.lo.hi.hi = _mm_cvtep##TI##8_epi64(_mm_srli_si128(x, 6));\
	res.hi.lo.lo = _mm_cvtep##TI##8_epi64(_mm_srli_si128(x, 8));\
	res.hi.lo.hi = _mm_cvtep##TI##8_epi64(_mm_srli_si128(x, 10));\
	res.hi.hi.lo = _mm_cvtep##TI##8_epi64(_mm_srli_si128(x, 12));\
	res.hi.hi.hi = _mm_cvtep##TI##8_epi64(_mm_srli_si128(x, 14));\
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
	res = _mm_cvtep##TI##16_epi64(param);\
	return res;\
	}\
	TONAME##long3 __attribute__ ((overloadable)) convert_##TONAME##long3##RMODE(TINAME##short3 x)\
	{\
	_8##TI##16 param;\
	_4##TO##64 res;\
	param.lo =as_##TINAME##short4(x);\
	res.lo = _mm_cvtep##TI##16_epi64(param);\
	res.hi = _mm_cvtep##TI##16_epi64(_mm_srli_si128(param, 4));\
	return as_##TONAME##long3(res);\
	}\
	_4##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long4##RMODE(_4##TI##16 x)\
	{\
	_8##TI##16 param;\
	_4##TO##64 res;\
	param.lo = x;\
	res.lo = _mm_cvtep##TI##16_epi64(param);\
	res.hi = _mm_cvtep##TI##16_epi64(_mm_srli_si128(param, 4));\
	return res;\
	}\
	_8##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long8##RMODE(_8##TI##16 x)\
	{\
	_8##TO##64 res;\
	res.lo.lo = _mm_cvtep##TI##16_epi64(x);\
	res.lo.hi = _mm_cvtep##TI##16_epi64(_mm_srli_si128(x, 4));\
	res.hi.lo = _mm_cvtep##TI##16_epi64(_mm_srli_si128(x, 8));\
	res.hi.hi = _mm_cvtep##TI##16_epi64(_mm_srli_si128(x, 12));\
	return res;\
	}\
	_16##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long16##RMODE(_16##TI##16 x)\
	{\
	_16##TO##64 res;\
	res.lo.lo.lo = _mm_cvtep##TI##16_epi64(x.lo);\
	res.lo.lo.hi = _mm_cvtep##TI##16_epi64(_mm_srli_si128(x.lo, 4));\
	res.lo.hi.lo = _mm_cvtep##TI##16_epi64(_mm_srli_si128(x.lo, 8));\
	res.lo.hi.hi = _mm_cvtep##TI##16_epi64(_mm_srli_si128(x.lo, 12));\
	res.hi.lo.lo = _mm_cvtep##TI##16_epi64(x.hi);\
	res.hi.lo.hi = _mm_cvtep##TI##16_epi64(_mm_srli_si128(x.hi, 4));\
	res.hi.hi.lo = _mm_cvtep##TI##16_epi64(_mm_srli_si128(x.hi, 8));\
	res.hi.hi.hi = _mm_cvtep##TI##16_epi64(_mm_srli_si128(x.hi, 12));\
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
	res = _mm_cvtep##TI##32_epi64(param);\
	return res;\
	}\
	TONAME##long3 __attribute__ ((overloadable)) convert_##TONAME##long3##RMODE(TINAME##int3 x)\
	{\
	_4##TO##64 res;\
	res.lo = _mm_cvtep##TI##32_epi64(as_##TONAME##int4(x));\
	res.hi = _mm_cvtep##TI##32_epi64(_mm_srli_si128(as_##TONAME##int4(x), 8));\
	return as_##TONAME##long3(res);\
	}\
	_4##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long4##RMODE(_4##TI##32 x)\
	{\
	_4##TO##64 res;\
	res.lo = _mm_cvtep##TI##32_epi64(x);\
	res.hi = _mm_cvtep##TI##32_epi64(_mm_srli_si128(x, 8));\
	return res;\
	}\
	_8##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long8##RMODE(_8##TI##32 x)\
	{\
	_8##TO##64 res;\
	res.lo.lo = _mm_cvtep##TI##32_epi64(x.lo);\
	res.lo.hi = _mm_cvtep##TI##32_epi64(_mm_srli_si128(x.lo, 8));\
	res.hi.lo = _mm_cvtep##TI##32_epi64(x.hi);\
	res.hi.hi = _mm_cvtep##TI##32_epi64(_mm_srli_si128(x.hi, 8));\
	return res;\
	}\
	_16##TO##64 __attribute__ ((overloadable)) convert_##TONAME##long16##RMODE(_16##TI##32 x)\
	{\
	_16##TO##64 res;\
	res.lo.lo.lo = _mm_cvtep##TI##32_epi64(x.lo.lo);\
	res.lo.lo.hi = _mm_cvtep##TI##32_epi64(_mm_srli_si128(x.lo.lo, 8));\
	res.lo.hi.lo = _mm_cvtep##TI##32_epi64(x.lo.hi);\
	res.lo.hi.hi = _mm_cvtep##TI##32_epi64(_mm_srli_si128(x.lo.hi, 8));\
	res.hi.lo.lo = _mm_cvtep##TI##32_epi64(x.hi.lo);\
	res.hi.lo.hi = _mm_cvtep##TI##32_epi64(_mm_srli_si128(x.hi.lo, 8));\
	res.hi.hi.lo = _mm_cvtep##TI##32_epi64(x.hi.hi);\
	res.hi.hi.hi = _mm_cvtep##TI##32_epi64(_mm_srli_si128(x.hi.hi, 8));\
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
    _4##TO##64 res = {0};\
    float4 ftmp = {1.0};\
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
    _4##TO##64 res = {0};\
    double4 ftmp = {1.0};\
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
	float4 res = _mm_cvtepi32_ps(t);\
	return res.lo;\
	}\
	float3 __attribute__ ((overloadable)) convert_float3##RMODE(TINAME##char3 x)\
	{\
	TINAME##char4 y = as_##TINAME##char4(x);\
	_4i32 t = convert_int4##RMODE(y);\
	float4 res = _mm_cvtepi32_ps(t);\
	return as_float3(res);\
	}\
	float4 __attribute__ ((overloadable)) convert_float4##RMODE(_4##TI##8 x)\
	{\
	_4i32 t = convert_int4##RMODE(x);\
	float4 res = _mm_cvtepi32_ps(t);\
	return res;\
	}\
	float8 __attribute__ ((overloadable)) convert_float8##RMODE(_8##TI##8 x)\
	{\
	float8 res;\
	_8i32 t = convert_int8##RMODE(x);\
	res.lo = _mm_cvtepi32_ps(t.lo);\
	res.hi = _mm_cvtepi32_ps(t.hi);\
	return res;\
	}\
	float16 __attribute__ ((overloadable)) convert_float16##RMODE(_16##TI##8 x)\
	{\
	float16 res;\
	_16i32 t = convert_int16##RMODE(x);\
	res.lo.lo = _mm_cvtepi32_ps(t.lo.lo);\
	res.lo.hi = _mm_cvtepi32_ps(t.lo.hi);\
	res.hi.lo = _mm_cvtepi32_ps(t.hi.lo);\
	res.hi.hi = _mm_cvtepi32_ps(t.hi.hi);\
	return res;\
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
	float4 res = _mm_cvtepi32_ps(t);\
	return res.lo;\
	}\
	float3 __attribute__ ((overloadable)) convert_float3##RMODE(TINAME##short3 x)\
	{\
	TINAME##short4 y = as_##TINAME##short4(x);\
	_4i32 t = convert_int4##RMODE(y);\
	float4 res = _mm_cvtepi32_ps(t);\
	return as_float3(res);\
	}\
	float4 __attribute__ ((overloadable)) convert_float4##RMODE(_4##TI##16 x)\
	{\
	_4i32 t = convert_int4##RMODE(x);\
	float4 res = _mm_cvtepi32_ps(t);\
	return res;\
	}\
	float8 __attribute__ ((overloadable)) convert_float8##RMODE(_8##TI##16 x)\
	{\
	float8 res;\
	_8i32 t = convert_int8##RMODE(x);\
	res.lo = _mm_cvtepi32_ps(t.lo);\
	res.hi = _mm_cvtepi32_ps(t.hi);\
	return res;\
	}\
	float16 __attribute__ ((overloadable)) convert_float16##RMODE(_16##TI##16 x)\
	{\
	float16 res;\
	_16i32 t = convert_int16##RMODE(x);\
	res.lo.lo = _mm_cvtepi32_ps(t.lo.lo);\
	res.lo.hi = _mm_cvtepi32_ps(t.lo.hi);\
	res.hi.lo = _mm_cvtepi32_ps(t.hi.lo);\
	res.hi.hi = _mm_cvtepi32_ps(t.hi.hi);\
	return res;\
	}\

#define DEF_INT_PROTOF_I32_F816_AS_F4(RMODE, RMODEVAL, FLAG)\
	float8 __attribute__ ((overloadable)) convert_float8##RMODE(_8i32 x)\
	{\
	float8 res;\
	res.lo = intToFloat##FLAG(x.lo, RMODEVAL);\
	res.hi = intToFloat##FLAG(x.hi, RMODEVAL);\
	return res;\
	}\
	float16 __attribute__ ((overloadable)) convert_float16##RMODE(_16i32 x)\
	{\
	float16 res;\
	res.lo.lo = intToFloat##FLAG(x.lo.lo, RMODEVAL);\
	res.lo.hi = intToFloat##FLAG(x.lo.hi, RMODEVAL);\
	res.hi.lo = intToFloat##FLAG(x.hi.lo, RMODEVAL);\
	res.hi.hi = intToFloat##FLAG(x.hi.hi, RMODEVAL);\
	return res;\
	}

#define DEF_INT_PROTOF_I32_F816_AS_F8(RMODE, RMODEVAL, FLAG)\
	float8 __attribute__ ((overloadable)) convert_float8##RMODE(_8i32 x)\
	{\
	float8 res;\
	res = (float8) intToFloat8##FLAG(x, RMODEVAL);\
	return res;\
	}\
	float16 __attribute__ ((overloadable)) convert_float16##RMODE(_16i32 x)\
	{\
	float16 res;\
	res.lo = intToFloat8##FLAG(x.lo, RMODEVAL);\
	res.hi = intToFloat8##FLAG(x.hi, RMODEVAL);\
	return res;\
	}

#define DEF_INT_PROTOF_I32_F1234_AS_F4(RMODE, RMODEVAL, FLAG)\
	float __attribute__ ((overloadable)) convert_float##RMODE(_1i32 x)\
	{\
	_4i32 param;\
	param.s0 = x;\
	float4 res =  intToFloat##FLAG(param, RMODEVAL);\
	return res.s0;\
	}\
	float2 __attribute__ ((overloadable)) convert_float2##RMODE(_2i32 x)\
	{\
	_4i32 param;\
	param.lo = x;\
	float4 res = intToFloat##FLAG(param, RMODEVAL);\
	return res.lo;\
	}\
	float3 __attribute__ ((overloadable)) convert_float3##RMODE(int3 x)\
	{\
	float4 res;\
	_4i32 param;\
	param.s012 = x;\
	res = intToFloat##FLAG(param, RMODEVAL);\
	return res.s012;\
	}\
	float4 __attribute__ ((overloadable)) convert_float4##RMODE(_4i32 x)\
	{\
	return intToFloat##FLAG(x, RMODEVAL);\
	}

#if defined(__AVX__)
#define DEF_INT_PROTOF_I32(RMODE, RMODEVAL, FLAG)           \
    DEF_INT_PROTOF_I32_F1234_AS_F4(RMODE, RMODEVAL, FLAG)   \
    DEF_INT_PROTOF_I32_F816_AS_F8(RMODE, RMODEVAL, FLAG)
#else // #if defined(__AVX__)
#define DEF_INT_PROTOF_I32(RMODE, RMODEVAL, FLAG)           \
    DEF_INT_PROTOF_I32_F1234_AS_F4(RMODE, RMODEVAL, FLAG)   \
    DEF_INT_PROTOF_I32_F816_AS_F4(RMODE, RMODEVAL, FLAG)
#endif // #if defined(__AVX__)

#define DEF_INT_PROTOF_U32(RMODE, RMODEVAL, FLAG)\
	float __attribute__ ((overloadable)) convert_float##RMODE(_1u32 x)\
	{\
	float res = uintToFloat##FLAG(x, RMODEVAL);\
	return res;\
	}\
	float2 __attribute__ ((overloadable)) convert_float2##RMODE(_2u32 x)\
	{\
	float2 res;\
	res.lo = uintToFloat##FLAG(x.lo, RMODEVAL);\
	res.hi = uintToFloat##FLAG(x.hi, RMODEVAL);\
	return res;\
	}\
	float3 __attribute__ ((overloadable)) convert_float3##RMODE(uint3 x)\
	{\
	float3 res;\
	res.s0 = uintToFloat##FLAG(x.s0, RMODEVAL);\
	res.s1 = uintToFloat##FLAG(x.s1, RMODEVAL);\
	res.s2 = uintToFloat##FLAG(x.s2, RMODEVAL);\
	return res;\
	}\
	float4 __attribute__ ((overloadable)) convert_float4##RMODE(_4u32 x)\
	{\
	float4 res;\
	res.lo = convert_float2##RMODE(x.lo);\
	res.hi = convert_float2##RMODE(x.hi);\
	return res;\
	}\
	float8 __attribute__ ((overloadable)) convert_float8##RMODE(_8u32 x)\
	{\
	float8 res;\
	res.lo = convert_float4##RMODE(x.lo);\
	res.hi = convert_float4##RMODE(x.hi);\
	return res;\
	}\
	float16 __attribute__ ((overloadable)) convert_float16##RMODE(_16u32 x)\
	{\
	float16 res;\
	res.lo.lo = convert_float4##RMODE(x.lo.lo);\
	res.lo.hi = convert_float4##RMODE(x.lo.hi);\
	res.hi.lo = convert_float4##RMODE(x.hi.lo);\
	res.hi.hi = convert_float4##RMODE(x.hi.hi);\
	return res;\
	}\

#define DEF_INT_PROTOF_64(TI, TINAME, RMODE, RSVML, CPUTYPE)\
	float __attribute__((overloadable)) convert_float##RMODE(_1##TI##64 x)\
	{\
	float res = __ocl_svml_##CPUTYPE##_cvt##TI##64tofp##RSVML##f1(x);\
	return res;\
	}\
	float2 __attribute__((overloadable)) convert_float2##RMODE(_2##TI##64 x)\
	{\
    float4 res = {0};\
    _4##TI##64 ftmp = {0};\
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
	}\

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

#define DEF_INT_PROTOF_D_D12_AS_D2(RMODE, RMODEVAL, FLAG)\
	float __attribute__((overloadable)) convert_float##RMODE(double x)\
	{\
	double2 param;\
    param.lo = x;\
    float4 res = double2ToFloat4##FLAG( param, RMODEVAL);\
	return res.s0;\
	}\
	float2 __attribute__((overloadable)) convert_float2##RMODE(double2 x)\
	{\
	float4 res;\
	res = double2ToFloat4##FLAG( x, RMODEVAL);\
	return res.lo;\
	}
	
#define DEF_INT_PROTOF_D_D34816_AS_D2(RMODE, RMODEVAL, FLAG)\
    float3 __attribute__((overloadable)) convert_float3##RMODE(double3 x)\
	{\
	float4 res, t;\
	double4 y = as_double4(x);\
    t = double2ToFloat4##FLAG( y.lo, RMODEVAL);\
    res.lo = t.lo;\
    t = double2ToFloat4##FLAG( y.hi, RMODEVAL);\
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

#define DEF_INT_PROTOF_D_D34816_AS_D4(RMODE, RMODEVAL, FLAG)\
    float3 __attribute__((overloadable)) convert_float3##RMODE(double3 x)\
	{\
	float4 res, t;\
	double4 y = as_double4(x);\
    res = double4ToFloat4##FLAG( y, RMODEVAL);\
    return as_float3(res);\
	}\
    float4 __attribute__((overloadable)) convert_float4##RMODE(double4 x)\
	{\
	float4 res;\
    res = double4ToFloat4##FLAG( x, RMODEVAL);\
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
#define DEF_INT_PROTOF_D(RMODE, RMODEVAL, FLAG)\
    DEF_INT_PROTOF_D_D12_AS_D2(RMODE, RMODEVAL, FLAG)\
    DEF_INT_PROTOF_D_D34816_AS_D4(RMODE, RMODEVAL, FLAG)
#else // defined(__AVX__)
#define DEF_INT_PROTOF_D(RMODE, RMODEVAL, FLAG)\
    DEF_INT_PROTOF_D_D12_AS_D2(RMODE, RMODEVAL, FLAG)\
    DEF_INT_PROTOF_D_D34816_AS_D2(RMODE, RMODEVAL, FLAG)
#endif // defined(__AVX__)

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
	double2 res = _mm_cvtepi32_pd(t);\
	return res;\
	}\
	double3 __attribute__((overloadable)) convert_double3##RMODE(TINAME##char3 x)\
	{\
	TINAME##char4 y = as_##TINAME##char4(x);\
	_4i32 t = convert_int4##RMODE(y);\
	double4 res;\
	res.lo = _mm_cvtepi32_pd(t);\
	t = _mm_srli_si128(t, 8);\
	res.hi = _mm_cvtepi32_pd(t);\
	return as_double3(res);\
	}\
	double4 __attribute__((overloadable)) convert_double4##RMODE(_4##TI##8 x)\
	{\
	_4i32 t = convert_int4##RMODE(x);\
	double4 res;\
	res.lo = _mm_cvtepi32_pd(t);\
	t = _mm_srli_si128(t, 8);\
	res.hi = _mm_cvtepi32_pd(t);\
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
	res = _mm_cvtepi32_pd(t);\
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
	res =  _mm_cvtepi32_pd(param);\
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
	res = _mm_cvtepi32_pd(param);\
	setRound(rm);\
	return res;\
	}\
	double3 __attribute__((overloadable)) convert_double3##RMODE(int3 x)\
	{\
	double4 res;\
	int rm = getRound();\
	int4 y = as_int4(x);\
	setRound((rm& ~_MM_ROUND_MASK) | RMODEVAL);\
	res.lo = _mm_cvtepi32_pd(y);\
	y = _mm_srli_si128(y, 8);\
	res.hi = _mm_cvtepi32_pd(y);\
	setRound(rm);\
	return as_double3(res);\
	}\
	double4 __attribute__((overloadable)) convert_double4##RMODE(_4i32 x)\
	{\
	double4 res;\
	int rm = getRound();\
	setRound((rm& ~_MM_ROUND_MASK) | RMODEVAL);\
	res.lo = _mm_cvtepi32_pd(x);\
	x = _mm_srli_si128(x, 8);\
	res.hi = _mm_cvtepi32_pd(x);\
	setRound(rm);\
	return res;\
	}\
	double8 __attribute__((overloadable)) convert_double8##RMODE(_8i32 x)\
	{\
	double8 res;\
	int rm = getRound();\
	setRound((rm& ~_MM_ROUND_MASK) | RMODEVAL);\
	res.lo.lo = _mm_cvtepi32_pd(x.lo);\
	x.lo = _mm_srli_si128(x.lo, 8);\
	res.lo.hi = _mm_cvtepi32_pd(x.lo);\
	res.hi.lo = _mm_cvtepi32_pd(x.hi);\
	x.hi = _mm_srli_si128(x.hi, 8);\
	res.hi.hi = _mm_cvtepi32_pd(x.hi);\
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
DEF_INT_PROTOD_U32_WRAPPER_DECL(RMODE,RSVML,CPUTYPE)\
	double __attribute__((overloadable)) convert_double##RMODE(_1u32 x)\
	{\
    double res = CONVERT_DOUBLE_WRAPPER_FUNCNAME(RSVML,CPUTYPE,u)(x);\
	return res;\
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
	

#define DEF_INT_PROTOD_64(TI, TINAME, RMODE, RSVML, CPUTYPE)\
	double __attribute__((overloadable)) convert_double##RMODE(_1##TI##64 x)\
	{\
	double res = __ocl_svml_##CPUTYPE##_cvt##TI##64tofp##RSVML##1(x);\
	return res;\
	}\
	double2 __attribute__((overloadable)) convert_double2##RMODE(_2##TI##64 x)\
	{\
    _4##TI##64 param = {0};\
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
	res.lo = _mm_cvtps_pd(y);\
	y = _mm_srli_si128(y, 8);\
	res.hi = _mm_cvtps_pd(y);\
	return as_double3(res);\
	}\
	double4 __attribute__((overloadable)) convert_double4##RMODE(float4 x)\
	{\
	double4 res;\
	res.lo = _mm_cvtps_pd(x);\
	x = _mm_srli_si128(x, 8);\
	res.hi = _mm_cvtps_pd(x);\
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
	res = _mm_pack##TONAME##s_epi16(param, param);\
	return res.s01;\
	}\
	TONAME##char3 __attribute__ ((overloadable)) convert_##TONAME##char3_sat##RMODE(short3 x)\
	{\
	_16##TO##8 res;\
	_8i16 param;\
	param.lo = as_short4(x);\
	res = _mm_pack##TONAME##s_epi16(param, param);\
	return as_##TONAME##char3(res.s0123);\
	}\
	_4##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char4_sat##RMODE(_4i16 x)\
	{\
	_16##TO##8 res;\
	_8i16 param;\
	param.lo = x;\
	res = _mm_pack##TONAME##s_epi16(param, param);\
	return res.s0123;\
	}\
	_8##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char8_sat##RMODE(_8i16 x)\
	{\
	_16##TO##8 res;\
	res = _mm_pack##TONAME##s_epi16(x, x);\
	return res.lo;\
	}\
	_16##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char16_sat##RMODE(_16i16 x)\
	{\
	_16##TO##8 res, temp1, temp2;\
	res = _mm_pack##TONAME##s_epi16(x.lo, x.hi);\
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
	res = _mm_packs_epi32(param, param);\
	res = _mm_pack##TONAME##s_epi16(res, res);\
	return res.s01;\
	}\
	TONAME##char3 __attribute__ ((overloadable)) convert_##TONAME##char3_sat##RMODE(int3 x)\
	{\
	_16##TO##8 res;\
	int4 y = as_int4(x);\
	res = _mm_packs_epi32(y, y);\
	res = _mm_pack##TONAME##s_epi16(res, res);\
	return as_##TONAME##char3(res.s0123);\
	}\
	_4##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char4_sat##RMODE(_4i32 x)\
	{\
	_16##TO##8 res;\
	res = _mm_packs_epi32(x, x);\
	res = _mm_pack##TONAME##s_epi16(res, res);\
	return res.s0123;\
	}\
	_8##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char8_sat##RMODE(_8i32 x)\
	{\
	_16##TO##8 res;\
	res = _mm_packs_epi32(x.lo, x.hi);\
	res = _mm_pack##TONAME##s_epi16(res, res);\
	return res.lo;\
	}\
	_16##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char16_sat##RMODE(_16i32 x)\
	{\
	_16##TO##8 res, temp1, temp2;\
	temp1 = _mm_packs_epi32(x.lo.lo, x.lo.hi);\
	temp2 = _mm_packs_epi32(x.hi.lo, x.hi.hi);\
	res   = _mm_pack##TONAME##s_epi16(temp1, temp2);\
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

#define DEF_SAT_PROTO8_F(TO, TONAME, RMODE, RMODEVAL, MAX, MIN, FLAG)\
	_1##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char_sat##RMODE(float x)\
	{\
	_1##TO##8 res;\
	if(x > MAX) return MAX;\
	if(x < MIN) return MIN;\
	float4 param;\
	param.s0 = x;\
	return floatToInt##FLAG(param, RMODEVAL);\
	}\
	_2##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char2_sat##RMODE(float2 x)\
	{\
	_16##TO##8 res;\
	_4i32 t;\
	float4 param;\
	param.lo = x;\
	t = floatToIntSat##FLAG(param, RMODEVAL);\
	res.s0123 =  convert_##TONAME##char4_sat(t);\
	return res.s01;\
	}\
	TONAME##char3 __attribute__ ((overloadable)) convert_##TONAME##char3_sat##RMODE(float3 x)\
	{\
	_4##TO##8 res;\
	_4i32 t;\
	t = floatToIntSat##FLAG(as_float4(x), RMODEVAL);\
	res =  convert_##TONAME##char4_sat(t);\
	return as_##TONAME##char3(res);\
	}\
	_4##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char4_sat##RMODE(float4 x)\
	{\
	_4##TO##8 res;\
	_4i32 t;\
	t = floatToIntSat##FLAG(x, RMODEVAL);\
	res =  convert_##TONAME##char4_sat(t);\
	return res;\
	}\
	_8##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char8_sat##RMODE(float8 x)\
	{\
	_16##TO##8 res;\
	_4i32 t1 = floatToIntSat##FLAG(x.lo, RMODEVAL);\
	_4i32 t2 = floatToIntSat##FLAG(x.hi, RMODEVAL);\
	res.s0123 =  convert_##TONAME##char4_sat(t1);\
	res.s4567 =  convert_##TONAME##char4_sat(t2);\
	return res.lo;\
	}\
	_16##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char16_sat##RMODE(float16 x)\
	{\
	_16##TO##8 res;\
	res.lo = convert_##TONAME##char8_sat##RMODE(x.lo);\
	res.hi = convert_##TONAME##char8_sat##RMODE(x.hi);\
	return res;\
	}\

#define DEF_SAT_PROTO8_D(TO, TONAME, RMODE, RMODEVAL)\
	_1##TO##8 __attribute__((overloadable)) convert_##TONAME##char_sat##RMODE(double x)\
	{\
	int t = doubleToIntSat(x, RMODEVAL);\
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
	tmp = _mm_cvtep##TI##8_epi16(param);\
	res = convert_##TONAME##short8##RMODE(tmp);\
	return res.s01;\
	}\
	TONAME##short3 __attribute__ ((overloadable)) convert_##TONAME##short3##RMODE(TINAME##char3 x)\
	{\
	_8##TO##16 res;\
	_16##TI##8 param;\
	_8##TI##16 tmp;\
	param.s012 = x;\
	tmp = _mm_cvtep##TI##8_epi16(param);\
	res = convert_##TONAME##short8##RMODE(tmp);\
	return res.s012;\
	}\
	_4##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short4##RMODE(_4##TI##8 x)\
	{\
	_8##TO##16 res;\
	_16##TI##8 param;\
	_8##TI##16 tmp;\
	param.s0123 = x;\
	tmp = _mm_cvtep##TI##8_epi16(param);\
	res = convert_##TONAME##short8##RMODE(tmp);\
	return res.s0123;\
	}\
	_8##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short8##RMODE(_8##TI##8 x)\
	{\
	_8##TO##16 res;\
	_16##TI##8 param;\
	_8##TI##16 tmp;\
	param.lo = x;\
	tmp = _mm_cvtep##TI##8_epi16(param);\
	res = convert_##TONAME##short8##RMODE(tmp);\
	return res;\
	}\
	_16##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short16##RMODE(_16##TI##8 x)\
	{\
	_16##TO##16 res;\
	_8##TI##16 tmp;\
	tmp = _mm_cvtep##TI##8_epi16(x);\
	res.lo = convert_##TONAME##short8##RMODE(tmp);\
	tmp = _mm_cvtep##TI##8_epi16(_mm_srli_si128(x, 8));\
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
	res = _mm_pack##TONAME##s_epi32(param, param);\
	return res.s01;\
	}\
	TONAME##short3 __attribute__ ((overloadable)) convert_##TONAME##short3_sat##RMODE(int3 x)\
	{\
	_8##TO##16 res;\
	int4 y = as_int4(x);\
	res = _mm_pack##TONAME##s_epi32(y, y);\
	return as_##TONAME##short3(res.lo);\
	}\
	_4##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short4_sat##RMODE(_4i32 x)\
	{\
	_8##TO##16 res;\
	res = _mm_pack##TONAME##s_epi32(x, x);\
	return res.lo;\
	}\
	_8##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short8_sat##RMODE(_8i32 x)\
	{\
	_8##TO##16 res;\
	res = _mm_pack##TONAME##s_epi32(x.lo, x.hi);\
	return res;\
	}\
	_16##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short16_sat##RMODE(_16i32 x)\
	{\
	_16##TO##16 res;\
	res.lo = _mm_pack##TONAME##s_epi32(x.lo.lo, x.lo.hi);\
	res.hi = _mm_pack##TONAME##s_epi32(x.hi.lo, x.hi.hi);\
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
	_4i32 t = floatToIntSat##FLAG(param, RMODEVAL);\
	res.lo = convert_##TONAME##short4_sat##RMODE(t);\
	return res.s0;\
	}\
	_2##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short2_sat##RMODE(float2 x)\
	{\
	_8##TO##16 res;\
	float4 param;\
	param.lo = x;\
	_4i32 t = floatToIntSat##FLAG(param, RMODEVAL);\
	res.lo = convert_##TONAME##short4_sat##RMODE(t);\
	return res.s01;\
	}\
	TONAME##short3 __attribute__ ((overloadable)) convert_##TONAME##short3_sat##RMODE(float3 x)\
	{\
	_8##TO##16 res;\
	_4i32 t = floatToIntSat##FLAG(as_float4(x), RMODEVAL);\
	res.lo = convert_##TONAME##short4_sat##RMODE(t);\
	return as_##TONAME##short3(res.lo);\
	}\
	_4##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short4_sat##RMODE(float4 x)\
	{\
	_8##TO##16 res;\
	_4i32 t = floatToIntSat##FLAG(x, RMODEVAL);\
	res.lo = convert_##TONAME##short4_sat##RMODE(t);\
	return res.lo;\
	}\
	_8##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short8_sat##RMODE(float8 x)\
	{\
	_8##TO##16 res;\
	_4i32 t1 = floatToIntSat##FLAG(x.lo, RMODEVAL);\
	_4i32 t2 = floatToIntSat##FLAG(x.hi, RMODEVAL);\
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
	}\


#define DEF_SAT_PROTO16_D(TO, TONAME, RMODE, RMODEVAL)\
	_1##TO##16 __attribute__((overloadable)) convert_##TONAME##short_sat##RMODE(double x)\
	{\
	_1##TO##16 res;\
	int t = doubleToIntSat(x, RMODEVAL);\
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
	}\


#define DEF_SAT_PROTOI32_F(RMODE, RMODEVAL, MAX, MIN, FLAG)\
	_1i32 __attribute__ ((overloadable)) convert_int_sat##RMODE(float x)\
	{\
	_1i32 res;\
	if(x >= MAX) return MAX;\
	if(x <= MIN) return MIN;\
	float4 p;\
	p.s0 = x;\
	res = floatToInt##FLAG(p, RMODEVAL);\
	return res;\
	}\
	_2i32 __attribute__ ((overloadable)) convert_int2_sat##RMODE(float2 x)\
	{\
	_4i32 res;\
	float4 param;\
	param.lo = x;\
	res = floatToIntSat##FLAG(param, RMODEVAL);\
	return res.lo;\
	}\
	int3 __attribute__ ((overloadable)) convert_int3_sat##RMODE(float3 x)\
	{\
	_4i32 res;\
	res = floatToIntSat##FLAG(as_float4(x), RMODEVAL);\
	return as_int3(res);\
	}\
	_4i32 __attribute__ ((overloadable)) convert_int4_sat##RMODE(float4 x)\
	{\
	_4i32 res;\
	res = floatToIntSat##FLAG(x, RMODEVAL);\
	return res;\
	}\
	_8i32 __attribute__ ((overloadable)) convert_int8_sat##RMODE(float8 x)\
	{\
	_8i32 res;\
	res.lo = floatToIntSat##FLAG(x.lo, RMODEVAL);\
	res.hi = floatToIntSat##FLAG(x.hi, RMODEVAL);\
	return res;\
	}\
	_16i32 __attribute__ ((overloadable)) convert_int16_sat##RMODE(float16 x)\
	{\
	_16i32 res;\
	res.lo.lo = floatToIntSat##FLAG(x.lo.lo, RMODEVAL);\
	res.lo.hi = floatToIntSat##FLAG(x.lo.hi, RMODEVAL);\
	res.hi.lo = floatToIntSat##FLAG(x.hi.lo, RMODEVAL);\
	res.hi.hi = floatToIntSat##FLAG(x.hi.hi, RMODEVAL);\
	return res;\
	}



#define DEF_SAT_PROTOI32_D(RMODE, RMODEVAL)\
	_1i32 __attribute__((overloadable)) convert_int_sat##RMODE(double x)\
	{\
		return doubleToIntSat(x, RMODEVAL);\
	}\
	_2i32 __attribute__((overloadable)) convert_int2_sat##RMODE(double2 x)\
	{\
	_2i32 res;\
	res.lo = doubleToIntSat(x.lo, RMODEVAL);\
	res.hi = doubleToIntSat(x.hi, RMODEVAL);\
	return res;\
	}\
	int3 __attribute__((overloadable)) convert_int3_sat##RMODE(double3 x)\
	{\
	int3 res;\
	res.s01 = convert_int2_sat##RMODE(x.s01);\
	res.s2 = doubleToIntSat(x.s2, RMODEVAL);\
	return res;\
	}\
	_4i32 __attribute__((overloadable)) convert_int4_sat##RMODE(double4 x)\
	{\
	_4i32 res;\
	res.lo = convert_int2_sat##RMODE(x.lo);\
	res.hi = convert_int2_sat##RMODE(x.hi);\
	return res;\
	}\
	_8i32 __attribute__((overloadable)) convert_int8_sat##RMODE(double8 x)\
	{\
	_8i32 res;\
	res.lo = convert_int4_sat##RMODE(x.lo);\
	res.hi = convert_int4_sat##RMODE(x.hi);\
	return res;\
	}\
	_16i32 __attribute__((overloadable)) convert_int16_sat##RMODE(double16 x)\
	{\
	_16i32 res;\
	res.lo = convert_int8_sat##RMODE(x.lo);\
	res.hi = convert_int8_sat##RMODE(x.hi);\
	return res;\
	}

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
	_8##TO##16 t1 = _mm_cmpgt_epi16(_mm_setzero_si128(), param);\
	_8##TO##16 t2 = _mm_cmpgt_epi16(param, *((_8##TO##16*)max##TO##Char16));\
	t1 = _mm_or_si128(t1, t2); \
	res = _mm_and_si128(t1, *((_8##TO##16*)max##TO##Char16) );\
	t1 = _mm_andnot_si128(t1, param);\
	res = _mm_or_si128(t1, res);\
	res = _mm_shuffle_epi8(res, *((_16##TO##8 *) _8x16to8x8));\
	return res.s01;\
	}\
	TONAME##char3 __attribute__ ((overloadable)) convert_##TONAME##char3_sat##RMODE(ushort3 x)\
	{\
	_16##TO##8 res;\
	_8u16 param;\
	param.lo = as_ushort4(x);\
	_8##TO##16 t1 = _mm_cmpgt_epi16(_mm_setzero_si128(), param);\
	_8##TO##16 t2 = _mm_cmpgt_epi16(param, *((_8##TO##16*)max##TO##Char16));\
	t1 = _mm_or_si128(t1, t2); \
	res = _mm_and_si128(t1, *((_8##TO##16*)max##TO##Char16) );\
	t1 = _mm_andnot_si128(t1, param);\
	res = _mm_or_si128(t1, res);\
	res = _mm_shuffle_epi8(res, *((_16##TO##8 *) _8x16to8x8));\
	return res.s012;\
	}\
	_4##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char4_sat##RMODE(_4u16 x)\
	{\
	_16##TO##8 res;\
	_8u16 param;\
	param.lo = x;\
	_8##TO##16 t1 = _mm_cmpgt_epi16(_mm_setzero_si128(), param);\
	_8##TO##16 t2 = _mm_cmpgt_epi16(param, *((_8##TO##16*)max##TO##Char16));\
	t1 = _mm_or_si128(t1, t2); \
	res = _mm_and_si128(t1, *((_8##TO##16*)max##TO##Char16) );\
	t1 = _mm_andnot_si128(t1, param);\
	res = _mm_or_si128(t1, res);\
	res = _mm_shuffle_epi8(res, *((_16##TO##8 *) _8x16to8x8));\
	return res.s0123;\
	}\
	_8##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char8_sat##RMODE(_8u16 x)\
	{\
	_16##TO##8 res;\
	_8u16 t1 = _mm_cmpgt_epi16(_mm_setzero_si128(), x);\
	_8u16 t2 = _mm_cmpgt_epi16(x, *((_8##TO##16*)max##TO##Char16));\
	t1 = _mm_or_si128(t1, t2); \
	res = _mm_and_si128(t1, *((_8##TO##16*)max##TO##Char16) );\
	t1 = _mm_andnot_si128(t1, x);\
	res = _mm_or_si128(t1, res);\
	res = _mm_shuffle_epi8(res, *((_16##TO##8 *) _8x16to8x8));\
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
	_4i32 t1 = _mm_cmpgt_epi32(_mm_setzero_si128(), param);\
	_4i32 t2 = _mm_cmpgt_epi32(param, *((_4##TO##32*)max##TO##Char32));\
	t1 = _mm_or_si128(t1, t2);\
	res = _mm_and_si128(t1, *((_4##TO##32*)max##TO##Char32) );\
	t1 = _mm_andnot_si128(t1, param);\
	res = _mm_or_si128(t1, res);\
	res = _mm_shuffle_epi8(res, *((_4##TO##32 *) _4x32to4x8));\
	return res.s01;\
	}\
	TONAME##char3 __attribute__ ((overloadable)) convert_##TONAME##char3_sat##RMODE(uint3 x)\
	{\
	_16##TO##8 res;\
	uint4 y = as_uint4(x);\
	_4i32 t1 = _mm_cmpgt_epi32(_mm_setzero_si128(), y);\
	_4i32 t2 = _mm_cmpgt_epi32(y, *((_4##TO##32*)max##TO##Char32));\
	t1 = _mm_or_si128(t1, t2);\
	res = _mm_and_si128(t1, *((_4##TO##32*)max##TO##Char32) );\
	t1 = _mm_andnot_si128(t1, y);\
	res = _mm_or_si128(t1, res);\
	res = _mm_shuffle_epi8(res, *((_4##TO##32 *) _4x32to4x8));\
	return res.s012;\
	}\
	_4##TO##8 __attribute__ ((overloadable)) convert_##TONAME##char4_sat##RMODE(_4u32 x)\
	{\
	_16##TO##8 res;\
	_4i32 t1 = _mm_cmpgt_epi32(_mm_setzero_si128(), x);\
	_4i32 t2 = _mm_cmpgt_epi32(x, *((_4##TO##32*)max##TO##Char32));\
	t1 = _mm_or_si128(t1, t2);\
	res = _mm_and_si128(t1, *((_4##TO##32*)max##TO##Char32) );\
	t1 = _mm_andnot_si128(t1, x);\
	res = _mm_or_si128(t1, res);\
	res = _mm_shuffle_epi8(res, *((_4##TO##32 *) _4x32to4x8));\
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
	_4##TO##32 t1 = _mm_cmpgt_epi32(_mm_setzero_si128(), param);\
	_4##TO##32 t2 = _mm_cmpgt_epi32(param, *((_4##TO##32*)max##TO##Short32));\
	t1 = _mm_or_si128(t1, t2); \
	res = _mm_and_si128(t1, *((_4##TO##32*)max##TO##Short32) );\
	t1 = _mm_andnot_si128(t1, param);\
	res = _mm_or_si128(t1, res);\
	res = _mm_shuffle_epi8(res, *((__m128i *) _4x32to4x16));\
	return res.s01;\
	}\
	TONAME##short3 __attribute__ ((overloadable)) convert_##TONAME##short3_sat##RMODE(uint3 x)\
	{\
	_8##TO##16 res;\
	uint4 y = as_uint4(x);\
	_4##TO##32 t1 = _mm_cmpgt_epi32(_mm_setzero_si128(), y);\
	_4##TO##32 t2 = _mm_cmpgt_epi32(y, *((_4##TO##32*)max##TO##Short32));\
	t1 = _mm_or_si128(t1, t2); \
	res = _mm_and_si128(t1, *((_4##TO##32*)max##TO##Short32) );\
	t1 = _mm_andnot_si128(t1, y);\
	res = _mm_or_si128(t1, res);\
	res = _mm_shuffle_epi8(res, *((__m128i *) _4x32to4x16));\
	return res.s012;\
	}\
	_4##TO##16 __attribute__ ((overloadable)) convert_##TONAME##short4_sat##RMODE(_4u32 x)\
	{\
	_8##TO##16 res;\
	_4##TO##32 t1 = _mm_cmpgt_epi32(_mm_setzero_si128(), x);\
	_4##TO##32 t2 = _mm_cmpgt_epi32(x, *((_4##TO##32*)max##TO##Short32));\
	t1 = _mm_or_si128(t1, t2); \
	res = _mm_and_si128(t1, *((_4##TO##32*)max##TO##Short32) );\
	t1 = _mm_andnot_si128(t1, x);\
	res = _mm_or_si128(t1, res);\
	res = _mm_shuffle_epi8(res, *((__m128i *) _4x32to4x16));\
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
	res =  _mm_cvtep##TI##8_epi32(param);\
	res = convert_##TONAME##int4##RMODE((_4##TI##32)res);\
	return res.lo;\
	}\
	TONAME##int3 __attribute__ ((overloadable)) convert_##TONAME##int3##RMODE(TINAME##char3 x)\
	{\
	_4##TO##32 res;\
	_16##TI##8 param;\
	param.s012 = x;\
	res =  _mm_cvtep##TI##8_epi32(param);\
	res = convert_##TONAME##int4##RMODE((_4##TI##32)res);\
	return res.s012;\
	}\
	_4##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int4##RMODE(_4##TI##8 x)\
	{\
	_4##TO##32 res;\
	_16##TI##8 param;\
	param.s0123 = x;\
	res =  _mm_cvtep##TI##8_epi32(param);\
	res = convert_##TONAME##int4##RMODE((_4##TI##32)res);\
	return res;\
	}\
	_8##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int8##RMODE(_8##TI##8 x)\
	{\
	_8##TO##32 res;\
	_16##TI##8 param;\
	param.lo = x;\
	res.lo = _mm_cvtep##TI##8_epi32(param);\
	res.lo = convert_##TONAME##int4##RMODE((_4##TI##32)res.lo);\
	res.hi = _mm_cvtep##TI##8_epi32(_mm_srli_si128(param, 4));\
	res.hi = convert_##TONAME##int4##RMODE((_4##TI##32)res.hi);\
	return res;\
	}\
	_16##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int16##RMODE(_16##TI##8 x)\
	{\
	_16##TO##32 res;\
	res.lo.lo = _mm_cvtep##TI##8_epi32(x);\
	res.lo.lo = convert_##TONAME##int4##RMODE((_4##TI##32)res.lo.lo);\
	res.lo.hi = _mm_cvtep##TI##8_epi32(_mm_srli_si128(x, 4));\
	res.lo.hi = convert_##TONAME##int4##RMODE((_4##TI##32)res.lo.hi);\
	res.hi.lo = _mm_cvtep##TI##8_epi32(_mm_srli_si128(x, 8));\
	res.hi.lo = convert_##TONAME##int4##RMODE((_4##TI##32)res.hi.lo);\
	res.hi.hi = _mm_cvtep##TI##8_epi32(_mm_srli_si128(x, 12));\
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
	res = _mm_cvtep##TI##16_epi32(param);\
	res = convert_##TONAME##int4##RMODE((_4##TI##32)res);\
	return res.lo;\
	}\
	TONAME##int3 __attribute__ ((overloadable)) convert_##TONAME##int3##RMODE(TINAME##short3 x)\
	{\
	_8##TI##16 param;\
	param.s012 = x;\
	TONAME##int4 res = convert_##TONAME##int4##RMODE((_4##TI##32)_mm_cvtep##TI##16_epi32(param));\
	return res.s012;\
	}\
	_4##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int4##RMODE(_4##TI##16 x)\
	{\
	_8##TI##16 param;\
	param.lo = x;\
	return convert_##TONAME##int4##RMODE((_4##TI##32)_mm_cvtep##TI##16_epi32(param));\
	}\
	_8##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int8##RMODE(_8##TI##16 x)\
	{\
	_8##TO##32 res;\
	res.lo = _mm_cvtep##TI##16_epi32(x);\
	res.lo = convert_##TONAME##int4##RMODE((_4##TI##32)res.lo);\
	res.hi = _mm_cvtep##TI##16_epi32(_mm_srli_si128(x, 8));\
	res.hi = convert_##TONAME##int4##RMODE((_4##TI##32)res.hi);\
	return res;\
	}\
	_16##TO##32 __attribute__ ((overloadable)) convert_##TONAME##int16##RMODE(_16##TI##16 x)\
	{\
	_16##TO##32 res;\
	res.lo.lo = _mm_cvtep##TI##16_epi32(x.lo);\
	res.lo.lo = convert_##TONAME##int4##RMODE((_4##TI##32)res.lo.lo);\
	res.lo.hi = _mm_cvtep##TI##16_epi32(_mm_srli_si128(x.lo, 8));\
	res.lo.hi = convert_##TONAME##int4##RMODE((_4##TI##32)res.lo.hi);\
	res.hi.lo = _mm_cvtep##TI##16_epi32(x.hi);\
	res.hi.lo = convert_##TONAME##int4##RMODE((_4##TI##32)res.hi.lo);\
	res.hi.hi = _mm_cvtep##TI##16_epi32(_mm_srli_si128(x.hi, 8));\
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

#define DEF_SAT_PROTOU32_F(RMODE, RMODEVAL)\
	_1u32 __attribute__ ((overloadable)) convert_uint_sat##RMODE(float x)\
	{\
	_1u32 res;\
	int rm = getRound();\
	setRound((rm& ~_MM_ROUND_MASK) | RMODEVAL);\
	res = floatToUintSat(x);\
	setRound(rm);\
	return res;\
	}\
	_2u32 __attribute__ ((overloadable)) convert_uint2_sat##RMODE(float2 x)\
	{\
	int rm = getRound();\
	_2u32 res;\
	setRound((rm& ~_MM_ROUND_MASK) | RMODEVAL);\
	res.lo = floatToUintSat(x.lo);\
	res.hi = floatToUintSat(x.hi);\
	setRound(rm);\
	return res;\
	}\
	uint3 __attribute__ ((overloadable)) convert_uint3_sat##RMODE(float3 x)\
	{\
	int rm = getRound();\
	uint3 res;\
	setRound((rm& ~_MM_ROUND_MASK) | RMODEVAL);\
	res.s0 = floatToUintSat(x.s0);\
	res.s1 = floatToUintSat(x.s1);\
	res.s2 = floatToUintSat(x.s2);\
	setRound(rm);\
	return res;\
	}\
	_4u32 __attribute__ ((overloadable)) convert_uint4_sat##RMODE(float4 x)\
	{\
	int rm = getRound();\
	_4u32 res;\
	setRound((rm& ~_MM_ROUND_MASK) | RMODEVAL);\
	res.s0 = floatToUintSat(x.s0);\
	res.s1 = floatToUintSat(x.s1);\
	res.s2 = floatToUintSat(x.s2);\
	res.s3 = floatToUintSat(x.s3);\
	setRound(rm);\
	return res;\
	}\
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

#define DEF_SAT_PROTOU32_D(RMODE, RSVML, CPUTYPE)\
	_1u32 __attribute__((overloadable)) convert_uint_sat##RMODE(double x)\
	{\
	_1u32 res = __ocl_svml_##CPUTYPE##_cvtfptou32##RSVML##sat1(x);\
	return res;\
	}\
	_2u32 __attribute__((overloadable)) convert_uint2_sat##RMODE(double2 x)\
	{\
    double4 param = {0.0};\
    param.lo=x;\
    _4u32 res = __ocl_svml_##CPUTYPE##_cvtfptou32##RSVML##sat4(param);\
	return res.lo;\
	}\
	uint3 __attribute__((overloadable)) convert_uint3_sat##RMODE(double3 x)\
	{\
	_4u32 res = __ocl_svml_##CPUTYPE##_cvtfptou32##RSVML##sat4(as_double4(x));\
	return as_uint3(res);\
	}\
	_4u32 __attribute__((overloadable)) convert_uint4_sat##RMODE(double4 x)\
	{\
	_4u32 res = __ocl_svml_##CPUTYPE##_cvtfptou32##RSVML##sat4(x);\
	return res;\
	}\
	_8u32 __attribute__((overloadable)) convert_uint8_sat##RMODE(double8 x)\
	{\
	_8u32 res = __ocl_svml_##CPUTYPE##_cvtfptou32##RSVML##sat8(x);\
	return res;\
	}\
	_16u32 __attribute__((overloadable)) convert_uint16_sat##RMODE(double16 x)\
	{\
	_16u32 res;\
    res.lo = __ocl_svml_##CPUTYPE##_cvtfptou32##RSVML##sat8(x.lo);\
    res.hi = __ocl_svml_##CPUTYPE##_cvtfptou32##RSVML##sat8(x.hi);\
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
	res =  _mm_cvtepi8_epi64(param);\
	res =  convert_ulong2##RMODE((_2i64)res);\
	return res;\
	}\
	ulong3 __attribute__ ((overloadable)) convert_ulong3##RMODE(char3 x)\
	{\
	_4u64 res;\
	_16i8 param;\
	param.s012 = x;\
	res.lo =  _mm_cvtepi8_epi64(param);\
	res.hi =  _mm_cvtepi8_epi64(_mm_srli_si128(param, 2));\
	res = convert_ulong4##RMODE((_4i64)res);\
	return as_ulong3(res);\
	}\
	_4u64 __attribute__ ((overloadable)) convert_ulong4##RMODE(_4i8 x)\
	{\
	_4u64 res;\
	_16i8 param;\
	param.s0123 = x;\
	res.lo =  _mm_cvtepi8_epi64(param);\
	res.hi =  _mm_cvtepi8_epi64(_mm_srli_si128(param, 2));\
	res = convert_ulong4##RMODE((_4i64)res);\
	return res;\
	}\
	_8u64 __attribute__ ((overloadable)) convert_ulong8##RMODE(_8i8 x)\
	{\
	_8u64 res;\
	_16i8 param;\
	param.lo = x;\
	res.lo.lo = _mm_cvtepi8_epi64(param);\
	res.lo.hi = _mm_cvtepi8_epi64(_mm_srli_si128(param, 2));\
	res.hi.lo = _mm_cvtepi8_epi64(_mm_srli_si128(param, 4));\
	res.hi.hi = _mm_cvtepi8_epi64(_mm_srli_si128(param, 6));\
	res = convert_ulong8##RMODE((_8i64)res);\
	return res;\
	}\
	_16u64 __attribute__ ((overloadable)) convert_ulong16##RMODE(_16i8 x)\
	{\
	_16u64 res;\
	res.lo.lo.lo = _mm_cvtepi8_epi64(x);\
	res.lo.lo.hi = _mm_cvtepi8_epi64(_mm_srli_si128(x, 2));\
	res.lo.hi.lo = _mm_cvtepi8_epi64(_mm_srli_si128(x, 4));\
	res.lo.hi.hi = _mm_cvtepi8_epi64(_mm_srli_si128(x, 6));\
	res.hi.lo.lo = _mm_cvtepi8_epi64(_mm_srli_si128(x, 8));\
	res.hi.lo.hi = _mm_cvtepi8_epi64(_mm_srli_si128(x, 10));\
	res.hi.hi.lo = _mm_cvtepi8_epi64(_mm_srli_si128(x, 12));\
	res.hi.hi.hi = _mm_cvtepi8_epi64(_mm_srli_si128(x, 14));\
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
	res =  _mm_cvtepi16_epi64(param);\
	res =  convert_ulong2##RMODE((_2i64)res);\
	return res;\
	}\
	ulong3 __attribute__ ((overloadable)) convert_ulong3##RMODE(short3 x)\
	{\
	_4u64 res;\
	_8i16 param;\
	param.s012 = x;\
	res.lo =  _mm_cvtepi16_epi64(param);\
	res.hi =  _mm_cvtepi16_epi64(_mm_srli_si128(param, 4));\
	res = convert_ulong4##RMODE((_4i64)res);\
	return as_ulong3(res);\
	}\
	_4u64 __attribute__ ((overloadable)) convert_ulong4##RMODE(_4i16 x)\
	{\
	_4u64 res;\
	_8i16 param;\
	param.lo = x;\
	res.lo =  _mm_cvtepi16_epi64(param);\
	res.hi =  _mm_cvtepi16_epi64(_mm_srli_si128(param, 4));\
	res = convert_ulong4##RMODE((_4i64)res);\
	return res;\
	}\
	_8u64 __attribute__ ((overloadable)) convert_ulong8##RMODE(_8i16 x)\
	{\
	_8u64 res;\
	res.lo.lo = _mm_cvtepi16_epi64(x);\
	res.lo.hi = _mm_cvtepi16_epi64(_mm_srli_si128(x, 4));\
	res.hi.lo = _mm_cvtepi16_epi64(_mm_srli_si128(x, 8));\
	res.hi.hi = _mm_cvtepi16_epi64(_mm_srli_si128(x, 12));\
	res = convert_ulong8##RMODE((_8i64)res);\
	return res;\
	}\
	_16u64 __attribute__ ((overloadable)) convert_ulong16##RMODE(_16i16 x)\
	{\
	_16u64 res;\
	res.lo.lo.lo = _mm_cvtepi16_epi64(x.lo);\
	res.lo.lo.hi = _mm_cvtepi16_epi64(_mm_srli_si128(x.lo, 4));\
	res.lo.hi.lo = _mm_cvtepi16_epi64(_mm_srli_si128(x.lo, 8));\
	res.lo.hi.hi = _mm_cvtepi16_epi64(_mm_srli_si128(x.lo, 12));\
	res.hi.lo.lo = _mm_cvtepi16_epi64(x.hi);\
	res.hi.lo.hi = _mm_cvtepi16_epi64(_mm_srli_si128(x.hi, 4));\
	res.hi.hi.lo = _mm_cvtepi16_epi64(_mm_srli_si128(x.hi, 8));\
	res.hi.hi.hi = _mm_cvtepi16_epi64(_mm_srli_si128(x.hi, 12));\
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
	res =  _mm_cvtepi32_epi64(param);\
	res =  convert_ulong2##RMODE((_2i64)res);\
	return res;\
	}\
	ulong3 __attribute__ ((overloadable)) convert_ulong3##RMODE(int3 x)\
	{\
	_4u64 res;\
	int4 y = as_int4(x);\
	res.lo =  _mm_cvtepi32_epi64(y);\
	res.hi =  _mm_cvtepi32_epi64(_mm_srli_si128(y, 8));\
	res = convert_ulong4##RMODE((_4i64)res);\
	return as_ulong3(res);\
	}\
	_4u64 __attribute__ ((overloadable)) convert_ulong4##RMODE(_4i32 x)\
	{\
	_4u64 res;\
	res.lo =  _mm_cvtepi32_epi64(x);\
	res.hi =  _mm_cvtepi32_epi64(_mm_srli_si128(x, 8));\
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
    float4 param = {0.0};\
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
    double4 param = {0.0};\
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
	_16u8 t1 = _mm_cmpgt_epi8(*((__m128i *)minuChar8), x);\
	res = _mm_andnot_si128(t1, x);\
	t1 = _mm_and_si128(t1, *((__m128i *)minuCharVal8));\
	res = _mm_or_si128(res, t1);\
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
	_16i8 t1 = _mm_cmpgt_epi8(*((__m128i *)minuChar8), x);\
	res = _mm_andnot_si128(t1, x);\
	t1 = _mm_and_si128(t1, *((__m128i *)miniCharVal8));\
	res = _mm_or_si128(res, t1);\
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
	_8i16 t1 = _mm_cmpgt_epi16(*((_8i16 *)minuShort16), x);\
	res = _mm_andnot_si128(t1, x);\
	t1 = _mm_and_si128(t1, *((_8i16 *)minuShort16));\
	res = _mm_or_si128(res, t1);\
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
	_8i16 t1 = _mm_cmpgt_epi16(*((_8i16 *)minuShort16), x);\
	res = _mm_andnot_si128(t1, x);\
	t1 = _mm_and_si128(t1, *((_8i16 *)maxiShort16));\
	res = _mm_or_si128(res, t1);\
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
	_4u32 t1 = _mm_cmpgt_epi32(*((__m128i *)minuInt32), x);\
	res = _mm_andnot_si128(t1, x);\
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
	_4i32 t1 = _mm_cmpgt_epi32(*((__m128i *)minuInt32), x);\
	res = _mm_andnot_si128(t1, x);\
	t1 = _mm_and_si128(t1, *((__m128i *)maxiInt32));\
	res = _mm_or_si128(res, t1);\
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
	DEF_INT_PROTO16_8(u, u, u, u, RMODE)\
	DEF_INT_PROTO16_32(u, u, u, u, RMODE)\
	DEF_INT_PROTO16_16(u, u, u, u, RMODE)\
	DEF_INT_PROTO16_64(u, u, u, u, RMODE)\
	DEF_INT_PROTO16_8(u, i, u, , RMODE)\
	DEF_INT_PROTO16_16(u, i, u, , RMODE)\
	DEF_INT_PROTO16_32(u, i, u, , RMODE)\
	DEF_INT_PROTO16_64(u, i, u, , RMODE)\
	DEF_INT_PROTO16_8(i, u, , u, RMODE)\
	DEF_INT_PROTO16_16(i, u, , u, RMODE)\
	DEF_INT_PROTO16_32(i, u, , u, RMODE)\
	DEF_INT_PROTO16_64(i, u, , u, RMODE)\
	DEF_INT_PROTO16_8(i, i, , , RMODE)\
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
#define DEF_OUT_INT_RTX(RMODE, RMODEVAL, RSVML, CPUTYPE, FLAG)\
	DEF_INT_PROTO32_8(u, u, u, u, RMODE)\
	DEF_INT_PROTO32_8(u, i, u, , RMODE)\
	DEF_INT_PROTO32_8(i, u, , u, RMODE)\
	DEF_INT_PROTO32_8(i, i, , , RMODE)\
	DEF_INT_PROTO32_16(u, u, u, u, RMODE)\
	DEF_INT_PROTO32_16(u, i, u, , RMODE)\
	DEF_INT_PROTO32_16(i, u, , u, RMODE)\
	DEF_INT_PROTO32_16(i, i, , , RMODE)\
	DEF_INT_PROTO32_32(u, u, u, u, RMODE)\
	DEF_INT_PROTO32_32(u, i, u, , RMODE)\
	DEF_INT_PROTO32_32(i, u, , u, RMODE)\
	DEF_INT_PROTO32_32(i, i, , , RMODE)\
	DEF_INT_PROTO32_64(u, u, u, u, RMODE)\
	DEF_INT_PROTO32_64(u, i, u, , RMODE)\
	DEF_INT_PROTO32_64(i, u, , u, RMODE)\
	DEF_INT_PROTO32_64(i, i, , , RMODE)\
	DEF_INT_PROTOI32_F(RMODE, RMODEVAL, FLAG)\
	DEF_INT_PROTOU32_F(RMODE, RMODEVAL, FLAG)\
	DEF_INT_PROTOI32_D(RMODE, RMODEVAL) \
	DEF_INT_PROTOU32_D(RMODE, RSVML, CPUTYPE)

#define DEF_OUT_INT()\
	DEF_OUT_INT_RTX(, 0x6000, rtz, CTYPE,)\
	DEF_OUT_INT_RTX(_rtz, 0x6000, rtz, CTYPE,)\
	DEF_OUT_INT_RTX(_rte, 0x0, rtn, CTYPE, Round)\
	DEF_OUT_INT_RTX(_rtn, 0x2000, down, CTYPE, Round)\
	DEF_OUT_INT_RTX(_rtp, 0x4000, up, CTYPE, Round)

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
	DEF_OUT_LONG_RTX(_rte, 0x0, rtn, CTYPE)\
	DEF_OUT_LONG_RTX(_rtn, 0x2000, down, CTYPE)\
	DEF_OUT_LONG_RTX(_rtp, 0x4000, up, CTYPE)

	//out float in all with RMODE
#define DEF_OUT_FLOAT_RTX(RMODE, RMODEVAL, RSVML, RSTACK, FLAG, CPUTYPE)\
	DEF_INT_PROTOF_8(i, , RMODE)\
	DEF_INT_PROTOF_8(u, u, RMODE)\
	DEF_INT_PROTOF_16(i, , RMODE)\
	DEF_INT_PROTOF_16(u, u, RMODE)\
	DEF_INT_PROTOF_I32(RMODE, RMODEVAL, FLAG)\
	DEF_INT_PROTOF_U32(RMODE, RMODEVAL, FLAG)\
	DEF_INT_PROTOF_F(, , , , RMODE)\
	DEF_INT_PROTOF_D(RMODE, RMODEVAL, FLAG)\
	DEF_INT_PROTOF_64(i, , RMODE, RSVML, CPUTYPE)\
	DEF_INT_PROTOF_64(u, u, RMODE, RSVML, CPUTYPE)

//#define DEF_OUT_FLOAT_RTX(RMODE, RMODEVAL, RSVML, RSTACK, FLAG, CPUTYPE)\

#define DEF_OUT_FLOAT()\
	DEF_OUT_FLOAT_RTX(,0x0, rtn, 0x0000, ,CTYPE)\
	DEF_OUT_FLOAT_RTX(_rtz, 0x6000, rtz, 0x0300, Round, CTYPE)\
	DEF_OUT_FLOAT_RTX(_rte, 0x0, rtn, 0x0000, , CTYPE)\
	DEF_OUT_FLOAT_RTX(_rtn, 0x2000, down, 0x0100, Round, CTYPE)\
	DEF_OUT_FLOAT_RTX(_rtp, 0x4000, up, 0x0200, Round, CTYPE)

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
	DEF_OUT_DOUBLE_RTX(,0x0, rtn, CTYPE)\
	DEF_OUT_DOUBLE_RTX(_rtz, 0x6000, rtz, CTYPE)\
	DEF_OUT_DOUBLE_RTX(_rte, 0x0, rtn, CTYPE)\
	DEF_OUT_DOUBLE_RTX(_rtn, 0x2000, down, CTYPE)\
	DEF_OUT_DOUBLE_RTX(_rtp, 0x4000, up, CTYPE)

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
	DEF_INT_PROTO16_8(u, u, u, u, _sat##RMODE)\
	DEF_SAT_PROTO16_I32(u, u, RMODE, 65535, 0)\
	DEF_SAT_PROTO16_I64(u, u, RMODE, 65535, 0)\
	DEF_INT_PROTO16_8(u, i, u, , _sat##RMODE)\
	DEF_SAT_PROTO16_I32(i, , RMODE, 32767, -32768)\
	DEF_SAT_PROTO16_I64(i, , RMODE, 32767, -32768)\
	DEF_SAT_PROTO16_8(i, u, , u, _sat##RMODE)\
	DEF_SAT_PROTO16_U32(u, u, RMODE, 65535)\
	DEF_SAT_PROTO16_U64(u, u, RMODE, 65535)\
	DEF_INT_PROTO16_8(i, i, , , _sat##RMODE)\
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
#define DEF_OUT_INT_SAT(RMODE, RMODEVAL, RSVML, CPUTYPE, FLAG)\
	DEF_SAT_PROTOU32(RMODE)\
	DEF_SAT_PROTOI32(RMODE)\
	DEF_INT_PROTO32_8(u, u, u, u, _sat##RMODE)\
	DEF_INT_PROTO32_8(u, i, u, , _sat##RMODE)\
	DEF_SAT_PROTOU32_I8(i, u, , u, _sat##RMODE)\
	DEF_INT_PROTO32_8(i, i, , , _sat##RMODE)\
	DEF_INT_PROTO32_16(u, u, u , u, _sat##RMODE)\
	DEF_INT_PROTO32_16(u, i, u, , _sat##RMODE)\
	DEF_SAT_PROTOU32_I16(i, u, , u, _sat##RMODE)\
	DEF_INT_PROTO32_16(i, i, , , _sat##RMODE)\
	DEF_SAT_PROTO32_I64(u, u, RMODE, _UINT_MAX, 0)\
	DEF_SAT_PROTO32_I64(i, , RMODE, _INT_MAX, _INT_MIN)\
	DEF_SAT_PROTO32_U64(u, u, RMODE, _UINT_MAX)\
	DEF_SAT_PROTO32_U64(i, , RMODE, _INT_MAX)\
	DEF_SAT_PROTOI32_F(RMODE, RMODEVAL, _INT_MAX, _INT_MIN, FLAG)\
	DEF_SAT_PROTOU32_F(RMODE, RMODEVAL)\
	DEF_SAT_PROTOI32_D(RMODE, RMODEVAL)\
	DEF_INT_PROTO32_32(u, u, u, u, _sat##RMODE)\
	DEF_INT_PROTO32_32(i, i, , , _sat##RMODE) \
	DEF_SAT_PROTOU32_D(RMODE, RSVML, CPUTYPE)



#define DEF_SAT_INT()\
	DEF_OUT_INT_SAT( , 0x6000, rtz, CTYPE,)\
	DEF_OUT_INT_SAT(_rtz, 0x6000, rtz, CTYPE,)\
	DEF_OUT_INT_SAT(_rte, 0x0, rtn, CTYPE, Round)\
	DEF_OUT_INT_SAT(_rtn, 0x2000, down, CTYPE, Round)\
	DEF_OUT_INT_SAT(_rtp, 0x4000, up, CTYPE, Round)

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
	DEF_OUT_LONG_SAT(_rte, 0x0, rtn, CTYPE)\
	DEF_OUT_LONG_SAT(_rtn, 0x2000, down, CTYPE)\
	DEF_OUT_LONG_SAT(_rtp, 0x4000, up, CTYPE)

//#define DEF_OUT_FLOAT_RTX(RMODE, RMODEVAL, RSVML, RSTACK, FLAG, CPUTYPE)\

	//out float in all with RMODE, SAT
#define DEF_OUT_FLOAT_SAT(RMODE, RMODEVAL, RSVML, RSTACK, FLAG, CPUTYPE)\
	DEF_INT_PROTOF_8(i, , _sat##RMODE)\
	DEF_INT_PROTOF_8(u, u, _sat##RMODE)\
	DEF_INT_PROTOF_16(i, , _sat##RMODE)\
	DEF_INT_PROTOF_16(u, u, _sat##RMODE)\
	DEF_INT_PROTOF_I32(_sat##RMODE, RMODEVAL, FLAG)\
	DEF_INT_PROTOF_U32(_sat##RMODE, RMODEVAL, FLAG)\
	DEF_INT_PROTOF_64(i, ,_sat##RMODE, RSVML, CPUTYPE)\
	DEF_INT_PROTOF_64(u, u, _sat##RMODE, RSVML, CPUTYPE)\
	DEF_INT_PROTOF_F(, , , , _sat##RMODE)\
	DEF_SAT_PROTOF_D(_sat##RMODE, RMODEVAL)


#define DEF_SAT_FLOAT()\
	DEF_OUT_FLOAT_SAT(, 0x0, rtn, 0x0000, , CTYPE)\
	DEF_OUT_FLOAT_SAT(_rtz, 0x6000, rtz, 0x0300, Round, CTYPE)\
	DEF_OUT_FLOAT_SAT(_rte, 0x0, rte, 0x0, , CTYPE)\
	DEF_OUT_FLOAT_SAT(_rtn, 0x2000, down, 0x0100, Round, CTYPE)\
	DEF_OUT_FLOAT_SAT(_rtp, 0x4000, up, 0x0200, Round, CTYPE)

//out double in all with RMODE, SAT
#define DEF_OUT_DOUBLE_SAT(RMODE, RMODEVAL, RSVML, CPUTYPE)\
	DEF_INT_PROTOD_8(i, , _sat##RMODE)\
	DEF_INT_PROTOD_8(u, u, _sat##RMODE)\
	DEF_INT_PROTOD_16(i, , _sat##RMODE)\
	DEF_INT_PROTOD_16(u, u, _sat##RMODE)\
	DEF_INT_PROTOD_I32(_sat##RMODE, RMODEVAL)\
	DEF_INT_PROTOD_F(_sat##RMODE)\
	DEF_INT_PROTOD_D(_sat##RMODE) \
	DEF_INT_PROTOD_U32(_sat##RMODE, RSVML, CPUTYPE)\
	DEF_INT_PROTOD_64(i, ,_sat##RMODE, RSVML, CPUTYPE)\
	DEF_INT_PROTOD_64(u, u, _sat##RMODE, RSVML, CPUTYPE)



#define DEF_SAT_DOUBLE()\
	DEF_OUT_DOUBLE_SAT(, 0x0, rtn, CTYPE)\
	DEF_OUT_DOUBLE_SAT(_rtz, 0x6000, rtz, CTYPE)\
	DEF_OUT_DOUBLE_SAT(_rte, 0x0, rtn, CTYPE)\
	DEF_OUT_DOUBLE_SAT(_rtn, 0x2000, down, CTYPE)\
	DEF_OUT_DOUBLE_SAT(_rtp, 0x4000, up, CTYPE)

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
DEF_SAT_FLOAT()
DEF_SAT_DOUBLE()


#endif


#ifdef __cplusplus
}
#endif
