// Copyright (c) 2006-2009 Intel Corporation
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

#if !defined (__MIC__) && !defined(__MIC2__)
#define _INTERNAL_DEBUG

#ifdef __cplusplus
extern "C" {
#endif

#pragma OPENCL EXTENSION cl_khr_fp64 : enable

#include <intrin.h>
#define ALIGN16 __attribute__((aligned(16)))

#include "cl_types2.h"

void* memcpy(void*, const void*, size_t);

ALIGN16 int abss[] = {0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff};
//ALIGN16 __int64 absd[] = {0x7fffffffffffffff, 0x7fffffffffffffff};
ALIGN16 int sign4[] = {0x08000, 0x08000, 0x08000, 0x08000};
//ALIGN16 __int64 dsign4[] = {0x08000, 0x08000};
ALIGN16 int abs4[] = {0x07fff, 0x07fff, 0x07fff, 0x07fff};
//ALIGN16 __int64 dabs4[] = {0x07fff, 0x07fff};
ALIGN16 int nans[] = {0x0200, 0x0200, 0x0200, 0x0200};
//ALIGN16 __int64 dnans[] = {0x0200, 0x0200};
ALIGN16 int inf[] = {0x7f800000, 0x7f800000, 0x7f800000, 0x7f800000};
//ALIGN16 __int64 dinf[] = {0x7ff0000000000000ULL, 0x7ff0000000000000ULL};
ALIGN16 int infval[] = {0x7c00, 0x7c00, 0x7c00, 0x7c00};
ALIGN16 int oflw[] = {0x47800000, 0x47800000, 0x47800000, 0x47800000};
//ALIGN16 __int64 doflw[] = {0x40f0000000000000, 0x40f0000000000000};
ALIGN16 int oflwval[] = {0x7bff, 0x7bff, 0x7bff, 0x7bff};
ALIGN16 int uflw[] = {0x33800000, 0x33800000, 0x33800000, 0x33800000};
//ALIGN16 __int64 duflw[] = {0x3e70000000000000, 0x3e70000000000000};
ALIGN16 int dnrml[] = {0x38800000, 0x38800000, 0x38800000, 0x38800000};
//ALIGN16 __int64 ddnrml[] = {0x3f10000000000000, 0x3f10000000000000};
ALIGN16 int dnrmlval[] = {0x4b800000, 0x4b800000, 0x4b800000, 0x4b800000};
//ALIGN16 __int64 ddnrmlval[] = {0x4170000000000000, 0x4170000000000000};
ALIGN16 int mantissas[] = {0xffffe000, 0xffffe000, 0xffffe000, 0xffffe000};
//ALIGN16 __int64 dmantissas[] = {0xFFFFFC0000000000ULL, 0xFFFFFC0000000000ULL};
ALIGN16 int mask[] = {0x38000000, 0x38000000, 0x38000000, 0x38000000};
//ALIGN16 __int64 dmask[] = {0x3F00000000000000ULL, 0x3F00000000000000ULL};
ALIGN16 int oflwRtn[] = {0x477ff000, 0x477ff000, 0x477ff000, 0x477ff000};
//ALIGN16 __int64 doflwRtn[] = {0x40effe0000000000, 0x40effe0000000000};
ALIGN16 int oflwRtnval[] = {0x7c00, 0x7c00, 0x7c00, 0x7c00};
ALIGN16 int negoflwRtn[] = {0xc7800000, 0xc7800000, 0xc7800000, 0xc7800000};
//ALIGN16 __int64 dnegoflwRtn[] = {0xc7800000, 0xc7800000, 0xc7800000, 0xc7800000};
ALIGN16 int negoflwRtnval[] = {0xfbff, 0xfbff, 0xfbff, 0xfbff};
ALIGN16 int neginf[] = {0xff800000, 0xff800000, 0xff800000, 0xff800000};
//ALIGN16 __int64 dneginf[] = {0xfff0000000000000ULL, 0xfff0000000000000ULL};
ALIGN16 int negoflw[] = {0xc77fe000, 0xc77fe000, 0xc77fe000, 0xc77fe000};
//ALIGN16 __int64 dnegoflw[] = {0xc0effc0000000000, 0xc0effc0000000000};
ALIGN16 int neginfval[] = {0xfc00, 0xfc00, 0xfc00, 0xfc00};
ALIGN16 int ones[] = {1, 1, 1, 1};
//ALIGN16 __int64 dones[] = {1, 1, 1, 1};
ALIGN16 int maskRtn[] = {0x00002000, 0x00002000, 0x00002000, 0x00002000};
//ALIGN16 __int64 dmaskRtn[] = {0x0000040000000000ULL, 0x0000040000000000ULL};
ALIGN16 int uflwval[] = {0x8001, 0x8001, 0x8001, 0x8001};
ALIGN16 int uflwRte[] = {0x33000000, 0x33000000, 0x33000000, 0x33000000};
//ALIGN16 __int64 duflwRte[] = {0x3e60000000000000, 0x3e60000000000000};
ALIGN16 int smallRte[] = {0x33c00000, 0x33c00000, 0x33c00000, 0x33c00000};
//ALIGN16 __int64 dsmallRte[] = {0x3e78000000000000, 0x3e78000000000000};
ALIGN16 int dnrmlRteval[] = {0x01000000, 0x01000000, 0x01000000, 0x01000000};
//ALIGN16 __int64 ddnrmlRteval[] = {0x0000000001000000, 0x0000000001000000};
ALIGN16 int rteVal[] = {0x46000000, 0x46000000, 0x46000000, 0x46000000};
//ALIGN16 __int64 drteVal[] = {0x4290000000000000, 0x4290000000000000};
ALIGN16 int rteMask[] = {0x07800000, 0x07800000, 0x07800000, 0x07800000};
//ALIGN16 __int64 drteMask[] = {0x00f0000000000000, 0x00f0000000000000};
ALIGN16 short  Fvec8Float16ExponentMask[] = {0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00};
ALIGN16 short  Fvec8Float16MantissaMask[] = {0x03FF, 0x03FF, 0x03FF, 0x03FF, 0x03FF, 0x03FF, 0x03FF, 0x03FF};
ALIGN16 short  Fvec8Float16SignMask[]     = {0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000};
ALIGN16 int Fvec4Float16NaNExpMask[]   = {0x7C00, 0x7C00, 0x7C00, 0x7C00};
ALIGN16 int Fvec4Float32ExponentMask[] = {0x7F800000, 0x7F800000, 0x7F800000, 0x7F800000};
ALIGN16 int Fvec4Float32NanMask[] = {0x7FC00000, 0x7FC00000, 0x7FC00000, 0x7FC00000};
ALIGN16 int FVec4Float16Implicit1Mask[] = {(1<<10), (1<<10), (1<<10), (1<<10)};
ALIGN16 int Fvec4Float16ExpMin[] = {(1<<10), (1<<10), (1<<10), (1<<10)};
ALIGN16 int Fvec4Float16BiasDiffDenorm[] = {((127 - 15 - 10) << 23), ((127 - 15 - 10) << 23), ((127 - 15 - 10) << 23), ((127 - 15 - 10) << 23)};
ALIGN16 int Fvec4Float16ExpBiasDifference[] = {((127 - 15) << 10), ((127 - 15) << 10), ((127 - 15) << 10), ((127 - 15) << 10)};

float HalfToFloat( half param );
/// !!! This function is copy-pasted to images module.
/// In case of any changes they should also be applied to image_callback_functions.cpp
float4 Half4ToFloat4(_8i16 xmm0);
float8 Half8ToFloat8(_8i16 xmm0);

#if defined(__AVX2__)
float HalfToFloat( half param )
{
	half8 x;
	x.s0 = param;
	return Half4ToFloat4(as_short8(x)).s0;
}
/// !!! This function is copy-pasted to images module.
/// In case of any changes they should also be applied to image_callback_functions.cpp
float4 Half4ToFloat4(_8i16 xmm0)
{
	return _mm_cvtph_ps(*(__m128i*)&xmm0);
}
float8 Half8ToFloat8(_8i16 xmm0)
{
	return _mm256_cvtph_ps(*(__m128i*)&xmm0);
}

#else // defined(__AVX2__)
float HalfToFloat( half param )
{
	unsigned short expHalf16 = as_short(param) & 0x7C00;
	int exp1 = (int)expHalf16;
	unsigned short mantissa16 = as_short(param) & 0x03FF;
	int mantissa1 = (int)mantissa16;
	int sign = (int)(as_short(param) & 0x8000);
	sign = sign << 16;
	
	if (expHalf16 == 0x7C00) // nan or inf
	{
		if (mantissa16 > 0) // nan
		{
			int res = (0x7FC00000 | sign); //silance the nans
			float fres = *((float*)(&res));
			return fres;
		}
		// inf
		int res = (0x7F800000 | sign);
		float fres = *((float*)(&res));
		return fres;
	}
	if (expHalf16 != 0) // normalized number
	{
		exp1 += ((127 - 15) << 10); //exponents converted to float32 bias
		int res = (exp1 | mantissa1);
		res = res << 13 ;
		res = ( res | sign );
		float fres = *((float*)(&res));
		return fres;
	}

	int xmm1 = max (exp1, (1 << 10));
	xmm1 = (xmm1 << 13);
	xmm1 += ((127 - 15 - 10) << 23);  // add the bias difference to xmm1
	xmm1 = xmm1 | sign; // Combine with the sign mask

	float res = (float)mantissa1;  // Convert mantissa to float
	res *= *((float*) (&xmm1));
	
	return res;	
}

/// !!! This function is copy-pasted to images module.
/// In case of any changes they should also be applied to image_callback_functions.cpp
float4 Half4ToFloat4(_8i16 xmm0)
{
	_4i32 xmm7 = (_4i32)_mm_setzero_si128();
	_4i32 xmm1 = (_4i32)_mm_and_si128((__m128i)xmm0,(__m128i) *((_4i32*)Fvec8Float16ExponentMask));  
	xmm1 = (_4i32) _mm_unpacklo_epi16((__m128i)xmm1, (__m128i)xmm7); // xmm1 = exponents as DWORDS
    _4i32 xmm2 = (_4i32)_mm_and_si128((__m128i)xmm0,(__m128i) *((_4i32*)Fvec8Float16MantissaMask));
	xmm2 = (_4i32) _mm_unpacklo_epi16((__m128i)xmm2, (__m128i)xmm7); // xmm2 = mantissas as DWORDS
	xmm0 = (_8i16) _mm_and_si128((__m128i)xmm0,(__m128i) *((_4i32 * )Fvec8Float16SignMask)); 
	_4i32 xmm6 = (_4i32)_mm_unpacklo_epi16((__m128i)xmm7, (__m128i)xmm0); // xmm6 = sign mask as DWORDS

	// We need to handle the case where the number is NaN or INF
	// If the float16 is one of these, then we create an all '1' exponent for the 32bit float and store it in xmm6 for later use
	xmm0 = (_8i16) _mm_cmpeq_epi32((__m128i)xmm1,(__m128i) *((_4i32 *)Fvec4Float16NaNExpMask)); // xmm0.any dword = 0xFFFFFFFF if exponent is all '1'
	_4i32 xmm4 = (_4i32)_mm_cmpgt_epi32((__m128i)xmm2, (__m128i)xmm7); // xmm4.any dword = 0xFFFFFFFF if mantissa > 0
	xmm4 = (_4i32) _mm_and_si128((__m128i)xmm4,(__m128i) xmm0); // xmm4.any dword = 0xFFFFFFFF if NAN
	xmm4 = (_4i32) _mm_and_si128((__m128i)xmm4,(__m128i) *((_4i32 * )Fvec4Float32NanMask)); // silence the SNaNs
	xmm0 = (_8i16) _mm_and_si128((__m128i)xmm0, (__m128i)*((_4i32 * )Fvec4Float32ExponentMask)); // // xmm0 = If float16 has all '1' exp, then convert to 32 bit float all '1' exp, otherwise 0
	xmm0 = (_8i16) _mm_or_si128((__m128i)xmm0,(__m128i) xmm4); 

	_4i32 xmm3 = (_4i32)_mm_cmpeq_epi32((__m128i)xmm1, (__m128i)xmm7); // xmm3.any dword = 0xFFFFFFFF if exp is zero
	int normals = _mm_movemask_epi8((__m128i)xmm3);
	if(normals == 0)  
	{
		xmm1 = (_4i32) _mm_add_epi32((__m128i)xmm1,(__m128i) *((_4i32 * )Fvec4Float16ExpBiasDifference));// xmm1 = exponents converted to float32 bias
		xmm1 = (_4i32) _mm_or_si128((__m128i)xmm1,(__m128i) xmm2); // xmm1 = exponent + mantissa
		xmm1 = (_4i32) _mm_slli_epi32((__m128i)xmm1, 13);
		xmm1 = (_4i32) _mm_or_si128((__m128i)xmm1,(__m128i) xmm6);  // xmm1 = signed number
		float4 res = (float4)_mm_or_si128((__m128i)xmm1,(__m128i) xmm0);
		return res;  // If the original number was NaN or INF, then xmm6 has all '1' for exp. xmm0 will hold a float32 that is NaN or INF 
	}

	xmm3 = (_4i32) _mm_andnot_si128((__m128i)xmm3,(__m128i) *((_4i32 * )FVec4Float16Implicit1Mask));
	xmm2 = (_4i32) _mm_or_si128((__m128i)xmm2,(__m128i) xmm3); // add implicit 1 to the mantissa
	xmm1 = (_4i32) _mm_max_epi16((__m128i)xmm1, (__m128i)*((_4i32 * )Fvec4Float16ExpMin)); // xmm1 = max(exp, 1)

	// we can do the comparison on words since we know that the high word of each dword is 0
	xmm1 = (_4i32) _mm_slli_epi32((__m128i)xmm1, 13);
	xmm1 = (_4i32) _mm_add_epi32((__m128i)xmm1,(__m128i) *((_4i32 * )Fvec4Float16BiasDiffDenorm));  // add the bias difference to xmm1
	xmm1 = (_4i32) _mm_or_si128((__m128i)xmm1,(__m128i) xmm6); // Combine with the sign mask
	
	float4 xmm5 = _mm_cvtepi32_ps((__m128i)xmm2);  // Convert mantissa to float
	xmm5 = _mm_mul_ps(xmm5, _mm_castsi128_ps((__m128i)xmm1));
	
	xmm5 = _mm_or_ps((__m128)xmm5, _mm_castsi128_ps((__m128i)xmm0));// If the original number was NaN or INF, then xmm6 has all '1' for exp. xmm0 will hold a float32 that is NaN or INF 
	return xmm5;
}

float8 Half8ToFloat8(_8i16 xmm0)
{
	float8 res;
	_8i16 inp1;
	inp1.s0123 = xmm0.s4567;
	res.lo = Half4ToFloat4(xmm0);
	res.hi = Half4ToFloat4(inp1);
	return res;
}

#endif // __AVX2__

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

ALIGN16 long x7fffffffffffffff[] = {0x7fffffffffffffff, 0x7fffffffffffffff};
ALIGN16 long x7ff0000000000000[] = {0x7ff0000000000000, 0x7ff0000000000000};
ALIGN16 long x40f0000000000000[] = {0x40f0000000000000, 0x40f0000000000000};
ALIGN16 long x3e70000000000000[] = {0x3e70000000000000, 0x3e70000000000000};
ALIGN16 long x3f10000000000000[] = {0x3f10000000000000, 0x3f10000000000000};
ALIGN16 long x4170000000000000[] = {0x4170000000000000, 0x4170000000000000};
ALIGN16 long xFFFFFC0000000000[] = {0xFFFFFC0000000000, 0xFFFFFC0000000000};
ALIGN16 long x3F00000000000000[] = {0x3F00000000000000, 0x3F00000000000000};
ALIGN16 long x40effe0000000000[] = {0x40effe0000000000, 0x40effe0000000000};
ALIGN16 long x40effc0000000000[] = {0x40effc0000000000, 0x40effc0000000000};
ALIGN16 long x00f0000000000000[] = {0x00f0000000000000, 0x00f0000000000000};
ALIGN16 long x4290000000000000[] = {0x4290000000000000, 0x4290000000000000};
ALIGN16 long x0000000001000000[] = {0x0000000001000000, 0x0000000001000000};
ALIGN16 long x3e78000000000000[] = {0x3e78000000000000, 0x3e78000000000000};
ALIGN16 long x3e60000000000000[] = {0x3e60000000000000, 0x3e60000000000000};
ALIGN16 long x0000040000000000[] = {0x0000040000000000, 0x0000040000000000};
ALIGN16 long xc0effc0000000000[] = {0xc0effc0000000000, 0xc0effc0000000000};
ALIGN16 long xfff0000000000000[] = {0xfff0000000000000, 0xfff0000000000000};
ALIGN16 long xc0f0000000000000[] = {0xc0f0000000000000, 0xc0f0000000000000};

ALIGN16 int conversion_ones[] = {1, 1, 1, 1};
ALIGN16 long dones[] = {1, 1};
ALIGN16 char g_vls_4x32to4x16[] = {0,1, 4,5,  8,9, 12,13, 0, 0, 0, 0, 0,0,0,0};
ALIGN16 char g_vls_2x64to2x16[] = {0,1, 8,9,0,0, 0, 0, 0, 0, 0, 0,0,0,0,0};

float4 _ocl_float2half_rte(float4 param);
float4 _ocl_float2half_rtz(float4 param);
float4 _ocl_float2half_rtn(float4 param);
float4 _ocl_float2half_rtp(float4 param);
float4 _ocl_float2half(float4 param);

float4 _ocl_float2half8_rte(float8 param);
float4 _ocl_float2half8_rtz(float8 param);
float4 _ocl_float2half8_rtn(float8 param);
float4 _ocl_float2half8_rtp(float8 param);
float4 _ocl_float2half8(float8 param);

#if defined(__AVX2__)

#define PROTO_OCL_FLOAT2HALF(RMODE, CPUFLAG)					\
	float4 _ocl_float2half##RMODE(float4 param)					\
	{\
		return _mm_castsi128_ps(_mm_cvtps_ph(param, CPUFLAG));\
	}

PROTO_OCL_FLOAT2HALF(_rte, _MM_FROUND_TO_NEAREST_INT)
PROTO_OCL_FLOAT2HALF(_rtz, _MM_FROUND_TO_ZERO)
PROTO_OCL_FLOAT2HALF(_rtn, _MM_FROUND_TO_NEG_INF)
PROTO_OCL_FLOAT2HALF(_rtp, _MM_FROUND_TO_POS_INF)

#define PROTO_OCL_FLOAT2HALF8(RMODE, CPUFLAG)					\
	float4 _ocl_float2half8##RMODE(float8 param)				\
	{\
		return _mm_castsi128_ps(_mm256_cvtps_ph(param, CPUFLAG));\
	}

PROTO_OCL_FLOAT2HALF8(_rte, _MM_FROUND_TO_NEAREST_INT)
PROTO_OCL_FLOAT2HALF8(_rtz, _MM_FROUND_TO_ZERO)
PROTO_OCL_FLOAT2HALF8(_rtn, _MM_FROUND_TO_NEG_INF)
PROTO_OCL_FLOAT2HALF8(_rtp, _MM_FROUND_TO_POS_INF)

#else // defined(__AVX2__)
/// !!! This function is copy-pasted to images module.
/// In case of any changes they should also be applied to image_callback_functions.cpp
float4 _ocl_float2half_rte(float4 param)
{
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
    eq0 = (_8i16) _mm_and_si128((__m128i)eq1, _mm_or_si128((__m128i)signs,(__m128i) *((_4i32 *)conversion_ones)));
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

    eq1 = _mm_mul_ps(param, *((float4 *)x46000000));  
    eq0 = (_8i16) _mm_and_si128((__m128i)eq1,(__m128i) *((_4i32 *)x7f800000)); //u
    eq1 = _mm_add_ps( (float4)eq0 ,absParam); //x
    eq1 = _mm_sub_ps(eq1, (float4)eq0); //u
    eq1 = _mm_mul_ps(eq1, *((float4 *)x07800000));  
    eq0 = (_8i16) _mm_srli_epi32((__m128i)eq1, 0x0d);
    eq0 = (_8i16) _mm_or_si128((__m128i)eq0,(__m128i) signs);
    eq0 = (_8i16) _mm_andnot_si128((__m128i)dflt,(__m128i) eq0);
    eq = (_8i16) _mm_or_si128((__m128i)eq,(__m128i) eq0);

    eq1 = _mm_castsi128_ps(_mm_shuffle_epi8((__m128i)eq, *((__m128i *)g_vls_4x32to4x16)));

    return eq1;
}

float4 _ocl_float2half_rtz(float4 param)
{

    //cl_uint sign = (u.u >> 16) & 0x8000;
    __m128 temp = _mm_castsi128_ps(_mm_srli_epi32(_mm_castps_si128(param), 0x10));
    __m128 signs = _mm_and_ps(temp, *(__m128*)x8000);
    param = (float4)_mm_and_si128(_mm_castps_si128(param), *((__m128i*)x7fffffff));

    //Nan
    //if( x != x ) 
    __m128 eq0 = _mm_cmpneq_ps(param, param);
    __m128 eq = _mm_and_ps(param, eq0);
    //u.u >>= (24-11);
    eq = _mm_castsi128_ps(_mm_srli_epi32(_mm_castps_si128(eq), 0x0d));
    //u.u &= 0x7fff;
    eq = _mm_and_ps(eq, *((__m128*)x7fff));
    //u.u |= 0x0200;   -- silence the NaN
    eq = _mm_or_ps(eq, *((__m128*)x0200));
    //return u.u | sign;
    eq = _mm_or_ps(eq, signs);
    eq = _mm_and_ps(eq, (__m128)eq0);
    __m128 dflt = eq0;

    // overflow
    //if( x >= MAKE_HEX_FLOAT(0x1.0p16f, 0x1L, 16) )

    __m128 eq1 = _mm_cmpge_ps(param, *((__m128*)x47800000));

    //if( x == INFINITY )
    //return 0x7c00 | sign;
    eq0 = _mm_castsi128_ps(_mm_cmpeq_epi32(_mm_castps_si128(param), *((__m128i*)x7f800000)));
    eq0 = _mm_and_ps(eq0, eq1);
    __m128 eq2 = _mm_and_ps(eq0, *((__m128*)x7c00));
    eq2 = _mm_or_ps(eq2, signs);
    eq2 = _mm_and_ps(eq0, eq2);
    eq2 = _mm_andnot_ps(dflt, eq2);
    eq = _mm_or_ps(eq, eq2);
    dflt = _mm_or_ps(eq0, dflt);


    //else return 0x7bff | sign;
    eq0 = _mm_xor_ps(eq1, eq0);
    eq2 = _mm_and_ps(eq0, *((__m128*)x7bff));
    eq2 = _mm_or_ps(eq2, signs);
    eq2 = _mm_and_ps(eq2, eq0);
    eq2 = _mm_andnot_ps(dflt, eq2);
    eq = _mm_or_ps(eq, eq2);
    dflt = _mm_or_ps(eq0, dflt);


    // underflow
    //	if( x < MAKE_HEX_FLOAT(0x1.0p-24f, 0x1L, -24) )
    //  return sign;    -- The halfway case can return 0x0001 or 0. 0 is even.
    eq1 = _mm_cmplt_ps(param, *((__m128*)x33800000));
    eq0 = _mm_and_ps(eq1, signs);
    eq0 = _mm_andnot_ps(dflt, eq0);
    eq = _mm_or_ps(eq, eq0);
    dflt = _mm_or_ps(eq1, dflt);


    // half denormal
    //  if( x < MAKE_HEX_FLOAT(0x1.0p-14f, 0x1L, -14) )
    //	x *= MAKE_HEX_FLOAT(0x1.0p24f, 0x1L, 24);
    //  return (short)( (int)x | sign);
    eq1 = _mm_cmplt_ps(param, *((__m128*)x38800000));
    eq2 = eq1;
    eq1 = _mm_and_ps(eq1, param);
    eq1 = _mm_mul_ps(eq1, *((__m128*)x4b800000));
    eq0 = _mm_castsi128_ps(_mm_cvttps_epi32(eq1));
    eq0 = _mm_or_ps(eq0, signs);
    eq0 = _mm_and_ps(eq2, eq0);
    eq0 = _mm_andnot_ps(dflt, eq0);
    eq  = _mm_or_ps(eq, eq0); 
    dflt = _mm_or_ps(eq2, dflt);


    //u.u &= 0xFFFFE000U;
    eq0 = _mm_and_ps(param, *((__m128*)xffffe000));
    //u.u -= 0x38000000U;
    eq0 = _mm_castsi128_ps(_mm_sub_epi32(_mm_castps_si128(eq0), *((__m128i*)x38000000)));
    eq0 = _mm_castsi128_ps(_mm_srli_epi32(_mm_castps_si128(eq0), 13));
    eq0 = _mm_or_ps(eq0, signs);
    eq0 = _mm_andnot_ps(dflt, eq0);
    eq  = _mm_or_ps(eq, eq0);

    eq1 = _mm_castsi128_ps(_mm_shuffle_epi8((__m128i)eq, *((__m128i *)g_vls_4x32to4x16)));
    return eq1;
}

float4 _ocl_float2half_rtp(float4 param)
{
    float4 zeros = _mm_setzero_ps();

    //cl_uint sign = (u.u >> 16) & 0x8000;
    __m128 temp = _mm_castsi128_ps(_mm_srli_epi32(_mm_castps_si128(param), 0x10));
    __m128 signs = _mm_and_ps(temp, *((__m128*)x8000));
    float4 absParam = _mm_and_ps(param, *((__m128*)x7fffffff));

    //Nan
    //if( x != x ) 
    __m128 eq0 = _mm_cmpneq_ps(absParam, absParam);
    __m128 eq = _mm_and_ps(absParam, eq0);
    //u.u >>= (24-11);
    eq = _mm_castsi128_ps(_mm_srli_epi32(_mm_castps_si128(eq), 0x0d));
    //u.u &= 0x7fff;
    eq = _mm_and_ps(eq, *((__m128*)x7fff));
    //u.u |= 0x0200;   -- silence the NaN
    eq = _mm_or_ps(eq, *((__m128 *)x0200));
    //return u.u | sign;
    eq = _mm_or_ps(eq, signs);
    eq = _mm_and_ps(eq, eq0);
    __m128 dflt = eq0;

    // overflow
    //if( f > MAKE_HEX_FLOAT(0x1.ffcp15f, 0x1ffcL, 3) )
    //return 0x7c00;

    float4 eq1 = _mm_cmpgt_ps(param, *((float4 *)x477ff000));
    eq0 = _mm_and_ps(eq1, *((__m128*)x7c00));
    eq0 = _mm_andnot_ps(dflt, eq0);
    eq = _mm_or_ps(eq, eq0);
    dflt = _mm_or_ps(eq1, dflt);

    //	if( f <= MAKE_HEX_FLOAT(-0x1.0p16f, -0x1L, 16) )
    eq1 = _mm_cmple_ps(param, *((float4 *)xc7800000));

    //if( f == -INFINITY )
    //return 0xfc00;
    eq0 = _mm_castsi128_ps(_mm_cmpeq_epi32(_mm_castps_si128(param), *((__m128i*)xff800000)));
    eq0 = _mm_and_ps(eq0, eq1);
    float4 eq2 = _mm_and_ps(eq0, *((float4*)xfc00));
    eq2 = _mm_andnot_ps(dflt, eq2);
    eq = _mm_or_ps(eq, eq2);
    dflt = _mm_or_ps(eq0, dflt);

    //else return 0xfbff;
    eq0 = _mm_xor_ps(eq1, eq0);
    eq2 = _mm_and_ps(eq0, *(float4*)xfbff);
    eq2 = _mm_andnot_ps(dflt, eq2);
    eq = _mm_or_ps(eq, eq2);
    dflt = _mm_or_ps(eq0, dflt);

    // underflow
    //	if( x < MAKE_HEX_FLOAT(0x1.0p-24f, 0x1L, -24) )
    eq1 = _mm_cmplt_ps(absParam, *((float4 *)x33800000));

    // if (f > 0) return 1;
    eq0 = _mm_cmpgt_ps(param, zeros);
    eq0 = _mm_and_ps(eq0, eq1);
    eq2 = _mm_and_ps(eq0, *((float4*)conversion_ones));
    eq2 = _mm_andnot_ps(dflt, eq2);
    eq = _mm_or_ps(eq, eq2);
    dflt = _mm_or_ps(eq0, dflt);
    // else return sign
    eq0 = _mm_xor_ps(eq1, eq0);
    eq2 = _mm_and_ps(eq0, signs);
    eq2 = _mm_andnot_ps(dflt, eq2);
    eq = _mm_or_ps(eq, eq2);
    dflt = _mm_or_ps(eq0, dflt);

    // half denormal         
    //  if( x < MAKE_HEX_FLOAT(0x1.0p-14f, 0x1L, -14) )
    //	x *= MAKE_HEX_FLOAT(0x1.0p24f, 0x1L, 24);
    //  int r = (int)x;
    eq1 = _mm_cmplt_ps(absParam, *((float4 *)x38800000));
    eq2 = eq1;
    eq1 = _mm_and_ps(eq1, absParam);
    eq1 = _mm_mul_ps(eq1, *((float4 *)x4b800000));  //x
    eq0 = _mm_castsi128_ps(_mm_cvttps_epi32(eq1)); //r
    // r += (float)r != x && f > 0.0f;
    float4 eq3 = _mm_cvtepi32_ps(_mm_castps_si128(eq0)); //(float)r
    eq1 = _mm_cmpneq_ps(eq1, eq3); // (float)r != x
    eq3 = _mm_cmpgt_ps(param, zeros); //f > 0.0f
    eq1 = _mm_and_ps(eq1, eq3); //(float)r != x && f > 0.0f
    __m128 eq4 = _mm_and_ps(eq1, *((float4*)conversion_ones));
    eq0 = _mm_castsi128_ps(_mm_add_epi32(_mm_castps_si128(eq0), _mm_castps_si128(eq4)));
    // return (short)(r | sign)
    eq0 = _mm_or_ps(eq0, signs); 
    eq0 = _mm_and_ps(eq2, eq0);
    eq0 = _mm_andnot_ps(dflt, eq0);
    eq  = _mm_or_ps(eq, eq0); 
    dflt = _mm_or_ps(eq2, dflt);

    //u.u &= 0xFFFFE000U;
    eq0 = _mm_and_ps(param, *((float4*)xffffe000));
    //if (f > u.f)
    //u.u += 0x00002000U;
    eq1 = _mm_cmpgt_ps(param, (float4)eq0);
    eq2 = _mm_castsi128_ps(
        _mm_add_epi32(_mm_castps_si128(eq0), _mm_castps_si128(_mm_and_ps(eq1, *((float4*)x00002000)))));
    //u.u -= 0x38000000U;
    //return ((u.u >> 13) | sign);
    eq2 = _mm_castsi128_ps(_mm_sub_epi32(_mm_castps_si128(eq2), *((__m128i*)x38000000)));
    eq2 = _mm_castsi128_ps(_mm_srli_epi32(_mm_castps_si128(eq2), 13));
    eq2 = _mm_or_ps(eq2, signs);
    eq2 = _mm_andnot_ps(dflt, eq2);
    eq  = _mm_or_ps(eq, eq2); 

    eq1 = _mm_castsi128_ps(_mm_shuffle_epi8((__m128i)eq, *((__m128i *)g_vls_4x32to4x16)));

    return eq1;
}

float4 _ocl_float2half_rtn(float4 param)
{
    float4 zeros = _mm_setzero_ps();

    //cl_uint sign = (u.u >> 16) & 0x8000;
    float4 temp = _mm_castsi128_ps(_mm_srli_epi32(_mm_castps_si128(param), 0x10));
    float4 signs = _mm_and_ps(temp, *(float4*)x8000);
    float4 absParam = _mm_and_ps(param, *(float4*)x7fffffff);

    //Nan
    //if( x != x ) 
    float4 eq0 = _mm_cmpneq_ps(absParam, absParam);
    float4 eq = _mm_and_ps(absParam, eq0);
    //u.u >>= (24-11);
    eq = _mm_castsi128_ps(_mm_srli_epi32(_mm_castps_si128(eq), 0x0d));
    //u.u &= 0x7fff;
    eq = _mm_and_ps(eq, *(float4*)x7fff);
    //u.u |= 0x0200;   -- silence the NaN
    eq = _mm_or_ps(eq, *(float4*)x0200);
    //return u.u | sign;
    eq = _mm_or_ps(eq, signs);
    eq = _mm_and_ps(eq, eq0);
    float4 dflt = eq0;


    // overflow
    //if( f >= MAKE_HEX_FLOAT(0x1.0p16f, 0x1L, 16) )
    float4 eq1 = _mm_cmpge_ps(param, *(float4 *)x47800000);

    //if( f == INFINITY )
    //return 0x7c00;
    eq0 = _mm_castsi128_ps(_mm_cmpeq_epi32(_mm_castps_si128(param), *(__m128i*)x7f800000));
    eq0 = _mm_and_ps(eq0, eq1);
    float4 eq2 = _mm_and_ps(eq0, *(float4*)x7c00);
    eq2 = _mm_andnot_ps(dflt, eq2);
    eq = _mm_or_ps(eq, eq2);
    dflt = _mm_or_ps(eq0, dflt);

    //else return 0x7bff;
    eq0 = _mm_xor_ps(eq1, eq0);
    eq2 = _mm_and_ps(eq0, *(float4*)x7bff);
    eq2 = _mm_andnot_ps(dflt, eq2);
    eq = _mm_or_ps(eq, eq2);
    dflt = _mm_or_ps(eq0, dflt);

    //if( f < MAKE_HEX_FLOAT(-0x1.ffcp15f, -0x1ffcL, 3) )
    //return 0xfc00;
    eq1 = _mm_cmplt_ps(param, *(float4 *)xc77fe000);
    eq0 = _mm_and_ps(eq1, *(float4*)xfc00);
    eq0 = _mm_andnot_ps(dflt, eq0);
    eq = _mm_or_ps(eq, eq0);
    dflt = _mm_or_ps(eq1, dflt);

    // underflow
    //	if( x < MAKE_HEX_FLOAT(0x1.0p-24f, 0x1L, -24) )
    eq1 = _mm_cmple_ps(absParam, *((float4 *)x33800000));

    // if (f < 0) return 0x8001;
    eq0 = _mm_cmplt_ps(param, zeros);
    eq0 = _mm_and_ps(eq0, eq1);
    eq2 = _mm_and_ps(eq0, *(float4*)x8001);
    eq2 = _mm_andnot_ps(dflt, eq2);
    eq = _mm_or_ps(eq, eq2);
    dflt = _mm_or_ps(eq0, dflt);
    // else return sign
    eq0 = _mm_xor_ps(eq1, eq0);
    eq2 = _mm_and_ps(eq0, signs);
    eq2 = _mm_andnot_ps(dflt, eq2);
    eq = _mm_or_ps(eq, eq2);
    dflt = _mm_or_ps(eq0, dflt);

    // half denormal         
    //  if( x < MAKE_HEX_FLOAT(0x1.0p-14f, 0x1L, -14) )
    //	x *= MAKE_HEX_FLOAT(0x1.0p24f, 0x1L, 24);
    //  int r = (int)x;
    eq1 = _mm_cmplt_ps(absParam, *((float4 *)x38800000));
    eq2 = eq1;
    eq1 = _mm_and_ps(eq1, absParam);
    eq1 = _mm_mul_ps(eq1, *((float4 *)x4b800000));  //x
    eq0 = _mm_castsi128_ps(_mm_cvttps_epi32(eq1)); //r

    // r += (float)r != x && f < 0.0f;
    float4 eq3 = _mm_cvtepi32_ps(_mm_castps_si128(eq0)); //(float)r
    eq1 = _mm_cmpneq_ps(eq1, eq3); // (float)r != x
    eq3 = _mm_cmplt_ps(param, zeros); //f < 0.0f
    eq1 = _mm_and_ps(eq1, eq3); //(float)r != x && f < 0.0f
    float4 eq4 = _mm_and_ps(eq1, *(float4*)conversion_ones);
    eq0 = _mm_castsi128_ps(_mm_add_epi32(_mm_castps_si128(eq0), _mm_castps_si128(eq4)));
    // return (short)(r | sign)
    eq0 = _mm_or_ps(eq0, signs); 
    eq0 = _mm_and_ps(eq2, eq0);
    eq0 = _mm_andnot_ps(dflt, eq0);
    eq  = _mm_or_ps(eq, eq0); 
    dflt = _mm_or_ps(eq2, dflt);

    //u.u &= 0xFFFFE000U;
    eq0 = _mm_and_ps(param, *(float4*)xffffe000);
    //if (u.f > f)
    //u.u += 0x00002000U;
    eq1 = _mm_cmpgt_ps(eq0, param);
    eq2 = _mm_castsi128_ps(
        _mm_add_epi32(
        _mm_castps_si128(eq0), _mm_castps_si128(_mm_and_ps(eq1, *(float4*)x00002000))));
    //u.u -= 0x38000000U;
    //return ((u.u >> 13) | sign);
    eq2 = _mm_castsi128_ps(_mm_sub_epi32(_mm_castps_si128(eq2), *(__m128i*)x38000000));
    eq2 = _mm_castsi128_ps(_mm_srli_epi32(_mm_castps_si128(eq2), 13));
    eq2 = _mm_or_ps(eq2, signs);
    eq2 = _mm_andnot_ps(dflt, eq2);
    eq  = _mm_or_ps(eq, eq2); 

    eq1 = _mm_castsi128_ps(_mm_shuffle_epi8((__m128i)eq, *((__m128i *)g_vls_4x32to4x16)));

    return eq1;
}


// define function calling half4 twice with RMODE
#define PROTO_OCL_FLOAT2HALF8_ASHALF4(RMODE)			\
		float4 _ocl_float2half8##RMODE(float8 param)		\
		{\
		float4 res;\
		res.lo = _ocl_float2half##RMODE(param.lo).lo;\
		res.hi = _ocl_float2half##RMODE(param.hi).lo;\
		return res;\
		}

PROTO_OCL_FLOAT2HALF8_ASHALF4(_rte)
PROTO_OCL_FLOAT2HALF8_ASHALF4(_rtz)
PROTO_OCL_FLOAT2HALF8_ASHALF4(_rtn)
PROTO_OCL_FLOAT2HALF8_ASHALF4(_rtp)

#endif // defined(__AVX2__)

/// !!! This function is copy-pasted to images module.
/// In case of any changes they should also be applied to image_callback_functions.cpp
float4 _ocl_float2half(float4 param)
{
    return _ocl_float2half_rte(param);
}

float4 _ocl_float2half8(float8 param)
{
	return _ocl_float2half8_rte(param);
}

float4 _ocl_double2ToHalf2_rte(double2 param);
float4 _ocl_double2ToHalf2_rtz(double2 param);
float4 _ocl_double2ToHalf2_rtn(double2 param);
float4 _ocl_double2ToHalf2_rtp(double2 param);

float4 _ocl_double4ToHalf4_rte(double4 param);
float4 _ocl_double4ToHalf4_rtz(double4 param);
float4 _ocl_double4ToHalf4_rtn(double4 param);
float4 _ocl_double4ToHalf4_rtp(double4 param);
float4 _ocl_double4ToHalf4(double4 param);


// float4 _ocl_double2ToHalf2_rte(double2 param). 
// Rounding toward nearest cannot be implemented using convert_float and vstore_half
// because of incorrect rounding from double->float in rte when double can be converted to float
// which is not nearest to original double

float4 _ocl_double2ToHalf2_rtp(double2 param)
{
	float2 f2 = convert_float2_rtp(param);
	float4 res;
	half2 t;
	vstore_half2_rtp(f2, 0, (half*)&t);
	res.s0 = as_float(t);
	return res;
}

float4 _ocl_double2ToHalf2_rtn(double2 param)
{
	float2 f2 = convert_float2_rtn(param);
	float4 res;
	half2 t;
	vstore_half2_rtn(f2, 0, (half*)&t);
	res.s0 = as_float(t);
	return res;
}

float4 _ocl_double2ToHalf2_rtz(double2 param)
{
	float2 f2 = convert_float2_rtz(param);
	float4 res;
	half2 t;
	vstore_half2_rtz(f2, 0, (half*)&t);
	res.s0 = as_float(t);
	return res;
}
#if defined(__AVX2__)
/*
	AVX2 does not have direct conversion from double to half.
	Below is hack implementation suggested by Bob Hanek

The sequence double -> float, float->half float suffers from double rounding
when the rounding mode is round to nearest. 
If you want to have a correctly rounded result at the end 
you need to “pre-condition” the double values via some bit twiddling.

The specific issue is that some of ‘sticky bits’ 
that are required for the final rounding are lost during the first rounding. 

A way to fix the problem in to manually include the lost sticky bits 
into the original double value via sequence of logical ands and ors

*/
float4 _ocl_double2ToHalf2_rte(double2 param)
{
	typedef union { double2 d; long2 ix; } double2_t;
	double2_t x,j;
	float2 f;
	float4 res;
	half2 h;
	x.d = param;
	// The key point  is to set the 29 bit to 1 if any of bits 28:0 are 1
	j.ix = x.ix + 0x1fffffff;
	j.ix &= 0x20000000;
	x.ix |= j.ix;
	x.ix &= ~0x1fffffff;
	f = convert_float2_rte(x.d);
	vstore_half2_rte(f, 0, (half*)&h);
	res.s0 = as_float(h);
	return res;	
}
float4 _ocl_double4ToHalf4_rte(double4 param)		
{
	typedef union { double4 d; long4 ix; } double4_t;
	double4_t x,j;
	float4 f,res;
	half4 h;
	x.d = param;
	// The key point  is to set the 29 bit to 1 if any of bits 28:0 are 1
	j.ix = x.ix + 0x1fffffff;
	j.ix &= 0x20000000;
	x.ix |= j.ix;
	x.ix &= ~0x1fffffff;
	f = convert_float4_rte(x.d);
	vstore_half4_rte(f, 0, (half*)&h);
	res.s01 = as_float2(h);
	return res;	
}

#else // defined(__AVX2__)
float4 _ocl_double2ToHalf2_rte(double2 param)
{
    //cl_ulong sign = (u.u >> 48) & 0x8000;
    //double x = fabs(f);
    double2 temp = _mm_castsi128_pd(_mm_srli_epi64(_mm_castpd_si128(param), 48));
    double2 signs = _mm_and_pd(temp, *(double2 *)x8000);
    double2 absParam = _mm_and_pd(param, *(double2*)x7fffffffffffffff);

    //Nan
    //if( x != x ) 
    //	u.u >>= (53-11);
    //	u.u &= 0x7fff;
    //	u.u |= 0x0200;   -- silence the NaN
    //	return u.u | sign;

    double2 eq0 = _mm_cmpneq_pd(absParam, absParam);
    double2 eq = _mm_and_pd(absParam, eq0);
    eq = _mm_castsi128_pd(_mm_srli_epi64(_mm_castpd_si128(eq), 42));
    eq = _mm_and_pd(eq, *(double2*)x7fff);
    eq = _mm_or_pd(eq, *(double2*)x0200);
    eq = _mm_or_pd(eq, signs);
    eq = _mm_and_pd(eq, eq0);
    double2 dflt = eq0;

    //// overflow
    ////if( x >= MAKE_HEX_DOUBLE(0x1.ffep15, 0x1ffeL, 3) )
    ////         0x40effe0000000000
    ////return 0x7c00 | sign;

    double2 eq1 = _mm_cmpge_pd(absParam, *(double2*)x40effe0000000000);
    eq0 = _mm_and_pd(eq1, *(double2*)x7c00);
    eq0 = _mm_or_pd(signs, eq0);
    eq0 = _mm_andnot_pd(dflt, eq0);
    eq = _mm_or_pd(eq, eq0);
    dflt = _mm_or_pd(eq1, dflt);

    //// underflow
    ////	if( x <= MAKE_HEX_DOUBLE(0x1.0p-25, 0x1L, -25) )
    ////			 0x3e60000000000000
    //// return sign
    eq1 = _mm_cmple_pd(absParam, *(double2*)x3e60000000000000);
    eq0 = _mm_and_pd(eq1, signs);
    eq0 = _mm_andnot_pd(dflt, eq0);
    eq = _mm_or_pd(eq, eq0);
    dflt = _mm_or_pd(eq1, dflt);


    //// very small
    ////	if( x < MAKE_HEX_DOUBLE(0x1.8p-24, 0x18L, -28) )
    ////			0x3e78000000000000
    //// return sign | 1;
    eq1 = _mm_cmplt_pd(absParam, *(double2*)x3e78000000000000);
    eq0 = _mm_and_pd(eq1, _mm_or_pd(signs, *(double2*)dones));
    eq0 = _mm_andnot_pd(dflt, eq0);
    eq = _mm_or_pd( eq, eq0);
    dflt = _mm_or_pd(eq1, dflt);

    //// half denormal         
    ////  if( x < MAKE_HEX_DOUBLE(0x1.0p-14, 0x1L, -14) )
    ////			0x3f10000000000000
    ////	u.f = x * MAKE_HEX_DOUBLE(0x1.0p-1050, 0x1L, -1050);
    ////			  0x0000000001000000
    ////  return sign | x;
    eq1 = _mm_cmplt_pd(absParam, *(double2*)x3f10000000000000);
    double2 eq2 = _mm_mul_pd(absParam, *(double2*)x0000000001000000);  //x
    eq0 = _mm_and_pd(eq1, _mm_or_pd(signs, eq2));
    eq0 = _mm_andnot_pd(dflt, eq0);
    eq = _mm_or_pd(eq, eq0);
    dflt = _mm_or_pd(eq1, dflt);

    //// u.f *= MAKE_HEX_DOUBLE(0x1.0p42, 0x1L, 42);
    ////		  0x4290000000000000
    //// u.u &= 0x7ff0000000000000UL;
    //// x += u.f;
    //// u.f = x - u.f;
    //// u.f *= MAKE_HEX_DOUBLE(0x1.0p-1008, 0x1L, -1008);
    ////		  0x00f0000000000000
    //// return (u.u >> (53-11)) | sign;
    //
    double2 res = _mm_mul_pd(param, *(double2 *)x4290000000000000);  
    double2 tmp = _mm_and_pd(res, *(double2*)x7ff0000000000000); //u
    res = _mm_add_pd( tmp ,absParam); //x
    res = _mm_sub_pd( res, tmp); //u
    res = _mm_mul_pd(res, *(double2*)x00f0000000000000);
    res = _mm_castsi128_pd(_mm_srli_epi64(_mm_castpd_si128(res), 42));
    res = _mm_or_pd(res, signs);
    res = _mm_andnot_pd(dflt, res);
    eq = _mm_or_pd(eq, res);

    return _mm_castsi128_ps(_mm_shuffle_epi8(_mm_castpd_si128(eq), *(__m128i*)g_vls_2x64to2x16));
}

// call double2 version
float4 _ocl_double4ToHalf4_rte(double4 param)		
{
		float4 res;
		res.s0 = _ocl_double2ToHalf2_rte(param.lo).s0;
		res.s1 = _ocl_double2ToHalf2_rte(param.hi).s0;
		return res;
}

#endif // defined(__AVX2__) 

float4 _ocl_double4ToHalf4_rtp(double4 param)		
{
	float4 f4 = convert_float4_rtp(param);
	float4 res;
	half4 t;
	vstore_half4_rtp(f4, 0, (half*)&t);
	res.s01 = as_float2(t);
	return res;
}

float4 _ocl_double4ToHalf4_rtn(double4 param)		
{
	float4 f4 = convert_float4_rtn(param);
	float4 res;
	half4 t;
	vstore_half4_rtn(f4, 0, (half*)&t);
	res.s01 = as_float2(t);
	return res;
}

float4 _ocl_double4ToHalf4_rtz(double4 param)		
{
	float4 f4 = convert_float4_rtz(param);
	float4 res;
	half4 t;
	vstore_half4_rtz(f4, 0, (half*)&t);
	res.s01 = as_float2(t);
	return res;
}


float4 _ocl_double2ToHalf2(double2 param)
{
	return _ocl_double2ToHalf2_rte(param);
}

float4 _ocl_double4ToHalf4(double4 param)
{
	return _ocl_double4ToHalf4_rte(param);
}

#define DEF_VLOADVSTORE_PROTOV_X_X_Y(FUNC, TI, TYP, ADR, SIGN, SIZ, NUM, VEC)\
	_##VEC##TI##NUM __attribute__((overloadable)) FUNC##VEC(size_t offset,const ADR SIGN SIZ *ptr)\
	{\
		void* pSrc = ((char*)ptr + (offset * VEC * sizeof(SIZ)));\
		_##VEC##TI##NUM res;\
		memcpy((void*)&res, pSrc, VEC*sizeof(SIZ));\
		return res;\
	}

#define DEF_VLOADVSTORE_PROTO8_X_X_Y(FUNC, TI, TYP, ADR, SIGN, SIZ, NUM)\
	DEF_VLOADVSTORE_PROTOV_X_X_Y(FUNC, TI, TYP, ADR, SIGN, SIZ, NUM, 2)\
	DEF_VLOADVSTORE_PROTOV_X_X_Y(FUNC, TI, TYP, ADR, SIGN, SIZ, NUM, 3)\
	DEF_VLOADVSTORE_PROTOV_X_X_Y(FUNC, TI, TYP, ADR, SIGN, SIZ, NUM, 4)\
	DEF_VLOADVSTORE_PROTOV_X_X_Y(FUNC, TI, TYP, ADR, SIGN, SIZ, NUM, 8)\
	DEF_VLOADVSTORE_PROTOV_X_X_Y(FUNC, TI, TYP, ADR, SIGN, SIZ, NUM, 16)

#define DEF_VLOADVSTORE_PROTOFV_X_X_Y(FUNC, TYP, ADR, VEC)\
	float##VEC __attribute__((overloadable)) FUNC##VEC(size_t offset, const ADR float *ptr)\
	{\
		const void *pSrc = ((char*)ptr + (offset * VEC * sizeof(float)));\
		float##VEC res;\
		memcpy((void*)&res, pSrc, VEC*sizeof(float));\
		return res;\
	}

#define DEF_VLOADVSTORE_PROTODV_X_X_Y(FUNC, TYP, ADR, VEC)\
	double##VEC __attribute__((overloadable)) FUNC##VEC(size_t offset, const ADR double *ptr)\
	{\
		const void *pSrc = ((char*)ptr + (offset * VEC * sizeof(double)));\
		double##VEC res;\
		memcpy((void*)&res, pSrc, VEC*sizeof(double));\
		return res;\
	}

#define DEF_VLOADVSTORE_PROTOF_X_X_Y(FUNC, TYP, ADR)\
	DEF_VLOADVSTORE_PROTOFV_X_X_Y(FUNC, TYP, ADR, 2)\
	DEF_VLOADVSTORE_PROTODV_X_X_Y(FUNC, TYP, ADR, 2)\
	DEF_VLOADVSTORE_PROTOFV_X_X_Y(FUNC, TYP, ADR, 3)\
	DEF_VLOADVSTORE_PROTODV_X_X_Y(FUNC, TYP, ADR, 3)\
	DEF_VLOADVSTORE_PROTOFV_X_X_Y(FUNC, TYP, ADR, 4)\
	DEF_VLOADVSTORE_PROTODV_X_X_Y(FUNC, TYP, ADR, 4)\
	DEF_VLOADVSTORE_PROTOFV_X_X_Y(FUNC, TYP, ADR, 8)\
	DEF_VLOADVSTORE_PROTODV_X_X_Y(FUNC, TYP, ADR, 8)\
	DEF_VLOADVSTORE_PROTOFV_X_X_Y(FUNC, TYP, ADR, 16)\
	DEF_VLOADVSTORE_PROTODV_X_X_Y(FUNC, TYP, ADR, 16)

#define DEF_VLOADVSTORE_PROTO_HALF_X_X_Y(A, TYP, ADR, Alligned)\
	float __attribute__((overloadable))  vload##A##_half(size_t offset,const ADR half *ptr)\
	{\
		return HalfToFloat(ptr[offset]);\
	}\
	float4 __attribute__((overloadable))  vload##A##_half4(size_t offset,const ADR half *ptr)\
	{\
		return Half4ToFloat4((_8i16)_mm_lddqu_si128((__m128i *)(ptr + 4*offset)));\
	}\
	float3 __attribute__((overloadable))  vload##A##_half3(size_t offset,const ADR half *ptr)\
	{\
		if( Alligned )\
		{\
			float4 temp = Half4ToFloat4((_8i16)_mm_lddqu_si128((__m128i *)(ptr + 4*offset)));\
			return temp.s012;\
		}\
		else\
		{\
			float4 temp = Half4ToFloat4((_8i16)_mm_lddqu_si128((__m128i *)(ptr + 3*offset)));\
			return temp.s012;\
		}\
	}\
	float2 __attribute__((overloadable))  vload##A##_half2(size_t offset,const ADR half *ptr)\
	{\
		float4 temp = Half4ToFloat4((_8i16)_mm_lddqu_si128((__m128i *)(ptr + 2*offset)));\
		return temp.lo;\
	}\
	float8 __attribute__((overloadable))  vload##A##_half8(size_t offset,const ADR half *ptr)\
	{\
		float8 res;\
		const _8i16 inp = vload8(offset, (short*)ptr);\
		return Half8ToFloat8(inp);\
	}\
	float16 __attribute__((overloadable))  vload##A##_half16(size_t offset,const ADR half *ptr)\
	{\
		float16 res;\
		const _16i16 inp = vload16(offset, (short*)ptr);\
		res.lo = Half8ToFloat8(inp.lo);\
        res.hi = Half8ToFloat8(inp.hi);\
		return res;	\
	}

#define DEF_VLOADVSTORE_PROTO8V_X_X_X(FUNC, TI, TYP, ADR, SIGN, SIZ, NUM, VEC)\
	void __attribute__((overloadable)) FUNC##VEC(_##VEC##TI##NUM data, size_t offset, ADR SIGN SIZ *ptr)\
	{\
		void* pDst = ((char*)ptr + (offset * VEC * sizeof(SIZ)));\
		memcpy(pDst, (const void*)&data, VEC * sizeof(SIZ));\
	}

#define DEF_VLOADVSTORE_PROTO8_X_X_X(FUNC, TI, TYP, ADR, SIGN, SIZ, NUM)\
	DEF_VLOADVSTORE_PROTO8V_X_X_X(FUNC, TI, TYP, ADR, SIGN, SIZ, NUM, 2)\
	DEF_VLOADVSTORE_PROTO8V_X_X_X(FUNC, TI, TYP, ADR, SIGN, SIZ, NUM, 3)\
	DEF_VLOADVSTORE_PROTO8V_X_X_X(FUNC, TI, TYP, ADR, SIGN, SIZ, NUM, 4)\
	DEF_VLOADVSTORE_PROTO8V_X_X_X(FUNC, TI, TYP, ADR, SIGN, SIZ, NUM, 8)\
	DEF_VLOADVSTORE_PROTO8V_X_X_X(FUNC, TI, TYP, ADR, SIGN, SIZ, NUM, 16)

#define DEF_VLOADVSTORE_PROTOFV_X_X_X(FUNC, TYP, ADR, VEC)\
	void __attribute__((overloadable)) FUNC##VEC(float##VEC data, size_t offset, ADR float *ptr)\
	{\
		void* pDst = ((char*)ptr + (offset * VEC * sizeof(float)));\
		memcpy(pDst, (const void*)&data, VEC * sizeof(float));\
	}

#define DEF_VLOADVSTORE_PROTODV_X_X_X(FUNC, TYP, ADR, VEC)\
	void __attribute__((overloadable)) FUNC##VEC(double##VEC data, size_t offset, ADR double *ptr)\
	{\
		void* pDst = ((char*)ptr + (offset * VEC * sizeof(double)));\
		memcpy(pDst, (const void*)&data, VEC * sizeof(double));\
	}
	
#define DEF_VLOADVSTORE_PROTOF_X_X_X(FUNC, TYP, ADR)\
	DEF_VLOADVSTORE_PROTOFV_X_X_X(FUNC, TYP, ADR, 2)\
	DEF_VLOADVSTORE_PROTODV_X_X_X(FUNC, TYP, ADR, 2)\
	DEF_VLOADVSTORE_PROTOFV_X_X_X(FUNC, TYP, ADR, 3)\
	DEF_VLOADVSTORE_PROTODV_X_X_X(FUNC, TYP, ADR, 3)\
	DEF_VLOADVSTORE_PROTOFV_X_X_X(FUNC, TYP, ADR, 4)\
	DEF_VLOADVSTORE_PROTODV_X_X_X(FUNC, TYP, ADR, 4)\
	DEF_VLOADVSTORE_PROTOFV_X_X_X(FUNC, TYP, ADR, 8)\
	DEF_VLOADVSTORE_PROTODV_X_X_X(FUNC, TYP, ADR, 8)\
	DEF_VLOADVSTORE_PROTOFV_X_X_X(FUNC, TYP, ADR, 16)\
	DEF_VLOADVSTORE_PROTODV_X_X_X(FUNC, TYP, ADR, 16)

#define DEF_VLOADVSTORE_PROTO_RTX_HALF_X_X_X(RMODE, A, TYP, ADR, Alligned)\
	void __attribute__((overloadable))  vstore##A##_half##RMODE(float data, size_t offset, ADR half *ptr)\
	{\
		float4 f4;\
		f4.s0 = data;\
		f4 = _ocl_float2half##RMODE(f4);\
		((short *)ptr)[offset] = (short)_mm_extract_epi16( (__m128i)f4, 0);\
	}\
	void __attribute__((overloadable))  vstore##A##_half2##RMODE(float2 data, size_t offset, ADR half *ptr)\
	{\
		float4 f4;\
		f4.lo = data;\
		ptr = ptr + (offset*2);\
		f4 = _ocl_float2half##RMODE(f4);\
		*((float*)ptr) = f4.s0;\
	}\
	void __attribute__((overloadable))  vstore##A##_half3##RMODE(float3 data, size_t offset, ADR half *ptr)\
	{\
		float4 f4;\
		f4.s012 = data;\
		f4 = _ocl_float2half##RMODE(f4);\
		if(Alligned)\
		{\
			ptr = ptr + (offset*4);\
			_mm_storel_epi64((__m128i*)ptr, (__m128i)f4);\
		}\
		else\
		{\
			ptr = ptr + (offset*3);\
			*((float*)ptr) = f4.s0;\
			((short*)ptr)[2] = (short)_mm_extract_epi16( (__m128i)f4, 2);\
		}\
	}\
	/* !!! This function is copy-pasted to images module
	In case of any changes they should also be done in image_callback_functions.cpp */\
	void __attribute__((overloadable))  vstore##A##_half4##RMODE(float4 data, size_t offset, ADR half *ptr)\
	{\
		ptr = ptr + (offset*4);\
		data = _ocl_float2half##RMODE(data);\
		*((float2 *)ptr) = data.lo;\
	}\
	void __attribute__((overloadable))  vstore##A##_half8##RMODE(float8 data, size_t offset, ADR half *ptr)\
	{\
		float4 res = _ocl_float2half8##RMODE(data);\
		vstore8(*(short8*)&res, offset, (short*)ptr);\
	}\
	void __attribute__((overloadable))  vstore##A##_half16##RMODE(float16 data, size_t offset, ADR half *ptr)\
	{\
		float8 res;\
		res.lo = _ocl_float2half8##RMODE(data.lo);\
		res.hi = _ocl_float2half8##RMODE(data.hi);\
		vstore16(*(short16*)&res, offset, (short*)ptr);\
	}\
	void __attribute__((overloadable))  vstore##A##_half##RMODE(double data, size_t offset, ADR half *ptr)\
	{\
		double2 d2 = (double2)(0.0);\
		d2.x = data;\
		float4 f4 = _ocl_double2ToHalf2##RMODE(d2);\
		((short *)ptr)[offset] = (short)_mm_extract_epi16( (__m128i)f4, 0);\
	}\
	void __attribute__((overloadable))  vstore##A##_half2##RMODE(double2 data, size_t offset, ADR half *ptr)\
	{\
		ptr = ptr + (offset*2);\
		float4 f4 = _ocl_double2ToHalf2##RMODE(data);\
		*((float*)ptr) = f4.s0;\
	}\
	void __attribute__((overloadable))  vstore##A##_half3##RMODE(double3 data, size_t offset, ADR half *ptr)\
	{\
		float4 t1, t2, f4;\
		t1 = _ocl_double2ToHalf2##RMODE(data.xy);\
		t2 = _ocl_double2ToHalf2##RMODE(data.zz);\
		f4.x = t1.x;\
		f4.y = t2.x;\
		if(Alligned)\
		{\
			ptr = ptr + (offset*4);\
			_mm_storel_epi64((__m128i*)ptr, (__m128i)f4);\
		}\
		else\
		{\
			ptr = ptr + (offset*3);\
			*((float*)ptr) = f4.s0;\
			((short *)ptr)[2] = (short)_mm_extract_epi16( (__m128i)f4, 2);\
		}\
	}\
	void __attribute__((overloadable))  vstore##A##_half4##RMODE(double4 data, size_t offset, ADR half *ptr)\
	{\
		ptr = ptr + (offset*4);\
		float4 t1, t2, f4;\
		t1 = _ocl_double4ToHalf4##RMODE(data);\
		_mm_storel_epi64((__m128i*)ptr, (__m128i)t1);\
	}\
	void __attribute__((overloadable))  vstore##A##_half8##RMODE(double8 data, size_t offset, ADR half *ptr)\
	{\
		ptr = ptr + (offset*8);\
		float8 f8;\
		float4 t1, t2;\
		t1 = _ocl_double4ToHalf4##RMODE(data.s0123);\
		t2 = _ocl_double4ToHalf4##RMODE(data.s4567);\
		f8.xy = t1.lo;\
		f8.zw = t2.lo;\
		if(Alligned)\
			_mm_store_ps((float*)ptr, f8.lo);\
		else\
			_mm_storeu_ps((float*)ptr, f8.lo);\
	}\
	void __attribute__((overloadable))  vstore##A##_half16##RMODE(double16 data, size_t offset, ADR half *ptr)\
	{\
		ptr = ptr + (offset*16);\
		float16 f16;\
		float4 t1, t2, t3, t4, t5, t6, t7, t8;\
		t1 = _ocl_double2ToHalf2##RMODE(data.s01);\
		t2 = _ocl_double2ToHalf2##RMODE(data.s23);\
		t3 = _ocl_double2ToHalf2##RMODE(data.s45);\
		t4 = _ocl_double2ToHalf2##RMODE(data.s67);\
		t5 = _ocl_double2ToHalf2##RMODE(data.s89);\
		t6 = _ocl_double2ToHalf2##RMODE(data.sAB);\
		t7 = _ocl_double2ToHalf2##RMODE(data.sCD);\
		t8 = _ocl_double2ToHalf2##RMODE(data.sEF);\
		f16.s0 = t1.x;\
		f16.s1 = t2.x;\
		f16.s2 = t3.x;\
		f16.s3 = t4.x;\
		f16.s4 = t5.x;\
		f16.s5 = t6.x;\
		f16.s6 = t7.x;\
		f16.s7 = t8.x;\
		if(Alligned)\
		{\
			_mm_store_ps((float*)ptr, f16.lo.lo);\
			ptr = ptr + 8;\
			_mm_store_ps((float*)ptr, f16.lo.hi);\
		}\
		else\
		{\
			_mm_storeu_ps((float*)ptr, f16.lo.lo);\
			ptr = ptr + 8;\
			_mm_storeu_ps((float*)ptr, f16.lo.hi);\
		}\
	}

#define DEF_LOAD_INT(FUNC, TI, TYP, ADR, SIGN)\
	DEF_VLOADVSTORE_PROTO8_X_X_Y(FUNC, TI, TYP, ADR, SIGN, char, 8)\
	DEF_VLOADVSTORE_PROTO8_X_X_Y(FUNC, TI, TYP, ADR, SIGN, short, 16)\
	DEF_VLOADVSTORE_PROTO8_X_X_Y(FUNC, TI, TYP, ADR, SIGN, int, 32)\
	DEF_VLOADVSTORE_PROTO8_X_X_Y(FUNC, TI, TYP, ADR, SIGN, _1i64, 64)

#define DEF_STORE_INT(FUNC, TI, TYP, ADR, SIGN)\
	DEF_VLOADVSTORE_PROTO8_X_X_X(FUNC, TI, TYP, ADR, SIGN, char, 8)\
	DEF_VLOADVSTORE_PROTO8_X_X_X(FUNC, TI, TYP, ADR, SIGN, short, 16)\
	DEF_VLOADVSTORE_PROTO8_X_X_X(FUNC, TI, TYP, ADR, SIGN, int, 32)\
	DEF_VLOADVSTORE_PROTO8_X_X_X(FUNC, TI, TYP, ADR, SIGN, _1i64, 64)


#define DEF_LOAD_ALL(TYP,ADR)\
	DEF_LOAD_INT(vload, i, TYP, ADR, )\
	DEF_LOAD_INT(vload, u, TYP, ADR, unsigned)\
	DEF_VLOADVSTORE_PROTOF_X_X_Y(vload, TYP, ADR)\
	DEF_VLOADVSTORE_PROTO_HALF_X_X_Y(, TYP, ADR, false)\
	DEF_VLOADVSTORE_PROTO_HALF_X_X_Y(a, TYP, ADR, true)

#define DEF_STORE_ALL(TYP,ADR)\
	DEF_STORE_INT(vstore, i, TYP, ADR, )\
	DEF_STORE_INT(vstore, u, TYP, ADR, unsigned)\
	DEF_VLOADVSTORE_PROTOF_X_X_X(vstore, TYP, ADR)\
	DEF_VLOADVSTORE_PROTO_RTX_HALF_X_X_X(,, TYP, ADR, false)\
	DEF_VLOADVSTORE_PROTO_RTX_HALF_X_X_X(_rte,, TYP, ADR, false)\
	DEF_VLOADVSTORE_PROTO_RTX_HALF_X_X_X(_rtz,, TYP, ADR, false)\
	DEF_VLOADVSTORE_PROTO_RTX_HALF_X_X_X(_rtn,, TYP, ADR, false)\
	DEF_VLOADVSTORE_PROTO_RTX_HALF_X_X_X(_rtp,, TYP, ADR, false)\
	DEF_VLOADVSTORE_PROTO_RTX_HALF_X_X_X(, a, TYP, ADR, true)\
	DEF_VLOADVSTORE_PROTO_RTX_HALF_X_X_X(_rte, a, TYP, ADR, true)\
	DEF_VLOADVSTORE_PROTO_RTX_HALF_X_X_X(_rtz, a, TYP, ADR, true)\
	DEF_VLOADVSTORE_PROTO_RTX_HALF_X_X_X(_rtn, a, TYP, ADR, true)\
	DEF_VLOADVSTORE_PROTO_RTX_HALF_X_X_X(_rtp, a, TYP, ADR, true)


DEF_LOAD_ALL( , )
DEF_LOAD_ALL(l,__local)
DEF_LOAD_ALL(g,__global)
DEF_LOAD_ALL(c,__constant)

DEF_STORE_ALL( , )
DEF_STORE_ALL(l,__local)
DEF_STORE_ALL(g,__global)

#ifdef __cplusplus
}
#endif

#endif // !defined (__MIC__) && !defined(__MIC2__)
