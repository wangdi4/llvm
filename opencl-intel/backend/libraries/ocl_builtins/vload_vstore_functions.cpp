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

#define _INTERNAL_DEBUG

#ifdef __cplusplus
extern "C" {
#endif

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


#ifdef __SSSE3__
	#define SHUFFLE_EPI8(x, mask, res)\
		res = (float4)_mm_shuffle_epi8((__m128i)x, *((__m128i*)mask));
#else
	#define SHUFFLE_EPI8(x, mask, res)									\
		ALIGN16 _1i8 tempX[16];											\
		ALIGN16 _1i8 tampMask[16];										\
		ALIGN16 _1i8 result[16];										\
																		\
		_mm_store_si128((__m128i *)tempX, x);							\
		_mm_store_si128((__m128i *)tampMask, *((__m128i*)mask));		\
		for(int i=0; i<16; ++i)											\
		{																\
			_1i8 index = tampMask[i] & 15;								\
			result[i] = (tampMask[i] < 0) ? 0 : tempX[index];			\
		}																\
		res = (float4)_mm_load_si128((const __m128i *)result);			
#endif

float HalfToFloat( half param )
{
	unsigned short expHalf16 = param & 0x7C00;
	int exp1 = (int)expHalf16;
	unsigned short mantissa16 = param & 0x03FF;
	int mantissa1 = (int)mantissa16;
	int sign = (int)(param & 0x8000);
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

float4 float2half_rte(float4 param);
float4 float2half_rtz(float4 param);
float4 float2half_rtn(float4 param);
float4 float2half_rtp(float4 param);

float4 float2half(float4 param)
{
	return float2half_rte(param);
}

float4 double2ToHalf2_rte(double2 param);
float4 double2ToHalf2_rtz(double2 param);
float4 double2ToHalf2_rtn(double2 param);
float4 double2ToHalf2_rtp(double2 param);

float4 double2ToHalf2(double2 param)
{
	return double2ToHalf2_rte(param);
}

#define DEF_VLOADVSTORE_PROTOV_X_X_Y(FUNC, TI, TYP, ADR, SIGN, SIZ, NUM, VEC)\
	_##VEC##TI##NUM __attribute__((overloadable)) FUNC##VEC(size_t offset,const SIGN SIZ __attribute__((address_space(ADR))) *ptr)\
	{\
		const void* pSrc = ((char*)ptr + (offset * VEC * sizeof(SIZ)));\
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
	float##VEC __attribute__((overloadable)) FUNC##VEC(size_t offset, const float __attribute__((address_space(ADR))) *ptr)\
	{\
		const void *pSrc = ((char*)ptr + (offset * VEC * sizeof(float)));\
		float##VEC res;\
		memcpy((void*)&res, pSrc, VEC*sizeof(float));\
		return res;\
	}

#define DEF_VLOADVSTORE_PROTODV_X_X_Y(FUNC, TYP, ADR, VEC)\
	double##VEC __attribute__((overloadable)) FUNC##VEC(size_t offset, const double __attribute__((address_space(ADR))) *ptr)\
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
	float __attribute__((overloadable))  vload##A##_half(size_t offset,const half __attribute__((address_space(ADR))) *ptr)\
	{\
		return HalfToFloat(ptr[offset]);\
	}\
	float4 __attribute__((overloadable))  vload##A##_half4(size_t offset,const half __attribute__((address_space(ADR))) *ptr)\
	{\
		return Half4ToFloat4((_8i16)_mm_lddqu_si128((__m128i *)(ptr + 4*offset)));\
	}\
	float3 __attribute__((overloadable))  vload##A##_half3(size_t offset,const half __attribute__((address_space(ADR))) *ptr)\
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
	float2 __attribute__((overloadable))  vload##A##_half2(size_t offset,const half __attribute__((address_space(ADR))) *ptr)\
	{\
		float4 temp = Half4ToFloat4((_8i16)_mm_lddqu_si128((__m128i *)(ptr + 2*offset)));\
		return temp.lo;\
	}\
	float8 __attribute__((overloadable))  vload##A##_half8(size_t offset,const half __attribute__((address_space(ADR))) *ptr)\
	{\
		float8 res;\
		const half __attribute__((address_space(ADR))) *p = ptr + (offset * 8);\
		res.lo = Half4ToFloat4((_8i16)_mm_lddqu_si128((__m128i *)p));\
		p = p + 4;\
		res.hi = Half4ToFloat4((_8i16)_mm_lddqu_si128((__m128i *)p));\
		return res;\
	}\
	float16 __attribute__((overloadable))  vload##A##_half16(size_t offset,const half __attribute__((address_space(ADR))) *ptr)\
	{\
		float16 res;\
		const half __attribute__((address_space(ADR))) *p = ptr + (offset * 16);\
		res.lo.lo = Half4ToFloat4((_8i16)_mm_lddqu_si128((__m128i *)p));\
		p = p + 4;\
		res.lo.hi = Half4ToFloat4((_8i16)_mm_lddqu_si128((__m128i *)p));\
		p = p + 4;\
		res.hi.lo = Half4ToFloat4((_8i16)_mm_lddqu_si128((__m128i *)p));\
		p = p + 4;\
		res.hi.hi = Half4ToFloat4((_8i16)_mm_lddqu_si128((__m128i *)p));\
		return res;	\
	}

#define DEF_VLOADVSTORE_PROTO8V_X_X_X(FUNC, TI, TYP, ADR, SIGN, SIZ, NUM, VEC)\
	void __attribute__((overloadable)) FUNC##VEC(_##VEC##TI##NUM data, size_t offset, SIGN SIZ __attribute__((address_space(ADR))) *ptr)\
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
	void __attribute__((overloadable)) FUNC##VEC(float##VEC data, size_t offset, float __attribute__((address_space(ADR))) *ptr)\
	{\
		void* pDst = ((char*)ptr + (offset * VEC * sizeof(float)));\
		memcpy(pDst, (const void*)&data, VEC * sizeof(float));\
	}

#define DEF_VLOADVSTORE_PROTODV_X_X_X(FUNC, TYP, ADR, VEC)\
	void __attribute__((overloadable)) FUNC##VEC(double##VEC data, size_t offset, double __attribute__((address_space(ADR))) *ptr)\
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
	void __attribute__((overloadable))  vstore##A##_half##RMODE(float data, size_t offset, half __attribute__((address_space(ADR))) *ptr)\
	{\
		float4 f4;\
		f4.s0 = data;\
		f4 = float2half##RMODE(f4);\
		ptr[offset] = (half)_mm_extract_epi16( (__m128i)f4, 0);\
	}\
	void __attribute__((overloadable))  vstore##A##_half2##RMODE(float2 data, size_t offset, half __attribute__((address_space(ADR))) *ptr)\
	{\
		float4 f4;\
		f4.lo = data;\
		ptr = ptr + (offset*2);\
		f4 = float2half##RMODE(f4);\
		*((float*)ptr) = f4.s0;\
	}\
	void __attribute__((overloadable))  vstore##A##_half3##RMODE(float3 data, size_t offset, half __attribute__((address_space(ADR))) *ptr)\
	{\
		float4 f4;\
		f4.s012 = data;\
		f4 = float2half##RMODE(f4);\
		if(Alligned)\
		{\
			ptr = ptr + (offset*4);\
			_mm_storel_epi64((__m128i*)ptr, (__m128i)f4);\
		}\
		else\
		{\
			ptr = ptr + (offset*3);\
			*((float*)ptr) = f4.s0;\
			ptr[2] = (half)_mm_extract_epi16( (__m128i)f4, 2);\
		}\
	}\
	void __attribute__((overloadable))  vstore##A##_half4##RMODE(float4 data, size_t offset, half __attribute__((address_space(ADR))) *ptr)\
	{\
		ptr = ptr + (offset*4);\
		data = float2half##RMODE(data);\
		*((float2 *)ptr) = data.lo;\
	}\
	void __attribute__((overloadable))  vstore##A##_half8##RMODE(float8 data, size_t offset, half __attribute__((address_space(ADR))) *ptr)\
	{\
		ptr = ptr + (offset*8);\
		data.lo = float2half##RMODE(data.lo);\
		data.hi = float2half##RMODE(data.hi);\
		data.lo = (float4)_mm_unpacklo_epi64((__m128i)data.lo, (__m128i)data.hi);\
		if(Alligned)\
			_mm_store_ps((float*)ptr, data.lo);\
		else\
			_mm_storeu_ps((float*)ptr, data.lo);\
	}\
	void __attribute__((overloadable))  vstore##A##_half16##RMODE(float16 data, size_t offset, half __attribute__((address_space(ADR))) *ptr)\
	{\
		ptr = ptr + (offset*16);\
		data.lo.lo = float2half##RMODE(data.lo.lo);\
		data.lo.hi = float2half##RMODE(data.lo.hi);\
		data.lo.lo = (float4)_mm_unpacklo_epi64((__m128i)data.lo.lo, (__m128i)data.lo.hi);\
		if(Alligned)\
		_mm_store_ps((float*)ptr, data.lo.lo);\
		else\
		_mm_storeu_ps((float*)ptr, data.lo.lo);\
		ptr = ptr + 8;\
		data.hi.lo = float2half##RMODE(data.hi.lo);\
		data.hi.hi = float2half##RMODE(data.hi.hi);\
		data.hi.lo = (float4)_mm_unpacklo_epi64((__m128i)data.hi.lo, (__m128i)data.hi.hi);\
		if(Alligned)\
		_mm_store_ps((float*)ptr, data.hi.lo);\
		else\
		_mm_storeu_ps((float*)ptr, data.hi.lo);\
	}\
	void __attribute__((overloadable))  vstore##A##_half##RMODE(double data, size_t offset, half __attribute__((address_space(ADR))) *ptr)\
	{\
		double2 d2;\
		d2.x = data;\
		float4 f4 = double2ToHalf2##RMODE(d2);\
		ptr[offset] = (half)_mm_extract_epi16( (__m128i)f4, 0);\
	}\
	void __attribute__((overloadable))  vstore##A##_half2##RMODE(double2 data, size_t offset, half __attribute__((address_space(ADR))) *ptr)\
	{\
		ptr = ptr + (offset*2);\
		float4 f4 = double2ToHalf2##RMODE(data);\
		*((float*)ptr) = f4.s0;\
	}\
	void __attribute__((overloadable))  vstore##A##_half3##RMODE(double3 data, size_t offset, half __attribute__((address_space(ADR))) *ptr)\
	{\
		float4 t1, t2, f4;\
		t1 = double2ToHalf2##RMODE(data.xy);\
		t2 = double2ToHalf2##RMODE(data.zz);\
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
			ptr[2] = (half)_mm_extract_epi16( (__m128i)f4, 2);\
		}\
	}\
	void __attribute__((overloadable))  vstore##A##_half4##RMODE(double4 data, size_t offset, half __attribute__((address_space(ADR))) *ptr)\
	{\
		ptr = ptr + (offset*4);\
		float4 t1, t2, f4;\
		t1 = double2ToHalf2##RMODE(data.xy);\
		t2 = double2ToHalf2##RMODE(data.zw);\
		f4.x = t1.x;\
		f4.y = t2.x;\
		_mm_storel_epi64((__m128i*)ptr, (__m128i)f4);\
	}\
	void __attribute__((overloadable))  vstore##A##_half8##RMODE(double8 data, size_t offset, half __attribute__((address_space(ADR))) *ptr)\
	{\
		ptr = ptr + (offset*8);\
		float8 f8;\
		float4 t1, t2, t3, t4;\
		t1 = double2ToHalf2##RMODE(data.s01);\
		t2 = double2ToHalf2##RMODE(data.s23);\
		t3 = double2ToHalf2##RMODE(data.s45);\
		t4 = double2ToHalf2##RMODE(data.s67);\
		f8.x = t1.x;\
		f8.y = t2.x;\
		f8.z = t3.x;\
		f8.w = t4.x;\
		if(Alligned)\
			_mm_store_ps((float*)ptr, f8.lo);\
		else\
			_mm_storeu_ps((float*)ptr, f8.lo);\
	}\
	void __attribute__((overloadable))  vstore##A##_half16##RMODE(double16 data, size_t offset, half __attribute__((address_space(ADR))) *ptr)\
	{\
		ptr = ptr + (offset*16);\
		float16 f16;\
		float4 t1, t2, t3, t4, t5, t6, t7, t8;\
		t1 = double2ToHalf2##RMODE(data.s01);\
		t2 = double2ToHalf2##RMODE(data.s23);\
		t3 = double2ToHalf2##RMODE(data.s45);\
		t4 = double2ToHalf2##RMODE(data.s67);\
		t5 = double2ToHalf2##RMODE(data.s89);\
		t6 = double2ToHalf2##RMODE(data.sAB);\
		t7 = double2ToHalf2##RMODE(data.sCD);\
		t8 = double2ToHalf2##RMODE(data.sEF);\
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


DEF_LOAD_ALL( ,0)
DEF_LOAD_ALL(l,3)
DEF_LOAD_ALL(g,1)
DEF_LOAD_ALL(c,2)

DEF_STORE_ALL( ,0)
DEF_STORE_ALL(l,3)
DEF_STORE_ALL(g,1)
DEF_STORE_ALL(c,2)

#ifdef __cplusplus
}
#endif
